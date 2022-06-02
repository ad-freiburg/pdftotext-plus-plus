/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // std::sort
#include <iostream>
#include <vector>

#include "./utils/StringUtils.h"
#include "./PdfDocument.h"
#include "./XYCut.h"


// _________________________________________________________________________________________________
void xyCut(const std::vector<PdfElement*>& elements, const ChooseCutsFunc chooseXCutsFunc,
    const ChooseCutsFunc chooseYCutsFunc, double minXCutGapWidth, double minYCutGapHeight,
    int maxNumCuttingElements, bool silent, std::vector<std::vector<PdfElement*>>* resultGroups,
    std::vector<Cut*>* chosenCuts) {
  // Do nothing if no elements are given.
  if (elements.empty()) {
    return;
  }

  // Check if the elements can be separated into groups by one or more vertical cuts (= x-cuts).
  std::vector<std::vector<PdfElement*>> xGroups;
  bool ok = xCut(elements, chooseXCutsFunc, minXCutGapWidth, maxNumCuttingElements, silent,
      &xGroups, chosenCuts);

  if (!ok) {
    // The elements could *not* be separated by a vertical cut.
    // Try to separate the elements by one ore more horizontal cuts (= y-cuts).
    std::vector<std::vector<PdfElement*>> yGroups;
    ok = yCut(elements, chooseYCutsFunc, minYCutGapHeight, silent, &yGroups, chosenCuts);

    if (!ok) {
      // The elements could also *not* be separated by a horizontal cut.
      // So add the group of elements to the result list.
      resultGroups->push_back(elements);
      return;
    }

    // The elements could be separated by one or more y-cuts.
    // Try to further separate each sub-group recursively by vertical and horizontal cuts.
    for (const auto& yGroup : yGroups) {
      xyCut(yGroup, chooseXCutsFunc, chooseYCutsFunc, minXCutGapWidth, minYCutGapHeight,
          maxNumCuttingElements, silent, resultGroups, chosenCuts);
    }

    return;
  }

  // The elements could be separated by one or more x-cuts.
  // Now try to separate each group by y-cuts.
  for (const auto& xGroup : xGroups) {
    std::vector<std::vector<PdfElement*>> yGroups;
    ok = yCut(xGroup, chooseYCutsFunc, minYCutGapHeight, silent, &yGroups, chosenCuts);

    if (!ok) {
      // The group could *not* be further separated by a horizontal cut.
      // So add the group of elements to the result.
      resultGroups->push_back(xGroup);
      continue;
    }

    // The elements could be further separated by one or more horizontal cuts.
    // Try to further separate each group recursively by vertical and horizontal cuts.
    for (const auto& yGroup : yGroups) {
      xyCut(yGroup, chooseXCutsFunc, chooseYCutsFunc, minXCutGapWidth, minYCutGapHeight,
          maxNumCuttingElements, silent, resultGroups, chosenCuts);
    }
  }
}

// _________________________________________________________________________________________________
bool xCut(const std::vector<PdfElement*>& elements, const ChooseCutsFunc chooseCutsFunc,
    double minGapWidth, int maxNumCuttingElements, bool silent,
    std::vector<std::vector<PdfElement*>>* resultGroups, std::vector<Cut*>* resultCuts) {
  // Do nothing if no elements are given.
  if (elements.empty()) {
    return false;
  }

  // Sort the elements by their leftX-values.
  std::vector<PdfElement*> sElements = elements;
  std::sort(sElements.begin(), sElements.end(), [](const PdfElement* e1, const PdfElement* e2) {
    return e1->position->leftX < e2->position->leftX;
  });

  // Compute minY and maxY of the bounding box around the elements.
  double elementsMinY = std::numeric_limits<double>::max();
  double elementsMaxY = std::numeric_limits<double>::min();
  for (const auto* element : sElements) {
    elementsMinY = std::min(elementsMinY, element->position->upperY);
    elementsMaxY = std::max(elementsMaxY, element->position->lowerY);
  }

  std::vector<Cut*> cuts;
  PdfElement* elementLargestRightX = sElements[0];
  PdfElement* elementSecondLargestRightX = sElements[0];

  // Iterate the elements in sorted order (= from left to right) and find all gaps with
  // width > minGapWidth.
  int numGroupElements = 0;
  for (size_t pos = 1; pos < sElements.size(); pos++) {
    PdfElement* element = sElements[pos];

    // TODO: Currently, there is no difference in setting maxNumCuttingElements to 1,
    double gapWidth = element->position->leftX - elementLargestRightX->position->rightX;
    double gapX = elementLargestRightX->position->rightX + ((gapWidth) / 2.0);

    if (gapWidth >= minGapWidth) {
      Cut* cut = new Cut(CutDir::X);
      cut->id = string_utils::createRandomString(3);
      cut->posInElements = pos;
      cut->elementBefore = elementLargestRightX;
      cut->elementAfter = element;
      cut->pageNum = element->position->pageNum;
      cut->x1 = gapX;
      cut->y1 = elementsMinY;
      cut->x2 = gapX;
      cut->y2 = elementsMaxY;
      cut->gapWidth = gapWidth;
      cut->gapHeight = elementsMaxY - elementsMinY;

      cuts.push_back(cut);
      numGroupElements = 0;
    } else {
      // Error-Tolerant.
      std::vector<PdfElement*> cuttingElements;
      if (maxNumCuttingElements > 0 && numGroupElements > maxNumCuttingElements) {
        gapWidth = element->position->leftX - elementSecondLargestRightX->position->rightX;
        gapX = elementSecondLargestRightX->position->rightX + ((gapWidth) / 2.0);

        if (gapWidth >= minGapWidth) {
          Cut* cut = new Cut(CutDir::X);
          cut->id = string_utils::createRandomString(3);
          cut->posInElements = pos;
          cut->elementBefore = elementSecondLargestRightX;
          cut->elementAfter = element;
          cut->cuttingElements.push_back(elementLargestRightX);
          cut->pageNum = element->position->pageNum;
          cut->x1 = gapX;
          cut->y1 = elementsMinY;
          cut->x2 = gapX;
          cut->y2 = elementsMaxY;
          cut->gapWidth = gapWidth;
          cut->gapHeight = elementsMaxY - elementsMinY;

          cuts.push_back(cut);
          numGroupElements = 0;
        }
      }
    }

    if (element->position->rightX > elementLargestRightX->position->rightX) {
      elementSecondLargestRightX = elementLargestRightX;
      elementLargestRightX = element;
    } else if (element->position->rightX > elementSecondLargestRightX->position->rightX) {
      elementSecondLargestRightX = element;
    }

    numGroupElements++;
  }

  // Choose the valid cuts.
  chooseCutsFunc(sElements, cuts, silent);

  size_t prevCutPos = 0;
  bool hasChosenCut = false;
  for (auto* cut : cuts) {
    size_t cutPos = cut->posInElements;

    if (resultCuts) {
      resultCuts->push_back(cut);
    }

    // Skip the cut if it was not chosen.
    if (!cut->isChosen) {
      continue;
    }

    if (resultGroups) {
      std::vector<PdfElement*> group(sElements.begin() + prevCutPos, sElements.begin() + cutPos);
      resultGroups->push_back(group);
    }
    hasChosenCut = true;
    prevCutPos = cutPos;
  }

  // Don't forget to add the last group to the result groups.
  if (resultGroups) {
    std::vector<PdfElement*> group(sElements.begin() + prevCutPos, sElements.end());
    resultGroups->push_back(group);
  }

  return hasChosenCut;
}

