/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <iostream>  // std::endl
#include <unordered_map>

#include "./PdfDocument.h"
#include "./TextLinesStatisticsCalculation.h"
#include "./utils/Counter.h"
#include "./utils/Log.h"
#include "./utils/MathUtils.h"

using std::endl;
using std::unordered_map;

using ppp::config::TextLinesStatisticsCalculationConfig;
using ppp::types::PdfDocument;
using ppp::types::PdfTextLine;
using ppp::utils::counter::DoubleCounter;
using ppp::utils::log::BOLD;
using ppp::utils::log::OFF;
using ppp::utils::log::Logger;
using ppp::utils::math::equal;
using ppp::utils::math::maximum;
using ppp::utils::math::round;
using ppp::utils::math::smaller;

// =================================================================================================

namespace ppp::modules {

// _________________________________________________________________________________________________
TextLinesStatisticsCalculation::TextLinesStatisticsCalculation(
    PdfDocument* doc,
    const TextLinesStatisticsCalculationConfig* config) {
  _doc = doc;
  _config = config;
  _log = new Logger(config->logLevel, config->logPageFilter);
}

// _________________________________________________________________________________________________
TextLinesStatisticsCalculation::~TextLinesStatisticsCalculation() {
  delete _log;
}

// _________________________________________________________________________________________________
void TextLinesStatisticsCalculation::process() const {
  assert(_doc);

  _log->info() << "Calculating text line statistics..." << endl;
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

        // Calculate the line distance between the lines by comparing their *base bounding boxes*
        // (= the bounding box around the characters that are not a subscript or superscript).
        // The motivation behind using the base bounding box instead of the normal bounding box is
        // that the vertical gap between two text lines is usually smaller than it actually is,
        // when one or both lines contain sub- or superscripts. By our experience, calculating the
        // line distance with sub- and superscripts ignored results in more accurate line distances.
        double dist = currLine->baseBBoxUpperY - prevLine->baseBBoxLowerY;
        dist = maximum(0.0, round(dist, _config->lineDistancePrecision));
        lineDistanceCounter[dist]++;

        // If the font sizes of the text lines are equal, add the distance also to
        // lineDistanceCountersPerFontSize, for calculating the most frequent line distances broken
        // down by font size.
        if (equal(prevLine->fontSize, currLine->fontSize, _config->fsEqualTolerance)) {
          lineDistanceCountersPerFontSize[currLine->fontSize][dist]++;
        }
      }
    }
  }

  // Calculate the most frequent line distance.
  if (lineDistanceCounter.sumCounts() > 0) {
    _doc->mostFreqLineDistance = lineDistanceCounter.mostFreq();
  }

  // Calculate the most frequent line distances broken down by font sizes.
  unordered_map<double, int> mostFreqLineDistanceCountPerFontSize;
  for (const auto& doubleMapPair : lineDistanceCountersPerFontSize) {
    const double fontSize = doubleMapPair.first;
    const DoubleCounter& lineDistanceCounter = doubleMapPair.second;

    if (lineDistanceCounter.sumCounts() > 0) {
      _doc->mostFreqLineDistancePerFontSize[fontSize] = lineDistanceCounter.mostFreq();
    }
  }

  _log->debug() << "doc.mostFreqLineDist: " << _doc->mostFreqEstimatedLineDistance << endl;
  for (auto& x : _doc->mostFreqLineDistancePerFontSize) {
    _log->debug() << "doc.mostFreqLineDistPerFontsize[" << x.first << "]: " << x.second << endl;
  }
  _log->debug() << "=======================================" << endl;
}

}  // namespace ppp::modules
