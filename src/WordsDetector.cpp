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

#include "./WordsDetector.h"
#include "./PdfDocument.h"

#include "./utils/Utils.h"

// The inter-glyph space width which will cause the tokenize() method to start a new word.
#define minWordBreakSpace 0.15


// _________________________________________________________________________________________________
WordsDetector::WordsDetector(PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
WordsDetector::~WordsDetector() = default;

// _________________________________________________________________________________________________
void WordsDetector::detect() const {
  tokenize();
  mergeStackedWords();
}

// _________________________________________________________________________________________________
void WordsDetector::tokenize() const {
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
    double currWordMinX = std::numeric_limits<double>::max();
    double currWordMinY = std::numeric_limits<double>::max();
    double currWordMaxX = std::numeric_limits<double>::min();
    double currWordMaxY = std::numeric_limits<double>::min();
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

      double glyphMinX = std::min(glyph->position->getRotLeftX(), glyph->position->getRotRightX());
      double glyphMinY = std::min(glyph->position->getRotLowerY(), glyph->position->getRotUpperY());
      double glyphMaxX = std::max(glyph->position->getRotLeftX(), glyph->position->getRotRightX());
      double glyphMaxY = std::max(glyph->position->getRotLowerY(), glyph->position->getRotUpperY());

      if (prevGlyph) {
        bool startsNewWord = false;

        // Assume a word boundary between the two glyphs when the writing modes and/or rotations of
        // both glyphs differ.
        if (prevGlyph->position->wMode != glyph->position->wMode) {
          startsNewWord = true;
        }

        if (prevGlyph->position->rotation != glyph->position->rotation) {
          startsNewWord = true;
        }

        double minMaxY = std::min(currWordMaxY, glyphMaxY);
        double maxMinY = std::max(currWordMinY, glyphMinY);
        double vOverlap = std::max(0.0, minMaxY - maxMinY);
        double glyphHeight = glyphMaxY - glyphMinY;
        double wordHeight = currWordMaxY - currWordMinY;
        double glyphOverlapRatio = glyphHeight > 0 ? vOverlap / glyphHeight : 0;
        double wordOverlapRatio = wordHeight > 0 ? vOverlap / wordHeight : 0;
        if (glyphOverlapRatio < 0.5 && wordOverlapRatio < 0.5) {
          startsNewWord = true;
        }

        double hDistanceLeft = 0.0;
        double hDistanceRight = 0.0;
        switch(glyph->position->rotation) {
          case 0:
          case 1:
          default:
            hDistanceLeft = glyphMinX - currWordMaxX;
            hDistanceRight = currWordMinX - glyphMaxX;
            break;
          case 2:
          case 3:
            hDistanceLeft = currWordMinX - glyphMaxX;
            hDistanceRight = glyphMinX - currWordMaxX;
            break;
        }
        if (hDistanceLeft > minWordBreakSpace * maxWordFontSize) {
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
          currWordMinX = glyphMinX;
          currWordMinY = glyphMinY;
          currWordMaxX = glyphMaxX;
          currWordMaxY = glyphMaxY;
          maxWordFontSize = glyph->fontSize;
          prevGlyph = glyph;
          continue;
        }
      }

      currentWordGlyphs.push_back(glyph);

      currWordMinX = std::min(currWordMinX, glyphMinX);
      currWordMinY = std::min(currWordMinY, glyphMinY);
      currWordMaxX = std::max(currWordMaxX, glyphMaxX);
      currWordMaxY = std::max(currWordMaxY, glyphMaxY);
      maxWordFontSize = std::max(maxWordFontSize, glyph->fontSize);
      prevGlyph = glyph;
    }

    // Dont forget to create the last word of the page.
    createWord(currentWordGlyphs, &page->words);
  }
}



// _________________________________________________________________________________________________

