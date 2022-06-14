/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef XYCUT_H_
#define XYCUT_H_

#include <functional>  // std::function
#include <vector>

#include "./PdfDocument.h"

using std::vector;

// * used for page segmentation; detecting columns
// * recursively divide the given elements into smaller subgroups, by x-cuts and y-cpu_set_t
// * explain x-cuts and y-cuts
// * explain the concept: computing candidates; candidates are chosen by the PageSegmentator and
//   ReadingOrderDetector.

// =================================================================================================

/**
 * A wrapper for the function that needs to be passed to the xCut() and yCut() methods below. The
 * wrapped function is supposed to choose those cuts from a given vector of candidate cuts, which
 * should be actually made on segmenting a given vector of elements.
 *
 * The motivation behind this wrapper is that the `PageSegmentator` class and `ReadingOrderDetector`
 * class use the same XYcut algorithm under the hood, but with different cut choosing algorithms.
 * Thanks to the  wrapper, we do not have to implement the logic behind XY-cut twice, but can pass
 * different functions, implementing different cut choosing strategies, to the xCut() and yCut()
 * algorithm (so that the logic of xCut() and yCut() can be re-used).
 *
 * For each given cut candidate, the function is supposed to set the isChosen property to true, if
 * the cut should actually made, and set to false otherwise.
 *
 * @param candidates
 *   The cut candidates computed by the XY-cut algorithm. For each candidate, the function is
 *   supposed to set the isChosen property to true, if the cut should actually made, and set to
 *   false otherwise.
 * @param elements
 *   The elements to segment (and on the basis of which the cut candidates were computed).
 * @param silent
 *    Whether or not the function should output debug information to the console.
 *    NOTE: We introduced this flag because we use the xCut() and yCut() methods also for
 *    lookaheads. For example, one possible strategy is to choose a y-cut when it allows another,
 *    subsequent x-cut (in which case a lookeahead is required to check if a subsequent y-cut
 *    is actually possible). We usually do not want to output the debug information of the function
 *    if it is used in a lookahead, since it would mess up the log.
 *    Set this parameter to true to suppress the debug information, and to false to not suppress
 *    the debug information.
 */
typedef std::function<void(const vector<Cut*>& candidates, const vector<PdfElement*>& elements,
    bool silent)> ChooseCutsFunc;

// =================================================================================================

/**
 * This method recursively divides the given elements, which can consist of text elements (like
 * characters or words) and non-text elements (like figures and shapes) into smaller (sub-)groups.
 *
 * At each recursion step, it tries to first divide the elements by one or more vertical cuts (also
 * called "x-cuts", because the lines are "moved" through the elements in x-direction in order to
 * find cut positions) and then by one or more horizontal lines (also called "y-cuts", because they
 * are moved to the elements in y-direction). This process is repeated recursively until no group
 * can be divided further by an x-cut or y-cut.
 *
 * How the x-cuts and y-cuts are computed exactly is described in the respective comments of the of
 * the xCut() and yCut() methods below.
 *
 * @param elements
 *   The elements to divide into groups.
 * @param chooseXCutsFunc
 *   A function that chooses those cuts from a given vector of x-cuts that should be actually made.
 * @param chooseYCutsFunc
 *   A function that chooses those cuts from a given vector of y-cuts that should be actually made.
 * @param minXCutGapWidth
 *   For a given x-cut c, the minimum value for `c.gapWidth` in order to be considered as a cut
 *   candidate.
 * @param minYCutGapHeight
 *   For a given y-cut c, the minimum value for `c.gapHeight` in order to be considered as a cut
 *   candidate.
 * @param maxNumOverlappingElements
 *   The maximum number of elements a cut candidate is allowed to overlap. TODO: Explain better.
 * @param silent
 *    Whether or not subsequent calls to the xCut()- and yCut()-methods should output debug
 *    information to the console.
 *    NOTE: The number of calls to the xCut()- and yCut()-methods may be large, in particular when
 *    the methods are used for "looking ahead". This parameter allows to manually suppress the
 *    output of debug information of certain calls; e.g., to keep the log readable.
 * @param resultGroups
 *   The vector to which the groups which can't be divided further (by an x-cut or y-cut) should be
 *   appended.
 * @param resultCuts
 *   If specified, this method adds the chosen cuts to this vector, each together with
 *   positional information. This is particularly helpful for debugging and visualization purposes.
 */
void xyCut(const vector<PdfElement*>& elements, double minXCutGapWidth, double minYCutGapHeight,
  int maxNumOverlappingElements, const ChooseCutsFunc chooseXCutsFunc,
  const ChooseCutsFunc chooseYCutsFunc, bool silent, vector<vector<PdfElement*>>* resultGroups,
  vector<Cut*>* resultCuts = nullptr);

