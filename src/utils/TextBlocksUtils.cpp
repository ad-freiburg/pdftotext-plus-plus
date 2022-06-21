/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // min
#include <string>
#include <unordered_map>
#include <vector>

#include "./Counter.h"
#include "./MathUtils.h"
#include "./PdfElementsUtils.h"
#include "./StringUtils.h"
#include "./TextBlocksUtils.h"
#include "./TextLinesUtils.h"

using std::max;
using std::min;
using std::pair;
using std::string;
using std::unordered_map;
using std::vector;
using text_blocks_utils::config::FORMULA_ID_ALPHABET;
using text_blocks_utils::config::LAST_NAME_PREFIXES;

// _________________________________________________________________________________________________
bool text_blocks_utils::computeIsTextLinesCentered(const PdfTextBlock* block,
      double xOffsetEqualToleranceFactor, double maxNumJustifiedLines) {
  assert(block);

  // The lines in the block are not obviously not centered if the block does not contain any lines.
  if (block->lines.empty()) {
    return false;
  }

  // A boolean indicating whether or not the block contains a line (not representing a display
  // formula) with a leftX offset (resp. rightX offset) larger than a threshold.
  bool hasNonFormulaWithLargeXOffset = false;
  // The number of justified lines (that is: lines with leftX offset == rightX offset == 0).
  int numJustifiedLines = 0;

  for (size_t i = 1; i < block->lines.size(); i++) {
    const PdfTextLine* prevLine = block->lines[i - 1];
    const PdfTextLine* currLine = block->lines[i];

    // The lines in the block are not centered when there is at least one line which is not
    // centered compared to the previous line.
    bool centered = text_lines_utils::computeIsCentered(prevLine, currLine,
        xOffsetEqualToleranceFactor);
    if (!centered) {
      return false;
    }

    // Check if the line or the previous line contains a formula.
    const char* c;
    bool prevLineContainsFormula = false;
    for (c = FORMULA_ID_ALPHABET; *c != '\0'; c++) {
      if (prevLine->text.find(*c) != string::npos) {
        prevLineContainsFormula = true;
        break;
      }
    }
    bool currLineContainsFormula = false;
    for (c = FORMULA_ID_ALPHABET; *c != '\0'; c++) {
      if (currLine->text.find(*c) != string::npos) {
        currLineContainsFormula = true;
        break;
      }
    }
    bool isFormula = prevLineContainsFormula || currLineContainsFormula;

    // Check if the line has a leftX offset (or rightX offset) larger than the threshold.
    double avgCharWidth = currLine->doc->avgCharWidth;
    double xOffsetEqualTolerance = xOffsetEqualToleranceFactor * avgCharWidth;
    double absLeftXOffset = abs(element_utils::computeLeftXOffset(prevLine, currLine));
    double absRightXOffset = abs(element_utils::computeRightXOffset(prevLine, currLine));
    bool isLargeLeftXOffset = math_utils::larger(absLeftXOffset, xOffsetEqualTolerance);
    bool isLargeRightXOffset = math_utils::larger(absRightXOffset, xOffsetEqualTolerance);
    bool isLargeXOffset = isLargeLeftXOffset || isLargeRightXOffset;

    // Check if the line is not a formula and has a leftX offset (or rightX offset) larger than
    // the threshold. Count the number of justified lines.
    if (!isFormula && isLargeXOffset) {
      hasNonFormulaWithLargeXOffset = true;
    } else {
      numJustifiedLines++;
    }
  }

  return hasNonFormulaWithLargeXOffset && numJustifiedLines <= maxNumJustifiedLines;
}

