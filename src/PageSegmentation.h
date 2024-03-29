/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef PAGESEGMENTATION_H_
#define PAGESEGMENTATION_H_

#include <vector>

#include "./Config.h"
#include "./Types.h"
#include "./utils/Log.h"
#include "./utils/PageSegmentationUtils.h"
#include "./utils/Trool.h"

using std::vector;

using ppp::config::PageSegmentationConfig;
using ppp::types::Cut;
using ppp::types::PdfDocument;
using ppp::types::PdfElement;
using ppp::types::PdfPage;
using ppp::types::PdfPageSegment;
using ppp::utils::PageSegmentationUtils;
using ppp::utils::Trool;
using ppp::utils::log::Logger;

// =================================================================================================

namespace ppp::modules {

/**
 * This class is responsible for dividing the pages of a given PDF document into segments, by using
 * the recursive XY-cut algorithm. Each page is segmented separately. The input are the words,
 * figures, graphics, and shapes of a page. The output is a vector of `PdfPageSegment` objects.
 * The purpose of the segmentation is to separate the elements of different columns. Each segment
 * contains the elements of a single column (no segment contains text from two columns).
 * The segmentation is a preprocessing step for detecting text lines and detecting text blocks.
 */
class PageSegmentation {
 public:
  /**
   * The default constructor.
   *
   * @param doc
   *   The PDF document to process.
   * @param config
   *   The configuration to use.
   */
  PageSegmentation(PdfDocument* doc, const PageSegmentationConfig* config);

  /** The deconstructor. **/
  ~PageSegmentation();

  /**
   * This method starts the segmentation process. The given PDF document is processed page-wise.
   * For each page, the words, figures, graphics and shapes contained in the page are divided into
   * segments, by using the recursive XY-cut algorithm. The segments created from the i-th page are
   * appended to _doc->pages[i]->segments.
   */
  void process();

 private:
  /**
   * This method divides the words, figures, graphics and shapes contained in the given page into
   * segments, by using the recursive XY-cut algorithm. The crated segments are appended to the
   * given vector.
   *
   * @param page
   *    The page to segment.
   * @param segments
   *    The vector to which the created segments should be appended.
   */
  void processPage(PdfPage* page, vector<PdfPageSegment*>* segments);

  /**
   * This method chooses the x-cut candidates that should be actually used to divide the given
   * elements into segments. The candidates are chosen depending on certain layout features, for
   * example: the gap width and gap height of the cuts, or the widths of the resulting element
   * groups. For each chosen cut candidate, this method sets the `isChosen` flag to true.
   *
   * NOTE: This function is required on invoking the xCut() method of the XY-cut algorithm.
   *
   * @param candidates
   *   The x-cut candidates computed by the XY-cut algorithm, from which to choose the cuts that
   *   should be actually used to divide the elements into segments.
   * @param elements
   *   The elements to divide by the x-cuts.
   * @param silent
   *   Whether or not this method should output debug information to the console.
   *   NOTE: We introduced this flag because the xCut() method is also used for lookaheads. For
   *   example, one possible cut choosing strategy is to choose a y-cut iff it enables the option
   *   for another, subsequent x-cut (in which case a lookahead is required to check if a
   *   subsequent x-cut is actually possible). Since the debug information produced by the
   *   lookaheads can be confusing, it can be suppressed by using this parameter. Setting this
   *   parameter to true suppresses the debug information, setting it to false prints the debug
   *   information.
   */
  void chooseXCuts(const vector<Cut*>& candidates, const vector<PdfElement*>& elements,
      bool silent);

  /**
   * This method checks whether the given x-cut should *not* be chosen, because the cut overlaps at
   * least one element and
   * (a) the number of elements to divide is smaller than a threshold,
   * (b) the top margin of at least one overlapping element (= the distance between the upperY of
   *     the overlapping element and the upperY of the cut) or the bottom margin (= the distance
   *     between the lowerY of the cut and the lowerY of the overlapping element) is smaller than a
   *     threshold.
   *
   * Requirement (a) is there to avoid to split a block with a small number of words, because for
   * such blocks, it is more difficult to decide if a text line extends beyond column boundaries.
   * Requirement (b) is there to avoid to accidentally split headers and footers that are
   * positioned above or below a multi-column layout. Consider the following example:
   *
   * This is a header of page 1.
   * XXXXXXXXXXXX  YYYYYYYYYYY
   * XXXXXXXXXXXX  YYYYYYYYYYY
   * XXXXXXXXXXXX  YYYYYYYYYYY
   * XXXXXXXXXXXX  YYYYYYYYYYY
   *
   * Without requirement (b), the word "header" could be considered as a part of the left column,
   * with extending beyond the column boundaries. This would split the header between the words
   * "header" and "of" (which is if course not the expected output).
   *
   * @param cut
   *    The cut candidate for which to decide whether to choose or not.
   * @param elements
   *    The elements that are divided by the cut.
   * @param silent
   *   Whether or not this method should output debug information to the console.
   *
   * @return
   *    Trool::False if the cut should *not* be chosen.
   *    Trool::None if this method couldn't decide whether the cut should not be chosen (e.g.,
   *      because it does not overlap any elements).
   *    Note that this method never returns Trool::True.
   */
  Trool chooseXCut_overlappingElements(const Cut* cut, const vector<PdfElement*>& elements,
      bool silent = false) const;

