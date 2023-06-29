/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <utility>  // std::pair
#include <vector>

#include "./utils/Log.h"
#include "./utils/Math.h"
#include "./utils/PageSegmentationUtils.h"
#include "./utils/PdfElementsUtils.h"
#include "./utils/Trool.h"
#include "./Config.h"
#include "./PageSegmentation.h"
#include "./PdfDocument.h"
#include "./XYCut.h"

using std::bind;
using std::endl;
using std::pair;
using std::vector;

using ppp::config::PageSegmentationConfig;
using ppp::utils::elements::computeMaxYOverlapRatio;
using ppp::utils::log::Logger;
using ppp::utils::log::BLUE;
using ppp::utils::log::BOLD;
using ppp::utils::log::OFF;
using ppp::utils::math::smaller;
using ppp::utils::PageSegmentationUtils;

// _________________________________________________________________________________________________
PageSegmentation::PageSegmentation(PdfDocument* doc, const PageSegmentationConfig& config) {
  _doc = doc;
  _config = config;
  _utils = new PageSegmentationUtils(config);
  _log = new Logger(config.logLevel, config.logPageFilter);
}

// _________________________________________________________________________________________________
PageSegmentation::~PageSegmentation() {
  delete _log;
  delete _utils;
}

// _________________________________________________________________________________________________
void PageSegmentation::process() {
  assert(_doc);

  _log->info() << "Segmenting the pages..." << endl;
  _log->debug() << "=======================================" << endl;
  _log->debug() << BOLD << "DEBUG MODE" << OFF << endl;

  // Segment each page separately.
  for (auto* page : _doc->pages) {
    processPage(page, &page->segments);
  }

  _log->debug() << "=======================================" << endl;
}

