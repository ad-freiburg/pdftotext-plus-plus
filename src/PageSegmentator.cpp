/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // std::max, std::min
#include <functional>  // std::function, std::bind
#include <limits>  // std::numeric_limits
#include <vector>

#include "./utils/Log.h"
#include "./utils/MathUtils.h"
#include "./utils/PdfElementsUtils.h"
#include "./utils/StringUtils.h"
#include "./utils/Trool.h"

#include "./PageSegmentator.h"
#include "./PdfDocument.h"
#include "./XYCut.h"

using std::endl;
using std::max;
using std::min;
using std::vector;

// _________________________________________________________________________________________________
PageSegmentator::PageSegmentator(const PdfDocument* doc, bool debug, int debugPageFilter) {
  _log = new Logger(debug ? DEBUG : INFO, debugPageFilter);
  _doc = doc;

  _minXCutGapWidth = 2 * _doc->mostFreqWordDistance;
  _minYCutGapHeight = 2;
  _maxNumXCutOverlappingElements = 1;
}

// _________________________________________________________________________________________________
PageSegmentator::~PageSegmentator() {
  delete _log;
}

// _________________________________________________________________________________________________
void PageSegmentator::process() {
  assert(_doc);

  _log->debug() << BOLD << "Page Segmentation - DEBUG MODE" << OFF << endl;
  _log->debug() << " └─ min x-cut gap width:  " << _minXCutGapWidth << endl;
  _log->debug() << " └─ min x-cut gap height: " << _minYCutGapHeight << endl;
  _log->debug() << " └─ max num overlapping elements: " << _maxNumXCutOverlappingElements << endl;

  // Do nothing if no pages are given.
  if (_doc->pages.empty()) {
    return;
  }

  // Segment each page separately.
  for (auto* page : _doc->pages) {
    processPage(page, &page->segments);
  }
}

