/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TEXTBLOCKSUTILS_H_
#define TEXTBLOCKSUTILS_H_

#include <string>
#include <unordered_set>
#include <vector>

#include "../PdfDocument.h"

using std::string;
using std::unordered_set;

/**
 * A set of common last name prefixes. This is used for checking if a given block is in hanging
 * indent format. Normally, a block is not considered as a block in hanging indent when it contains
 * one or more unindented lines that start with a lowercase character. However, there are references
 * in hanging indent format that start with a (lowercased) last name prefix. To not accidentally
 * consider a block to be not in hanging indent format when it contains a line starting with
 * such prefix, we do not count a line as a lowercased line if it starts with a word contained in
 * this set.
 */
const unordered_set<string> LAST_NAME_PREFIXES = { "van", "von", "de" };

// =================================================================================================

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
 *      (b) one line completely overlaps the other line horizontally
 * (2) There is at least one line (which does not represent a display formula) for which the leftX
 *     offset (or rightX offset) is larger than a given threshold
 * (3) The number of justified text lines (lines with leftX offset == rightX offset == 0) is
 *     smaller than a given threshold.
 *
 * @param block
 *    The text block to process.
 *
 * @return
 *    True if the lines contained in the given text block are centered; false otherwise.
 */
bool computeIsTextLinesCentered(const PdfTextBlock* block);

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
 *     to the most frequent left margin (which is computed among the lines in the block with a left
 *     margin larger than a threshold).
 * (3) The block does not contain any non-indented line that starts with a lowercase character.
 *
 * Additionally, one of the following requirements must be fulfilled:
 * (a) The first line is not indented, but all other lines, and the first line has no capacity.
 *     This should identify single enumeration items, e.g., in the format:
 *       Dynamics: The low energy behavior of
 *         a physical system depends on its
 *         dynamics.
 * (b) The number of non-indented lines exceeds a given threshold, and all non-indented lines start
 *     with an uppercase character.
 * (c) There is at least one indented line that start with an uppercase character, and the number
 *     of lines exceeds a given threshold.
 *
 * @param block
 *    The text block to process.
 *
 * @return
 *    A value > 0 if the block is in hanging indent format, and 0.0 otherwise.
 */
double computeHangingIndent(const PdfTextBlock* block);

/**
 * This method computes the left and right margins of the text lines in the given block.
 *
 * For a text line L and a text block B, the left margin of L is computed as: L.leftX - B.leftX.
 * The right margin is computed as: B.rightX - L.rightX.
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

#endif  // TEXTBLOCKSUTILS_H_