/**
 * This method divides the given elements into groups by one or more x-cuts. The basic approach is
 * as follows: TODO
 * First, the elements are sorted by their leftX values and iterated in sorted order (= from left
 * to right). In iteration step i, the pair (elementLargestMaxX, elements[i+1]) is considered,
 * where `elementLargestMaxX` is the element in elements[0..i] with the largest rightX. For
 * each pair, the given `IsValidCutFunc`-function is invoked, with the purpose to find out whether
 * or not the current position is a valid x-cut position (that is: whether or not to divide the
 * elements between elementLargestMaxX and elements[i+1] by a x-cut).
 * When the `IsValidCutFunc` function affirms that the current position is a valid x-cut position,
 * a vector consisting of the elements[prevCutPos..i] is added to `resultGroups`, where
 * `prevCutPos` is the index of the previous x-cut position. The iteration stops when
 * `maxNumXCuts`-many valid x-cuts positions were found.
 *
 * @param elements
 *   The elements to divide into groups.
 * @param chooseCutsFunc
 *   A function that chooses those cuts from a given vector of x-cuts that should be actually made.
 * @param minGapWidth
 *   For a given x-cut c, the minimum value for `c.gapWidth` in order to be considered as a cut
 *   candidate.
 * @param maxNumOverlappingElements
 *   The maximum number of elements a cut candidate is allowed to overlap. TODO: Explain better.
 * @param silent
 *   Whether or not this method should output debug information to the console.
 *   NOTE: The number of calls to this method may be large, in particular when it is used for
 *   "looking ahead". This parameter allows to manually suppress the output of debug information
 *   of certain calls; e.g., to keep the log readable.
 * @param resultGroups
 *   The vector to which the groups which can't be divided further (by an x-cut or y-cut) should be
 *   appended.
 * @param resultCuts
 *   If specified, this method adds the chosen cuts to this vector, each together with
 *   positional information. This is particularly helpful for debugging and visualization purposes.
 *
 * @return True, if the elements were divided into two or more groups; false otherwise.
 */
bool xCut(const vector<PdfElement*>& elements, double minGapWidth, int maxNumOverlappingElements,
  const ChooseCutsFunc chooseCutsFunc, bool silent,
  vector<vector<PdfElement*>>* resultGroups = nullptr, vector<Cut*>* resultCuts = nullptr);

/**
 * TODO
 * This method divides the given elements into groups by one or more x-cuts. The basic approach is
 * as follows: TODO
 * First, the elements are sorted by their leftX values and iterated in sorted order (= from left
 * to right). In iteration step i, the pair (elementLargestMaxX, elements[i+1]) is considered,
 * where `elementLargestMaxX` is the element in elements[0..i] with the largest rightX. For
 * each pair, the given `IsValidCutFunc`-function is invoked, with the purpose to find out whether
 * or not the current position is a valid x-cut position (that is: whether or not to divide the
 * elements between elementLargestMaxX and elements[i+1] by a x-cut).
 * When the `IsValidCutFunc` function affirms that the current position is a valid x-cut position,
 * a vector consisting of the elements[prevCutPos..i] is added to `resultGroups`, where
 * `prevCutPos` is the index of the previous x-cut position. The iteration stops when
 * `maxNumXCuts`-many valid x-cuts positions were found.
 *
 * @param elements
 *   The elements to divide into groups.
 * @param chooseCutsFunc
 *   A function that chooses those cuts from a given vector of x-cuts that should be actually made.
 * @param minGapHeight
 *   For a given y-cut c, the minimum value for `c.gapHeight` in order to be considered as a cut
 *   candidate.
 * @param silent
 *   Whether or not this method should output debug information to the console.
 *   NOTE: The number of calls to this method may be large, in particular when it is used for
 *   "looking ahead". This parameter allows to manually suppress the output of debug information
 *   of certain calls; e.g., to keep the log readable.
 * @param resultGroups
 *   The vector to which the groups which can't be divided further (by an x-cut or y-cut) should be
 *   appended.
 * @param resultCuts
 *   If specified, this method adds the chosen cuts to this vector, each together with
 *   positional information. This is particularly helpful for debugging and visualization purposes.
 *
 * @return True, if the elements were divided into two or more groups; false otherwise.
 */
bool yCut(const vector<PdfElement*>& elements, double minGapHeight,
  const ChooseCutsFunc chooseCutsFunc, bool silent,
  vector<vector<PdfElement*>>* resultGroups = nullptr, vector<Cut*>* resultCuts = nullptr);

#endif  // XYCUT_H_



// * @param cutPos
//  *   The index identifying the position in `elements` for which to decide whether or not it is a
//  *   valid cut position.
//  * @param closestElementAlreadySeen
//  *   The element in elements[0..cutPos] with the largest rightX when this function is passed
//  *   to the xCut() method; or the largest lowerY when it passed to the yCut() method.
//  *   NOTE: This element is usually *not* equal to elements[cutPos-1], because the elements are
//  *   *not* sorted by rightX values (resp. lowerY values), but by their leftX values (resp.
// upperY values).