// _________________________________________________________________________________________________
void PageSegmentator::processPage(PdfPage* page, vector<PdfPageSegment*>* segments) {
  assert(page);
  assert(segments);

  // Create a vector with all words, figures, graphics, and shapes of the page.
  vector<PdfElement*> pageElements;
  pageElements.reserve(page->words.size() + page->figures.size()
      + page->graphics.size() +  page->shapes.size());
  for (auto* word : page->words) { pageElements.push_back(word); }
  for (auto* figure : page->figures) { pageElements.push_back(figure); }
  for (auto* graphic : page->graphics) { pageElements.push_back(graphic); }
  for (auto* shape : page->shapes) { pageElements.push_back(shape); }

  int p = page->pageNum;
  _log->debug(p) << "=======================================" << endl;
  _log->debug(p) << BOLD << "PROCESSING PAGE " << p << OFF << endl;
  _log->debug(p) << " └─ # elements: " << pageElements.size() << endl;
  _log->debug(p) << " └─ # words: " << page->words.size() << endl;
  _log->debug(p) << " └─ # figures: " << page->figures.size() << endl;
  _log->debug(p) << " └─ # graphics: " << page->graphics.size() << endl;
  _log->debug(p) << " └─ # shapes: " << page->shapes.size() << endl;

  // Create the binds required to pass the chooseXCuts() and chooseYCuts() methods to xyCut().
  auto chooseXCutsBind = std::bind(&PageSegmentator::chooseXCuts, this,
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  auto chooseYCutsBind = std::bind(&PageSegmentator::chooseYCuts, this,
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

  // Segment the page using the XY-cut algorithm.
  vector<vector<PdfElement*>> groups;
  xyCut(pageElements, _minXCutGapWidth, _minYCutGapHeight, _maxNumXCutOverlappingElements,
      chooseXCutsBind, chooseYCutsBind, false, &groups, &page->blockDetectionCuts);

  // Create a `PdfPageSegment` from each group and append it to the result vector.
  for (const auto& group : groups) {
    createPageSegment(group, segments);
  }
}

// _________________________________________________________________________________________________
void PageSegmentator::chooseXCuts(const vector<Cut*>& cuts, const vector<PdfElement*>& elements,
      bool silent) {
  // Do nothing if no elements are given.
  if (elements.empty()) {
    return;
  }

  int p = elements[0]->position->pageNum;
  if (!silent) {
    _log->debug(p) << "====================" << endl;
    _log->debug(p) << BOLD << "Choosing x-cuts..." << OFF << endl;
    _log->debug(p) << " └─ # elements: " << elements.size() << endl;
    _log->debug(p) << " └─ # cut candidates: " << cuts.size() << endl;
  }

  // Iterate through the cut candidates and choose the cuts that should be actually used.
  Cut* prevChosenCut = nullptr;
  for (Cut* cut : cuts) {
    if (!silent) {
      _log->debug(p) << "--------------------" << endl;
      _log->debug(p) << BOLD << "x-cut: " << cut->id << OFF << endl;
      _log->debug(p) << " └─ cut.pageNum: " << cut->pageNum << endl;
      _log->debug(p) << " └─ cut.x1: " << cut->x1 << endl;
      _log->debug(p) << " └─ cut.y1: " << cut->y1 << endl;
      _log->debug(p) << " └─ cut.x2: " << cut->x2 << endl;
      _log->debug(p) << " └─ cut.y2: " << cut->y2 << endl;
      _log->debug(p) << " └─ cut.gapWidth: " << cut->gapWidth << endl;
      _log->debug(p) << " └─ cut.gapHeight: " << cut->gapHeight << endl;
      _log->debug(p) << " └─ cut.posInElements: " << cut->posInElements << endl;
      _log->debug(p) << " └─ cut.elementBefore: " << cut->elementBefore->toString() << endl;
      _log->debug(p) << " └─ cut.elementAfter: " << cut->elementAfter->toString() << endl;
      _log->debug(p) << " └─ #overlapping elements: " << cut->overlappingElements.size() << endl;
    }

    // Check if to *not* choose the x-cut because there are overlapping elements that are
    // positioned near the top or the bottom of the cut. This should avoid to accidentally divide
    // page headers or -footers that are positioned above or below a multi-column layout.
    Trool res = chooseXCut_overlappingElements(cut, elements);
    if (res == Trool::True) {
      cut->isChosen = true;
      continue;
    } else if (res == Trool::False) {
      cut->isChosen = false;
      continue;
    }

    // Check if to *not* choose the x-cut because its gap width *and* gap height are smaller than
    // a threshold.
    res = chooseXCut_smallGapWidthHeight(cut);
    if (res == Trool::True) {
      cut->isChosen = true;
      continue;
    } else if (res == Trool::False) {
      cut->isChosen = false;
      continue;
    }

    // Check if to *not* choose the x-cut because it divides contiguous words.
    res = chooseXCut_contiguousWords(cut);
    if (res == Trool::True) {
      cut->isChosen = true;
      continue;
    } else if (res == Trool::False) {
      cut->isChosen = false;
      continue;
    }

    // Check if to *not* choose the x-cut because the resulting groups are too slim.
    res = chooseXCut_slimGroups(prevChosenCut, cut, elements);
    if (res == Trool::True) {
      cut->isChosen = true;
      continue;
    } else if (res == Trool::False) {
      cut->isChosen = false;
      continue;
    }

    // Choose the cut, since no rule above was applied.
    cut->isChosen = true;
    prevChosenCut = cut;
  }
}

// _________________________________________________________________________________________________
Trool PageSegmentator::chooseXCut_overlappingElements(const Cut* cut,
    const vector<PdfElement*>& elements, double minNumElements,
    double marginToleranceFactor) const {
  assert(cut);

  // Skip the cut when it does not overlap any elements.
  if (cut->overlappingElements.empty()) {
    return Trool::None;
  }

  // Do not choose the cut when the number of given elements is smaller than the given minimum
  // number of elements.
  if (elements.size() < minNumElements) {
    return Trool::False;
  }

  double marginTolerance = marginToleranceFactor * _doc->avgCharHeight;

  // Do not choose the cut when the top margin (= the distance between the upperY of an element and
  // the upperY of the cut) or the bottom margin (= the distance between the lowerY of the cut and
  // the lowerY of an element) of an overlapping element is smaller than the tolerance.
  for (const auto* element : cut->overlappingElements) {
    // Compute the top margin and bottom margin.
    double topMargin = element->position->upperY - cut->y1;
    double bottomMargin = cut->y2 - element->position->lowerY;

    if (topMargin < marginTolerance || bottomMargin < marginTolerance) {
      return Trool::False;
    }
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool PageSegmentator::chooseXCut_smallGapWidthHeight(const Cut* cut, double widthThresholdFactor,
      double heightThresholdFactor) const {
  assert(cut);

  double wThreshold = widthThresholdFactor * _doc->avgCharWidth;
  double hThreshold = heightThresholdFactor * _doc->avgCharHeight;

  if (cut->gapWidth < wThreshold && cut->gapHeight < hThreshold) {
    return Trool::False;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool PageSegmentator::chooseXCut_contiguousWords(const Cut* cut) const {
  assert(cut);

  // Check if the elements are words, by casting them to `PdfWord` objects.
  // An object will be null, if it is not a word.
  const PdfWord* wordLeft = dynamic_cast<const PdfWord*>(cut->elementBefore);
  const PdfWord* wordRight = dynamic_cast<const PdfWord*>(cut->elementAfter);

  if (!wordLeft || !wordRight) {
    return Trool::None;
  }

  // if (!silent) {
  //   _log->debug(p) << "Checking contiguousness..." << endl;
  //   _log->debug(p) << " └─ wordLeft.rank: " << wordLeft->rank << endl;
  //   _log->debug(p) << " └─ wordRight.rank: " << wordRight->rank << endl;
  //   _log->debug(p) << " └─ y-ratios: " << ratios.first << ", " << ratios.second << endl;
  // }

  bool isContiguous = true;

  // The words are not contiguous, if they are not neighbors in the extraction order.
  if (wordLeft->rank + 1 != wordRight->rank) {
    isContiguous = false;
  }

  // The words are not contiguous, if they do not share the same text line (= if they do
  // not overlap vertically).
  double maxYOverlapRatio = element_utils::computeMaxYOverlapRatio(wordLeft, wordRight);
  if (math_utils::smaller(maxYOverlapRatio, 0.1)) {
    isContiguous = false;
  }

  return isContiguous ? Trool::False : Trool::None;
}

// _________________________________________________________________________________________________
Trool PageSegmentator::chooseXCut_slimGroups(const Cut* prevChosenCut, const Cut* cut,
    const vector<PdfElement*>& elements, double widthThresholdFactor) const {
  assert(cut);

  double widthThreshold = widthThresholdFactor * _doc->avgCharWidth;

  // Compute the width of the resulting left group.
  const PdfElement* leftGroupFirstElem = prevChosenCut ? prevChosenCut->elementAfter : elements[0];
  double leftGroupMinX = leftGroupFirstElem->position->leftX;
  double leftGroupMaxX = cut->elementBefore->position->rightX;
  double leftGroupWidth = leftGroupMaxX - leftGroupMinX;

  // if (!silent) {
  //   _log->debug(p) << "Checking left group width..." << endl;
  //   _log->debug(p) << " └─ prevElementRight: " << prevElementRight->toString() << endl;
  //   _log->debug(p) << " └─ leftGroup.width: " << leftGroupWidth << endl;
  //   _log->debug(p) << " └─ threshold: " << 10 * widthThreshold << endl;
  // }

  if (leftGroupWidth < widthThreshold) {
    // if (!silent) {
    //   _log->debug(p) << "\033[1mCut not chosen (left group width too small).\033[0m" << endl;
    // }
    return Trool::False;
  }

  // Compute the width of the resulting right group.
  double rightGroupMinX = cut->elementAfter->position->leftX;
  // TODO(korzen): The elements are sorted by leftX, so the last element isn't necessarily the
  // element with the largest rightX in the right group.
  double rightGroupMaxX = elements[elements.size() - 1]->position->rightX;
  double rightGroupWidth = rightGroupMaxX - rightGroupMinX;

  // if (!silent) {
  //   _log->debug(p) << "Checking right group width..." << endl;
  //   _log->debug(p) << " └─ rightGroup.width: " << rightGroupWidth << endl;
  //   _log->debug(p) << " └─ threshold: " << 10 * _doc->avgCharWidth << endl;
  // }

  if (rightGroupWidth < widthThreshold) {
    // if (!silent) {
    //   _log->debug(p) << "\033[1mCut not chosen (right group width too small).\033[0m" << endl;
    // }
    return Trool::False;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
void PageSegmentator::chooseYCuts(const vector<Cut*>& cuts, const vector<PdfElement*>& elements,
      bool silent) {
  // Do nothing if no cuts are given.
  if (cuts.empty()) {
    return;
  }

  // Do nothing if no elements are given.
  if (elements.empty()) {
    return;
  }

  // Create the bind required to pass the chooseXCuts() method to xCut().
  auto chooseXCutsBind = std::bind(&PageSegmentator::chooseXCuts, this,
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

  // Add two "sentinel cuts", representing the top boundary and the bottom boundary of the page, to
  // the vector of cuts. They are not an actual part of the choosable cuts. Their purpose is to
  // make the code for choosing the y-cuts more compact (and also more readable).
  Cut topCut(CutDir::Y, 0);
  Cut bottomCut(CutDir::Y, elements.size());
  vector<Cut*> ccuts;
  ccuts.push_back(&topCut);
  for (auto* cut : cuts) { ccuts.push_back(cut); }
  ccuts.push_back(&bottomCut);



  // Iterate through the cuts and find a partner cut for each.
  for (size_t cutIdx = 0; cutIdx < ccuts.size(); cutIdx++) {
    Cut* cut = ccuts[cutIdx];

    Cut* partnerCut = nullptr;
    for (size_t otherCutIdx = cutIdx + 1; otherCutIdx < ccuts.size(); otherCutIdx++) {
      Cut* otherCut = ccuts[otherCutIdx];

      size_t beginPos = cut->posInElements;
      size_t endPos = otherCut->posInElements;
      vector<PdfElement*> elems(elements.begin() + beginPos, elements.begin() + endPos);

      // Abort the search for a partner cut, when the elements can't be divided by an x-cut.
      if (!xCut(elems, _minXCutGapWidth, _maxNumXCutOverlappingElements, chooseXCutsBind, true)) {
        break;
      }

      partnerCut = otherCut;
      cutIdx = otherCutIdx;
    }

    if (partnerCut) {
      cut->isChosen = true;
      partnerCut->isChosen = true;
    }
  }
}

// _________________________________________________________________________________________________
void PageSegmentator::createPageSegment(const vector<PdfElement*>& elements,
    vector<PdfPageSegment*>* segments) const {
  // Do nothing if no elements are given.
  if (elements.empty()) {
    return;
  }

  PdfPageSegment* segment = new PdfPageSegment();
  segment->doc = _doc;

  // Create a (unique) id.
  segment->id = string_utils::createRandomString(8, "ps-");

  // Set the page number.
  segment->position->pageNum = elements[0]->position->pageNum;

  // Compute and set the coordinates of the bounding box.
  for (const auto* element : elements) {
    segment->position->leftX = min(segment->position->leftX, element->position->leftX);
    segment->position->upperY = min(segment->position->upperY, element->position->upperY);
    segment->position->rightX = max(segment->position->rightX, element->position->rightX);
    segment->position->lowerY = max(segment->position->lowerY, element->position->lowerY);
  }

  // Set the elements.
  segment->elements = elements;

  segments->push_back(segment);
}
