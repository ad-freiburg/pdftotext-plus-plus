/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // std::sort, std::min, std::max
#include <functional>  // std::bind, std::function
#include <limits>  // std::numeric_limits
#include <unordered_set>
#include <vector>
#include <iostream>

#include "./Config.h"
#include "./PdfDocument.h"
#include "./ReadingOrderDetector.h"
#include "./XYCut.h"

using std::max;
using std::min;
using std::numeric_limits;
using std::vector;

// _________________________________________________________________________________________________
ReadingOrderDetector::ReadingOrderDetector(const PdfDocument* doc, const ppp::Config* config) {
  _doc = doc;
  _config = config;
  _semanticRolesPredictor = new SemanticRolesPredictor(config);
}

// _________________________________________________________________________________________________
ReadingOrderDetector::~ReadingOrderDetector() {
  delete _semanticRolesPredictor;
}

// _________________________________________________________________________________________________
void ReadingOrderDetector::detect() {
  detectSemanticRoles();
  detectReadingOrder();
}

// _________________________________________________________________________________________________
void ReadingOrderDetector::detectSemanticRoles() {
  _semanticRolesPredictor->predict(_doc);
}

// _________________________________________________________________________________________________
void ReadingOrderDetector::detectReadingOrder() {
  assert(_doc);

  // Do nothing if no pages are given.
  if (_doc->pages.empty()) {
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
    vector<PdfElement*> pageElements;
    pageElements.reserve(page->blocks.size() + page->figures.size() + page->shapes.size());
    for (auto* block : page->blocks) { pageElements.push_back(block); }
    for (auto* figure : page->figures) { pageElements.push_back(figure); }
    for (auto* shape : page->shapes) { pageElements.push_back(shape); }

    // Compute the coordinates of the bounding box around the page elements.
    _pageElementsMinX = numeric_limits<double>::max();
    _pageElementsMinY = numeric_limits<double>::max();
    _pageElementsMaxX = numeric_limits<double>::min();
    _pageElementsMaxY = numeric_limits<double>::min();
    for (const auto* element : pageElements) {
      _pageElementsMinX = min(_pageElementsMinX, element->pos->leftX);
      _pageElementsMinY = min(_pageElementsMinY, element->pos->upperY);
      _pageElementsMaxX = max(_pageElementsMaxX, element->pos->rightX);
      _pageElementsMaxY = max(_pageElementsMaxY, element->pos->lowerY);
    }

    vector<vector<PdfElement*>> groups;

    // Identify the primary x-cuts and divide the page elements into groups at each primary x-cut.
    vector<vector<PdfElement*>> primaryXCutGroups;
    xCut(pageElements, _minXCutGapWidth, 0, choosePrimaryXCutsBind, false, &primaryXCutGroups,
        &page->readingOrderCuts);

    for (const auto& primXCutGroup : primaryXCutGroups) {
      // Identify the primary y-cuts and divide the page elements into groups at each primary y-cut.
      vector<vector<PdfElement*>> primaryYCutGroups;
      yCut(primXCutGroup, _minYCutGapHeight, choosePrimaryYCutsBind, false, &primaryYCutGroups,
          &page->readingOrderCuts);

      // Divide each group further by using the recursive XY-cut algorithm.
      for (const auto& primYCutGroup : primaryYCutGroups) {
        xyCut(primYCutGroup, _minXCutGapWidth, _minYCutGapHeight, 0, chooseXCutsBind,
          chooseYCutsBind, false, &groups, &page->readingOrderCuts);
      }
    }

    // Sort the elements of each group from top to bottom and filter the text blocks.
    vector<PdfTextBlock*> blocksSorted;
    for (auto& group : groups) {
      std::sort(group.begin(), group.end(), [](const PdfElement* e1, const PdfElement* e2) {
        return e1->pos->upperY < e2->pos->upperY;
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
void ReadingOrderDetector::chooseXCuts(const vector<Cut*>& cuts,
    const vector<PdfElement*>& elements, bool silent) {
  // Consider all cut candidates as valid cuts.
  for (Cut* cut : cuts) {
    cut->isChosen = true;
  }
}

// _________________________________________________________________________________________________
void ReadingOrderDetector::choosePrimaryXCuts(const vector<Cut*>& cuts,
    const vector<PdfElement*>& elements, bool silent) {
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
      if (blockLeft->pos->wMode != 0 || blockLeft->pos->rotation != 0) {
        cut->isChosen = true;
        continue;
      }
    }

    const PdfTextBlock* blockRight = dynamic_cast<const PdfTextBlock*>(cut->elementAfter);
    if (blockRight) {
      if (blockRight->pos->wMode != 0 || blockRight->pos->rotation != 0) {
        cut->isChosen = true;
        continue;
      }
    }

    if (blockLeft && blockRight) {
      // Consider the gap to be a primary x-cut when the rotations of the element to the left and
      // to the right of the cut differ.
      if (blockLeft->pos->wMode != blockRight->pos->wMode) {
        cut->isChosen = true;
        continue;
      }

      // Consider the gap to be a primary x-cut when the writing modes of the element to the left
      // and to the right of the cut differ.
      if (blockLeft->pos->rotation != blockRight->pos->rotation) {
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
    double pageElemsMid = _pageElementsMinY + (_pageElementsMaxY - _pageElementsMinY) / 2.0;

    const PdfElement* elementLeft = cut->elementBefore;
    const PdfNonTextElement* nonTextLeft = dynamic_cast<const PdfNonTextElement*>(elementLeft);
    if (nonTextLeft != nullptr) {
      double upperY = nonTextLeft->pos->upperY;
      double lowerY = nonTextLeft->pos->lowerY;
      double height = nonTextLeft->pos->getHeight();
      // The element must exceed a certain width; one end point must start in the left half of the
      // bounding box around the page elements; and the other end point in the right half.
      if (height > 10 * _doc->avgCharHeight && upperY < pageElemsMid && lowerY > pageElemsMid) {
        cut->isChosen = true;
        continue;
      }
    }

    const PdfElement* elementRight = cut->elementAfter;
    const PdfNonTextElement* nonTextRight = dynamic_cast<const PdfNonTextElement*>(elementRight);
    if (nonTextRight != nullptr) {
      double upperY = nonTextRight->pos->upperY;
      double lowerY = nonTextRight->pos->lowerY;
      double height = nonTextRight->pos->getHeight();
      // The element must exceed a certain width; one end point must start in the left half of the
      // bounding box around the page elements; and the other end point in the right half.
      if (height > 10 * _doc->avgCharHeight && upperY < pageElemsMid && lowerY > pageElemsMid) {
        cut->isChosen = true;
        continue;
      }
    }
  }
}

// _________________________________________________________________________________________________
void ReadingOrderDetector::choosePrimaryYCuts(const vector<Cut*>& cuts,
    const vector<PdfElement*>& elements, bool silent) {
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
    bool isTitleAbove = blockAbove && blockAbove->role == SemanticRole::TITLE;
    bool isAuthorInfoAbove = blockAbove && blockAbove->role == SemanticRole::AUTHOR_INFO;
    bool isMarginalAbove = blockAbove && blockAbove->role == SemanticRole::MARGINAL;

    const PdfTextBlock* blockBelow = dynamic_cast<const PdfTextBlock*>(cut->elementAfter);
    bool isTitleBelow = blockBelow && blockBelow->role == SemanticRole::TITLE;
    bool isAuthorInfoBelow = blockBelow && blockBelow->role == SemanticRole::AUTHOR_INFO;
    bool isMarginalBelow = blockBelow && blockBelow->role == SemanticRole::MARGINAL;

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
    const PdfElement* elementAbove = cut->elementBefore;
    const PdfNonTextElement* nonTextAbove = dynamic_cast<const PdfNonTextElement*>(elementAbove);
    if (nonTextAbove != nullptr) {
      double leftX = nonTextAbove->pos->leftX;
      double rightX = nonTextAbove->pos->rightX;
      double width = nonTextAbove->pos->getWidth();
      // The element must exceed a certain width; one end point must start in the left half of the
      // bounding box around the page elements; and the other end point in the right half.
      if (width > 10 * _doc->avgCharWidth && leftX < pageElementsMid && rightX > pageElementsMid) {
        cut->isChosen = true;
        continue;
      }
    }
    const PdfElement* elementBelow = cut->elementAfter;
    const PdfNonTextElement* nonTextBelow = dynamic_cast<const PdfNonTextElement*>(elementBelow);
    if (nonTextBelow != nullptr) {
      double leftX = nonTextBelow->pos->leftX;
      double rightX = nonTextBelow->pos->rightX;
      double width = nonTextBelow->pos->getWidth();
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
void ReadingOrderDetector::chooseYCuts(const vector<Cut*>& cuts,
    const vector<PdfElement*>& elements, bool silent) {
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
    vector<PdfElement*> group(elements.begin() + cut->posInElements, elements.end());
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
    vector<PdfElement*> group(elements.begin(), elements.begin() + cut->posInElements);
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
      vector<PdfElement*> group(elements.begin() + highCut->posInElements,
          elements.begin() + lowCut->posInElements);
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
