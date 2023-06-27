/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // std::max, std::min
#include <regex>
#include <string>
#include <unordered_set>
#include <utility>  // pair
#include <vector>

#include "./Counter.h"
#include "./MathUtils.h"
#include "./PdfElementsUtils.h"
#include "./StringUtils.h"
#include "./TextBlocksDetectionUtils.h"

using std::max;
using std::min;
using std::pair;
using std::smatch;
using std::string;
using std::unordered_set;
using std::vector;

using ppp::utils::elements::computeHasEqualFont;
using ppp::utils::elements::computeHasEqualFontSize;
using ppp::utils::elements::computeHasEqualLeftX;
using ppp::utils::elements::computeLeftXOffset;
using ppp::utils::elements::computeMaxXOverlapRatio;
using ppp::utils::elements::computeRightXOffset;
using ppp::utils::elements::computeVerticalGap;
using ppp::utils::elements::computeXOverlapRatios;
using ppp::utils::elements::computeYOverlapRatios;
using ppp::utils::math::equal;
using ppp::utils::math::equalOrLarger;
using ppp::utils::math::equalOrSmaller;
using ppp::utils::math::larger;
using ppp::utils::math::round;
using ppp::utils::math::smaller;
using ppp::utils::text::createRandomString;

// =================================================================================================

