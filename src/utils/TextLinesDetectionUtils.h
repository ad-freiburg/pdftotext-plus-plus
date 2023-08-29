/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_TEXTLINESDETECTIONUTILS_H_
#define UTILS_TEXTLINESDETECTIONUTILS_H_

#include <tuple>

#include "../Config.h"
#include "../PdfDocument.h"

using std::tuple;

using ppp::config::TextLinesDetectionConfig;
using ppp::types::PdfPage;
using ppp::types::PdfPageSegment;

// =================================================================================================

namespace ppp::utils {

/**
 * A collection of some useful and commonly used functions in context of text lines detection.
 */
class TextLinesDetectionUtils {
 public:
  /**
   * The default constructor.
   *
   * @param config
   *   The configuration to use.
   */
  explicit TextLinesDetectionUtils(const TextLinesDetectionConfig* config);

  /** The deconstructor. */
  ~TextLinesDetectionUtils();

  // ===============================================================================================

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
   * - Previous Sibling Text Line: a text line L is the previous sibling text line of text line M
   *   if
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
  void computeTextLineHierarchy(const PdfPage* page) const;

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
   * This should illustrate two segments, both contained in two different columns: the first
   * segment is build by the "AAA..." lines, the second segment is built by the "BBB..." and
   * "XXX..." lines. Note that the "XXX..." line extends beyond the actual boundary of the second
   * segment, as all other lines in the second segment are actually justified and the "XXX..." line
   * is longer than the other lines. The trim box of the second segment is the bounding box around
   * all "BBB..." lines.
   *
   * The motivation behind computing the trim box is to compute the right margins of text lines
   * more accurately (the right margin is needed by, for example, the computeHasPrevLineCapacity()
   * method). Initially, for a segment S and a text line L, we computed the right margin of L by
   * computing the gap between the right boundary of the L and the right boundary of S (that is:
   * S.position.rightX - L.position.rightX). This however resulted in inaccurately computed right
   * margins when there was a text line L' that extended beyond the actual boundaries of S. The
   * reason was that the bounding box of S was broader than expected because of L' and thus, the
   * computed right margin were usually too large. Our new approach is to compute the trim box of
   * S and to compute the right margin of L by computing S.trimRightX - L.position.rightX, where
   * S.trimRightX is the rightX coordinate of the trim box of S.
   *
   * NOTE 1: the decision whether or not a line extends beyond an actual segment boundary can be
   * challenging. For example, in the illustration above, the lines in the second segment could
   * also be left-aligned, with all "BBB..." lines occassionally having the same width and the
   * "XXX..." *not* extending the segment boundary. Our approach is to compute the most frequent
   * rightX among the text lines in the segment. If at least half of the lines of the segment
   * exhibit the most frequent rightX, we assume that this value represents the rightX of the
   * segment's trim box.
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
  tuple<double, double, double, double> computeTrimBox(const PdfPageSegment* segment) const;

 private:
  // The configuration to use.
  const TextLinesDetectionConfig* _config;
};

}  // namespace ppp::utils

#endif  // UTILS_TEXTLINESDETECTIONUTILS_H_
