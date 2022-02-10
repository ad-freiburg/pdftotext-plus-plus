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
    currentWordGlyphs.push_back(page->glyphs[0]);

    // Consider (previous glyph, current glyph) pairs. For each pair, analyze the spacing, font
    // size and writing mode of both glyphs and decide whether or not there is a word boundary
    // between the two glyphs.
    for (size_t i = 1; i < page->glyphs.size(); i++) {
      PdfGlyph* prevGlyph = page->glyphs.at(i - 1);
      PdfGlyph* currGlyph = page->glyphs.at(i);

      // Ignore diacritic marks, as they were already merged with their base characters.
      if (currGlyph->isDiacriticMarkOfBaseGlyph) {
        continue;
      }

      // Assume a word boundary between the two glyphs when the writing modes and/or rotations of
      // both glyphs differ.
      if (prevGlyph->wMode != currGlyph->wMode || prevGlyph->rotation != currGlyph->rotation) {
        createWord(currentWordGlyphs, &page->words);
        currentWordGlyphs.clear();
        currentWordGlyphs.push_back(currGlyph);
        continue;
      }

      double hDistance = 0;
      double vOverlap = 0;
      switch (currGlyph->rotation) {
        case 0:
          {
            hDistance = fabs(currGlyph->minX - prevGlyph->maxX);
            double minMaxY = std::min(prevGlyph->maxY, currGlyph->maxY);
            double maxMinY = std::max(prevGlyph->minY, currGlyph->minY);
            vOverlap = std::max(.0, minMaxY - maxMinY);
            break;
          }
        case 1:
          {
            hDistance = fabs(currGlyph->minY - prevGlyph->maxY);
            double minMaxX = std::min(prevGlyph->maxX, currGlyph->maxX);
            double maxMinX = std::max(prevGlyph->minX, currGlyph->minX);
            vOverlap = std::max(.0, minMaxX - maxMinX);
            break;
          }
        case 2:
          {
            hDistance = fabs(prevGlyph->minX - currGlyph->maxX);
            double minMaxY = std::min(prevGlyph->maxY, currGlyph->maxY);
            double maxMinY = std::max(prevGlyph->minY, currGlyph->minY);
            vOverlap = std::max(.0, minMaxY - maxMinY);
            break;
          }
        case 3:
          {
            hDistance = fabs(prevGlyph->minY - currGlyph->maxY);
            double minMaxX = std::min(prevGlyph->maxX, currGlyph->maxX);
            double maxMinX = std::max(prevGlyph->minX, currGlyph->minX);
            vOverlap = std::max(.0, minMaxX - maxMinX);
            break;
          }
      }

      // Assume a word boundary between the two glyphs if the horizontal distance between the
      // glpyh is too large, or if they do not overlap vertically.
      if (hDistance > minWordBreakSpace * prevGlyph->fontSize
            || vOverlap < 0.1 * prevGlyph->fontSize) {
        createWord(currentWordGlyphs, &page->words);
        currentWordGlyphs.clear();
        currentWordGlyphs.push_back(currGlyph);
        continue;
      }

      currentWordGlyphs.push_back(currGlyph);
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