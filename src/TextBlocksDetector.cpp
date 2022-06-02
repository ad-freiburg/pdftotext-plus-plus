/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // max
#include <cmath>
#include <iostream>
#include <regex>
#include <stack>
#include <string>
#include <vector>

#include "./Constants.h"
#include "./PdfDocument.h"
#include "./TextBlocksDetector.h"

#include "./utils/LogUtils.h"
#include "./utils/MathUtils.h"
#include "./utils/PdfElementUtils.h"
#include "./utils/TextBlocksUtils.h"
#include "./utils/TextLinesUtils.h"
#include "./utils/Trool.h"

using std::string;

// _________________________________________________________________________________________________
TextBlocksDetector::TextBlocksDetector(PdfDocument* doc, bool debug, int debugPageFilter) {
  _log = new Logger(debug ? DEBUG : INFO, debugPageFilter);
  _doc = doc;
}

// _________________________________________________________________________________________________
TextBlocksDetector::~TextBlocksDetector() {
  delete _log;
}

// _________________________________________________________________________________________________
void TextBlocksDetector::detect() {
  assert(_doc);

  _log->debug() << BOLD << "Text Block Detection - DEBUG MODE" << OFF << endl;

  _log->debug() << "Detecting preliminary text blocks..." << endl;
  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      vector<PdfTextLine*> currentBlockLines;
      for (auto* line : segment->lines) {
        if (startsPreliminaryBlock(line) && !currentBlockLines.empty()) {
          text_blocks_utils::createTextBlock(currentBlockLines, &segment->blocks);
          currentBlockLines.clear();
        }
        currentBlockLines.push_back(line);
      }
      if (!currentBlockLines.empty()) {
        text_blocks_utils::createTextBlock(currentBlockLines, &segment->blocks);
      }
    }
  }

  _log->debug() << "Splitting preliminary text blocks..." << endl;
  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      for (auto* block : segment->blocks) {
        vector<PdfTextLine*> currentBlockLines;
        for (auto* line : block->lines) {
          // Detect potential footnote labels in the line (= superscripted numbers and/or
          // characters). This is needed to detect the start of footnotes (we want to detect each
          // footnote as a separate block).
          text_lines_utils::computePotentialFootnoteLabels(line, &_potentialFnLabels);

          if (startsBlock(block, line) && !currentBlockLines.empty()) {
            text_blocks_utils::createTextBlock(currentBlockLines, &page->blocks);
            currentBlockLines.clear();
          }
          currentBlockLines.push_back(line);
        }
        if (!currentBlockLines.empty()) {
          text_blocks_utils::createTextBlock(currentBlockLines, &page->blocks);
        }
      }
    }
  }
}

