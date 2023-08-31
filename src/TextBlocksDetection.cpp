/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <string>
#include <utility>  // std::pair
#include <vector>

#include "./Config.h"
#include "./PdfDocument.h"
#include "./TextBlocksDetection.h"
#include "./utils/Log.h"
#include "./utils/MathUtils.h"
#include "./utils/PdfElementsUtils.h"
#include "./utils/TextBlocksDetectionUtils.h"
#include "./utils/TextUtils.h"
#include "./utils/Trool.h"

using std::endl;
using std::string;
using std::vector;

using ppp::config::TextBlocksDetectionConfig;
using ppp::types::PdfDocument;
using ppp::types::PdfFigure;
using ppp::types::PdfTextBlock;
using ppp::types::PdfTextLine;
using ppp::utils::Trool;
using ppp::utils::TextBlocksDetectionUtils;
using ppp::utils::elements::computeHasEqualFont;
using ppp::utils::elements::computeHasEqualFontSize;
using ppp::utils::elements::computeLeftXOffset;
using ppp::utils::elements::computeVerticalGap;
using ppp::utils::log::BLUE;
using ppp::utils::log::BOLD;
using ppp::utils::log::OFF;
using ppp::utils::log::Logger;
using ppp::utils::math::between;
using ppp::utils::math::equal;
using ppp::utils::math::equalOrSmaller;
using ppp::utils::math::larger;
using ppp::utils::math::maximum;
using ppp::utils::math::round;
using ppp::utils::math::smaller;
using ppp::utils::text::endsWithSentenceDelimiter;
using ppp::utils::text::shorten;
using ppp::utils::text::startsWithUpper;

// =================================================================================================