void WordsDetector::createWord(const std::vector<PdfGlyph*>& glyphs, std::vector<PdfWord*>* words)
    const {
  PdfWord* word = new PdfWord();
  word->id = createRandomString(8, "w-");

  // Iterate through the glyphs to compute the text, they x,y-coordinates of the bounding box, and
  // font information.
  std::string text;
  std::unordered_map<std::string, int> fontNameFreqs;
  std::unordered_map<double, int> fontSizeFreqs;
  for (auto* glyph : glyphs) {
    double glyphMinX = std::min(glyph->position->leftX, glyph->position->rightX);
    double glyphMinY = std::min(glyph->position->upperY, glyph->position->lowerY);
    double glyphMaxX = std::max(glyph->position->leftX, glyph->position->rightX);
    double glyphMaxY = std::max(glyph->position->upperY, glyph->position->lowerY);

    // Update the x.y-coordinates.
    word->position->leftX = std::min(word->position->leftX, glyphMinX);
    word->position->upperY = std::min(word->position->upperY, glyphMinY);
    word->position->rightX = std::max(word->position->rightX, glyphMaxX);
    word->position->lowerY = std::max(word->position->lowerY, glyphMaxY);

    // Compose the text.
    if (glyph->isBaseGlyphOfDiacriticMark) {
      text += glyph->textWithDiacriticMark;
    } else if (!glyph->isDiacriticMarkOfBaseGlyph) {
      text += glyph->text;
    }

    // Count the font names and font sizes, for computing the most frequent font name / font size.
    fontNameFreqs[glyph->fontName]++;
    fontSizeFreqs[glyph->fontSize]++;

    glyph->word = word;
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
  word->position->pageNum = glyphs[0]->position->pageNum;

  // Set the writing mode.
  word->position->wMode = glyphs[0]->position->wMode;

  // Set the rotation value.
  word->position->rotation = glyphs[0]->position->rotation;

  // Set the rank.
  word->rank = words->size();

  words->push_back(word);
}

// _________________________________________________________________________________________________
void WordsDetector::mergeStackedWords() const {
  for (auto* page : _doc->pages) {
    for (size_t i = 0; i < page->words.size(); i++) {
      PdfWord* word = page->words.at(i);

      // Check if the word contains a symbol that could be a base of a stacked word.
      bool containsBaseOfStackedWord = false;
      for (auto* glyph : word->glyphs) {
        if (glyph->text == "∑" || glyph->text == "∏" || glyph->text == "∫" || glyph->text == "⊗") {
          containsBaseOfStackedWord = true;
          break;
        }
        if (glyph->charName == "summationDisplay" || glyph->charName == "productdisplay"
            || glyph->charName == "integraldisplay" || glyph->charName == "circlemultiplydisplay") {
          containsBaseOfStackedWord = true;
          break;
        }
      }

      if (word->text == "sup" || word->text == "lim") {
        containsBaseOfStackedWord = true;
      }

      if (containsBaseOfStackedWord) {
        for (size_t j = i; j --> 0 ;) {
          auto* otherWord = page->words.at(j);
          double xOverlapRatio = computeMaximumXOverlapRatio(word, otherWord);
          bool isSmallerFontSize = smaller(otherWord->fontSize, word->fontSize, 1);
          if (xOverlapRatio > 0.5 && isSmallerFontSize) {
            word->isBaseOfStackedWords.push_back(otherWord);
            page->words.at(j)->isPartOfStackedWord = word;
            continue;
          }
          break;
        }

        for (size_t j = i + 1; j < page->words.size(); j++) {
          auto* otherWord = page->words.at(j);
          double xOverlapRatio = computeMaximumXOverlapRatio(word, otherWord);
          bool isSmallerFontSize = smaller(otherWord->fontSize, word->fontSize, 1);
          if (xOverlapRatio > 0.5 && isSmallerFontSize) {
            word->isBaseOfStackedWords.push_back(otherWord);
            page->words.at(j)->isPartOfStackedWord = word;
            continue;
          }
          break;
        }
      }
    }
  }
}