/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cmath>

#include "./MathUtils.h"
#include "./PageSegmentUtils.h"

using namespace std;

// _________________________________________________________________________________________________
tuple<double, double, double, double> page_segment_utils::computeTrimBox(
      const PdfPageSegment* segment) {
  double trimLeftX = segment->position->leftX;
  double trimUpperY = segment->position->upperY;
  double trimRightX = segment->position->rightX;
  double trimLowerY = segment->position->lowerY;

  // Compute the most frequent rightX coordinates among the text lines in the segment.
  unordered_map<double, int> rightXFreqs;
  for (auto* line : segment->lines) {
    double rightX = round(line->position->getRotRightX());
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

  double numLines = segment->lines.size();
  double mostFreqRightXRatio = numLines > 0.0 ? mostFreqRightXCount / numLines : 0.0;

  if (mostFreqRightXRatio >= 0.5) {
    trimRightX = mostFreqRightX;
  }

  return make_tuple(trimLeftX, trimUpperY, trimRightX, trimLowerY);
}