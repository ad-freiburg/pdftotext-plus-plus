/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cmath>
#include <iostream>
#include <regex>
#include <stack>
#include <vector>

#include "./PdfDocument.h"
#include "./TextBlocksDetector.h"

#include "./utils/LogUtils.h"
#include "./utils/MathUtils.h"
#include "./utils/PdfElementUtils.h"
#include "./utils/TextBlockUtils.h"
#include "./utils/TextLineUtils.h"
#include "./utils/Utils.h"

using namespace std;

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
        if (startsPreliminaryBlock(line)) {
          if (!currentBlockLines.empty()) {
            text_block_utils::createTextBlock(currentBlockLines, &segment->blocks);
            currentBlockLines.clear();
          }
        }
        currentBlockLines.push_back(line);
      }
      if (!currentBlockLines.empty()) {
        text_block_utils::createTextBlock(currentBlockLines, &segment->blocks);
      }
    }
  }

  _log->debug() << "Detecting final text blocks..." << endl;
  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      for (auto* pBlock : segment->blocks) {
        vector<PdfTextLine*> currBlockLines;
        for (auto* line : pBlock->lines) {
          // Detect potential footnote labels (= superscripted numbers and/or characters).
          text_line_utils::computePotentialFootnoteLabels(line, &_potentialFnLabels);

          if (startsBlock(pBlock, line) && !currBlockLines.empty()) {
            text_block_utils::createTextBlock(currBlockLines, &page->blocks);
            currBlockLines.clear();
          }
          currBlockLines.push_back(line);
        }
        if (!currBlockLines.empty()) {
          text_block_utils::createTextBlock(currBlockLines, &page->blocks);
        }
      }
    }
  }
}