// _________________________________________________________________________________________________
void PageSegmentation::processPage(PdfPage* page, vector<PdfPageSegment*>* segments) {
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
  _log->debug(p) << BOLD << "page " << p << OFF << endl;
  _log->debug(p) << " └─ # elements: " << pageElements.size() << endl;
  _log->debug(p) << " └─ # words: " << page->words.size() << endl;
  _log->debug(p) << " └─ # figures: " << page->figures.size() << endl;
  _log->debug(p) << " └─ # graphics: " << page->graphics.size() << endl;
  _log->debug(p) << " └─ # shapes: " << page->shapes.size() << endl;

  // Create the binds required to pass chooseXCuts() and chooseYCuts() of this class to xyCut().
  auto chooseXCutsBind = bind(&PageSegmentation::chooseXCuts, this,
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  auto chooseYCutsBind = bind(&PageSegmentation::chooseYCuts, this,
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

  // Segment the page using the XY-cut algorithm.
  vector<vector<PdfElement*>> groups;
  xyCut(pageElements,
      _config.getXCutMinGapWidth(_doc),
      _config.getYCutMinGapHeight(_doc),
      _config.xCutMaxNumOverlappingElements,
      chooseXCutsBind,
      chooseYCutsBind,
      false,
      &groups,
      &page->blockDetectionCuts);

  // Create a `PdfPageSegment` from each group and append it to the result vector.
  for (const auto& group : groups) {
    PdfPageSegment* segment = _utils->createPageSegment(group, _doc);
    segments->push_back(segment);
  }
}

// _________________________________________________________________________________________________
void PageSegmentation::chooseXCuts(const vector<Cut*>& cuts, const vector<PdfElement*>& elements,
      bool silent) {
  // Do nothing if no elements are given.
  if (elements.empty()) {
    return;
  }

  int p = elements[0]->pos->pageNum;
  if (!silent) {
    _log->debug(p) << "---------------------------------------" << endl;
    _log->debug(p) << BOLD << "Choosing x-cuts..." << OFF << endl;
    _log->debug(p) << " └─ # elements: " << elements.size() << endl;
    _log->debug(p) << " └─ # cut candidates: " << cuts.size() << endl;
  }

  // Iterate through the cut candidates and choose the cuts that should be actually used.
  Cut* prevChosenCut = nullptr;
  for (size_t i = 0; i < cuts.size(); i++) {
    Cut* cut = cuts[i];

    if (!silent) {
      _log->debug(p) << "--------------------" << endl;
      _log->debug(p) << BOLD << "x-cut #" << (i + 1) << OFF << endl;
      _log->debug(p) << " └─ cut.id: " << cut->id << endl;
      _log->debug(p) << " └─ cut.pageNum: " << cut->pageNum << endl;
      _log->debug(p) << " └─ cut.x1: " << cut->x1 << endl;
      _log->debug(p) << " └─ cut.y1: " << cut->y1 << endl;
      _log->debug(p) << " └─ cut.x2: " << cut->x2 << endl;
      _log->debug(p) << " └─ cut.y2: " << cut->y2 << endl;
      _log->debug(p) << " └─ cut.gapWidth: " << cut->gapWidth << endl;
      _log->debug(p) << " └─ cut.gapHeight: " << cut->gapHeight << endl;
      _log->debug(p) << " └─ cut.posInElements: " << cut->posInElements << endl;
      _log->debug(p) << " └─ cut.elementBefore: " << cut->elementBefore->toShortString() << endl;
      _log->debug(p) << " └─ cut.elementAfter:  " << cut->elementAfter->toShortString() << endl;
      _log->debug(p) << " └─ #overlapping elements: " << cut->overlappingElements.size() << endl;
    }

    // Check if to *not* choose the x-cut because there are overlapping elements that are
    // positioned near the top or the bottom of the cut. This should avoid to accidentally divide
    // page headers or -footers that are positioned above or below a multi-column layout.
    Trool res = chooseXCut_overlappingElements(cut, elements, silent);
    if (res == Trool::False) {
      cut->isChosen = false;
      continue;
    }

    // Check if to *not* choose the x-cut because its gap width *and* gap height are smaller than
    // a threshold.
    res = chooseXCut_smallGapWidthHeight(cut, silent);
    if (res == Trool::False) {
      cut->isChosen = false;
      continue;
    }

    // Check if to *not* choose the x-cut because it divides contiguous words.
    res = chooseXCut_contiguousWords(cut, elements, silent);
    if (res == Trool::False) {
      cut->isChosen = false;
      continue;
    }

    // Check if to *not* choose the x-cut because the resulting groups are too slim.
    res = chooseXCut_slimGroups(prevChosenCut, cut, elements, silent);
    if (res == Trool::False) {
      cut->isChosen = false;
      continue;
    }

    // Choose the cut, since no rule from above was applied.
    cut->isChosen = true;
    prevChosenCut = cut;
    if (!silent) {
      _log->debug(p) << BLUE << BOLD << " no rule applied → choose cut" << OFF << endl;
    }
  }
}

// _________________________________________________________________________________________________
Trool PageSegmentation::chooseXCut_overlappingElements(const Cut* cut, const vector<PdfElement*>&
      elements, bool silent) const {
  assert(cut);

  int p = cut->pageNum;
  double marginThreshold = _config.getOverlappingElementsMarginThreshold(_doc);

  if (!silent) {
    _log->debug(p) << BLUE << "Are there overlapping elements at the top/bottom?" << OFF << endl;
    _log->debug(p) << " └─ #overlappingElements: " << cut->overlappingElements.size() << endl;
    _log->debug(p) << " └─ #elements: " << elements.size() << endl;
    _log->debug(p) << " └─ numElementsThreshold: " << _config.overlappingMinNumElements << endl;
    _log->debug(p) << " └─ marginThreshold: " << marginThreshold << endl;
  }

  // Skip the cut when it does not overlap any elements.
  if (cut->overlappingElements.empty()) {
    return Trool::None;
  }

  // Do not choose the cut when the number of elements is smaller than the threshold.
  if (elements.size() < _config.overlappingMinNumElements) {
    if (!silent) {
      _log->debug(p) << BLUE << BOLD << " #elements < threshold → do not choose" << OFF << endl;
    }
    return Trool::False;
  }

  // Do not choose the cut when the top margin (= the distance between the upperY of an element and
  // the upperY of the cut) or the bottom margin (= the distance between the lowerY of the cut and
  // the lowerY of an element) of an overlapping element is smaller than the threshold.
  for (const auto* element : cut->overlappingElements) {
    double topMargin = element->pos->upperY - cut->y1;
    double bottomMargin = cut->y2 - element->pos->lowerY;

    if (smaller(topMargin, marginThreshold)) {
      if (!silent) {
        _log->debug(p) << BLUE << BOLD << " yes → do not choose" << OFF << endl;
        _log->debug(p) << "  └─ element: " << element->toShortString() << endl;
        _log->debug(p) << "  └─ element.topMargin: " << topMargin << endl;
      }
      return Trool::False;
    }

    if (smaller(bottomMargin, marginThreshold)) {
      if (!silent) {
        _log->debug(p) << BLUE << BOLD << " yes → do not choose" << OFF << endl;
        _log->debug(p) << "  └─ element: " << element->toShortString() << endl;
        _log->debug(p) << "  └─ element.bottomMargin: " << bottomMargin << endl;
      }
      return Trool::False;
    }
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool PageSegmentation::chooseXCut_smallGapWidthHeight(const Cut* cut, bool silent) const {
  assert(cut);

  int p = cut->pageNum;
  pair<double, double> thresholds = _config.getSmallGapWidthHeightThresholds(_doc);
  double wThreshold = thresholds.first;
  double hThreshold = thresholds.second;

  if (!silent) {
    _log->debug(p) << BLUE << "Are the width and height of the gap too small?" << OFF << endl;
    _log->debug(p) << " └─ cut.gapWidth: " << cut->gapWidth << endl;
    _log->debug(p) << " └─ threshold gapWidth: " << wThreshold << endl;
    _log->debug(p) << " └─ cut.gapHeight: " << cut->gapHeight << endl;
    _log->debug(p) << " └─ threshold gapHeight: " << hThreshold << endl;
  }

  if (smaller(cut->gapWidth, wThreshold)
        && smaller(cut->gapHeight, hThreshold)) {
    _log->debug(p) << BLUE << BOLD << " si → do not choose" << OFF << endl;
    return Trool::False;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool PageSegmentation::chooseXCut_contiguousWords(const Cut* cut,
      const vector<PdfElement*>& elements, bool silent) const {
  assert(cut);

  // Determine the rightmost word to the left of the cut.
  int p = cut->pageNum;
  const PdfWord* leftWord = dynamic_cast<const PdfWord*>(cut->elementBefore);
  double yOverlapRatioThreshold = _config.contiguousWordsYOverlapRatioThreshold;

  if (!silent) {
    _log->debug(p) << BLUE << "Does the cut divide contiguous words?" << OFF << endl;
    _log->debug(p) << " └─ leftWord: " << (leftWord ? leftWord->toShortString() : "-") << endl;
    _log->debug(p) << " └─ leftWord.rank: " << (leftWord ? leftWord->rank : -1) << endl;
  }

  if (!leftWord) {
    return Trool::None;
  }

  // Iterate through the elements to the right of the cut. Check if there is a word with rank
  // `leftWord.rank + 1` which vertically overlaps `leftWord`. If so, do not choose the cut, since
  // there is a pair of words that are contiguous.
  for (size_t i = cut->posInElements; i < elements.size(); i++) {
    const PdfWord* rightWord = dynamic_cast<const PdfWord*>(elements[i]);

    // Skip the element if it is not a word.
    if (!rightWord) {
      continue;
    }

    // Skip the word if it is not a neighbor of `leftWord` in the extraction order.
    if (leftWord->rank + 1 != rightWord->rank) {
      continue;
    }

    // Skip the word if the max y-overlap ratio between the word and `leftWord` is < the threshold.
    double maxYOverlapRatio = computeMaxYOverlapRatio(leftWord, rightWord);
    if (!silent) {
      _log->debug(p) << " └─ rightWord: " << rightWord->toShortString() << endl;
      _log->debug(p) << " └─ rightWord.rank: " << rightWord->rank << endl;
      _log->debug(p) << " └─ max y-overlap ratio: " << maxYOverlapRatio << endl;
      _log->debug(p) << " └─ max y-overlap ratio threshold: " << yOverlapRatioThreshold << endl;
    }
    if (smaller(maxYOverlapRatio, yOverlapRatioThreshold)) {
      continue;
    }

    // The `rightWord` and `leftWord` are contiguous.
    if (!silent) { _log->debug(p) << BLUE << BOLD << " yes → do not choose" << OFF << endl; }
    return Trool::False;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool PageSegmentation::chooseXCut_slimGroups(const Cut* prevChosenCut, const Cut* cut,
    const vector<PdfElement*>& elements, bool silent) const {
  assert(cut);

  // Do nothing if no elements are given.
  if (elements.empty()) {
    return Trool::None;
  }

  int p = cut->pageNum;
  double widthThreshold = _config.getSlimGroupWidthThreshold(_doc);

  // Compute the width of the resulting left group.
  const PdfElement* leftGroupFirstElem = prevChosenCut ? prevChosenCut->elementAfter : elements[0];
  const PdfElement* leftGroupLastElem = cut->elementBefore;
  double leftGroupMinX = leftGroupFirstElem->pos->leftX;
  double leftGroupMaxX = leftGroupLastElem->pos->rightX;
  double leftGroupWidth = leftGroupMaxX - leftGroupMinX;

  if (!silent) {
    _log->debug(p) << BLUE << "Is the width of one resulting group too small?" << OFF << endl;
    _log->debug(p) << " └─ leftGroup.firstElem: " << leftGroupFirstElem->toShortString() << endl;
    _log->debug(p) << " └─ leftGroup.lastElem:  " << leftGroupLastElem->toShortString() << endl;
    _log->debug(p) << " └─ leftGroup.width: " << leftGroupWidth << endl;
    _log->debug(p) << " └─ threshold: " << widthThreshold << endl;
  }

  if (smaller(leftGroupWidth, widthThreshold)) {
    if (!silent) {
      _log->debug(p) << BLUE << BOLD << " yes (leftGroup) → do not choose" << OFF << endl;
    }
    return Trool::False;
  }

  // Compute the width of the resulting right group.
  const PdfElement* rightGroupFirstElem = cut->elementAfter;
  // TODO(korzen): The elements are sorted by leftX, so the last element isn't necessarily the
  // element with the largest rightX in the right group.
  const PdfElement* rightGroupLastElem = elements[elements.size() - 1];
  double rightGroupMinX = rightGroupFirstElem->pos->leftX;
  double rightGroupMaxX = rightGroupLastElem->pos->rightX;
  double rightGroupWidth = rightGroupMaxX - rightGroupMinX;

  if (!silent) {
    _log->debug(p) << " └─ rightGroup.firstElem: " << rightGroupFirstElem->toShortString() << endl;
    _log->debug(p) << " └─ rightGroup.lastElem: " << rightGroupLastElem->toShortString() << endl;
    _log->debug(p) << " └─ rightGroup.width: " << rightGroupWidth << endl;
    _log->debug(p) << " └─ threshold: " << widthThreshold << endl;
  }

  if (smaller(rightGroupWidth, widthThreshold)) {
    if (!silent) {
      _log->debug(p) << BLUE << BOLD << " yes (rightGroup) → do not choose" << OFF << endl;
    }
    return Trool::False;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
void PageSegmentation::chooseYCuts(const vector<Cut*>& cuts, const vector<PdfElement*>& elements,
      bool silent) {
  // Do nothing if no cuts are given.
  if (cuts.empty()) {
    return;
  }

  // Do nothing if no elements are given.
  if (elements.empty()) {
    return;
  }

  int p = elements[0]->pos->pageNum;
  if (!silent) {
    _log->debug(p) << "---------------------------------------" << endl;
    _log->debug(p) << BOLD << "Choosing y-cuts..." << OFF << endl;
    _log->debug(p) << " └─ # elements: " << elements.size() << endl;
    _log->debug(p) << " └─ # cut candidates: " << cuts.size() << endl;
    _log->debug(p) << "--------------------" << endl;
  }

  // Create the bind required to pass the chooseXCuts() method to xCut().
  auto chooseXCutsBind = bind(&PageSegmentation::chooseXCuts, this,
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

  // Add two "sentinel cuts", representing the top boundary and the bottom boundary of the page, to
  // the vector of cuts. They are not an actual part of the choosable cuts. Their purpose is to
  // make the code below for choosing the y-cuts more compact (and also more readable).
  Cut topCut(CutDir::Y, "(top)", 0);
  Cut bottomCut(CutDir::Y, "(bottom)", elements.size());
  vector<Cut*> ccuts;
  ccuts.push_back(&topCut);
  for (auto* cut : cuts) { ccuts.push_back(cut); }
  ccuts.push_back(&bottomCut);

  // Iterate through the cuts and find a partner cut for each.
  for (size_t idx = 0; idx < ccuts.size(); idx++) {
    Cut* cut = ccuts[idx];

    if (!silent) {
      _log->debug(p) << BLUE << "y-cut #" << (idx + 1)
          << ": id: " << cut->id << "; page: " << cut->pageNum << "; x1: " << cut->x1
          << "; y1: " << cut->y1 << "; x2: " << cut->x2 << "; y2: " << cut->y2 << OFF << endl;
    }

    Cut* partnerCut = nullptr;
    for (size_t otherIdx = idx + 1; otherIdx < ccuts.size(); otherIdx++) {
      Cut* otherCut = ccuts[otherIdx];

      size_t beginPos = cut->posInElements;
      size_t endPos = otherCut->posInElements;
      vector<PdfElement*> elems(elements.begin() + beginPos, elements.begin() + endPos);

      bool cutOk = xCut(elems,
          _config.getXCutMinGapWidth(_doc),
          _config.xCutMaxNumOverlappingElements,
          chooseXCutsBind,
          true);

      if (!silent) {
        _log->debug(p) << " other y-cut #" << (otherIdx + 1)
            << ": id: " << otherCut->id << "; page: " << otherCut->pageNum
            << "; x1: " << otherCut->x1 << "; y1: " << otherCut->y1
            << "; x2: " << otherCut->x2 << "; y2: " << otherCut->y2
            << BOLD << " → cutOk: " << (cutOk ? "yes" : "no") << OFF << endl;
      }

      // Abort the search for a partner cut, when the elements can't be divided by an x-cut.
      if (!cutOk) {
        break;
      }

      partnerCut = otherCut;
      idx = otherIdx;
    }

    if (partnerCut) {
      cut->isChosen = true;
      partnerCut->isChosen = true;
      if (!silent) {
        _log->debug(p) << BOLD << "choose " << cut->id << " + " << partnerCut->id << OFF << endl;
      }
    }
  }
}
