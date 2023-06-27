/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // max
#include <unordered_map>

#include "./utils/Counter.h"
#include "./utils/Log.h"
#include "./utils/MathUtils.h"
#include "./utils/PdfElementsUtils.h"
#include "./PdfDocument.h"
#include "./StatisticsCalculation.h"

using std::endl;
using std::max;
using std::unordered_map;

using ppp::config::StatisticsCalculationConfig;
using ppp::utils::elements::computeHorizontalGap;
using ppp::utils::elements::computeMaxYOverlapRatio;
using ppp::utils::elements::computeVerticalGap;
using ppp::utils::math::equal;
using ppp::utils::math::equalOrLarger;
using ppp::utils::math::equalOrSmaller;
using ppp::utils::math::round;
using ppp::utils::math::smaller;

namespace ppp {

// _________________________________________________________________________________________________
StatisticsCalculation::StatisticsCalculation(
    PdfDocument* doc,
    const StatisticsCalculationConfig& config) {
  _doc = doc;
  _config = config;
  _log = new Logger(config.logLevel, config.logPageFilter);
}

// _________________________________________________________________________________________________
StatisticsCalculation::~StatisticsCalculation() {
  delete _log;
}

// _________________________________________________________________________________________________
void StatisticsCalculation::computeGlyphStatistics() const {
  assert(_doc);

  _log->info() << "Computing glyph statististics..." << endl;
  _log->debug() << "=======================================" << endl;
  _log->debug() << BOLD << "DEBUG MODE" << OFF << endl;
  _log->debug() << "=======================================" << endl;

  // A counter for the font sizes of the glyphs.
  DoubleCounter fontSizeCounter;
  // A counter for the font names of the glyphs.
  StringCounter fontNameCounter;

  // The sum of the char widths and -heights, for computing the average char width/-height.
  double sumWidths = 0;
  double sumHeights = 0;

  // The number of glyphs in the document.
  int numGlyphs = 0;

  for (const auto* page : _doc->pages) {
    for (const auto* character : page->characters) {
      fontSizeCounter[character->fontSize]++;
      fontNameCounter[character->fontName]++;
      sumWidths += character->pos->getWidth();
      sumHeights += character->pos->getHeight();
      numGlyphs++;
    }
  }

  // Abort if the document contains no glyphs.
  if (numGlyphs == 0) {
    return;
  }

  // Compute the most frequent font size and font name.
  _doc->mostFreqFontSize = fontSizeCounter.mostFreq();
  _doc->mostFreqFontName = fontNameCounter.mostFreq();

  _log->debug() << "doc.mostFreqFontSize: " << _doc->mostFreqFontSize << endl;
  _log->debug() << "doc.mostFreqFontName: " << _doc->mostFreqFontName << endl;

  // Compute the average glyph width and -height.
  _doc->avgCharWidth = sumWidths / static_cast<double>(numGlyphs);
  _doc->avgCharHeight = sumHeights / static_cast<double>(numGlyphs);

  _log->debug() << "doc.avgCharWidth:  " << _doc->avgCharWidth << endl;
  _log->debug() << "doc.avgCharHeight: " << _doc->avgCharHeight << endl;
  _log->debug() << "=======================================" << endl;
}

// _________________________________________________________________________________________________
void StatisticsCalculation::computeWordStatistics() const {
  assert(_doc);

  double minYOverlapRatioSameLine = _config.minYOverlapRatioSameLine;
  double maxYOverlapRatioDiffLine = _config.maxYOverlapRatioDifferentLine;

  _log->info() << "Computing word statististics..." << endl;
  _log->debug() << "=======================================" << endl;
  _log->debug() << BOLD << "DEBUG MODE" << OFF << endl;
  _log->debug() << " └─ minYOverlapRatioSameLine: " << minYOverlapRatioSameLine << endl;
  _log->debug() << " └─ maxYOverlapRatioDifferentLine: " << maxYOverlapRatioDiffLine << endl;
  _log->debug() << "=======================================" << endl;

  // A counter for the horizontal gaps between two consecutive words that overlap vertically.
  DoubleCounter horizontalGapCounter;
  // A counter for the vertical gaps between two consecutive words that don't overlap vertically.
  DoubleCounter verticalGapCounter;
  // A counter for the word heights.
  DoubleCounter wordHeightCounter;

  for (const auto* page : _doc->pages) {
    for (size_t i = 0; i < page->words.size(); i++) {
      PdfWord* prevWord = i > 0 ? page->words[i - 1] : nullptr;
      PdfWord* word = page->words[i];

      // Skip the word if its font size is smaller than the most frequent font size.
      if (smaller(word->fontSize, _doc->mostFreqFontSize, _config.fsEqualTolerance)) {
        continue;
      }

      // Count the word height.
      // TODO(korzen): The height should not be rounded here, but on creating the word. Remove
      // _coordinatePrecision if not necessary anymore.
      double height = round(word->pos->getHeight(), _config.coordinatePrecision);
      wordHeightCounter[height]++;

      // Skip to the next word if there is no previous word.
      if (!prevWord) {
        continue;
      }

      // Skip to the next word if the word does not have the same rotation than the previous word.
      if (prevWord->pos->rotation != word->pos->rotation) {
        continue;
      }

      // Skip to the next word if the word does not have the same writing mode than the prev word.
      if (prevWord->pos->wMode != word->pos->wMode) {
        continue;
      }

      // Skip to the next word if the font size of the previous word is not equal to the most
      // frequent font size.
      if (!equal(prevWord->fontSize, _doc->mostFreqFontSize, _config.fsEqualTolerance)) {
        continue;
      }

      double maxYOverlapRatio = computeMaxYOverlapRatio(prevWord, word);

      // Add the horizontal gap between the previous word and the current word to the counter,
      // when one word vertically overlaps at least the half of the height of the other word.
      if (equalOrLarger(maxYOverlapRatio, minYOverlapRatioSameLine)) {
        double gap = computeHorizontalGap(prevWord, word);
        gap = round(gap, _config.coordinatePrecision);
        horizontalGapCounter[gap]++;
      }

      // Add the vertical gap between the previous word and the current word to the counter, when
      // they do *not* vertically overlap.
      if (equalOrSmaller(maxYOverlapRatio, maxYOverlapRatioDiffLine)) {
        double gap = computeVerticalGap(prevWord, word);
        gap = round(gap, _config.coordinatePrecision);
        verticalGapCounter[gap]++;
      }
    }
  }

  _doc->mostFreqWordHeight = wordHeightCounter.mostFreq();
  _doc->mostFreqWordDistance = horizontalGapCounter.mostFreq();
  _doc->mostFreqEstimatedLineDistance = verticalGapCounter.mostFreq();

  _log->debug() << "doc.mostFreqWordHeight: " << _doc->mostFreqWordHeight << endl;
  _log->debug() << "doc.mostFreqWordDistance: " << _doc->mostFreqWordDistance << endl;
  _log->debug() << "doc.mostFreqEstimatedLineDist: " << _doc->mostFreqEstimatedLineDistance << endl;
  _log->debug() << "=======================================" << endl;
}

// _________________________________________________________________________________________________
void StatisticsCalculation::computeTextLineStatistics() const {
  assert(_doc);

  _log->info() << "Computing text line statististics..." << endl;
  _log->debug() << "=======================================" << endl;
  _log->debug() << BOLD << "DEBUG MODE" << OFF << endl;
  _log->debug() << "=======================================" << endl;

  // A counter for the line distances between two consecutive lines.
  DoubleCounter lineDistanceCounter;
  // The counters for the line distances between two consecutive lines, broken down by font sizes.
  // The counter at lineDistanceCountersPerFontSize[x] is the counter for the line distances
  // between two consecutive lines with font size x.
  unordered_map<double, DoubleCounter> lineDistanceCountersPerFontSize;

  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      for (size_t i = 1; i < segment->lines.size(); i++) {
        PdfTextLine* prevLine = segment->lines.at(i - 1);
        PdfTextLine* currLine = segment->lines.at(i);

        // Skip to the next line if the line does not have the same rotation than the previous line.
        if (prevLine->pos->rotation != currLine->pos->rotation) {
          continue;
        }

        // Skip to the next line if the line does not have the same writing mode than the prev line.
        if (prevLine->pos->wMode != currLine->pos->wMode) {
          continue;
        }

        // Compute the line distance between the lines by comparing their *base bounding boxes*
        // (= the bounding box around the characters that are not a subscript or superscript).
        // The motivation behind using the base bounding box instead of the normal bounding box is
        // that the vertical gap between two text lines is usually smaller than it actually is,
        // when one or both lines contain sub- or superscripts. By our experience, computing the
        // line distance with sub- and superscripts ignored results in more accurate line distances.
        double dist = currLine->baseBBoxUpperY - prevLine->baseBBoxLowerY;
        dist = max(0.0, round(dist, _config.lineDistancePrecision));
        lineDistanceCounter[dist]++;

        // If the font sizes of the text lines are equal, add the distance also to
        // lineDistanceCountersPerFontSize, for computing the most frequent line distances broken
        // down by font size.
        if (equal(prevLine->fontSize, currLine->fontSize, _config.fsEqualTolerance)) {
          lineDistanceCountersPerFontSize[currLine->fontSize][dist]++;
        }
      }
    }
  }

  // Compute the most frequent line distance.
  _doc->mostFreqLineDistance = lineDistanceCounter.mostFreq();

  // Compute the most frequent line distances broken down by font sizes.
  unordered_map<double, int> mostFreqLineDistanceCountPerFontSize;
  for (const auto& doubleMapPair : lineDistanceCountersPerFontSize) {
    const double fontSize = doubleMapPair.first;
    const DoubleCounter& lineDistanceCounter = doubleMapPair.second;

    _doc->mostFreqLineDistancePerFontSize[fontSize] = lineDistanceCounter.mostFreq();
  }

  _log->debug() << "doc.mostFreqLineDist: " << _doc->mostFreqEstimatedLineDistance << endl;
  for (auto& x : _doc->mostFreqLineDistancePerFontSize) {
    _log->debug() << "doc.mostFreqLineDistPerFontsize[" << x.first << "]: " << x.second << endl;
  }
  _log->debug() << "=======================================" << endl;
}

}  // namespace ppp
