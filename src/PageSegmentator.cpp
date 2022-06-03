/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <iostream>
#include <algorithm>  // std::sort
#include <cmath>  // round, fabs
#include <functional>  // std::function, std::bind
#include <iostream>  // std::cout
#include <limits>  // std::numeric_limits

#include "./PdfDocument.h"
#include "./PageSegmentator.h"
#include "./utils/Log.h"
#include "./utils/PdfElementsUtils.h"
#include "./utils/StringUtils.h"
#include "./XYCut.h"

const int maxNumCuttingElements = 1;

// _________________________________________________________________________________________________
PageSegmentator::PageSegmentator(PdfDocument* doc, bool debug, int debugPageFilter) {
  _doc = doc;
  _minXCutGapWidth = 2 * _doc->mostFreqWordDistance;
  // _minYCutGapHeight = 2 * _doc->mostFreqEstimatedLineDistance;
  _minYCutGapHeight = 2;
  _log = new Logger(debug ? DEBUG : INFO, debugPageFilter);

  _log->debug() << "=======================================" << std::endl;
  _log->debug() << "\033[1mDEBUG MODE | Page Segmentation\033[0m" << std::endl;
  _log->debug() << " └─ min gap width:  " << _minXCutGapWidth << std::endl;
  _log->debug() << " └─ min gap height: " << _minYCutGapHeight << std::endl;
  _log->debug() << "=======================================" << std::endl;
}

// _________________________________________________________________________________________________
PageSegmentator::~PageSegmentator() {
  delete _log;
}

// _________________________________________________________________________________________________
void PageSegmentator::segment() {
  // Do nothing if no document is given.
  if (!_doc) {
    return;
  }

  // Do nothing if no pages are given.
  if (_doc->pages.size() == 0) {
    return;
  }

  for (auto* page : _doc->pages) {
    segmentPage(page, &page->segments);
  }
}

// _________________________________________________________________________________________________
void PageSegmentator::segmentPage(PdfPage* page, std::vector<PdfPageSegment*>* segments) {
  // The binds required to pass the chooseXCuts() and chooseYCuts() methods to the XY-cut class.
  // auto choosePrimaryYCutsBind = std::bind(&PageSegmentator::choosePrimaryYCuts, this,
  //     std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  auto chooseXCutsBind = std::bind(&PageSegmentator::chooseXCuts, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  auto chooseYCutsBind = std::bind(&PageSegmentator::chooseYCuts, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

  int p = page->pageNum;
  _log->debug(p) << "=======================================" << std::endl;
  _log->debug(p) << "\033[1mPROCESSING PAGE " << p << "\033[0m" << std::endl;

  // Create a list with all words, figures, shapes.
  std::vector<PdfElement*> pageElements;
  pageElements.reserve(page->words.size() + page->figures.size() + page->shapes.size());
  for (auto* word : page->words) { pageElements.push_back(word); }
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

  _log->debug(p) << " └─ # elements: " << pageElements.size() << std::endl;
  _log->debug(p) << " └─ # words: " << page->words.size() << std::endl;
  _log->debug(p) << " └─ # figures: " << page->figures.size() << std::endl;
  _log->debug(p) << " └─ # shapes: " << page->shapes.size() << std::endl;
  _log->debug(p) << " └─ bbox elements: leftX: " << _pageElementsMinX
      << "; upperY: " << _pageElementsMinY
      << "; rightX: " << _pageElementsMaxX
      << "; lowerY: " << _pageElementsMaxY << std::endl;

  std::vector<std::vector<PdfElement*>> groups;

  xyCut(pageElements, chooseXCutsBind, chooseYCutsBind, _minXCutGapWidth, _minYCutGapHeight,
      maxNumCuttingElements, false, &groups, &page->blockDetectionCuts);

  // Create a text segment from each group and append it to the result list.
  for (const auto& group : groups) {
    createPageSegment(group, segments);
  }
}

