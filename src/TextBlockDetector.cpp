/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cmath>
#include <iostream>
#include <vector>

#include "./PdfDocument.h"
#include "./TextBlockDetector.h"
#include "./utils/Utils.h"

// _________________________________________________________________________________________________
TextBlockDetector::TextBlockDetector(PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
TextBlockDetector::~TextBlockDetector() = default;

// _________________________________________________________________________________________________
void TextBlockDetector::detect() {
  // Do nothing if no document is given.
  if (!_doc) {
    return;
  }

  // Do nothing if no pages are given.
  if (_doc->pages.size() == 0) {
    return;
  }

  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      std::vector<PdfTextLine*> currentTextBlockLines;
      for (size_t i = 0; i < segment->lines.size(); i++) {
        PdfTextLine* prevTextLine = i > 0 ? segment->lines.at(i - 1) : nullptr;
        PdfTextLine* currTextLine = segment->lines.at(i);
        PdfTextLine* nextTextLine = i < segment->lines.size() - 1
            ? segment->lines.at(i + 1) : nullptr;

        // The line introduces a new text block if ...
        bool introducesNewTextBlock = false;

        if (prevTextLine) {
          // The line distance is larger than the most freq. line distance.
          double distance = std::max(0.0, currTextLine->maxY - prevTextLine->maxY);
          distance = static_cast<double>(static_cast<int>(distance * 10.)) / 10.;
          if (distance > 1.1 * _doc->mostFreqLineDistance) {
            introducesNewTextBlock = true;
          }

          // ... its font size is different to the fontsize of the previous line.
          if (fabs(currTextLine->fontSize - prevTextLine->fontSize) > 0.5) {
            introducesNewTextBlock = true;
          }

          // ... its font weight is larger than the most frequent font weight.
          double prevFontWeight = _doc->fontInfos[prevTextLine->fontName]->weight;
          double currFontWeight = _doc->fontInfos[currTextLine->fontName]->weight;
          if (equalOrLarger(currTextLine->fontSize, prevTextLine->fontSize)
              && currFontWeight - prevFontWeight > 100) {
            introducesNewTextBlock = true;
          }
          // if (equalOrLarger(prevTextLine->fontSize, currTextLine->fontSize)
          //      && prevFontWeight -currFontWeight > 100) {
          //   introducesNewTextBlock = true;
          // }


          if (nextTextLine) {
            // Line is intended?
            double xOffsetCurrLine = currTextLine->minX - prevTextLine->minX;
            double xOffsetNextLine = nextTextLine->minX - prevTextLine->minX;
            if (equal(xOffsetCurrLine, _doc->mostFreqLineIndent) && equal(xOffsetNextLine, 0)) {
              introducesNewTextBlock = true;
            }
          }
        }

        if (introducesNewTextBlock) {
          createTextBlock(currentTextBlockLines, &page->blocks);
          currentTextBlockLines.clear();
        }
        currentTextBlockLines.push_back(currTextLine);
      }
      // Dont forget to create the last block of the segment.
      createTextBlock(currentTextBlockLines, &page->blocks);
    }
  }
}

// _________________________________________________________________________________________________
void TextBlockDetector::createTextBlock(const std::vector<PdfTextLine*>& lines,
    std::vector<PdfTextBlock*>* blocks) {
  // Do nothing if no words are given.
  if (lines.size() == 0) {
    return;
  }

  PdfTextBlock* block = new PdfTextBlock();
  block->id = createRandomString(8, "tb-");

  std::unordered_map<std::string, int> fontNameFreqs;
  std::unordered_map<double, int> fontSizeFreqs;
  for (const auto* line : lines) {
    // Update the x,y-coordinates.
    block->minX = std::min(block->minX, line->minX);
    block->minY = std::min(block->minY, line->minY);
    block->maxX = std::max(block->maxX, line->maxX);
    block->maxY = std::max(block->maxY, line->maxY);

    // Count the font names and font sizes, for computing the most frequent font name / font size.
    fontNameFreqs[line->fontName]++;
    fontSizeFreqs[line->fontSize]++;
  }

  // Compute and set the most frequent font name.
  int mostFreqFontNameCount = 0;
  for (const auto& pair : fontNameFreqs) {
    if (pair.second > mostFreqFontNameCount) {
      block->fontName = pair.first;
      mostFreqFontNameCount = pair.second;
    }
  }

  // Compute and set the most frequent font size.
  int mostFreqFontSizeCount = 0;
  for (const auto& pair : fontSizeFreqs) {
    if (pair.second > mostFreqFontSizeCount) {
      block->fontSize = pair.first;
      mostFreqFontSizeCount = pair.second;
    }
  }

  // Set the page number.
  block->pageNum = lines[0]->pageNum;

  // Set the writing mode.
  block->wMode = lines[0]->wMode;

  // Set the rotation value.
  block->rotation = lines[0]->rotation;

  // Set the text.
  for (size_t i = 0; i < lines.size(); i++) {
    const auto* line = lines.at(i);
    for (size_t j = 0; j < line->words.size(); j++) {
      auto* word = line->words.at(j);
      block->text += word->text;
      if (j < line->words.size() - 1) {
        block->text += " ";
      }
    }
    if (i < lines.size() - 1) {
      block->text += " ";
    }
  }

  block->isEmphasized = computeIsTextBlockEmphasized(lines);

  block->lines = lines;

  blocks->push_back(block);
}

