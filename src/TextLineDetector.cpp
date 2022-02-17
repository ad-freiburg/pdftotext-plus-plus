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
#include "./TextLineDetector.h"
#include "./utils/Utils.h"

// _________________________________________________________________________________________________
TextLineDetector::TextLineDetector(PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
TextLineDetector::~TextLineDetector() = default;

// _________________________________________________________________________________________________
void TextLineDetector::detect() {
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
      // Filter the elements of the segment by words.
      std::vector<PdfWord*> words;
      for (auto* element : segment->elements) {
        PdfWord* word = dynamic_cast<PdfWord*>(element);
        if (word) {
          words.push_back(word);
        }
      }

      // Do nothing if the segment does not contain any words.
      if (words.size() == 0) {
        continue;
      }

      std::sort(words.begin(), words.end(), [](const PdfWord* w1, const PdfWord* w2) {
        // Compute the vertical overlap of the two words.
        double minMaxY = std::min(w1->maxY, w2->maxY);
        double maxMinY = std::max(w1->minY, w2->minY);
        double yOverlap = std::max(.0, minMaxY - maxMinY);

        // Compute the overlap ratio for both words.
        double w1Height = w1->maxY - w1->minY;
        double w2Height = w2->maxY - w2->minY;
        double yOverlapRatioW1 = w1Height > 0 ? yOverlap / w1Height : 0;
        double yOverlapRatioW2 = w2Height > 0 ? yOverlap / w2Height : 0;

        // If the words overlap by more than half of the height of one of the words (= if they
        // share the same text line), sort the words by x-coordinates.
        if (yOverlapRatioW1 > 0.5 || yOverlapRatioW2 > 0.5) {
          return w1->minX < w2->minX;
        }

        // If the words do not share the same text line, sort them by y-coordinates.
        return w1->minY < w2->minY;


        // return w1->rank < w2->rank;
      });

      std::vector<PdfWord*> currTextLineWords;
      currTextLineWords.push_back(words[0]);
      double currTextLineMinX = words[0]->minX;
      double currTextLineMinY = words[0]->minY;
      double currTextLineMaxX = words[0]->maxX;
      double currTextLineMaxY = words[0]->maxY;

      for (size_t i = 1; i < words.size(); i++) {
        // PdfWord* prevWord = words.at(i - 1);
        PdfWord* currWord = words.at(i);

        // double maxMinX = std::max(currWord->minX, prevWord->minX);
        // double minMaxX = std::min(currWord->maxX, prevWord->maxX);
        // double maxMinY = std::max(currWord->minY, currTextLineMinY);
        // double minMaxY = std::min(currWord->maxY, currTextLineMaxY);
        // double xOverlap = std::max(.0, minMaxX - maxMinX);
        // double yOverlap = std::max(.0, minMaxY - maxMinY);
        // double xOverlapRatio = xOverlap / prevWord->getWidth();
        // double yOverlapRatio = yOverlap / fabs(currTextLineMaxY - currTextLineMinY);;

        double textLineOverlap = 0;
        switch (currWord->rotation) {
          case 0:
          case 2:
            {
              double minMaxY = std::min(currWord->maxY, currTextLineMaxY);
              double maxMinY = std::max(currWord->minY, currTextLineMinY);
              double overlap = std::max(.0, minMaxY - maxMinY);
              double currTextLineHeight = fabs(currTextLineMaxY - currTextLineMinY);
              textLineOverlap = currTextLineHeight > 0 ? overlap / currTextLineHeight : 0;
            }
            break;
          case 1:
          case 3:
            {
              double minMaxX = std::min(currWord->maxX, currTextLineMaxX);
              double maxMinX = std::max(currWord->minX, currTextLineMinX);
              double overlap = std::max(.0, minMaxX - maxMinX);
              double currTextLineHeight = fabs(currTextLineMaxX - currTextLineMinX);
              textLineOverlap = currTextLineHeight > 0 ? overlap / currTextLineHeight : 0;
            }
            break;
        }

        if (textLineOverlap < 0.25) {
          createTextLine(currTextLineWords, &segment->lines);
          currTextLineWords.clear();
          currTextLineMinX = std::numeric_limits<double>::max();
          currTextLineMinY = std::numeric_limits<double>::max();
          currTextLineMaxX = std::numeric_limits<double>::min();
          currTextLineMaxY = std::numeric_limits<double>::min();
        }
        currTextLineWords.push_back(currWord);
        currTextLineMinX = std::min(currWord->minX, currTextLineMinX);
        currTextLineMinY = std::min(currWord->minY, currTextLineMinY);
        currTextLineMaxX = std::max(currWord->maxX, currTextLineMaxX);
        currTextLineMaxY = std::max(currWord->maxY, currTextLineMaxY);
      }

      // Dont forget to create the last line of the page.
      createTextLine(currTextLineWords, &segment->lines);
    }
  }
}

