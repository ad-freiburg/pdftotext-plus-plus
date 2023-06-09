/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <iostream>

#include <algorithm>  // min
#include <string>
#include <unordered_map>
#include <utility>  // pair
#include <vector>

#include "./Counter.h"
#include "./MathUtils.h"
#include "./PdfElementsUtils.h"
#include "./StringUtils.h"
#include "./TextBlocksUtils.h"
#include "./TextLinesUtils.h"

using ppp::math_utils::equal;
using ppp::math_utils::equalOrLarger;
using ppp::math_utils::equalOrSmaller;
using ppp::math_utils::larger;
using ppp::math_utils::round;
using ppp::math_utils::smaller;
using std::max;
using std::min;
using std::pair;
using std::string;
using std::unordered_map;
using std::vector;

// _________________________________________________________________________________________________
bool text_blocks_utils::computeIsTextLinesCentered(const PdfTextBlock* block) {
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
    bool centered = text_lines_utils::computeIsCentered(prevLine, currLine);

    if (!centered) {
      return false;
    }

    // Check if the line or the previous line contains a formula.
    const char* c;
    bool prevLineContainsFormula = false;
    for (c = config::FORMULA_ID_ALPHABET; *c != '\0'; c++) {
      if (prevLine->text.find(*c) != string::npos) {
        prevLineContainsFormula = true;
        break;
      }
    }
    bool currLineContainsFormula = false;
    for (c = config::FORMULA_ID_ALPHABET; *c != '\0'; c++) {
      if (currLine->text.find(*c) != string::npos) {
        currLineContainsFormula = true;
        break;
      }
    }
    bool isFormula = prevLineContainsFormula || currLineContainsFormula;

    // Check if the line has a leftX offset (or rightX offset) larger than the threshold.
    double absLeftXOffset = abs(element_utils::computeLeftXOffset(prevLine, currLine));
    double absRightXOffset = abs(element_utils::computeRightXOffset(prevLine, currLine));
    double xOffsetThreshold = config::getCenteringXOffsetThreshold(currLine->doc);
    bool isLargeLeftXOffset = larger(absLeftXOffset, xOffsetThreshold);
    bool isLargeRightXOffset = larger(absRightXOffset, xOffsetThreshold);
    bool isLargeXOffset = isLargeLeftXOffset || isLargeRightXOffset;

    // Check if the line is not a formula and has a leftX offset (or rightX offset) larger than
    // the threshold. Count the number of justified lines.
    if (!isFormula && isLargeXOffset) {
      hasNonFormulaWithLargeXOffset = true;
    } else {
      numJustifiedLines++;
    }
  }

  return hasNonFormulaWithLargeXOffset
      && numJustifiedLines <= config::CENTERING_MAX_NUM_JUSTIFIED_LINES;
}

