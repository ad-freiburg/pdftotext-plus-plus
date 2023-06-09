/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_TEXTLINESUTILS_H_
#define UTILS_TEXTLINESUTILS_H_

#include <regex>
#include <string>
#include <unordered_set>

#include "../Constants.h"
#include "../PdfDocument.h"

using std::max;
using std::regex;
using std::string;
using std::unordered_set;

// =================================================================================================
// CONFIG

namespace text_lines_utils::config {

// A parameter used for computing whether or not two coordinates are (approximately) equal. It
// denotes the maximum allowed difference between two coordinates so that both coordinates are
// considered to be equal.
const double COORDS_EQUAL_TOLERANCE = global_config::COORDS_EQUAL_TOLERANCE;

// -------
// Config for computeHasPrevLineCapacity().

/**
 * This method returns a threshold that is used for computing whether or not the previous text line
 * has capacity. If the difference between the right margin of the previous line and the width of
 * the first word of the current text line is larger than this threshold, the previous line is
 * considered to have capacity. Otherwise, the previous line is considered to have *no* capacity.
 *
 * @param doc
 *    The currently processed PDF document.
 *
 * @return
 *    The threshold.
 */
constexpr double getPrevTextLineCapacityThreshold(const PdfDocument* doc) {
  return 2.0 * doc->avgCharWidth;
}

// -------
// Config for computeTextLineHierarchy().

// A parameter that is used for computing the text line hierarchy. It denotes the maximum line
// distance between two text lines so that the one text line is considered to be a candidate for
// the parent text line (or a sibling text line) of the other text line.
const double LINE_HIERARCHY_MAX_LINE_DIST = 10.0;

/**
 * This method returns a threshold that is used for computing whether or not a text line is a
 * parent text line or a sibling text line of another text line. If the leftX value of a text line
 * L is larger than the leftX value of another text line M, M is considered to be the parent text
 * line of L. If the difference between the leftX values is smaller than this threshold, the text
 * lines are considered to be sibling text lines.
 *
 * @param doc
 *    The currently processed PDF document.
 *
 * @return
 *    The threshold.
 */
constexpr double getTextLineHierarchyLeftXOffsetThreshold(const PdfDocument* doc) {
  return 1.0 * doc->avgCharWidth;
}

// -------
// Config for computeIsCentered().

// A parameter used for computing whether or not a text line is centered compared to another
// text line. It denotes the minimum ratio by which one of the text line must horizontally overlap
// the the other text line, so that the text lines are considered to be centered to each other.
// If the maximum x-overlap ratio between both text lines is smaller than this value, the
// text lines are considered to be *not* centered.
const double CENTERING_X_OVERLAP_RATIO_THRESHOLD = 0.99;

/**
 * This method returns the maximum allowed difference between the left x-offset and right x-offset
 * of a text line (computed relatively to the previous text line), so that both offsets are
 * considered to be equal and that the text line is considered to be centered compared to the
 * previous text line.
 *
 * @param doc
 *    The currently processed PDF document.
 *
 * @return
 *    The maximum allowed difference between the left x-offset and right x-offset of a text line.
 */
constexpr double getCenteringXOffsetEqualTolerance(const PdfDocument* doc) {
  assert(doc);
  return 2.0 * doc->avgCharWidth;
}

// -------
// Config for computeIsPrefixedByItemLabel().

// The regular expressions we use to detect enumeration item labels.
const regex ITEM_LABEL_REGEXES[] = {
  // A regex to find item labels of form "• ", or "- ", or "+ ", etc.
  regex("^(•|-|–|\\+)\\s+"),
  // A regex to find item labels of form "I. ", "II. ", "III. ", "IV. ", etc.
  regex("^(X{0,1}(IX|IV|V?I{0,3}))\\.\\s+", std::regex_constants::icase),
  // A regex to find item labels of form "(I)", "(II)", "(III)", "(IV) ", etc.
  regex("^\\((X{0,1}(IX|IV|V?I{0,3}))\\)\\s+", std::regex_constants::icase),
  // A regex to find item labels of form "a. ", "b. ", "c. ", etc.
  regex("^([a-z])\\.\\s+"),
  // A regex to find item labels of form "1. ", "2. ", "3. ", etc.
  regex("^([0-9]+)\\.\\s+"),
  // A regex to find item labels of form "(A) ", "(1) ", "(C1) ", "[1] ", "[2] ", etc.
  regex("^(\\(|\\[)([a-z0-9][0-9]{0,2})(\\)|\\])\\s+", std::regex_constants::icase),
  // A regex to find item labels of form "[Bu2] ", "[Ch] ", "[Enn2020] ", etc.
  regex("^(\\[)([A-Z][a-zA-Z0-9]{0,5})(\\])\\s+"),
  // A regex to find item labels of form "A) " or "1) " or "a1) ".
  regex("^([a-z0-9][0-9]{0,1})\\)\\s+", std::regex_constants::icase),
  // A regex to find item labels of form "PACS" (1011.5073).
  regex("^PACS\\s+", std::regex_constants::icase)
};

// An alphabet of characters which we consider to be a valid part of a superscripted item label.
const char* const SUPER_ITEM_LABEL_ALPHABET = "*∗abcdefghijklmnopqrstuvwxyz01234567890()";

}  // namespace text_lines_utils::config

