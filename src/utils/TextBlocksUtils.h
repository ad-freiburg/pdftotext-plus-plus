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

using std::string;
using std::unordered_set;
using std::vector;

// =================================================================================================

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
 * @param formulaIdAlphabet
 *    An alphabet that is used for computing whether or not a text line is part of a formula.
 *    It contains characters we consider to be part of a formula.
 * @param centeringXOverlapRatioThreshold
 *    A parameter used for computing whether or not a text line is centered compared to another
 *    text line. It denotes the minimum ratio by which one of the text line must horizontally
 *    overlap the the other text line, so that the text lines are considered to be centered to each
 *    other. If the maximum x-overlap ratio between both text lines is smaller than this value, the
 *    text lines are considered to be *not* centered.
 * @param centeringXOffsetEqualToleranceFactor
 *    This method returns a factor that is used to compute a threshold (= <factor> *
 *    'average character width in the PDF document') denoting the maximum allowed difference
 *    between the left x-offset and right x-offset of a text line (computed relatively to the
 *    previous text line), so that both offsets are considered to be equal and that the text line
 *    is considered to be centered compared to the previous text line.
 * @param centeringMaxNumJustifiedLines
 *    A parameter that is used for computing whether or not the text lines of a text block are
 *    centered among each other. It denotes the maximum number of justified lines (= lines with a
 *    left margin and right margin == 0) a text block is allowed to contain so that the text lines
 *    are considered to be centered.
 * @return
 *    True if the lines contained in the given text block are centered with regard to the
 *    requirements described above; false otherwise.
 */
bool computeIsTextLinesCentered(
    const PdfTextBlock* block,
    const string& formulaIdAlphabet,
    double centeringXOverlapRatioThreshold,
    double centeringXOffsetEqualToleranceFactor,
    int centeringMaxNumJustifiedLines);

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
 * @param lastNamePrefixes
 *    A set of common last name prefixes, e.g.: "van", "de", etc.
 * @param hangIndentMinLengthLongLines
 *    The min length of a text line so that the line is considered to be a "long" text line.
 * @param hangIndentMinPercLinesSameLeftMargin
 *    A parameter in [0, 1] denoting the minimum percentage of *indented* lines in a given text
 *    block that must exhibit the most frequent left margin > 0. If the percentage of such lines is
 *    smaller than this threshold, the text block is considered to be *not* in hanging indent
 *    format.
 * @param hangIndentNumNonIndentedLinesThreshold
 *    If all non-indented lines of a text block start with an uppercase character and if
 *    the number of non-indented lines is larger than this threshold, the block is considered to
 *    be in hanging indent format.
 * @param hangIndentMarginThresholdFactor
 *    A factor used to compute a threshold for checking if the left margin of a text line is
 *    "large enough" so that the text line is considered to be indented. If the left margin is
 *    larger than this threshold, the text line is considered to be indented; otherwise it is
 *    considered to be not indented.
 * @param hangIndentNumLowerNonIndentedLinesThreshold
 *    A parameter that denotes the maximum number of lowercased non-indented text lines a text
 *    block is allowed to contain so that the text block is considered to be in hanging indent
 *    format.
 * @param hangIndentNumLongLinesThreshold
 *    If there is at least one indented line that starts with a lowercase character, and
 *    the number of long lines is larger than this threshold, the text block is considered to be
 *    in hanging indent format
 * @param hangIndentNumLowerIndentedLinesThreshold
 *    A parameter that is used for computing whether or not a text block is in hanging indent
 *    format. It denotes the minimum number of lowercased indented lines a text block is allowed
 *    to contain so that the text block is considered to be in hanging indent format.
 * @param prevTextLineCapacityThresholdFactor
 *    A factor used to compute a threshold that is used for computing whether or not the previous
 *    text line has capacity (the threshold is computed as <factor> * 'avg. character width of the
 *    PDF document'). If the difference between the right margin of the previous line and the
 *    width of the first word of the current text line is larger than this threshold, the previous
 *    line is considered to have capacity. Otherwise, the previous line is considered to have *no*
 *    capacity.
 *
 * @return
 *    A value > 0 if the block is in hanging indent format, and 0.0 otherwise.
 */
double computeHangingIndent(
    const PdfTextBlock* block,
    const unordered_set<string>& lastNamePrefixes,
    int hangIndentMinLengthLongLines,
    double prevTextLineCapacityThresholdFactor,
    double hangIndentMinPercLinesSameLeftMargin,
    int hangIndentNumNonIndentedLinesThreshold,
    double hangIndentMarginThresholdFactor,
    int hangIndentNumLowerNonIndentedLinesThreshold,
    int hangIndentNumLongLinesThreshold,
    int hangIndentNumLowerIndentedLinesThreshold);

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
 * @param idLength
 *    The length of the block's id to be created.
 * @param formulaIdAlphabet
 *    The characters to use as an identifier for formulas.
 * @param centeringXOverlapRatioThreshold
 *    A parameter used for computing whether or not a text line is centered compared to another
 *    text line. It denotes the minimum ratio by which one of the text line must horizontally
 *    overlap the the other text line, so that the text lines are considered to be centered to each
 *    other. If the maximum x-overlap ratio between both text lines is smaller than this value, the
 *    text lines are considered to be *not* centered.
 * @param centeringXOffsetEqualToleranceFactor
 *    This method returns a factor that is used to compute a threshold (= <factor> *
 *    'average character width in the PDF document') denoting the maximum allowed difference
 *    between the left x-offset and right x-offset of a text line (computed relatively to the
 *    previous text line), so that both offsets are considered to be equal and that the text line
 *    is considered to be centered compared to the previous text line.
 * @param centeringMaxNumJustifiedLines
 *    A parameter that is used for computing whether or not the text lines of a text block are
 *    centered among each other. It denotes the maximum number of justified lines (= lines with a
 *    left margin and right margin == 0) a text block is allowed to contain so that the text lines
 *    are considered to be centered.
 * @param prevTextLineCapacityThresholdFactor
 *    A factor used to compute a threshold that is used for computing whether or not the previous
 *    text line has capacity (the threshold is computed as <factor> * 'avg. character width of the
 *    PDF document'). If the difference between the right margin of the previous line and the
 *    width of the first word of the current text line is larger than this threshold, the previous
 *    line is considered to have capacity. Otherwise, the previous line is considered to have *no*
 *    capacity.
 * @param blocks
 *    The vector to which the created text block should be appended.
 */
// TODO: Document the missing parameters.
void createTextBlock(
    const vector<PdfTextLine*>& lines,
    int idLength,
    const string& formulaIdAlphabet,
    double centeringXOverlapRatioThreshold,
    double centeringXOffsetEqualToleranceFactor,
    int centeringMaxNumJustifiedLines,
    double prevTextLineCapacityThresholdFactor,
    const unordered_set<string>& lastNamePrefixes,
    int hangIndentMinLengthLongLines,
    double hangIndentMinPercLinesSameLeftMargin,
    int hangIndentNumNonIndentedLinesThreshold,
    double hangIndentMarginThresholdFactor,
    int hangIndentNumLowerNonIndentedLinesThreshold,
    int hangIndentNumLongLinesThreshold,
    int hangIndentNumLowerIndentedLinesThreshold,
    double fontSizeEqualTolerance,
    double fontWeightEqualTolerance,
    vector<PdfTextBlock*>* blocks);

}  // namespace text_blocks_utils

#endif  // UTILS_TEXTBLOCKSUTILS_H_
