/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cmath>  // min, max
#include <vector>

#include "../PdfDocument.h"
#include "./MathUtils.h"
#include "./PdfElementUtils.h"
#include "./Utils.h"

// _________________________________________________________________________________________________
std::pair<double, double> element_utils::computeOverlapRatios(double s1, double e1,
      double s2, double e2) {
  // Compute the length of the first interval.
  double min1 = std::min(s1, e1);
  double max1 = std::max(s1, e1);
  double length1 = max1 - min1;

  // Compute the length of the second interval.
  double min2 = std::min(s2, e2);
  double max2 = std::max(s2, e2);
  double length2 = max2 - min2;

  // Compute the length of the overlap.
  double minMax = std::min(max1, max2);
  double maxMin = std::max(min1, min2);
  double overlapLength = std::max(0.0, minMax - maxMin);

  // Compute the overlap ratios.
  double overlapRatio1 = length1 > 0 ? overlapLength / length1 : 0;
  double overlapRatio2 = length2 > 0 ? overlapLength / length2 : 0;

  return std::make_pair(overlapRatio1, overlapRatio2);
}

// _________________________________________________________________________________________________
std::pair<double, double> element_utils::computeXOverlapRatios(const PdfElement* element1,
      const PdfElement* element2) {
  assert(element1);
  assert(element2);

  double s1 = element1->position->rightX;
  double e1 = element1->position->leftX;
  double s2 = element2->position->rightX;
  double e2 = element2->position->leftX;
  return element_utils::computeOverlapRatios(s1, e1, s2, e2);
}

// _________________________________________________________________________________________________
double element_utils::computeMaxXOverlapRatio(const PdfElement* elem1, const PdfElement* elem2) {
  pair<double, double> ratios = element_utils::computeXOverlapRatios(elem1, elem2);
  return max(ratios.first, ratios.second);
}

// _________________________________________________________________________________________________
std::pair<double, double> element_utils::computeYOverlapRatios(const PdfElement* element1,
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
PdfFigure* element_utils::computeOverlapsFigure(const PdfElement* elem,
      const std::vector<PdfFigure*>& figures, double minXOverlapRatio, double minYOverlapRatio) {

  for (auto* figure : figures) {
    std::pair<double, double> xOverlapRatios = element_utils::computeXOverlapRatios(elem, figure);
    std::pair<double, double> yOverlapRatios = element_utils::computeYOverlapRatios(elem, figure);

    // Check if the figure fulfills the required minimum overlap ratios.
    if (xOverlapRatios.first > minXOverlapRatio && yOverlapRatios.first > minYOverlapRatio) {
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
bool text_element_utils::computeHasEqualFontSize(const PdfTextElement* e1, const PdfTextElement* e2,
      double tolerance) {
  assert(e1);
  assert(e2);
  return math_utils::equal(e1->fontSize, e2->fontSize, tolerance);
}

// _________________________________________________________________________________________________
bool text_element_utils::computeEndsWithSentenceDelimiter(const PdfTextElement* elem) {
  assert(elem);

  if (elem->text.empty()) {
    return false;
  }

  return SENTENCE_DELIMITER_ALPHABET.find(elem->text.back()) != std::string::npos;
}

// _________________________________________________________________________________________________
bool text_element_utils::computeStartsWithUpper(const PdfTextElement* elem) {
  assert(elem);

  if (elem->text.empty()) {
    return false;
  }

  return isupper(elem->text[0]);
}

// _________________________________________________________________________________________________
bool text_element_utils::computeIsEmphasized(const PdfTextElement* element) {
  assert(element);

  const PdfFontInfo* docFontInfo = element->doc->fontInfos.at(element->doc->mostFreqFontName);
  const PdfFontInfo* elementFontInfo = element->doc->fontInfos.at(element->fontName);

  // The element is emphasized if ...

  double mostFreqFontSize = element->doc->mostFreqFontSize;

  // ... its font size is larger than the most frequent font size in the document.
  if (math_utils::larger(element->fontSize, mostFreqFontSize, FS_EQUAL_TOLERANCE)) {
    return true;
  }

  // ... its font weight is larger than the most frequent font weight.
  if (math_utils::equalOrLarger(element->fontSize, mostFreqFontSize, FS_EQUAL_TOLERANCE)
      && math_utils::larger(elementFontInfo->weight, docFontInfo->weight, 100)) {
    return true;
  }

  // ... it is printed in italics.
  if (math_utils::equalOrLarger(element->fontSize, mostFreqFontSize, FS_EQUAL_TOLERANCE)
      && elementFontInfo->isItalic) {
    return true;
  }

  // ... it contains at least one alphabetic character and all alphabetic characters are
  // in uppercase.
  bool containsAlpha = false;
  bool isAllAlphaUpper = true;
  for (size_t j = 0; j < element->text.size(); j++) {
    if (isalpha(element->text[j])) {
      containsAlpha = true;
      if (islower(element->text[j])) {
        isAllAlphaUpper = false;
        break;
      }
    }
  }
  if (containsAlpha && isAllAlphaUpper) {
    return true;
  }

  return false;
}