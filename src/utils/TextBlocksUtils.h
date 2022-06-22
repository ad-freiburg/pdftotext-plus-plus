/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_TEXTBLOCKSUTILS_H_
#define UTILS_TEXTBLOCKSUTILS_H_

#include <string>
#include <unordered_set>
#include <vector>

#include "../PdfDocument.h"

using std::vector;

// =================================================================================================
// CONFIG

namespace text_blocks_utils::config {

// The characters we use to identify formulas.
const char* const FORMULA_ID_ALPHABET = global_config::FORMULA_ID_ALPHABET;

// A set of common last name prefixes, e.g.: "van", "de", etc.
const unordered_set<string> LAST_NAME_PREFIXES = global_config::LAST_NAME_PREFIXES;

// A factor that is used to compute a tolerance for checking if two given leftX- or rightX
// offsets are equal. The tolerance is computed as <FACTOR> * doc->avgCharWidth. If the
// difference between two offsets is smaller than this tolerance, they are considered to be
// equal. Otherwise they are considered to be not equal.
const double CENTERING_XOFFSET_EQUAL_TOLERANCE_FACTOR = 2.0;

// The maximum number of justified lines a text block may contain so that the text lines of the
// text block are considered to be centered.
const int CENTERING_MAX_NUM_JUSTIFIED_LINES = 5;

// The minimum length of a text line so that the line is considered to be a "long" text line.
const double HANG_INDENT_MIN_LENGTH_LONG_LINES = 3;

// If all non-indented lines of a text block start with an uppercase character and if the number
// of non-indented lines is larger than this threshold, the block is in hanging indent format.
const int HANG_INDENT_MIN_NUM_NON_INDENTED_LINES = 10;

// If there is at least one indented line that starts with a lowercase character, and the number
// of long lines is larger than this threshold, the text block is in hanging indent format.
const int HANG_INDENT_MIN_NUM_LONG_LINES = 4;

// A factor that is used to compute a threshold for checking if the left margin of a text line is
// "large enough" so that the text line is considered to be indented. The threshold is computed as
// <FACTOR> * doc->avgCharWidth. If the left margin is larger than this threshold, the text line is
// considered to be indented; otherwise it is considered to be not indented.
const double HANG_INDENT_MARGIN_THRESHOLD_FACTOR = 1.0;

// A value in [0, 1] denoting the minimum percentage of *indented* lines in a given text block that
// exhibit the most frequent left margin. If the percentage of such lines is smaller than this
// value, the text block is considered to be not in hanging indent format.
const double HANG_INDENT_MIN_PERC_LINES_SAME_LEFT_MARGIN = 0.5;

// The maximum number of lowercased non-indented lines in a text block so that the text block
// is considered to be in hanging indent format.
const int HANG_INDENT_MAX_NUM_LOWERCASED_NON_INDENTED_LINES = 0;

// The minimum number of lowercased indented lines in a text block so that the text block
// is considered to be in hanging indent format.
const int HANG_INDENT_MIN_NUM_LOWERCASED_INDENTED_LINES = 1;

}  // namespace text_blocks_utils::config

// =================================================================================================

using text_blocks_utils::config::CENTERING_MAX_NUM_JUSTIFIED_LINES;
using text_blocks_utils::config::CENTERING_XOFFSET_EQUAL_TOLERANCE_FACTOR;
using text_blocks_utils::config::HANG_INDENT_MARGIN_THRESHOLD_FACTOR;
using text_blocks_utils::config::HANG_INDENT_MAX_NUM_LOWERCASED_NON_INDENTED_LINES;
using text_blocks_utils::config::HANG_INDENT_MIN_LENGTH_LONG_LINES;
using text_blocks_utils::config::HANG_INDENT_MIN_NUM_LONG_LINES;
using text_blocks_utils::config::HANG_INDENT_MIN_NUM_LOWERCASED_INDENTED_LINES;
using text_blocks_utils::config::HANG_INDENT_MIN_NUM_NON_INDENTED_LINES;
using text_blocks_utils::config::HANG_INDENT_MIN_PERC_LINES_SAME_LEFT_MARGIN;