// _________________________________________________________________________________________________
bool TextBlocksDetector::startsPreliminaryBlock(const PdfTextLine* line) const {
  assert(line);

  int p = line->position->pageNum;
  string prevStr = line->prevLine ? line->prevLine->text : "-";
  string nextStr = line->nextLine ? line->nextLine->text : "-";

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

  _log->debug(p) << " └─ line.prevLine: " << prevStr << endl;
  _log->debug(p) << " └─ line.nextLine: " << nextStr << endl;
  _log->debug(p) << "-------------------------" << endl;

  // Check if the line starts a block because no previous line exists.
  Trool res = startsBlock_existsPrevLine(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because the line and prev line are part of the same figure.
  res = startsBlock_sameFigure(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because of its rotation.
  res = startsBlock_rotation(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because of its writing mode.
  res = startsBlock_wMode(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because of its font size.
  res = startsBlock_fontSize(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because of its distance to the previous line.
  res = startsBlock_lineDistance(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because its distance to the previous line is increased
  // compared to the distance of the previous to the previous but one line.
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
  string prevStr = line->prevLine ? line->prevLine->text : "-";
  string nextStr = line->nextLine ? line->nextLine->text : "-";
  string parStr = line->parentTextLine ? line->parentTextLine->text : "-";
  string prevSibStr = line->prevSiblingTextLine ? line->prevSiblingTextLine->text : "-";
  string nextSibStr = line->nextSiblingTextLine ? line->nextSiblingTextLine->text : "-";

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

  _log->debug(p) << " └─ line.prevLine: " << prevStr << endl;
  _log->debug(p) << " └─ line.nextLine: " << nextStr << endl;
  _log->debug(p) << " └─ line.parent: " << parStr << endl;
  _log->debug(p) << " └─ line.prevSibling: " << prevSibStr << endl;
  _log->debug(p) << " └─ line.nextSibling: " << nextSibStr << endl;
  _log->debug(p) << "-------------------------" << endl;

  // Check if a previous line exists.
  Trool res = startsBlock_existsPrevLine(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because no previous line exists.
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

  // Check if the line starts a block because of its emphasis.
  res = startsBlock_emphasized(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because of its identation, given that the preliminary text
  // block is in hanging indent format.
  res = startsBlock_hangingIndent(pBlock, line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the line starts a block because of its identation, given that the preliminary text
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
  _log->debug(p) << BLUE << "Does a previous text line exist?" << OFF << endl;
  _log->debug(p) << " └─ line.prevLine: " << (prevLine ? prevLine->text : "-") << endl;

  // The line starts a new block if no previous line exists.
  if (!prevLine) {
    _log->debug(p) << BLUE << " no → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_sameFigure(const PdfTextLine* line) const {
  assert(line);
  assert(line->prevLine);

  // Compute the figure overlapped by the previous and current line.
  int p = line->position->pageNum;
  PdfTextLine* prevLine = line->prevLine;

  vector<PdfFigure*>& figures = _doc->pages[p - 1]->figures;
  PdfFigure* prevLineOverlapsFigure = element_utils::computeOverlapsFigure(prevLine, figures);
  PdfFigure* currLineOverlapsFigure = element_utils::computeOverlapsFigure(line, figures);

  _log->debug(p) << BLUE << "Do the prev and curr line overlap the same figure?" << OFF << endl;
  _log->debug(p) << " └─ prevLine.overlapsFigure: " << prevLineOverlapsFigure << endl;
  _log->debug(p) << " └─ currLine.overlapsFigure: " << currLineOverlapsFigure << endl;

  // The line does not start a new block if the prev and curr line overlap the same figure.
  if (prevLineOverlapsFigure && prevLineOverlapsFigure == currLineOverlapsFigure) {
    _log->debug(p) << BLUE << " yes → continues block" << OFF << endl;
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

  _log->debug(p) << BLUE << "Are the rotations of the prev and curr line different?" << OFF << endl;
  _log->debug(p) << " └─ prevLine.rotation: " << prevLine->position->rotation << endl;
  _log->debug(p) << " └─ currLine.rotation: " << line->position->rotation << endl;

  // The line starts a new block if its rotation differs from the rotation of the previous line.
  if (prevLine->position->rotation != line->position->rotation) {
    _log->debug(p) << BLUE << " yes → starts block" << OFF << endl;
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

  _log->debug(p) << BLUE << "Are the writing modes of the prev+curr line different?" << OFF << endl;
  _log->debug(p) << " └─ prevLine.wMode: " << prevLine->position->wMode << endl;
  _log->debug(p) << " └─ currLine.wMode: " << line->position->wMode << endl;

  // The line starts a new block if its writing mode differs from the writing mode of the prev line.
  if (prevLine->position->wMode != line->position->wMode) {
    _log->debug(p) << BLUE << " yes → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_fontSize(const PdfTextLine* line, double maxDelta) const {
  assert(line);
  assert(line->prevLine);

  int p = line->position->pageNum;
  PdfTextLine* prevLine = line->prevLine;

  _log->debug(p) << BLUE << "Are the font sizes of the prev+curr line different?" << OFF << endl;
  _log->debug(p) << " └─ prevLine.mostFreqFontSize: " << prevLine->fontSize << endl;
  _log->debug(p) << " └─ currLine.mostFreqFontSize: " << line->fontSize << endl;
  _log->debug(p) << " └─ prevLine.maxFontSize:      " << prevLine->maxFontSize << endl;
  _log->debug(p) << " └─ currLine.maxFontSize:      " << line->maxFontSize << endl;
  _log->debug(p) << " └─ maxDelta: " << maxDelta << endl;

  // The line starts a new block if the difference between neither the most frequent font sizes nor
  // the maximum font sizes of the previous text line and of the current text line are equal, under
  // consideration of the given delta. This rule exists to split e.g., headings (which usually
  // have a larger font size) from the body text. The first condition exists to not split text
  // lines when they contain some words with larger font sizes (e.g., in a caption, the "Figure X:"
  // parts is likely to have a larger font size than the rest of the caption). The second condition
  // exists to not split text lines with many small characters (which is particularly often the
  // case when the text line contains an inline formula).
  if (!math_utils::equal(prevLine->fontSize, line->fontSize, maxDelta) &&
        !math_utils::equal(prevLine->maxFontSize, line->maxFontSize, maxDelta)) {
    _log->debug(p) << BLUE << " yes → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_lineDistance(const PdfTextLine* line, double minTolerance,
      double toleranceFactor) const {
  assert(line);
  assert(line->prevLine);

  PdfTextLine* prevLine = line->prevLine;

  // Compute the expected line distance.
  double fontSize = round(line->fontSize, FONT_SIZE_PREC);
  double expectedLineDistance = 0;
  if (_doc->mostFreqLineDistancePerFontSize.count(fontSize) > 0) {
    double eld = _doc->mostFreqLineDistancePerFontSize.at(fontSize);
    expectedLineDistance = max(expectedLineDistance, eld);
  }
  expectedLineDistance = max(expectedLineDistance, _doc->mostFreqLineDistance);

  // Compute the actual line distance.
  double actualLineDistance = text_line_utils::computeTextLineDistance(prevLine, line);
  actualLineDistance = round(actualLineDistance, LINE_DIST_PREC);

  // Compute the tolerance.
  double tolerance = max(minTolerance, toleranceFactor * expectedLineDistance);

  int p = line->position->pageNum;
  _log->debug(p) << BLUE << "Is the dist to the prev line larger than expected?" << OFF << endl;
  _log->debug(p) << " └─ actual line distance:   " << actualLineDistance << endl;
  _log->debug(p) << " └─ expected line distance: " << expectedLineDistance << endl;
  _log->debug(p) << " └─ minTolerance:    " << minTolerance << endl;
  _log->debug(p) << " └─ toleranceFactor: " << toleranceFactor << endl;
  _log->debug(p) << " └─ tolerance:       " << tolerance << endl;

  // The line does *not* start a block if the actual line distance is negative.
  if (math_utils::equalOrSmaller(actualLineDistance, 0)) {
    _log->debug(p) << BLUE << " distance is negative → continues block" << OFF << endl;
    return Trool::False;
  }

  // The line starts a block if the actual line distance is larger than the expected line
  // distance, under consideration of the computed tolerance.
  if (math_utils::larger(actualLineDistance, expectedLineDistance, tolerance)) {
    _log->debug(p) << BLUE << " yes → starts block" << OFF << endl;
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

  // Compute the distance between the previous but one line and the previous line.
  double prevDistance = text_line_utils::computeTextLineDistance(prevPrevLine, prevLine);
  prevDistance = round(prevDistance, LINE_DIST_PREC);

  // Compute the distance between the previous line and the current line.
  double distance = text_line_utils::computeTextLineDistance(prevLine, line);
  distance = round(distance, LINE_DIST_PREC);

  // Compute the tolerance.
  double tolerance = toleranceFactor * _doc->mostFreqWordHeight;

  int p = line->position->pageNum;
  _log->debug(p) << BLUE << "Is the distance between the curr+prev line > the distance between "
     << "the prev+prevPrev line?" << OFF << endl;
  _log->debug(p) << " └─ distance btw. prev+prevPrev line: " << prevDistance << endl;
  _log->debug(p) << " └─ distance btw. curr+prev line:     " << distance << endl;
  _log->debug(p) << " └─ toleranceFactor: " << toleranceFactor << endl;
  _log->debug(p) << " └─ tolerance:       " << tolerance << endl;

  // The line starts a block if the distance between the current line and the previous line is
  // larger than the distance between the previous but one line and the previous line, under
  // consideration of the given tolerance.
  if (math_utils::larger(distance, prevDistance, tolerance)) {
    _log->debug(p) << BLUE << " yes → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_centered(const PdfTextBlock* pBlock, const PdfTextLine* line)
      const {
  assert(pBlock);
  assert(line);

  // Check if the line is the first line of an enumeration item. The motivation behind is that
  // affiliation information are often arranged in centered blocks, with the first lines of blocks
  // belonging to different blocks prefixed by an enumeration label. We want to detect the blocks
  // belonging to different affiliations as separate blocks.
  bool isFirstLineOfItem = text_line_utils::computeIsFirstLineOfItem(line);

  int p = line->position->pageNum;
  _log->debug(p) << BLUE << "Is the preliminary block centered?" << OFF << endl;
  _log->debug(p) << " └─ block.isCentered: " << pBlock->isCentered << endl;
  _log->debug(p) << " └─ line.isFirstLineOfItem: " << isFirstLineOfItem << endl;

  if (pBlock->isCentered) {
    _log->debug(p) << BLUE << " yes, block is centered" << OFF << endl;

    if (isFirstLineOfItem) {
      _log->debug(p) << BLUE << " + line is first line of item → starts block" << OFF << endl;
      return Trool::True;
    }

    _log->debug(p) << BLUE << " + line is not first line of item → continues block" << OFF << endl;
    return Trool::False;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_item(const PdfTextBlock* pBlock, const PdfTextLine* line,
      double lowerXOffsetToleranceFactor, double upperXOffsetToleranceFactor,
      double rightMarginToleranceFactor) const {
  assert(pBlock);
  assert(line);

  if (line->words.empty()) {
    return Trool::None;
  }

  PdfTextLine* prevLine = line->prevLine;
  bool isPrevFirstLine = text_line_utils::computeIsFirstLineOfItem(prevLine, &_potentialFnLabels);
  bool isCurrFirstLine = text_line_utils::computeIsFirstLineOfItem(line, &_potentialFnLabels);
  bool isPrevContLine = text_line_utils::computeIsContinuationOfItem(prevLine, &_potentialFnLabels);
  bool isCurrContLine = text_line_utils::computeIsContinuationOfItem(line, &_potentialFnLabels);
  bool isPrevPartOfItem = isPrevFirstLine || isPrevContLine;
  bool isCurrPartOfItem = isCurrFirstLine || isCurrContLine;
  double xOffset = element_utils::computeLeftXOffset(prevLine, line);
  bool hasPrevLineCapacity = text_line_utils::computeHasPrevLineCapacity(line);
  double lowerXOffsetTolerance = lowerXOffsetToleranceFactor * _doc->avgGlyphWidth;
  double upperXOffsetTolerance = upperXOffsetToleranceFactor * _doc->avgGlyphWidth;
  double rightMarginTolerance = rightMarginToleranceFactor * _doc->avgGlyphWidth;

  int p = line->position->pageNum;
  _log->debug(p) << BLUE << "Is the line part of an item?" << OFF << endl;
  _log->debug(p) << " └─ prevLine.isFirstLineOfItem: " << isPrevFirstLine << endl;
  _log->debug(p) << " └─ currLine.isFirstLineOfItem: " << isCurrFirstLine << endl;
  _log->debug(p) << " └─ prevLine.isContinuationOfItem:  " << isPrevContLine << endl;
  _log->debug(p) << " └─ currLine.isContinuationOfItem:  " << isCurrContLine << endl;
  _log->debug(p) << " └─ xOffset btw. prevLine+currLine:  " << xOffset << endl;
  _log->debug(p) << " └─ prevLine.hasCapacity: " << hasPrevLineCapacity << endl;
  _log->debug(p) << " └─ block.isCentered: " << pBlock->isCentered << endl;
  _log->debug(p) << " └─ lowerXOffsetToleranceFactor: " << lowerXOffsetToleranceFactor << endl;
  _log->debug(p) << " └─ upperXOffsetToleranceFactor: " << upperXOffsetToleranceFactor << endl;
  _log->debug(p) << " └─ lowerXOffsetTolerance: " << lowerXOffsetTolerance << endl;
  _log->debug(p) << " └─ upperXOffsetTolerance: " << upperXOffsetTolerance << endl;
  _log->debug(p) << " └─ rightMarginToleranceFactor: " << rightMarginToleranceFactor << endl;
  _log->debug(p) << " └─ rightMarginTolerance: " << rightMarginTolerance << endl;

  // The line starts a new block if it is the first line of an item.
  if (isCurrFirstLine) {
    _log->debug(p) << BLUE << " yes, line is first line of item → starts block" << OFF << endl;
    return Trool::True;
  }

  if (isCurrContLine) {
    _log->debug(p) << BLUE << " yes, line is continuation of item" << OFF << endl;

    // The line does *not* start a block if it is a continuation of an item + the block is centered.
    if (pBlock->isCentered) {
      _log->debug(p) << BLUE << " + block is centered → continues block" << OFF << endl;
      return Trool::False;
    }

    // The line starts a block if it is a continuation of an item + the previous line has capacity.
    if (hasPrevLineCapacity) {
      _log->debug(p) << BLUE << " + previous line has capacity → starts block" << OFF << endl;
      return Trool::True;
    }

    // The line does *not* start a block if it is a continuation of an item and the previous line
    // is the first line of an item.
    if (isPrevFirstLine) {
      _log->debug(p) << BLUE << " + prev line is first item line → continues block" << OFF << endl;
      return Trool::False;
    }

    if (isPrevContLine) {
      // The line does *not* start a new block if the line and the previous line are a continuation
      // of an item, and the x-offset between the lines is in the given tolerance.
      if (math_utils::between(xOffset, lowerXOffsetTolerance, upperXOffsetTolerance)) {
        _log->debug(p) << BLUE << " + previous line is continuation of item + xOffset in "
            << "tolerance → continues block" << OFF << endl;
        return Trool::False;
      }

      // The line starts a block if the line and the previous line are a continuation of an item,
      // and the x-offset between the lines is *not* in the given tolerance.
      _log->debug(p) << BLUE << " + previous line is continuation of item + xOffset not in "
            << "tolerance → starts block" << OFF << endl;
      return Trool::True;
    }

    _log->debug(p) << BLUE << " + no further rule applied → continues block" << OFF << endl;
    return Trool::False;
  }

  if (isPrevPartOfItem && !isCurrPartOfItem) {
    _log->debug(p) << BLUE << " no, but prev line is part of an item" << OFF << endl;

    // The line starts a new block if the previous line is part of an item (= is either the first
    // line or the continuation of an item), the current line is *not* part of an item, and the
    // previous line has capacity.
    if (hasPrevLineCapacity) {
      _log->debug(p) << BLUE << " + prev line has capacity → starts block" << OFF << endl;
      return Trool::True;
    }

    // The line does *not* start a new block if the previous line is part of an item, the current
    // line is *not* identified as the part of an item, the previous line does not end with a
    // sentence delimiter, and the current line does not start with an uppercase. This should
    // identify an item of the following form:
    //    (i) This is an item that continues in the next
    //  line. Note the smaller leftX of the second line.
    if (!text_element_utils::computeEndsWithSentenceDelimiter(prevLine) &&
          !text_element_utils::computeStartsWithUpper(line)) {
      _log->debug(p) << BLUE << " + prev line does not end with sentence delimiter + "
          << "curr line does not start with an uppercase → continues block" << OFF << endl;
      return Trool::False;
    }

    // The line starts a new block if the previous line is part of an item, the current line is
    // *not* part of an item, and the right margin of the previous line is larger than the given
    // tolerance.
    if (math_utils::larger(prevLine->rightMargin, rightMarginTolerance)) {
      _log->debug(p) << BLUE << " + right margin of prev line > tolerance → starts block"
          << OFF << endl;
      return Trool::True;
    }
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_emphasized(const PdfTextLine* line) const {
  assert(line);
  assert(line->prevLine);

  bool isPrevLineEmphasized = text_element_utils::computeIsEmphasized(line->prevLine);
  bool isCurrLineEmphasized = text_element_utils::computeIsEmphasized(line);
  bool hasEqualFontName = text_element_utils::computeHasEqualFont(line->prevLine, line);
  bool hasEqualFontSize = text_element_utils::computeHasEqualFontSize(line->prevLine, line);

  int p = line->position->pageNum;
  _log->debug(p) << "Are the prev+curr line emphasized and do they have the same font?" << endl;
  _log->debug(p) << " └─ prevLine.isEmphasized: " << isPrevLineEmphasized << endl;
  _log->debug(p) << " └─ currLine.isEmphasized: " << isCurrLineEmphasized << endl;
  _log->debug(p) << " └─ prevLine.fontName: " << line->prevLine->fontName << endl;
  _log->debug(p) << " └─ currLine.fontName: " << line->fontName << endl;
  _log->debug(p) << " └─ prevLine.fontSize: " << line->prevLine->fontSize << endl;
  _log->debug(p) << " └─ currLine.fontSize: " << line->fontSize << endl;

  // The line does not start a new block if the previous line and the current line are emphasized,
  // and if both lines exhibits the same font and the same font size. This rule exists to not split
  // the lines belonging to the same title or heading apart (e.g., because they are centered and
  // thus, exhibit different leftX values).
  if (isPrevLineEmphasized && isCurrLineEmphasized && hasEqualFontName && hasEqualFontSize) {
    _log->debug(p) << BLUE << " yes → continues block" << OFF << endl;
    return Trool::False;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_hangingIndent(const PdfTextBlock* pBlock,
      const PdfTextLine* line, double lowerXOffsetToleranceFactor,
      double upperXOffsetToleranceFactor) const {
  assert(pBlock);
  assert(line);

  // Do nothing if the block is not in hanging indent format.
  double hangingIndent = pBlock->hangingIndent;
  if (math_utils::equalOrSmaller(hangingIndent, 0)) {
    return Trool::None;
  }

  double prevLeftMargin = line->prevLine->leftMargin;
  double currLeftMargin = line->leftMargin;
  bool isPrevNotIndented = math_utils::smaller(prevLeftMargin, hangingIndent, _doc->avgGlyphWidth);
  bool isCurrNotIndented = math_utils::smaller(currLeftMargin, hangingIndent, _doc->avgGlyphWidth);
  bool isPrevIndented = math_utils::equal(prevLeftMargin, hangingIndent, _doc->avgGlyphWidth);
  bool isCurrIndented = math_utils::equal(currLeftMargin, hangingIndent, _doc->avgGlyphWidth);
  bool isPrevMoreIndented = math_utils::larger(prevLeftMargin, hangingIndent, _doc->avgGlyphWidth);
  bool isCurrMoreIndented = math_utils::larger(currLeftMargin, hangingIndent, _doc->avgGlyphWidth);
  double xOffset = element_utils::computeLeftXOffset(line->prevLine, line);
  bool hasPrevLineCapacity = text_line_utils::computeHasPrevLineCapacity(line);
  double lowerXOffsetTolerance = lowerXOffsetToleranceFactor * _doc->avgGlyphWidth;
  double upperXOffsetTolerance = upperXOffsetToleranceFactor * _doc->avgGlyphWidth;

  int p = line->position->pageNum;
  _log->debug(p) << "Is line part of a block in hanging indent format?" << endl;
  _log->debug(p) << " └─ block.hangingIndent:     " << pBlock->hangingIndent << endl;
  _log->debug(p) << " └─ prevLine.leftMargin:     " << prevLeftMargin << endl;
  _log->debug(p) << " └─ prevLine.isNotIndented:  " << isPrevNotIndented << endl;
  _log->debug(p) << " └─ prevLine.isIndented:     " << isPrevIndented << endl;
  _log->debug(p) << " └─ prevLine.isMoreIndented: " << isPrevMoreIndented << endl;
  _log->debug(p) << " └─ prevLine.hasCapacity:    " << hasPrevLineCapacity << endl;
  _log->debug(p) << " └─ currLine.leftMargin:     " << currLeftMargin << endl;
  _log->debug(p) << " └─ currLine.isNotIndented:  " << isCurrNotIndented << endl;
  _log->debug(p) << " └─ currLine.isIndented:     " << isCurrIndented << endl;
  _log->debug(p) << " └─ currLine.isMoreIndented: " << isCurrMoreIndented << endl;
  _log->debug(p) << " └─ xOffset prevLine/currLine: " << xOffset << endl;
  _log->debug(p) << " └─ lowerXOffsetToleranceFactor: " << lowerXOffsetToleranceFactor << endl;
  _log->debug(p) << " └─ upperXOffsetToleranceFactor: " << upperXOffsetToleranceFactor << endl;
  _log->debug(p) << " └─ lowerXOffsetTolerance: " << lowerXOffsetTolerance << endl;
  _log->debug(p) << " └─ upperXOffsetTolerance: " << upperXOffsetTolerance << endl;

  // The line starts a new block if it is not indented.
  if (isCurrNotIndented) {
    _log->debug(p) << BLUE << " yes + curr line is not indented → starts block" << OFF << endl;
    return Trool::True;
  }

  if (isCurrIndented) {
    _log->debug(p) << BLUE << " yes + curr line is indented." << OFF << endl;

    if (isPrevMoreIndented) {
      _log->debug(p) << BLUE << " + prev line is more indented." << OFF << endl;

      // The line does *not* start a new block if it is indented, the prev. line is more indented
      // than the tolerance, and the xOffset between both lines is in the given tolerance.
      if (math_utils::between(xOffset, lowerXOffsetTolerance, upperXOffsetTolerance)) {
        _log->debug(p) << BLUE << " + xOffset in tolerance → continues block" << OFF << endl;
        return Trool::False;
      }

      // The line starts a new block if it is indented, the prev. line is more indented than the
      // the tolerance, and the xOffset between both lines is *not* in the given tolerance.
      _log->debug(p) << BLUE << " + xOffset not in tolerance → starts block" << OFF << endl;
      return Trool::True;
    }

    // The line starts a new block if it is indented and the prev. line has capacity.
    if (hasPrevLineCapacity) {
      _log->debug(p) << BLUE << " + prev line has capacity → starts block" << OFF << endl;
      return Trool::True;
    }

    _log->debug(p) << BLUE << " no further rule applied → continues block" << OFF << endl;
    return Trool::False;
  }

  if (isCurrMoreIndented) {
    _log->debug(p) << BLUE << " yes + curr line is more indented." << OFF << endl;

    if (isPrevMoreIndented) {
      _log->debug(p) << BLUE << " + prev line is more indented." << OFF << endl;

      // The line does *not* start a new block if the current and previous line are more indented
      // than the tolerance, and the xOffset between both lines is in the given tolerance.
      if (math_utils::between(xOffset, lowerXOffsetTolerance, upperXOffsetTolerance)) {
        _log->debug(p) << BLUE << " + xOffset in tolerance → continues block" << OFF << endl;
        return Trool::False;
      }

      // The line starts a new block if the current and previous line are more indented
      // than the tolerance, and the xOffset between both lines is *not* in the given tolerance.
      _log->debug(p) << BLUE << " + xOffset not in tolerance → starts block" << OFF << endl;
      return Trool::True;
    }

    _log->debug(p) << BLUE << " + no further rule applied → continues block" << OFF << endl;
    return Trool::True;
  }

  _log->debug(p) << BLUE << " yes, no further rule applied → continues block" << OFF << endl;
  return Trool::False;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_indent(const PdfTextLine* line,
      double lowerLeftMarginToleranceFactor, double upperLeftMarginToleranceFactor,
      double rightXToleranceFactor) const {
  assert(line);
  assert(line->prevLine);

  double prevLeftX = line->prevLine->position->leftX;
  double currLeftX = line->position->leftX;
  double prevRightX = line->prevLine->position->rightX;
  double currRightX = line->position->rightX;
  double prevLeftMargin = line->prevLine->leftMargin;
  double currLeftMargin = line->leftMargin;
  double lowerLeftMarginTol = lowerLeftMarginToleranceFactor * _doc->avgGlyphWidth;
  double upperLeftMarginTol = upperLeftMarginToleranceFactor * _doc->avgGlyphWidth;
  bool isPrevIndented = math_utils::between(prevLeftMargin, lowerLeftMarginTol, upperLeftMarginTol);
  bool isCurrIndented = math_utils::between(currLeftMargin, lowerLeftMarginTol, upperLeftMarginTol);
  bool isPrevMoreIndented = math_utils::larger(prevLeftMargin, upperLeftMarginTol);
  bool isCurrMoreIndented = math_utils::larger(currLeftMargin, upperLeftMarginTol);
  double xOffset = element_utils::computeLeftXOffset(line->prevLine, line);
  bool hasPrevLineCapacity = text_line_utils::computeHasPrevLineCapacity(line);
  double rightXTolerance = rightXToleranceFactor * _doc->avgGlyphWidth;

  int p = line->position->pageNum;
  _log->debug(p) << "Is the line indented?" << endl;
  _log->debug(p) << " └─ prevLine.leftX:          " << prevLeftX << endl;
  _log->debug(p) << " └─ prevLine.leftMargin:     " << prevLeftMargin << endl;
  _log->debug(p) << " └─ prevLine.isIndented:     " << isPrevIndented << endl;
  _log->debug(p) << " └─ prevLine.isMoreIndented: " << isPrevMoreIndented << endl;
  _log->debug(p) << " └─ prevLine.hasCapacity:    " << hasPrevLineCapacity << endl;
  _log->debug(p) << " └─ currLine.leftX:          " << currLeftX << endl;
  _log->debug(p) << " └─ currLine.leftMargin:     " << currLeftMargin << endl;
  _log->debug(p) << " └─ currLine.isIndented:     " << isCurrIndented << endl;
  _log->debug(p) << " └─ currLine.isMoreIndented: " << isCurrMoreIndented << endl;
  _log->debug(p) << " └─ xOffset prevLine/currLine: " << xOffset << endl;
  _log->debug(p) << " └─ lowerLeftMarginToleranceFac: " << lowerLeftMarginToleranceFactor << endl;
  _log->debug(p) << " └─ upperLeftMarginToleranceFac: " << upperLeftMarginToleranceFactor << endl;
  _log->debug(p) << " └─ lowerLeftMarginTolerance: " << lowerLeftMarginTol << endl;
  _log->debug(p) << " └─ upperLeftMarginTolerance: " << upperLeftMarginTol << endl;
  _log->debug(p) << " └─ rightXToleranceFactor: " << rightXToleranceFactor << endl;
  _log->debug(p) << " └─ rightXTolerance: " << rightXTolerance << endl;

  if (isCurrMoreIndented) {
    _log->debug(p) << BLUE << " yes, curr line is more indented." << OFF << endl;

    // The line does *not* start a new block if it is more indented than the given tolerance and
    // its leftX coordinate is equal to the leftX coordinate of the previous line.
    if (math_utils::equal(prevLeftX, currLeftX, _doc->avgGlyphWidth)) {
      _log->debug(p) << BLUE << " + prevLine.leftX == line.leftX → continues block" << OFF << endl;
      return Trool::False;
    }

    // The line starts a new block if it is more indented than the given tolerance and its leftX
    // coordinate is *not* equal to the leftX coordinate of the previous line.
    _log->debug(p) << BLUE << " + prevLine.leftX != line.leftX → starts block" << OFF << endl;
    return Trool::True;
  }

  if (isPrevMoreIndented) {
    _log->debug(p) << BLUE << " unknown, but previous line is more indented." << endl;

    // The line does *not* start a new block if the previous line is more indented than the given
    // tolerance and its leftX coordinate is equal to the leftX coordinate of the current line.
    if (math_utils::equal(prevLeftX, currLeftX, _doc->avgGlyphWidth)) {
      _log->debug(p) << BLUE << " + prevLine.leftX == line.leftX → continues block" << OFF << endl;
      return Trool::False;
    }

    // The line starts a new block if the previous line is more indented than the given tolerance
    // and its leftX coordinate is *not* equal to the leftX coordinate of the current line.
    _log->debug(p) << BLUE << " + prevLine.leftX != line.leftX → starts block" << OFF << endl;
    return Trool::True;
  }

  // The line starts a new block if it is indented.
  if (isCurrIndented) {
    _log->debug(p) << BLUE << " yes → starts block" << OFF << endl;
    return Trool::True;
  }

  // The line starts a new block if the previous line has capacity.
  if (hasPrevLineCapacity) {
    _log->debug(p) << BLUE << " prev line has capacity → starts block" << OFF << endl;
    return Trool::True;
  }

  // The line starts a new block if the rightX offset between the current and previous line is
  // larger than the given threshold.
  if (math_utils::smaller(prevRightX, currRightX, rightXTolerance)) {
    _log->debug(p) << BLUE << " rightXOffset > tolerance → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}
