/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // std::min, std::max
#include <cmath>  // fabs
#include <iostream>
#include <limits> // std::numeric_limits

#include <poppler/UTF.h>  // UnicodeIsWhitespace

#include "./WordsTokenizer.h"
#include "./PdfDocument.h"

#include "./utils/Utils.h"

// The inter-glyph space width which will cause the tokenize() method to start a new word.
#define minWordBreakSpace 0.15


// _________________________________________________________________________________________________
WordsTokenizer::WordsTokenizer(PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
WordsTokenizer::~WordsTokenizer() = default;

// _________________________________________________________________________________________________
void WordsTokenizer::tokenize() const {
  // Do nothing if no document is given.
  if (!_doc) {
    return;
  }

  // Do nothing if no pages are given.
  if (_doc->pages.size() == 0) {
    return;
  }

  // Process the document page-wise. For each page, iterate through the glyphs in extraction order
  // and merge them to words by analyzing different layout information.
  for (auto* page : _doc->pages) {
    // Do nothing if the page does not contain any glyphs.
    if (page->glyphs.size() == 0) {
      continue;
    }

    std::vector<PdfGlyph*> currentWordGlyphs;
    double currentWordMinX = std::numeric_limits<double>::max();
    double currentWordMinY = std::numeric_limits<double>::max();
    double currentWordMaxX = std::numeric_limits<double>::min();
    double currentWordMaxY = std::numeric_limits<double>::min();
    double maxWordFontSize = std::numeric_limits<double>::min();

    // Consider (previous glyph, current glyph) pairs. For each pair, analyze the spacing, font
    // size and writing mode of both glyphs and decide whether or not there is a word boundary
    // between the two glyphs.
    PdfGlyph* prevGlyph = nullptr;
    for (auto* glyph : page->glyphs) {
      // Ignore diacritic marks, as they were already merged with their base characters.
      if (glyph->isDiacriticMarkOfBaseGlyph) {
        continue;
      }

      if (prevGlyph) {
        // Assume a word boundary between the two glyphs when the writing modes and/or rotations of
        // both glyphs differ.
        if (prevGlyph->wMode != glyph->wMode || prevGlyph->rotation != glyph->rotation) {
          createWord(currentWordGlyphs, &page->words);
          currentWordGlyphs.clear();
          currentWordGlyphs.push_back(glyph);
          currentWordMinX = glyph->minX;
          currentWordMinY = glyph->minY;
          currentWordMaxX = glyph->maxX;
          currentWordMaxY = glyph->maxY;
          maxWordFontSize = glyph->fontSize;
          prevGlyph = glyph;
          continue;
        }

        double hDistanceLeft = 0;
        double hDistanceRight = 0;
        double vOverlap = 0;
        double minMaxX = std::min(currentWordMaxX, glyph->maxX);
        double maxMinX = std::max(currentWordMinX, glyph->minX);
        double minMaxY = std::min(currentWordMaxY, glyph->maxY);
        double maxMinY = std::max(currentWordMinY, glyph->minY);
        double wordHeight = 0;
        double glyphHeight = 0;

        switch (glyph->rotation) {
          case 0:
            {
              hDistanceLeft = glyph->minX - currentWordMaxX;
              hDistanceRight = currentWordMinX - glyph->maxX;
              vOverlap = std::max(.0, minMaxY - maxMinY);
              wordHeight = currentWordMaxY - currentWordMinY;
              glyphHeight = glyph->maxY - glyph->minY;
              break;
            }
          case 1:
            {
              hDistanceLeft = glyph->minY - currentWordMaxY;
              hDistanceRight = currentWordMinY - glyph->maxY;
              vOverlap = std::max(.0, minMaxX - maxMinX);
              break;
            }
          case 2:
            {
              hDistanceLeft = currentWordMinX - glyph->maxX;
              hDistanceRight = glyph->minX - currentWordMaxX;
              vOverlap = std::max(.0, minMaxY - maxMinY);
              break;
            }
          case 3:
            {
              hDistanceLeft = currentWordMinY - glyph->maxY;
              hDistanceRight = glyph->minY - currentWordMaxY;
              vOverlap = std::max(.0, minMaxX - maxMinX);
              break;
            }
        }

        double glyphOverlapRatio = glyphHeight > 0 ? vOverlap / glyphHeight : 0;
        double wordOverlapRatio = wordHeight > 0 ? vOverlap / wordHeight : 0;

        bool startsNewWord = false;

        if (glyphOverlapRatio < 0.5 && wordOverlapRatio < 0.5) {
          startsNewWord = true;
        } else if (hDistanceLeft > minWordBreakSpace * maxWordFontSize) {
          startsNewWord = true;
        } else if (hDistanceRight > minWordBreakSpace * maxWordFontSize) {
          startsNewWord = true;
        }

        // Assume a word boundary between the two glyphs if the horizontal distance between the
        // glyphs is too large, or if they do not overlap vertically.
        if (startsNewWord) {
          createWord(currentWordGlyphs, &page->words);
          currentWordGlyphs.clear();
          currentWordGlyphs.push_back(glyph);
          currentWordMinX = glyph->minX;
          currentWordMinY = glyph->minY;
          currentWordMaxX = glyph->maxX;
          currentWordMaxY = glyph->maxY;
          maxWordFontSize = glyph->fontSize;
          prevGlyph = glyph;
          continue;
        }
      }

      currentWordGlyphs.push_back(glyph);
      currentWordMinX = std::min(currentWordMinX, glyph->minX);
      currentWordMinY = std::min(currentWordMinY, glyph->minY);
      currentWordMaxX = std::max(currentWordMaxX, glyph->maxX);
      currentWordMaxY = std::max(currentWordMaxY, glyph->maxY);
      maxWordFontSize = std::max(maxWordFontSize, glyph->fontSize);
      prevGlyph = glyph;
    }

    // Dont forget to create the last word of the page.
    createWord(currentWordGlyphs, &page->words);
  }
}



// _________________________________________________________________________________________________

void WordsTokenizer::createWord(const std::vector<PdfGlyph*>& glyphs, std::vector<PdfWord*>* words)
    const {
  PdfWord* word = new PdfWord();
  word->id = createRandomString(8, "w-");

  // Iterate through the glyphs to compute the text, they x,y-coordinates of the bounding box, and
  // font information.
  std::string text;
  std::unordered_map<std::string, int> fontNameFreqs;
  std::unordered_map<double, int> fontSizeFreqs;
  for (auto* glyph : glyphs) {
    // Update the x.y-coordinates.
    word->minX = std::min(word->minX, glyph->minX);
    word->minY = std::min(word->minY, glyph->minY);
    word->maxX = std::max(word->maxX, glyph->maxX);
    word->maxY = std::max(word->maxY, glyph->maxY);

    // Compose the text.
    if (glyph->isBaseGlyphOfDiacriticMark) {
      text += glyph->textWithDiacriticMark;
    } else if (!glyph->isDiacriticMarkOfBaseGlyph) {
      text += glyph->text;
    }

    // Count the font names and font sizes, for computing the most frequent font name / font size.
    fontNameFreqs[glyph->fontName]++;
    fontSizeFreqs[glyph->fontSize]++;

    word->glyphs.push_back(glyph);
  }

  // Set the text.
  word->text = text;

  // Compute and set the most frequent font name.
  int mostFreqFontNameCount = 0;
  for (const auto& pair : fontNameFreqs) {
    if (pair.second > mostFreqFontNameCount) {
      word->fontName = pair.first;
      mostFreqFontNameCount = pair.second;
    }
  }

  // Compute and set the most frequent font size.
  int mostFreqFontSizeCount = 0;
  for (const auto& pair : fontSizeFreqs) {
    if (pair.second > mostFreqFontSizeCount) {
      word->fontSize = pair.first;
      mostFreqFontSizeCount = pair.second;
    }
  }

  // Set the page number.
  word->pageNum = glyphs[0]->pageNum;

  // Set the writing mode.
  word->wMode = glyphs[0]->wMode;

  // Set the rotation value.
  word->rotation = glyphs[0]->rotation;

  // Set the rank.
  word->rank = glyphs[0]->rank;

  words->push_back(word);
}