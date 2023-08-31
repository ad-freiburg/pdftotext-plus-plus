/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_XYCUT_H_
#define UTILS_XYCUT_H_

#include <functional>  // std::function
#include <vector>

#include "../PdfDocument.h"

using std::vector;

using ppp::types::Cut;
using ppp::types::PdfElement;

// =================================================================================================

namespace ppp::utils {

/**
 * A wrapper for a function that needs to be passed to the xCut() and yCut() methods of the `XYCut`
 * class. The function is supposed to choose those cuts from the given vector of cut candidates,
 * which should be actually used to divide given elements into sub-groups.
 *
 * The motivation behind this wrapper is that, under the hood, the `PageSegmentator` class and
 * `ReadingOrderDetector` class use the same XY-cut algorithm, only differing in the cut choosing
 * strategies. Thanks to the wrapper, we do not have to implement the logic behind the XY-cut
 * algorithm twice, but can pass different functions, implementing different cut choosing
 * strategies, to the xCut() and yCut() methods (so that the logic of xCut() and yCut() can be
 * re-used).
 *
 * For each given cut candidate, the function is supposed to set the isChosen property to true, if
 * the cut should actually be used to divide the elements.
 *
 * @param candidates
 *   The cut candidates computed by the XY-cut algorithm. For each candidate, the function is
 *   supposed to set the isChosen property to true, if the cut should actually be used.
 * @param elements
 *   The elements to divide into groups.
 * @param silent
 *    Whether or not the function should output debug information to the console.
 *    NOTE: We introduced this flag because we use the xCut() and yCut() methods also for
 *    lookaheads. For example, one possible cut choosing strategy is to choose a y-cut iff it
 *    enables the option for another, subsequent x-cut (in which case a lookahead is required to
 *    check if a subsequent x-cut is actually possible). We do not want to output the debug
 *    information of the lookaheads, since it would blow up the log, without printing essential
 *    information. Setting this parameter to true suppresses the debug information, setting it to
 *    false prints the debug information.
 */
typedef std::function<void(const vector<Cut*>& candidates, const vector<PdfElement*>& elements,
    bool silent)> ChooseCutsFunc;

// =================================================================================================

/**
 * This method recursively divides the given PDF elements (characters, word, figures, shapes, etc.)
 * into smaller groups by x-cuts and/or y-cuts.
 *
 * An x-cut is a vertical line that divides the elements into a left half and a right half.
 * An y-cut is a horizontal line that divides the elements into an upper half and a lower half.
 *
 * In each recursion step, this method first tries to divide the elements alternately by x-cuts and
 * y-cuts, starting with x-cuts. When the elements have been successfully divided by x-cuts, the
 * method tries to divide the sub-groups by y-cuts afterwards. When the elements have been
 * successfully divided by y-cuts, the method tries to divide the sub-groups by x-cuts afterwards.
 * The recursion stops if no group can be divided by further x-cuts or y-cuts anymore.
 *
 * How the elements are divided by x-cuts and y-cuts exactly is described in the respective
 * comments of the xCut() and yCut() methods below.
 *
 * @param elements
 *   The elements to divide into groups.
 * @param minXCutGapWidth
 *   The minimum horizontal gap between two elements for considering the position between the
 *   elements as a valid position for an x-cut candidate. See the comment given for the xCut()
 *   method for more details.
 * @param minYCutGapHeight
 *   The minimum vertical gap between two elements for considering the position between the
 *   elements as a valid position for an y-cut candidate. See the comment given for the yCut()
 *   method for more details.
 * @param maxNumOverlappingElements
 *   The maximum number of elements an x-cut is allowed to overlap. This parameter was introdcued
 *   for handling text lines that accidentally extend beyond actual column boundaries, see the
 *   comment given for the xCut() method for more details.
 * @param chooseXCutsFunc
 *   A function that chooses those cuts from computed x-cut candidates that should be actually
 *   used to divide the elements. The function is supposed to set the `isChosen` flag to true
 *   for each chosen cut candidate. See the comment given for the xCut() method for more details.
 * @param chooseYCutsFunc
 *   A function that chooses those cuts from computed y-cut candidates that should be actually
 *   used to divide the elements. The function is supposed to set the `isChosen` flag to true
 *   for each chosen cut candidate. See the comment given for the yCut() method for more details.
 * @param silent
 *   Whether or not subsequent calls to the xCut()- and yCut()-methods should output debug
 *   information to the console.
 *   NOTE: The number of calls to the xCut()- and yCut()-methods may be large, in particular when
 *   they are used for lookaheads. This parameter enables the option to suppress the output of
 *   debug information for certain calls; e.g., to keep the log readable.
 * @param resultGroups
 *   If specified, this method appends the groups, into which the elements were divided, to this
 *   vector. If the elements were not divided, this vector will contain a copy of `elements`.
 * @param resultCuts
 *   If specified, this method appends the computed cut candidates to this vector. This is
 *   particularly helpful for debugging and visualization purposes.
 */
void xyCut(const vector<PdfElement*>& elements, double minXCutGapWidth, double minYCutGapHeight,
  int maxNumOverlappingElements, const ChooseCutsFunc& chooseXCutsFunc,
  const ChooseCutsFunc& chooseYCutsFunc, bool silent = false,
  vector<vector<PdfElement*>>* resultGroups = nullptr, vector<Cut*>* resultCuts = nullptr);

/**
 * This method divides the given PDF elements (characters, word, figures, shapes, etc.) into
 * smaller groups by one or more x-cuts. By which and how many x-cuts the elements are actually
 * divided, depends on the given `minGapHeight`, the given `maxNumOverlappingElements` and the
 * given `chooseCutsFunc`. This is explained in more detail in the following.
 *
 * The overall division process consists of three steps. The first step is the computation of all
 * x-cut candidates, that is: all cuts with `cut.gapHeight >= minGapHeight` and
 * `cut.overlappingElements.size() <= maxNumOverlappingElements`. To compute the cut candidates,
 * the elements are sorted by their leftX values in ascending order and iterated "from left to
 * right". For each element, the `maxNumOverlappingElements + 1`-th` previous elements with the
 * `maxNumOverlappingElements + 1`-th largest rightX values are considered (sorted by the rightX
 * values in descending order). Those previous elements are iterated from right to left. For each
 * (prevElement, element) pair, the horizontal gap between the two elements is computed. If the
 * horizontal gap is larger than `minGapWidth`, an x-cut candidate positioned between the two
 * elements is created (the iteration through the previous elements stops, and the next element
 * in `elements` is processed).
 *
 * NOTE: Initially, we required `maxNumOverlappingElements == 0`. This is actually a stronger, but
 * reasonable requirement, as long as the layout of a PDF document is well-formed. However, text
 * lines can accidentally extend beyond the actual column boundaries and extend into other columns.
 * If this is the case, dividing the elements into columns would be impossible without allowing for
 * `maxNumOverlappingElement > 0`. Here is an example:
 *
 * AA AA AAA     BB BB BBB
 * AAAA AAAAAAAAAA BBB BBB
 * AA AAA AA     B BBB BBB
 * A AAAA AA     BB BB B B
 * AA AA AAA     BBB BBBBB
 *
 * From a visual perspective, these elements need to be divided into two groups: a group containing
 * the "A-words" of the left column and a group containing the "B-words" of the right column.
 * However, the second line of the left column is accidentally longer than the other lines in the
 * same column, and extend into the other column. Thus, there is no (prevWord, word) pair, with
 * `prevWord` being a part of the left column and `word` being a part of the right column, where
 * the horizontal gap between the words is >= `minGapWidth`. Allowing for
 * maxNumOverlappingElements > 0 enables the option to consider the "AAAAAAAAAA" word as an
 * "overlapping element", and to ignore the word on computing the horizontal gap (instead, the
 * horizontal gap is computed from the previous element with the next larger rightX value).
 *
 * The second step is choosing those x-cuts from the computed candidates that should be actually
 * used to divide the elements. This is done by passing the cut candidates to the given
 * `chooseCutsFunc` function. This function is supposed to set the `isChosen` flag to true for each
 * cut candidate that should be actually used.
 * NOTE: Passing the cut candidates to an extra function enables the option to use the same
 * implementation of the XY-cut algorithm for different purposes, using different cut choosing
 * strategies. For example, the XY-cut algorithm is used by the `PageSegmentator` class and the
 * `ReadingOrderDetector` class. Both use different cut choosing strategies by passing different
 * `chooseCutsFunc` functions.
 *
 * The third step is to divide the elements into groups at the chosen cut candidates. If specified,
 * the groups are appended to the given `resultGroups` vector. If no cut candidate was chosen, a
 * copy of `elements` is appended instead.
 *
 * @param elements
 *   The elements to divide into groups.
 * @param minGapWidth
 *   The minimum horizontal gap between two elements for considering the position between the two
 *   elements as a valid position for an x-cut candidate.
 * @param maxNumOverlappingElements
 *   The maximum number of elements a cut candidate is allowed to overlap.
 * @param chooseCutsFunc
 *   A function that chooses those cuts from the computed cut candidates that should be actually
 *   used to divide the elements. The function is supposed to set the `isChosen` flag to true
 *   for each chosen cut candidate.
 * @param silent
 *   Whether or not this method should output debug information to the console.
 *   NOTE: The number of calls to this method may be large, in particular when it is used for
 *   lookaheads. This parameter enables the option to suppress the output of debug information
 *   for certain calls; e.g., to keep the log readable.
 * @param resultGroups
 *   If specified, this method appends the groups, into which the elements were divided, to this
 *   vector. If the elements were not divided, this vector will contain a copy of `elements`.
 * @param resultCuts
 *   If specified, this method appends the computed cut candidates to this vector. This is
 *   particularly helpful for debugging and visualization purposes.
 *
 * @return
 *   True, if there is at least one chosen cut candidate, and the elements were divided into two
 *   or more groups; false otherwise.
 */
bool xCut(const vector<PdfElement*>& elements, double minGapWidth, int maxNumOverlappingElements,
  const ChooseCutsFunc& chooseCutsFunc, bool silent,
  vector<vector<PdfElement*>>* resultGroups = nullptr, vector<Cut*>* resultCuts = nullptr);

/**
 * This method divides the given PDF elements (characters, word, figures, shapes, etc.) into
 * smaller groups by one or more y-cuts. By which and how many y-cuts the elements are actually
 * divided, depends on the given `minGapHeight`, and the given `chooseCutsFunc`. This is explained
 * in more detail in the following.
 *
 * The overall division process consists of three steps. The first step is the computation of all
 * y-cut candidates, that is: all cuts with `cut.gapHeight >= minGapHeight`. To compute the cut
 * candidates, the elements are sorted by their upperY values in ascending order and iterated "from
 * top to bottom". For each element, the vertical gap between the element and the previous
 * element with the largest lowerY is computed. If the vertical gap is larger than `minGapHeight`,
 * an y-cut candidate positioned between the two elements is created.
 *
 * The second step is choosing those y-cuts from the computed candidates that should be actually
 * used to divide the elements. This is done by passing the cut candidates to the given
 * `chooseCutsFunc` function. This function is supposed to set the `isChosen` flag to true for each
 * cut candidate that should be actually used.
 * NOTE: Passing the cut candidates to an extra function enables the option to use the same
 * implementation of the XY-cut algorithm for different purposes, using different cut choosing
 * strategies. For example, the XY-cut algorithm is used by the `PageSegmentator` class and the
 * `ReadingOrderDetector` class. Both use different cut choosing strategies by passing different
 * `chooseCutsFunc` functions.
 *
 * The third step is to divide the elements into groups at the chosen cut candidates. If specified,
 * the groups are appended to the given `resultGroups` vector. If no cut candidate was chosen, a
 * copy of `elements` is appended instead.
 *
 * @param elements
 *   The elements to divide into groups.
 * @param minGapHeight
 *   The minimum vertical gap between two elements for considering the position between the two
 *   elements as a valid position for an y-cut candidate.
 * @param chooseCutsFunc
 *   A function that chooses those cuts from the computed cut candidates that should be actually
 *   used to divide the elements. The function is supposed to set the `isChosen` flag to true
 *   for each chosen cut candidate.
 * @param silent
 *   Whether or not this method should output debug information to the console.
 *   NOTE: The number of calls to this method may be large, in particular when it is used for
 *   lookaheads. This parameter enables the option to suppress the output of debug information
 *   for certain calls; e.g., to keep the log readable.
 * @param resultGroups
 *   If specified, this method appends the groups, into which the elements were divided, to this
 *   vector. If the elements were not divided, this vector will contain a copy of `elements`.
 * @param resultCuts
 *   If specified, this method appends the computed cut candidates to this vector. This is
 *   particularly helpful for debugging and visualization purposes.
 *
 * @return
 *   True, if there is at least one chosen cut candidate, and the elements were divided into two
 *   or more groups; false otherwise.
 */
bool yCut(const vector<PdfElement*>& elements, double minGapHeight,
  const ChooseCutsFunc& chooseCutsFunc, bool silent,
  vector<vector<PdfElement*>>* resultGroups = nullptr, vector<Cut*>* resultCuts = nullptr);

}  // namespace ppp::utils

#endif  // UTILS_XYCUT_H_
