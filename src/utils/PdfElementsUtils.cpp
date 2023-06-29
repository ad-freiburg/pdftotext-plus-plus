/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // std::min, std::max
#include <utility>  // std::pair
#include <vector>

#include "../PdfDocument.h"
#include "./Math.h"
#include "./PdfElementsUtils.h"

using std::make_pair;
using std::max;
using std::min;
using std::pair;
using std::vector;

using ppp::utils::math::equal;
using ppp::utils::math::equalOrLarger;
using ppp::utils::math::larger;
using ppp::utils::math::smaller;

namespace ppp::utils::elements {

// _________________________________________________________________________________________________
double computeHorizontalGap(const PdfElement* e1, const PdfElement* e2) {
  assert(e1);
  assert(e2);

  // Determine the leftmost element, that is: the element with the minimum leftX.
  const PdfElement* leftElement;
  const PdfElement* rightElement;
  if (smaller(e1->pos->leftX, e2->pos->leftX)) {
    leftElement = e1;
    rightElement = e2;
  } else {
    leftElement = e2;
    rightElement = e1;
  }

  // Compute the horizontal gap between the elements, under consideration of the rotation.
  switch (leftElement->pos->rotation) {
    case 0:
    case 1:
    default:
      return rightElement->pos->leftX - leftElement->pos->rightX;
    case 2:
    case 3:
      return leftElement->pos->rightX - rightElement->pos->leftX;
  }
}

// _________________________________________________________________________________________________
double computeVerticalGap(const PdfElement* e1, const PdfElement* e2) {
  assert(e1);
  assert(e2);

  // Determine the upper element, that is: the element with the minimum upperY.
  const PdfElement* upperElement;
  const PdfElement* lowerElement;
  if (smaller(e1->pos->upperY, e2->pos->upperY)) {
    upperElement = e1;
    lowerElement = e2;
  } else {
    upperElement = e2;
    lowerElement = e1;
  }

  // Compute the vertical gap between the elements, under consideration of the rotation.
  switch (upperElement->pos->rotation) {
    case 0:
    case 1:
    default:
      return lowerElement->pos->upperY - upperElement->pos->lowerY;
    case 2:
    case 3:
      return upperElement->pos->lowerY - lowerElement->pos->upperY;
  }
}

// _________________________________________________________________________________________________
pair<double, double> computeOverlapRatios(double s1, double e1, double s2, double e2) {
  // Compute the length of the first interval.
  double min1 = min(s1, e1);
  double max1 = max(s1, e1);
  double length1 = max1 - min1;

  // Compute the length of the second interval.
  double min2 = min(s2, e2);
  double max2 = max(s2, e2);
  double length2 = max2 - min2;

  // Compute the length of the overlap.
  double minMax = min(max1, max2);
  double maxMin = max(min1, min2);
  double overlapLength = max(0.0, minMax - maxMin);

  // Compute the overlap ratios.
  double overlapRatio1 = length1 > 0 ? overlapLength / length1 : 0;
  double overlapRatio2 = length2 > 0 ? overlapLength / length2 : 0;

  return make_pair(overlapRatio1, overlapRatio2);
}

// _________________________________________________________________________________________________
pair<double, double> computeXOverlapRatios(const PdfElement* element1, const PdfElement* element2) {
  assert(element1);
  assert(element2);

  double s1 = element1->pos->leftX;
  double e1 = element1->pos->rightX;
  double s2 = element2->pos->leftX;
  double e2 = element2->pos->rightX;

  return computeOverlapRatios(s1, e1, s2, e2);
}

// _________________________________________________________________________________________________
pair<double, double> computeYOverlapRatios(const PdfElement* element1, const PdfElement* element2) {
  assert(element1);
  assert(element2);

  double s1 = element1->pos->upperY;
  double e1 = element1->pos->lowerY;
  double s2 = element2->pos->upperY;
  double e2 = element2->pos->lowerY;

  return computeOverlapRatios(s1, e1, s2, e2);
}

// _________________________________________________________________________________________________
double computeMaxXOverlapRatio(const PdfElement* elem1, const PdfElement* elem2) {
  pair<double, double> ratios = computeXOverlapRatios(elem1, elem2);
  return max(ratios.first, ratios.second);
}

// _________________________________________________________________________________________________
double computeMaxYOverlapRatio(const PdfElement* elem1, const PdfElement* elem2) {
  pair<double, double> ratios = computeYOverlapRatios(elem1, elem2);
  return max(ratios.first, ratios.second);
}

// _________________________________________________________________________________________________
bool computeHasEqualLeftX(const PdfElement* elem1, const PdfElement* elem2, double tolerance) {
  assert(elem1);
  assert(elem2);

  return equal(elem1->pos->leftX, elem2->pos->leftX, tolerance);
}

// _________________________________________________________________________________________________
bool computeHasEqualUpperY(const PdfElement* elem1, const PdfElement* elem2, double tolerance) {
  assert(elem1);
  assert(elem2);

  return equal(elem1->pos->upperY, elem2->pos->upperY, tolerance);
}

// _________________________________________________________________________________________________
bool computeHasEqualRightX(const PdfElement* elem1, const PdfElement* elem2, double tolerance) {
  assert(elem1);
  assert(elem2);

  return equal(elem1->pos->rightX, elem2->pos->rightX, tolerance);
}

// _________________________________________________________________________________________________
bool computeHasEqualLowerY(const PdfElement* elem1, const PdfElement* elem2, double tolerance) {
  assert(elem1);
  assert(elem2);

  return equal(elem1->pos->lowerY, elem2->pos->lowerY, tolerance);
}

// _________________________________________________________________________________________________
double computeLeftXOffset(const PdfElement* elem1, const PdfElement* elem2) {
  assert(elem1);
  assert(elem2);

  return elem1->pos->leftX - elem2->pos->leftX;
}

// _________________________________________________________________________________________________
double computeRightXOffset(const PdfElement* elem1, const PdfElement* elem2) {
  assert(elem1);
  assert(elem2);

  return elem1->pos->rightX - elem2->pos->rightX;
}

// =================================================================================================

// _________________________________________________________________________________________________
bool computeHasEqualFont(const PdfTextElement* e1, const PdfTextElement* e2) {
  assert(e1);
  assert(e2);

  return e1->fontName == e2->fontName;
}

// _________________________________________________________________________________________________
bool computeHasEqualFontSize(const PdfTextElement* e1, const PdfTextElement* e2, double tolerance) {
  assert(e1);
  assert(e2);

  return equal(e1->fontSize, e2->fontSize, tolerance);
}

// _________________________________________________________________________________________________
bool computeStartsWithUpper(const PdfTextElement* element) {
  assert(element);

  if (element->text.empty()) {
    return false;
  }

  return isupper(element->text[0]);
}

}  // namespace ppp::utils::elements
