/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // min, max
#include <utility>  // pair
#include <vector>

#include "../PdfDocument.h"

#include "./MathUtils.h"
#include "./PdfElementsUtils.h"

using element_utils::config::SENTENCE_DELIMITER_ALPHABET;
using std::make_pair;
using std::max;
using std::min;
using std::pair;
using std::vector;

// _________________________________________________________________________________________________
double element_utils::computeHorizontalGap(const PdfElement* element1, const PdfElement* element2) {
  assert(element1);
  assert(element2);

  // Determine the leftmost element, that is: the element with the minimum leftX.
  const PdfElement* leftElement;
  const PdfElement* rightElement;
  if (element1->position->leftX < element2->position->leftX) {
    leftElement = element1;
    rightElement = element2;
  } else {
    leftElement = element2;
    rightElement = element1;
  }

  // Compute the horizontal gap between the elements, under consideration of the rotation.
  switch (element1->position->rotation) {
    case 0:
    case 1:
    default:
      return rightElement->position->leftX - leftElement->position->rightX;
    case 2:
    case 3:
      return leftElement->position->rightX - rightElement->position->leftX;
  }
}

// _________________________________________________________________________________________________
double element_utils::computeVerticalGap(const PdfElement* element1, const PdfElement* element2) {
  assert(element1);
  assert(element2);

  // Determine the upper element, that is: the element with the minimum upperY.
  const PdfElement* upperElement;
  const PdfElement* lowerElement;
  if (element1->position->upperY < element2->position->upperY) {
    upperElement = element1;
    lowerElement = element2;
  } else {
    upperElement = element2;
    lowerElement = element1;
  }

  // Compute the vertical gap between the elements, under consideration of the rotation.
  switch (element1->position->rotation) {
    case 0:
    case 1:
    default:
      return lowerElement->position->upperY - upperElement->position->lowerY;
    case 2:
    case 3:
      return upperElement->position->lowerY - lowerElement->position->upperY;
  }
}

