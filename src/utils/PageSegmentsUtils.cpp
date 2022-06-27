/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <tuple>
#include <unordered_map>
#include <utility>  // std::pair

#include "./Counter.h"
#include "./MathUtils.h"
#include "./PageSegmentsUtils.h"

using std::make_tuple;
using std::pair;
using std::tuple;
using std::unordered_map;

// _________________________________________________________________________________________________
tuple<double, double, double, double> page_segment_utils::computeTrimBox(
    const PdfPageSegment* segment) {
  assert(segment);

  // Initialize the coordinates of the trim box with the respective coordinates of the bounding box.
  double trimLeftX = segment->pos->leftX;
  double trimUpperY = segment->pos->upperY;
  double trimRightX = segment->pos->rightX;
  double trimLowerY = segment->pos->lowerY;

  // Compute the most frequent rightX among the text lines.
  DoubleCounter rightXCounter;
  for (auto* line : segment->lines) {
    double rightX = math_utils::round(line->pos->getRotRightX(), config::TRIM_BOX_COORDS_PREC);
    rightXCounter[rightX]++;
  }
  pair<double, double> mostFreqRightXPair = rightXCounter.mostFreqAndCount();
  double mostFreqRightX = mostFreqRightXPair.first;
  int mostFreqRightXCount = mostFreqRightXPair.second;

  // Compute the percentage of lines exhibiting the most frequent rightX.
  size_t nLines = segment->lines.size();
  double mostFreqRightXRatio = nLines > 0 ? mostFreqRightXCount / static_cast<double>(nLines) : 0.0;

  // If the percentage is larger or equal to the given threshold, set trimRightX to this value.
  if (math_utils::equalOrLarger(mostFreqRightXRatio, config::MIN_PERC_LINES_SAME_RIGHT_X)) {
    trimRightX = mostFreqRightX;
  }

  return make_tuple(trimLeftX, trimUpperY, trimRightX, trimLowerY);
}
