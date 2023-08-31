/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // std::sort
#include <limits>  // std::numeric_limits
#include <vector>

#include "./Comparators.h"
#include "./FixedCapacityPriorityQueue.h"
#include "./MathUtils.h"
#include "./PdfElementsUtils.h"
#include "./TextUtils.h"
#include "./XYCut.h"
#include "../Types.h"

using std::numeric_limits;
using std::sort;
using std::vector;

using ppp::types::Cut;
using ppp::types::CutDir;
using ppp::types::PdfElement;
using ppp::utils::comparators::LeftXAscComparator;
using ppp::utils::comparators::RightXDescComparator;
using ppp::utils::comparators::UpperYAscComparator;
using ppp::utils::elements::computeHorizontalGap;
using ppp::utils::elements::computeVerticalGap;
using ppp::utils::math::equal;
using ppp::utils::math::equalOrLarger;
using ppp::utils::math::larger;
using ppp::utils::math::maximum;
using ppp::utils::math::minimum;
using ppp::utils::text::createRandomString;


// =================================================================================================

namespace ppp::utils {

// _________________________________________________________________________________________________
void xyCut(const vector<PdfElement*>& elements,
    double minXCutGapWidth, double minYCutGapHeight, int maxNumOverlappingElements,
    const ChooseCutsFunc& chooseXCutsFunc, const ChooseCutsFunc& chooseYCutsFunc, bool silent,
    vector<vector<PdfElement*>>* resultGroups, vector<Cut*>* resultCuts) {
  // Do nothing if no elements are given.
  if (elements.empty()) {
    return;
  }

  // Check if the group of elements can be divided into sub-groups by one or more x-cuts.
  vector<vector<PdfElement*>> xGroups;
  bool ok = xCut(elements, minXCutGapWidth, maxNumOverlappingElements,
      chooseXCutsFunc, silent, &xGroups, resultCuts);

  if (!ok) {
    // The group could not be divided by x-cuts. Try to divide it by y-cuts.
    vector<vector<PdfElement*>> yGroups;
    ok = yCut(elements, minYCutGapHeight, chooseYCutsFunc, silent, &yGroups, resultCuts);

    if (!ok) {
      // The group could also not be divided by y-cuts. Add the group to the result vector.
      resultGroups->push_back(elements);
      return;
    }

    // The group could be divided by y-cuts. Try to further divide each sub-group recursively.
    for (const auto& yGroup : yGroups) {
      xyCut(yGroup, minXCutGapWidth, minYCutGapHeight, maxNumOverlappingElements,
        chooseXCutsFunc, chooseYCutsFunc, silent, resultGroups, resultCuts);
    }

    return;
  }

  // The group could be divided into sub-groups by x-cuts. Try to divide each sub-group by y-cuts.
  for (const auto& xGroup : xGroups) {
    vector<vector<PdfElement*>> yGroups;
    ok = yCut(xGroup, minYCutGapHeight, chooseYCutsFunc, silent, &yGroups, resultCuts);

    if (!ok) {
      // The group could *not* be further divided by y-cuts. So add the group to the result vector.
      resultGroups->push_back(xGroup);
      continue;
    }

    // The elements could be divided by y-cuts. Try to further divide each sub-group recursively.
    for (const auto& yGroup : yGroups) {
      xyCut(yGroup, minXCutGapWidth, minYCutGapHeight, maxNumOverlappingElements,
        chooseXCutsFunc, chooseYCutsFunc, silent, resultGroups, resultCuts);
    }
  }
}

// _________________________________________________________________________________________________
bool xCut(const vector<PdfElement*>& elements, double minGapWidth, int maxNumOverlappingElements,
    const ChooseCutsFunc& chooseCutsFunc, bool silent, vector<vector<PdfElement*>>* resultGroups,
    vector<Cut*>* resultCuts) {
  // Do nothing if no elements are given.
  if (elements.empty()) {
    return false;
  }

  // Sort the elements by their leftX values, in ascending order.
  vector<PdfElement*> sElements = elements;
  sort(sElements.begin(), sElements.end(), LeftXAscComparator());

  // Compute minY and maxY among the elements, needed for computing the y-coordinates of the cuts.
  double elementsMinY = numeric_limits<double>::max();
  double elementsMaxY = numeric_limits<double>::min();
  for (const auto* element : sElements) {
    elementsMinY = minimum(elementsMinY, element->pos->upperY);
    elementsMaxY = maximum(elementsMaxY, element->pos->lowerY);
  }

  // Create a fixed-size queue for storing the elements with the <maxNumOverlappingElements + 1>-th
  // largest rightX values seen so far while iterating through the elements (from left to right).
  // The queue is a min-PQ, meaning that the element with the smallest rightX appears at the top of
  // the queue. This makes it easier to check if the current element needs to be inserted into the
  // PQ (because its rightX is larger than the smallest rightX in the PQ), and to remove the
  // element with the smallest rightX when the size of the PQ exceeds its capacity after inserting
  // a new element.
  // NOTE: The elements in this queue are used for computing the horizontal gap width between the
  // element currently processed and a previous element stored in the queue.
  // If maxNumOverlappingElements == 0 (meaning that a cut is not allowed to overlap any element),
  // this queue contains exactly one element (the element with the largest rightX seen before the
  // current element). The gap width is computed as (E.leftX - queue.top().rightX), where `E` is
  // the element currently processed. If the gap width is >= minGapWidth, a cut candidate dividing
  // the elements between queue.top() and E is created. Otherwise, the same procedure is repeated
  // for the next element.
  // If maxNumOverlappingElements > 0 (meaning that a cut is allowed to overlap
  // <maxNumOverlappingElements>-many elements), the elements in the queue are iterated in
  // reversed order (starting at the element with the largest rightX value). For each element Q
  // in the queue the gap width (E.leftX - Q.rightX) is computed. If the gap width between E and Q
  // is >= minGapWidth, a cut candidate dividing the elements between Q and E is created.
  int qSize = maxNumOverlappingElements + 1;
  FixedCapacityPriorityQueue<PdfElement*, RightXDescComparator> elementsLargestRightXQueue(qSize);
  elementsLargestRightXQueue.push(sElements[0]);

  // Iterate through the elements from left to right and compute the cut candidates.
  vector<Cut*> candidates;
  for (size_t pos = 1; pos < sElements.size(); pos++) {
    PdfElement* element = sElements[pos];

    // Iterate through the elements in the queue in reversed order (starting at the element with
    // the largest rightX value). Compute the gap width between a queue element and `element`. If
    // the gap width is larger or equal to minGapWidth, create a cut candidate.
    // NOTE: To iterate the queue elements in reversed order, we have to sort the elements
    // manually, since the queue stores the elements by their rightX values in *ascending* order.
    vector<PdfElement*> elementsLargestRightXSorted;
    elementsLargestRightXQueue.sort(RightXDescComparator(), &elementsLargestRightXSorted);

    vector<PdfElement*> overlappingElements;
    for (auto* prevElement : elementsLargestRightXSorted) {
      // Compute the gap width (= the horizontal gap between prevElement and element).
      double gapWidth = computeHorizontalGap(prevElement, element);
      // Compute the x-coordinate of the cut (= the horizontal midpoint of the gap).
      double gapX = prevElement->pos->rightX + ((gapWidth) / 2.0);

      if (equalOrLarger(gapWidth, minGapWidth)) {
        Cut* cut = new Cut(CutDir::X);
        cut->id = createRandomString(3);
        cut->posInElements = pos;
        cut->elementBefore = prevElement;
        cut->elementAfter = element;
        cut->pageNum = element->pos->pageNum;
        cut->x1 = gapX;
        cut->y1 = elementsMinY;
        cut->x2 = gapX;
        cut->y2 = elementsMaxY;
        cut->gapWidth = gapWidth;
        cut->gapHeight = elementsMaxY - elementsMinY;
        cut->overlappingElements = overlappingElements;

        candidates.push_back(cut);
        break;
      }

      overlappingElements.push_back(prevElement);
    }

    // Add the element to the queue if its rightX is larger than the smallest rightX in the queue.
    if (larger(element->pos->rightX, elementsLargestRightXQueue.top()->pos->rightX)) {
      elementsLargestRightXQueue.push(element);
    }
  }

  // Choose the cut candidates that should be actually used to divide the elements.
  chooseCutsFunc(candidates, sElements, silent);

  // Iterate through the cut candidates and check which of the cut candidates were chosen.
  // Divide the elements at the chosen cut candidates.
  size_t prevCutPos = 0;
  bool hasChosenCut = false;
  for (auto* candidate : candidates) {
    size_t cutPos = candidate->posInElements;

    // Add the cut to the result cuts.
    if (resultCuts) {
      resultCuts->push_back(candidate);
    }

    // Skip the cut if it was not chosen.
    if (!candidate->isChosen) {
      continue;
    }

    // Divide the elements at the chosen cut.
    if (resultGroups) {
      vector<PdfElement*> group(sElements.begin() + prevCutPos, sElements.begin() + cutPos);
      resultGroups->push_back(group);
    }
    hasChosenCut = true;
    prevCutPos = cutPos;
  }

  // Don't forget to add the last group to the result groups.
  if (resultGroups) {
    vector<PdfElement*> group(sElements.begin() + prevCutPos, sElements.end());
    resultGroups->push_back(group);
  }

  return hasChosenCut;
}

// _________________________________________________________________________________________________
bool yCut(const vector<PdfElement*>& elements, double minGapHeight,
    const ChooseCutsFunc& chooseCutsFunc, bool silent, vector<vector<PdfElement*>>* resultGroups,
    vector<Cut*>* resultCuts) {
  // Do nothing if no elements are given.
  if (elements.empty()) {
    return false;
  }

  // Sort the elements by their upperY in ascending order.
  vector<PdfElement*> sElements = elements;
  sort(sElements.begin(), sElements.end(), UpperYAscComparator());

  // Compute minY and maxY among the elements, needed for computing the x-coordinates of the cuts.
  double elementsMinX = numeric_limits<double>::max();
  double elementsMaxX = numeric_limits<double>::min();
  for (const auto* element : sElements) {
    elementsMinX = minimum(elementsMinX, element->pos->leftX);
    elementsMaxX = maximum(elementsMaxX, element->pos->rightX);
  }

  // The element with the largest lowerY seen so far.
  PdfElement* elementLargestLowerY = sElements[0];

  // Iterate through the elements in sorted order (= from top to bottom). For each element E,
  // compute the vertical gap between elementLargestLowerY and E. For each gap >= minGapHeight,
  // create a cut candidate.
  vector<Cut*> candidates;
  for (size_t pos = 1; pos < sElements.size(); pos++) {
    PdfElement* element = sElements[pos];

    // Compute the gap height (= the vertical gap between the elementLargestLowerY and element).
    double gapHeight = computeVerticalGap(elementLargestLowerY, element);
    // Compute the y-coordinate of the cut (= the vertical midpoint of the gap).
    double gapY = elementLargestLowerY->pos->lowerY + ((gapHeight) / 2.0);

    if (equalOrLarger(gapHeight, minGapHeight)) {
      Cut* cut = new Cut(CutDir::Y);
      cut->id = createRandomString(3);
      cut->posInElements = pos;
      cut->elementBefore = elementLargestLowerY;
      cut->elementAfter = element;
      cut->pageNum = element->pos->pageNum;
      cut->x1 = elementsMinX;
      cut->y1 = gapY;
      cut->x2 = elementsMaxX;
      cut->y2 = gapY;
      cut->gapWidth = elementsMaxX - elementsMinX;
      cut->gapHeight = gapHeight;

      candidates.push_back(cut);
    }

    // Update elementLargestLowerY if lowerY of the current element is larger.
    if (larger(element->pos->lowerY, elementLargestLowerY->pos->lowerY)) {
      elementLargestLowerY = element;
    }
  }

  // Choose the cut candidates that should be actually used to divide the elements.
  chooseCutsFunc(candidates, sElements, silent);

  // Iterate through the cut candidates and check which of the cut candidates were chosen.
  // Divide the elements at the chosen cut candidates.
  size_t prevCutPos = 0;
  bool hasChosenCut = false;
  for (auto* candidate : candidates) {
    size_t cutPos = candidate->posInElements;

    // Add the cut to the result cuts.
    if (resultCuts) {
      resultCuts->push_back(candidate);
    }

    // Skip the cut if it was not chosen.
    if (!candidate->isChosen) {
      continue;
    }

    // Divide the elements at the chosen cut.
    if (resultGroups) {
      vector<PdfElement*> group(sElements.begin() + prevCutPos, sElements.begin() + cutPos);
      resultGroups->push_back(group);
    }
    hasChosenCut = true;
    prevCutPos = cutPos;
  }

  // Don't forget to add the last group to the result groups.
  if (resultGroups) {
    vector<PdfElement*> group(sElements.begin() + prevCutPos, sElements.end());
    resultGroups->push_back(group);
  }

  return hasChosenCut;
}

}  // namespace ppp::utils
