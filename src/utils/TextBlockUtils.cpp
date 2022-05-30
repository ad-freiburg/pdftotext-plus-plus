/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cmath>
#include <iostream>

#include "./MathUtils.h"
#include "./PdfElementUtils.h"
#include "./TextBlockUtils.h"
#include "./Utils.h"

using namespace std;

// _________________________________________________________________________________________________
bool text_block_utils::computeIsCentered(const PdfTextBlock* block) {
  bool hasLineWithLargeMarginNoFormula = false;
  int numLinesNoMargin = 0;

  for (size_t i = 1; i < block->lines.size(); i++) {
    const PdfTextLine* prevLine = block->lines[i - 1];
    const PdfTextLine* currLine = block->lines[i];

    pair<double, double> ratios = element_utils::computeXOverlapRatios(prevLine, currLine);
    double maxXOverlapRatio = max(ratios.first, ratios.second);

    if (math_utils::smaller(maxXOverlapRatio, 1, 0.01)) {
      return false;
    }

    double leftXOffset = abs(prevLine->position->leftX - currLine->position->leftX);
    double rightXOffset = abs(prevLine->position->rightX - currLine->position->rightX);
    bool isEqualOffset = math_utils::equal(leftXOffset, rightXOffset, 2 * currLine->doc->avgGlyphWidth);

    if (!isEqualOffset) {
      return false;
    }

    bool isLargeLeftXOffset = math_utils::larger(leftXOffset, 0, 2 * currLine->doc->avgGlyphWidth);
    bool isLargeRightXOffset = math_utils::larger(rightXOffset, 0, 2 * currLine->doc->avgGlyphWidth);
    bool isLargeXOffset = isLargeLeftXOffset || isLargeRightXOffset;
    bool prevIsFormula = false;
    for (char c : FORMULA_ID_ALPHABET) {
      if (prevLine->text.find(c) != string::npos) {
        prevIsFormula = true;
        break;
      }
    }
    bool currIsFormula = false;
    for (char c : FORMULA_ID_ALPHABET) {
      if (currLine->text.find(c) != string::npos) {
        currIsFormula = true;
        break;
      }
    }

    if (isLargeXOffset && !prevIsFormula && !currIsFormula) {
      hasLineWithLargeMarginNoFormula = true;
    } else {
      numLinesNoMargin++;
    }
  }
  return hasLineWithLargeMarginNoFormula && numLinesNoMargin <= 5;
}

