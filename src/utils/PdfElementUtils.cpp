/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cmath>
#include <vector>

#include "../PdfDocument.h"
#include "./MathUtils.h"
#include "./PdfElementUtils.h"

// _________________________________________________________________________________________________
std::pair<double, double> element_utils::computeOverlapRatios(double s1, double e1,
      double s2, double e2) {
  double min1 = std::min(s1, e1);
  double max1 = std::max(s1, e1);
  double min2 = std::min(s2, e2);
  double max2 = std::max(s2, e2);
  double height1 = max1 - min1;
  double height2 = max2 - min2;

  double minMax = std::min(max1, max2);
  double maxMin = std::max(min1, min2);
  double overlapLength = std::max(0.0, minMax - maxMin);
  double overlapRatio1 = height1 > 0 ? overlapLength / height1 : 0;
  double overlapRatio2 = height2 > 0 ? overlapLength / height2 : 0;

  return std::make_pair(overlapRatio1, overlapRatio2);
}

// _________________________________________________________________________________________________
std::pair<double, double> element_utils::computeXOverlapRatios(const PdfElement* element1,
      const PdfElement* element2) {
  return element_utils::computeOverlapRatios(element1->position->rightX, element1->position->leftX,
      element2->position->rightX, element2->position->leftX);
}

// _________________________________________________________________________________________________
double element_utils::computeMaxXOverlapRatio(const PdfElement* elem1, const PdfElement* elem2) {
  pair<double, double> ratios = element_utils::computeXOverlapRatios(elem1, elem2);
  return max(ratios.first, ratios.second);
}

// _________________________________________________________________________________________________
std::pair<double, double> element_utils::computeYOverlapRatios(const PdfElement* element1,
      const PdfElement* element2) {
  return element_utils::computeOverlapRatios(element1->position->upperY, element1->position->lowerY,
      element2->position->upperY, element2->position->lowerY);
}

// _________________________________________________________________________________________________
double element_utils::computeMaxYOverlapRatio(const PdfElement* elem1, const PdfElement* elem2) {
  pair<double, double> ratios = element_utils::computeYOverlapRatios(elem1, elem2);
  return max(ratios.first, ratios.second);
}

// _________________________________________________________________________________________________
PdfFigure* element_utils::computeOverlapsFigure(const PdfElement* elem,
      const std::vector<PdfFigure*>& figures, double minXOverlapRatio, double minYOverlapRatio) {

  for (auto* figure : figures) {
    std::pair<double, double> xOverlapRatios = element_utils::computeXOverlapRatios(elem, figure);
    std::pair<double, double> yOverlapRatios = element_utils::computeYOverlapRatios(elem, figure);

    if (xOverlapRatios.first > minXOverlapRatio && yOverlapRatios.first > minYOverlapRatio) {
      return figure;
    }
  }

  return nullptr;
}

// _________________________________________________________________________________________________
bool element_utils::computeHasEqualLeftX(const PdfElement* elem1, const PdfElement* elem2,
      double tolerance) {
  assert(elem1);
  assert(elem2);
  assert(elem1->doc == elem2->doc);
  if (std::isnan(tolerance)) { tolerance = elem1->doc->avgGlyphWidth; }
  return math_utils::equal(elem1->position->leftX, elem2->position->leftX, tolerance);
}

// _________________________________________________________________________________________________
bool element_utils::computeHasEqualUpperY(const PdfElement* elem1, const PdfElement* elem2,
      double tolerance) {
  assert(elem1);
  assert(elem2);
  assert(elem1->doc == elem2->doc);
  if (std::isnan(tolerance)) { tolerance = elem1->doc->avgGlyphHeight; }
  return math_utils::equal(elem1->position->upperY, elem2->position->upperY, tolerance);
}

// _________________________________________________________________________________________________
bool element_utils::computeHasEqualRightX(const PdfElement* elem1, const PdfElement* elem2,
      double tolerance) {
  assert(elem1);
  assert(elem2);
  assert(elem1->doc == elem2->doc);
  if (std::isnan(tolerance)) { tolerance = elem1->doc->avgGlyphWidth; }
  return math_utils::equal(elem1->position->rightX, elem2->position->rightX, tolerance);
}

// _________________________________________________________________________________________________
bool element_utils::computeHasEqualLowerY(const PdfElement* elem1, const PdfElement* elem2,
      double tolerance) {
  assert(elem1);
  assert(elem2);
  assert(elem1->doc == elem2->doc);
  if (std::isnan(tolerance)) { tolerance = elem1->doc->avgGlyphHeight; }
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
bool text_element_utils::computeHasEqualFont(const PdfTextElement* elem1,
      const PdfTextElement* elem2) {
  assert(elem1);
  assert(elem2);
  return elem1->fontName == elem2->fontName;
}

// _________________________________________________________________________________________________
bool text_element_utils::computeHasEqualFontSize(const PdfTextElement* elem1,
    const PdfTextElement* elem2, double tolerance) {
  assert(elem1);
  assert(elem2);
  return math_utils::equal(elem1->fontSize, elem2->fontSize, tolerance);
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
  const PdfFontInfo* docFontInfo = element->doc->fontInfos.at(element->doc->mostFreqFontName);
  const PdfFontInfo* elementFontInfo = element->doc->fontInfos.at(element->fontName);

  // The element is emphasized if ...

  // ... its font size is significantly larger than the most frequent font size in the document.
  // TODO: Parameterize the 0.5
  if (element->fontSize - element->doc->mostFreqFontSize > 0.5) {
    return true;
  }

  // ... its font weight is larger than the most frequent font weight.
  if (element->fontSize - element->doc->mostFreqFontSize >= -1
      && elementFontInfo->weight - docFontInfo->weight > 100) {
    return true;
  }

  // ... it is printed in italics.
  if (element->fontSize - element->doc->mostFreqFontSize >= -1 && elementFontInfo->isItalic) {
    return true;
  }

  // ... it contains at least one alphabetic character and all alphabetic characters are
  // upper case.
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