  /**
   * This method checks whether the given cut should *not* be chosen, because the gap width *and*
   * gap height of the cut is smaller than a threshold.
   *
   * This should avoid to accidentally split text blocks, that only consists of few text lines,
   * at a position where the whitespaces of two or more text lines occasionally fall together.
   * Here is an example:
   *
   * This is a text block
   * with only two lines.
   *
   * Note that the whitespaces behind "a" and "only" fall together (meaning that they overlap
   * horizontally). If these widths of the whitespaces are large enough, an x-cut at this position
   * may be accidentally chosen.
   *
   * NOTE: In the example above, the gap height requirement alone would be enough to decide to not
   * choose the x-cut. The additional gap *width* requirement exists to choose an x-cut when the
   * width of the whitespace is "large enough". Here is an example:
   *
   * This is the   This is the
   * 1st column.   2nd column.
   *
   * In this case, there are also whitespaces falling together (the whitespaces behind "the" and
   * "column." of the left column). But this time, the widths of the whitespaces are larger (since
   * they represent an actual column boundary).
   *
   * @param cut
   *    The cut candidate for which to decide whether to choose or not.
   * @param silent
   *   Whether or not this method should output debug information to the console.
   *
   * @return
   *    Trool::False if the given cut should not be chosen, because its gap width and gap height are
   *    smaller than the computed threshold; Trool::None otherwise. Note that this method never
   *    returns Trool::True.
   */
  Trool chooseXCut_smallGapWidthHeight(const Cut* cut, bool silent) const;

  /**
   * This method checks whether the given cut should *not* be chosen, because it divides contiguous
   * words. Two words are contiguous, if the one word immediately follows behind the other word in
   * the extraction order and if both words vertically overlap (= they share the same text line).
   *
   * This rule exists to not accidentally divide the words of a title when a word boundary within
   * the title coincide with a column boundary, as shown in the following example:
   *
   * THIS  IS  | THE  TITLE
   *           |
   * XXXXXXXXX | XXXXXXXXXX
   * XXXXXXXXX | XXXXXXXXXX
   * XXXXXXXXX | XXXXXXXXXX
   *
   * @param cut
   *   The cut candidate for which to decide whether to choose or not.
   * @param elements
   *   The elements divided by the cut.
   * @param silent
   *   Whether or not this method should output debug information to the console.
   *
   * @return
   *    Trool::False if the given cut should not be chosen, because it divides two contiguous words;
   *    Trool::None otherwise. Note that this method never returns Trool::True.
   */
  Trool chooseXCut_contiguousWords(const Cut* cut, const vector<PdfElement*>& elements,
      bool silent) const;

  /**
   * This method checks whether the given cut should *not* be chosen, because the width of one of
   * the resulting groups would be smaller than a given threshold. Here are two examples explaining
   * why this rule exists:
   *
   * (1) In a bibliography, there could be a vertical gap between the reference anchors and the
   *     reference bodies, like illustrated in the following:
   *       [1]   W. Smith et al: Lorem ipsum ...
   *       [2]   F. Miller et al: Lorem ipsum ...
   *       [3]   T. Redford et al: Lorem ipsum ...
   *     Of course, the reference anchors ([1], [2], etc.) should *not* be separated from the
   *     reference bodies by an x-cut.
   *
   * (2) A formula could have a numbering, with a (large) vertical gap in between, like
   *     illustrated in the following example:
   *       x + y = z     (1)
   *     The numbering should *not* be separated from the formula by an x-cut.
   *
   * @param prevChosenCut
   *   The previous chosen cut. This is needed to get the leftmost element in the resulting
   *   left group. The leftmost element is needed to compute the width of the resulting left group.
   * @param cut
   *   The cut candidate for which to decide whether to choose or not.
   * @param elements
   *   The elements divided by the given x-cut.
   * @param silent
   *   Whether or not this method should output debug information to the console.
   *
   * @return
   *    Trool::False if the given cut should not be chosen, because the width of one of the
   *    resulting groups is smaller than the treshold; Trool::None otherwise. Note that this
   *    method never returns Trool::True.
   */
  Trool chooseXCut_slimGroups(const Cut* prevChosenCut, const Cut* cut,
      const vector<PdfElement*>& elements, bool silent) const;

  // ===============================================================================================

