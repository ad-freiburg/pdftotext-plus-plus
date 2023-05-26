/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // std::max
#include <tuple>
#include <unordered_map>
#include <utility>  // std::pair
#include <vector>

#include "./Counter.h"
#include "./MathUtils.h"
#include "./PageSegmentsUtils.h"
#include "./StringUtils.h"

using global_config::ID_LENGTH;

using std::make_tuple;
using std::max;
using std::min;
using std::pair;
using std::tuple;
using std::unordered_map;

// _________________________________________________________________________________________________
PdfPageSegment* page_segment_utils::createPageSegment(const vector<PdfElement*>& elements,
    const PdfDocument* doc) {
  PdfPageSegment* segment = new PdfPageSegment();
  segment->doc = doc;

  // Create a (unique) id.
  segment->id = ppp::string_utils::createRandomString(ID_LENGTH, "segment-");

  // Set the page number.
  segment->pos->pageNum = !elements.empty() ? elements[0]->pos->pageNum : -1;

  // Compute and set the coordinates of the bounding box.
  for (const auto* element : elements) {
    segment->pos->leftX = min(segment->pos->leftX, element->pos->leftX);
    segment->pos->upperY = min(segment->pos->upperY, element->pos->upperY);
    segment->pos->rightX = max(segment->pos->rightX, element->pos->rightX);
    segment->pos->lowerY = max(segment->pos->lowerY, element->pos->lowerY);
  }

  // Set the elements.
  segment->elements = elements;

  return segment;
}

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
    double rightX = ppp::math_utils::round(line->pos->getRotRightX(), config::TRIM_BOX_COORDS_PREC);
    rightXCounter[rightX]++;
  }
  pair<double, double> mostFreqRightXPair = rightXCounter.mostFreqAndCount();
  double mostFreqRightX = mostFreqRightXPair.first;
  int mostFreqRightXCount = mostFreqRightXPair.second;

  // Compute the percentage of lines exhibiting the most frequent rightX.
  size_t nLines = segment->lines.size();
  double mostFreqRightXRatio = nLines > 0 ? mostFreqRightXCount / static_cast<double>(nLines) : 0.0;

  // If the percentage is larger or equal to the given threshold, set trimRightX to this value.
  if (ppp::math_utils::equalOrLarger(mostFreqRightXRatio, config::MIN_PERC_LINES_SAME_RIGHT_X)) {
    trimRightX = mostFreqRightX;
  }

  return make_tuple(trimLeftX, trimUpperY, trimRightX, trimLowerY);
}