// _________________________________________________________________________________________________
double text_blocks_utils::computeHangingIndent(const PdfTextBlock* block) {
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

  double marginThreshold = config::getHangIndentMarginThreshold(block->doc);

  for (const auto* line : block->lines) {
    // Count the number of lines with a length >= the given threshold.
    if (line->text.size() >= config::HANG_INDENT_MIN_LENGTH_LONG_LINES) {
      numLongLines++;
    }

    // Count the number of lines with a left margin >= the given threshold.
    double leftMargin = round(line->leftMargin);
    if (equalOrLarger(leftMargin, marginThreshold)) {
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
  if (equalOrSmaller(mostFreqLargeLeftMarginCount,
        config::HANG_INDENT_MIN_PERC_LINES_SAME_LEFT_MARGIN * numLargeLeftMarginLines)) {
    return 0.0;
  }

  // Count the lines exhibiting features required for the block to be in hanging indent format.
  for (size_t i = 0; i < block->lines.size(); i++) {
    const PdfTextLine* line = block->lines[i];

    // Ignore short lines.
    if (line->text.size() < config::HANG_INDENT_MIN_LENGTH_LONG_LINES) {
      continue;
    }

    // Ignore lines that are centered.
    bool isEqualMargin = equal(line->leftMargin, line->rightMargin, marginThreshold);
    bool isLargeMargin = larger(line->leftMargin, marginThreshold);
    bool isCentered = isEqualMargin && isLargeMargin;
    if (isCentered) {
      continue;
    }

    // Count the number of non-indented lines.
    bool isNonIndented = equal(line->leftMargin, 0, marginThreshold);
    if (isNonIndented) {
      numNonIndentedLines++;
    }

    // Count the number of indented lines.
    bool isIndented = equal(line->leftMargin, mostFreqLargeLeftMargin, marginThreshold);
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
    bool startsWithLastNamePrefix = config::LAST_NAME_PREFIXES.count(line->words[0]->text) > 0;
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
  if (numLowercasedNonIndentedLines > config::HANG_INDENT_NUM_LOWER_NON_INDENTED_LINES_THRESHOLD) {
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
  if (numNonIndentedLines >= config::HANG_INDENT_NUM_NON_INDENTED_LINES_THRESHOLD &&
      numLowercasedNonIndentedLines <= config::HANG_INDENT_NUM_LOWER_NON_INDENTED_LINES_THRESHOLD) {
    return mostFreqLargeLeftMargin;
  }

  // The block is in hanging indent format if there is at least one indented line that start
  // with a lowercase character.
  if (numLongLines >= config::HANG_INDENT_NUM_LONG_LINES_THRESHOLD
        && numLowercasedIndentedLines >= config::HANG_INDENT_NUM_LOWER_INDENTED_LINES_THRESHOLD) {
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
    double leftMargin = block->pos->leftX - block->segment->pos->leftX;
    double rightMargin = block->segment->pos->rightX - block->pos->rightX;
    double isCentered = ppp::math_utils::equal(leftMargin, rightMargin, block->doc->avgCharWidth);
    if (!isCentered) {
      if (prevBlock) { blockTrimRightX = max(blockTrimRightX, prevBlock->trimRightX); }
      if (nextBlock) { blockTrimRightX = max(blockTrimRightX, nextBlock->trimRightX); }
    }
  }

  for (auto* line : block->lines) {
    // TODO(korzen): Should this really be rounded?
    line->leftMargin = ppp::math_utils::round(line->pos->leftX - block->trimLeftX);
    line->rightMargin = ppp::math_utils::round(blockTrimRightX - line->pos->rightX);
  }
}

// _________________________________________________________________________________________________
void text_blocks_utils::createTextBlock(const vector<PdfTextLine*>& lines,
      vector<PdfTextBlock*>* blocks) {
  assert(!lines.empty());
  assert(blocks);

  PdfTextBlock* block = new PdfTextBlock();
  block->id = ppp::string_utils::createRandomString(global_config::ID_LENGTH, "block-");

  // Set the reference to the document.
  block->doc = lines[0]->doc;

  // Set the reference to the parent segment.
  block->segment = lines[0]->segment;

  // Set the lines.
  block->lines = lines;

  // Set the page number.
  block->pos->pageNum = lines[0]->pos->pageNum;

  // Set the writing mode.
  block->pos->wMode = lines[0]->pos->wMode;

  // Set the rotation value.
  block->pos->rotation = lines[0]->pos->rotation;

  // Set the rank.
  block->rank = blocks->size();

  // Compute the bounding box and the trim box and count the different font names and font sizes.
  StringCounter fontNameCounter;
  DoubleCounter fontSizeCounter;
  for (size_t i = 0; i < lines.size(); i++) {
    PdfTextLine* prevLine = i > 0 ? lines[i-1] : nullptr;
    PdfTextLine* currLine = lines[i];
    PdfTextLine* nextLine = i < lines.size() - 1 ? lines[i+1] : nullptr;

    double lineMinX = min(currLine->pos->leftX, currLine->pos->rightX);
    double lineMinY = min(currLine->pos->upperY, currLine->pos->lowerY);
    double lineMaxX = max(currLine->pos->leftX, currLine->pos->rightX);
    double lineMaxY = max(currLine->pos->upperY, currLine->pos->lowerY);

    // Compute the bounding box.
    block->pos->leftX = min(block->pos->leftX, lineMinX);
    block->pos->upperY = min(block->pos->upperY, lineMinY);
    block->pos->rightX = max(block->pos->rightX, lineMaxX);
    block->pos->lowerY = max(block->pos->lowerY, lineMaxY);

    // Compute the trim box.
    block->trimLeftX = max(block->pos->leftX, block->segment->trimLeftX);
    block->trimUpperY = max(block->pos->upperY, block->segment->trimUpperY);
    block->trimRightX = min(block->pos->rightX, block->segment->trimRightX);
    block->trimLowerY = min(block->pos->lowerY, block->segment->trimLowerY);

    // Count the font names and font sizes, for computing the most frequent font name / font size.
    fontNameCounter[currLine->fontName]++;
    fontSizeCounter[currLine->fontSize]++;

    // TODO(korzen): prevLine and nextLine should be computed document-wide, not block-wide.
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