// _________________________________________________________________________________________________
double text_blocks_utils::computeHangingIndent(const PdfTextBlock* block,
    double minLengthLongLines, double marginThresholdFactor, double minPercLinesSameLeftMargin,
    double maxNumLowercasedNonIndentedLines, double minNumNonIndentedLines,
    double minNumLowercasedIndentedLines, double minNumLongLines) {
  assert(block);

  // The number of lines with a length larger than the threshold.
  int numLongLines = 0;
  // The number of lines with a left margin larger than the threshold.
  int numLargeLeftMarginLines = 0;
  // The frequencies of the different left margins which are larger than the threshold.
  DoubleCounter largeLeftMarginCounter;
  // The most frequent left margin among the lines with a left margin larger than the threshold.
  double mostFreqLargeLeftMargin = 0.0;
  // The number of lines exhibiting the most frequent left margin.
  int mostFreqLargeLeftMarginCount = 0;
  // A boolean indicating if the first line is indented (= if its left margin is equal to the most
  // frequent left margin).
  bool isFirstLineIndented = false;
  // A boolean indicating if the first line has capacity.
  bool hasFirstLineCapacity = false;
  // A boolean indicating if all lines except the first are indented.
  bool isAllOtherLinesIndented = true;
  // The number of non-indented lines that start with a lowercase character.
  int numLowercasedNonIndentedLines = 0;
  // The number of indented lines that start with a lowercase character.
  int numLowercasedIndentedLines = 0;
  // The number of non-indented lines.
  int numNonIndentedLines = 0;
  // The number of indented lines.
  int numIndentedLines = 0;

  for (const auto* line : block->lines) {
    // Count the number of lines with a length >= the given threshold.
    if (line->text.size() >= minLengthLongLines) {
      numLongLines++;
    }

    // Count the number of lines with a left margin >= the given threshold.
    double leftMargin = math_utils::round(line->leftMargin);
    double avgCharWidth = line->doc->avgCharWidth;
    double leftMarginThreshold = marginThresholdFactor * avgCharWidth;
    if (math_utils::equalOrLarger(leftMargin, leftMarginThreshold)) {
      largeLeftMarginCounter[leftMargin]++;
      numLargeLeftMarginLines++;
    }
  }

  // Compute the most freq left margin among the lines with a left margin larger than the threshold.
  pair<double, double> mostFreqLargeLeftMarginPair = largeLeftMarginCounter.mostFreqAndCount();
  mostFreqLargeLeftMargin = mostFreqLargeLeftMarginPair.first;
  mostFreqLargeLeftMarginCount = mostFreqLargeLeftMarginPair.second;

  // The block is *not* in hanging indent format if the percentage of lines exhibiting the
  // most frequent left margin is smaller than a threshold.
  double numLargeLeftMarginTresh = minPercLinesSameLeftMargin * numLargeLeftMarginLines;
  if (math_utils::equalOrSmaller(mostFreqLargeLeftMarginCount, numLargeLeftMarginTresh)) {
    return 0.0;
  }

  // Count the lines exhibiting features required for the block to be in hanging indent format.
  for (size_t i = 0; i < block->lines.size(); i++) {
    const PdfTextLine* line = block->lines[i];

    // Ignore short lines.
    if (line->text.size() < minLengthLongLines) {
      continue;
    }

    // Ignore lines that are centered.
    double avgCharWidth = line->doc->avgCharWidth;
    double marginThreshold = marginThresholdFactor * avgCharWidth;
    bool isEqualMargin = math_utils::equal(line->leftMargin, line->rightMargin, marginThreshold);
    bool isLargeMargin = math_utils::larger(line->leftMargin, marginThreshold);
    bool isCentered = isEqualMargin && isLargeMargin;
    if (isCentered) {
      continue;
    }

    // Count the number of non-indented lines.
    bool isNonIndented = math_utils::equal(line->leftMargin, 0, marginThreshold);
    if (isNonIndented) {
      numNonIndentedLines++;
    }

    // Count the number of indented lines.
    bool isIndented = math_utils::equal(line->leftMargin, mostFreqLargeLeftMargin, marginThreshold);
    if (isIndented) {
      numIndentedLines++;
    }

    // Count the number of indented lines that start with a lowercase.
    bool isLower = islower(line->text[0]);
    if (isLower && isIndented) {
      numLowercasedIndentedLines++;
    }

    // Count the number of non-indented lines that start with a lowercase and do not start with
    // a lowercase last name prefix.
    bool startsWithLastNamePrefix = LAST_NAME_PREFIXES.count(line->words[0]->text) > 0;
    if (isLower && !startsWithLastNamePrefix && isNonIndented) {
      numLowercasedNonIndentedLines++;
    }

    // Check if the first line is indented.
    // Check if the first line has capacity.
    // Check if all lines except the first are indented.
    if (i == 0) {
      isFirstLineIndented = isIndented;
    }
    if (i == 1) {
      hasFirstLineCapacity = text_lines_utils::computeHasPrevLineCapacity(line);
    }
    if (i > 0) {
      isAllOtherLinesIndented &= isIndented;
    }
  }

  // The block is *not* in hanging indent format if it does not contain any indented lines.
  if (numIndentedLines == 0) {
    return 0.0;
  }

  // The block is *not* in hanging indent format if it contains at least one non-indented line
  // that starts with a lowercase character.
  if (numLowercasedNonIndentedLines > maxNumLowercasedNonIndentedLines) {
    return 0.0;
  }

  // The block is in hanging indent format if the first line is not indented, but all other
  // lines. This should identify single enumeration items, e.g., in the format:
  // Dynamics: The low energy behavior of
  //    a physical system depends on its
  //    dynamics.
  if (!isFirstLineIndented && !hasFirstLineCapacity && isAllOtherLinesIndented) {
    return mostFreqLargeLeftMargin;
  }

  // The block is in hanging indent format if all non-indented lines start with an uppercase
  // character and if the number of non-indented lines exceed a certain threshold.
  if (numNonIndentedLines >= minNumNonIndentedLines
        && numLowercasedNonIndentedLines <= maxNumLowercasedNonIndentedLines) {
    return mostFreqLargeLeftMargin;
  }

  // The block is in hanging indent format if there is at least one indented line that start
  // with a lowercase character.
  if (numLongLines >= minNumLongLines
        && numLowercasedIndentedLines >= minNumLowercasedIndentedLines) {
    return mostFreqLargeLeftMargin;
  }

  return 0.0;
}