// _________________________________________________________________________________________________
double text_block_utils::computeHangingIndent(const PdfTextBlock* block) {
  // Compute the most frequent left margin > 0.
  int numLines = 0;
  int numLeftMarginLines = 0;
  unordered_map<double, int> leftMarginCounts;
  for (const auto* line : block->lines) {
    if (line->text.size() < 3) { continue; }
    if (math_utils::larger(line->leftMargin, 0, line->doc->avgGlyphWidth)) {
      leftMarginCounts[line->leftMargin]++;
      numLeftMarginLines++;
    }
    numLines++;
  }
  double mostFreqLeftMargin = 0.0;
  int mostFreqLeftMarginCount = 0;
  for (const auto& pair : leftMarginCounts) {
    if (pair.second > mostFreqLeftMarginCount) {
      mostFreqLeftMargin = pair.first;
      mostFreqLeftMarginCount = pair.second;
    }
  }

  // Abort if there are no more than two lines.
  if (numLines <= 1) {
    return 0.0;
  }

  // Abort if there are no lines with leftMargin > 0.
  if (numLeftMarginLines == 0) {
    return 0.0;
  }

  // Abort if less than 50% of the indented lines are indented by the same level.
  if (mostFreqLeftMarginCount <= 0.5 * numLeftMarginLines) {
    return 0.0;
  }

  bool isFirstLineIndented = false;
  bool isFirstLineShort = false;
  bool isAllOtherLinesIndented = true;
  int numLowercasedNotIndentedLines = 0;
  int numLowercasedIndentedLines = 0;
  int numNotIndentedLines = 0;
  int numIndentedLines = 0;

  for (size_t i = 0; i < block->lines.size(); i++) {
    const PdfTextLine* line = block->lines[i];

    if (line->text.size() < 3) { continue; }

    bool isCentered = math_utils::equal(line->leftMargin, line->rightMargin, line->doc->avgGlyphWidth) &&
        math_utils::larger(line->leftMargin, line->doc->avgGlyphWidth, 0);
    bool isNotIndented = math_utils::equal(line->leftMargin, 0, line->doc->avgGlyphWidth);
    bool isIndented = math_utils::equal(line->leftMargin, mostFreqLeftMargin, line->doc->avgGlyphWidth);
    bool isLower = !line->text.empty() && islower(line->text[0]);
    bool startsWithLastNamePrefix = LAST_NAME_PREFIXES.count(line->words[0]->text) > 0;

    if (isCentered) {
      continue;
    }

    if (i == 0) {
      isFirstLineIndented = isIndented;
      isFirstLineShort = math_utils::larger(line->rightMargin, 0, 4 * line->doc->avgGlyphWidth);
    } else {
      isAllOtherLinesIndented &= isIndented;
    }

    if (isLower && !startsWithLastNamePrefix && isNotIndented) {
      numLowercasedNotIndentedLines++;
    }
    if (isLower && isIndented) {
      numLowercasedIndentedLines++;
    }
    if (isIndented) {
      numIndentedLines++;
    }
    if (isNotIndented) {
      numNotIndentedLines++;
    }
  }

  if (numIndentedLines == 0) {
    return 0.0;
  }

  // The block is *not* in hanging indent format if there is at least one non-indented line
  // that starts with a lowercase character.
  if (numLowercasedNotIndentedLines > 0) {
    return 0.0;
  }

  // The block is in hanging indent format if the first line is not indented, but all other
  // lines. This should identify single enumeration items, e.g., in the format:
  // Dynamics: The low energy behavior of
  //    a physical system depends on its
  //    dynamics.
  if (!isFirstLineIndented && !isFirstLineShort && isAllOtherLinesIndented) {
    return mostFreqLeftMargin;
  }

  // The block is in hanging indent format if all non-indented lines start with an uppercase
  // character and if the number of non-indented lines exceed a certain threshold.
  if (numNotIndentedLines >= 10 && numLowercasedNotIndentedLines == 0) {
    return mostFreqLeftMargin;
  }

  // The block is in hanging indent format if there is at least one indented line that start
  // with a lowercase character.
  if (numLines >= 4 && numLowercasedIndentedLines > 0) {
    return mostFreqLeftMargin;
  }

  return 0.0;
}

// _________________________________________________________________________________________________
void text_block_utils::computeTextLineMargins(const PdfTextBlock* block) {
  assert(block);

  // A mapping of left margins to their frequencies, for computing the most freq. left margin.
  unordered_map<double, int> leftMarginFreqs;
  unordered_map<double, int> rightMarginFreqs;

  PdfTextBlock* prevBlock = block->prevBlock;
  PdfTextBlock* nextBlock = block->nextBlock;

  // Enlarge short text blocks that consists of two lines.
  double blockTrimRightX = block->trimRightX;
  if (block->lines.size() == 2) {
    double leftMargin = block->position->leftX - block->segment->position->leftX;
    double rightMargin = block->segment->position->rightX - block->position->rightX;
    double isCentered = math_utils::equal(leftMargin, rightMargin, block->doc->avgGlyphWidth);
    if (!isCentered) {
      if (prevBlock) { blockTrimRightX = max(blockTrimRightX, prevBlock->trimRightX); }
      if (nextBlock) { blockTrimRightX = max(blockTrimRightX, nextBlock->trimRightX); }
    }
  }

  for (size_t j = 0; j < block->lines.size(); j++) {
    PdfTextLine* prevLine = j > 0 ? block->lines.at(j - 1) : nullptr;
    PdfTextLine* currLine = block->lines.at(j);
    PdfTextLine* nextLine = j < block->lines.size() - 1 ? block->lines.at(j + 1) : nullptr;

    // double trimLeftX = max(segment->trimLeftX, block->position->leftX);
    // double trimRightX = min(segment->trimRightX, block->position->rightX);
    // double trimRightX = segment->trimRightX;
    currLine->leftMargin = round(currLine->position->leftX - block->trimLeftX);
    currLine->rightMargin = round(blockTrimRightX - currLine->position->rightX);
  }
}

// _________________________________________________________________________________________________
void text_block_utils::createTextBlock(const vector<PdfTextLine*>& lines,
      vector<PdfTextBlock*>* blocks) {
  // Do nothing if no words are given.
  if (lines.empty()) {
    return;
  }

  PdfTextBlock* block = new PdfTextBlock();
  block->id = createRandomString(8, "tb-");
  block->doc = lines[0]->doc;

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