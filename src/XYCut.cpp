/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // std::sort
#include <iostream>
#include <vector>

#include "./PdfDocument.h"
#include "./XYCut.h"

const int GAP_MIN_HEIGHT = 0;
const int GAP_MIN_WIDTH = 0;

// _________________________________________________________________________________________________
void xyCut(const std::vector<PdfElement*>& elements, const ChooseCutsFunc chooseXCutsFunc,
    const ChooseCutsFunc chooseYCutsFunc, std::vector<std::vector<PdfElement*>>* resultGroups,
    std::vector<Cut*>* chosenCuts) {
  // Do nothing if no elements are given.
  if (elements.size() == 0) {
    return;
  }

  // Check if the elements can be separated into groups by vertical cuts (= x-cuts).
  std::vector<std::vector<PdfElement*>> xGroups;
  bool ok = xCut(elements, chooseXCutsFunc, &xGroups, chosenCuts);

  if (!ok) {
    // The elements could *not* be separated by a vertical cut.
    // Try to separate the elements by horizontal cuts (= y-cuts).
    std::vector<std::vector<PdfElement*>> yGroups;
    ok = yCut(elements, chooseYCutsFunc, &yGroups, chosenCuts);

    if (!ok) {
      // The blocks could also *not* be separated by a horizontal cut.
      // So add the group of elements to the result.
      resultGroups->push_back(elements);
      return;
    }

    // The elements could be separated by one or more horizontal cuts.
    // Try to further separate each "Y-group" recursively by vertical and horizontal cuts.
    for (const auto& yGroup : yGroups) {
      xyCut(yGroup, chooseXCutsFunc, chooseYCutsFunc, resultGroups, chosenCuts);
    }

    return;
  }

  // The elements could be separated by one or more vertical cuts.
  // Now try to separate each "X-group" by horizontal cuts (= y-cuts).
  for (const auto& xGroup : xGroups) {
    std::vector<std::vector<PdfElement*>> yGroups;
    ok = yCut(xGroup, chooseYCutsFunc, &yGroups, chosenCuts);

    if (!ok) {
      // The group could *not* be further separated by a horizontal cut.
      // So add the group of elements to the result.
      resultGroups->push_back(xGroup);
      continue;
    }

    // The elements could be further separated by one or more horizontal cuts.
    // Try to further separate each group recursively by vertical and horizontal cuts.
    for (const auto& yGroup : yGroups) {
      xyCut(yGroup, chooseXCutsFunc, chooseYCutsFunc, resultGroups, chosenCuts);
    }
  }
}

// _________________________________________________________________________________________________
bool xCut(const std::vector<PdfElement*>& elements, const ChooseCutsFunc chooseCutsFunc,
    std::vector<std::vector<PdfElement*>>* resultGroups, std::vector<Cut*>* resultCuts) {
  // Do nothing if no elements are given.
  if (elements.size() == 0) {
    return false;
  }

  // Sort the elements by their leftX-values.
  std::vector<PdfElement*> sElements = elements;
  std::sort(sElements.begin(), sElements.end(), [](const PdfElement* e1, const PdfElement* e2) {
    return e1->position->leftX < e2->position->leftX;
  });

  PdfElement* elementLargestMaxX = sElements[0];
  std::vector<size_t> gapPositions;
  std::vector<PdfElement*> gapStartElements;

  // Iterate the elements in sorted order (= from left to right) and find all gaps with width > 0.
  for (size_t pos = 1; pos < sElements.size(); pos++) {
    PdfElement* element = sElements[pos];

    double gapWidth = element->position->leftX - elementLargestMaxX->position->rightX;

    if (gapWidth > GAP_MIN_WIDTH) {
      gapPositions.push_back(pos);
      gapStartElements.push_back(elementLargestMaxX);
    }

    if (element->position->rightX > elementLargestMaxX->position->rightX) {
      elementLargestMaxX = element;
    }
  }

  std::vector<size_t> cutIndices;
  chooseCutsFunc(sElements, gapPositions, gapStartElements, &cutIndices);

  double elementsMinY = std::numeric_limits<double>::max();
  double elementsMaxY = std::numeric_limits<double>::min();
  for (const auto* element : sElements) {
    elementsMinY = std::min(elementsMinY, element->position->upperY);
    elementsMaxY = std::max(elementsMaxY, element->position->lowerY);
  }

  size_t prevCutPos = 0;
  for (size_t cutIndex : cutIndices) {
    size_t cutPos = gapPositions[cutIndex];
    PdfElement* cutStartElement = gapStartElements[cutIndex];

    if (resultGroups) {
      std::vector<PdfElement*> group(sElements.begin() + prevCutPos, sElements.begin() + cutPos);
      resultGroups->push_back(group);
    }

    if (resultCuts) {
      int pageNum = elements[0]->position->pageNum;
      double x = cutStartElement->position->rightX + ((sElements[cutPos]->position->leftX - cutStartElement->position->rightX) / 2);
      resultCuts->push_back(new Cut(CutDir::X, pageNum, x, elementsMinY, x, elementsMaxY));
    }

    prevCutPos = cutPos;
  }

  // Don't forget to add the last group to the result groups.
  if (resultGroups) {
    std::vector<PdfElement*> group(sElements.begin() + prevCutPos, sElements.end());
    resultGroups->push_back(group);
  }

  return cutIndices.size() > 0;
}