// _________________________________________________________________________________________________
void text_blocks_utils::computeTextLineMargins(const PdfTextBlock* block) {
  assert(block);

  const PdfTextBlock* prevBlock = block->prevBlock;
  const PdfTextBlock* nextBlock = block->nextBlock;

  // Enlarge text blocks consisting of short lines.
  // TODO(korzen): What does this mean?
  double blockTrimRightX = block->trimRightX;
  if (block->lines.size() == 2) {
    double leftMargin = block->position->leftX - block->segment->position->leftX;
    double rightMargin = block->segment->position->rightX - block->position->rightX;
    double isCentered = math_utils::equal(leftMargin, rightMargin, block->doc->avgCharWidth);
    if (!isCentered) {
      if (prevBlock) { blockTrimRightX = max(blockTrimRightX, prevBlock->trimRightX); }
      if (nextBlock) { blockTrimRightX = max(blockTrimRightX, nextBlock->trimRightX); }
    }
  }

  for (auto* line : block->lines) {
    line->leftMargin = math_utils::round(line->position->leftX - block->trimLeftX);
    line->rightMargin = math_utils::round(blockTrimRightX - line->position->rightX);
  }
}

// _________________________________________________________________________________________________
void text_blocks_utils::createTextBlock(const vector<PdfTextLine*>& lines,
      vector<PdfTextBlock*>* blocks) {
  assert(!lines.empty());
  assert(blocks);

  PdfTextBlock* block = new PdfTextBlock();
  block->id = string_utils::createRandomString(global_config::IDS_LENGTH, "block-");

  // Set the reference to the document.
  block->doc = lines[0]->doc;

  // Set the reference to the parent segment.
  block->segment = lines[0]->segment;

  // Set the lines.
  block->lines = lines;

  // Set the page number.
  block->position->pageNum = lines[0]->position->pageNum;

  // Set the writing mode.
  block->position->wMode = lines[0]->position->wMode;

  // Set the rotation value.
  block->position->rotation = lines[0]->position->rotation;

  // Set the rank.
  block->rank = blocks->size();

  // Compute the bounding box and the trim box and count the different font names and font sizes.
  StringCounter fontNameCounter;
  DoubleCounter fontSizeCounter;
  for (size_t i = 0; i < lines.size(); i++) {
    PdfTextLine* prevLine = i > 0 ? lines[i-1] : nullptr;
    PdfTextLine* currLine = lines[i];
    PdfTextLine* nextLine = i < lines.size() - 1 ? lines[i+1] : nullptr;

    double lineMinX = min(currLine->position->leftX, currLine->position->rightX);
    double lineMinY = min(currLine->position->upperY, currLine->position->lowerY);
    double lineMaxX = max(currLine->position->leftX, currLine->position->rightX);
    double lineMaxY = max(currLine->position->upperY, currLine->position->lowerY);

    // Compute the bounding box.
    block->position->leftX = min(block->position->leftX, lineMinX);
    block->position->upperY = min(block->position->upperY, lineMinY);
    block->position->rightX = max(block->position->rightX, lineMaxX);
    block->position->lowerY = max(block->position->lowerY, lineMaxY);

    // Compute the trim box.
    block->trimLeftX = max(block->position->leftX, block->segment->trimLeftX);
    block->trimUpperY = max(block->position->upperY, block->segment->trimUpperY);
    block->trimRightX = min(block->position->rightX, block->segment->trimRightX);
    block->trimLowerY = min(block->position->lowerY, block->segment->trimLowerY);

    // Count the font names and font sizes, for computing the most frequent font name / font size.
    fontNameCounter[currLine->fontName]++;
    fontSizeCounter[currLine->fontSize]++;

    currLine->prevLine = prevLine;
    currLine->nextLine = nextLine;

    currLine->block = block;
  }

  // Compute and set the most frequent font name and -size.
  block->fontName = fontNameCounter.mostFreq();
  block->fontSize = fontSizeCounter.mostFreq();

  // Compute and set the text.
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
  }

  // Set the references to the previous and next text blocks.
  if (blocks->size() > 0) {
    PdfTextBlock* prevBlock = blocks->at(blocks->size() - 1);
    prevBlock->nextBlock = block;
    block->prevBlock = prevBlock;
  }

  // Compute and set the flag indicating whether or not the block is emphasized.
  block->isEmphasized = text_element_utils::computeIsEmphasized(block);

  // Compute and set the flag indicating whether or not the text lines in the block are centered.
  block->isLinesCentered = text_blocks_utils::computeIsTextLinesCentered(block);

  // Compute the margins of the text lines in the block.
  text_blocks_utils::computeTextLineMargins(block);

  // Compute and set the hanging indent.
  block->hangingIndent = text_blocks_utils::computeHangingIndent(block);

  blocks->push_back(block);
}