namespace ppp::utils {

// _________________________________________________________________________________________________
TextBlocksDetectionUtils::TextBlocksDetectionUtils(const TextBlocksDetectionConfig& config) {
  _config = config;
}

// _________________________________________________________________________________________________
TextBlocksDetectionUtils::~TextBlocksDetectionUtils() = default;

// _________________________________________________________________________________________________
bool TextBlocksDetectionUtils::computeIsCentered(const PdfTextLine* l1, const PdfTextLine* l2) {
  assert(l1);
  assert(l2);

  // The lines are not centered when the maximum x-overlap ratio between the lines is smaller than
  // the threshold.
  double maxXOverlapRatio = computeMaxXOverlapRatio(l1, l2);
  if (smaller(maxXOverlapRatio, _config.centeringXOverlapRatioThreshold)) {
    return false;
  }

  // The lines are not centered when the leftX-offset and the rightX-offset between the lines
  // are not equal.
  double absLeftXOffset = abs(computeLeftXOffset(l1, l2));
  double absRightXOffset = abs(computeRightXOffset(l1, l2));
  // TODO(korzen): Move the following computation into config?
  double xOffsetTolerance = _config.centeringXOffsetEqualToleranceFactor * l1->doc->avgCharWidth;
  if (!equal(absLeftXOffset, absRightXOffset, xOffsetTolerance)) {
    return false;
  }

  return true;
}

// _________________________________________________________________________________________________
bool TextBlocksDetectionUtils::computeIsTextLinesCentered(const PdfTextBlock* block) {
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
    bool centered = computeIsCentered(prevLine, currLine);

    if (!centered) {
      return false;
    }

    // Check if the line or the previous line contains a formula.
    bool prevLineContainsFormula = false;
    for (size_t i = 0; i < _config.formulaIdAlphabet.size(); i++) {
      if (prevLine->text.find(_config.formulaIdAlphabet[i]) != string::npos) {
        prevLineContainsFormula = true;
        break;
      }
    }
    bool currLineContainsFormula = false;
    for (size_t i = 0; i < _config.formulaIdAlphabet.size(); i++) {
      if (currLine->text.find(_config.formulaIdAlphabet[i]) != string::npos) {
        currLineContainsFormula = true;
        break;
      }
    }
    bool isFormula = prevLineContainsFormula || currLineContainsFormula;

    // Check if the line has a leftX offset (or rightX offset) larger than the threshold.
    double absLeftXOffset = abs(computeLeftXOffset(prevLine, currLine));
    double absRightXOffset = abs(computeRightXOffset(prevLine, currLine));
    // TODO(korzen): Move the following computation into config?
    double xOffsetThreshold =
        _config.centeringXOffsetEqualToleranceFactor * currLine->doc->avgCharWidth;
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
      && numJustifiedLines <= _config.centeringMaxNumJustifiedLines;
}

// _________________________________________________________________________________________________
void TextBlocksDetectionUtils::computePotentialFootnoteLabels(const PdfTextLine* line,
      unordered_set<string>* result) {
  assert(line);
  assert(result);

  // Iterate through the characters of the word. For each character, check if it is a label that
  // potentially reference a footnote, that is: if it is a superscipted alphanumerical or if it
  // occurs in our alphabet we defined to identify special footnote labels. Merge each consecutive
  // character that is part of such a label and that are positioned behind the word (we don't want
  // to consider labels that are positioned in front of a word, since footnote labels are usually
  // positioned behind words).
  // TODO(korzen): We do not store the info about whether a superscript is positioned before or
  // after a word. As a workaround, consider a superscript as part of a potential footnote marker
  // only when a non-subscript and non-superscript was already seen.
  for (const auto* word : line->words) {
    string label;
    bool nonSubSuperscriptSeen = false;
    for (const auto* ch : word->characters) {
      // Ignore sub- and superscripts that are positioned before the word.
      if (!nonSubSuperscriptSeen && !ch->isSubscript && !ch->isSuperscript) {
        nonSubSuperscriptSeen = true;
        continue;
      }
      // Ignore the character when no subscript and superscript was seen yet.
      if (!nonSubSuperscriptSeen) {
        continue;
      }
      // Ignore the character when it does not contain any text.
      if (ch->text.empty()) {
        continue;
      }

      // The character is part of a potential footnote label when it occurs in our alphabet we
      // defined to identify special (= non-alphanumerical) footnote labels.
      bool isLabel = _config.specialFootnoteLabelsAlphabet.find(ch->text[0]) != std::string::npos;

      // The character is also a potential footnote label when it is a superscripted alphanumerical.
      if (ch->isSuperscript && isalnum(ch->text[0])) {
        isLabel = true;
      }

      // When the character is part of a potential footnote label, add it to the current label
      // string.
      if (isLabel) {
        label += ch->text;
        continue;
      }

      // Otherwise the end of a potential label is reached. When the current label string is not
      // empty, append it to the result vector.
      if (!label.empty()) {
        result->insert(label);
        label.clear();
      }
    }

    // Don't forget to add the last label string to the result vector (if it is not empty).
    if (!label.empty()) {
      result->insert(label);
    }
  }
}

// _________________________________________________________________________________________________
bool TextBlocksDetectionUtils::computeIsEmphasized(const PdfTextElement* element) {
  assert(element);
  assert(element->doc);

  if (element->doc->fontInfos.count(element->doc->mostFreqFontName) == 0) {
    return false;
  }

  if (element->doc->fontInfos.count(element->fontName) == 0) {
    return false;
  }

  const PdfFontInfo* docFontInfo = element->doc->fontInfos.at(element->doc->mostFreqFontName);
  const PdfFontInfo* elemFontInfo = element->doc->fontInfos.at(element->fontName);
  double mostFreqFontSize = element->doc->mostFreqFontSize;

  // The element is emphasized if...

  // ... its font size is larger than the most frequent font size in the document.
  if (larger(element->fontSize, mostFreqFontSize, _config.fsEqualTolerance)) {
    return true;
  }

  // ... its font weight is larger than the most frequent font weight (and its font size is not
  // smaller than the most frequent font size).
  if (equalOrLarger(element->fontSize, mostFreqFontSize, _config.fsEqualTolerance)
      && larger(elemFontInfo->weight, docFontInfo->weight, _config.fontWeightEqualTolerance)) {
    return true;
  }

  // ... it is printed in italics (and its font size is not smaller than the most freq font size).
  if (equalOrLarger(element->fontSize, mostFreqFontSize, _config.fsEqualTolerance)
      && elemFontInfo->isItalic) {
    return true;
  }

  // ... it contains at least one alphabetic character and all alphabetic characters are in
  // uppercase.
  bool containsAlpha = false;
  bool isAllAlphaUpper = true;
  for (char c : element->text) {
    if (isalpha(c)) {
      containsAlpha = true;
      if (islower(c)) {
        isAllAlphaUpper = false;
        break;
      }
    }
  }

  return containsAlpha && isAllAlphaUpper;
}

// _________________________________________________________________________________________________
bool TextBlocksDetectionUtils::computeHasPrevLineCapacity(const PdfTextLine* line) {
  assert(line);

  // The previous line has of course no capacity if there is no previous line.
  if (!line->prevLine) {
    return false;
  }

  // The previous line has no capacity if the given line does not contain any words.
  if (line->words.empty()) {
    return false;
  }

  // Compute the width of the first word of the given line.
  double firstWordWidth = line->words[0]->pos->getWidth();

  // The previous line has capacity if its right margin is larger than the width of the first word
  // of the given line, under consideration of the threshold.
  // TODO(korzen): Move this computation into config?
  double threshold = _config.prevTextLineCapacityThresholdFactor * line->doc->avgCharWidth;
  return larger(line->prevLine->rightMargin, firstWordWidth, threshold);
}

// _________________________________________________________________________________________________
double TextBlocksDetectionUtils::computeHangingIndent(const PdfTextBlock* block) {
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

  // TODO(korzen): Move the computation into config?
  double marginThreshold = _config.hangIndentMarginThresholdFactor * block->doc->avgCharWidth;

  for (const auto* line : block->lines) {
    // Count the number of lines with a length >= the given threshold.
    if (line->text.size() >= _config.hangIndentMinLengthLongLines) {
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
        _config.hangIndentMinPercLinesSameLeftMargin * numLargeLeftMarginLines)) {
    return 0.0;
  }

  // Count the lines exhibiting features required for the block to be in hanging indent format.
  for (size_t i = 0; i < block->lines.size(); i++) {
    const PdfTextLine* line = block->lines[i];

    // Ignore short lines.
    if (line->text.size() < _config.hangIndentMinLengthLongLines) {
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
    bool startsWithLastNamePrefix = _config.lastNamePrefixes.count(line->words[0]->text) > 0;
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
      hasFirstLineCapacity = computeHasPrevLineCapacity(line);
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
  if (numLowercasedNonIndentedLines > _config.hangIndentNumLowerNonIndentedLinesThreshold) {
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
  if (numNonIndentedLines >= _config.hangIndentNumNonIndentedLinesThreshold &&
      numLowercasedNonIndentedLines <= _config.hangIndentNumLowerNonIndentedLinesThreshold) {
    return mostFreqLargeLeftMargin;
  }

  // The block is in hanging indent format if there is at least one indented line that start
  // with a lowercase character.
  if (numLongLines >= _config.hangIndentNumLongLinesThreshold
        && numLowercasedIndentedLines >= _config.hangIndentNumLowerIndentedLinesThreshold) {
    return mostFreqLargeLeftMargin;
  }

  return 0.0;
}

// _________________________________________________________________________________________________
void TextBlocksDetectionUtils::computeTextLineMargins(const PdfTextBlock* block) {
  assert(block);

  const PdfTextBlock* prevBlock = block->prevBlock;
  const PdfTextBlock* nextBlock = block->nextBlock;

  // Enlarge text blocks consisting of short lines.
  // TODO(korzen): What does this mean?
  double blockTrimRightX = block->trimRightX;
  if (block->lines.size() == 2) {
    double leftMargin = block->pos->leftX - block->segment->pos->leftX;
    double rightMargin = block->segment->pos->rightX - block->pos->rightX;
    double isCentered = equal(leftMargin, rightMargin, block->doc->avgCharWidth);
    if (!isCentered) {
      if (prevBlock) { blockTrimRightX = max(blockTrimRightX, prevBlock->trimRightX); }
      if (nextBlock) { blockTrimRightX = max(blockTrimRightX, nextBlock->trimRightX); }
    }
  }

  for (auto* line : block->lines) {
    // TODO(korzen): Should this really be rounded?
    line->leftMargin = round(line->pos->leftX - block->trimLeftX);
    line->rightMargin = round(blockTrimRightX - line->pos->rightX);
  }
}

// _________________________________________________________________________________________________
PdfFigure* TextBlocksDetectionUtils::computeOverlapsFigure(const PdfElement* element,
    const vector<PdfFigure*>& figures) {
  assert(element);

  for (auto* figure : figures) {
    pair<double, double> xOverlapRatios = computeXOverlapRatios(element, figure);
    pair<double, double> yOverlapRatios = computeYOverlapRatios(element, figure);

    // Check if the figure overlaps the element by the required overlap ratios.
    if (equalOrLarger(xOverlapRatios.first, _config.figureXOverlapThreshold)
        && equalOrLarger(yOverlapRatios.first, _config.figureYOverlapThreshold)) {
      return figure;
    }
  }

  return nullptr;
}

// _________________________________________________________________________________________________
void TextBlocksDetectionUtils::createTextBlock(const vector<PdfTextLine*>& lines,
    vector<PdfTextBlock*>* blocks) {
  assert(!lines.empty());
  assert(blocks);

  PdfTextBlock* block = new PdfTextBlock();
  block->id = createRandomString(_config.idLength, "block-");

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
    // TODO(korzen): block->segment->trimLeftX was computed by page_segment_utils::computeTrimBox()
    // which does not exist anymore.
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
  block->isEmphasized = computeIsEmphasized(block);

  // Compute and set the flag indicating whether or not the text lines in the block are centered.
  block->isLinesCentered = computeIsTextLinesCentered(block);

  // Compute the margins of the text lines in the block.
  computeTextLineMargins(block);

  // Compute and set the hanging indent.
  block->hangingIndent = computeHangingIndent(block);

  blocks->push_back(block);
}

// _________________________________________________________________________________________________
bool TextBlocksDetectionUtils::computeIsFirstLineOfItem(const PdfTextLine* line,
      const unordered_set<string>* potentialFootnoteLabels) {
  assert(line);

  // The line is not the first line of an item if it does not contain any words.
  if (line->words.empty()) {
    return false;
  }

  // The line is not the first line of an item if it is not prefixed by an item label.
  bool isPrefixedByItemLabel = computeIsPrefixedByItemLabel(line);
  bool isPrefixedByFootnoteLabel = computeIsPrefixedByFootnoteLabel(line, potentialFootnoteLabels);
  if (!isPrefixedByItemLabel && !isPrefixedByFootnoteLabel) {
    return false;
  }

  double avgCharWidth = line->doc->avgCharWidth;

  // EXPERIMENTAL: The line is not the first line of a footnote when all of the following
  // requirements are fulfilled:
  // (1) the previous line is not prefixed by an item label;
  // (2) the previous line and the current line have the same font;
  // (3) the previous line and the current line have the same font size;
  // (4) the distance between the previous and current line is <= 0;
  // (5) the previous line does not end with a sentence delimiter;
  // (6) the previous and current line have the same leftX.
  // This should avoid to detect lines that occasionally start with a footnote label, but that are
  // actually not part of a footnote, as a footnote. Example: 0901.4737, page 11 ("25Mg and 26Mg..")
  if (line->prevLine) {
    bool isPrevPrefixedByLabel = computeIsPrefixedByItemLabel(line->prevLine);
    bool hasEqualFont = computeHasEqualFont(line->prevLine, line);
    bool hasEqualFontSize = computeHasEqualFontSize(line->prevLine, line, _config.fsEqualTolerance);
    double distance = computeVerticalGap(line->prevLine, line);
    bool hasNegativeDistance = equalOrSmaller(distance, 0);
    bool hasSentenceDelim = computeEndsWithSentenceDelimiter(line->prevLine);
    bool hasEqualLeftX = computeHasEqualLeftX(line->prevLine, line, avgCharWidth);

    if (!isPrevPrefixedByLabel && hasEqualFont && hasEqualFontSize && hasNegativeDistance
          && !hasSentenceDelim && hasEqualLeftX) {
      return false;
    }
  }

  // Check if there is a previous sibling line. The current line is the first line of an item
  // if the previous sibling line is also prefixed by an item label and if it exhibits the same
  // font and font size as the given line.
  const PdfTextLine* prevSibling = line->prevSiblingLine;
  if (prevSibling && !prevSibling->words.empty()) {
    PdfWord* firstWord = line->words[0];
    PdfWord* prevFirstWord = prevSibling->words[0];
    bool prevIsPrefixedByItemLabel = computeIsPrefixedByItemLabel(prevSibling);
    bool hasEqualFont = computeHasEqualFont(prevFirstWord, firstWord);
    bool hasEqualFs = computeHasEqualFontSize(prevFirstWord, firstWord, _config.fsEqualTolerance);
    if (prevIsPrefixedByItemLabel && hasEqualFont && hasEqualFs) {
      return true;
    }
  }

  // Check if there is a next sibling line. The current line is the first line of an item if the
  // next sibling line is also prefixed by an item label and if it exhibits the same font and font
  // size as the current line.
  const PdfTextLine* nextSibling = line->nextSiblingLine;
  if (nextSibling && !nextSibling->words.empty()) {
    PdfWord* firstWord = line->words[0];
    PdfWord* nextFirstWord = nextSibling->words[0];
    bool nextIsPrefixedByItemLabel = computeIsPrefixedByItemLabel(nextSibling);
    bool hasEqualFont = computeHasEqualFont(nextFirstWord, firstWord);
    bool hasEqualFs = computeHasEqualFontSize(nextFirstWord, firstWord, _config.fsEqualTolerance);
    if (nextIsPrefixedByItemLabel && hasEqualFont && hasEqualFs) {
      return true;
    }
  }

  // The line is the first line of an item if it starts with a footnote label.
  if (isPrefixedByFootnoteLabel) {
    return true;
  }

  return false;
}

// _________________________________________________________________________________________________
bool TextBlocksDetectionUtils::computeIsContinuationOfItem(
      const PdfTextLine* line,
      const unordered_set<string>* potentialFootnoteLabels) {
  assert(line);

  // The line is not a continuation of an item (a footnote) if it does not have a parent line.
  const PdfTextLine* parentLine = line->parentLine;
  if (!parentLine) {
    return false;
  }

  // The line is a continuation of an item (a footnote) if the parent line is the first line or a
  // continuation of an item (a footnote).
  return computeIsFirstLineOfItem(parentLine, potentialFootnoteLabels) ||
         computeIsContinuationOfItem(parentLine, potentialFootnoteLabels);
}

// _________________________________________________________________________________________________
bool TextBlocksDetectionUtils::computeIsPrefixedByItemLabel(const PdfTextLine* line) {
  assert(line);

  // The line is not prefixed by an enumeration item label if it does not contain any words.
  const vector<PdfWord*>& words = line->words;
  if (words.empty()) {
    return false;
  }

  // The line is not prefixed by an enumeration item label if the first word is empty.
  const vector<PdfCharacter*>& firstWordChars = words[0]->characters;
  if (firstWordChars.empty()) {
    return false;
  }

  // The line is prefixed by an enumeration item label if the first char is superscripted and if
  // it is contained in our alphabet we defined for identifying superscripted item labels.
  // TODO(korzen): Instead of analyzing only the first char, we should analyze the first *word*.
  // This would identify also lines that are prefixed by something like "a)".
  PdfCharacter* ch = firstWordChars[0];
  string charStr = ch->text;
  if (ch->isSuperscript && strstr(_config.superItemLabelAlphabet, charStr.c_str()) != nullptr) {
    return true;
  }

  // The line is prefixed by an enumeration item label if it matches one of our regexes we defined
  // for identifying item labels. The matching parts must not be superscripted.
  smatch m;
  for (const auto& regex : _config.itemLabelRegexes) {
    if (regex_search(line->text, m, regex)) {
      return true;
    }
  }

  return false;
}

// _________________________________________________________________________________________________
bool TextBlocksDetectionUtils::computeIsPrefixedByFootnoteLabel(const PdfTextLine* line,
      const unordered_set<string>* potentialFootnoteLabels) {
  assert(line);

  // The line is not prefixed by a footnote label if it does not contain any words.
  const vector<PdfWord*>& words = line->words;
  if (words.empty()) {
    return false;
  }

  // Compute the superscripted prefix of the line, that is: the concatenation of all superscripted
  // characters in front of the line.
  const PdfWord* firstWord = words[0];
  string superScriptPrefix;
  for (const auto* ch : firstWord->characters) {
    if (!ch->isSuperscript) {
      break;
    }
    superScriptPrefix += ch->text;
  }

  // If potentialFootnoteLabels is specified, it must contain the superscripted prefix.
  if (potentialFootnoteLabels) {
    return potentialFootnoteLabels->count(superScriptPrefix) > 0;
  }

  // The superscripted prefix must not be empty.
  return !superScriptPrefix.empty();
}

// _________________________________________________________________________________________________
bool TextBlocksDetectionUtils::computeEndsWithSentenceDelimiter(const PdfTextLine* line) {
  assert(line);

  if (line->text.empty()) {
    return false;
  }

  return _config.sentenceDelimiterAlphabet.find(line->text.back()) != std::string::npos;
}


}  // namespace ppp::utils
