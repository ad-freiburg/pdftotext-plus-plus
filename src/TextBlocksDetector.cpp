/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 *
 *
 // The detection of text blocks is split in two steps.
 // In the first step, we split the text lines of each segment into (preliminary) text blocks
 // using rules regarding, for example, the vertical distances between the text lines and the font
 // sizes. This step was introduced because a PDF can contain text blocks with different
 // alignments and different margins (= the width of gaps between the text and the page
 // boundaries). For example, the left and right margin of the abstract is often larger than of
 // the body text. The preliminary text blocks are used to compute the identations and the margins
 // of the text lines.
 // NOTE: Initially, we computed the text line indentations by computing the gap between the text
 // lines and the *segment* boundaries. This approach often resulted in inaccurately computed text
 // line indentations, since the segments were often broader than expected, because of text parts
 // that do not share the same alignment than the body text paragraphs (like page headers or page
 // footers). A frequent consequence is that the text lines of the body text paragraphs are not
 // justified with the segment boundaries, but are positioned somewhere in the middle of the
 // segments instead. In the second step, the preliminary text blocks are split further using
 // further rules regarding, for example, the computed text line indentations or the prefixes of
 // the text lines.
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

  _log->debug() << "=========================" << endl;
  _log->debug() << BOLD << "Text Block Detection - DEBUG MODE" << OFF << endl;

  _log->debug() << "Detecting preliminary text blocks..." << endl;
  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      vector<PdfTextLine*> currentBlockLines;
      for (auto* line : segment->lines) {
        if (startsPreliminaryBlock(line)) {
          if (!currentBlockLines.empty()) {
            createTextBlock(currentBlockLines, &segment->blocks);
            currentBlockLines.clear();
          }
        }
        currentBlockLines.push_back(line);
      }
      if (!currentBlockLines.empty()) {
        createTextBlock(currentBlockLines, &segment->blocks);
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
            createTextBlock(currBlockLines, &page->blocks);
            currBlockLines.clear();
          }
          currBlockLines.push_back(line);
        }
        if (!currBlockLines.empty()) {
          createTextBlock(currBlockLines, &page->blocks);
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

  // Check if a previous line exists.
  Trool res = startsBlock_existsPrevLine(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the previous line and the current line are part of the same figure.
  res = startsBlock_sameFigure(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check the rotations of the previous and current line.
  res = startsBlock_rotation(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check the writing modes of the previous and current line.
  res = startsBlock_wMode(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check the font size.
  res = startsBlock_fontSize(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check the line distance.
  res = startsBlock_lineDistance(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  _log->debug(p) << BLUE << "continues block (no rule applied)." << OFF << endl;
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

  // Check if the distance between the current line and the previous line is increased compared to
  // the distance between the previous line and the previous but one line.
  res = startsBlock_increasedLineDistance(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the block to which the line belongs is centered.
  res = startsBlock_centered(pBlock, line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check for an enumeration item.
  res = startsBlock_item(pBlock, line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check for emphasis.
  res = startsBlock_emphasized(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check hanging indent.
  res = startsBlock_hangingIndent(pBlock, line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check indentation.
  res = startsBlock_indent(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  _log->debug(p) << BLUE << "continues block (no rule applied)." << OFF << endl;
  return false;
}

// =================================================================================================

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_existsPrevLine(const PdfTextLine* line) const {
  assert(line);

  int p = line->position->pageNum;
  _log->debug(p) << BLUE << "Does a previous text line exists?" << OFF << endl;
  _log->debug(p) << " └─ prevLine: " << (line->prevLine ? line->prevLine->text : "-") << endl;

  if (!line->prevLine) {
    _log->debug(p) << BLUE << " no → line starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_sameFigure(const PdfTextLine* line) const {
  assert(line);
  assert(line->prevLine);

  // Compute the figure overlapped by the previous line and the current line.
  int p = line->position->pageNum;
  vector<PdfFigure*>& figures = _doc->pages[p - 1]->figures;
  PdfFigure* prevLineOverlapsFigure = element_utils::overlapsFigure(line->prevLine, figures);
  PdfFigure* currLineOverlapsFigure = element_utils::overlapsFigure(line, figures);

  _log->debug(p) << BLUE << "Are the prev+curr line part of the same figure?" << OFF << endl;
  _log->debug(p) << " └─ prevLine.overlapsFigure: " << prevLineOverlapsFigure << endl;
  _log->debug(p) << " └─ currLine.overlapsFigure: " << currLineOverlapsFigure << endl;

  // The line does not start a block if the prev+curr line are part of the same figure.
  if (prevLineOverlapsFigure && prevLineOverlapsFigure == currLineOverlapsFigure) {
    _log->debug(p) << BLUE << " yes → line continues block" << OFF << endl;
    return Trool::False;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_rotation(const PdfTextLine* line) const {
  assert(line);
  assert(line->prevLine);

  int p = line->position->pageNum;
  _log->debug(p) << BLUE << "Are the rotations of the prev+curr line different?" << OFF << endl;
  _log->debug(p) << " └─ prevLine.rotation: " << line->prevLine->position->rotation << endl;
  _log->debug(p) << " └─ currLine.rotation: " << line->position->rotation << endl;

  // The line starts a new block if its rotation differs from the rotation of the previous line.
  if (line->prevLine->position->rotation != line->position->rotation) {
    _log->debug(p) << BLUE << " yes → line starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_wMode(const PdfTextLine* line) const {
  assert(line);
  assert(line->prevLine);

  int p = line->position->pageNum;
  _log->debug(p) << BLUE << "Are the wModes of the prev+curr line different?" << OFF << endl;
  _log->debug(p) << " └─ prevLine.wMode: " << line->prevLine->position->wMode << endl;
  _log->debug(p) << " └─ currLine.wMode: " << line->position->wMode << endl;

  // The line starts a new block if its writing mode differs from the writing mode of the prev line.
  if (line->prevLine->position->wMode != line->position->wMode) {
    _log->debug(p) << BLUE << " yes → line starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_fontSize(const PdfTextLine* line, double maxDelta) const {
  assert(line);
  assert(line->prevLine);

  int p = line->position->pageNum;
  _log->debug(p) << BLUE << "Are the font sizes of the prev+curr line different?" << OFF << endl;
  _log->debug(p) << " └─ prevLine.mostFreqFontSize: " << line->prevLine->fontSize << endl;
  _log->debug(p) << " └─ currLine.mostFreqFontSize: " << line->fontSize << endl;
  _log->debug(p) << " └─ prevLine.maxFontSize:      " << line->prevLine->maxFontSize << endl;
  _log->debug(p) << " └─ currLine.maxFontSize:      " << line->maxFontSize << endl;
  _log->debug(p) << " └─ maxDelta: " << maxDelta << endl;

  // The line starts a new block if the difference between neither the most frequent font sizes nor
  // the maximum font sizes of the previous text line and of the current text line are equal, under
  // consideration of a small threshold. This rule exists to split e.g., headings (which usually
  // have a larger font size) from the body text. The first condition exists to not split text
  // lines when they contain some words with larger font sizes (e.g., in a caption, the "Figure X:"
  // parts is likely to have a larger font size than the rest of the caption). The second condition
  // exists to not split text lines with many small characters (which is particularly often the
  // case when the text line contains an inline formula).
  if (!math_utils::equal(line->prevLine->fontSize, line->fontSize, maxDelta) &&
        !math_utils::equal(line->prevLine->maxFontSize, line->maxFontSize, maxDelta)) {
    _log->debug(p) << BLUE << " yes → line starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_lineDistance(const PdfTextLine* line, double minTolerance,
      double toleranceFactor) const {
  assert(line);
  assert(line->prevLine);

  // Compute the expected line distance.
  double fontSize = round(line->fontSize, FONT_SIZE_PREC);
  double expectedLineDistance = 0;
  if (_doc->mostFreqLineDistancePerFontSize.count(fontSize) > 0) {
    double eld = _doc->mostFreqLineDistancePerFontSize.at(fontSize);
    expectedLineDistance = max(expectedLineDistance, eld);
  }
  expectedLineDistance = max(expectedLineDistance, _doc->mostFreqLineDistance);

  // Compute the actual line distance.
  double actualLineDistance = text_line_utils::computeTextLineDistance(line->prevLine, line);
  actualLineDistance = round(actualLineDistance, LINE_DIST_PREC);

  // Compute the tolerance.
  double tolerance = max(minTolerance, toleranceFactor * expectedLineDistance);

  int p = line->position->pageNum;
  _log->debug(p) << BLUE << "Is the distance to prev line larger than expected?" << OFF << endl;
  _log->debug(p) << " └─ expected line distance: " << expectedLineDistance << endl;
  _log->debug(p) << " └─ actual line distance:   " << actualLineDistance << endl;
  _log->debug(p) << " └─ minTolerance:    " << minTolerance << endl;
  _log->debug(p) << " └─ toleranceFactor: " << toleranceFactor << endl;
  _log->debug(p) << " └─ tolerance:       " << tolerance << endl;

  // The line does *not* start a block if the actual line distance is negative.
  if (math_utils::equalOrSmaller(actualLineDistance, 0, 0)) {
    _log->debug(p) << BLUE << " no, distance is negative → line continues block" << OFF << endl;
    return Trool::False;
  }

  // The line starts a block if the actual line distance is larger than the expected line
  // distance, under consideration of a small threshold.
  if (math_utils::larger(actualLineDistance, expectedLineDistance, tolerance)) {
    _log->debug(p) << BLUE << " yes → line starts block" << OFF << endl;
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
  _log->debug(p) << BLUE << "Is the distance between the curr+prev line larger than "
      << "the distance between the prev+prevPrev line?" << OFF << endl;
  _log->debug(p) << " └─ distance prev+prevPrev line: " << prevDistance << endl;
  _log->debug(p) << " └─ distance curr+prev line:     " << distance << endl;
  _log->debug(p) << " └─ toleranceFactor: " << toleranceFactor << endl;
  _log->debug(p) << " └─ tolerance:       " << tolerance << endl;

  // The line starts a block if the distance between the current line and the previous line is
  // larger than the distance between the previous but one line and the previous line, under
  // consideration of the given tolerance.
  if (math_utils::larger(distance, prevDistance, tolerance)) {
    _log->debug(p) << BLUE << " yes → line starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_centered(const PdfTextBlock* pBlock, const PdfTextLine* line)
    const {
  assert(pBlock);
  assert(line);

  // Check if the line is the first line of an enumeration item. This should primarily detect
  // blocks containing affiliation information, which are often centered and prefixed by a
  // superscript.
  bool isFirstLineOfItem = text_line_utils::computeIsFirstLineOfItem(line);

  int p = line->position->pageNum;
  _log->debug(p) << BLUE << "Is the block centered?" << OFF << endl;
  _log->debug(p) << " └─ block.isCentered: " << pBlock->isCentered << endl;
  _log->debug(p) << " └─ line.isFirstLineOfItem: " << isFirstLineOfItem << endl;

  if (pBlock->isCentered) {
    _log->debug(p) << BLUE << " yes, block is centered" << OFF << endl;

    if (isFirstLineOfItem) {
      _log->debug(p) << BLUE << " + first line of item → line starts block" << OFF << endl;
      return Trool::True;
    } else {
      _log->debug(p) << BLUE << " + no first line of item → line continues block" << OFF << endl;
      return Trool::False;
    }
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_item(const PdfTextBlock* pBlock, const PdfTextLine* line) const {
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
  double xOffset = element_utils::computeLeftXOffset(line->prevLine, line);
  bool hasPrevLineCapacity = text_line_utils::computeHasPrevLineCapacity(line);

  int p = line->position->pageNum;
  _log->debug(p) << BLUE << "Is the line part of an item?" << endl;
  _log->debug(p) << " └─ prevLine.isFirstLineOfItem: " << isPrevFirstLine << endl;
  _log->debug(p) << " └─ currLine.isFirstLineOfItem: " << isCurrFirstLine << endl;
  _log->debug(p) << " └─ prevLine.isContinuationOfItem:  " << isPrevContLine << endl;
  _log->debug(p) << " └─ currLine.isContinuationOfItem:  " << isCurrContLine << endl;
  _log->debug(p) << " └─ xOffset prevLine/currLine:  " << xOffset << endl;
  _log->debug(p) << " └─ prevLine.hasCapacity: " << hasPrevLineCapacity << endl;

  // The line starts a new block if it is the first line of an item.
  if (isCurrFirstLine) {
    _log->debug(p) << BLUE << " yes, line is first line of item → starts block" << OFF << endl;
    return Trool::True;
  }

  if (isCurrContLine) {
    _log->debug(p) << BLUE << " yes, line is continuation of item" << OFF << endl;

    if (pBlock->isCentered) {
      _log->debug(p) << BLUE << " + block is centered → continues block" << OFF << endl;
      return Trool::False;
    }

    if (hasPrevLineCapacity) {
      _log->debug(p) << BLUE << " + prev line has capacity → starts block" << OFF << endl;
      return Trool::True;
    }

    if (isPrevFirstLine) {
      _log->debug(p) << BLUE << " + prev line is first item line → continues block" << OFF << endl;
      return Trool::False;
    }

    if (isPrevContLine) {
      // TODO: Parameterize the tolerance and add the tolerance to the debug output.
      if (math_utils::between(xOffset, -_doc->avgGlyphWidth, 6 * _doc->avgGlyphWidth)) {
        _log->debug(p) << BLUE << " + xOffset in indent tolerance → continues block" << OFF << endl;
        return Trool::False;
      } else {
        _log->debug(p) << BLUE << " + xOffset not in indent toleran. → starts block" << OFF << endl;
        return Trool::True;
      }
    }

    _log->debug(p) << BLUE << " → continues block" << OFF << endl;
    return Trool::False;
  }

  if (isPrevPartOfItem && !isCurrPartOfItem) {
    _log->debug(p) << BLUE << " no, but prev line is part of an item" << OFF << endl;

    if (hasPrevLineCapacity) {
      _log->debug(p) << BLUE << " + prev line has capacity → line starts block" << OFF << endl;
      return Trool::True;
    }

    // There could be an item in the following format:
    //    (i) This is an item that continues in the next
    //  line. Note the smaller leftX of the second line.
    if (!text_element_utils::computeEndsWithSentenceDelimiter(line->prevLine) &&
          !text_element_utils::computeStartsWithUpper(line)) {
      _log->debug(p) << BLUE << " + prev line does not end with sentence delimiter + "
          << "curr line does not start with an uppercase → line continues block" << OFF << endl;
      return Trool::False;
    }

    // TODO: Parameterize the 3.
    if (math_utils::larger(line->prevLine->rightMargin, 3 * _doc->avgGlyphWidth, 0)) {
      _log->debug(p) << BLUE << " + right margin of previous line is too large → line starts block"
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

  // ---------------
  // The line does not start a new block if the previous line and the current line are emphasized,
  // and if both lines exhibits the same font and the same font size. This rule exists to not split
  // titles and headings, which are often centered (which means that the left margin of the text
  // lines are > 0), in two parts in the next rule (which assumes the start of a new block if the
  // left margin of the current line is > 0).
  bool isPrevLineEmphasized = text_element_utils::computeIsEmphasized(line->prevLine);
  bool isCurrLineEmphasized = text_element_utils::computeIsEmphasized(line);
  bool hasEqualFontName = text_element_utils::computeHasEqualFont(line->prevLine, line);
  bool hasEqualFontSize = text_element_utils::computeHasEqualFontSize(line->prevLine, line, 0.1);  // TODO

  int p = line->position->pageNum;
  _log->debug(p) << "Are the previous and current line emphasized?" << endl;
  _log->debug(p) << " └─ prevLine.isEmphasized: " << isPrevLineEmphasized << endl;
  _log->debug(p) << " └─ currLine.isEmphasized: " << isCurrLineEmphasized << endl;
  _log->debug(p) << " └─ prevLine.fontName: " << line->prevLine->fontName << endl;
  _log->debug(p) << " └─ currLine.fontName: " << line->fontName << endl;
  _log->debug(p) << " └─ prevLine.fontSize: " << line->prevLine->fontSize << endl;
  _log->debug(p) << " └─ currLine.fontSize: " << line->fontSize << endl;

  if (isPrevLineEmphasized && isCurrLineEmphasized && hasEqualFontName && hasEqualFontSize) {
    _log->debug(p) << BLUE << " yes + font names/-sizes are equal → line continues block" << endl;
    return Trool::False;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_hangingIndent(const PdfTextBlock* block,
      const PdfTextLine* line) const {
  assert(block);
  assert(line);

  double hangingIndent = block->hangingIndent;
  if (math_utils::equalOrSmaller(hangingIndent, 0, 0.0001)) {
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

  int p = line->position->pageNum;
  _log->debug(p) << "Is line part of a hanging indent block?" << endl;
  _log->debug(p) << " └─ block.hangingIndent: " << block->hangingIndent << endl;
  _log->debug(p) << " └─ prevLine.leftMargin:     " << line->prevLine->leftMargin << endl;
  _log->debug(p) << " └─ prevLine.isNotIndented:  " << isPrevIndented << endl;
  _log->debug(p) << " └─ prevLine.isIndented:     " << isPrevNotIndented << endl;
  _log->debug(p) << " └─ prevLine.isMoreIndented: " << isPrevMoreIndented << endl;
  _log->debug(p) << " └─ prevLine.hasCapacity:    " << hasPrevLineCapacity << endl;
  _log->debug(p) << " └─ currLine.leftMargin:     " << line->leftMargin << endl;
  _log->debug(p) << " └─ currLine.isNotIndented:  " << isCurrIndented << endl;
  _log->debug(p) << " └─ currLine.isIndented:     " << isCurrNotIndented << endl;
  _log->debug(p) << " └─ currLine.isMoreIndented: " << isCurrMoreIndented << endl;
  _log->debug(p) << " └─ xOffset prevLine/currLine:  " << xOffset << endl;

  if (isCurrNotIndented) {
    _log->debug(p) << BLUE << " yes + current line is not indented → line starts block" << endl;
    return Trool::True;
  }

  if (isCurrIndented) {
    _log->debug(p) << BLUE << " yes + current line is indented." << endl;

    if (isPrevMoreIndented) {
      // TODO: Parameterize.
      if (math_utils::between(xOffset, -_doc->avgGlyphWidth, 3 * _doc->avgGlyphWidth)) {
        _log->debug(p) << BLUE << " + xOffset in indent tolerance → continues block" << OFF << endl;
        return Trool::False;
      }
      _log->debug(p) << BLUE << " + xOffset not in indent tolerance → starts block" << OFF << endl;
      return Trool::True;
    }

    if (hasPrevLineCapacity) {
      _log->debug(p) << BLUE << " + prev line has capacity (starts block)." << endl;
      return Trool::True;
    }

    _log->debug(p) << BLUE << " (continues block)." << endl;
    return Trool::False;
  }

  if (isCurrMoreIndented) {
    _log->debug(p) << BLUE << " yes + curr line is more indented." << endl;
    if (isPrevMoreIndented) {
      // TODO: Parameterize.
      if (math_utils::between(xOffset, -_doc->avgGlyphWidth, _doc->avgGlyphWidth)) {
        _log->debug(p) << BLUE << " + xOffset in indent tolerance (continues block)." << OFF << endl;
        return Trool::False;
      } else {
        _log->debug(p) << BLUE << " + xOffset not in indent tolerance (starts block)." << OFF << endl;
        return Trool::True;
      }
    } else {
      _log->debug(p) << BLUE << " (starts block)." << endl;
      return Trool::True;
    }
  }

  _log->debug(p) << BLUE << "yes, no rule applied (continues block)." << OFF << endl;
  return Trool::False;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_indent(const PdfTextLine* line) const {
  assert(line);

  // TODO
  bool isPrevIndented = math_utils::between(line->prevLine->leftMargin, _doc->avgGlyphWidth, 6 * _doc->avgGlyphWidth);
  bool isPrevMoreIndented = math_utils::larger(line->prevLine->leftMargin, 6 * _doc->avgGlyphWidth, 0);
  bool isCurrIndented = math_utils::between(line->leftMargin, _doc->avgGlyphWidth, 6 * _doc->avgGlyphWidth);
  bool isCurrMoreIndented = math_utils::larger(line->leftMargin, 6 * _doc->avgGlyphWidth, 0);
  double xOffset = element_utils::computeLeftXOffset(line->prevLine, line);
  bool hasPrevLineCapacity = text_line_utils::computeHasPrevLineCapacity(line);

  int p = line->position->pageNum;
  _log->debug(p) << "Is line indented?" << endl;
  _log->debug(p) << " └─ prevLine.leftMargin:     " << line->prevLine->leftMargin << endl;
  _log->debug(p) << " └─ prevLine.isIndented:     " << isPrevIndented << endl;
  _log->debug(p) << " └─ prevLine.isMoreIndented: " << isPrevMoreIndented << endl;
  _log->debug(p) << " └─ prevLine.hasCapacity:    " << hasPrevLineCapacity << endl;
  _log->debug(p) << " └─ currLine.leftMargin:     " << line->leftMargin << endl;
  _log->debug(p) << " └─ currLine.isIndented:     " << isCurrIndented << endl;
  _log->debug(p) << " └─ currLine.isMoreIndented: " << isCurrMoreIndented << endl;
  _log->debug(p) << " └─ xOffset prevLine/currLine:  " << xOffset << endl;

  if (isCurrMoreIndented) {
    if (math_utils::equal(line->position->leftX, line->prevLine->position->leftX, _doc->avgGlyphWidth)) {
      return Trool::False;
    } else {
      return Trool::True;
    }
  }

  if (isPrevMoreIndented) {
    if (math_utils::equal(line->position->leftX, line->prevLine->position->leftX, _doc->avgGlyphWidth)) {
      return Trool::False;
    } else {
      return Trool::True;
    }
  }

  if (isCurrIndented) {
    return Trool::True;
  }

  if (hasPrevLineCapacity) {
    return Trool::True;
  }

  if (smaller(line->prevLine->position->rightX, line->position->rightX, 5 * _doc->avgGlyphWidth)) {
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
void TextBlocksDetector::createTextBlock(const vector<PdfTextLine*>& lines,
      vector<PdfTextBlock*>* blocks) {
  // Do nothing if no words are given.
  if (lines.size() == 0) {
    return;
  }

  PdfTextBlock* block = new PdfTextBlock();
  block->id = createRandomString(8, "tb-");
  block->doc = _doc;

  unordered_map<string, int> fontNameFreqs;
  unordered_map<double, int> fontSizeFreqs;
  const PdfPageSegment* segment = lines[0]->segment;

  for (size_t i = 0; i < lines.size(); i++) {
    PdfTextLine* prevLine = i > 0 ? lines[i-1] : nullptr;
    PdfTextLine* currLine = lines[i];
    PdfTextLine* nextLine = i < lines.size() - 1 ? lines[i+1] : nullptr;

    double lineMinX = min(currLine->position->leftX, currLine->position->rightX);
    double lineMinY = min(currLine->position->upperY, currLine->position->lowerY);
    double lineMaxX = max(currLine->position->leftX, currLine->position->rightX);
    double lineMaxY = max(currLine->position->upperY, currLine->position->lowerY);

    // Update the x,y-coordinates.
    block->position->leftX = min(block->position->leftX, lineMinX);
    block->position->upperY = min(block->position->upperY, lineMinY);
    block->position->rightX = max(block->position->rightX, lineMaxX);
    block->position->lowerY = max(block->position->lowerY, lineMaxY);

    block->trimLeftX = std::max(block->position->leftX, segment->trimLeftX);
    block->trimUpperY = std::max(block->position->upperY, segment->trimUpperY);
    block->trimRightX = std::min(block->position->rightX, segment->trimRightX);
    block->trimLowerY = std::min(block->position->lowerY, segment->trimLowerY);

    // Count the font names and font sizes, for computing the most frequent font name / font size.
    fontNameFreqs[currLine->fontName]++;
    fontSizeFreqs[currLine->fontSize]++;

    currLine->prevLine = prevLine;
    currLine->nextLine = nextLine;
  }

  // Compute and set the most frequent font name.
  int mostFreqFontNameCount = 0;
  for (const auto& pair : fontNameFreqs) {
    if (pair.second > mostFreqFontNameCount) {
      block->fontName = pair.first;
      mostFreqFontNameCount = pair.second;
    }
  }

  // Compute and set the most frequent font size.
  int mostFreqFontSizeCount = 0;
  for (const auto& pair : fontSizeFreqs) {
    if (pair.second > mostFreqFontSizeCount) {
      block->fontSize = pair.first;
      mostFreqFontSizeCount = pair.second;
    }
  }

  // Set the page number.
  block->position->pageNum = lines[0]->position->pageNum;

  // Set the writing mode.
  block->position->wMode = lines[0]->position->wMode;

  // Set the rotation value.
  block->position->rotation = lines[0]->position->rotation;

  // Set the text.
  for (size_t i = 0; i < lines.size(); i++) {
    auto* line = lines.at(i);
    for (size_t j = 0; j < line->words.size(); j++) {
      auto* word = line->words.at(j);
      block->text += word->text;
      if (j < line->words.size() - 1) {
        block->text += " ";
      }
    }
    if (i < lines.size() - 1) {
      block->text += " ";
    }

    line->block = block;
  }

  block->isEmphasized = text_element_utils::computeIsEmphasized(block);

  block->lines = lines;

  // Set the rank.
  block->rank = blocks->size();

  block->isCentered = text_block_utils::computeIsCentered(block);

  if (blocks->size() > 0) {
    PdfTextBlock* prevBlock = blocks->at(blocks->size() - 1);
    prevBlock->nextBlock = block;
    block->prevBlock = prevBlock;
  }
  block->segment = segment;

  text_block_utils::computeTextLineMargins(block);
  block->hangingIndent = text_block_utils::computeHangingIndent(block);

  blocks->push_back(block);
}
