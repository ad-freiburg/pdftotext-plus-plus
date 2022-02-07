/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cmath>  // fabs
#include <iostream>  // std::cout

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
  // A mapping of font sizes to their frequencies, for computing the most common font size.
  std::unordered_map<double, int> fontSizeFreqs;
  // A mapping of font names to their frequencies, for computing the most common font name.
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

      sumWidths += glyph->getWidth();
      sumHeights += glyph->getHeight();
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
  _doc->avgGlyphWidth = sumWidths / static_cast<double>(numGlyphs);
  _doc->avgGlyphHeight = sumHeights / static_cast<double>(numGlyphs);
}

// _________________________________________________________________________________________________
void PdfDocumentStatisticsCalculator::computeWordStatistics() const {
  // A mapping of line distances to their frequencies, for computing the most freq. line distance.
  std::unordered_map<double, int> lineDistanceFreqs;
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
        // Compute the vertical overlap between `prevWord` and `currWord`.
        double minMaxY = std::min(prevWord->maxY, currWord->maxY);
        double maxMinY = std::max(prevWord->minY, currWord->minY);
        double verticalOverlap = std::max(0.0, minMaxY - maxMinY);

        // Ignore all word pairs that overlap vertically (that are in the same text line). In some
        // PDFs, two words can slightly overlap even when they are from different text lines, so
        // allow a small tolerance.
        double minHeight = std::min(prevWord->getHeight(), currWord->getHeight());
        if (verticalOverlap > 0.2 * minHeight) {
          continue;
        }

        // Ignore all word pairs that have a different font size than the most frequent font size.
        if ((fabs(prevWord->fontSize - _doc->mostFreqFontSize) > 0.01) ||
            (fabs(currWord->fontSize - _doc->mostFreqFontSize) > 0.01)) {
          continue;
        }

        // Compute the vertical distance between the base lines of the two words with a precision
        // of one decimal. Add the distance to `lineDistanceFreqs`. Note that the y-coordinates
        // of the base lines of the words are given by the maxX values, since the origin of the
        // coordinate system is the upper left of the page.
        double distance = std::max(0.0, currWord->maxY - prevWord->maxY);
        distance = static_cast<double>(static_cast<int>(distance * 10.)) / 10.;

        lineDistanceFreqs[distance]++;
      }

      // Count the word heights, for computing the most frequent word height.
      if (currWord->getHeight() >= 1) {
        wordHeightFreqs[currWord->getHeight()]++;
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

  // Compute the most frequent line distance.
  double mostFreqLineDistance = 0;
  int mostFreqLineDistanceCount = 0;
  for (const auto& pair : lineDistanceFreqs) {
    if (pair.second > mostFreqLineDistanceCount) {
      mostFreqLineDistance = pair.first;
      mostFreqLineDistanceCount = pair.second;
    }
  }
  _doc->mostFreqLineDistance = mostFreqLineDistance;
}

// _________________________________________________________________________________________________
void PdfDocumentStatisticsCalculator::computeLineStatistics() const {
  // A mapping of line indentation frequencies, for computing the most freq. line indentation.
  std::unordered_map<double, int> lineIndentFreqs;

  for (const auto* page : _doc->pages) {
    for (const auto* segment : page->segments) {
      for (size_t i = 0; i < segment->lines.size(); i++) {
        PdfTextLine* prevLine = i > 0 ? segment->lines.at(i - 1) : nullptr;
        PdfTextLine* currLine = segment->lines.at(i);
        PdfTextLine* nextLine = i < segment->lines.size() - 1 ? segment->lines.at(i + 1) : nullptr;

        if (!prevLine || prevLine->wMode != 0 || prevLine->rotation != 0) {
          continue;
        }

        if (!currLine || currLine->wMode != 0 || currLine->rotation != 0) {
          continue;
        }

        if (!nextLine || nextLine->wMode != 0 || nextLine->rotation != 0) {
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

        // Check if the line is indented compared to the previous line and next line.
        double xOffsetCurrLine = currLine->minX - prevLine->minX;
        double xOffsetNextLine = nextLine->minX - prevLine->minX;

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

  // Compute the most frequent line indentation.
  double mostFreqLineIndent = 0;
  int mostFreqLineIndentCount = 0;
  for (const auto& pair : lineIndentFreqs) {
    if (pair.second > mostFreqLineIndentCount) {
      mostFreqLineIndent = pair.first;
      mostFreqLineIndentCount = pair.second;
    }
  }
  _doc->mostFreqLineIndent = mostFreqLineIndent;
}
