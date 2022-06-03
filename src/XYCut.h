/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef XYCUT_H_
#define XYCUT_H_

#include <functional>
#include <vector>

#include "./PdfDocument.h"


/**
 * TODO This typedef defines a declaration for a function that needs to be passed to the xCut() and
 * yCut() methods below. It returns true, if the position between `closestElementAlreadySeen` and
 * elements[i] denotes a valid position for dividing the elements by an x-cut (if the function is
 * passed to the xCut() method) or a y-cut (if the function is passed to the yCut() method).
 * Otherwise (if the position does not denote a valid cut position) this method returns false.
 *
 * @param elements
 *   The elements that should be divided by an x-cut or y-cut.
 * @param cutPos
 *   The index identifying the position in `elements` for which to decide whether or not it is a
 *   valid cut position.
 * @param closestElementAlreadySeen
 *   The element in elements[0..cutPos] with the largest rightX when this function is passed
 *   to the xCut() method; or the largest lowerY when it passed to the yCut() method.
 *   NOTE: This element is usually *not* equal to elements[cutPos-1], because the elements are
 *   *not* sorted by rightX values (resp. lowerY values), but by their leftX values (resp. upperY values).
 * @return
 *   True if the position between `closestElementAlreadySeen` and `elements[cutPos]` denotes a
 *   valid cut position, false otherwise.
 */
typedef std::function<void(const std::vector<PdfElement*>& elements, std::vector<Cut*>& cuts,
    bool silent)> ChooseCutsFunc;

// =================================================================================================

/**
 * TODO This method recursively divides the given elements, which can consist of text elements (like
 * glyphs or words) and non-text elements (like figures and shapes) into (sub-)groups.
 * At each recursion step, it tries to first divide the elements by vertical lines (also called
 * "x-cuts") and then by horizontal lines (also called "y-cuts"). This process is repeated
 * recursively until no group can be divided further by an x-cut or y-cut.
 *
 * For more information about how the division by x-cuts and y-cuts works exactly, see the
 * information given in the comments of the xCut() and yCut() methods below.
 *
 * @param elements
 *   The elements to divide into groups.
 * @param isValidXCutFunc
 *   The function telling this method whether or not a given position in `elements` is a valid
 *   position for dividing the elements by an x-cut.
 * @param isValidYCutFunc
 *   The function telling this method whether or not a given position in `elements` is a valid
 *   position for dividing the elements by an x-cut.
 * @param maxNumXCutsPerStep
 *   The max number of x-cuts to do in each recursion step. Does *all* valid x-cuts if set to -1.
 * @param maxNumYCutsPerStep
 *   The max number of y-cuts to do in each recursion step. Does *all* valid x-cuts if set to -1.
 * @param resultGroups
 *   The vector to which the groups which can't be divided further should be appended.
 * @param resultCuts
 *   If specified, this method adds the cuts made to divide the elements to this vector, each
 *   together with its x,y-coordinates. This is particularly helpful for debugging and
 *   visualization purposes.
 */
void xyCut(const std::vector<PdfElement*>& elements, const ChooseCutsFunc chooseXCutsFunc,
  const ChooseCutsFunc chooseYCutsFunc, double minXCutGapWidth, double minYCutGapHeight,
  int maxNumCuttingElements, bool silent,
  std::vector<std::vector<PdfElement*>>* resultGroups, std::vector<Cut*>* resultCuts = nullptr);

/**
 * TODO This method divides the given elements into groups by x-cuts. The basic approach is as follows:
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
 *   The elements to divide by x-cuts.
 * @param isValidXCutFunc
 *   The function telling this method whether or not a given position in `elements` is a valid
 *   position for dividing the elements by an x-cut.
 * @param maxNumXCuts
 *   The maximum number of x-cuts to do. Aborts the iteration through the elements when
 *   `maxNumXCuts`-many cuts were made. Set it to -1 to do *all* valid x-cuts.
 * @param resultGroups
 *   The vector to which the result groups should be appended.
 * @param resultCuts
 *   If specified, this method adds the cuts made to divide the elements to this vector, each
 *   together with its x,y-coordinates.
 *
 * @return True, if the elements were divided into two or more groups; false otherwise.
 */
bool xCut(const std::vector<PdfElement*>& elements, const ChooseCutsFunc chooseCutsFunc,
  double minGapWidth, int maxNumCuttingElements, bool silent,
  std::vector<std::vector<PdfElement*>>* resultGroups = nullptr,
  std::vector<Cut*>* resultCuts = nullptr);

/**
 * TODO This method divides the given elements into groups by y-cuts. The basic approach is as follows:
 * First, the elements are sorted by their upperY values and iterated in sorted order (= from top
 * to bottom). In iteration step i, the pair (elementLargestMaxY, elements[i+1]) is considered,
 * where `elementLargestMaxY` is the element in elements[0..i] with the largest lowerY. For
 * each pair, the given `IsValidCutFunc`-function is invoked, with the purpose to find out whether
 * or not the current position is a valid y-cut position (that is: whether or not to divide the
 * elements between elementLargestMaxY and elements[i+1] by a y-cut).
 * When the `IsValidCutFunc` function affirms that the current position is a valid y-cut position,
 * a vector consisting of the elements[prevCutPos..i] is added to `resultGroups`, where
 * `prevCutPos` is the index of the previous y-cut position. The iteration stops when
 * `maxNumYCuts`-many valid y-cuts positions were found.
 *
 * @param elements
 *   The elements to divide by y-cuts.
 * @param isValidYCutFunc
 *   The function telling this method whether or not a given position in `elements` is a valid
 *   position for dividing the elements by an y-cut.
 * @param maxNumYCuts
 *   The maximum number of y-cuts to do. Aborts the iteration through the elements when
 *   `maxNumYCuts`-many cuts were made. Set it to -1 to do *all* valid y-cuts.
 * @param resultGroups
 *   The vector to which the result groups should be appended.
 * @param resultCuts
 *   If specified, this method adds the cuts made to divide the elements to this vector, each
 *   together with its x,y-coordinates.
 *
 * @return True, if the elements were divided into two or more groups; false otherwise.
 */
bool yCut(const std::vector<PdfElement*>& elements, const ChooseCutsFunc chooseCutsFunc,
  double minGapHeight, bool silent,
  std::vector<std::vector<PdfElement*>>* resultGroups = nullptr,
  std::vector<Cut*>* resultCuts = nullptr);

#endif  // XYCUT_H_
