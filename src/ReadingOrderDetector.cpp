/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cmath>  // ceil
#include <functional>  // std::bind, std::function
#include <unordered_set>  // std::bind, std::function

#include "./ReadingOrderDetector.h"
#include "./PdfDocument.h"
#include "./XYCut.h"

// _________________________________________________________________________________________________
ReadingOrderDetector::ReadingOrderDetector(PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
ReadingOrderDetector::~ReadingOrderDetector() = default;

// _________________________________________________________________________________________________
void ReadingOrderDetector::detect() {
  detectSemanticRoles();
  detectReadingOrder();
}

// _________________________________________________________________________________________________
void ReadingOrderDetector::detectSemanticRoles() {
  _semanticRolesPredictor.predict(_doc);
}

// _________________________________________________________________________________________________
void ReadingOrderDetector::detectReadingOrder() {
  // Do nothing if no document is given.
  if (!_doc) {
    return;
  }

  // Do nothing if no pages are given.
  if (_doc->pages.size() == 0) {
    return;
  }

  // Define the binds required to pass the chooseXCuts()/chooseYCuts() methods to the XY-cut class.
  auto choosePrimaryXCutsBind = std::bind(&ReadingOrderDetector::choosePrimaryXCuts, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
  auto choosePrimaryYCutsBind = std::bind(&ReadingOrderDetector::choosePrimaryYCuts, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
  auto chooseXCutsBind = std::bind(&ReadingOrderDetector::chooseXCuts, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
  auto chooseYCutsBind = std::bind(&ReadingOrderDetector::chooseYCuts, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

  // Process the document page-wise. For each page, divide the page elements (= the text blocks and
  // the non-text elements of the page) into groups by using the XY-cut algorithm. Deduce the
  // reading order of the text blocks as follows: whenever the page elements are divided by an
  // x-cut, order all text blocks on the left side of the cut before the text blocks on the right
  // side of the cut. Whenever the page elements are divided by an y-cut, order all text blocks
  // above the cut before the text blocks below the cut.
  for (auto* page : _doc->pages) {
    // Create a vector containing the page elements (= the text blocks and non-text elements).
    std::vector<PdfElement*> pageElements;
    pageElements.reserve(page->blocks.size() + page->nonTexts.size());
    for (auto* block : page->blocks) { pageElements.push_back(block); }
    for (auto* nonText : page->nonTexts) { pageElements.push_back(nonText); }

    // Compute the coordinates of the bounding box around the page elements.
    _pageElementsMinX = std::numeric_limits<double>::max();
    _pageElementsMinY = std::numeric_limits<double>::max();
    _pageElementsMaxX = std::numeric_limits<double>::min();
    _pageElementsMaxY = std::numeric_limits<double>::min();
    for (const auto* element : pageElements) {
      _pageElementsMinX = std::min(_pageElementsMinX, element->minX);
      _pageElementsMinY = std::min(_pageElementsMinY, element->minY);
      _pageElementsMaxX = std::max(_pageElementsMaxX, element->maxX);
      _pageElementsMaxY = std::max(_pageElementsMaxY, element->maxY);
    }

    std::vector<std::vector<PdfElement*>> groups;

    // Identify the primary x-cuts and divide the page elements into groups at each primary x-cut.
    std::vector<std::vector<PdfElement*>> primaryXCutGroups;
    xCut(pageElements, choosePrimaryXCutsBind, &primaryXCutGroups, &page->readingOrderCuts);

    for (const auto& primXCutGroup : primaryXCutGroups) {
      // Identify the primary y-cuts and divide the page elements into groups at each primary y-cut.
      std::vector<std::vector<PdfElement*>> primaryYCutGroups;
      yCut(primXCutGroup, choosePrimaryYCutsBind, &primaryYCutGroups, &page->readingOrderCuts);

      // Divide each group further by using the recursive XY-cut algorithm.
      for (const auto& primYCutGroup : primaryYCutGroups) {
        xyCut(primYCutGroup, chooseXCutsBind, chooseYCutsBind, &groups, &page->readingOrderCuts);
      }
    }

    // Sort the elements of each group from top to bottom and filter the text blocks.
    std::vector<PdfTextBlock*> blocksSorted;
    for (auto& group : groups) {
      std::sort(group.begin(), group.end(), [](const PdfElement* e1, const PdfElement* e2) {
        return e1->minY < e2->minY;
      });

      for (auto* element : group) {
        PdfTextBlock* block = dynamic_cast<PdfTextBlock*>(element);
        if (block != nullptr) { blocksSorted.push_back(block); }
      }
    }

    page->blocks = blocksSorted;
  }
}

// _________________________________________________________________________________________________
void ReadingOrderDetector::chooseXCuts(const std::vector<PdfElement*>& elements,
    const std::vector<size_t>& gapPositions, const std::vector<PdfElement*>& gapStartElements,
    std::vector<size_t>* cutIndices) {
  // Consider all given gaps to be a valid x-cut.
  for (size_t i = 0; i < gapPositions.size(); i++) {
    cutIndices->push_back(i);
  }
}

// _________________________________________________________________________________________________
void ReadingOrderDetector::choosePrimaryXCuts(const std::vector<PdfElement*>& elements,
    const std::vector<size_t>& gapPositions, const std::vector<PdfElement*>& gapStartElements,
    std::vector<size_t>* cutIndices) {
  // Do nothing if no elements are given.
  if (elements.size() == 0) {
    return;
  }
  // Do nothing if no gaps are given.
  if (gapPositions.size() == 0) {
    return;
  }
  // Do nothing if no gap start elements are given.
  if (gapStartElements.size() == 0) {
    return;
  }

  // Iterate through the gap positions. For each, decide whether or not it denotes a primary x-cut.
  for (size_t i = 0; i < gapPositions.size(); i++) {
    int gapPos = gapPositions[i];

    // Determine the closest element left and right of the gap.
    const PdfElement* elementLeft = gapStartElements[i];
    const PdfElement* elementRight = elements[gapPos];

    const PdfTextBlock* blockLeft = dynamic_cast<const PdfTextBlock*>(elementLeft);
    if (blockLeft) {
      if (blockLeft->wMode != 0 || blockLeft->rotation != 0) {
        cutIndices->push_back(i);
        continue;
      }
    }

    const PdfTextBlock* blockRight = dynamic_cast<const PdfTextBlock*>(elementRight);
    if (blockRight) {
      if (blockRight->wMode != 0 || blockRight->rotation != 0) {
        cutIndices->push_back(i);
        continue;
      }
    }

    if (blockLeft && blockRight) {
      // Consider the gap to be a primary x-cut when the rotations of the element to the left and
      // to the right of the cut differ.
      if (blockLeft->wMode != blockRight->wMode) {
        cutIndices->push_back(i);
        continue;
      }

      // Consider the gap to be a primary x-cut when the writing modes of the element to the left
      // and to the right of the cut differ.
      if (blockLeft->rotation != blockRight->rotation) {
        cutIndices->push_back(i);
      }
    }

    // Consider the gap to be a primary y-cut, when the element above or below the cut is a
    // horizontal line with a given minimum length and one end point of the line located in the
    // left half of the bounding box around the page elements and the other end point is located in
    // the right half, like illustrated in the following example (the "-----" part should denote a
    // horizontal line that visually separates the text blocks above the line from the text blocks
    // below the line.
    // xxxxx  yyyyyy
    // xxxxx  yyyyyy
    //    ------
    // xxxxx  yyyyyy
    // xxxxx  yyyyyy
    double pageElementsMid = _pageElementsMinY + (_pageElementsMaxY - _pageElementsMinY) / 2.0;

    const PdfNonText* nonTextLeft = dynamic_cast<const PdfNonText*>(elementLeft);
    if (nonTextLeft != nullptr) {
      double minY = nonTextLeft->minY;
      double maxY = nonTextLeft->maxY;
      double height = nonTextLeft->getHeight();
      // The element must exceed a certain width; one end point must start in the left half of the
      // bounding box around the page elements; and the other end point in the right half.
      if (height > 10 * _doc->avgGlyphHeight && minY < pageElementsMid && maxY > pageElementsMid) {
        cutIndices->push_back(i);
        continue;
      }
    }
    const PdfNonText* nonTextRight = dynamic_cast<const PdfNonText*>(elementRight);
    if (nonTextRight != nullptr) {
      double minY = nonTextRight->minY;
      double maxY = nonTextRight->maxY;
      double height = nonTextRight->getHeight();
      // The element must exceed a certain width; one end point must start in the left half of the
      // bounding box around the page elements; and the other end point in the right half.
      if (height > 10 * _doc->avgGlyphHeight && minY < pageElementsMid && maxY > pageElementsMid) {
        cutIndices->push_back(i);
        continue;
      }
    }
  }
}

// _________________________________________________________________________________________________
void ReadingOrderDetector::choosePrimaryYCuts(const std::vector<PdfElement*>& elements,
    const std::vector<size_t>& gapPositions, const std::vector<PdfElement*>& gapStartElements,
    std::vector<size_t>* cutIndices) {
  // Do nothing if no elements are given.
  if (elements.size() == 0) {
    return;
  }
  // Do nothing if no gaps are given.
  if (gapPositions.size() == 0) {
    return;
  }
  // Do nothing if no gap start elements are given.
  if (gapStartElements.size() == 0) {
    return;
  }

  // Iterate through the gap positions. For each, decide whether or not it denotes a primary y-cut.
  for (size_t i = 0; i < gapPositions.size(); i++) {
    int gapPos = gapPositions[i];

    // Determine the closest element above and below the gap.
    const PdfElement* elementAbove = gapStartElements[i];
    const PdfElement* elementBelow = elements[gapPos];

    // Consider the gap to be a primary y-cut when the element above or below the gap denotes the
    // title, a text block with author info, a page header, or a page footer.
    const PdfTextBlock* blockAbove = dynamic_cast<const PdfTextBlock*>(elementAbove);
    bool isTitleAbove = blockAbove && blockAbove->role == "TITLE";
    bool isAuthorInfoAbove = blockAbove && blockAbove->role == "AUTHOR_INFO";
    bool isMarginalAbove = blockAbove && blockAbove->role == "MARGINAL";

    const PdfTextBlock* blockBelow = dynamic_cast<const PdfTextBlock*>(elementBelow);
    bool isTitleBelow = blockBelow && blockBelow->role == "TITLE";
    bool isAuthorInfoBelow = blockBelow && blockBelow->role == "AUTHOR_INFO";
    bool isMarginalBelow = blockBelow && blockBelow->role == "MARGINAL";

    if (isTitleAbove != isTitleBelow) {
      cutIndices->push_back(i);
      continue;
    }

    if (isAuthorInfoAbove != isAuthorInfoBelow) {
      cutIndices->push_back(i);
      continue;
    }

    if (isMarginalAbove != isMarginalBelow) {
      cutIndices->push_back(i);
      continue;
    }

    // Consider the gap to be a primary y-cut, when the element above or below the cut is a
    // horizontal line with a given minimum length and one end point of the line located in the
    // left half of the bounding box around the page elements and the other end point is located in
    // the right half, like illustrated in the following example (the "-----" part should denote a
    // horizontal line that visually separates the text blocks above the line from the text blocks
    // below the line.
    // xxxxx  yyyyyy
    // xxxxx  yyyyyy
    //    ------
    // xxxxx  yyyyyy
    // xxxxx  yyyyyy
    double pageElementsMid = _pageElementsMinX + (_pageElementsMaxX - _pageElementsMinX) / 2.0;
    const PdfNonText* nonTextAbove = dynamic_cast<const PdfNonText*>(elementAbove);
    if (nonTextAbove != nullptr) {
      double minX = nonTextAbove->minX;
      double maxX = nonTextAbove->maxX;
      double width = nonTextAbove->getWidth();
      // The element must exceed a certain width; one end point must start in the left half of the
      // bounding box around the page elements; and the other end point in the right half.
      if (width > 10 * _doc->avgGlyphWidth && minX < pageElementsMid && maxX > pageElementsMid) {
        cutIndices->push_back(i);
        continue;
      }
    }
    const PdfNonText* nonTextBelow = dynamic_cast<const PdfNonText*>(elementBelow);
    if (nonTextBelow != nullptr) {
      double minX = nonTextBelow->minX;
      double maxX = nonTextBelow->maxX;
      double width = nonTextBelow->getWidth();
      // The element must exceed a certain width; one end point must start in the left half of the
      // bounding box around the page elements; and the other end point in the right half.
      if (width > 10 * _doc->avgGlyphWidth && minX < pageElementsMid && maxX > pageElementsMid) {
        cutIndices->push_back(i);
        continue;
      }
    }
  }
}

// _________________________________________________________________________________________________
void ReadingOrderDetector::chooseYCuts(const std::vector<PdfElement*>& elements,
    const std::vector<size_t>& gapPositions, const std::vector<PdfElement*>& gapStartElements,
    std::vector<size_t>* cutIndices) {
  // Do nothing if no elements are given.
  if (elements.size() == 0) {
    return;
  }
  // Do nothing if no gaps are given.
  if (gapPositions.size() == 0) {
    return;
  }
  // Do nothing if no gap start elements are given.
  if (gapStartElements.size() == 0) {
    return;
  }

  // Define the bind required to pass the chooseXCuts() method to the XY-cut class.
  auto chooseXCutsBind = std::bind(&ReadingOrderDetector::chooseXCuts, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

  // Instead of appending the cut indices directly to `cutIndices`, store them in a set to avoid
  // to add duplicate entries to `cutIndices`.
  std::unordered_set<size_t> cutIndicesSet;

  size_t firstGapPositionIndex = 0;
  size_t lastGapPositionIndex = gapPositions.size() - 1;

  // Iterate through the gaps. Consider a gap to be a valid y-cut if all page elements below the
  // gap can be subsequently divided by a valid x-cut.
  for (size_t i = 0; i < gapPositions.size(); i++) {
    size_t gapPos = gapPositions[i];
    std::vector<PdfElement*> group(elements.begin() + gapPos, elements.end());
    bool cutOk = xCut(group, chooseXCutsBind);
    if (cutOk) {
      cutIndicesSet.insert(i);
      lastGapPositionIndex = i;
      break;
    }
  }

  // Iterate through the remaining gaps (= all gaps above the previous cut). Consider a gap to be a
  // valid y-cut if all page elements above the gap can be subsequently divided by a valid x-cut.
  // for (size_t i = lastGapPositionIndex; i >= firstGapPositionIndex; i--) {
  for (size_t i = lastGapPositionIndex + 1; i --> firstGapPositionIndex; ) {
    size_t gapPos = gapPositions[i];
    std::vector<PdfElement*> group(elements.begin(), elements.begin() + gapPos);
    bool cutOk = xCut(group, chooseXCutsBind);
    if (cutOk) {
      cutIndicesSet.insert(i);
      firstGapPositionIndex = i;
      break;
    }
  }

  // Iterate through the remaining gaps (= all gaps between the cuts found in the previous two
  // iterations) and inspect gap pairs (an upper gap and a lower gap). Consider both gaps to be
  // valid y-cuts if the elements between the two gaps can be subsequently divided by a valid x-cut.
  for (size_t i = firstGapPositionIndex; i < lastGapPositionIndex; i++) {
    size_t highGapPos = gapPositions[i];
    // for (size_t j = lastGapPositionIndex; j > i; j--) {
    for (size_t j = lastGapPositionIndex + 1; j --> i; ) {
      size_t lowGapPos = gapPositions[j];
      std::vector<PdfElement*> group(elements.begin() + highGapPos, elements.begin() + lowGapPos);
      bool cutOk = xCut(group, chooseXCutsBind);

      if (cutOk) {
        cutIndicesSet.insert(i);
        cutIndicesSet.insert(j);
        i = j;
        break;
      }
    }
  }

  // Add the cut indices to the result vector and sort them in ascending order.
  cutIndices->assign(cutIndicesSet.begin(), cutIndicesSet.end());
  std::sort(cutIndices->begin(), cutIndices->end());
}
