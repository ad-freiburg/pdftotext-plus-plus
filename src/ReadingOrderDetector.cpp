/**
 * Copyright 2022, University of Freiburg,
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
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  auto choosePrimaryYCutsBind = std::bind(&ReadingOrderDetector::choosePrimaryYCuts, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  auto chooseXCutsBind = std::bind(&ReadingOrderDetector::chooseXCuts, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  auto chooseYCutsBind = std::bind(&ReadingOrderDetector::chooseYCuts, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

  // Process the document page-wise. For each page, divide the page elements (= the text blocks and
  // the non-text elements of the page) into groups by using the XY-cut algorithm. Deduce the
  // reading order of the text blocks as follows: whenever the page elements are divided by an
  // x-cut, order all text blocks on the left side of the cut before the text blocks on the right
  // side of the cut. Whenever the page elements are divided by an y-cut, order all text blocks
  // above the cut before the text blocks below the cut.
  for (auto* page : _doc->pages) {
    // Create a vector containing the page elements (= the text blocks, figures and shapes).
    std::vector<PdfElement*> pageElements;
    pageElements.reserve(page->blocks.size() + page->figures.size() + page->shapes.size());
    for (auto* block : page->blocks) { pageElements.push_back(block); }
    for (auto* figure : page->figures) { pageElements.push_back(figure); }
    for (auto* shape : page->shapes) { pageElements.push_back(shape); }

    // Compute the coordinates of the bounding box around the page elements.
    _pageElementsMinX = std::numeric_limits<double>::max();
    _pageElementsMinY = std::numeric_limits<double>::max();
    _pageElementsMaxX = std::numeric_limits<double>::min();
    _pageElementsMaxY = std::numeric_limits<double>::min();
    for (const auto* element : pageElements) {
      _pageElementsMinX = std::min(_pageElementsMinX, element->position->leftX);
      _pageElementsMinY = std::min(_pageElementsMinY, element->position->upperY);
      _pageElementsMaxX = std::max(_pageElementsMaxX, element->position->rightX);
      _pageElementsMaxY = std::max(_pageElementsMaxY, element->position->lowerY);
    }

    std::vector<std::vector<PdfElement*>> groups;

    // Identify the primary x-cuts and divide the page elements into groups at each primary x-cut.
    std::vector<std::vector<PdfElement*>> primaryXCutGroups;
    xCut(pageElements, _minXCutGapWidth, 0, choosePrimaryXCutsBind, false, &primaryXCutGroups,
        &page->readingOrderCuts);

    for (const auto& primXCutGroup : primaryXCutGroups) {
      // Identify the primary y-cuts and divide the page elements into groups at each primary y-cut.
      std::vector<std::vector<PdfElement*>> primaryYCutGroups;
      yCut(primXCutGroup, _minYCutGapHeight, choosePrimaryYCutsBind, false, &primaryYCutGroups,
          &page->readingOrderCuts);

      // Divide each group further by using the recursive XY-cut algorithm.
      for (const auto& primYCutGroup : primaryYCutGroups) {
        xyCut(primYCutGroup, _minXCutGapWidth, _minYCutGapHeight, 0, chooseXCutsBind,
          chooseYCutsBind, false, &groups, &page->readingOrderCuts);
      }
    }

    // Sort the elements of each group from top to bottom and filter the text blocks.
    std::vector<PdfTextBlock*> blocksSorted;
    for (auto& group : groups) {
      std::sort(group.begin(), group.end(), [](const PdfElement* e1, const PdfElement* e2) {
        return e1->position->upperY < e2->position->upperY;
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
void ReadingOrderDetector::chooseXCuts(const std::vector<Cut*>& cuts,
    const std::vector<PdfElement*>& elements, bool silent) {
  // Consider all cut candidates as valid cuts.
  for (Cut* cut : cuts) {
    cut->isChosen = true;
  }
}

// _________________________________________________________________________________________________
void ReadingOrderDetector::choosePrimaryXCuts(const std::vector<Cut*>& cuts,
    const std::vector<PdfElement*>& elements, bool silent) {
  // Do nothing if no elements are given.
  if (elements.empty()) {
    return;
  }
  // Do nothing if no cuts are given.
  if (cuts.empty()) {
    return;
  }

  // Iterate through the cuts. For each, decide whether or not it denotes a primary x-cut.
  for (Cut* cut : cuts) {
    const PdfTextBlock* blockLeft = dynamic_cast<const PdfTextBlock*>(cut->elementBefore);
    if (blockLeft) {
      if (blockLeft->position->wMode != 0 || blockLeft->position->rotation != 0) {
        cut->isChosen = true;
        continue;
      }
    }

    const PdfTextBlock* blockRight = dynamic_cast<const PdfTextBlock*>(cut->elementAfter);
    if (blockRight) {
      if (blockRight->position->wMode != 0 || blockRight->position->rotation != 0) {
        cut->isChosen = true;
        continue;
      }
    }

    if (blockLeft && blockRight) {
      // Consider the gap to be a primary x-cut when the rotations of the element to the left and
      // to the right of the cut differ.
      if (blockLeft->position->wMode != blockRight->position->wMode) {
        cut->isChosen = true;
        continue;
      }

      // Consider the gap to be a primary x-cut when the writing modes of the element to the left
      // and to the right of the cut differ.
      if (blockLeft->position->rotation != blockRight->position->rotation) {
        cut->isChosen = true;
        continue;
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

    const PdfNonTextElement* nonTextLeft = dynamic_cast<const PdfNonTextElement*>(cut->elementBefore);
    if (nonTextLeft != nullptr) {
      double upperY = nonTextLeft->position->upperY;
      double lowerY = nonTextLeft->position->lowerY;
      double height = nonTextLeft->position->getHeight();
      // The element must exceed a certain width; one end point must start in the left half of the
      // bounding box around the page elements; and the other end point in the right half.
      if (height > 10 * _doc->avgCharHeight && upperY < pageElementsMid && lowerY > pageElementsMid) {
        cut->isChosen = true;
        continue;
      }
    }
    const PdfNonTextElement* nonTextRight = dynamic_cast<const PdfNonTextElement*>(cut->elementAfter);
    if (nonTextRight != nullptr) {
      double upperY = nonTextRight->position->upperY;
      double lowerY = nonTextRight->position->lowerY;
      double height = nonTextRight->position->getHeight();
      // The element must exceed a certain width; one end point must start in the left half of the
      // bounding box around the page elements; and the other end point in the right half.
      if (height > 10 * _doc->avgCharHeight && upperY < pageElementsMid && lowerY > pageElementsMid) {
        cut->isChosen = true;
        continue;
      }
    }
  }
}

// _________________________________________________________________________________________________
void ReadingOrderDetector::choosePrimaryYCuts(const std::vector<Cut*>& cuts,
    const std::vector<PdfElement*>& elements, bool silent) {
  // Do nothing if no elements are given.
  if (elements.empty()) {
    return;
  }
  // Do nothing if no cuts are given.
  if (cuts.empty()) {
    return;
  }

  // Iterate through the gap positions. For each, decide whether or not it denotes a primary y-cut.
  for (Cut* cut : cuts) {
    // Consider the gap to be a primary y-cut when the element above or below the gap denotes the
    // title, a text block with author info, a page header, or a page footer.
    const PdfTextBlock* blockAbove = dynamic_cast<const PdfTextBlock*>(cut->elementBefore);
    bool isTitleAbove = blockAbove && blockAbove->role == "TITLE";
    bool isAuthorInfoAbove = blockAbove && blockAbove->role == "AUTHOR_INFO";
    bool isMarginalAbove = blockAbove && blockAbove->role == "MARGINAL";

    const PdfTextBlock* blockBelow = dynamic_cast<const PdfTextBlock*>(cut->elementAfter);
    bool isTitleBelow = blockBelow && blockBelow->role == "TITLE";
    bool isAuthorInfoBelow = blockBelow && blockBelow->role == "AUTHOR_INFO";
    bool isMarginalBelow = blockBelow && blockBelow->role == "MARGINAL";

    if (isTitleAbove != isTitleBelow) {
      cut->isChosen = true;
      continue;
    }

    if (isAuthorInfoAbove != isAuthorInfoBelow) {
      cut->isChosen = true;
      continue;
    }

    if (isMarginalAbove != isMarginalBelow) {
      cut->isChosen = true;
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
    const PdfNonTextElement* nonTextAbove = dynamic_cast<const PdfNonTextElement*>(cut->elementBefore);
    if (nonTextAbove != nullptr) {
      double leftX = nonTextAbove->position->leftX;
      double rightX = nonTextAbove->position->rightX;
      double width = nonTextAbove->position->getWidth();
      // The element must exceed a certain width; one end point must start in the left half of the
      // bounding box around the page elements; and the other end point in the right half.
      if (width > 10 * _doc->avgCharWidth && leftX < pageElementsMid && rightX > pageElementsMid) {
        cut->isChosen = true;
        continue;
      }
    }
    const PdfNonTextElement* nonTextBelow = dynamic_cast<const PdfNonTextElement*>(cut->elementAfter);
    if (nonTextBelow != nullptr) {
      double leftX = nonTextBelow->position->leftX;
      double rightX = nonTextBelow->position->rightX;
      double width = nonTextBelow->position->getWidth();
      // The element must exceed a certain width; one end point must start in the left half of the
      // bounding box around the page elements; and the other end point in the right half.
      if (width > 10 * _doc->avgCharWidth && leftX < pageElementsMid && rightX > pageElementsMid) {
        cut->isChosen = true;
        continue;
      }
    }
  }
}

// _________________________________________________________________________________________________
void ReadingOrderDetector::chooseYCuts(const std::vector<Cut*>& cuts,
    const std::vector<PdfElement*>& elements, bool silent) {
  // Do nothing if no elements are given.
  if (elements.empty()) {
    return;
  }
  // Do nothing if no gaps are given.
  if (cuts.empty()) {
    return;
  }

  // Define the bind required to pass the chooseXCuts() method to the XY-cut class.
  auto chooseXCutsBind = std::bind(&ReadingOrderDetector::chooseXCuts, this, std::placeholders::_1,
    std::placeholders::_2, std::placeholders::_3);

  size_t firstCutIndex = 0;
  size_t lastCutIndex = cuts.size() - 1;

  // Iterate through the gaps. Consider a gap to be a valid y-cut if all page elements below the
  // gap can be subsequently divided by a valid x-cut.
  for (size_t i = 0; i < cuts.size(); i++) {
    Cut* cut = cuts[i];
    std::vector<PdfElement*> group(elements.begin() + cut->posInElements, elements.end());
    bool cutOk = xCut(group, _minXCutGapWidth, 0, chooseXCutsBind, true);
    if (cutOk) {
      cut->isChosen = true;
      lastCutIndex = i;
      break;
    }
  }

  // Iterate through the remaining gaps (= all gaps above the previous cut). Consider a gap to be a
  // valid y-cut if all page elements above the gap can be subsequently divided by a valid x-cut.
  // for (size_t i = lastGapPositionIndex; i >= firstGapPositionIndex; i--) {
  for (size_t i = lastCutIndex + 1; i --> firstCutIndex; ) {
    Cut* cut = cuts[i];
    std::vector<PdfElement*> group(elements.begin(), elements.begin() + cut->posInElements);
    bool cutOk = xCut(group, _minXCutGapWidth, 0, chooseXCutsBind, true);
    if (cutOk) {
      cut->isChosen = true;
      firstCutIndex = i;
      break;
    }
  }

  // Iterate through the remaining gaps (= all gaps between the cuts found in the previous two
  // iterations) and inspect gap pairs (an upper gap and a lower gap). Consider both gaps to be
  // valid y-cuts if the elements between the two gaps can be subsequently divided by a valid x-cut.
  for (size_t i = firstCutIndex; i < lastCutIndex; i++) {
    Cut* highCut = cuts[i];
    // for (size_t j = lastGapPositionIndex; j > i; j--) {
    for (size_t j = lastCutIndex + 1; j --> i; ) {
      Cut* lowCut = cuts[j];
      std::vector<PdfElement*> group(elements.begin() + highCut->posInElements, elements.begin() + lowCut->posInElements);
      bool cutOk = xCut(group, _minXCutGapWidth, 0, chooseXCutsBind, true);

      if (cutOk) {
        highCut->isChosen = true;
        lowCut->isChosen = true;
        i = j;
        break;
      }
    }
  }
}
