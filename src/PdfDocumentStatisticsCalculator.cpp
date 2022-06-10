/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cmath>  // fabs
#include <iostream>  // cout

#include "./utils/MathUtils.h"
#include "./utils/PdfElementsUtils.h"

#include "./PdfDocument.h"
#include "./PdfDocumentStatisticsCalculator.h"

using namespace std;

// _________________________________________________________________________________________________
PdfDocumentStatisticsCalculator::PdfDocumentStatisticsCalculator(PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
PdfDocumentStatisticsCalculator::~PdfDocumentStatisticsCalculator() = default;

// _________________________________________________________________________________________________
void PdfDocumentStatisticsCalculator::computeCharStatistics() const {
  // A mapping of font sizes to their frequencies, for computing the most frequent font size.
  unordered_map<double, int> fontSizeFreqs;
  // A mapping of font names to their frequencies, for computing the most frequent font name.
  unordered_map<string, int> fontNameFreqs;

  // The sum of char widths and heights, for computing the average char width/height.
  double sumWidths = 0;
  double sumHeights = 0;

  // The number of characters seen.
  int numChars = 0;

  // Iterate through the characters of the document for computing the characters statistics.
  for (const auto* page : _doc->pages) {
    for (const auto* ch : page->characters) {
      fontSizeFreqs[ch->fontSize]++;
      fontNameFreqs[ch->fontName]++;

      sumWidths += ch->position->getWidth();
      sumHeights += ch->position->getHeight();
      numChars++;
    }
  }

  // Abort if no characters were seen.
  if (numChars == 0) {
    return;
  }

  // Compute the most frequent font size.
  double mostFreqFontSize = 0;
  int mostFreqFontSizeCount = 0;
  for (const auto& pair : fontSizeFreqs) {
    if (pair.second > mostFreqFontSizeCount) {
      mostFreqFontSize = pair.first;
      mostFreqFontSizeCount = pair.second;
    }
  }
  _doc->mostFreqFontSize = mostFreqFontSize;

  // Compute the most frequent font name.
  string mostFreqFontName;
  int mostFreqFontNameCount = 0;
  for (const auto& pair : fontNameFreqs) {
    if (pair.second > mostFreqFontNameCount) {
      mostFreqFontName = pair.first;
      mostFreqFontNameCount = pair.second;
    }
  }
  _doc->mostFreqFontName = mostFreqFontName;

  // Compute the average character width and -height.
  _doc->avgCharWidth = math_utils::round(sumWidths / static_cast<double>(numChars), 1);
  _doc->avgCharHeight = math_utils::round(sumHeights / static_cast<double>(numChars), 1);
}

// _________________________________________________________________________________________________
void PdfDocumentStatisticsCalculator::computeWordStatistics() const {
  unordered_map<double, int> xDistanceFreqs;
  // A mapping of vertical word distances, for computing the most frequent vertical word distance.
  // This is used to estimate the most frequent line distance, needed to define a minimum gap
  // height on page segmentation (text line detection comes after page segmentation, that's why
  // we estimate the the most frequent line distance based on words instead of computing it
  // exactly from text lines).
  unordered_map<double, int> yDistanceFreqs;
  // A mapping of word heights to their frequencies, for computing the most freq. word height.
  unordered_map<double, int> wordHeightFreqs;

  // For computing the most frequent line distance, identify line breaks between two words by
  // iterating through the words of the document in extraction order and inspecting (previous word,
  // current word) pairs. When both words do not overlap vertically, assume a line break and add
  // the vertical distance between the base lines of both words to `lineDistanceFreqs`.
  for (const auto* page : _doc->pages) {
    for (size_t i = 0; i < page->words.size(); i++) {
      PdfWord* prevWord = i > 0 ? page->words[i - 1] : nullptr;
      PdfWord* currWord = page->words[i];

      if (prevWord) {
        // Ignore all word pairs that have a different font size than the most frequent font size.
        if ((fabs(prevWord->fontSize - _doc->mostFreqFontSize) > 0.01) ||
            (fabs(currWord->fontSize - _doc->mostFreqFontSize) > 0.01)) {
          continue;
        }

        pair<double, double> yOverlapRatios = element_utils::computeYOverlapRatios(prevWord, currWord);
        double maxYOverlap = max(yOverlapRatios.first, yOverlapRatios.second);

        // Count the horizontal distance.
        if (maxYOverlap > 0.5) {
          double xd = currWord->position->getRotLeftX() - prevWord->position->getRotRightX();
          xd = math_utils::round(xd, 1);
          xDistanceFreqs[xd]++;
        }

        if (maxYOverlap == 0) {
          double yd = currWord->position->getRotUpperY() - prevWord->position->getRotLowerY();
          yd = math_utils::round(yd, 1);
          yDistanceFreqs[yd]++;
        }
      }

      // Count the word heights, for computing the most frequent word height.
      if (currWord->position->getHeight() >= 1) {
        wordHeightFreqs[currWord->position->getHeight()]++;
      }
    }
  }

  // Compute the most frequent word height.
  double mostFreqWordHeight = 0;
  int mostFreqWordHeightCount = 0;
  for (const auto& pair : wordHeightFreqs) {
    if (pair.second > mostFreqWordHeightCount) {
      mostFreqWordHeight = pair.first;
      mostFreqWordHeightCount = pair.second;
    }
  }
  _doc->mostFreqWordHeight = mostFreqWordHeight;

  // Compute the most frequent horizontal distance.
  double mostFreqXDistance = 0;
  int mostFreqXDistanceCount = 0;
  for (const auto& pair : xDistanceFreqs) {
    if (pair.second > mostFreqXDistanceCount) {
      mostFreqXDistance = pair.first;
      mostFreqXDistanceCount = pair.second;
    }
  }
  _doc->mostFreqWordDistance = mostFreqXDistance;

  // Compute the most frequent vertical distance.
  double mostFreqYDistance = 0;
  int mostFreqYDistanceCount = 0;
  for (const auto& pair : yDistanceFreqs) {
    if (pair.second > mostFreqYDistanceCount) {
      mostFreqYDistance = pair.first;
      mostFreqYDistanceCount = pair.second;
    }
  }
  _doc->mostFreqEstimatedLineDistance = mostFreqYDistance;
}

// _________________________________________________________________________________________________
void PdfDocumentStatisticsCalculator::computeLineStatistics() const {
  // A mapping of line distances to their frequencies, for computing the most freq. line distance.
  unordered_map<double, int> lineDistanceFreqs;
  unordered_map<double, unordered_map<double, int>> lineDistanceFreqsPerFontSize;

  // Iterate through the text lines and consider (prev. line, curr line) pairs.
  // Compute the vertical distance between both lines and count the distances.
  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      for (size_t i = 1; i < segment->lines.size(); i++) {
        PdfTextLine* prevLine = segment->lines.at(i - 1);
        PdfTextLine* currLine = segment->lines.at(i);

        if (!prevLine || !currLine) {
          continue;
        }

        // Ignore the lines if they are positioned on different pages.
        if (prevLine->position->pageNum != currLine->position->pageNum) {
          continue;
        }

        // Ignore the lines if their writing mode differ.
        if (prevLine->position->wMode != 0 || currLine->position->wMode != 0) {
          continue;
        }

        // Ignore the lines if their rotation differ.
        if (prevLine->position->rotation != 0 || currLine->position->rotation != 0) {
          continue;
        }

        // Compute the line distance and count it.
        // TODO: Explain why baseBBox is used here.
        double lineDistance = math_utils::round(currLine->baseBBoxUpperY - prevLine->baseBBoxLowerY, 1);
        lineDistance = max(0.0, lineDistance);
        lineDistanceFreqs[lineDistance]++;

        // For computing line distances per font size, ignore the lines if their font sizes differ.
        double prevFontSize = math_utils::round(prevLine->fontSize, 1);
        double currFontSize = math_utils::round(currLine->fontSize, 1);
        if (math_utils::equal(prevFontSize, currFontSize, 0.01)) {
          lineDistanceFreqsPerFontSize[currFontSize][lineDistance]++;
        }
      }
    }
  }

  // Compute the most frequent line distance.
  int mostFreqLineDistanceFreq = 0;
  for (const auto& pair : lineDistanceFreqs) {
    if (pair.second > mostFreqLineDistanceFreq) {
      _doc->mostFreqLineDistance = pair.first;
      mostFreqLineDistanceFreq = pair.second;
    }
  }

  // Compute the most frequent line distances per font size.
  unordered_map<double, int> mostFreqLineDistanceCountPerFontSize;
  for (const auto& doubleMapPair : lineDistanceFreqsPerFontSize) {
    const double fontSize = doubleMapPair.first;
    const unordered_map<double, int>& lineDistanceFreqs = doubleMapPair.second;
    for (const auto& doubleIntPair : lineDistanceFreqs) {
      double lineDistance = doubleIntPair.first;
      double count = doubleIntPair.second;
      int mostFreqCount = 0;
      if (mostFreqLineDistanceCountPerFontSize.count(fontSize) > 0) {
        mostFreqCount = mostFreqLineDistanceCountPerFontSize.at(fontSize);
      }
      if (count > mostFreqCount) {
        _doc->mostFreqLineDistancePerFontSize[fontSize] = lineDistance;
        mostFreqLineDistanceCountPerFontSize[fontSize] = count;
      }
    }
  }
}