/**
 * A collection of some useful and commonly used functions in context of text blocks.
 */
namespace text_blocks_utils {

/**
 * This method returns true if the lines contained in the given text block are centered; false
 * otherwise.
 *
 * For the returned value to be true, all of the following requirements must be fulfilled:
 * (1) Each line in the block is centered compared to the respective previous line.
 *     NOTE: We consider a text line L as centered compared to another text line M, if:
 *      (a) the leftX offset (= L.leftX - M.leftX) is equal to the rightX offset
 *          (= L.rightX - M.rightX)
 *      (b) one line completely overlaps the other line horizontally, that is: one of the values
 *          returned by element_utils::computeXOverlapRatios(line.prevLine, line) is equal to 1;
 * (2) There is at least one line (which does not represent a display formula) for which the leftX
 *     offset (resp. rightX offset) is larger than a given threshold;
 * (3) The number of justified text lines (that is: lines with leftX offset == rightX offset == 0)
 *     is smaller than a given threshold.
 *
 * @param block
 *    The text block to process.
 * @param xOffsetEqualToleranceFactor
 *    A factor that is used to compute a tolerance for checking if the leftX- and rightX offsets
 *    of a line in the given block is equal. The tolerance is computed as:
 *      <xOffsetEqualToleranceFactor> * doc->avgCharWidth.
 *    If the difference between the two offsets is smaller than this tolerance, they are considered
 *    to be equal. Otherwise they are considered to be not equal.
 * @param maxNumJustifiedLines
 *    The maximum number of justified lines the given text block may contain so that the text lines
 *    of the block are considered to be centered.
 *
 * @return
 *    True if the lines contained in the given text block are centered with regard to the
 *    requirements described above; false otherwise.
 */
bool computeIsTextLinesCentered(const PdfTextBlock* block,
    double xOffsetEqualToleranceFactor = CENTERING_XOFFSET_EQUAL_TOLERANCE_FACTOR,
    double maxNumJustifiedLines = CENTERING_MAX_NUM_JUSTIFIED_LINES);

/**
 * This method checks if the given block is in hanging indent format (meaning that the first line
 * of a text block is not indented and the continuation lines are indented by a certain value).
 * If the block is in hanging indent format, this method returns a value > 0, denoting the value
 * (in pt) by which the continuation lines are indented. If the block is not in hanging indent
 * format, this method returns 0.0.
 *
 * For the returned value to be > 0.0, all of the following requirements must be fulfilled:
 * (1) The block must contain at least two lines with a length larger than a threshold.
 * (2) The block must contain at least one indented line, that is: a line with a left margin equal
 *     to the most frequent left margin (which is computed among the lines in the block that
 *     exhibit a left margin larger than a threshold).
 * (3) The block does not contain any non-indented line that starts with a lowercase character.
 *
 * Additionally, one of the following requirements must be fulfilled:
 * (a) The first line is not indented, but all other lines, and the first line has no capacity.
 *     This should identify single enumeration items in the format:
 *       Dynamics: The low energy behavior of
 *         a physical system depends on its
 *         dynamics.
 * (b) The number of non-indented lines exceeds a given threshold, and all non-indented lines start
 *     with an uppercase character.
 * (c) There is at least one indented line that start with an uppercase character, and the number
 *     of lines exceeds a given threshold.
 *
 * NOTE: The given text block may be a preliminary text block (computed by the text block
 * detector), meaning that it could contain multiple text blocks, which need to be split further
 * in a subsequent step.
 *
 * @param block
 *    The text block to process.
 * @param minLengthLongLines
 *    The minimum length of a line in the given block so that the line is considered to be a long
 *    line (the block is only then considered to be in hanging indent format when the number of
 *    long lines exceeds <minNumLongLines>).
 * @param marginThresholdFactor
 *    A factor that is used to compute a threshold for checking if the left margin of a text line
 *    in the given block is "large enough" so that the text line is considered to be indented. The
 *    threshold is computed as
 *      <marginThresholdFactor> * doc->avgCharWidth
 *    If the left margin is larger than this threshold, the text line is considered to be indented;
 *    otherwise it is considered to be not indented.
 * @param minPercLinesSameLeftMargin
 *    A value in [0, 1] denoting the minimum percentage of *indented* lines in the given text block
 *    that exhibit the most frequent left margin. If the percentage of such lines is smaller than
 *    this value, the text block is considered to be not in hanging indent format.
 * @param maxNumLowercasedNonIndentedLines
 *    The maximum number of lowercased non-indented lines in the given text block so that the text
 *    block is considered to be in hanging indent format.
 * @param minNumNonIndentedLines
 *    If all non-indented lines of a text block start with an uppercase character and if the number
 *    of non-indented lines is larger than this value, the block is considered to be in hanging
 *    indent format.
 * @param minNumLowercasedIndentedLines
 *    The minimum number of lowercased indented lines in the given text block so that the text
 *    block is considered to be in hanging indent format.
 * @param minNumLongLines
 *    If there is at least one indented line that starts with a lowercase character, and the number
 *    of long lines is larger than this value, the text block is conisdered to be in hanging indent
 *    format.
 *
 * @return
 *    A value > 0 if the block is in hanging indent format, and 0.0 otherwise.
 */
double computeHangingIndent(const PdfTextBlock* block,
    double minLengthLongLines = HANG_INDENT_MIN_LENGTH_LONG_LINES,
    double marginThresholdFactor = HANG_INDENT_MARGIN_THRESHOLD_FACTOR,
    double minPercLinesSameLeftMargin = HANG_INDENT_MIN_PERC_LINES_SAME_LEFT_MARGIN,
    double maxNumLowercasedNonIndentedLines = HANG_INDENT_MAX_NUM_LOWERCASED_NON_INDENTED_LINES,
    double minNumNonIndentedLines = HANG_INDENT_MIN_NUM_NON_INDENTED_LINES,
    double minNumLowercasedIndentedLines = HANG_INDENT_MIN_NUM_LOWERCASED_INDENTED_LINES,
    double minNumLongLines = HANG_INDENT_MIN_NUM_LONG_LINES);

/**
 * This method iterates through the text lines of the given block (stored in block.lines),
 * computes the left and right margins of each. Writes the computed left margin of text line L to
 * L.leftMargin and the computed right margin to L.rightMargin.
 *
 * The left margin of the text line L in block B is the distance between the left boundary of B and
 * the left boundary of L, that is: abs(L.leftX - B.trimLeftX).
 * The right margin of L is the distance between the right boundary of L and the right boundary of
 * B, that is: abs(B.trimRightX - L.rightX).
 *
 * TODO: The right margin of a text line is primarily used to check if the text line has capacity
 * (see the comment of text_lines_utils::computeHasPrevLineCapacity() for information about how the
 * capacity of a line is defined). There are text blocks that consists of only short lines
 * (meaning that they are shorter than the lines in the same column). See the second block on page
 * 2 in hep-ex0205091 for an example. For such blocks, the computed right margins of the text lines
 * are smaller than they actually are. A frequent consequence is that it is assumed that the text
 * lines have no capacity, although they actually do (because they are short, and there is enough
 * space for some further text). So the correct way of computing the right margin would be to
 * compute the distance between the right boundary of the text line and the "correct" right
 * boundary of the column. However, finding the "correct" right boundary of the column is
 * surprisingly difficult. One option is to use the largest rightX value of a line in the column.
 * But there could be outlier lines, which extend beyond the actual column boundaries. Another
 * option is to use the most frequent rightX among the lines in the column (this sounds promising
 * and should be tried out).
 *
 * @param block
 *    The text block to process.
 */
void computeTextLineMargins(const PdfTextBlock* block);

/**
 * This method creates a new `PdfTextBlock` instance consisting of the given text lines, computes
 * and sets all parameters of the instance and appends the instance to the given vector.
 *
 * @param lines
 *    The text lines belonging to the text block to create.
 * @param blocks
 *    The vector to which the created text block should be appended.
 */
void createTextBlock(const vector<PdfTextLine*>& lines, vector<PdfTextBlock*>* blocks);

}  // namespace text_blocks_utils

#endif  // UTILS_TEXTBLOCKSUTILS_H_