// =================================================================================================

// _________________________________________________________________________________________________
bool TextBlockDetector::computeIsTextBlockEmphasized(const std::vector<PdfTextLine*>& lines) {
  const PdfFontInfo* docFontInfo = _doc->fontInfos.at(_doc->mostFreqFontName);

  for (size_t i = 0; i < lines.size(); i++) {
    PdfTextLine* line = lines[i];
    const PdfFontInfo* lineFontInfo = _doc->fontInfos.at(line->fontName);

    // The line is emphasized if ...
    bool isLineEmphasized = false;

    // ... its font size is significantly larger than the most frequent font size in the document.
    if (line->fontSize - _doc->mostFreqFontSize > 0.5) {
      isLineEmphasized = true;
    }

    // ... its font weight is larger than the most frequent font weight.
    if (line->fontSize - _doc->mostFreqFontSize >= -1
        && lineFontInfo->weight > docFontInfo->weight) {
      isLineEmphasized = true;
    }

    // ... the line is printed in italics.
    if (line->fontSize - _doc->mostFreqFontSize >= -1 && lineFontInfo->isItalic) {
      isLineEmphasized = true;
    }

    // ... the line contains at least one alphabetic characters and all alphabetic characters are
    // upper case.
    bool containsAlpha = false;
    bool isAllAlphaUpper = true;
    for (size_t j = 0; j < line->text.size(); j++) {
      if (isalpha(line->text[j])) {
        containsAlpha = true;
        if (islower(line->text[j])) {
          isAllAlphaUpper = false;
          break;
        }
      }
    }
    if (containsAlpha && isAllAlphaUpper) {
      isLineEmphasized = true;
    }

    if (!isLineEmphasized) {
      return false;
    }
  }

  return true;
}

// =================================================================================================



// // ______________________________________________________________________________________________
// std::string PageSegmentator::computeText(const PdfTextBlock* block) const {
//   // Sort the words according to the natural reading order (top to bottom, left to right).
//   std::vector<TeiWord*> sortedWords = block->words;
//   std::sort(sortedWords.begin(), sortedWords.end(), [](const TeiWord* w1, const TeiWord* w2) {
//     // Compute the vertical overlap of the two words.
//     double minMaxY = std::min(w1->maxY, w2->maxY);
//     double maxMinY = std::max(w1->minY, w2->minY);
//     double yOverlap = std::max(.0, minMaxY - maxMinY);

//     // Compute the overlap ratio for both words.
//     double w1Height = w1->maxY - w1->minY;
//     double w2Height = w2->maxY - w2->minY;
//     double yOverlapRatioW1 = w1Height > 0 ? yOverlap / w1Height : 0;
//     double yOverlapRatioW2 = w2Height > 0 ? yOverlap / w2Height : 0;

//     // If the words overlap by more than half of the height of one of the words (= if they share
//     // the same text line), sort the words by x-coordinates.
//     if (yOverlapRatioW1 > 0.5 || yOverlapRatioW2 > 0.5) {
//       return w1->minX < w2->minX;
//     }

//     // If the words do not share the same text line, sort them by y-coordinates.
//     return w1->minY < w2->minY;
//   });

//   // Iterate through the words in sorted order and join them, each separated by a whitespace.
//   std::string text;
//   for (size_t i = 0; i < sortedWords.size(); i++) {
//     TeiWord* word = sortedWords[i];
//     text += word->text;
//     if (i < sortedWords.size() - 1) {
//       text += " ";
//     }
//   }

//   return text;
// }