// _________________________________________________________________________________________________
bool TextBlocksDetector::startsPreliminaryBlock(const PdfTextLine* line) const {
  assert(line);

  int p = line->position->pageNum;
  string prevLineStr = line->prevLine ? line->prevLine->text : "-";
  string nextLineStr = line->nextLine ? line->nextLine->text : "-";

  _log->debug(p) << "= (pre) =================" << endl;
  _log->debug(p) << BOLD << "Line: \"" << line->text << "\"" << OFF << endl;
  _log->debug(p) << " └─ page:   " << line->position->pageNum << endl;
  _log->debug(p) << " └─ leftX:  " << line->position->leftX << endl;
  _log->debug(p) << " └─ upperY: " << line->position->upperY << endl;
  _log->debug(p) << " └─ rightX: " << line->position->rightX << endl;
  _log->debug(p) << " └─ lowerY: " << line->position->lowerY << endl;

  if (line->position->rotation != 0) {
    _log->debug(p) << " └─ rotation:  " << line->position->rotation << endl;
    _log->debug(p) << " └─ rotLeftX:  " << line->position->getRotLeftX() << endl;
    _log->debug(p) << " └─ rotUpperY: " << line->position->getRotUpperY() << endl;
    _log->debug(p) << " └─ rotRightX: " << line->position->getRotRightX() << endl;
    _log->debug(p) << " └─ rotLowerY: " << line->position->getRotLowerY() << endl;
  }

  _log->debug(p) << " └─ line.prevLine: " << prevLineStr << endl;
  _log->debug(p) << " └─ line.nextLine: " << nextLineStr << endl;
  _log->debug(p) << "-------------------------" << endl;

  // Check if the line starts a block because no previous line exists.
  Trool res = startsBlock_existsPrevLine(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line doesn't start a block because it is part of the same figure as the prev line.
  res = startsBlock_sameFigure(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because it has another rotation than its previous line.
  res = startsBlock_rotation(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because it has another writing mode than its previous line.
  res = startsBlock_wMode(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because it has another font size than its previous line.
  res = startsBlock_fontSize(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because its distance to the previous line is larger than the
  // most frequent line distance in the document.
  res = startsBlock_lineDistance(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because the line distance between the line and its previous
  // line is larger than the line distance between the previous line and the previous but one line.
  res = startsBlock_increasedLineDistance(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  _log->debug(p) << BLUE << "no rule applied → continues block" << OFF << endl;
  return false;
}

// _________________________________________________________________________________________________
bool TextBlocksDetector::startsBlock(const PdfTextBlock* pBlock, const PdfTextLine* line) {
  assert(pBlock);
  assert(line);

  int p = line->position->pageNum;
  string prevLineStr = line->prevLine ? line->prevLine->text : "-";
  string nextLineStr = line->nextLine ? line->nextLine->text : "-";
  string parentLineStr = line->parentLine ? line->parentLine->text : "-";
  string prevSibLineStr = line->prevSiblingLine ? line->prevSiblingLine->text : "-";
  string nextSibLineStr = line->nextSiblingLine ? line->nextSiblingLine->text : "-";

  _log->debug(p) << "=========================" << endl;
  _log->debug(p) << BOLD << "Line: \"" << line->text << "\"" << OFF << endl;
  _log->debug(p) << " └─ page:   " << line->position->pageNum << endl;
  _log->debug(p) << " └─ leftX:  " << line->position->leftX << endl;
  _log->debug(p) << " └─ upperY: " << line->position->upperY << endl;
  _log->debug(p) << " └─ rightX: " << line->position->rightX << endl;
  _log->debug(p) << " └─ lowerY: " << line->position->lowerY << endl;

  if (line->position->rotation != 0) {
    _log->debug(p) << " └─ rotation:  " << line->position->rotation << endl;
    _log->debug(p) << " └─ rotLeftX:  " << line->position->getRotLeftX() << endl;
    _log->debug(p) << " └─ rotUpperY: " << line->position->getRotUpperY() << endl;
    _log->debug(p) << " └─ rotRightX: " << line->position->getRotRightX() << endl;
    _log->debug(p) << " └─ rotLowerY: " << line->position->getRotLowerY() << endl;
  }

  _log->debug(p) << " └─ line.prevLine: " << prevLineStr << endl;
  _log->debug(p) << " └─ line.nextLine: " << nextLineStr << endl;
  _log->debug(p) << " └─ line.parentLine: " << parentLineStr << endl;
  _log->debug(p) << " └─ line.prevSiblingLine: " << prevSibLineStr << endl;
  _log->debug(p) << " └─ line.nextSiblingLine: " << nextSibLineStr << endl;
  _log->debug(p) << "-------------------------" << endl;

  // Check if the line starts a block because no previous line exists.
  Trool res = startsBlock_existsPrevLine(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because the preliminary block and the line is centered.
  res = startsBlock_centered(pBlock, line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because it is part of an enumeration item.
  res = startsBlock_item(pBlock, line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because of its emphasis. // TODO: Move to preliminary block.
  res = startsBlock_emphasized(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because of its indentation, given that the preliminary text
  // block is in hanging indent format.
  res = startsBlock_hangingIndent(pBlock, line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because of its indentation, given that the preliminary text
  // block is not in hanging indent format.
  res = startsBlock_indent(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  _log->debug(p) << BLUE << "no rule applied → continues block" << OFF << endl;
  return false;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_existsPrevLine(const PdfTextLine* line) const {
  assert(line);

  int p = line->position->pageNum;
  PdfTextLine* prevLine = line->prevLine;

  _log->debug(p) << BLUE << "Does the line has no previous line?" << OFF << endl;
  _log->debug(p) << " └─ line.prevLine: " << (prevLine ? prevLine->text : "-") << endl;

  // The line starts a new block if it has no previous line.
  if (!prevLine) {
    _log->debug(p) << BLUE << BOLD << " yes → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_sameFigure(const PdfTextLine* line) const {
  assert(line);
  assert(line->prevLine);

  int p = line->position->pageNum;
  PdfTextLine* prevLine = line->prevLine;

  // Compute the figure overlapped by the previous and the current line.
  vector<PdfFigure*>& figures = _doc->pages[p - 1]->figures;
  PdfFigure* prevLineOverlapsFigure = element_utils::computeOverlapsFigure(prevLine, figures);
  PdfFigure* currLineOverlapsFigure = element_utils::computeOverlapsFigure(line, figures);

  _log->debug(p) << BLUE << "Does the line overlap the same fig as the prev line?" << OFF << endl;
  _log->debug(p) << " └─ prevLine.overlapsFigure: " << prevLineOverlapsFigure << endl;
  _log->debug(p) << " └─ currLine.overlapsFigure: " << currLineOverlapsFigure << endl;

  // The line does *not* start a new block if it overlaps the same figure as the previous line.
  if (prevLineOverlapsFigure && prevLineOverlapsFigure == currLineOverlapsFigure) {
    _log->debug(p) << BLUE << BOLD << " yes → continues block" << OFF << endl;
    return Trool::False;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_rotation(const PdfTextLine* line) const {
  assert(line);
  assert(line->prevLine);

  int p = line->position->pageNum;
  PdfTextLine* prevLine = line->prevLine;

  _log->debug(p) << BLUE << "Does the line have another rotation than prev line?" << OFF << endl;
  _log->debug(p) << " └─ prevLine.rotation: " << prevLine->position->rotation << endl;
  _log->debug(p) << " └─ currLine.rotation: " << line->position->rotation << endl;

  // The line starts a block if it has another rotation than the previous line.
  if (prevLine->position->rotation != line->position->rotation) {
    _log->debug(p) << BLUE << BOLD << " yes → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_wMode(const PdfTextLine* line) const {
  assert(line);
  assert(line->prevLine);

  int p = line->position->pageNum;
  PdfTextLine* prevLine = line->prevLine;

  _log->debug(p) << BLUE << "Does the line have another wMode than the prev line?" << OFF << endl;
  _log->debug(p) << " └─ prevLine.wMode: " << prevLine->position->wMode << endl;
  _log->debug(p) << " └─ currLine.wMode: " << line->position->wMode << endl;

  // The line starts a block if it has another writing mode than the previous line.
  if (prevLine->position->wMode != line->position->wMode) {
    _log->debug(p) << BLUE << BOLD << " yes → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_fontSize(const PdfTextLine* line, double tolerance) const {
  assert(line);
  assert(line->prevLine);

  int p = line->position->pageNum;
  PdfTextLine* prevLine = line->prevLine;

  _log->debug(p) << BLUE << "Does the line have another font size than prev line?" << OFF << endl;
  _log->debug(p) << " └─ prevLine.mostFreqFontSize: " << prevLine->fontSize << endl;
  _log->debug(p) << " └─ currLine.mostFreqFontSize: " << line->fontSize << endl;
  _log->debug(p) << " └─ prevLine.maxFontSize:      " << prevLine->maxFontSize << endl;
  _log->debug(p) << " └─ currLine.maxFontSize:      " << line->maxFontSize << endl;
  _log->debug(p) << " └─ tolerance: " << tolerance << endl;

  // The line starts a block if neither the most frequent font size of the line is equal to
  // the most frequent font size in the prev line (under consideration of the given tolerance), nor
  // the max font size of the line is equal to the max font size in the prev line (under
  // consideration of the given tolerance). The first condition exists to not split text
  // lines when they contain few words with larger font sizes (e.g., in a caption, the "Figure X:"
  // parts is likely to have a larger font size than the rest of the caption). The second condition
  // exists to not split text lines with many small characters (which is particularly often the
  // case when the text line contains an inline formula).
  bool isEqualFontSize = math_utils::equal(prevLine->fontSize, line->fontSize, tolerance);
  bool isEqualMaxFontSize = math_utils::equal(prevLine->maxFontSize, line->maxFontSize, tolerance);
  if (!isEqualFontSize && !isEqualMaxFontSize) {
    _log->debug(p) << BLUE << BOLD << " yes → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_lineDistance(const PdfTextLine* line, double minTolerance,
      double toleranceFactor) const {
  assert(line);
  assert(line->prevLine);

  int p = line->position->pageNum;
  PdfTextLine* prevLine = line->prevLine;

  // Compute the expected line distance.
  double fontSize = math_utils::round(line->fontSize, FONT_SIZE_PREC);
  double expectedLineDistance = 0;
  if (_doc->mostFreqLineDistancePerFontSize.count(fontSize) > 0) {
    double eld = _doc->mostFreqLineDistancePerFontSize.at(fontSize);
    expectedLineDistance = max(expectedLineDistance, eld);
  }
  expectedLineDistance = max(expectedLineDistance, _doc->mostFreqLineDistance);

  // Compute the actual line distance.
  double actualLineDistance = element_utils::computeVerticalGap(prevLine, line);
  actualLineDistance = math_utils::round(actualLineDistance, LINE_DIST_PREC);

  double lineDistanceDiff = actualLineDistance - expectedLineDistance;

  // Compute the tolerance.
  double tolerance = max(minTolerance, toleranceFactor * expectedLineDistance);

  _log->debug(p) << BLUE << "Is the dist to the prev line larger than expected?" << OFF << endl;
  _log->debug(p) << " └─ actual line distance:   " << actualLineDistance << endl;
  _log->debug(p) << " └─ expected line distance: " << expectedLineDistance << endl;
  _log->debug(p) << " └─ line distance diff:     " << lineDistanceDiff << endl;
  _log->debug(p) << " └─ minTolerance:    " << minTolerance << endl;
  _log->debug(p) << " └─ toleranceFactor: " << toleranceFactor << endl;
  _log->debug(p) << " └─ tolerance:       " << tolerance << endl;

  // The line does *not* start a new block if the actual line distance is negative.
  if (math_utils::equalOrSmaller(actualLineDistance, 0)) {
    _log->debug(p) << BLUE << BOLD << " no, distance is negative → continues block" << OFF << endl;
    return Trool::False;
  }

  // The line starts a new block if the actual line distance is larger than the expected line
  // distance, under consideration of the computed tolerance.
  if (math_utils::larger(actualLineDistance, expectedLineDistance, tolerance)) {
    _log->debug(p) << BLUE << BOLD << " yes → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_increasedLineDistance(const PdfTextLine* line,
      double toleranceFactor) const {
  assert(line);
  assert(line->prevLine);

  PdfTextLine* prevLine = line->prevLine;
  PdfTextLine* prevPrevLine = prevLine->prevLine;
  if (!prevPrevLine) {
    return Trool::None;
  }

  int p = line->position->pageNum;

  // Compute the distance between the previous but one line and the previous line.
  double prevDistance = element_utils::computeVerticalGap(prevPrevLine, prevLine);
  prevDistance = math_utils::round(prevDistance, LINE_DIST_PREC);

  // Compute the distance between the previous line and the current line.
  double distance = element_utils::computeVerticalGap(prevLine, line);
  distance = math_utils::round(distance, LINE_DIST_PREC);

  // Compute the tolerance.
  double tolerance = toleranceFactor * _doc->mostFreqWordHeight;

  _log->debug(p) << BLUE << "Is the curr+prev distance > prev+prevPrev distance?" << OFF << endl;
  _log->debug(p) << " └─ curr+prev line distance:     " << distance << endl;
  _log->debug(p) << " └─ prev+prevPrev line distance: " << prevDistance << endl;
  _log->debug(p) << " └─ toleranceFactor: " << toleranceFactor << endl;
  _log->debug(p) << " └─ tolerance:       " << tolerance << endl;

  // The line starts a block if the curr+prev line distance is larger than the prev+prevPrev line
  // distance, under consideration of the computed tolerance.
  if (math_utils::larger(distance, prevDistance, tolerance)) {
    _log->debug(p) << BLUE << BOLD << " yes → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_centered(const PdfTextBlock* pBlock, const PdfTextLine* line)
      const {
  assert(pBlock);
  assert(line);

  // Check if the line is the first line of an enumeration item.
  bool isFirstLineOfItem = text_lines_utils::computeIsFirstLineOfItem(line);

  int p = line->position->pageNum;
  _log->debug(p) << BLUE << "Is the line part of a centered block?" << OFF << endl;
  _log->debug(p) << " └─ block.isCentered: " << pBlock->isCentered << endl;
  _log->debug(p) << " └─ line.isFirstLineOfItem: " << isFirstLineOfItem << endl;

  // Do nothing if the block is not centered.
  if (!pBlock->isCentered) {
    return Trool::None;
  }

  if (isFirstLineOfItem) {
    _log->debug(p) << BLUE << BOLD << " yes + line is 1st item line → starts block" << OFF << endl;
    return Trool::True;
  }

  _log->debug(p) << BLUE << BOLD << " yes → continues block" << OFF << endl;
  return Trool::False;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_item(const PdfTextBlock* pBlock, const PdfTextLine* line,
      double leftXOffsetToleranceFactorLow, double leftXOffsetToleranceFactorHigh) const {
  assert(pBlock);
  assert(line);

  int p = line->position->pageNum;
  PdfTextLine* prevLine = line->prevLine;

  bool isPrevFirstLine = text_lines_utils::computeIsFirstLineOfItem(prevLine, &_potentialFnLabels);
  bool isCurrFirstLine = text_lines_utils::computeIsFirstLineOfItem(line, &_potentialFnLabels);
  bool isPrevContLine = text_lines_utils::computeIsContinuationOfItem(prevLine, &_potentialFnLabels);
  bool isCurrContLine = text_lines_utils::computeIsContinuationOfItem(line, &_potentialFnLabels);
  bool isPrevPartOfItem = isPrevFirstLine || isPrevContLine;
  bool isCurrPartOfItem = isCurrFirstLine || isCurrContLine;
  double leftXOffset = element_utils::computeLeftXOffset(prevLine, line);
  bool hasPrevLineCapacity = text_lines_utils::computeHasPrevLineCapacity(line);
  double leftXOffsetToleranceLow = leftXOffsetToleranceFactorLow * _doc->avgGlyphWidth;
  double leftXOffsetToleranceHigh = leftXOffsetToleranceFactorHigh * _doc->avgGlyphWidth;

  _log->debug(p) << BLUE << "Is the curr line or prev line part of an item?" << OFF << endl;
  _log->debug(p) << " └─ prevLine.isFirstLineOfItem: " << isPrevFirstLine << endl;
  _log->debug(p) << " └─ currLine.isFirstLineOfItem: " << isCurrFirstLine << endl;
  _log->debug(p) << " └─ prevLine.isContinuationOfItem: " << isPrevContLine << endl;
  _log->debug(p) << " └─ currLine.isContinuationOfItem: " << isCurrContLine << endl;
  _log->debug(p) << " └─ leftXOffset prevLine/currLine: " << leftXOffset << endl;
  _log->debug(p) << " └─ prevLine.hasCapacity: " << hasPrevLineCapacity << endl;
  _log->debug(p) << " └─ block.isCentered: " << pBlock->isCentered << endl;
  _log->debug(p) << " └─ leftXOffsetToleranceFactorLow: " << leftXOffsetToleranceFactorLow << endl;
  _log->debug(p) << " └─ leftXOffsetToleranceFactorHigh:" << leftXOffsetToleranceFactorHigh << endl;
  _log->debug(p) << " └─ leftXOffsetToleranceLow:  " << leftXOffsetToleranceLow << endl;
  _log->debug(p) << " └─ leftXOffsetToleranceHigh: " << leftXOffsetToleranceHigh << endl;

  // The line starts a block if it is the first line of an item.
  if (isCurrFirstLine) {
    _log->debug(p) << BLUE << BOLD << " yes, line is 1st item line → starts block" << OFF << endl;
    return Trool::True;
  }

  if (isCurrContLine) {
    _log->debug(p) << BLUE << BOLD << " yes, line is continuation of item" << OFF << endl;

    // The line does *not* start a block if it is a continuation of an item + the block is centered.
    if (pBlock->isCentered) {
      _log->debug(p) << BLUE << BOLD << " + block is centered → continues block" << OFF << endl;
      return Trool::False;
    }

    // The line starts a block if it is a continuation of an item + the prev line has capacity.
    if (hasPrevLineCapacity) {
      _log->debug(p) << BLUE << BOLD << " + prev line has capacity → starts block" << OFF << endl;
      return Trool::True;
    }

    // The line does *not* start a block if it is a continuation of an item and the previous line
    // is the first line of an item.
    if (isPrevFirstLine) {
      _log->debug(p) << BLUE << BOLD << " + prev line is first line of item → continues block"
          << OFF << endl;
      return Trool::False;
    }

    if (isPrevContLine) {
      // The line does *not* start a block if the line and the previous line are a continuation
      // of an item, and the leftX-offset between the lines is in the given tolerance.
      if (math_utils::between(leftXOffset, leftXOffsetToleranceLow, leftXOffsetToleranceHigh)) {
        _log->debug(p) << BLUE << BOLD << " + prev line is continuation of item + leftX-offset in "
            << "tolerance → continues block" << OFF << endl;
        return Trool::False;
      }

      // The line starts a block if the line and the previous line are a continuation of an item,
      // and the leftX-offset between the lines is *not* in the given tolerance.
      _log->debug(p) << BLUE << BOLD << " + prev line is continuation of item + leftX-offset not "
            << "in tolerance → starts block" << OFF << endl;
      return Trool::True;
    }

    _log->debug(p) << BLUE << BOLD << " + no further rule applied → continues block" << OFF << endl;
    return Trool::False;
  }

  if (isPrevPartOfItem && !isCurrPartOfItem) {
    _log->debug(p) << BLUE << BOLD << " no, but prev line is part of an item" << OFF << endl;

    // The line starts a block if the previous line is part of an item (= is either the first
    // line or the continuation of an item), the current line is *not* part of an item, and the
    // previous line has capacity.
    if (hasPrevLineCapacity) {
      _log->debug(p) << BLUE << BOLD << " + prev line has capacity → starts block" << OFF << endl;
      return Trool::True;
    }

    // The line does *not* start a block if the previous line is part of an item, the current
    // line is *not* identified as the part of an item, the previous line does not end with a
    // sentence delimiter, and the current line does not start with an uppercase. This should
    // identify an item of the following form:
    //    (i) This is an item that continues in the next
    //  line. Note the smaller leftX of the second line.
    if (!text_element_utils::computeEndsWithSentenceDelimiter(prevLine) &&
          !text_element_utils::computeStartsWithUpper(line)) {
      _log->debug(p) << BLUE << BOLD << " + prev line does not end with sentence delimiter + "
          << "curr line does not start with an uppercase → continues block" << OFF << endl;
      return Trool::False;
    }
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_emphasized(const PdfTextLine* line) const {
  assert(line);
  assert(line->prevLine);

  int p = line->position->pageNum;
  PdfTextLine* prevLine = line->prevLine;

  bool isPrevLineEmphasized = text_element_utils::computeIsEmphasized(prevLine);
  bool isCurrLineEmphasized = text_element_utils::computeIsEmphasized(line);
  bool hasEqualFontName = text_element_utils::computeHasEqualFont(prevLine, line);
  bool hasEqualFontSize = text_element_utils::computeHasEqualFontSize(prevLine, line);

  _log->debug(p) << BLUE << "Is the line and prev line emphasized with same font?" << OFF << endl;
  _log->debug(p) << " └─ prevLine.isEmphasized: " << isPrevLineEmphasized << endl;
  _log->debug(p) << " └─ currLine.isEmphasized: " << isCurrLineEmphasized << endl;
  _log->debug(p) << " └─ prevLine.fontName: " << line->prevLine->fontName << endl;
  _log->debug(p) << " └─ currLine.fontName: " << line->fontName << endl;
  _log->debug(p) << " └─ prevLine.fontSize: " << line->prevLine->fontSize << endl;
  _log->debug(p) << " └─ currLine.fontSize: " << line->fontSize << endl;

  // The line does *not* start a block if the previous line and the current line are emphasized,
  // and if both lines exhibits the same font and the same font size.
  if (isPrevLineEmphasized && isCurrLineEmphasized && hasEqualFontName && hasEqualFontSize) {
    _log->debug(p) << BLUE << BOLD << " yes → continues block" << OFF << endl;
    return Trool::False;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_hangingIndent(const PdfTextBlock* pBlock,
      const PdfTextLine* line, double leftXOffsetToleranceFactorLow,
      double leftXOffsetToleranceFactorHigh) const {
  assert(pBlock);
  assert(line);
  assert(line->prevLine);

  int p = line->position->pageNum;
  PdfTextLine* prevLine = line->prevLine;

  double hangingIndent = pBlock->hangingIndent;
  double prevLeftMargin = prevLine->leftMargin;
  double currLeftMargin = line->leftMargin;
  bool isPrevNotIndented = math_utils::smaller(prevLeftMargin, hangingIndent, _doc->avgGlyphWidth);
  bool isCurrNotIndented = math_utils::smaller(currLeftMargin, hangingIndent, _doc->avgGlyphWidth);
  bool isPrevIndented = math_utils::equal(prevLeftMargin, hangingIndent, _doc->avgGlyphWidth);
  bool isCurrIndented = math_utils::equal(currLeftMargin, hangingIndent, _doc->avgGlyphWidth);
  bool isPrevMoreIndented = math_utils::larger(prevLeftMargin, hangingIndent, _doc->avgGlyphWidth);
  bool isCurrMoreIndented = math_utils::larger(currLeftMargin, hangingIndent, _doc->avgGlyphWidth);
  double leftXOffset = element_utils::computeLeftXOffset(prevLine, line);
  bool hasPrevLineCapacity = text_lines_utils::computeHasPrevLineCapacity(line);
  double leftXOffsetToleranceLow = leftXOffsetToleranceFactorLow * _doc->avgGlyphWidth;
  double leftXOffsetToleranceHigh = leftXOffsetToleranceFactorHigh * _doc->avgGlyphWidth;

  _log->debug(p) << BLUE << "Is the line part of a block in hanging indent format?" << OFF << endl;
  _log->debug(p) << " └─ block.hangingIndent:     " << hangingIndent << endl;
  _log->debug(p) << " └─ prevLine.leftMargin:     " << prevLeftMargin << endl;
  _log->debug(p) << " └─ prevLine.isNotIndented:  " << isPrevNotIndented << endl;
  _log->debug(p) << " └─ prevLine.isIndented:     " << isPrevIndented << endl;
  _log->debug(p) << " └─ prevLine.isMoreIndented: " << isPrevMoreIndented << endl;
  _log->debug(p) << " └─ prevLine.hasCapacity:    " << hasPrevLineCapacity << endl;
  _log->debug(p) << " └─ currLine.leftMargin:     " << currLeftMargin << endl;
  _log->debug(p) << " └─ currLine.isNotIndented:  " << isCurrNotIndented << endl;
  _log->debug(p) << " └─ currLine.isIndented:     " << isCurrIndented << endl;
  _log->debug(p) << " └─ currLine.isMoreIndented: " << isCurrMoreIndented << endl;
  _log->debug(p) << " └─ leftXOffset prevLine/currLine: " << leftXOffset << endl;
  _log->debug(p) << " └─ leftXOffsetToleranceFactorLow: " << leftXOffsetToleranceFactorLow << endl;
  _log->debug(p) << " └─ leftXOffsetToleranceFactorHigh:" << leftXOffsetToleranceFactorHigh << endl;
  _log->debug(p) << " └─ leftXOffsetToleranceLow: " << leftXOffsetToleranceLow << endl;
  _log->debug(p) << " └─ leftXOffsetToleranceHigh: " << leftXOffsetToleranceHigh << endl;

  // Do nothing if the block is not in hanging indent format.
  if (math_utils::equalOrSmaller(hangingIndent, 0.0)) {
    return Trool::None;
  }

  // The line starts a block if it is not indented.
  if (isCurrNotIndented) {
    _log->debug(p) << BLUE << BOLD << " yes + line is not indented → starts block" << OFF << endl;
    return Trool::True;
  }

  if (isCurrIndented) {
    _log->debug(p) << BLUE << BOLD << " yes + line is indented." << OFF << endl;

    if (isPrevMoreIndented) {
      _log->debug(p) << BLUE << BOLD << " + prev line is more indented." << OFF << endl;

      // The line does *not* start a block if it is indented, the prev. line is more indented
      // than the tolerance, and the leftX-offset between both lines is in the given tolerance.
      if (math_utils::between(leftXOffset, leftXOffsetToleranceLow, leftXOffsetToleranceHigh)) {
        _log->debug(p) << BLUE << BOLD << " + offset in tolerance → continues block" << OFF << endl;
        return Trool::False;
      }

      // The line starts a block if it is indented, the prev. line is more indented than the
      // the tolerance, and the leftX-offset between both lines is *not* in the given tolerance.
      _log->debug(p) << BLUE << BOLD << " + offset not in tolerance → starts block" << OFF << endl;
      return Trool::True;
    }

    // The line starts a block if it is indented and the prev. line has capacity.
    if (hasPrevLineCapacity) {
      _log->debug(p) << BLUE << BOLD << " + prev line has capacity → starts block" << OFF << endl;
      return Trool::True;
    }

    _log->debug(p) << BLUE << BOLD << " + no further rule applied → continues block" << OFF << endl;
    return Trool::False;
  }

  if (isCurrMoreIndented) {
    _log->debug(p) << BLUE << BOLD << " yes + line is more indented." << OFF << endl;

    if (isPrevMoreIndented) {
      _log->debug(p) << BLUE << BOLD << " + prev line is more indented." << OFF << endl;

      // The line does *not* start a block if the current and previous line are more indented
      // than the tolerance, and the leftX-offset between both lines is in the given tolerance.
      if (math_utils::between(leftXOffset, leftXOffsetToleranceLow, leftXOffsetToleranceHigh)) {
        _log->debug(p) << BLUE << BOLD << " + offset in tolerance → continues block" << OFF << endl;
        return Trool::False;
      }

      // The line starts a block if the current and previous line are more indented than the
      // tolerance, and the leftX-Offset between both lines is *not* in the tolerance.
      _log->debug(p) << BLUE << BOLD << " + offset not in tolerance → starts block" << OFF << endl;
      return Trool::True;
    }

    _log->debug(p) << BLUE << BOLD << " + no further rule applied → continues block" << OFF << endl;
    return Trool::True;
  }

  _log->debug(p) << BLUE << BOLD << " yes, no rule applied → continues block" << OFF << endl;
  return Trool::False;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_indent(const PdfTextLine* line,
      double indentToleranceFactorLow, double indentToleranceFactorHigh) const {
  assert(line);
  assert(line->prevLine);

  int p = line->position->pageNum;
  PdfTextLine* prevLine = line->prevLine;

  double prevLeftMargin = prevLine->leftMargin;
  double currLeftMargin = line->leftMargin;
  double indentTolLow = indentToleranceFactorLow * _doc->avgGlyphWidth;
  double indentTolHigh = indentToleranceFactorHigh * _doc->avgGlyphWidth;
  bool isPrevIndented = math_utils::between(prevLeftMargin, indentTolLow, indentTolHigh);
  bool isCurrIndented = math_utils::between(currLeftMargin, indentTolLow, indentTolHigh);
  bool isPrevMoreIndented = math_utils::larger(prevLeftMargin, indentTolHigh);
  bool isCurrMoreIndented = math_utils::larger(currLeftMargin, indentTolHigh);
  double absLeftXOffset = abs(element_utils::computeLeftXOffset(prevLine, line));
  bool hasPrevLineCapacity = text_lines_utils::computeHasPrevLineCapacity(line);

  _log->debug(p) << BLUE <<  "Is the line indented?" << OFF << endl;
  _log->debug(p) << " └─ prevLine.leftMargin:     " << prevLeftMargin << endl;
  _log->debug(p) << " └─ prevLine.isIndented:     " << isPrevIndented << endl;
  _log->debug(p) << " └─ prevLine.isMoreIndented: " << isPrevMoreIndented << endl;
  _log->debug(p) << " └─ prevLine.hasCapacity:    " << hasPrevLineCapacity << endl;
  _log->debug(p) << " └─ currLine.leftMargin:     " << currLeftMargin << endl;
  _log->debug(p) << " └─ currLine.isIndented:     " << isCurrIndented << endl;
  _log->debug(p) << " └─ currLine.isMoreIndented: " << isCurrMoreIndented << endl;
  _log->debug(p) << " └─ absLeftXOffset prevLine/currLine: " << absLeftXOffset << endl;
  _log->debug(p) << " └─ indentToleranceFactorLow:  " << indentToleranceFactorLow << endl;
  _log->debug(p) << " └─ indentToleranceFactorHigh: " << indentToleranceFactorHigh << endl;
  _log->debug(p) << " └─ indentToleranceLow:  " << indentTolLow << endl;
  _log->debug(p) << " └─ indentToleranceHigh: " << indentTolHigh << endl;
  _log->debug(p) << " └─ absLeftXOffset prevLine/currLine: " << absLeftXOffset << endl;

  if (isCurrMoreIndented) {
    _log->debug(p) << BLUE << BOLD <<  " yes, curr line is more indented." << OFF << endl;

    // The line does *not* start a block if it is more indented than the given tolerance and
    // its leftX offset is equal to zero (under consideration of a small threshold).
    if (math_utils::equal(absLeftXOffset, 0, _doc->avgGlyphWidth)) {
      _log->debug(p) << BLUE << BOLD << " + leftXOffset ≈ 0 → continues block" << OFF << endl;
      return Trool::False;
    }

    // The line starts a block if the current and previous line are more indented
    // than the tolerance, and the leftX-offset between both lines is *not* equal to 0.
    _log->debug(p) << BLUE << BOLD << " + leftXOffset !≈ 0 → starts block" << OFF << endl;
    return Trool::True;
  }

  if (isPrevMoreIndented) {
    _log->debug(p) << BLUE << BOLD << " unknown, but previous line is more indented." << endl;

    // The line starts a block if the current and previous line are more indented
    // than the tolerance, and the leftX-offset between both lines is equal to 0 (under
    // consideration of a small threshold).
    if (math_utils::equal(absLeftXOffset, 0, _doc->avgGlyphWidth)) {
      _log->debug(p) << BLUE << BOLD << " + leftXOffset ≈ 0 → continues block" << OFF << endl;
      return Trool::False;
    }

    // The line starts a block if the current and previous line are more indented
    // than the tolerance, and the leftX-offset between both lines is *not* equal to 0 (under
    // consideration of a small threshold).
    _log->debug(p) << BLUE << BOLD << " + leftXOffset !≈ 0 → starts block" << OFF << endl;
    return Trool::True;
  }

  // The line starts a block if it is indented.
  if (isCurrIndented) {
    _log->debug(p) << BLUE << BOLD << " yes → starts block" << OFF << endl;
    return Trool::True;
  }

  // The line starts a block if the previous line has capacity.
  if (hasPrevLineCapacity) {
    _log->debug(p) << BLUE << BOLD << " prev line has capacity → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}
