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
#include "./utils/Utils.h"
#include "./XYCut.h"

const double GAP_MIN_WIDTH = 0.5;
const double GAP_MIN_HEIGHT = 0.5;

// _________________________________________________________________________________________________
PageSegmentator::PageSegmentator(PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
PageSegmentator::~PageSegmentator() = default;

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
  auto choosePrimaryYCutsBind = std::bind(&PageSegmentator::choosePrimaryYCuts, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
  auto chooseXCutsBind = std::bind(&PageSegmentator::chooseXCuts, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
  auto chooseYCutsBind = std::bind(&PageSegmentator::chooseYCuts, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

  // Create a list with all page elements (glyphs, figures, shapes).
  std::vector<PdfElement*> pageElements;
  pageElements.reserve(page->words.size() + page->nonTexts.size());
  for (auto* word : page->words) { pageElements.push_back(word); }
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

  // Identify the primary y-cuts and divide the page elements into groups at each primary y-cut.
  std::vector<std::vector<PdfElement*>> primaryYCutGroups;
  yCut(pageElements, choosePrimaryYCutsBind, &primaryYCutGroups, &page->blockDetectionCuts);

  // Divide each group further by using the recursive XY-cut algorithm.
  std::vector<std::vector<PdfElement*>> groups;
  for (const auto& primYCutGroup : primaryYCutGroups) {
    xyCut(primYCutGroup, chooseXCutsBind, chooseYCutsBind, &groups, &page->blockDetectionCuts);
  }

  // Create a text segment from each group and append it to the result list.
  for (const auto& group : groups) {
    createPageSegment(group, segments);
  }
}

// _________________________________________________________________________________________________
void PageSegmentator::chooseXCuts(const std::vector<PdfElement*>& elements,
      const std::vector<size_t>& gapPositions, const std::vector<PdfElement*>& gapStartElements,
      std::vector<size_t>* cutIndices) {
  for (size_t i = 0; i < gapPositions.size(); i++) {
    size_t gapPos = gapPositions[i];

    // Determine the closest element on the left and right side of the gap.
    const PdfElement* elementLeft = gapStartElements[i];
    const PdfElement* elementRight = elements[gapPos];

    // The gap is *not* a valid x-cut when the width is too small.
    double gapWidth = elementRight->minX - elementLeft->maxX;
    if (gapWidth < GAP_MIN_WIDTH) {
      continue;
    }

    const PdfWord* wordLeft = dynamic_cast<const PdfWord*>(elementLeft);
    if (wordLeft) {
      if (wordLeft->wMode != 0 || wordLeft->rotation != 0) {
        cutIndices->push_back(i);
        continue;
      }
    }

    const PdfWord* wordRight = dynamic_cast<const PdfWord*>(elementRight);
    if (wordRight) {
      if (wordRight->wMode != 0 || wordRight->rotation != 0) {
        cutIndices->push_back(i);
        continue;
      }
    }

    if (wordLeft && wordRight) {
      // The gap is a valid x-cut when the left and right word do not share the same writing mode.
      if (wordLeft->wMode != wordRight->wMode) {
        cutIndices->push_back(i);
        continue;
      }

      // The gap is a valid x-cut when the left and right word do not share the same rotation value.
      if (wordLeft->rotation != wordRight->rotation) {
        cutIndices->push_back(i);
        continue;
      }

      // The gap is *not* a valid x-cut when the left word and the right word are *contiguous*,
      // that is: if the right word immediately follows behind the left word in the extraction
      // order and if both words share the same text line. This rule exists to not accidentally
      // divide the words of a title when a word boundary coincide with a column boundary, as
      // shown in the following example:
      // THIS  IS  | THE  TITLE
      //           |
      // XXXXXXXXX | XXXXXXXXXX
      // XXXXXXXXX | XXXXXXXXXX
      // XXXXXXXXX | XXXXXXXXXX
      if (wordLeft->glyphs.size() > 0 && wordRight->glyphs.size() > 0) {
        bool isContiguous = true;
        PdfGlyph* glyphLeft = wordLeft->glyphs[wordLeft->glyphs.size() - 1];
        PdfGlyph* glyphRight = wordRight->glyphs[0];

        // The words are not contiguous, if they are not neighbors in the extraction order.
        if (glyphLeft->rank + 1 != glyphRight->rank) {
          isContiguous = false;
        }
        // The words are not contiguous, if they do not share the same text line (= if they do
        // not overlap vertically).
        if (glyphLeft->minY > glyphRight->maxY || glyphLeft->maxY < glyphRight->minY) {
          isContiguous = false;
        }
        // The words are not contiguous if the vertical distance between them is too large.
        // double wordDistance = glyphRight->minX - glyphLeft->maxX;
        // if (wordDistance > 5 * _doc->avgGlyphWidth) {
        //   isContiguous = false;
        // }
        if (isContiguous) {
          continue;
        }
      }
    }

    // The gap is *not* a valid x-cut when the width of one of the resulting groups is too small.
    // Here are two examples to explain why this rule exists:
    //
    // * In a bibliography, there could be a vertical gap between the reference anchors and the
    // reference bodies, like illustrated in the following:
    // [1]   W. Smith et al: Lorem ipsum ...
    // [2]   F. Miller et al: Lorem ipsum ...
    // [3]   T. Redford et al: Lorem ipsum ...
    // The reference anchors should *not* be separated from the reference bodies.
    //
    // * A formula could have a numbering, with a (large) vertical gap in between, like
    // illustrated in the following example:
    // x + y = z     (1)
    // The numbering should *not* be separated from the formula.
    double leftGroupMinX = elements[0]->minX;  // The elements are sorted by minX.
    double leftGroupMaxX = elementLeft->maxX;
    double leftGroupWidth = leftGroupMaxX - leftGroupMinX;
    if (leftGroupWidth < 10 * _doc->avgGlyphWidth) {
      continue;
    }
    double rightGroupMinX = elementRight->minX;
    // TODO(korzen): The elements are sorted by minX, so the last element isn't necessarily the
    // element with the largest maxX value in the right group.
    double rightGroupMaxX = elements[elements.size() - 1]->maxX;
    double rightGroupWidth = rightGroupMaxX - rightGroupMinX;
    if (rightGroupWidth < 10 * _doc->avgGlyphWidth) {
      continue;
    }

    // The gap is a valid x-cut if the left and/or right element is a non-text element (e.g., a
    // line or a figure) with a "sufficient" height. The height requirement exists to no split the
    // elements between vertical lines (e.g., fraction bars of math formulas).
    const PdfNonText* nonTextLeft = dynamic_cast<const PdfNonText*>(elementLeft);
    const PdfNonText* nonTextRight = dynamic_cast<const PdfNonText*>(elementRight);
    if (nonTextLeft && nonTextLeft->getHeight() > 2 * _doc->avgGlyphHeight) {
      cutIndices->push_back(i);
      continue;
    }
    if (nonTextRight && nonTextRight->getHeight() > 2 * _doc->avgGlyphHeight) {
      cutIndices->push_back(i);
      continue;
    }

    // The cut is a valid x-cut when its width is large enough.
    if (gapWidth > 2 * _doc->avgGlyphWidth) {
      cutIndices->push_back(i);
      continue;
    }
  }
}

// _________________________________________________________________________________________________
void PageSegmentator::choosePrimaryYCuts(const std::vector<PdfElement*>& elements,
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

    // TODO(korzen): Consider the gap to be a primary y-cut when the height of the resulting upper
    // group and/or
    // lower group is approximately equal to the average glpyh height. This should detect separate
    // page headers/footers that consists of a single line.
    double upperGroupMinY = elements[0]->minY;  // The elements are sorted by minX.
    double upperGroupMaxY = elementAbove->maxY;
    double upperGroupHeight = upperGroupMaxY - upperGroupMinY;
    if (upperGroupHeight < 2 * _doc->avgGlyphHeight) {
      cutIndices->push_back(i);
      continue;
    }
    double lowerGroupMinY = elementBelow->minY;
    double lowerGroupMaxY = elements[elements.size() - 1]->maxY;
    double lowerGroupHeight = lowerGroupMaxY - lowerGroupMinY;
    if (lowerGroupHeight < 2 * _doc->avgGlyphHeight) {
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
void PageSegmentator::chooseYCuts(const std::vector<PdfElement*>& elements,
      const std::vector<size_t>& gapPositions, const std::vector<PdfElement*>& gapStartElements,
      std::vector<size_t>* cutIndices) {
  for (size_t i = 0; i < gapPositions.size(); i++) {
    size_t gapPos = gapPositions[i];

    // Determine the closest element above and below the gap.
    const PdfElement* elementAbove = gapStartElements[i];
    const PdfElement* elementBelow = elements[gapPos];

    // The gap is *not* a valid x-cut when the height is too small.
    double gapHeight = elementBelow->minY - elementAbove->maxY;
    if (gapHeight < GAP_MIN_HEIGHT) {
      continue;
    }

    // The gap is a valid y-cut if the upper and/or lower element is a non-text element (e.g., a
    // line or a figure) with a "sufficient" width. The width requirement exists to no split the
    // elements between underlined words, in which case the underlining is stored as a non-text
    // element (a horizontal line).
    const PdfNonText* nonTextAbove = dynamic_cast<const PdfNonText*>(elementAbove);
    const PdfNonText* nonTextBelow = dynamic_cast<const PdfNonText*>(elementBelow);
    if (nonTextAbove && nonTextAbove->getWidth() > 5 * _doc->avgGlyphWidth) {
      cutIndices->push_back(i);
      continue;
    }
    if (nonTextBelow && nonTextBelow->getWidth() > 5 * _doc->avgGlyphWidth) {
      cutIndices->push_back(i);
      continue;
    }

    // The gap is a valid y-cut if the vertical distance between the upper and lower word is larger
    // than the most common line distance (with respecting a small tolerance).
    double lineDist = elementBelow->maxY - elementAbove->maxY;
    double ratio = 1.05;

    // If the element above the cut and the element below the cut are words with the same fontsize
    // that is larger than the most frequent font size, allow a larger ratio (which is relative to
    // the font size). This rule exists because the distance between two lines of a title is often
    // larger than the most frequent line distance (and thus, the lines were often extracted as
    // multiple text blocks).
    const PdfWord* wordAbove = dynamic_cast<const PdfWord*>(elementAbove);
    const PdfWord* wordBelow = dynamic_cast<const PdfWord*>(elementBelow);
    if (wordAbove && wordBelow) {
      if (fabs(wordAbove->fontSize - wordBelow->fontSize) < 0.1
          && wordAbove->fontSize - _doc->mostFreqFontSize > 0.1) {
        ratio = wordAbove->fontSize / _doc->mostFreqFontSize;
      }
    }

    if (lineDist > ratio * _doc->mostFreqEstimatedLineDistance) {
      cutIndices->push_back(i);
      continue;
    }
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
  segment->id = createRandomString(8, "ps-");

  // Set the page number.
  segment->pageNum = elements[0]->pageNum;

  // Iterate through the elements and compute the x,y-coordinates of the bounding box.
  for (const auto* element : elements) {
    segment->minX = std::min(segment->minX, element->minX);
    segment->minY = std::min(segment->minY, element->minY);
    segment->maxX = std::max(segment->maxX, element->maxX);
    segment->maxY = std::max(segment->maxY, element->maxY);
  }

  // Sort the elements by reading order.
  segment->elements = elements;

  segments->push_back(segment);
}
