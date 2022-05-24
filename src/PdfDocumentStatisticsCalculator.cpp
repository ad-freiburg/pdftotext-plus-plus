/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cmath>  // fabs
#include <iostream>  // std::cout

#include "./utils/Utils.h"

#include "./PdfDocument.h"
#include "./PdfDocumentStatisticsCalculator.h"

// _________________________________________________________________________________________________
PdfDocumentStatisticsCalculator::PdfDocumentStatisticsCalculator(PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
PdfDocumentStatisticsCalculator::~PdfDocumentStatisticsCalculator() = default;

// _________________________________________________________________________________________________
void PdfDocumentStatisticsCalculator::computeGlyphStatistics() const {
  // A mapping of font sizes to their frequencies, for computing the most frequent font size.
  std::unordered_map<double, int> fontSizeFreqs;
  // A mapping of font names to their frequencies, for computing the most frequent font name.
  std::unordered_map<std::string, int> fontNameFreqs;

  // The sum of glyph widths and heights, for computing the average glyph width/height.
  double sumWidths = 0;
  double sumHeights = 0;

  // The number of glyphs seen.
  int numGlyphs = 0;

  // Iterate through the glyphs of the document for computing the statistics about the glyphs.
  for (const auto* page : _doc->pages) {
    for (const auto* glyph : page->glyphs) {
      fontSizeFreqs[glyph->fontSize]++;
      fontNameFreqs[glyph->fontName]++;

      sumWidths += glyph->position->getWidth();
      sumHeights += glyph->position->getHeight();
      numGlyphs++;
    }
  }

  // Abort if no glyphs were seen.
  if (numGlyphs == 0) {
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
  std::string mostFreqFontName;
  int mostFreqFontNameCount = 0;
  for (const auto& pair : fontNameFreqs) {
    if (pair.second > mostFreqFontNameCount) {
      mostFreqFontName = pair.first;
      mostFreqFontNameCount = pair.second;
    }
  }
  _doc->mostFreqFontName = mostFreqFontName;

  // Compute the average glyph-width and -height.
  _doc->avgGlyphWidth = round(sumWidths / static_cast<double>(numGlyphs), 1);
  _doc->avgGlyphHeight = round(sumHeights / static_cast<double>(numGlyphs), 1);
}

// _________________________________________________________________________________________________
void PdfDocumentStatisticsCalculator::computeWordStatistics() const {
  std::unordered_map<double, int> xDistanceFreqs;
  // A mapping of vertical word distances, for computing the most frequent vertical word distance.
  // This is used to estimate the most frequent line distance, needed to define a minimum gap
  // height on page segmentation (text line detection comes after page segmentation, that's why
  // we estimate the the most frequent line distance based on words instead of computing it
  // exactly from text lines).
  std::unordered_map<double, int> yDistanceFreqs;
  // A mapping of word heights to their frequencies, for computing the most freq. word height.
  std::unordered_map<double, int> wordHeightFreqs;

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

        std::pair<double, double> yOverlapRatios = computeYOverlapRatios(prevWord, currWord);
        double maxYOverlap = std::max(yOverlapRatios.first, yOverlapRatios.second);

        // Count the horizontal distance.
        if (maxYOverlap > 0.5) {
          double xd = currWord->position->getRotLeftX() - prevWord->position->getRotRightX();
          xd = round(xd, 1);
          xDistanceFreqs[xd]++;
        }

        if (maxYOverlap == 0) {
          double yd = currWord->position->getRotUpperY() - prevWord->position->getRotLowerY();
          yd = round(yd, 1);
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
  // A mapping of line indentation frequencies, for computing the most freq. line indentation.
  std::unordered_map<double, int> lineIndentFreqs;
  // A mapping of line gaps to their frequencies, for computing the most freq. line gap.
  std::unordered_map<double, int> lineGapFreqs;

  for (const auto* page : _doc->pages) {
    for (const auto* segment : page->segments) {
      for (size_t i = 0; i < segment->lines.size(); i++) {
        PdfTextLine* prevLine = i > 0 ? segment->lines.at(i - 1) : nullptr;
        PdfTextLine* currLine = segment->lines.at(i);
        PdfTextLine* nextLine = i < segment->lines.size() - 1 ? segment->lines.at(i + 1) : nullptr;

        if (!prevLine || prevLine->position->wMode != 0 || prevLine->position->rotation != 0) {
          continue;
        }

        if (!currLine || currLine->position->wMode != 0 || currLine->position->rotation != 0) {
          continue;
        }

        if (!nextLine || nextLine->position->wMode != 0 || nextLine->position->rotation != 0) {
          continue;
        }

        if (prevLine->fontName != _doc->mostFreqFontName
            || prevLine->fontSize != _doc->mostFreqFontSize) {
          continue;
        }

        if (currLine->fontName != _doc->mostFreqFontName
            || currLine->fontSize != _doc->mostFreqFontSize) {
          continue;
        }

        if (nextLine->fontName != _doc->mostFreqFontName
            || nextLine->fontSize != _doc->mostFreqFontSize) {
          continue;
        }

        // Compute the vertical gap between the current line and the previous line.
        double gap = std::max(0.0, round(currLine->position->upperY - prevLine->position->lowerY, 1));
        lineGapFreqs[gap]++;

        // Check if the line is indented compared to the previous line and next line.
        double xOffsetCurrLine = currLine->position->leftX - prevLine->position->leftX;
        double xOffsetNextLine = nextLine->position->leftX - prevLine->position->leftX;

        if (xOffsetCurrLine < _doc->avgGlyphWidth) {
          continue;
        }

        if (fabs(xOffsetNextLine) > _doc->avgGlyphWidth) {
          continue;
        }

        double indent = static_cast<double>(static_cast<int>(xOffsetCurrLine * 10.)) / 10.;

        lineIndentFreqs[indent]++;
      }
    }
  }

  // Compute the most frequent line gap.
  double mostFreqLineGap = 0;
  int mostFreqLineGapCount = 0;
  for (const auto& pair : lineGapFreqs) {
    if (pair.second > mostFreqLineGapCount) {
      mostFreqLineGap = pair.first;
      mostFreqLineGapCount = pair.second;
    }
  }
  _doc->mostFreqLineGap = mostFreqLineGap;

  // // Compute the most frequent line indentation.
  // double mostFreqLineIndent = 0;
  // int mostFreqLineIndentCount = 0;
  // for (const auto& pair : lineIndentFreqs) {
  //   if (pair.second > mostFreqLineIndentCount) {
  //     mostFreqLineIndent = pair.first;
  //     mostFreqLineIndentCount = pair.second;
  //   }
  // }
  // _doc->mostFreqLineIndent = mostFreqLineIndent;
}