// _________________________________________________________________________________________________
void PageSegmentator::chooseXCuts(const std::vector<PdfElement*>& elements,
      std::vector<Cut*>& cuts, bool silent) {
  // Do nothing if no elements are given.
  if (elements.empty()) {
    return;
  }

  int p = elements[0]->position->pageNum;
  if (!silent) {
    _log->debug(p) << "====================" << std::endl;
    _log->debug(p) << "\033[1mChoosing x-cuts...\033[0m" << std::endl;
    _log->debug(p) << " └─ # elements: " << elements.size() << std::endl;
    _log->debug(p) << " └─ # cut candidates: " << cuts.size() << std::endl;
  }

  // Iterate through the given cuts and "choose" the valid cuts.
  Cut* prevChosenCut = nullptr;
  for (Cut* cut : cuts) {
    if (!silent) {
      _log->debug(p) << "--------------------" << std::endl;
      _log->debug(p) << "\033[1mx-cut: " << cut->id << "\033[0m" << std::endl;
      _log->debug(p) << " └─ cut.pageNum: " << cut->pageNum << std::endl;
      _log->debug(p) << " └─ cut.x1: " << cut->x1 << std::endl;
      _log->debug(p) << " └─ cut.y1: " << cut->y1 << std::endl;
      _log->debug(p) << " └─ cut.x2: " << cut->x2 << std::endl;
      _log->debug(p) << " └─ cut.y2: " << cut->y2 << std::endl;
      _log->debug(p) << " └─ cut.gapWidth: " << cut->gapWidth << std::endl;
      _log->debug(p) << " └─ cut.gapHeight: " <<cut->gapHeight << std::endl;
      _log->debug(p) << " └─ cut.posInElements: " << cut->posInElements << std::endl;
      _log->debug(p) << " └─ cut.elementBefore: " << cut->elementBefore->toString() << std::endl;
      _log->debug(p) << " └─ cut.elementAfter: " << cut->elementAfter->toString() << std::endl;
      _log->debug(p) << " └─ # cutting elements: " << cut->cuttingElements.size() << std::endl;
    }

    if (!silent) {
      _log->debug(p) << "Checking gap height / gap width ratio..." << std::endl;
      _log->debug(p) << " └─ cut.gapWidth: " << cut->gapWidth << std::endl;
      _log->debug(p) << " └─ cut.gapHeight: " << cut->gapHeight << std::endl;
      _log->debug(p) << " └─ _doc->avgGlyphHeight: " << _doc->avgGlyphHeight << std::endl;
      _log->debug(p) << " └─ _doc->avgGlyphWidth: " << _doc->avgGlyphWidth << std::endl;
    }


    if (cut->cuttingElements.size() > 0) {
      // Only allow cutting elements when the number of elements exceeds a threshold.
      if (elements.size() < 500) {
        continue;
      }

      bool x = false;
      for (PdfElement* element : cut->cuttingElements) {
        if (cut->y2 - element->position->lowerY < 5 * _doc->avgGlyphHeight) {
          x = true;
          break;
        }
        if (element->position->upperY - cut->y1 < 5 * _doc->avgGlyphHeight) {
          x = true;
        }
      }
      if (x) {
        continue;
      }
    }

    // Check if the elements are words, by casting them to `PdfWord` objects.
    // An object will be null, if it is not a word.
    PdfWord* wordLeft = dynamic_cast<PdfWord*>(cut->elementBefore);
    PdfWord* wordRight = dynamic_cast<PdfWord*>(cut->elementAfter);

      double hThreshold = 3 * _doc->avgGlyphHeight;
      double wThreshold = 3 * _doc->avgGlyphWidth;
      if (!silent) {
        _log->debug(p) << "h/w threshold: " << hThreshold << ", " << wThreshold << std::endl;
      }

      if (cut->gapHeight < hThreshold && cut->gapWidth < wThreshold) {
        if (!silent) {
          _log->debug(p) << "\033[1mCut not chosen (h/w ratio too small).\033[0m" << std::endl;
        }
        continue;
      }

      hThreshold = 6 * _doc->avgGlyphHeight;
      wThreshold = 2 * _doc->avgGlyphWidth;
      if (!silent) {
        _log->debug(p) << "h/w threshold: " << hThreshold << ", " << wThreshold << std::endl;
      }

      if (cut->gapHeight < hThreshold && cut->gapWidth < wThreshold) {
        if (!silent) {
          _log->debug(p) << "\033[1mCut not chosen (h/w ratio too small).\033[0m" << std::endl;
        }
        continue;
      }

    if (wordLeft && wordRight) {
      // ------------
      // The gap is *not* a valid cut position when the left word and the right word are
      // *contiguous*, that is: if the right word immediately follows behind the left word in the
      // extraction order and if both words share the same text line. This rule exists to not
      // accidentally divide the words of a title when a word boundary within the title coincide
      // with a column boundary, as shown in the following example:
      // THIS  IS  | THE  TITLE
      //           |
      // XXXXXXXXX | XXXXXXXXXX
      // XXXXXXXXX | XXXXXXXXXX
      // XXXXXXXXX | XXXXXXXXXX

      std::pair<double, double> ratios = element_utils::computeYOverlapRatios(wordLeft, wordRight);

      if (!silent) {
        _log->debug(p) << "Checking contiguousness..." << std::endl;
        _log->debug(p) << " └─ wordLeft.rank: " << wordLeft->rank << std::endl;
        _log->debug(p) << " └─ wordRight.rank: " << wordRight->rank << std::endl;
        _log->debug(p) << " └─ y-ratios: " << ratios.first << ", " << ratios.second << std::endl;
      }

      bool isContiguous = true;
      // The words are not contiguous, if they are not neighbors in the extraction order.
      if (wordLeft->rank + 1 != wordRight->rank) {
        isContiguous = false;
      }
      // The words are not contiguous, if they do not share the same text line (= if they do
      // not overlap vertically).
      if (std::max(ratios.first, ratios.second) < 0.1) {
        isContiguous = false;
      }
      if (isContiguous) {
        if (!silent) {
          _log->debug(p) << "\033[1mCut not chosen (words are contiguous).\033[0m" << std::endl;
        }
        continue;
      }
    }

    // ------------
    // The gap is *not* a valid cut position when the width of one of the resulting groups is too
    // small. Here are two examples to explain why this rule exists:
    //
    // (1) In a bibliography, there could be a vertical gap between the reference anchors and the
    // reference bodies, like illustrated in the following:
    // [1]   W. Smith et al: Lorem ipsum ...
    // [2]   F. Miller et al: Lorem ipsum ...
    // [3]   T. Redford et al: Lorem ipsum ...
    // The reference anchors should *not* be separated from the reference bodies.
    //
    // (2) A formula could have a numbering, with a (large) vertical gap in between, like
    // illustrated in the following example:
    // x + y = z     (1)
    // The numbering should *not* be separated from the formula.

    // Compute the index of the closest element on the right side of the last chosen cut
    // (0 if no cut was chosen yet). This will be used to compute minX of the left group.
    PdfElement* prevElementRight = prevChosenCut ? prevChosenCut->elementAfter : elements[0];
    double leftGroupMinX = prevElementRight->position->leftX;
    double leftGroupMaxX = cut->elementBefore->position->rightX;
    double leftGroupWidth = leftGroupMaxX - leftGroupMinX;

    if (!silent) {
      _log->debug(p) << "Checking left group width..." << std::endl;
      _log->debug(p) << " └─ prevElementRight: " << prevElementRight->toString() << std::endl;
      _log->debug(p) << " └─ leftGroup.width: " << leftGroupWidth << std::endl;
      _log->debug(p) << " └─ threshold: " << 10 * _doc->avgGlyphWidth << std::endl;
    }

    if (leftGroupWidth < 10 * _doc->avgGlyphWidth) {
      if (!silent) {
        _log->debug(p) << "\033[1mCut not chosen (left group width too small).\033[0m" << std::endl;
      }
      continue;
    }

    double rightGroupMinX = cut->elementAfter->position->leftX;
    // TODO(korzen): The elements are sorted by leftX, so the last element isn't necessarily the
    // element with the largest rightX value in the right group.
    double rightGroupMaxX = elements[elements.size() - 1]->position->rightX;
    double rightGroupWidth = rightGroupMaxX - rightGroupMinX;

    if (!silent) {
      _log->debug(p) << "Checking right group width..." << std::endl;
      _log->debug(p) << " └─ rightGroup.width: " << rightGroupWidth << std::endl;
      _log->debug(p) << " └─ threshold: " << 10 * _doc->avgGlyphWidth << std::endl;
    }

    if (rightGroupWidth < 10 * _doc->avgGlyphWidth) {
      if (!silent) {
        _log->debug(p) << "\033[1mCut not chosen (right group width too small).\033[0m" << std::endl;
      }
      continue;
    }

    // ------------
    // The gap is a valid cut position if the left and/or right element is a non-text element
    // (e.g., a line or a figure) with a "sufficient" height. The height requirement exists to no
    // split the elements between vertical lines that are part of math formulas (e.g., fraction
    // bars or absolute value bars).
    // const PdfNonTextElement* nonTextLeft = dynamic_cast<const PdfNonTextElement*>(elementLeft);
    // const PdfNonTextElement* nonTextRight = dynamic_cast<const PdfNonTextElement*>(elementRight);
    // if (nonTextLeft && nonTextLeft->position->getHeight() > 2 * _doc->avgGlyphHeight) {
    //   cutIndices->push_back(i);
    //   continue;
    // }
    // if (nonTextRight && nonTextRight->position->getHeight() > 2 * _doc->avgGlyphHeight) {
    //   cutIndices->push_back(i);
    //   continue;
    // }
    if (!silent) {
      _log->debug(p) << "\033[1mCut chosen (no rule applied)\033[0m" << std::endl;
    }
    cut->isChosen = true;
    prevChosenCut = cut;
  }
}