// _________________________________________________________________________________________________
void TextLineDetector::createTextLine(const std::vector<PdfWord*>& words, 
    std::vector<PdfTextLine*>* lines) {
  // Do nothing if no words are given.
  if (words.size() == 0) {
    return;
  }

  // Create a new text line and compute its layout information from the words.
  PdfTextLine* line = new PdfTextLine();
  line->id = createRandomString(8, "tl-");

  // Iterate through the words to compute they x,y-coordinates of the bounding box, and
  // font information.
  std::unordered_map<std::string, int> fontNameFreqs;
  std::unordered_map<double, int> fontSizeFreqs;
  std::unordered_map<double, int> baseFreqs;
  for (const auto* word : words) {
    // Update the x.y-coordinates.
    line->minX = std::min(line->minX, word->minX);
    line->minY = std::min(line->minY, word->minY);
    line->maxX = std::max(line->maxX, word->maxX);
    line->maxY = std::max(line->maxY, word->maxY);

    // Compute the most frequent font name, font size and base among the glyphs.
    for (const auto* glyph : word->glyphs) {
      fontNameFreqs[glyph->fontName]++;
      fontSizeFreqs[glyph->fontSize]++;
      baseFreqs[glyph->base]++;
    }
  }

  // Compute and set the most frequent font name.
  int mostFreqFontNameCount = 0;
  for (const auto& pair : fontNameFreqs) {
    if (pair.second > mostFreqFontNameCount) {
      line->fontName = pair.first;
      mostFreqFontNameCount = pair.second;
    }
  }

  // Compute and set the most frequent font size.
  int mostFreqFontSizeCount = 0;
  for (const auto& pair : fontSizeFreqs) {
    if (pair.second > mostFreqFontSizeCount) {
      line->fontSize = pair.first;
      mostFreqFontSizeCount = pair.second;
    }
  }

  // Compute and set the most baseline.
  int mostFreqBaseCount = 0;
  for (const auto& pair : baseFreqs) {
    if (pair.second > mostFreqBaseCount) {
      line->base = pair.first;
      mostFreqBaseCount = pair.second;
    }
  }

  // Set the page number.
  line->pageNum = words[0]->pageNum;

  // Set the writing mode.
  line->wMode = words[0]->wMode;

  // Set the rotation value.
  line->rotation = words[0]->rotation;

  // Set the glyphs.
  line->words = words;

  // Iterate through the words and join them, each separated by a whitespace.
  for (size_t i = 0; i < words.size(); i++) {
    PdfWord* word = words[i];
    line->text += word->text;
    if (i < words.size() - 1) {
      line->text += " ";
    }
  }
  
  lines->push_back(line);
}
 
// =================================================================================================



// // _____________________________________________________________________________________________
// std::string PageSegmentator::computeText(const TeiTextBlock* block) const {
//   // Sort the words according to the natural reading order (top to bottom, left to right).
//   std::vector<PdfWord*> sortedWords = block->words;
//   std::sort(sortedWords.begin(), sortedWords.end(), [](const PdfWord* w1, const PdfWord* w2) {
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
//     PdfWord* word = sortedWords[i];
//     text += word->text;
//     if (i < sortedWords.size() - 1) {
//       text += " ";
//     }
//   }

//   return text;
// }
