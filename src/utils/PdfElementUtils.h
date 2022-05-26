/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef PDFELEMENT_UTILS_H_
#define PDFELEMENT_UTILS_H_

#include <limits>
#include <vector>

#include "../PdfDocument.h"

// =================================================================================================

const std::string SENTENCE_DELIMITER_ALPHABET = "?!.);";

const double naan = std::numeric_limits<double>::quiet_NaN();

// =================================================================================================

namespace element_utils {

std::pair<double, double> computeOverlapRatios(double s1, double e1, double s2, double e2);
std::pair<double, double> computeXOverlapRatios(const PdfElement* elem1, const PdfElement* elem2);
std::pair<double, double> computeYOverlapRatios(const PdfElement* elem1, const PdfElement* elem2);

bool computeHasEqualLeftX(const PdfElement* elem1, const PdfElement* elem2, double tolerance=naan);
bool computeHasEqualUpperY(const PdfElement* elem1, const PdfElement* elem2, double tolerance=naan);
bool computeHasEqualRightX(const PdfElement* elem1, const PdfElement* elem2, double tolerance=naan);
bool computeHasEqualLowerY(const PdfElement* elem1, const PdfElement* elem2, double tolerance=naan);

double computeLeftXOffset(const PdfElement* elem1, const PdfElement* elem2);

PdfFigure* overlapsFigure(const PdfElement* element, const std::vector<PdfFigure*>& figures,
    double minXOverlapRatio = 0.5, double minYOverlapRatio = 0.5);

}

namespace text_element_utils {

bool computeHasEqualFont(const PdfTextElement* element1, const PdfTextElement* element2);
bool computeHasEqualFontSize(const PdfTextElement* element1, const PdfTextElement* element2,
    double tolerance=1);
bool computeEndsWithSentenceDelimiter(const PdfTextElement* element);
bool computeStartsWithUpper(const PdfTextElement* element);
bool computeIsEmphasized(const PdfTextElement* element);

}

#endif  // PDFELEMENT_UTILS_H_