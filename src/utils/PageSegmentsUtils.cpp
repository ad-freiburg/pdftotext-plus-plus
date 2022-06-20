/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <tuple>
#include <unordered_map>

#include "./Counter.h"
#include "./MathUtils.h"
#include "./PageSegmentsUtils.h"

using page_segment_utils::config::COORDS_PREC;
using page_segment_utils::config::MIN_PERCENTAGE_SAME_RIGHT_X;

using std::make_tuple;
using std::pair;
using std::tuple;
using std::unordered_map;

// _________________________________________________________________________________________________
tuple<double, double, double, double> page_segment_utils::computeTrimBox(
      const PdfPageSegment* segment) {
  assert(segment);

  // Initialize the coordinates of the trim box with the respective coordinates of the bounding box.
  double trimLeftX = segment->position->leftX;
  double trimUpperY = segment->position->upperY;
  double trimRightX = segment->position->rightX;
  double trimLowerY = segment->position->lowerY;

  // Compute the most frequent rightX among the text lines.
  DoubleCounter rightXCounter;
  for (auto* line : segment->lines) {
    double rightX = math_utils::round(line->position->getRotRightX(), COORDS_PREC);
    rightXCounter[rightX]++;
  }
  pair<double, double> mostFreqRightXPair = rightXCounter.mostFreqAndCount();
  double mostFreqRightX = mostFreqRightXPair.first;
  int mostFreqRightXCount = mostFreqRightXPair.second;

  // Compute the percentage of lines exhibiting the most frequent rightX.
  double numLines = segment->lines.size();
  double mostFreqRightXRatio = numLines > 0.0 ? mostFreqRightXCount / numLines : 0.0;

  // If at least half of the lines exhibit the most frequent rightX, set trimRightX to this value.
  trimRightX = mostFreqRightXRatio >= MIN_PERCENTAGE_SAME_RIGHT_X ? mostFreqRightX : trimRightX;

  return make_tuple(trimLeftX, trimUpperY, trimRightX, trimLowerY);
}