  /**
   * This method chooses the y-cut candidates that should be actually used to divide the given
   * elements into segments. The candidates are chosen depending on whether or not they allow for
   * subsequent x-cuts. To better understand the idea behind our strategy on choosing y-cuts,
   * consider the following example:
   *
   *   ┌───────────────────────┐                 ┌───────────────────────┐
   *   │ --------------------- │ <- 1            │ --------------------- │ <- 1
   *   │  xxxxxxxxxxxxxxxxxx   │                 │   xxxxxxx  xxxxxxxx   │
   *   │        xxxxxx         │                 │   xxxxxxx  xxxxxxxx   │
   *   │ --------------------- │ <- 2            │   xxxxxxx  xxxxxxxx   │
   *   │   xxxxxxx  xxxxxxxx   │                 │   xxxxxxx  xxxxxxxx   │
   *   │   xxxxxxx  xxxxxxxx   │                 │   xxxxxxx  xxxxxxxx   │
   *   │   xxxxxxx  xxxxxxxx   │                 │   xxxxxxx  xxxxxxxx   │
   *   | --------------------- | <- 3            | --------------------- | <- 2
   *   │   xxxxxxx  xxxxxxxx   │                 │    xxxxxxxxxxxxxxx    │
   *   │   xxxxxxx  xxxxxxxx   │                 │    xxxxxxxxxxxxxxx    │
   *   │ --------------------- │ <- 4            │    xxxxxxxxxxxxxxx    |
   *   │  xxxxxxxxxxxxxxxxxxx  │                 │    xxxxxxxxxxxxxxx    │
   *   │ --------------------- │ <- 5            │ --------------------- │ <- 3
   *   └───────────────────────┘                 └───────────────────────┘
   *
   * This should illustrate two PDF pages, with the x's being some text and the horizontal  "---"
   * lines (the lines labelled with a number at the right margin of each page) being the y-cut
   * candidates computed by the XY-cut algorithm. In the left example, the cuts labelled with 1 and
   * 5 are so called "cut sentinels", which represent the top boundary and the bottom boundary of
   * the page (they are not an actual part of the choosable cuts, their only purpose is to make
   * the implementation more compact and more elegant; more about this later). Intuitively, the
   * candidates 2 and 4 should be chosen, because they separate text that is aligned in a different
   * number of columns (the text above cut 2 is aligned in one column, but the text below the cut
   * in two columns; the text below cut 4 is again aligned in one column).
   * To choose the two cuts, we process the cuts iteratively. For each cut c, we try to find its
   * "partner cut", that is: the furthermost cut d, for which the elements between c and d can be
   * divided by an x-cut. If such a partner cut exists, we choose both c and d.
   *
   * Here is a concrete recipe how we choose the cuts in case of the left page above:
   * We process the cuts iteratively. For each, we iterate the respective subsequent cuts to find
   * the partner cut.
   * For cut 1, we iterate through the subsequent cuts [2,3,4,5] (from top to bottom). Since the
   * elements between cut 1 and cut 2 can't be divided by an x-cut, we can stop searching for a
   * partner of cut 1 (since the elements between cut 1 and cut 2 will remain for each other
   * subsequent cut).
   * We proceed with cut 2, and iterate through the cuts [3, 4, 5]. The elements between cut 2
   * and 3 can be divided by an x-cut, so we proceed with cut 4. The elements between cut 2 and 4
   * can also be divided by an x-cut, so we proceed with cut 5. Since the elements between cut 2
   * and 5 can *not* be divided, the partner of cut 2 is cut 4.
   * We proceed with cuts 4 and 5, for each of which there is no partner cut.
   *
   * For each chosen cut candidate, this method sets the `isChosen` flag to true.
   *
   * NOTE 1: Thanks to the sentinel cuts, it is guaranteed that there is always a potential partner
   * cut, even if there is only one "normal" y-cut candidate. Consider the page on the right.
   * Without the sentinel cuts (cuts 1 and 3), cut 2 would accidentally not be chosen (because
   * there would be no partner cut otherwise).
   *
   * NOTE 2: This function is passed to the yCut() method of the XY-cut algorithm.
   *
   * @param candidates
   *   The y-cut candidates computed by the XY-cut algorithm, from which to choose the cuts that
   *   should be actually used to divide the elements into segments.
   * @param elements
   *   The elements to divide by the y-cuts.
   * @param silent
   *    Whether or not this method should output debug information to the console.
   *    NOTE: We introduced this flag because the xCut() method is also used for lookaheads. For
   *    example, one possible cut choosing strategy is to choose a y-cut iff it enables the option
   *    for another, subsequent x-cut (in which case a lookahead is required to check if a
   *    subsequent x-cut is actually possible). Since the debug information produced by the
   *    lookaheads can be confusing, it can be suppressed by using this parameter. Setting this
   *    parameter to true suppresses the debug information, setting it to false prints the debug
   *    information.
   */
  void chooseYCuts(const vector<Cut*>& candidates, const vector<PdfElement*>& elements,
      bool silent);

  // The document to process.
  PdfDocument* _doc;
  // The configuration to use.
  const PageSegmentationConfig* _config;
  // The page segmentation utils.
  const PageSegmentationUtils* _utils;
  // The logger.
  const Logger* _log;
};

}  // namespace ppp::modules

#endif  // PAGESEGMENTATION_H_
