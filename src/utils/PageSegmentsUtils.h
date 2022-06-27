/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_PAGESEGMENTSUTILS_H_
#define UTILS_PAGESEGMENTSUTILS_H_

#include <tuple>

#include "../PdfDocument.h"

using std::tuple;

// =================================================================================================
// CONFIG

namespace page_segment_utils::config {

// A parameter used for computing the trim box of a segment. It denotes the precision to use when
// rounding the rightX values of the text lines of the segment before computing the most frequent
// rightX value.
const double TRIM_BOX_COORDS_PREC = 0;

// A parameter in [0, 1] used for computing the trim box of a segment. It denotes the minimum
// percentage of text lines in a given segment that must exhibit the most frequent rightX so that
// this rightX is considered to be the rightX of the trim box of the segment.
const double MIN_PERC_LINES_SAME_RIGHT_X = 0.5;

}  // namespace page_segment_utils::config

// =================================================================================================


/**
 * A collection of some useful and commonly used functions in context of page segments.
 */
namespace page_segment_utils {

/**
 * This method computes the trim box of the given page segment, that is: the bounding box around
 * the lines of the segment that do not extend beyond the actual segment boundaries. Here is an
 * example for illustration purposes:
 *
 * AAAAAAA   BBBBBBB
 * AAAAAAA   XXXXXXXXXX
 * AAAAAAA   BBBBBBB
 * AAAAAAA   BBBBBBB
 * AAAAAAA   BBBBBBB
 *
 * This should illustrate two segments, both contained in two different columns: the first segment
 * is build by the "AAA..." lines, the second segment is built by the "BBB..." and "XXX..." lines.
 * Note that the "XXX..." line extends beyond the actual boundary of the second segment, as all
 * other lines in the second segment are actually justified and the "XXX..." line is longer than
 * the other lines. The trim box of the second segment is the bounding box around all "BBB..."
 * lines.
 *
 * The motivation behind computing the trim box is to compute the right margins of text lines more
 * accurately (the right margin is needed by, for example, the computeHasPrevLineCapacity() method).
 * Initially, for a segment S and a text line L, we computed the right margin of L by computing
 * the gap between the right boundary of the L and the right boundary of S (that is:
 * S.position.rightX - L.position.rightX). This however resulted in inaccurately computed right
 * margins when there was a text line L' that extended beyond the actual boundaries of S. The
 * reason was that the bounding box of S was broader than expected because of L' and thus, the
 * computed right margin were usually too large. Our new approach is to compute the trim box of
 * S and to compute the right margin of L by computing S.trimRightX - L.position.rightX, where
 * S.trimRightX is the rightX coordinate of the trim box of S.
 *
 * NOTE 1: the decision whether or not a line extends beyond an actual segment boundary can be
 * challenging. For example, in the illustration above, the lines in the second segment could also
 * be left-aligned, with all "BBB..." lines occassionally having the same width and the "XXX..."
 * *not* extending the segment boundary. Our approach is to compute the most frequent rightX
 * among the text lines in the segment. If at least half of the lines of the segment exhibit the
 * most frequent rightX, we assume that this value represents the rightX of the segment's trim box.
 *
 * NOTE 2: Until now, only the rightX of the trim box is actually computed. The returned leftX,
 * upperY and lowerY are equal to those of the bounding box of the segment. This is because
 * text lines usually extend only beyond the right boundary of a segment.
 *
 * @param segment
 *    The segment for which to compute the trim box.
 *
 * @return
 *    The leftX, upperY, rightX, and lowerY of the computed trim box.
 */
tuple<double, double, double, double> computeTrimBox(const PdfPageSegment* segment);

}  // namespace page_segment_utils

#endif  // UTILS_PAGESEGMENTSUTILS_H_