// _________________________________________________________________________________________________
bool yCut(const std::vector<PdfElement*>& elements, const ChooseCutsFunc chooseCutsFunc,
    std::vector<std::vector<PdfElement*>>* resultGroups, std::vector<Cut*>* resultCuts) {
  // Do nothing if no elements are given.
  if (elements.size() == 0) {
    return false;
  }

  // Sort the elements by their upperY-values.
  std::vector<PdfElement*> sElements = elements;
  std::sort(sElements.begin(), sElements.end(), [](const PdfElement* e1, const PdfElement* e2) {
    return e1->position->upperY < e2->position->upperY;
  });

  PdfElement* elementLargestMaxY = sElements[0];
  std::vector<size_t> gapPositions;
  std::vector<PdfElement*> gapStartElements;

  // Iterate the elements in sorted order (= from top to bottom) and find all gaps with width > 0.
  for (size_t pos = 1; pos < sElements.size(); pos++) {
    PdfElement* element = sElements[pos];

    double gapHeight = element->position->upperY - elementLargestMaxY->position->lowerY;
    if (gapHeight > GAP_MIN_HEIGHT) {
      if (element->position->pageNum == 4) {
        std::cout << pos << std::endl;
      }
      gapPositions.push_back(pos);
      gapStartElements.push_back(elementLargestMaxY);
    }

    if (element->position->lowerY > elementLargestMaxY->position->lowerY) {
      elementLargestMaxY = element;
    }
  }

  std::vector<size_t> cutIndices;
  chooseCutsFunc(sElements, gapPositions, gapStartElements, &cutIndices);

  double elementsMinX = std::numeric_limits<double>::max();
  double elementsMaxX = std::numeric_limits<double>::min();
  for (const auto* element : sElements) {
    elementsMinX = std::min(elementsMinX, element->position->leftX);
    elementsMaxX = std::max(elementsMaxX, element->position->rightX);
  }

  size_t prevCutPos = 0;
  for (size_t cutIndex : cutIndices) {
    size_t cutPos = gapPositions[cutIndex];
    PdfElement* cutStartElement = gapStartElements[cutIndex];

    if (resultGroups) {
      std::vector<PdfElement*> group(sElements.begin() + prevCutPos, sElements.begin() + cutPos);
      resultGroups->push_back(group);
    }

    if (resultCuts) {
      int pageNum = elements[0]->position->pageNum;
      double y = cutStartElement->position->lowerY + ((sElements[cutPos]->position->upperY - cutStartElement->position->lowerY) / 2);
      resultCuts->push_back(new Cut(CutDir::Y, pageNum, elementsMinX, y, elementsMaxX, y));
    }

    prevCutPos = cutPos;
  }

  // Don't forget to add the last group to the result groups.
  if (resultGroups) {
    std::vector<PdfElement*> group(sElements.begin() + prevCutPos, sElements.end());
    resultGroups->push_back(group);
  }

  return cutIndices.size() > 0;
}
