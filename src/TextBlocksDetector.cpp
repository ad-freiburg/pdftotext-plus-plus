/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 *
 * // The detection of text blocks is split in two steps.
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
#include "./utils/Utils.h"
#include "./utils/PdfElementUtils.h"
#include "./utils/TextLineUtils.h"
#include "./utils/MathUtils.h"

using namespace std;

// =================================================================================================

const std::string FOOTNOTE_LABEL_ALPHABET = "*∗†‡?";
const std::string FORMULA_ID_ALPHABET = "=+";
const unordered_set<string> lastNamePrefixes = { "van", "von", "de" };

// _________________________________________________________________________________________________
TextBlocksDetector::TextBlocksDetector(PdfDocument* doc, bool debug, int debugPageFilter) {
  _doc = doc;
  _log = new Logger(debug ? DEBUG : INFO, debugPageFilter);

  _log->debug() << "=======================================" << endl;
  _log->debug() << "\033[1mDEBUG MODE | Detecting Text Blocks\033[0m" << endl;
  _log->debug() << " └─ debug page filter: " << debugPageFilter << endl;
}

// _________________________________________________________________________________________________
TextBlocksDetector::~TextBlocksDetector() {
  delete _log;
}

// _________________________________________________________________________________________________
void TextBlocksDetector::detect() {
  if (!_doc) {
    return;
  }

  if (_doc->pages.empty()) {
    return;
  }

  // Compute some statistics needed for detecting text blocks.
  // TODO: Compute this in the statistician.
  computeMostFreqTextLineDistance();

  // Detect the preliminary text blocks.
  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      vector<PdfTextLine*> currBlockLines;
      for (auto* line : segment->lines) {
        if (startsPreliminaryBlock(line)) {
          if (!currBlockLines.empty()) {
            createTextBlock(currBlockLines, &segment->blocks);
            currBlockLines.clear();
          }
        }
        currBlockLines.push_back(line);
      }
      if (!currBlockLines.empty()) {
        createTextBlock(currBlockLines, &segment->blocks);
      }
    }
  }

  // ===XXX===

  computeTextBlockTrimBoxes();
  computeTextLineIndentHierarchies();
  computeTextLineMargins();
  computeHangingIndents();

  for (auto* page : _doc->pages) {
    // Compute the potential footnote markers.
    unordered_set<string> potentialFootnoteMarkers;
    computePotentialFootnoteMarkers(page, &potentialFootnoteMarkers);

    for (auto* segment : page->segments) {
      for (auto* block : segment->blocks) {
        double percNoRightMarginLines = computePercentageNoRightMarginLines(block);
        bool isCentered = computeIsCentered(block);

        vector<PdfTextLine*> currBlockLines;
        for (auto* line : block->lines) {
          if (startsBlock(line, &potentialFootnoteMarkers, block->hangingIndent,
              percNoRightMarginLines, isCentered) && !currBlockLines.empty()) {
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
  if (!line) {
    return false;
  }

  int p = line->position->pageNum;
  _log->debug(p) << "= (pre) =================" << endl;
  _log->debug(p) << BOLD << "Line: \"" << line->text << "\"" << OFF << endl;
  _log->debug(p) << " └─ page:   " << line->position->pageNum << endl;
  _log->debug(p) << " └─ leftX:  " << line->position->leftX << endl;
  _log->debug(p) << " └─ upperY: " << line->position->upperY << endl;
  _log->debug(p) << " └─ rightX: " << line->position->rightX << endl;
  _log->debug(p) << " └─ lowerY: " << line->position->lowerY << endl;

  if (line->position->rotation != 0) {
    _log->debug(p) << " └─ rotation: " << line->position->rotation << endl;
    _log->debug(p) << " └─ rotLeftX:  " << line->position->getRotLeftX() << endl;
    _log->debug(p) << " └─ rotUpperY: " << line->position->getRotUpperY() << endl;
    _log->debug(p) << " └─ rotRightX: " << line->position->getRotRightX() << endl;
    _log->debug(p) << " └─ rotLowerY: " << line->position->getRotLowerY() << endl;
  }

  _log->debug(p) << "-------------------------" << endl;

  // Check if there is a previous line.
  if (!line->prevLine) {
    return true;
  }

  // Check if the line and the previous line is part of the same figure.
  Trool res = startsBlock_sameFigure(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check the rotation.
  res = startsBlock_rotation(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check the writing mode.
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
bool TextBlocksDetector::startsBlock(const PdfTextLine* line,
      const unordered_set<string>* potentialFootnoteLabels, double hangingIndent,
      double percNoRightMarginLines, bool isCentered) {

  if (!line) {
    return false;
  }

  int p = line->position->pageNum;
  string prevStr = line->prevLine ? line->prevLine->text : "-";
  string nextStr = line->nextLine ? line->nextLine->text : "-";
  string parStr = line->parentTextLine ? line->parentTextLine->text : "-";
  string prevSibStr = line->prevSiblingTextLine ? line->prevSiblingTextLine->text : "-";
  string nextSibStr = line->nextSiblingTextLine ? line->nextSiblingTextLine->text : "-";

  _log->debug(p) << BOLD << "Line: \"" << line->text << "\"" << OFF << endl;
  _log->debug(p) << " └─ page:   " << line->position->pageNum << endl;
  _log->debug(p) << " └─ leftX:  " << line->position->leftX << endl;
  _log->debug(p) << " └─ upperY: " << line->position->upperY << endl;
  _log->debug(p) << " └─ rightX: " << line->position->rightX << endl;
  _log->debug(p) << " └─ lowerY: " << line->position->lowerY << endl;

  if (line->position->rotation != 0) {
    _log->debug(p) << " └─ rotation: " << line->position->rotation << endl;
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

  if (!line->prevLine) {
    _log->debug(p) << "\033[1mstarts new block (no previous line).\033[0m" << endl;
    return true;
  }

  // Check if the distance between the current line and the previous line is increased compared to
  // the distance between the previous line and the previous but one line.
  Trool res = startsBlock_lineDistanceIncrease(line);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check if the block of which the line is a part of is centered.
  res = startsBlock_centered(line, isCentered);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check for enumeration items.
  res = startsBlock_item(line, isCentered, potentialFootnoteLabels);
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
  res = startsBlock_hangingIndent(line, hangingIndent);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  // Check indentation.
  res = startsBlock_indent(line, percNoRightMarginLines);
  if (res == Trool::True) {
    return true;
  } else if (res == Trool::False) {
    return false;
  }

  _log->debug(p) << BLUE << "continues block (no rule applied)." << OFF << endl;
  return false;
}

// =================================================================================================
// Methods to compute some statistics.

// _________________________________________________________________________________________________
void TextBlocksDetector::computeMostFreqTextLineDistance() {
  unordered_map<double, int> lineDistanceCounts;
  unordered_map<double, unordered_map<double, int>> lineDistanceCountsPerFontSize;

  // Iterate through the text lines and consider (prev. line, curr line) pairs.
  // Compute the vertical distance between both lines and count the distances.
  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      for (size_t i = 1; i < segment->lines.size(); i++) {
        PdfTextLine* prevLine = segment->lines.at(i - 1);
        PdfTextLine* currLine = segment->lines.at(i);

        if (!prevLine || !currLine) {
          continue;
        }

        // Ignore the lines if they are positioned on different pages.
        if (prevLine->position->pageNum != currLine->position->pageNum) {
          continue;
        }

        // Ignore the lines if their writing mode differ.
        if (prevLine->position->wMode != 0 || currLine->position->wMode != 0) {
          continue;
        }

        // Ignore the lines if their rotation differ.
        if (prevLine->position->rotation != 0 || currLine->position->rotation != 0) {
          continue;
        }

        // Compute the line distance and count it.
        // TODO: Explain why baseBBox is used here.
        double lineDistance = round(currLine->baseBBoxUpperY - prevLine->baseBBoxLowerY, 1);
        lineDistance = max(0.0, lineDistance);
        lineDistanceCounts[lineDistance]++;

        // For computing line distances per font size, ignore the lines if their font sizes differ.
        double prevFontSize = round(prevLine->fontSize, 1);
        double currFontSize = round(currLine->fontSize, 1);
        if (math_utils::equal(prevFontSize, currFontSize, 0.01)) {
          lineDistanceCountsPerFontSize[currFontSize][lineDistance]++;
        }
      }
    }
  }

  // Compute the most frequent line distance.
  int mostFreqLineDistanceCount = 0;
  for (const auto& pair : lineDistanceCounts) {
    if (pair.second > mostFreqLineDistanceCount) {
      _mostFreqLineDistance = pair.first;
      mostFreqLineDistanceCount = pair.second;
    }
  }

  // Compute the most frequent line distances per font size.
  unordered_map<double, int> mostFreqLineDistanceCountPerFontSize;
  for (const auto& doubleMapPair : lineDistanceCountsPerFontSize) {
    const double fontSize = doubleMapPair.first;
    const unordered_map<double, int>& lineDistanceFreqs = doubleMapPair.second;
    for (const auto& doubleIntPair : lineDistanceFreqs) {
      double lineDistance = doubleIntPair.first;
      double count = doubleIntPair.second;
      int mostFreqCount = 0;
      if (mostFreqLineDistanceCountPerFontSize.count(fontSize) > 0) {
        mostFreqCount = mostFreqLineDistanceCountPerFontSize.at(fontSize);
      }
      if (count > mostFreqCount) {
        _mostFreqLineDistancePerFontSize[fontSize] = lineDistance;
        mostFreqLineDistanceCountPerFontSize[fontSize] = count;
      }
    }
  }
}

// _________________________________________________________________________________________________
void TextBlocksDetector::computeTextLineMargins() {
  // A mapping of left margins to their frequencies, for computing the most freq. left margin.
  unordered_map<double, int> leftMarginFreqs;
  unordered_map<double, int> rightMarginFreqs;
  double numZeroRightMarginTextLines = 0;
  double numBodyTextLines = 0;

  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      for (size_t i = 0; i < segment->blocks.size(); i++) {
        PdfTextBlock* prevBlock = i > 0 ? segment->blocks.at(i - 1) : nullptr;
        PdfTextBlock* currBlock = segment->blocks.at(i);
        PdfTextBlock* nextBlock = i < segment->blocks.size() - 1 ? segment->blocks.at(i + 1) : nullptr;

        // Enlarge short text blocks that consists of two lines.
        double blockTrimRightX = currBlock->trimRightX;
        if (currBlock->lines.size() == 2) {
          double leftMargin = currBlock->position->leftX - segment->position->leftX;
          double rightMargin = segment->position->rightX - currBlock->position->rightX;
          double isCentered = math_utils::equal(leftMargin, rightMargin, _doc->avgGlyphWidth);
          if (!isCentered) {
            if (prevBlock) { blockTrimRightX = max(blockTrimRightX, prevBlock->trimRightX); }
            if (nextBlock) { blockTrimRightX = max(blockTrimRightX, nextBlock->trimRightX); }
          }
        }

        for (size_t j = 0; j < currBlock->lines.size(); j++) {
          PdfTextLine* prevLine = j > 0 ? currBlock->lines.at(j - 1) : nullptr;
          PdfTextLine* currLine = currBlock->lines.at(j);
          PdfTextLine* nextLine = j < currBlock->lines.size() - 1 ? currBlock->lines.at(j + 1) : nullptr;

          // double trimLeftX = max(segment->trimLeftX, block->position->leftX);
          // double trimRightX = min(segment->trimRightX, block->position->rightX);
          // double trimRightX = segment->trimRightX;
          currLine->leftMargin = round(currLine->position->leftX - currBlock->trimLeftX);
          currLine->rightMargin = round(blockTrimRightX - currLine->position->rightX);

          // Make sure that the indent is measured only for lines from body text paragraphs.
          // Reason: Lines from the bibliography could have other indents.
          if (prevLine) {
            if (prevLine->fontName != _doc->mostFreqFontName ||
                  !math_utils::equal(prevLine->fontSize, _doc->mostFreqFontSize, 1)) {
              continue;
            }
          }
          if (currLine) {
            if (currLine->fontName != _doc->mostFreqFontName ||
                  !math_utils::equal(currLine->fontSize, _doc->mostFreqFontSize, 1)) {
              continue;
            }
          }
          if (nextLine) {
            if (nextLine->fontName != _doc->mostFreqFontName ||
                  !math_utils::equal(nextLine->fontSize, _doc->mostFreqFontSize, 1)) {
              continue;
            }
          }

          // Count the number of justified text lines.
          if (math_utils::equal(currLine->rightMargin, 0, _doc->avgGlyphWidth)) {
            numZeroRightMarginTextLines++;
          }
          numBodyTextLines++;

          double prevLineLeftMargin = 0.0;
          if (prevLine) {
            prevLineLeftMargin = prevLine->leftMargin;
          }

          double nextLineLeftMargin = 0.0;
          if (nextLine) {
            nextLineLeftMargin = round(nextLine->position->getRotLeftX() - currBlock->position->getRotLeftX());
          }

          if (!math_utils::equal(prevLineLeftMargin, 0, _doc->avgGlyphWidth)) {
            continue;
          }

          if (!math_utils::equal(nextLineLeftMargin, 0, _doc->avgGlyphWidth)) {
            continue;
          }

          // We are only interested in left margins > 0.
          if (smaller(currLine->leftMargin, _doc->avgGlyphWidth)) {
            continue;
          }

          leftMarginFreqs[currLine->leftMargin]++;
          rightMarginFreqs[currLine->rightMargin]++;
        }
      }
    }
  }

  // Compute the most frequent left margin.
  double mostFreqLineLeftMargin = 0;
  int mostFreqLineLeftMarginCount = 0;
  for (const auto& pair : leftMarginFreqs) {
    if (pair.second > mostFreqLineLeftMarginCount) {
      mostFreqLineLeftMargin = pair.first;
      mostFreqLineLeftMarginCount = pair.second;
    }
  }
  _mostFreqLineLeftMargin = mostFreqLineLeftMargin;

  // Compute the most frequent right margin.
  double mostFreqLineRightMargin = 0;
  int mostFreqLineRightMarginCount = 0;
  for (const auto& pair : rightMarginFreqs) {
    if (pair.second > mostFreqLineRightMarginCount) {
      mostFreqLineRightMargin = pair.first;
      mostFreqLineRightMarginCount = pair.second;
    }
  }
  _mostFreqLineRightMargin = mostFreqLineRightMargin;

  // Compute the percentage of body text lines with right margin = 0.
  _percZeroRightMarginTextLines = numBodyTextLines > 0 ? numZeroRightMarginTextLines / numBodyTextLines : 0;
}

// _________________________________________________________________________________________________
void TextBlocksDetector::computeTextBlockTrimBoxes() const {
  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      // Compute the most frequent rightX coordinates among the text lines in the segment.
      unordered_map<double, int> rightXFreqs;

      for (auto* block : segment->blocks) {
        block->trimLeftX = block->position->leftX;
        block->trimUpperY = block->position->upperY;
        block->trimRightX = block->position->rightX;
        block->trimLowerY = block->position->lowerY;

        for (auto* line : block->lines) {
          double rightX = round(line->position->getRotRightX());
          rightXFreqs[rightX]++;
        }
      }

      double mostFreqRightX = 0;
      int mostFreqRightXCount = 0;
      for (const auto& pair : rightXFreqs) {
        if (pair.second > mostFreqRightXCount) {
          mostFreqRightX = pair.first;
          mostFreqRightXCount = pair.second;
        }
      }

      double numLines = segment->lines.size();
      double mostFreqRightXRatio = mostFreqRightXCount / numLines;

      if (mostFreqRightXRatio < 0.5) {
        continue;
      }

      for (auto* block : segment->blocks) {
        block->trimRightX = min(block->position->rightX, mostFreqRightX);
      }
    }
  }
}

// _________________________________________________________________________________________________
void TextBlocksDetector::computeTextLineIndentHierarchies() {
  for (auto* page : _doc->pages) {
    stack<PdfTextLine*> lineStack;
    PdfTextLine* prevLine = nullptr;
    for (auto* segment : page->segments) {
      for (auto* block : segment->blocks) {
        for (auto* line : block->lines) {
          // Compute the actual line distance.
          if (prevLine) {
            double actualLineDistance = 0;
            switch(line->position->rotation) {
              case 0:
              case 1:
                actualLineDistance = line->position->getRotUpperY() - prevLine->position->getRotLowerY();
                break;
              case 2:
              case 3:
                actualLineDistance = prevLine->position->getRotLowerY() - line->position->getRotUpperY();
                break;
            }
            if (larger(abs(actualLineDistance), max(10.0, 3 * _mostFreqLineDistance))) {
              lineStack = stack<PdfTextLine*>();
            }
          }
          prevLine = line;

          while (!lineStack.empty()) {
            if (!larger(lineStack.top()->position->leftX, line->position->leftX, _doc->avgGlyphWidth)) {
              break;
            }
            lineStack.pop();
          }

          if (lineStack.empty()) {
            lineStack.push(line);
            continue;
          }

          // pair<double, double> xOverlapRatios = computeXOverlapRatios(lineStack.top(), line);
          // double maxXOVerlapRatio = max(xOverlapRatios.first, xOverlapRatios.second);
          // if (maxXOVerlapRatio > 0) {
          if (lineStack.top()->position->lowerY < line->position->lowerY) {
            if (math_utils::equal(lineStack.top()->position->leftX, line->position->leftX, _doc->avgGlyphWidth)) {
              lineStack.top()->nextSiblingTextLine = line;
              line->prevSiblingTextLine = lineStack.top();
              line->parentTextLine = lineStack.top()->parentTextLine;
              lineStack.pop();
              lineStack.push(line);
              continue;
            }

            if (smaller(lineStack.top()->position->leftX, line->position->leftX, _doc->avgGlyphWidth)) {
              line->parentTextLine = lineStack.top();

              lineStack.push(line);
              continue;
            }
          }
        }
      }
    }
  }
}

// _________________________________________________________________________________________________
void TextBlocksDetector::computeHangingIndents() const {
  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      for (auto* block : segment->blocks) {
        // Compute the most frequent left margin > 0.
        int numLines = 0;
        int numLeftMarginLines = 0;
        unordered_map<double, int> leftMarginCounts;
        for (const auto* line : block->lines) {
          if (line->text.size() < 3) { continue; }
          if (larger(line->leftMargin, 0, _doc->avgGlyphWidth)) {
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
          continue;
        }

        // Abort if there are no lines with leftMargin > 0.
        if (numLeftMarginLines == 0) {
          continue;
        }

        // Abort if less than 50% of the indented lines are indented by the same level.
        if (mostFreqLeftMarginCount <= 0.5 * numLeftMarginLines) {
          continue;
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

          bool isCentered = math_utils::equal(line->leftMargin, line->rightMargin, _doc->avgGlyphWidth) &&
              larger(line->leftMargin, _doc->avgGlyphWidth, 0);
          bool isNotIndented = math_utils::equal(line->leftMargin, 0, _doc->avgGlyphWidth);
          bool isIndented = math_utils::equal(line->leftMargin, mostFreqLeftMargin, _doc->avgGlyphWidth);
          bool isLower = !line->text.empty() && islower(line->text[0]);
          bool startsWithLastNamePrefix = lastNamePrefixes.count(line->words[0]->text) > 0;

          if (isCentered) {
            continue;
          }

          if (i == 0) {
            isFirstLineIndented = isIndented;
            isFirstLineShort = larger(line->rightMargin, 0, 4 * _doc->avgGlyphWidth);
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
          continue;
        }

        // The block is *not* in hanging indent format if there is at least one non-indented line
        // that starts with a lowercase character.
        if (numLowercasedNotIndentedLines > 0) {
          continue;
        }

        // The block is in hanging indent format if the first line is not indented, but all other
        // lines. This should identify single enumeration items, e.g., in the format:
        // Dynamics: The low energy behavior of
        //    a physical system depends on its
        //    dynamics.
        if (!isFirstLineIndented && !isFirstLineShort && isAllOtherLinesIndented) {
          block->hangingIndent = mostFreqLeftMargin;
          continue;
        }

        // The block is in hanging indent format if all non-indented lines start with an uppercase
        // character and if the number of non-indented lines exceed a certain threshold.
        if (numNotIndentedLines >= 10 && numLowercasedNotIndentedLines == 0) {
          block->hangingIndent = mostFreqLeftMargin;
          continue;
        }

        // The block is in hanging indent format if there is at least one indented line that start
        // with a lowercase character.
        if (numLines >= 4 && numLowercasedIndentedLines > 0) {
          block->hangingIndent = mostFreqLeftMargin;
          continue;
        }
      }
    }
  }
}

// _________________________________________________________________________________________________
double TextBlocksDetector::computePercentageNoRightMarginLines(const PdfTextBlock* block) const {
  double numLines = 0;
  double numNoRightMarginLines = 0;
  for (const auto* line : block->lines) {
    if (math_utils::equal(line->rightMargin, 0, _doc->avgGlyphWidth)) {
      numNoRightMarginLines++;
    }
    numLines++;
  }
  return numLines > 0 ? numNoRightMarginLines / numLines : 0;
}

// _________________________________________________________________________________________________
void TextBlocksDetector::computePotentialFootnoteMarkers(const PdfPage* page,
      unordered_set<string>* footnoteMarkers) const {
  for (auto* segment : page->segments) {
    for (const auto* line : segment->lines) {
      for (const auto* word : line->words) {
        // Iterate through the glyphs of the word and merge each adjacent superscripts which are
        // positioned before the word (we don't want to consider superscript that are positioned
        // behind the word). Consider each merged superscript string as a potential footnote marker.
        // TODO: We do not store the info about whether a superscript is positioned before or after
        // a word. As a workaround, consider a superscript as part of a potential footnote marker
        // only when a non-subscript and non-superscript was already seen.
        bool nonSubSuperscriptSeen = false;
        string footnoteMarker;
        for (const auto* glyph : word->glyphs) {
          if (!nonSubSuperscriptSeen && !glyph->isSubscript && !glyph->isSuperscript) {
            nonSubSuperscriptSeen = true;
            continue;
          }
          if (!nonSubSuperscriptSeen) {
            continue;
          }

          bool isMarker = glyph->isSuperscript && !glyph->text.empty() && isalnum(glyph->text[0]);
          isMarker |= !glyph->text.empty() && FOOTNOTE_LABEL_ALPHABET.find(glyph->text[0]) != string::npos;

          if (!isMarker) {
            if (!footnoteMarker.empty()) {
              footnoteMarkers->insert(footnoteMarker);
              footnoteMarker.clear();
            }
            continue;
          }

          footnoteMarker += glyph->text;
        }
        if (!footnoteMarker.empty()) {
          footnoteMarkers->insert(footnoteMarker);
        }
      }
    }
  }
}

// =================================================================================================

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

  blocks->push_back(block);
}

// =================================================================================================

// _________________________________________________________________________________________________
bool TextBlocksDetector::computeIsCentered(const PdfTextBlock* block) const {
  bool hasLineWithLargeMarginNoFormula = false;
  int numLinesNoMargin = 0;

  for (size_t i = 1; i < block->lines.size(); i++) {
    const PdfTextLine* prevLine = block->lines[i - 1];
    const PdfTextLine* currLine = block->lines[i];

    pair<double, double> ratios = computeXOverlapRatios(prevLine, currLine);
    double maxXOverlapRatio = max(ratios.first, ratios.second);

    if (smaller(maxXOverlapRatio, 1, 0.01)) {
      return false;
    }

    double leftXOffset = abs(prevLine->position->leftX - currLine->position->leftX);
    double rightXOffset = abs(prevLine->position->rightX - currLine->position->rightX);
    bool isEqualOffset = math_utils::equal(leftXOffset, rightXOffset, 2 * _doc->avgGlyphWidth);

    if (!isEqualOffset) {
      return false;
    }

    bool isLargeLeftXOffset = larger(leftXOffset, 0, 2 * _doc->avgGlyphWidth);
    bool isLargeRightXOffset = larger(rightXOffset, 0, 2 * _doc->avgGlyphWidth);
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
bool TextBlocksDetector::isCenteredBlock(const PdfTextLine* prevLine, const PdfTextLine* currLine,
    bool verbose) const {


  if (!prevLine) {
    return false;
  }

  if (!currLine) {
    return false;
  }

  // The previous and current line is considered to be a part of a centered block, when all of the
  // following requirements are true:

  // (1) The differences between the leftX values is equal to the differences of the rightX values.
  double leftXOffset = abs(prevLine->position->leftX - currLine->position->leftX);
  double rightXOffset = abs(prevLine->position->rightX - currLine->position->rightX);
  bool isEqualOffset = math_utils::equal(leftXOffset, rightXOffset, 2 * _doc->avgGlyphWidth);




  // (2) One of the following values is larger than 0: the left margin or the right margin of the
  // previous or current line; leftXOffset, rightXOffset. This requirement exists to not
  // accidentally consider two text lines that are actually justified (for which requirement 1 is
  // also true) to be part of a centered block.
  bool isLargePrevLeftMargin = larger(prevLine->leftMargin, 0, _doc->avgGlyphWidth);
  bool isLargePrevRightMargin = larger(prevLine->rightMargin, 0, _doc->avgGlyphWidth);
  bool isLargeCurrLeftMargin = larger(currLine->leftMargin, 0, _doc->avgGlyphWidth);
  bool isLargeCurrRightMargin = larger(currLine->rightMargin, 0, _doc->avgGlyphWidth);
  bool isLargePrevMargin = isLargePrevLeftMargin || isLargePrevRightMargin;
  bool isLargeCurrMargin = isLargeCurrLeftMargin || isLargeCurrRightMargin;
  bool isLargeMargin = isLargePrevMargin || isLargeCurrMargin;
  bool isLargeLeftXOffset = larger(leftXOffset, 0, _doc->avgGlyphWidth);
  bool isLargeRightXOffset = larger(rightXOffset, 0, _doc->avgGlyphWidth);
  bool isLargeXOffset = isLargeLeftXOffset || isLargeRightXOffset;

  // (3) The height of the previous line and current line is almost the same. This requirement
  // exists to not accidentally consider a centered formula and a subsequent text line, which is
  // actually part of a justified block, to be part of a centered block.
  // TODO: This assumes that the height of formulas are larger than the height of text lines, which
  // is not true in any case.
  double prevLineHeight = prevLine->position->getHeight();
  double currLineHeight = currLine->position->getHeight();
  bool isSameHeight = math_utils::equal(prevLineHeight, currLineHeight, 0.25 * _doc->mostFreqWordHeight);

  bool isCentered = isEqualOffset && (isLargeMargin || isLargeXOffset) && isSameHeight;

  if (verbose) {
    int p = currLine->position->pageNum;
    _log->debug(p) << "Checking for centered block..." << endl;
    _log->trace(p) << " └─ leftXOffset: " << leftXOffset << endl;
    _log->trace(p) << " └─ rightXOffset: " << rightXOffset << endl;
    _log->debug(p) << " └─ isEqualOffset: " << isEqualOffset << endl;
    _log->trace(p) << " └─ isLargePrevLeftMargin: " << isLargePrevLeftMargin << endl;
    _log->trace(p) << " └─ isLargePrevRightMargin: " << isLargePrevRightMargin << endl;
    _log->trace(p) << " └─ isLargeCurrLeftMargin: " << isLargeCurrLeftMargin << endl;
    _log->trace(p) << " └─ isLargeCurrRightMargin: " << isLargeCurrRightMargin << endl;
    _log->trace(p) << " └─ isLargePrevMargin: " << isLargePrevMargin << endl;
    _log->trace(p) << " └─ isLargeCurrMargin: " << isLargeCurrMargin << endl;
    _log->trace(p) << " └─ isLargeMargin: " << isLargeMargin << endl;
    _log->trace(p) << " └─ isLargeLeftXOffset: " << isLargeLeftXOffset << endl;
    _log->trace(p) << " └─ isLargeRightXOffset: " << isLargeRightXOffset << endl;
    _log->debug(p) << " └─ isLargeXOffset: " << isLargeXOffset << endl;
    _log->trace(p) << " └─ prevLine.height: " << prevLineHeight << endl;
    _log->trace(p) << " └─ currLine.height: " << currLineHeight << endl;
    _log->trace(p) << " └─ _doc->mostFreqWordHeight: " << _doc->mostFreqWordHeight << endl;
    _log->debug(p) << " └─ isSameHeight: " << isSameHeight << endl;
    _log->debug(p) << "is centered block: " << isCentered << endl;
  }

  return isCentered;
}



// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_sameFigure(const PdfTextLine* line, bool verbose) const {
  assert(line);

  if (!line->prevLine) {
    return Trool::None;
  }

  int p = line->position->pageNum;

  // Compute the figure overlapped by the previous line and the current line.
  vector<PdfFigure*>& figures = _doc->pages[p - 1]->figures;
  PdfFigure* prevLineOverlapsFigure = element_utils::overlapsFigure(line->prevLine, figures);
  PdfFigure* currLineOverlapsFigure = element_utils::overlapsFigure(line, figures);

  if (verbose) {
    _log->debug(p) << BLUE << "Are prev+curr line part of the same figure?" << OFF << endl;
    _log->debug(p) << " └─ prevLine: \"" << line->prevLine->text << "\"" << endl;
    _log->debug(p) << " └─ prevLine.isPartOfFigure: " << prevLineOverlapsFigure << endl;
    _log->debug(p) << " └─ currLine.isPartOfFigure: " << currLineOverlapsFigure << endl;
  }

  // The line does not start a block if the prev+curr line are part of the same figure.
  if (prevLineOverlapsFigure && prevLineOverlapsFigure == currLineOverlapsFigure) {
    if (verbose) {
      _log->debug(p) << BLUE << " yes (line continues block)." << OFF << endl;
    }
    return Trool::False;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_rotation(const PdfTextLine* line, bool verbose) const {
  assert(line);

  if (!line->prevLine) {
    return Trool::None;
  }

  int p = line->position->pageNum;
  if (verbose) {
    _log->debug(p) << BLUE << "Are the rotations of the prev+curr line different?" << OFF << endl;
    _log->debug(p) << " └─ prevLine: \"" << line->prevLine->text << "\"" << endl;
    _log->debug(p) << " └─ prevLine.rotation: " << line->prevLine->position->rotation << endl;
    _log->debug(p) << " └─ currLine.rotation: " << line->position->rotation << endl;
  }

  // The line starts a new block if the rotations of the prev+curr line are different.
  if (line->prevLine->position->rotation != line->position->rotation) {
    if (verbose) {
      _log->debug(p) << BLUE << " yes (line starts new block)." << OFF << endl;
    }
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_wMode(const PdfTextLine* line, bool verbose) const {
  assert(line);

  if (!line->prevLine) {
    return Trool::None;
  }

  int p = line->position->pageNum;
  if (verbose) {
    _log->debug(p) << BLUE << "Are the wModes of the prev+curr line different?" << OFF << endl;
    _log->debug(p) << " └─ prevLine: \"" << line->prevLine->text << "\"" << endl;
    _log->debug(p) << " └─ prevLine.wMode: " << line->prevLine->position->wMode << endl;
    _log->debug(p) << " └─ currLine.wMode: " << line->position->wMode << endl;
  }

  // The line starts a new block if the writing modes of the prev+curr line are not equal.
  if (line->prevLine->position->wMode != line->position->wMode) {
    if (verbose) { _log->debug(p) << BLUE << " yes (line starts new block)." << OFF << endl; }
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_fontSize(const PdfTextLine* line, double maxDelta,
      bool verbose) const {
  assert(line);

  if (!line->prevLine) {
    return Trool::None;
  }

  int p = line->position->pageNum;
  if (verbose) {
    _log->debug(p) << BLUE << "Are the font sizes of the prev+curr line different?" << OFF << endl;
    _log->debug(p) << " └─ prevLine: \"" << line->prevLine->text << "\"" << endl;
    _log->debug(p) << " └─ prevLine.mostFreqFontSize: " << line->prevLine->fontSize << endl;
    _log->debug(p) << " └─ currLine.mostFreqFontSize: " << line->fontSize << endl;
    _log->debug(p) << " └─ prevLine.maxFontSize:      " << line->prevLine->maxFontSize << endl;
    _log->debug(p) << " └─ currLine.maxFontSize:      " << line->maxFontSize << endl;
    _log->debug(p) << " └─ maxDelta: " << maxDelta << endl;
  }

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
    if (verbose) { _log->debug(p) << BLUE << " yes (line starts new block)." << OFF << endl; }
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_lineDistance(const PdfTextLine* line, double minTolerance,
      double toleranceFactor, bool verbose) const {
  assert(line);

  if (!line->prevLine) {
    return Trool::None;
  }

  // Compute the expected line distance.
  double fontSize = round(line->fontSize, FONT_SIZE_PREC);
  double expectedLineDistance = 0;
  if (_mostFreqLineDistancePerFontSize.count(fontSize) > 0) {
    double eld = _mostFreqLineDistancePerFontSize.at(fontSize);
    expectedLineDistance = max(expectedLineDistance, eld);
  }
  expectedLineDistance = max(expectedLineDistance, _mostFreqLineDistance);

  // Compute the actual line distance.
  double actualLineDistance = text_line_utils::computeTextLineDistance(line->prevLine, line);
  actualLineDistance = round(actualLineDistance, LINE_DIST_PREC);

  // Compute the tolerance.
  double tolerance = max(minTolerance, toleranceFactor * expectedLineDistance);

  int p = line->position->pageNum;
  if (verbose) {
    _log->debug(p) << BLUE << "Is the distance to prev line larger than expected?" << OFF << endl;
    _log->debug(p) << " └─ expec. line distance: " << expectedLineDistance << endl;
    _log->debug(p) << " └─ actual line distance: " << actualLineDistance << endl;
    _log->debug(p) << " └─ minTolerance:    " << minTolerance << endl;
    _log->debug(p) << " └─ toleranceFactor: " << toleranceFactor << endl;
    _log->debug(p) << " └─ tolerance:       " << tolerance << endl;
  }

  // The line does *not* start a block if the actual line distance is negative.
  if (math_utils::equalOrSmaller(actualLineDistance, 0, 0)) {
    if (verbose) {
      _log->debug(p) << BLUE << " no, distance is negative (line continues block)" << OFF << endl;
    }
    return Trool::False;
  }

  // The line starts a block if the actual line distance is larger than the expected line
  // distance, under consideration of a small threshold.
  if (math_utils::larger(actualLineDistance, expectedLineDistance, tolerance)) {
    if (verbose) { _log->debug(p) << BLUE << " yes (line starts new block)." << OFF << endl; }
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_lineDistanceIncrease(const PdfTextLine* line,
      double toleranceFactor, bool verbose) const {
  assert(line);

  PdfTextLine* prevLine = line->prevLine;
  if (!prevLine) {
    return Trool::None;
  }

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
  if (verbose) {
    _log->debug(p) << BLUE << "Is the distance between the curr+prev line larger than "
        << "the distance between the prev+prevPrev line?" << OFF << endl;
    _log->debug(p) << " └─ distance curr+prev line:     " << distance << endl;
    _log->debug(p) << " └─ distance prev+prevPrev line: " << prevDistance << endl;
    _log->debug(p) << " └─ toleranceFactor: " << toleranceFactor << endl;
    _log->debug(p) << " └─ tolerance:       " << tolerance << endl;
  }

  // The line starts a block if the actual line distance is larger than the expected line distance,
  // under consideration of a small threshold.
  if (math_utils::larger(distance, prevDistance, tolerance)) {
    if (verbose) { _log->debug(p) << BLUE << " yes (line starts new block)." << OFF << endl; }
    return Trool::True;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_centered(const PdfTextLine* line, bool isCentered,
      bool verbose) const {
  assert(line);

  // Check if the line is the first line of an enumeration item. This should primarily detect
  // blocks containing affiliation information, which are often centered and prefixed by a
  // superscript.
  bool isFirstLineOfItem = text_line_utils::computeIsFirstLineOfItem(line);

  int p = line->position->pageNum;
  if (verbose) {
    _log->debug(p) << BLUE << "Is the preliminary block centered?" << OFF << endl;
    _log->debug(p) << " └─ block.isCentered: " << isCentered << endl;
    _log->debug(p) << " └─ line.isFirstLineOfItem: " << isFirstLineOfItem << endl;
  }

  if (!isCentered) {
    return Trool::None;
  }

  if (isFirstLineOfItem) {
    if (verbose) {
      _log->debug(p) << BLUE << " yes + line is 1st line of item (starts block)." << OFF << endl;
    }
    return Trool::True;
  }

  if (verbose) {
    _log->debug(p) << BLUE << " yes (continues block)." << OFF << endl;
  }
  return Trool::False;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_item(const PdfTextLine* line, bool isCentered,
      const unordered_set<string>* fnLabels, bool verbose) const {
  assert(line);

  if (line->words.empty()) {
    return Trool::None;
  }

  // The line starts a new block if it is the first line of an enumeration item.
  bool isPrevFirstLine = text_line_utils::computeIsFirstLineOfItem(line->prevLine, fnLabels);
  bool isCurrFirstLine = text_line_utils::computeIsFirstLineOfItem(line, fnLabels);
  bool isPrevContLine = text_line_utils::computeIsContinuationLineOfItem(line->prevLine, fnLabels);
  bool isCurrContLine = text_line_utils::computeIsContinuationLineOfItem(line, fnLabels);
  bool isPrevPartOfItem = isPrevFirstLine || isPrevContLine;
  bool isCurrPartOfItem = isCurrFirstLine || isCurrContLine;
  double xOffset = element_utils::computeLeftXOffset(line->prevLine, line);
  bool hasPrevLineCapacity = text_line_utils::computeHasPrevLineCapacity(line);

  int p = line->position->pageNum;
  if (verbose) {
    _log->debug(p) << BLUE << "Is the line part of an item?" << endl;
    _log->debug(p) << " └─ prevLine.isFirstLineOfItem: " << isPrevFirstLine << endl;
    _log->debug(p) << " └─ prevLine.isContLineOfItem:  " << isPrevContLine << endl;
    _log->debug(p) << " └─ currLine.isFirstLineOfItem: " << isCurrFirstLine << endl;
    _log->debug(p) << " └─ currLine.isContLineOfItem:  " << isCurrContLine << endl;
    _log->debug(p) << " └─ xOffset prevLine/currLine:  " << xOffset << endl;
    _log->debug(p) << " └─ prevLine.hasCapacity: " << hasPrevLineCapacity << endl;
  }

  if (isCurrFirstLine) {
    _log->debug(p) << BLUE << " yes, is first line of item (starts block)." << OFF << endl;
    return Trool::True;
  }

  if (isCurrContLine) {
    _log->debug(p) << BLUE << " yes, is continuation of item" << OFF << endl;

    if (isCentered) {
      _log->debug(p) << BLUE << " + block is centered (continues block)." << OFF << endl;
      return Trool::False;
    } else if (hasPrevLineCapacity) {
      _log->debug(p) << BLUE << " + prev line has capacity (starts block)." << OFF << endl;
      return Trool::True;
    } else if (isPrevFirstLine) {
      _log->debug(p) << BLUE << " + prev line is 1st item line (continues block)." << OFF << endl;
      return Trool::False;
    } else if (isPrevContLine) {
      // TODO: Parameterize the tolerance and add the tolerance to the debug output.
      if (math_utils::between(xOffset, -_doc->avgGlyphWidth, 6 * _doc->avgGlyphWidth)) {
        _log->debug(p) << BLUE << " + xOffset in indent tolerance (continues block)." << OFF << endl;
        return Trool::False;
      } else {
        _log->debug(p) << BLUE << " + xOffset not in indent tolerance (starts block)." << OFF << endl;
        return Trool::True;
      }
    } else {
      _log->debug(p) << BLUE << " (continues block)." << OFF << endl;
      return Trool::False;
    }
  }

  if (isPrevPartOfItem && !isCurrPartOfItem) {
    _log->debug(p) << BLUE << " no, but prev line is part of item" << OFF << endl;

    if (hasPrevLineCapacity) {
      _log->debug(p) << BLUE << " + prev line has capacity (starts block)." << OFF << endl;
      return Trool::True;
    }

    // There could be an item in the following format:
    //    (i) This is an item that continues in the next
    //  line. Note the smaller leftX of the second line.
    if (!text_element_utils::computeEndsWithSentenceDelimiter(line->prevLine) &&
          !text_element_utils::computeStartsWithUpper(line)) {
      _log->debug(p) << BLUE << " + prev line does not end with sentence delimiter + "
          << "curr line does not start with an uppercase (continues block)." << OFF << endl;
      return Trool::False;
    }

    // TODO: Parameterize the 3.
    if (math_utils::larger(line->prevLine->rightMargin, 3 * _doc->avgGlyphWidth, 0)) {
      _log->debug(p) << BLUE << " + right margin of previous line is too large." << OFF << endl;
      return Trool::True;
    }
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_emphasized(const PdfTextLine* line, bool verbose) const {
  assert(line);

  if (!line->prevLine) {
    return Trool::None;
  }

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
  if (verbose) {
    _log->debug(p) << "Are the prev and curr line emphasized?" << endl;
    _log->debug(p) << " └─ prevLine.isEmphasized: " << isPrevLineEmphasized << endl;
    _log->debug(p) << " └─ currLine.isEmphasized: " << isCurrLineEmphasized << endl;
    _log->debug(p) << " └─ prevLine.fontName: " << line->prevLine->fontName << endl;
    _log->debug(p) << " └─ currLine.fontName: " << line->fontName << endl;
    _log->debug(p) << " └─ prevLine.fontSize: " << line->prevLine->fontSize << endl;
    _log->debug(p) << " └─ currLine.fontSize: " << line->fontSize << endl;
  }

  if (isPrevLineEmphasized && isCurrLineEmphasized && hasEqualFontName && hasEqualFontSize) {
    _log->debug(p) << BLUE << " yes + font names and -sizes are equal (continues block)." << endl;
    return Trool::False;
  }

  return Trool::None;
}

// _________________________________________________________________________________________________
Trool TextBlocksDetector::startsBlock_hangingIndent(const PdfTextLine* line, double hangingIndent,
      bool verbose) const {
  assert(line);

  if (hangingIndent <= 0.0) {
    return Trool::None;
  }

  // TODO
  bool isPrevNotIndented = math_utils::smaller(line->prevLine->leftMargin, hangingIndent, _doc->avgGlyphWidth);
  bool isCurrNotIndented = math_utils::smaller(line->leftMargin, hangingIndent, _doc->avgGlyphWidth);
  bool isPrevIndented = math_utils::equal(line->prevLine->leftMargin, hangingIndent, _doc->avgGlyphWidth);
  bool isCurrIndented = math_utils::equal(line->leftMargin, hangingIndent, _doc->avgGlyphWidth);
  bool isPrevMoreIndented = math_utils::larger(line->prevLine->leftMargin, hangingIndent, _doc->avgGlyphWidth);
  bool isCurrMoreIndented = math_utils::larger(line->leftMargin, hangingIndent, _doc->avgGlyphWidth);
  double xOffset = element_utils::computeLeftXOffset(line->prevLine, line);
  bool hasPrevLineCapacity = text_line_utils::computeHasPrevLineCapacity(line);

  int p = line->position->pageNum;
  if (verbose) {
    _log->debug(p) << "Is line part of a block in hanging indent?" << endl;
    _log->debug(p) << " └─ hangingIndent: " << hangingIndent << endl;
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
  }

  if (isCurrNotIndented) {
    _log->debug(p) << BLUE << " yes + curr line is not indented (starts block)." << endl;
    return Trool::True;
  }

  if (isCurrIndented) {
    _log->debug(p) << BLUE << " yes + curr line is indented." << endl;
    if (isPrevMoreIndented) {
      // TODO: Parameterize.
      if (math_utils::between(xOffset, -_doc->avgGlyphWidth, 3 * _doc->avgGlyphWidth)) {
        _log->debug(p) << BLUE << " + xOffset in indent tolerance (continues block)." << OFF << endl;
        return Trool::False;
      } else {
        _log->debug(p) << BLUE << " + xOffset not in indent tolerance (starts block)." << OFF << endl;
        return Trool::True;
      }
    } else if (hasPrevLineCapacity) {
      _log->debug(p) << BLUE << " + prev line has capacity (starts block)." << endl;
      return Trool::True;
    } else {
      _log->debug(p) << BLUE << " (continues block)." << endl;
      return Trool::False;
    }
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
Trool TextBlocksDetector::startsBlock_indent(const PdfTextLine* line, double percNoRightMarginLines,
      bool verbose) const {
  assert(line);

  // TODO
  bool isPrevIndented = math_utils::between(line->prevLine->leftMargin, _doc->avgGlyphWidth, 6 * _doc->avgGlyphWidth);
  bool isPrevMoreIndented = math_utils::larger(line->prevLine->leftMargin, 6 * _doc->avgGlyphWidth, 0);
  bool isCurrIndented = math_utils::between(line->leftMargin, _doc->avgGlyphWidth, 6 * _doc->avgGlyphWidth);
  bool isCurrMoreIndented = math_utils::larger(line->leftMargin, 6 * _doc->avgGlyphWidth, 0);
  double xOffset = element_utils::computeLeftXOffset(line->prevLine, line);
  bool hasPrevLineCapacity = text_line_utils::computeHasPrevLineCapacity(line);

  int p = line->position->pageNum;
  if (verbose) {
    _log->debug(p) << "Is line indented?" << endl;
    _log->debug(p) << " └─ prevLine.leftMargin:     " << line->prevLine->leftMargin << endl;
    _log->debug(p) << " └─ prevLine.isIndented:     " << isPrevIndented << endl;
    _log->debug(p) << " └─ prevLine.isMoreIndented: " << isPrevMoreIndented << endl;
    _log->debug(p) << " └─ prevLine.hasCapacity:    " << hasPrevLineCapacity << endl;
    _log->debug(p) << " └─ currLine.leftMargin:     " << line->leftMargin << endl;
    _log->debug(p) << " └─ currLine.isIndented:     " << isCurrIndented << endl;
    _log->debug(p) << " └─ currLine.isMoreIndented: " << isCurrMoreIndented << endl;
    _log->debug(p) << " └─ xOffset prevLine/currLine:  " << xOffset << endl;
  }

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

  if (percNoRightMarginLines >= 0.75 && larger(line->prevLine->rightMargin, 0, 5 * _doc->avgGlyphWidth)) {
    return Trool::True;
  }

  if (_percZeroRightMarginTextLines > 0.5 && larger(line->prevLine->rightMargin, 0, 10 * _doc->avgGlyphWidth)) {
    return Trool::True;
  }

  return Trool::None;
}