// _________________________________________________________________________________________________
pair<double, double> element_utils::computeOverlapRatios(double s1, double e1,
      double s2, double e2) {
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
pair<double, double> element_utils::computeXOverlapRatios(const PdfElement* element1,
      const PdfElement* element2) {
  assert(element1);
  assert(element2);

  double s1 = element1->position->leftX;
  double e1 = element1->position->rightX;
  double s2 = element2->position->leftX;
  double e2 = element2->position->rightX;
  return element_utils::computeOverlapRatios(s1, e1, s2, e2);
}

// _________________________________________________________________________________________________
pair<double, double> element_utils::computeYOverlapRatios(const PdfElement* element1,
      const PdfElement* element2) {
  assert(element1);
  assert(element2);

  double s1 = element1->position->upperY;
  double e1 = element1->position->lowerY;
  double s2 = element2->position->upperY;
  double e2 = element2->position->lowerY;
  return element_utils::computeOverlapRatios(s1, e1, s2, e2);
}

// _________________________________________________________________________________________________
double element_utils::computeMaxXOverlapRatio(const PdfElement* elem1, const PdfElement* elem2) {
  pair<double, double> ratios = element_utils::computeXOverlapRatios(elem1, elem2);
  return max(ratios.first, ratios.second);
}

// _________________________________________________________________________________________________
double element_utils::computeMaxYOverlapRatio(const PdfElement* elem1, const PdfElement* elem2) {
  pair<double, double> ratios = element_utils::computeYOverlapRatios(elem1, elem2);
  return max(ratios.first, ratios.second);
}

// _________________________________________________________________________________________________
bool element_utils::computeHasEqualLeftX(const PdfElement* elem1, const PdfElement* elem2,
      double tolerance) {
  assert(elem1);
  assert(elem2);
  return math_utils::equal(elem1->position->leftX, elem2->position->leftX, tolerance);
}

// _________________________________________________________________________________________________
bool element_utils::computeHasEqualUpperY(const PdfElement* elem1, const PdfElement* elem2,
      double tolerance) {
  assert(elem1);
  assert(elem2);
  return math_utils::equal(elem1->position->upperY, elem2->position->upperY, tolerance);
}

// _________________________________________________________________________________________________
bool element_utils::computeHasEqualRightX(const PdfElement* elem1, const PdfElement* elem2,
      double tolerance) {
  assert(elem1);
  assert(elem2);
  return math_utils::equal(elem1->position->rightX, elem2->position->rightX, tolerance);
}

// _________________________________________________________________________________________________
bool element_utils::computeHasEqualLowerY(const PdfElement* elem1, const PdfElement* elem2,
      double tolerance) {
  assert(elem1);
  assert(elem2);
  return math_utils::equal(elem1->position->lowerY, elem2->position->lowerY, tolerance);
}

// _________________________________________________________________________________________________
double element_utils::computeLeftXOffset(const PdfElement* elem1, const PdfElement* elem2) {
  assert(elem1);
  assert(elem2);
  return elem1->position->leftX - elem2->position->leftX;
}

// _________________________________________________________________________________________________
double element_utils::computeRightXOffset(const PdfElement* elem1, const PdfElement* elem2) {
  assert(elem1);
  assert(elem2);
  return elem1->position->rightX - elem2->position->rightX;
}

// _________________________________________________________________________________________________
PdfFigure* element_utils::computeOverlapsFigure(const PdfElement* element,
      const vector<PdfFigure*>& figures, double minXOverlapRatio, double minYOverlapRatio) {
  assert(element);

  for (auto* figure : figures) {
    pair<double, double> xOverlapRatios = element_utils::computeXOverlapRatios(element, figure);
    pair<double, double> yOverlapRatios = element_utils::computeYOverlapRatios(element, figure);

    // Check if the figure overlaps the element by the required overlap ratios.
    if (xOverlapRatios.first >= minXOverlapRatio && yOverlapRatios.first >= minYOverlapRatio) {
      return figure;
    }
  }

  return nullptr;
}

// =================================================================================================

// _________________________________________________________________________________________________
bool text_element_utils::computeHasEqualFont(const PdfTextElement* e1, const PdfTextElement* e2) {
  assert(e1);
  assert(e2);
  return e1->fontName == e2->fontName;
}

// _________________________________________________________________________________________________
bool text_element_utils::computeHasEqualFontSize(const PdfTextElement* e1,
      const PdfTextElement* e2, double equalTolerance) {
  assert(e1);
  assert(e2);
  return math_utils::equal(e1->fontSize, e2->fontSize, equalTolerance);
}

// _________________________________________________________________________________________________
bool text_element_utils::computeEndsWithSentenceDelimiter(const PdfTextElement* element) {
  assert(element);

  if (element->text.empty()) {
    return false;
  }

  return strchr(SENTENCE_DELIMITER_ALPHABET, element->text.back()) != nullptr;
}

// _________________________________________________________________________________________________
bool text_element_utils::computeStartsWithUpper(const PdfTextElement* element) {
  assert(element);

  if (element->text.empty()) {
    return false;
  }

  return isupper(element->text[0]);
}

// _________________________________________________________________________________________________
bool text_element_utils::computeIsEmphasized(const PdfTextElement* element,
      double fontSizeEqualTolerance, double fontWeightEqualTolerance) {
  assert(element);

  const PdfFontInfo* docFontInfo = element->doc->fontInfos.at(element->doc->mostFreqFontName);
  const PdfFontInfo* elemFontInfo = element->doc->fontInfos.at(element->fontName);
  double mostFreqFontSize = element->doc->mostFreqFontSize;

  // The element is emphasized if...

  // ... its font size is larger than the most frequent font size in the document.
  if (math_utils::larger(element->fontSize, mostFreqFontSize, fontSizeEqualTolerance)) {
    return true;
  }

  // ... its font weight is larger than the most frequent font weight (and its font size is not
  // smaller than the most frequent font size).
  if (math_utils::equalOrLarger(element->fontSize, mostFreqFontSize, fontSizeEqualTolerance)
      && math_utils::larger(elemFontInfo->weight, docFontInfo->weight, fontWeightEqualTolerance)) {
    return true;
  }

  // ... it is printed in italics (and its font size is not smaller than the most freq font size).
  if (math_utils::equalOrLarger(element->fontSize, mostFreqFontSize, fontSizeEqualTolerance)
      && elemFontInfo->isItalic) {
    return true;
  }

  // ... it contains at least one alphabetic character and all alphabetic characters are in
  // uppercase.
  bool containsAlpha = false;
  bool isAllAlphaUpper = true;
  for (char c : element->text) {
    if (isalpha(c)) {
      containsAlpha = true;
      if (islower(c)) {
        isAllAlphaUpper = false;
        break;
      }
    }
  }

  return containsAlpha && isAllAlphaUpper;
}