// =================================================================================================

/**
 * A collection of some useful and commonly used functions in context of text lines.
 */
namespace text_lines_utils {

/**
 * This method returns true if the given text line is the first line of an enumeration item or of
 * a footnote.
 *
 * For the returned value to be true, the line must be prefixed by an item label (that is:
 * computeIsPrefixedByItemLabel(line) must return true) and one of the following further
 * requirements must be fulfilled:
 * (1) If the given line has a previous sibling line (stored in line.prevSibling), it is also
 *     prefixed by an item label, and it exhibits the same font and font size as the given line;
 * (2) If the given line has a next sibling line (stored in line.nextSibling), it is also
 *     prefixed by an item label, and it exhibits the same font and font size as the given line;
 * (3) The line is prefixed by a footnote label, that is: computeIsPrefixedByFootnoteLabel(line,
 *     potentialFootnoteLabels) returns true.
 *
 * @param line
 *    The text line to process.
 * @param potentialFootnoteLabels
 *    A set of strings that is used to check if the line is the first line of a footnote. It
 *    contains strings that occur somewhere in the document as a superscript, meaning that each
 *    string represents a potential footnote label. If a line starts with a string that occur in
 *    this set, we consider the line as a potential first line of a footnote. Further heuristics
 *    are used to distinguish lines which are indeed the first line of a footnote from lines that
 *    occasionally start with a footnote label (but are actually not part of a footnote).
 *
 * @return
 *    True if the given line is the first line of an enumeration item or of a footnote, false
 *    otherwise.
 */
bool computeIsFirstLineOfItem(const PdfTextLine* line,
    const unordered_set<string>* potentialFootnoteLabels = nullptr);

/**
 * This method returns true if the given line is a continuation line of an enumeration item or of
 * a footnote, that is: if the line belongs to an enumeration item (resp. a footnote) but it is not
 * the first line of the item (resp. the footnote).
 *
 * For the returned value to be true, the given line must have a parent line (stored in
 * line.parentLine), which is either the first line of an item (resp. footnote), or also the
 * continuation of an item (resp. footnote).
 *
 * TODO: The assumption here is that the continuation line of an item or footnote is indented
 * compared to the first line of the item (otherwise, the continuation does not have a parent line).
 * This is however not always the case (there are items where the continuation lines are not
 * intended).
 *
 * @param line
 *    The line to process.
 * @param potentialFootnoteLabels
 *    The set of potential footnote labels, passed to the computeIsFirstLineOfItem() method. See
 *    the comment given for this method for more information about this parameter.
 *
 * @return
 *    True if the given line is a continuation line of an enumeration item or a footnote, false
 *    otherwise.
 */
bool computeIsContinuationOfItem(const PdfTextLine* line,
    const unordered_set<string>* potentialFootnoteLabels = nullptr);

// =================================================================================================

/**
 * This method returns true if the given line is prefixed by an enumeration item label, that is:
 * if it starts with a *superscripted* character that occurs in SUPER_ITEM_LABEL_ALPHABET or if it
 * matches one of the regular expressions in ITEM_LABEL_REGEXES (note that the matching parts must
 * *not* be superscripted).
 *
 * @param line
 *    The line to process.
 *
 * @return
 *    True if the line is prefixed by an enumeration label, false otherwise.
 */
bool computeIsPrefixedByItemLabel(const PdfTextLine* line);

/**
 * This method returns true if the given line is prefixed by a footnote label.
 *
 * For the returned value to be true, all of the following requirements must be fulfilled:
 * (1) The given line starts with one or more superscripted characters.
 * (2) If 'potentialFootnoteLabels' is specified, it must contain the superscripted prefix (= the
 *     concatenation of all superscripted characters in front of the line).
 *
 * @param line
 *    The line to process.
 * @param potentialFootnoteLabels
 *    The set of potential footnote labels.
 *
 * @return
 *    True if the line is prefixed by an enumeration label, false otherwise.
 */
bool computeIsPrefixedByFootnoteLabel(const PdfTextLine* line, const unordered_set<string>*
    potentialFootnoteLabels = nullptr);

// =================================================================================================

/**
 * This method returns true if the previous line of the given line has capacity, that is: if the
 * first word of the given line would have enough space to be placed at the end of the previous
 * line (or: if the right margin of the previous line is larger than the width of the first word
 * of the given line + some extra space for an additional whitespace).
 *
 * This method is primarily used to detect text block boundaries and forced line breaks. If this
 * method returns true, it is assumed that the given line and its previous line do not belong to
 * the same text block, because otherwise the first word of the given line could have been placed
 * at the end of the previous line.
 *
 * @param line
 *    The line to process.
 *
 * @return
 *    True if the previous line of the given line has capacity, false otherwise.
 */
bool computeHasPrevLineCapacity(const PdfTextLine* line);

/**
 * This method computes the parent text line, the previous sibling text line and the next sibling
 * text line for each text line of the given page. Here is an explanation of the different types
 * of lines:
 *
 * - Parent Text Line: a text line L is the parent text line of text line M if
 *   (a) L is the nearest previous text line of M with L.leftX < M.leftX (meaning that M is
 *       indented compared to L).
 *   (b) the line distance between L and M is smaller than a given threshold.
 *   (c) L.lowerY < M.lowerY (meaning that M must be positioned below L).
 *
 * - Previous Sibling Text Line: a text line L is the previous sibling text line of text line M if
 *   (a) L is the nearest previous text line of M with L.leftX == M.leftX (under consideration
 *       of a small tolerance)
 *   (b) there is no other text line K between L and M with K.leftX < M.leftX.
 *   (c) the line distance between L and M is smaller than a given threshold.
 *   (d) L.lowerY < M.lowerY (meaning that M must be positioned below L).
 *
 * - Next Sibling Text Line: a text line L is the next sibling text line of text line M if:
 *   (a) L is the nearest next text line of M with L.leftX == M.leftX (under consideration
 *       of a small tolerance)
 *   (b) there is no other text line K between M and L with K.leftX < M.leftX.
 *   (c) the line distance between L and M is smaller than a given threshold.
 *   (d) L.lowerY < M.lowerY (meaning that M must be positioned below L).
 *
 * Here is an example which helps to understand the different line types:
 *
 * Aarseth S J 1999 PASP 111 1333            (1)
 * Amaro-Seoane P, Gair J R, Freitag M,      (2)
 *   Miller M C, Mandel I, Cutler C J        (3)
 *   and Babak S 2007 Classical and          (4)
 *   Quantum Gravity 24 113                  (5)
 * Brown D A, Brink J, Fang H, Gair J R,     (6)
 *   Li C, Lovelace G, Mandel I and Thorne   (7)
 *     K S 2007 PRL 99 201102                (8)
 *
 * Line (1):  parent: -        ; prev sibling: -        ; next sibling: line (2)
 * Line (2):  parent: -        ; prev sibling: line (1) ; next sibling: line (6)
 * Line (3):  parent: line (2) ; prev sibling: -        ; next sibling: line (4)
 * Line (4):  parent: line (2) ; prev sibling: line (3) ; next sibling: line (5)
 * Line (5):  parent: line (2) ; prev sibling: line (4) ; next sibling: -
 * Line (6):  parent: -        ; prev sibling: line (2) ; next sibling: -
 * Line (7):  parent: line (6) ; prev sibling: -        ; next sibling: -
 * Line (8):  parent: line (7) ; prev sibling: -        ; next sibling: -
 *
 * The entry for line (3) in the above listing is to be read as follows:
 *  - "The parent line of line (3) is line (2)";
 *  - "Line (3) has no previous sibling line."
 *  - "The next sibling line of line (3) is line (4)."
 *
 * The reason why line (5) is not a previous sibling of line (7) is that there is line (6)
 * in between, which have a smaller leftX than line (5) and line (7).
 *
 * @param page
 *    The page to process.
 */
void computeTextLineHierarchy(const PdfPage* page);

/**
 * This method computes potential footnote labels contained in the given line and appends it to
 * the given set.
 *
 * This method is primarily used by the text block detector, for detecting the first text lines of
 * footnotes. The motivation is the following: the first line of a footnote is usually prefixed by
 * a label that consists of a superscripted character or number, or a special symbol like:
 * *, †, ‡, §, ‖, ¶. However, a PDF can contain text lines which occasionally starts with such a
 * label, although they are not an actual part of a footnote. A possible consequence is that lines
 * which are not an actual part of a footnote are mistakenly detected as footnotes.
 *
 * One observation is that the label of a footnote usually occurs a second time in the body text
 * of the document (this is for referencing the footnote at a certain position in the body text).
 * We use this fact and scan the given line for labels (that is: superscripted numbers and the
 * special symbols mentioned above) that potentially reference a footnote. On detecting footnotes,
 * we consider a line to be the start of a footnote only when it is prefixed by text that occurs in
 * the computed set of potential footnote labels.
 *
 * @param line
 *    The line to process.
 * @param result
 *    The set to which the detected potential footnote labels should be appended.
 */
void computePotentialFootnoteLabels(const PdfTextLine* line, unordered_set<string>* result);

/**
 * This method returns true if the given lines are centered compared to each other.
 *
 * For the returned value to be true, all of the following requirements must be fulfilled:
 * (1) One of the lines must completely overlap the respective other line horizontally, that is:
 *     one of the values returned by element_utils::computeXOverlapRatios(line.prevLine, line) must
 *     be equal to 1.
 * (2) The leftX offset (= line1.leftX - line2.leftX) and the rightX offset (= line1.rightX -
 *     line2.rightX) must be equal, under consideration of a tolerance computed by using the given
 *     factor.
 *
 * @param line1
 *   The first line to process.
 * @param line2
 *   The second line to process.
 *
 * @return
 *    True, if the two given lines are centered with respect to the requirements mentioned above,
 *    false otherwise.
 */
bool computeIsCentered(const PdfTextLine* line1, const PdfTextLine* line2);

}  // namespace text_lines_utils

#endif  // UTILS_TEXTLINESUTILS_H_
