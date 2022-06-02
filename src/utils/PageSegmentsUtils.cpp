/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cmath>  // round
#include <tuple>
#include <unordered_map>

#include "./MathUtils.h"
#include "./PageSegmentsUtils.h"

using std::make_tuple;
using std::tuple;
using std::unordered_map;

// _________________________________________________________________________________________________
tuple<double, double, double, double> page_segment_utils::computeTrimBox(
      const PdfPageSegment* segment) {
  assert(segment);

  // Initialize the coordinates of the trim box with the respective values of the bounding box.
  double trimLeftX = segment->position->leftX;
  double trimUpperY = segment->position->upperY;
  double trimRightX = segment->position->rightX;
  double trimLowerY = segment->position->lowerY;

  // Compute the most frequent rightX value among the text lines.
  unordered_map<double, int> rightXFreqs;
  for (auto* line : segment->lines) {
    double rightX = math_utils::round(line->position->getRotRightX());
    rightXFreqs[rightX]++;
  }
  double mostFreqRightX = 0;
  int mostFreqRightXCount = 0;
  for (const auto& pair : rightXFreqs) {
    if (pair.second > mostFreqRightXCount) {
      mostFreqRightX = pair.first;
      mostFreqRightXCount = pair.second;
    }
  }

  // Compute the percentage of lines exhibiting the most frequent rightX value.
  double numLines = segment->lines.size();
  double mostFreqRightXRatio = numLines > 0.0 ? mostFreqRightXCount / numLines : 0.0;

  // If at least half of the lines exhibit the most frequent rightX value, change the trimRightX
  // to this value.
  trimRightX = mostFreqRightXRatio >= 0.5 ? mostFreqRightX : trimRightX;

  return make_tuple(trimLeftX, trimUpperY, trimRightX, trimLowerY);
}
