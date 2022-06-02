/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef PAGESEGMENTSUTILS_H_
#define PAGESEGMENTSUTILS_H_

#include <tuple>

#include "../PdfDocument.h"

using std::tuple;

// =================================================================================================

namespace page_segment_utils {

/**
 * This method computes the trim box of the given page segment, that is: the bounding box around
 * the lines of the segment that do not extend beyond the actual segment boundaries. Here is an
 * example:
 *
 * AAAAAAA   BBBBBBB
 * AAAAAAA   XXXXXXXXXX
 * AAAAAAA   BBBBBBB
 * AAAAAAA   BBBBBBB
 * AAAAAAA   BBBBBBB
 *
 * This should illustrate two segments A and B, contained in two different columns. Note that the
 * second line of the segment B (the "XXX..." line) extends beyond the actual boundary of the
 * second segment, as the lines in the second segment are actually justified and the second line is
 * longer than all other lines of the segment. The trim box of segment B is the bounding box around
 * all "BBB..." lines.
 *
 * The motivation behind computing the trim box is to compute the right margins of text lines more
 * accurately (the right margins are needed by, for example, the computeHasPrevLineCapacity()
 * method). Initially, for a segment S and a text line L (with L being a part of S), we computed
 * the right margin of L by computing S.position.rightX - L.position.rightX. This however resulted
 * in inaccurate right margins when there was a text line L' that extended beyond the actual
 * boundaries of S. The reason was that the bounding box of S was broader than expected because of
 * L' and thus, the computed right margin were usually too large. Our new approach is to compute
 * the right margin of L by computing S.trimRightX - L.pos.rightX, where S.trimRightX is the
 * rightX coordinate of the trim box of S.
 *
 * Note that the decision whether or not a line extends beyond a segment boundary can be
 * challenging. For example, in the illustration above, the lines in the second segment could also
 * be left-aligned, with all "BBB..." lines occassionally having the same width and the "XXX..."
 * *not* extending the segment boundary. Our approach is to compute the most frequent rightX
 * value among the text lines in the segment. If at least half of the lines in the segment exhibits
 * this value as its rightX value, we assume that this value represents the rightX value of the
 * trim box of the segment.
 *
 * NOTE: Until now, only the rightX value of the trim box is actually computed. The returned leftX,
 * upperY and lowerY values are equal to those of the bounding box of the segment. This is because
 * text lines usually extend only beyond the right boundary of a segment.
 *
 * @param segment
 *    The segment for which to compute the trim box.
 *
 * @return
 *    The leftX, upperY, rightX, and lowerY coordinates of the computed trim box.
 */
tuple<double, double, double, double> computeTrimBox(const PdfPageSegment* segment);

}  // namespace page_segment_utils

#endif  // PAGESEGMENTSUTILS_H_