// _________________________________________________________________________________________________
void PageSegmentator::chooseYCuts(const std::vector<PdfElement*>& elems, std::vector<Cut*>& cuts,
      bool silent) {
  // Do nothing if no elements are given.
  if (elems.empty()) {
    return;
  }

  auto chooseXCutsBind = std::bind(&PageSegmentator::chooseXCuts, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3);

  int p = elems[0]->position->pageNum;

  if (!silent) {
    _log->debug(p) << "====================" << std::endl;
    _log->debug(p) << "\033[1mChoosing y-cuts...\033[0m" << std::endl;
    _log->debug(p) << " └─ # elements: " << elems.size() << std::endl;
    _log->debug(p) << " └─ # cut candidates: " << cuts.size() << std::endl;
  }

  for (size_t i = 0; i < cuts.size(); i++) {
    Cut* prevCut = i > 0 ? cuts[i - 1] : nullptr;
    Cut* currCut = cuts[i];

    if (!silent) {
      _log->debug(p) << "--------------------" << std::endl;
      _log->debug(p) << "\033[1my-cut: " << currCut->id << "\033[0m" << std::endl;
      _log->debug(p) << " └─ cut.pageNum: " << currCut->pageNum << std::endl;
      _log->debug(p) << " └─ cut.x1: " << currCut->x1 << std::endl;
      _log->debug(p) << " └─ cut.y1: " << currCut->y1 << std::endl;
      _log->debug(p) << " └─ cut.x2: " << currCut->x2 << std::endl;
      _log->debug(p) << " └─ cut.y2: " << currCut->y2 << std::endl;
      _log->debug(p) << " └─ cut.gapWidth: " << currCut->gapWidth << std::endl;
      _log->debug(p) << " └─ cut.gapHeight: " << currCut->gapHeight << std::endl;
      _log->debug(p) << " └─ cut.posInElements: " << currCut->posInElements << std::endl;
      _log->debug(p) << " └─ cut.elementBefore: " << currCut->elementBefore->toString() << std::endl;
      _log->debug(p) << " └─ cut.elementAfter: " << currCut->elementAfter->toString() << std::endl;
    }

    size_t prevCutPos = prevCut ? prevCut->posInElements : 0;
    size_t currCutPos = currCut->posInElements;

    // Check if the elements between the previous cut and the current cut can be split vertically.
    std::vector<PdfElement*> elems1(elems.begin() + prevCutPos, elems.begin() + currCutPos);
    bool cutOk = xCut(elems1, chooseXCutsBind, _minXCutGapWidth, maxNumCuttingElements, true);

    if (!silent) {
      _log->debug(p) << "Checked if elements between prev/curr cut can be x-cut." << std::endl;
      _log->debug(p) << " └─ prevCut.id: " << (prevCut ? prevCut->id : "-") << std::endl;
      _log->debug(p) << " └─ prevCut.posInElements: " << prevCutPos << std::endl;
      _log->debug(p) << " └─ currCut.posInElements: " << currCutPos << std::endl;
      _log->debug(p) << " └─ # elements: " << elems1.size() << std::endl;
      _log->debug(p) << " └─ can be x-cut: " << (cutOk ? "yes" : "no") << std::endl;
    }

    if (cutOk) {
      // Check if *all* elements below the prev cut can be split vertically.
      std::vector<PdfElement*> elems2(elems.begin() + prevCutPos, elems.end());
      cutOk = xCut(elems2, chooseXCutsBind, _minXCutGapWidth, maxNumCuttingElements, true);

      if (!silent) {
        _log->debug(p) << "Checked if all elements below prev cut can be x-cut." << std::endl;
        _log->debug(p) << " └─ prevCut.id: " << (prevCut ? prevCut->id : "-") << std::endl;
        _log->debug(p) << " └─ prevCut.posInElements: " << prevCutPos << std::endl;
        _log->debug(p) << " └─ # elements: " << elems2.size() << std::endl;
        _log->debug(p) << " └─ can be x-cut: " << (cutOk ? "yes" : "no") << std::endl;
      }

      if (cutOk && prevCut) {
        if (!silent) {
          _log->debug(p) << "\033[1mChosen cut " << prevCut->id << "\033[0m" << std::endl;
        }
        prevCut->isChosen = true;
        return;
      }

      if (!silent) {
        _log->debug(p) << "Checking next cuts..." << std::endl;
      }

      // Iterate through the following cuts. For each, check if the elements between the previous
      // cut and the current cut can be vertically split.
      for (size_t j = i + 1; j < cuts.size(); j++) {
        Cut* nextCut = cuts[j];
        size_t nextCutPos = nextCut->posInElements;

        if (!silent) {
          _log->debug(p) << "--------------------" << std::endl;
          _log->debug(p) << "\033[1mNext Y-Cut: " << nextCut->id << "\033[0m" << std::endl;
          _log->debug(p) << " └─ cut.pageNum: " << nextCut->pageNum << std::endl;
          _log->debug(p) << " └─ cut.x1: " << nextCut->x1 << std::endl;
          _log->debug(p) << " └─ cut.y1: " << nextCut->y1 << std::endl;
          _log->debug(p) << " └─ cut.x2: " << nextCut->x2 << std::endl;
          _log->debug(p) << " └─ cut.y2: " << nextCut->y2 << std::endl;
          _log->debug(p) << " └─ cut.gapWidth: " << nextCut->gapWidth << std::endl;
          _log->debug(p) << " └─ cut.gapHeight: " << nextCut->gapHeight << std::endl;
        }

        std::vector<PdfElement*> elems3(elems.begin() + prevCutPos, elems.begin() + nextCutPos);
        cutOk = xCut(elems3, chooseXCutsBind, _minXCutGapWidth, maxNumCuttingElements, true);

        if (!silent) {
          _log->debug(p) << "Checked if elements between prev/next cut can be x-cut." << std::endl;
          _log->debug(p) << " └─ prevCut.id: " << (prevCut ? prevCut->id : "-") << std::endl;
          _log->debug(p) << " └─ prevCut.posInElements: " << prevCutPos << std::endl;
          _log->debug(p) << " └─ nextCut.posInElements: " << currCutPos << std::endl;
          _log->debug(p) << " └─ # elements: " << elems2.size() << std::endl;
          _log->debug(p) << " └─ can be x-cut: " << (cutOk ? "yes" : "no") << std::endl;
        }

        if (!cutOk) {
          break;
        }
        currCut = nextCut;
        i = j;
      }

      if (prevCut) {
        if (!silent) {
          _log->debug(p) << "\033[1mChosen prev cut: " << prevCut->id << "\033[0m" << std::endl;
        }
        prevCut->isChosen = true;
      }
      if (!silent) {
        _log->debug(p) << "\033[1mChosen curr cut: " << currCut->id << "\033[0m" << std::endl;
      }
      currCut->isChosen = true;
    }
  }

  Cut* lastCut = !cuts.empty() ? cuts[cuts.size() - 1] : nullptr;
  size_t lastCutPos = lastCut ? lastCut->posInElements : 0;
  std::vector<PdfElement*> elems4(elems.begin() + lastCutPos, elems.end());
  bool cutOk = xCut(elems4, chooseXCutsBind, _minXCutGapWidth, maxNumCuttingElements, true);

  if (!silent) {
    _log->debug(p) << "Checked if all elements below last cut can be x-cut." << std::endl;
    if (lastCut) {
      _log->debug(p) << " └─ lastCut.id: " << lastCut->id << std::endl;
      _log->debug(p) << " └─ lastCut.pageNum: " << lastCut->pageNum << std::endl;
      _log->debug(p) << " └─ lastCut.x1: " << lastCut->x1 << std::endl;
      _log->debug(p) << " └─ lastCut.y1: " << lastCut->y1 << std::endl;
      _log->debug(p) << " └─ lastCut.x2: " << lastCut->x2 << std::endl;
      _log->debug(p) << " └─ lastCut.y2: " << lastCut->y2 << std::endl;
      _log->debug(p) << " └─ # elements: " << elems4.size() << std::endl;
      _log->debug(p) << " └─ can be x-cut: " << (cutOk ? "yes" : "no") << std::endl;
    } else {
      _log->debug(p) << " └─ lastCut.id: -" << std::endl;
    }
  }

  if (cutOk && lastCut) {
    if (!silent) {
      _log->debug(p) << "\033[1mChosen last cut " << lastCut->id << "\033[0m" << std::endl;
    }
    lastCut->isChosen = true;
  }
}