// _________________________________________________________________________________________________
bool yCut(const std::vector<PdfElement*>& elements, const ChooseCutsFunc chooseCutsFunc,
    double minGapHeight, bool silent, std::vector<std::vector<PdfElement*>>* resultGroups,
    std::vector<Cut*>* resultCuts) {
  // Do nothing if no elements are given.
  if (elements.empty()) {
    return false;
  }

  // Sort the elements by their upperY-values.
  std::vector<PdfElement*> sElements = elements;
  std::sort(sElements.begin(), sElements.end(), [](const PdfElement* e1, const PdfElement* e2) {
    return e1->position->upperY < e2->position->upperY;
  });

  // Compute minY and maxY of the bounding box around the elements.
  double elementsMinX = std::numeric_limits<double>::max();
  double elementsMaxX = std::numeric_limits<double>::min();
  for (const auto* element : sElements) {
    elementsMinX = std::min(elementsMinX, element->position->leftX);
    elementsMaxX = std::max(elementsMaxX, element->position->rightX);
  }

  std::vector<Cut*> cuts;
  PdfElement* elementLargestLowerY = sElements[0];

  // Iterate the elements in sorted order (= from top to bottom) and find all gaps with
  // height > minGapHeight.
  for (size_t pos = 1; pos < sElements.size(); pos++) {
    PdfElement* element = sElements[pos];

    double gapHeight = element->position->upperY - elementLargestLowerY->position->lowerY;
    double gapY = elementLargestLowerY->position->lowerY + ((gapHeight) / 2.0);

    if (gapHeight > minGapHeight) {
      Cut* cut = new Cut(CutDir::Y);
      cut->id = string_utils::createRandomString(3);
      cut->posInElements = pos;
      cut->elementBefore = elementLargestLowerY;
      cut->elementAfter = element;
      cut->pageNum = element->position->pageNum;
      cut->x1 = elementsMinX;
      cut->y1 = gapY;
      cut->x2 = elementsMaxX;
      cut->y2 = gapY;
      cut->gapWidth = elementsMaxX - elementsMinX;
      cut->gapHeight = gapHeight;
      cuts.push_back(cut);
    }

    if (element->position->lowerY > elementLargestLowerY->position->lowerY) {
      elementLargestLowerY = element;
    }
  }

  // Choose the valid cuts.
  chooseCutsFunc(sElements, cuts, silent);

  size_t prevCutPos = 0;
  bool hasChosenCut = false;
  for (auto* cut : cuts) {
    size_t cutPos = cut->posInElements;

    if (resultCuts) {
      resultCuts->push_back(cut);
    }

    // Skip the cut if it was not chosen.
    if (!cut->isChosen) {
      continue;
    }

    if (resultGroups) {
      std::vector<PdfElement*> group(sElements.begin() + prevCutPos, sElements.begin() + cutPos);
      resultGroups->push_back(group);
    }
    hasChosenCut = true;
    prevCutPos = cutPos;
  }

  // Don't forget to add the last group to the result groups.
  if (resultGroups) {
    std::vector<PdfElement*> group(sElements.begin() + prevCutPos, sElements.end());
    resultGroups->push_back(group);
  }

  return hasChosenCut;
}
