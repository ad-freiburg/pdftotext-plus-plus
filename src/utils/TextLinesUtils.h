/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef text_lines_utils_H_
#define text_lines_utils_H_

#include <regex>

#include "../PdfDocument.h"

using std::string;

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

namespace text_lines_utils {

/**
 * This method computes the (vertical) distance between the two given text lines.
 *
 * It first checks which of the two text lines is positioned "above" the other. Let l1 be
 * positioned above l2. The vertical line distance is computed as l2.upperY - l1.lowerY.
 *
 * @param l1
 *    The first line to process.
 * @param l2
 *    The second line to process.
 *
 * @return
 *    The vertical line distance between l1 and l2.
 */
double computeTextLineDistance(const PdfTextLine* l1, const PdfTextLine* l2);

/**
 * This method returns true if the given text line is the first line of an enumeration item or of
 * a footnote.
 *
 * For the returned value to be true, the line must be prefixed by an item label
 * (that is: the method computeIsPrefixedByItemLabel() must return true for the line) and one of
 * the following further requirements must be fullfilled:
 * (1) If there is a previous sibling line, it is also prefixed by an item label, and it exhibits
 *     the same font and font size like the given line;
 * (2) If there is a next sibling line, it is also prefixed by an item label, and it exhibits
 *     the same font and font size like the given line;
 * (3) The method computeIsPrefixedByFootnoteLabel() returns true for the given line and the
 *     given set of potential footnote labels.
 *
 * @param line
 *    The text line to process.
 * @param potentialFootnoteLabels
 *    A set of strings that is used to identify the first line of a footnote. It contains strings
 *    that occur somewhere in the document as a superscript. Thus, each string represents a
 *    potential footnote label. If a line starts with a string that occur in this set, it may
 *    (but not necessarily must) be the first line of a footnote.
 *
 * @return
 *    True if the given line is the first line of an enumeration item or of a footnote, false
 *    otherwise.
 */
bool computeIsFirstLineOfItem(const PdfTextLine* line,
    const unordered_set<string>* potentialFootnoteLabels=nullptr);

bool computeIsContinuationOfItem(const PdfTextLine* line, const unordered_set<string>*
    potentialFootnoteLabels=nullptr);
bool computeIsPrefixedByItemLabel(const PdfTextLine* line);
bool computeIsPrefixedByFootnoteLabel(const PdfTextLine* line, const unordered_set<string>*
    potentialFootnoteLabels=nullptr);
bool computeHasPrevLineCapacity(const PdfTextLine* line);

void computeTextLineIndentHierarchies(const PdfPage* page);

void computePotentialFootnoteLabels(const PdfTextLine* line, unordered_set<string>* result);

bool computeIsCentered(const PdfTextLine* line1, const PdfTextLine* line2,
    double xOffsetToleranceFactor=1.0);
}  // namespace text_lines_utils

#endif  // TEXT_LINE_H_