// _________________________________________________________________________________________________
void PageSegmentator::createPageSegment(const std::vector<PdfElement*>& elements,
    std::vector<PdfPageSegment*>* segments) const {
  // // Filter the elements by words.
  // std::vector<PdfWord*> words;
  // for (auto* element : elements) {
  //   PdfWord* word = dynamic_cast<PdfWord*>(element);
  //   if (word != nullptr) {
  //     words.push_back(word);
  //   }
  // }

  // // Do nothing if no words are given.
  // if (words.size() == 0) {
  //   return;
  // }

  // Do nothing if no elements are given.
  if (elements.size() == 0) {
    return;
  }

  PdfPageSegment* segment = new PdfPageSegment();
  segment->id = string_utils::createRandomString(8, "ps-");
  segment->doc = _doc;

  // Set the page number.
  segment->position->pageNum = elements[0]->position->pageNum;

  // Iterate through the elements and compute the x,y-coordinates of the bounding box.
  for (const auto* element : elements) {
    segment->position->leftX = std::min(segment->position->leftX, element->position->leftX);
    segment->position->upperY = std::min(segment->position->upperY, element->position->upperY);
    segment->position->rightX = std::max(segment->position->rightX, element->position->rightX);
    segment->position->lowerY = std::max(segment->position->lowerY, element->position->lowerY);
  }

  // Sort the elements by reading order.
  segment->elements = elements;

  segments->push_back(segment);
}