namespace ppp::modules {

// _________________________________________________________________________________________________
TextBlocksDetection::TextBlocksDetection(
    PdfDocument* doc,
    const TextBlocksDetectionConfig* config) {
  _doc = doc;
  _config = config;
  _utils = new TextBlocksDetectionUtils(config);
  _log = new Logger(config->logLevel, config->logPageFilter);
}

// _________________________________________________________________________________________________
TextBlocksDetection::~TextBlocksDetection() {
  delete _log;
  delete _utils;
}

// _________________________________________________________________________________________________
void TextBlocksDetection::process() {
  assert(_doc);

  _log->info() << "Detecting text blocks..." << endl;
  _log->debug() << "=======================================" << endl;
  _log->debug() << BOLD << "DEBUG MODE" << OFF << endl;
  _log->debug() << "=======================================" << endl;
  _log->debug() << BOLD << "Detecting preliminary text blocks..." << OFF << endl;
  _log->debug() << "=======================================" << endl;
  _q = "(pre) ";

  for (auto* page : _doc->pages) {
    int p = page->pageNum;
    for (auto* segment : page->segments) {
      vector<PdfTextLine*> currentBlockLines;
      for (auto* line : segment->lines) {
        _log->debug(p) << _q << BOLD << "line: \"" << shorten(line->text) << "\"" << OFF << endl;
        _log->debug(p) << _q << " └─ line.page:   " << line->pos->pageNum << endl;
        _log->debug(p) << _q << " └─ line.leftX:  " << line->pos->leftX << endl;
        _log->debug(p) << _q << " └─ line.upperY: " << line->pos->upperY << endl;
        _log->debug(p) << _q << " └─ line.rightX: " << line->pos->rightX << endl;
        _log->debug(p) << _q << " └─ line.lowerY: " << line->pos->lowerY << endl;
        if (line->pos->rotation != 0) {
          _log->debug(p) << _q << " └─ line.rotation:  " << line->pos->rotation << endl;
          _log->debug(p) << _q << " └─ line.rotLeftX:  " << line->pos->getRotLeftX() << endl;
          _log->debug(p) << _q << " └─ line.rotUpperY: " << line->pos->getRotUpperY() << endl;
          _log->debug(p) << _q << " └─ line.rotRightX: " << line->pos->getRotRightX() << endl;
          _log->debug(p) << _q << " └─ line.rotLowerY: " << line->pos->getRotLowerY() << endl;
        }
        string prevLineStr = line->prevLine ? line->prevLine->text : "-";
        string nextLineStr = line->nextLine ? line->nextLine->text : "-";
        _log->debug(p) << _q << " └─ line.prevLine: " << shorten(prevLineStr) << endl;
        _log->debug(p) << _q << " └─ line.nextLine: " << shorten(nextLineStr) << endl;
        _log->debug(p) << "---------------------------------------" << endl;

        if (startsPreliminaryBlock(line) && !currentBlockLines.empty()) {
          _utils->createTextBlock(currentBlockLines, &segment->blocks);
          currentBlockLines.clear();
        }

        currentBlockLines.push_back(line);
        _log->debug(p) << "=======================================" << endl;
      }
      if (!currentBlockLines.empty()) {
        _utils->createTextBlock(currentBlockLines, &segment->blocks);
      }
    }
  }

  _log->debug() << BOLD << "Splitting preliminary text blocks..." << OFF << endl;
  _log->debug() << "=======================================" << endl;
  _q = "(fin) ";

  for (auto* page : _doc->pages) {
    int p = page->pageNum;
    for (auto* segment : page->segments) {
      for (auto* block : segment->blocks) {
        vector<PdfTextLine*> currentBlockLines;
        for (auto* line : block->lines) {
          _log->debug(p) << _q << BOLD << "line: \"" << line->text << "\"" << OFF << endl;
          _log->debug(p) << _q << " └─ line.page:   " << line->pos->pageNum << endl;
          _log->debug(p) << _q << " └─ line.leftX:  " << line->pos->leftX << endl;
          _log->debug(p) << _q << " └─ line.upperY: " << line->pos->upperY << endl;
          _log->debug(p) << _q << " └─ line.rightX: " << line->pos->rightX << endl;
          _log->debug(p) << _q << " └─ line.lowerY: " << line->pos->lowerY << endl;
          if (line->pos->rotation != 0) {
            _log->debug(p) << _q << " └─ line.rotation:  " << line->pos->rotation << endl;
            _log->debug(p) << _q << " └─ line.rotLeftX:  " << line->pos->getRotLeftX() << endl;
            _log->debug(p) << _q << " └─ line.rotUpperY: " << line->pos->getRotUpperY() << endl;
            _log->debug(p) << _q << " └─ line.rotRightX: " << line->pos->getRotRightX() << endl;
            _log->debug(p) << _q << " └─ line.rotLowerY: " << line->pos->getRotLowerY() << endl;
          }
          string prevLineStr = line->prevLine ? shorten(line->prevLine->text) : "-";
          string nextLineStr = line->nextLine ? shorten(line->nextLine->text) : "-";
          string parentLineStr = line->parentLine ? shorten(line->parentLine->text) : "-";
          string prevSibStr = line->prevSiblingLine ? shorten(line->prevSiblingLine->text) : "-";
          string nextSibStr = line->nextSiblingLine ? shorten(line->nextSiblingLine->text) : "-";
          _log->debug(p) << _q << " └─ line.prevLine: " << prevLineStr << endl;
          _log->debug(p) << _q << " └─ line.nextLine: " << nextLineStr << endl;
          _log->debug(p) << _q << " └─ line.parentLine: " << parentLineStr << endl;
          _log->debug(p) << _q << " └─ line.prevSiblingLine: " << prevSibStr << endl;
          _log->debug(p) << _q << " └─ line.nextSiblingLine: " << nextSibStr << endl;
          _log->debug(p) << "---------------------------------------" << endl;

          // Detect potential footnote labels in the line (= superscripted characters). This is
          // needed to detect the start of footnotes (we want to detect each footnote separately).
          _utils->computePotentialFootnoteLabels(line, &_potentFnLabels);

          if (startsBlock(block, line) && !currentBlockLines.empty()) {
            _utils->createTextBlock(currentBlockLines, &page->blocks);
            currentBlockLines.clear();
          }
          currentBlockLines.push_back(line);
          _log->debug(p) << "=======================================" << endl;
        }
        if (!currentBlockLines.empty()) {
          _utils->createTextBlock(currentBlockLines, &page->blocks);
        }
      }
    }
  }
}

// _________________________________________________________________________________________________
bool TextBlocksDetection::startsPreliminaryBlock(const PdfTextLine* line) const {
  assert(line);

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

  int p = line->pos->pageNum;
  _log->debug(p) << _q << BOLD << BLUE << "no rule applied → continues block" << OFF << endl;

  return false;
}

// _________________________________________________________________________________________________
bool TextBlocksDetection::startsBlock(const PdfTextBlock* pBlock, const PdfTextLine* line) const {
  assert(pBlock);
  assert(line);

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

  int p = line->pos->pageNum;
  _log->debug(p) << _q << BOLD << BLUE << "no rule applied → continues block" << OFF << endl;

  return false;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetection::startsBlock_existsPrevLine(const PdfTextLine* line) const {
  assert(line);

  int p = line->pos->pageNum;
  const PdfTextLine* prev = line->prevLine;

  _log->debug(p) << _q << BLUE << "Does it have no previous line?" << OFF << endl;
  _log->debug(p) << _q << " └─ line.prevLine: " << (prev ? shorten(prev->text) : "-") << endl;

  // The line starts a new block if it has no previous line.
  if (!prev) {
    _log->debug(p) << _q << BLUE << BOLD << " yes → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetection::startsBlock_sameFigure(const PdfTextLine* line) const {
  assert(line);
  assert(line->prevLine);

  int p = line->pos->pageNum;
  const PdfTextLine* prevLine = line->prevLine;

  // Compute the figure overlapped by the previous and the current line.
  vector<PdfFigure*>& figures = _doc->pages[p - 1]->figures;
  PdfFigure* prevLineOverlapsFigure = _utils->computeOverlapsFigure(prevLine, figures);
  PdfFigure* currLineOverlapsFigure = _utils->computeOverlapsFigure(line, figures);

  _log->debug(p) << _q << BLUE << "Does it overlap the same fig as the prev line?" << OFF << endl;
  _log->debug(p) << _q << " └─ prevLine.overlapsFigure: " << prevLineOverlapsFigure << endl;
  _log->debug(p) << _q << " └─ currLine.overlapsFigure: " << currLineOverlapsFigure << endl;

  // The line does *not* start a new block if it overlaps the same figure as the previous line.
  if (prevLineOverlapsFigure && prevLineOverlapsFigure == currLineOverlapsFigure) {
    _log->debug(p) << _q << BLUE << BOLD << " yes → continues block" << OFF << endl;
    return Trool::False;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetection::startsBlock_rotation(const PdfTextLine* line) const {
  assert(line);
  assert(line->prevLine);

  int p = line->pos->pageNum;
  const PdfTextLine* prevLine = line->prevLine;

  _log->debug(p) << _q << BLUE << "Does it have another rotation than prev line?" << OFF << endl;
  _log->debug(p) << _q << " └─ prevLine.rotation: " << prevLine->pos->rotation << endl;
  _log->debug(p) << _q << " └─ currLine.rotation: " << line->pos->rotation << endl;

  // The line starts a block if it has another rotation than the previous line.
  if (prevLine->pos->rotation != line->pos->rotation) {
    _log->debug(p) << _q << BLUE << BOLD << " yes → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetection::startsBlock_wMode(const PdfTextLine* line) const {
  assert(line);
  assert(line->prevLine);

  int p = line->pos->pageNum;
  const PdfTextLine* prevLine = line->prevLine;

  _log->debug(p) << _q << BLUE << "Does it have another wMode than the prev line?" << OFF << endl;
  _log->debug(p) << _q << " └─ prevLine.wMode: " << prevLine->pos->wMode << endl;
  _log->debug(p) << _q << " └─ currLine.wMode: " << line->pos->wMode << endl;

  // The line starts a block if it has another writing mode than the previous line.
  if (prevLine->pos->wMode != line->pos->wMode) {
    _log->debug(p) << _q << BLUE << BOLD << " yes → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetection::startsBlock_fontSize(const PdfTextLine* line) const {
  assert(line);
  assert(line->prevLine);

  int p = line->pos->pageNum;
  const PdfTextLine* prevLine = line->prevLine;
  double tolerance = _config->fsEqualTolerance;

  _log->debug(p) << _q << BLUE << "Does it have another font size than prev line?" << OFF << endl;
  _log->debug(p) << _q << " └─ prevLine.mostFreqFontSize: " << prevLine->fontSize << endl;
  _log->debug(p) << _q << " └─ currLine.mostFreqFontSize: " << line->fontSize << endl;
  _log->debug(p) << _q << " └─ prevLine.maxFontSize: " << prevLine->maxFontSize << endl;
  _log->debug(p) << _q << " └─ currLine.maxFontSize: " << line->maxFontSize << endl;
  _log->debug(p) << _q << " └─ tolerance: " << tolerance << endl;

  // The line starts a block if neither the most frequent font size of the line is equal to
  // the most frequent font size in the prev line (under consideration of the given tolerance), nor
  // the max font size of the line is equal to the max font size in the prev line (under
  // consideration of the given tolerance). The first condition exists to not split text
  // lines when they contain few words with larger font sizes (e.g., in a caption, the "Figure X:"
  // parts is likely to have a larger font size than the rest of the caption). The second condition
  // exists to not split text lines with many small characters (which is particularly often the
  // case when the text line contains an inline formula).
  bool isEqualFontSize = equal(prevLine->fontSize, line->fontSize, tolerance);
  bool isEqualMaxFontSize = equal(prevLine->maxFontSize, line->maxFontSize, tolerance);
  if (!isEqualFontSize && !isEqualMaxFontSize) {
    _log->debug(p) << _q << BLUE << BOLD << " yes → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetection::startsBlock_lineDistance(const PdfTextLine* line) const {
  assert(line);
  assert(line->prevLine);

  int p = line->pos->pageNum;
  const PdfTextLine* prevLine = line->prevLine;

  // Compute the expected line distance.
  double fontSize = round(line->fontSize, _config->fontSizePrecision);
  double expectedLineDistance = 0;
  if (_doc->mostFreqLineDistancePerFontSize.count(fontSize) > 0) {
    double eld = _doc->mostFreqLineDistancePerFontSize.at(fontSize);
    expectedLineDistance = maximum(expectedLineDistance, eld);
  }
  expectedLineDistance = maximum(expectedLineDistance, _doc->mostFreqLineDistance);

  // Compute the actual line distance.
  double actualLineDistance = computeVerticalGap(prevLine, line);
  actualLineDistance = round(actualLineDistance, _config->lineDistancePrecision);

  double lineDistanceDiff = actualLineDistance - expectedLineDistance;

  // Compute the threshold.
  double threshold = _config->getExpectedLineDistanceThreshold(_doc, expectedLineDistance);

  _log->debug(p) << _q << BLUE << "Is the dist to prev line larger than expected?" << OFF << endl;
  _log->debug(p) << _q << " └─ actual line distance: " << actualLineDistance << endl;
  _log->debug(p) << _q << " └─ expected line distance: " << expectedLineDistance << endl;
  _log->debug(p) << _q << " └─ line distance diff: " << lineDistanceDiff << endl;
  _log->debug(p) << _q << " └─ threshold: " << threshold << endl;

  // The line does *not* start a new block if the actual line distance is negative.
  if (equalOrSmaller(actualLineDistance, 0)) {
    _log->debug(p) << _q << BLUE << BOLD << " no, distance < 0 → continues block" << OFF << endl;
    return Trool::False;
  }

  // The line starts a new block if the actual line distance is larger than the expected line
  // distance, under consideration of the computed tolerance.
  if (larger(actualLineDistance, expectedLineDistance, threshold)) {
    _log->debug(p) << _q << BLUE << BOLD << " yes → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetection::startsBlock_increasedLineDistance(const PdfTextLine* line) const {
  assert(line);
  assert(line->prevLine);

  const PdfTextLine* prevLine = line->prevLine;
  const PdfTextLine* prevPrevLine = prevLine->prevLine;
  if (!prevPrevLine) {
    return Trool::None;
  }

  int p = line->pos->pageNum;

  // Compute the distance between the previous but one line and the previous line.
  double prevDistance = computeVerticalGap(prevPrevLine, prevLine);
  prevDistance = round(prevDistance, _config->lineDistancePrecision);

  // Compute the distance between the previous line and the current line.
  double distance = computeVerticalGap(prevLine, line);
  distance = round(distance, _config->lineDistancePrecision);

  // Compute the tolerance.
  double threshold = _config->getPrevCurrNextLineDistanceTolerance(_doc);

  _log->debug(p) << _q << BLUE << "Is curr+prev distance > prev+prevPrev distance?" << OFF << endl;
  _log->debug(p) << _q << " └─ curr+prev line distance: " << distance << endl;
  _log->debug(p) << _q << " └─ prev+prevPrev line distance: " << prevDistance << endl;
  _log->debug(p) << _q << " └─ threshold: " << threshold << endl;

  // The line starts a block if the curr+prev line distance is larger than the prev+prevPrev line
  // distance, under consideration of the computed tolerance.
  if (larger(distance, prevDistance, threshold)) {
    _log->debug(p) << _q << BLUE << BOLD << " yes → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetection::startsBlock_centered(const PdfTextBlock* pBlock, const PdfTextLine* line)
      const {
  assert(pBlock);
  assert(line);

  // Check if the line is the first line of an enumeration item.
  bool isFirstLineOfItem = _utils->computeIsFirstLineOfItem(line);

  int p = line->pos->pageNum;
  _log->debug(p) << _q << BLUE << "Is the line part of a centered block?" << OFF << endl;
  _log->debug(p) << _q << " └─ block.isCentered: " << pBlock->isLinesCentered << endl;
  _log->debug(p) << _q << " └─ line.isFirstLineOfItem: " << isFirstLineOfItem << endl;

  // Do nothing if the block is not centered.
  if (!pBlock->isLinesCentered) {
    return Trool::None;
  }

  if (isFirstLineOfItem) {
    _log->debug(p) << _q << BLUE << BOLD << " yes + is 1st item line → starts block" << OFF << endl;
    return Trool::True;
  }

  _log->debug(p) << _q << BLUE << BOLD << " yes → continues block" << OFF << endl;
  return Trool::False;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetection::startsBlock_item(const PdfTextBlock* pBlock, const PdfTextLine* line)
    const {
  assert(pBlock);
  assert(line);

  int p = line->pos->pageNum;
  const PdfTextLine* prevLine = line->prevLine;

  bool isPrevFirstLine = _utils->computeIsFirstLineOfItem(prevLine, &_potentFnLabels);
  bool isCurrFirstLine = _utils->computeIsFirstLineOfItem(line, &_potentFnLabels);
  bool isPrevContLine = _utils->computeIsContinuationOfItem(prevLine, &_potentFnLabels);
  bool isCurrContLine = _utils->computeIsContinuationOfItem(line, &_potentFnLabels);
  bool isPrevPartOfItem = isPrevFirstLine || isPrevContLine;
  bool isCurrPartOfItem = isCurrFirstLine || isCurrContLine;
  double leftXOffset = computeLeftXOffset(prevLine, line);
  bool hasPrevLineCapacity = _utils->computeHasPrevLineCapacity(line->prevLine, line);
  pair<double, double> leftXOffsetTolInterval = _config->getLeftXOffsetToleranceInterval(_doc);
  double leftXOffsetToleranceLow = leftXOffsetTolInterval.first;
  double leftXOffsetToleranceHigh = leftXOffsetTolInterval.second;

  _log->debug(p) << _q << BLUE << "Is the curr line or prev line part of an item?" << OFF << endl;
  _log->debug(p) << _q << " └─ prevLine.isFirstLineOfItem: " << isPrevFirstLine << endl;
  _log->debug(p) << _q << " └─ currLine.isFirstLineOfItem: " << isCurrFirstLine << endl;
  _log->debug(p) << _q << " └─ prevLine.isContinuationOfItem: " << isPrevContLine << endl;
  _log->debug(p) << _q << " └─ currLine.isContinuationOfItem: " << isCurrContLine << endl;
  _log->debug(p) << _q << " └─ leftXOffset prevLine/currLine: " << leftXOffset << endl;
  _log->debug(p) << _q << " └─ prevLine.hasCapacity: " << hasPrevLineCapacity << endl;
  _log->debug(p) << _q << " └─ block.isCentered: " << pBlock->isLinesCentered << endl;
  _log->debug(p) << _q << " └─ leftXOffsetToleranceLow:  " << leftXOffsetToleranceLow << endl;
  _log->debug(p) << _q << " └─ leftXOffsetToleranceHigh: " << leftXOffsetToleranceHigh << endl;

  // The line starts a block if it is the first line of an item.
  if (isCurrFirstLine) {
    _log->debug(p) << _q << BLUE << BOLD << " yes, is 1st item line → starts block" << OFF << endl;
    return Trool::True;
  }

  if (isCurrContLine) {
    _log->debug(p) << _q << BLUE << BOLD << " yes, is continuation of item" << OFF << endl;

    // The line does *not* start a block if it is a continuation of an item + the block is centered.
    if (pBlock->isLinesCentered) {
      _log->debug(p) << _q << BLUE << BOLD << " + block centered → continues block" << OFF << endl;
      return Trool::False;
    }

    // The line starts a block if it is a continuation of an item + the prev line has capacity.
    if (hasPrevLineCapacity) {
      _log->debug(p) << _q << BLUE << BOLD << " + prevLine capacity → starts block" << OFF << endl;
      return Trool::True;
    }

    // The line does *not* start a block if it is a continuation of an item and the previous line
    // is the first line of an item.
    if (isPrevFirstLine) {
      _log->debug(p) << _q << BLUE << BOLD << " + prev line is first line of item → continues block"
          << OFF << endl;
      return Trool::False;
    }

    if (isPrevContLine) {
      // The line does *not* start a block if the line and the previous line are a continuation
      // of an item, and the leftX-offset between the lines is in the given tolerance.
      if (between(leftXOffset, leftXOffsetToleranceLow, leftXOffsetToleranceHigh)) {
        _log->debug(p) << _q << BLUE << BOLD << " + prev line is continuation of item "
            << "+ leftX-offset in tolerance → continues block" << OFF << endl;
        return Trool::False;
      }

      // The line starts a block if the line and the previous line are a continuation of an item,
      // and the leftX-offset between the lines is *not* in the given tolerance.
      _log->debug(p) << _q << BLUE << BOLD << " + prev line is continuation of item "
            << "+ leftX-offset not in tolerance → starts block" << OFF << endl;
      return Trool::True;
    }

    _log->debug(p) << _q << BLUE << BOLD << " + no rule applied → continues block" << OFF << endl;
    return Trool::False;
  }

  if (isPrevPartOfItem && !isCurrPartOfItem) {
    _log->debug(p) << _q << BLUE << BOLD << " no, but prev line is part of an item" << OFF << endl;

    // The line starts a block if the previous line is part of an item (= is either the first
    // line or the continuation of an item), the current line is *not* part of an item, and the
    // previous line has capacity.
    if (hasPrevLineCapacity) {
      _log->debug(p) << _q << BLUE << BOLD << " + prevLine capacity → starts block" << OFF << endl;
      return Trool::True;
    }

    // The line does *not* start a block if the previous line is part of an item, the current
    // line is *not* identified as the part of an item, the previous line does not end with a
    // sentence delimiter, and the current line does not start with an uppercase. This should
    // identify an item of the following form:
    //    (i) This is an item that continues in the next
    //  line. Note the smaller leftX of the second line.
    if (!endsWithSentenceDelimiter(prevLine->text) && !startsWithUpper(line->text)) {
      _log->debug(p) << _q << BLUE << BOLD << " + prev line does not end with sentence delimiter "
          << "+ curr line does not start with an uppercase → continues block" << OFF << endl;
      return Trool::False;
    }
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetection::startsBlock_emphasized(const PdfTextLine* line) const {
  assert(line);
  assert(line->prevLine);

  int p = line->pos->pageNum;
  const PdfTextLine* prevLine = line->prevLine;

  bool isPrevLineEmphasized = _utils->computeIsEmphasized(prevLine);
  bool isCurrLineEmphasized = _utils->computeIsEmphasized(line);
  bool hasEqualFontName = computeHasEqualFont(prevLine, line);
  bool hasEqualFontSize = computeHasEqualFontSize(prevLine, line,
      _config->fsEqualTolerance);

  _log->debug(p) << _q << BLUE << "Is line and prevLine emphasized by same font?" << OFF << endl;
  _log->debug(p) << _q << " └─ prevLine.isEmphasized: " << isPrevLineEmphasized << endl;
  _log->debug(p) << _q << " └─ currLine.isEmphasized: " << isCurrLineEmphasized << endl;
  _log->debug(p) << _q << " └─ prevLine.fontName: " << line->prevLine->fontName << endl;
  _log->debug(p) << _q << " └─ currLine.fontName: " << line->fontName << endl;
  _log->debug(p) << _q << " └─ prevLine.fontSize: " << line->prevLine->fontSize << endl;
  _log->debug(p) << _q << " └─ currLine.fontSize: " << line->fontSize << endl;

  // The line does *not* start a block if the previous line and the current line are emphasized,
  // and if both lines exhibits the same font and the same font size.
  if (isPrevLineEmphasized && isCurrLineEmphasized && hasEqualFontName && hasEqualFontSize) {
    _log->debug(p) << _q << BLUE << BOLD << " yes → continues block" << OFF << endl;
    return Trool::False;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetection::startsBlock_hangingIndent(const PdfTextBlock* pBlock,
    const PdfTextLine* line) const {
  assert(pBlock);
  assert(line);
  assert(line->prevLine);

  int p = line->pos->pageNum;
  const PdfTextLine* prevLine = line->prevLine;

  double hangingIndent = pBlock->hangingIndent;
  double prevLeftMargin = prevLine->leftMargin;
  double currLeftMargin = line->leftMargin;
  bool isPrevNotIndented = smaller(prevLeftMargin, hangingIndent, _doc->avgCharWidth);
  bool isCurrNotIndented = smaller(currLeftMargin, hangingIndent, _doc->avgCharWidth);
  bool isPrevIndented = equal(prevLeftMargin, hangingIndent, _doc->avgCharWidth);
  bool isCurrIndented = equal(currLeftMargin, hangingIndent, _doc->avgCharWidth);
  bool isPrevMoreIndented = larger(prevLeftMargin, hangingIndent, _doc->avgCharWidth);
  bool isCurrMoreIndented = larger(currLeftMargin, hangingIndent, _doc->avgCharWidth);
  double leftXOffset = computeLeftXOffset(prevLine, line);
  bool hasPrevLineCapacity = _utils->computeHasPrevLineCapacity(line->prevLine, line);
  pair<double, double> leftXOffsetTolInterval = _config->getLeftXOffsetToleranceInterval(_doc);
  double leftXOffsetToleranceLow = leftXOffsetTolInterval.first;
  double leftXOffsetToleranceHigh = leftXOffsetTolInterval.second;

  _log->debug(p) << _q << BLUE << "Is it part of a block in hanging indent format?" << OFF << endl;
  _log->debug(p) << _q << " └─ block.hangingIndent:     " << hangingIndent << endl;
  _log->debug(p) << _q << " └─ prevLine.leftMargin:     " << prevLeftMargin << endl;
  _log->debug(p) << _q << " └─ prevLine.isNotIndented:  " << isPrevNotIndented << endl;
  _log->debug(p) << _q << " └─ prevLine.isIndented:     " << isPrevIndented << endl;
  _log->debug(p) << _q << " └─ prevLine.isMoreIndented: " << isPrevMoreIndented << endl;
  _log->debug(p) << _q << " └─ prevLine.hasCapacity:    " << hasPrevLineCapacity << endl;
  _log->debug(p) << _q << " └─ currLine.leftMargin:     " << currLeftMargin << endl;
  _log->debug(p) << _q << " └─ currLine.isNotIndented:  " << isCurrNotIndented << endl;
  _log->debug(p) << _q << " └─ currLine.isIndented:     " << isCurrIndented << endl;
  _log->debug(p) << _q << " └─ currLine.isMoreIndented: " << isCurrMoreIndented << endl;
  _log->debug(p) << _q << " └─ leftXOffset prevLine/currLine: " << leftXOffset << endl;
  _log->debug(p) << _q << " └─ leftXOffsetToleranceLow: " << leftXOffsetToleranceLow << endl;
  _log->debug(p) << _q << " └─ leftXOffsetToleranceHigh: " << leftXOffsetToleranceHigh << endl;

  // Do nothing if the block is not in hanging indent format.
  if (equalOrSmaller(hangingIndent, 0.0)) {
    return Trool::None;
  }

  // The line starts a block if it is not indented.
  if (isCurrNotIndented) {
    _log->debug(p) << _q << BLUE << BOLD << " yes + is not indented → starts block" << OFF << endl;
    return Trool::True;
  }

  if (isCurrIndented) {
    _log->debug(p) << _q << BLUE << BOLD << " yes + is indented." << OFF << endl;

    if (isPrevMoreIndented) {
      _log->debug(p) << _q << BLUE << BOLD << " + prev line is more indented." << OFF << endl;

      // The line does *not* start a block if it is indented, the prev. line is more indented
      // than the tolerance, and the leftX-offset between both lines is in the given tolerance.
      if (between(leftXOffset, leftXOffsetToleranceLow, leftXOffsetToleranceHigh)) {
        _log->debug(p) << _q << BLUE << BOLD << " + x-offset is in tolerance → continues block"
            << OFF << endl;
        return Trool::False;
      }

      // The line starts a block if it is indented, the prev. line is more indented than the
      // the tolerance, and the leftX-offset between both lines is *not* in the given tolerance.
      _log->debug(p) << _q << BLUE << BOLD << " + x-offset is not in tolerance → starts block"
          << OFF << endl;
      return Trool::True;
    }

    // The line starts a block if it is indented and the prev. line has capacity.
    if (hasPrevLineCapacity) {
      _log->debug(p) << _q << BLUE << BOLD << " + prev line capacity → starts block" << OFF << endl;
      return Trool::True;
    }

    _log->debug(p) << _q << BLUE << BOLD << " + no rule applied → continues block" << OFF << endl;
    return Trool::False;
  }

  if (isCurrMoreIndented) {
    _log->debug(p) << _q << BLUE << BOLD << " yes + line is more indented." << OFF << endl;

    if (isPrevMoreIndented) {
      _log->debug(p) << _q << BLUE << BOLD << " + prev line is more indented." << OFF << endl;

      // The line does *not* start a block if the current and previous line are more indented
      // than the tolerance, and the leftX-offset between both lines is in the given tolerance.
      if (between(leftXOffset, leftXOffsetToleranceLow, leftXOffsetToleranceHigh)) {
        _log->debug(p) << _q << BLUE << BOLD << " + x-offset is in tolerance → continues block"
            << OFF << endl;
        return Trool::False;
      }

      // The line starts a block if the current and previous line are more indented than the
      // tolerance, and the leftX-Offset between both lines is *not* in the tolerance.
      _log->debug(p) << _q << BLUE << BOLD << " + offset not in tolerance → starts block"
          << OFF << endl;
      return Trool::True;
    }

    _log->debug(p) << _q << BLUE << BOLD << " + no rule applied → continues block" << OFF << endl;
    return Trool::True;
  }

  _log->debug(p) << _q << BLUE << BOLD << " yes, no rule applied → continues block" << OFF << endl;
  return Trool::False;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetection::startsBlock_indent(const PdfTextLine* line) const {
  assert(line);
  assert(line->prevLine);

  int p = line->pos->pageNum;
  const PdfTextLine* prevLine = line->prevLine;

  double prevLeftMargin = prevLine->leftMargin;
  double currLeftMargin = line->leftMargin;
  pair<double, double> indentToleranceInterval = _config->getIndentToleranceInterval(_doc);
  double indentTolLow = indentToleranceInterval.first;
  double indentTolHigh = indentToleranceInterval.second;
  bool isPrevIndented = between(prevLeftMargin, indentTolLow, indentTolHigh);
  bool isCurrIndented = between(currLeftMargin, indentTolLow, indentTolHigh);
  bool isPrevMoreIndented = larger(prevLeftMargin, indentTolHigh);
  bool isCurrMoreIndented = larger(currLeftMargin, indentTolHigh);
  double absLeftXOffset = abs(computeLeftXOffset(prevLine, line));
  bool hasPrevLineCapacity = _utils->computeHasPrevLineCapacity(line->prevLine, line);

  _log->debug(p) << _q << BLUE <<  "Is the line indented?" << OFF << endl;
  _log->debug(p) << _q << " └─ prevLine.leftMargin:     " << prevLeftMargin << endl;
  _log->debug(p) << _q << " └─ prevLine.isIndented:     " << isPrevIndented << endl;
  _log->debug(p) << _q << " └─ prevLine.isMoreIndented: " << isPrevMoreIndented << endl;
  _log->debug(p) << _q << " └─ prevLine.hasCapacity:    " << hasPrevLineCapacity << endl;
  _log->debug(p) << _q << " └─ currLine.leftMargin:     " << currLeftMargin << endl;
  _log->debug(p) << _q << " └─ currLine.isIndented:     " << isCurrIndented << endl;
  _log->debug(p) << _q << " └─ currLine.isMoreIndented: " << isCurrMoreIndented << endl;
  _log->debug(p) << _q << " └─ absLeftXOffset prevLine/currLine: " << absLeftXOffset << endl;
  _log->debug(p) << _q << " └─ indentToleranceLow:  " << indentTolLow << endl;
  _log->debug(p) << _q << " └─ indentToleranceHigh: " << indentTolHigh << endl;
  _log->debug(p) << _q << " └─ absLeftXOffset prevLine/currLine: " << absLeftXOffset << endl;

  if (isCurrMoreIndented) {
    _log->debug(p) << _q << BLUE << BOLD <<  " yes, curr line is more indented." << OFF << endl;

    // The line does *not* start a block if it is more indented than the given tolerance and
    // its leftX offset is equal to zero (under consideration of a small threshold).
    if (equal(absLeftXOffset, 0, _doc->avgCharWidth)) {
      _log->debug(p) << _q << BLUE << BOLD << " + leftXOffset ≈ 0 → continues block" << OFF << endl;
      return Trool::False;
    }

    // The line starts a block if the current and previous line are more indented
    // than the tolerance, and the leftX-offset between both lines is *not* equal to 0.
    _log->debug(p) << _q << BLUE << BOLD << " + leftXOffset !≈ 0 → starts block" << OFF << endl;
    return Trool::True;
  }

  if (isPrevMoreIndented) {
    _log->debug(p) << _q << BLUE << BOLD << " unknown, but previous line is more indented." << endl;

    // The line starts a block if the current and previous line are more indented
    // than the tolerance, and the leftX-offset between both lines is equal to 0 (under
    // consideration of a small threshold).
    if (equal(absLeftXOffset, 0, _doc->avgCharWidth)) {
      _log->debug(p) << _q << BLUE << BOLD << " + leftXOffset ≈ 0 → continues block" << OFF << endl;
      return Trool::False;
    }

    // The line starts a block if the current and previous line are more indented
    // than the tolerance, and the leftX-offset between both lines is *not* equal to 0 (under
    // consideration of a small threshold).
    _log->debug(p) << _q << BLUE << BOLD << " + leftXOffset !≈ 0 → starts block" << OFF << endl;
    return Trool::True;
  }

  // The line starts a block if it is indented.
  if (isCurrIndented) {
    _log->debug(p) << _q << BLUE << BOLD << " yes → starts block" << OFF << endl;
    return Trool::True;
  }

  // The line starts a block if the previous line has capacity.
  if (hasPrevLineCapacity) {
    _log->debug(p) << _q << BLUE << BOLD << " prev line has capacity → starts block" << OFF << endl;
    return Trool::True;
  }

  return Trool::None;
}

}  // namespace ppp::modules
