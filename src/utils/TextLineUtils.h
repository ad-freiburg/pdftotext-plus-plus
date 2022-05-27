/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TEXT_LINE_UTILS_H_
#define TEXT_LINE_UTILS_H_

#include <regex>

#include "../PdfDocument.h"

using namespace std;

// =================================================================================================

// The characters which we consider to be a valid part of a superscripted item label.
const string SUPER_ITEM_LABEL_ALPHABET = "*∗abcdefghijklmnopqrstuvwxyz01234567890()";

// Some regular expressions to find item labels.
const regex ITEM_LABEL_REGEXES[] = {
  // A regex to find item labels of form "• ", or "- ", or "+ ", etc.
  regex("^(•|-|–|\\+)\\s+"),
  // A regex to find item labels of form "I. ", "II. ", "III. ", "IV. ", etc.
  regex("^(X{0,1}(IX|IV|V?I{0,3}))\\.\\s+", regex_constants::icase),
  // A regex to find item labels of form "(I)", "(II)", "(III)", "(IV) ", etc.
  regex("^\\((X{0,1}(IX|IV|V?I{0,3}))\\)\\s+", regex_constants::icase),
  // A regex to find item labels of form "a. ", "b. ", "c. ", etc.
  regex("^([a-z])\\.\\s+"),
  // A regex to find item labels of form "1. ", "2. ", "3. ", etc.
  regex("^([0-9]+)\\.\\s+"),
  // A regex to find item labels of form "(A) ", "(1) ", "(C1) ", "[1] ", "[2] ", etc.
  regex("^(\\(|\\[)([a-z0-9][0-9]{0,2})(\\)|\\])\\s+", regex_constants::icase),
  // A regex to find item labels of form "[Bu2] ", "[Ch] ", "[Enn2020] ", etc.
  regex("^(\\[)([A-Z][a-zA-Z0-9]{0,5})(\\])\\s+"),
  // A regex to find item labels of form "A) " or "1) " or "a1) ".
  regex("^([a-z0-9][0-9]{0,1})\\)\\s+", regex_constants::icase),
  // A regex to find item labels of form "PACS" (1011.5073).
  regex("^PACS\\s+", regex_constants::icase)
};

const std::string FOOTNOTE_LABEL_ALPHABET = "*∗†‡?";

// =================================================================================================

namespace text_line_utils {

double computeTextLineDistance(const PdfTextLine* prevLine, const PdfTextLine* line);
bool computeIsFirstLineOfItem(const PdfTextLine* line, const unordered_set<string>*
    potentialFootnoteLabels=nullptr);
bool computeIsContinuationLineOfItem(const PdfTextLine* line, const unordered_set<string>*
    potentialFootnoteLabels=nullptr);
bool computeIsPrefixedByItemLabel(const PdfTextLine* line);
bool computeIsPrefixedByFootnoteLabel(const PdfTextLine* line, const unordered_set<string>*
    potentialFootnoteLabels=nullptr);
bool computeHasPrevLineCapacity(const PdfTextLine* line);

void computeTextLineIndentHierarchies(const PdfPage* page);

void computePotentialFootnoteLabels(const PdfTextLine* line, unordered_set<string>* result);
}

#endif  // TEXT_LINE_H_