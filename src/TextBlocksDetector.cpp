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
#include "./utils/Utils.h"

// =================================================================================================

// Some regular expressions to identify common prefixes of enumeration items.
std::regex item_anchor_regexes[] = {
  // A regex to find items starting with "• ", or "- ", or "+ ".
  std::regex("^(•|-|–|\\+)\\s+"),
  // A regex to find items starting with "I. ", "II. ", "III. ", "IV. ", etc.
  std::regex("^(X{0,1}(IX|IV|V?I{0,3}))\\.\\s+", std::regex_constants::icase),
  // A regex to find items starting with "(I)", "(II)", "(III)", "(IV) ", etc.
  std::regex("^\\((X{0,1}(IX|IV|V?I{0,3}))\\)\\s+", std::regex_constants::icase),
  // A regex to find items starting with "a. ", "b. ", "c. ", etc.
  std::regex("^([a-z])\\.\\s+"),
  // A regex to find items starting with "1. ", "2. ", "3. ", etc.
  std::regex("^([0-9]+)\\.\\s+"),
  // A regex to find items starting with "(A) ", "(1) ", "(C1) ", "[1] ", "[2] ", etc.
  std::regex("^(\\(|\\[)([a-z0-9][0-9]{0,2})(\\)|\\])\\s+", std::regex_constants::icase),
  // A regex to find items starting with "[Bu2] ", "[Ch] ", "[Enn2020] ", etc.
  std::regex("^(\\[)([A-Z][a-zA-Z0-9]{0,5})(\\])\\s+"),
  // A regex to find items starting with "A) " or "1) " or "a1) ".
  std::regex("^([a-z0-9][0-9]{0,1})\\)\\s+", std::regex_constants::icase)
};

// A regular expression to find footnotes starting with a superscript "1", "2", "a", "b".
std::regex isFootnoteMarkerSuperscriptRegex("^(\\d+|[a-z])");
// A regular expression to find footnotes starting with "*", "†", or "‡", or "?". These markers
// must not be superscripted.
std::regex isFootnoteMarkerRegex("^(\\*|∗|†|‡|\\?)");

std::unordered_set<std::string> specialFootnoteMarkers { "∗", "†", "‡" };

// _________________________________________________________________________________________________
TextBlocksDetector::TextBlocksDetector(PdfDocument* doc, bool debug, int debugPageFilter) {
  _doc = doc;
  _log = new Logger(debug ? DEBUG : INFO, debugPageFilter);

  _log->debug() << "=======================================" << std::endl;
  _log->debug() << "\033[1mDEBUG MODE | Detecting Text Blocks\033[0m" << std::endl;
  _log->debug() << " └─ debug page filter: " << debugPageFilter << std::endl;
}

// _________________________________________________________________________________________________
TextBlocksDetector::~TextBlocksDetector() {
  delete _log;
}

// _________________________________________________________________________________________________
void TextBlocksDetector::detect() {
  // Do nothing if no document is given.
  if (!_doc) {
    return;
  }

  // Do nothing if the document contains no pages.
  if (_doc->pages.size() == 0) {
    return;
  }

  // Compute some statistics needed for detecting text blocks.
  computeMostFreqTextLineDistance();
  // TODO: Check if this can be called after the computation of the preliminary text blocks:
  computeTextLineIndentHierarchies();

  // Detect the text blocks in two steps.
  // In the first step, split the text lines of each page into (preliminary) text blocks using
  // rules regarding, for example, the vertical distances between the text lines. This step is
  // purposed to get text blocks that allow to compute the text line indentations more precisely
  // (by computing the horizontal gap between the text lines and the text block boundaries).
  // NOTE: Initially, we computed the text line indentations by computing the gap between the text
  // lines and the *segment* boundaries. This approach often resulted in inaccurately computed text
  // line indentations, since the segments were often broader than expected, because of text parts
  // that do not share the same alignment than the body text paragraphs (like page headers or page
  // footers). A frequent consequence is that the text lines of the body text paragraphs are not
  // justified with the segment boundaries, but are positioned somewhere in the middle of the
  // segments instead. In the second step, the preliminary text blocks are split further using
  // further rules regarding, for example, the computed text line indentations or the prefixes of
  // the text lines.
  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      std::vector<PdfTextLine*> currBlockLines;
      for (size_t i = 0; i < segment->lines.size(); i++) {
        PdfTextLine* prevLine = i > 0 ? segment->lines[i - 1] : nullptr;
        PdfTextLine* currLine = segment->lines[i];
        PdfTextLine* nextLine = i < segment->lines.size() - 1 ? segment->lines[i + 1] : nullptr;

        if (startsPreliminaryTextBlock(prevLine, currLine, nextLine) && !currBlockLines.empty()) {
          createTextBlock(currBlockLines, &page->blocks);
          currBlockLines.clear();
        }

        currBlockLines.push_back(currLine);
      }
      if (!currBlockLines.empty()) { createTextBlock(currBlockLines, &page->blocks); }
    }
  }

  computeTextLineMargins();

  for (auto* page : _doc->pages) {
    // Compute the potential footnote markers.
    std::unordered_set<std::string> potentialFootnoteMarkers;
    computePotentialFootnoteMarkers(page, &potentialFootnoteMarkers);

    std::vector<PdfTextBlock*> textBlocks;
    for (auto* block : page->blocks) {
      double hangIndent = computeHangingIndent(block);
      std::vector<PdfTextLine*> currBlockLines;
      for (size_t i = 0; i < block->lines.size(); i++) {
        PdfTextLine* prevLine = i > 0 ? block->lines[i - 1] : nullptr;
        PdfTextLine* currLine = block->lines[i];
        PdfTextLine* nextLine = i < block->lines.size() - 1 ? block->lines[i + 1] : nullptr;

        if (startsTextBlock(prevLine, currLine, nextLine, &potentialFootnoteMarkers, hangIndent)
              && !currBlockLines.empty()) {
          createTextBlock(currBlockLines, &textBlocks);
          currBlockLines.clear();
        }

        currBlockLines.push_back(currLine);
      }
      if (!currBlockLines.empty()) { createTextBlock(currBlockLines, &textBlocks); }
    }
    page->blocks = textBlocks;
  }
}

// _________________________________________________________________________________________________
bool TextBlocksDetector::startsPreliminaryTextBlock(const PdfTextLine* prevLine,
      const PdfTextLine* currLine, const PdfTextLine* nextLine) const {
  if (!currLine) {
    return false;
  }

  int p = currLine->position->pageNum;
  _log->debug(p) << "= (pre) =================" << std::endl;
  _log->debug(p) << "\033[1mLine: page:\033[0m " << currLine->position->pageNum
      << "; \033[1mleftX:\033[0m " << currLine->position->leftX
      << "; \033[1mupperY:\033[0m " << currLine->position->upperY
      << "; \033[1mrightX:\033[0m " << currLine->position->rightX
      << "; \033[1mlowerY:\033[0m " << currLine->position->lowerY
      << "; \033[1mtext:\033[0m \"" << currLine->text << "\"" << std::endl;
  if (currLine->position->rotation != 0) {
    _log->debug(p) << "\033[1mrot:\033[0m " << currLine->position->rotation
        << "; \033[1mrotLeftX:\033[0m " << currLine->position->getRotLeftX()
        << "; \033[1mrotUpperY:\033[0m " << currLine->position->getRotUpperY()
        << "; \033[1mrotRightX:\033[0m " << currLine->position->getRotRightX()
        << "; \033[1mrotLowerY:\033[0m " << currLine->position->getRotLowerY() << std::endl;
  }
  _log->debug(p) << "---------------" << std::endl;

  if (!prevLine) {
    _log->debug(p) << "\033[1mstarts new block (no previous line).\033[0m" << std::endl;
    return true;
  }

  // The line does *not* start a new block if the line and prev line are part of the same figure.
  _log->debug(p) << "Checking overlappings of figures..." << std::endl;
  PdfFigure* isPrevLinePartOfFigure = isPartOfFigure(prevLine);
  PdfFigure* isCurrLinePartOfFigure = isPartOfFigure(currLine);
  _log->debug(p) << " └─ prevLine.isPartOfFigure: " << isPrevLinePartOfFigure << std::endl;
  _log->debug(p) << " └─ currLine.isPartOfFigure: " << isCurrLinePartOfFigure << std::endl;
  if (isCurrLinePartOfFigure && isCurrLinePartOfFigure == isPrevLinePartOfFigure) {
    _log->debug(p) << "\033[1mcontinues block (part of the same figure).\033[0m" << std::endl;
    return false;
  }

  // The line starts a new block if it has another rotation than the previous line.
  _log->debug(p) << "Checking rotations..." << std::endl;
  _log->debug(p) << " └─ prevLine.rotation: " << prevLine->position->rotation << std::endl;
  _log->debug(p) << " └─ currLine.rotation: " << currLine->position->rotation << std::endl;
  if (prevLine->position->rotation != currLine->position->rotation) {
    _log->debug(p) << "\033[1mstarts new block (rotations differ).\033[0m" << std::endl;
    return true;
  }

  // The line starts a new block if it has another writing mode than the previous line.
  _log->debug(p) << "Checking writing modes..." << std::endl;
  _log->debug(p) << " └─ prevLine.wMode: " << prevLine->position->wMode << std::endl;
  _log->debug(p) << " └─ currLine.wMode: " << currLine->position->wMode << std::endl;
  if (prevLine->position->wMode != currLine->position->wMode) {
    _log->debug(p) << "\033[1mstarts new block (writing modes differ).\033[0m" << std::endl;
    return true;
  }

  // The line starts a new block if the difference between neither the most frequent font sizes nor
  // the maximum font sizes of the previous text line and of the current text line are equal, under
  // consideration of a small threshold. This rule exists to split e.g., headings (which usually
  // have a larger font size) from the body text. The first condition exists to not split text
  // lines when they contain some words with larger font sizes (e.g., in a caption, the "Figure X:"
  // parts is likely to have a larger font size than the rest of the caption). The second condition
  // exists to not split text lines with many small characters (which is particularly often the
  // case when the text line contains an inline formula).
  _log->debug(p) << "Checking font sizes..." << std::endl;
  _log->debug(p) << " └─ prevLine.mostFreqFontSize: " << prevLine->fontSize << std::endl;
  _log->debug(p) << " └─ currLine.mostFreqFontSize: " << currLine->fontSize << std::endl;
  _log->debug(p) << " └─ prevLine.maxFontSize: " << prevLine->maxFontSize << std::endl;
  _log->debug(p) << " └─ currLine.maxFontSize: " << currLine->maxFontSize << std::endl;
  if (!equal(prevLine->fontSize, currLine->fontSize, 1) &&
        !equal(prevLine->maxFontSize, currLine->maxFontSize, 1)) {
    _log->debug(p) << "\033[1mstarts new block (font sizes differ).\033[0m" << std::endl;
    return true;
  }

  _log->debug(p) << "Checking line distances..." << std::endl;
  // Compute the expected line distance.
  double fontSize = round(currLine->fontSize, 1);
  double expectedLineDistance = 0;
  if (_mostFreqLineDistancePerFontSize.count(fontSize) > 0) {
    double eld = _mostFreqLineDistancePerFontSize.at(fontSize);
    expectedLineDistance = std::max(expectedLineDistance, eld);
    _log->debug(p) << " └─ expected line distance regarding fontsize: " << eld << std::endl;
  }
  expectedLineDistance = std::max(expectedLineDistance, _mostFreqLineDistance);
  _log->debug(p) << " └─ expected line distance reg. doc: " << _mostFreqLineDistance << std::endl;
  _log->debug(p) << " └─ expected line distance: " << expectedLineDistance << std::endl;

  // Compute the actual line distance.
  double actualLineDistance = 0;
  switch(currLine->position->rotation) {
    case 0:
    case 1:
      actualLineDistance = currLine->position->getRotUpperY() - prevLine->position->getRotLowerY();
      break;
    case 2:
    case 3:
      actualLineDistance = prevLine->position->getRotLowerY() - currLine->position->getRotUpperY();
      break;
  }
  _log->debug(p) << " └─ actual line distance: " << actualLineDistance << std::endl;

  // The line does *not* start a new block if the actual line distance is <= 0.
  if (equalOrSmaller(actualLineDistance, 0)) {
    _log->debug(p) << "\033[1mcontinues block (actual line distance <= 0).\033[0m" << std::endl;
    return false;
  }

  // The line starts a new block if the actual line distance is larger than the expected line
  // distance, under consideration of a small threshold.
  if (larger(actualLineDistance, expectedLineDistance, std::max(1.0, 0.1 * expectedLineDistance))) {
    _log->debug(p) << "\033[1mstarts new block (actual line distance > expected line distance)."
        << "\033[0m" << std::endl;
    return true;
  }

  _log->debug(p) << "Checking for column break..." << std::endl;
  // Compute the horizontal offset between the previous line and the current line.
  double xOffset = 0;
  switch(currLine->position->rotation) {
    case 0:
    case 1:
      xOffset = currLine->position->getRotLeftX() - prevLine->position->getRotLeftX();
      break;
    case 2:
    case 3:
      xOffset = prevLine->position->getRotLeftX() - currLine->position->getRotLeftX();
      break;
  }
  _log->debug(p) << " └─ xOffset: " << xOffset << std::endl;

  // Compute the vertical offset between the previous line and the current line.
  double yOffset = 0;
  switch(currLine->position->rotation) {
    case 0:
    case 1:
      yOffset = currLine->position->getRotLowerY() - prevLine->position->getRotLowerY();
      break;
    case 2:
    case 3:
      xOffset = prevLine->position->getRotLowerY() - currLine->position->getRotLowerY();
      break;
  }
  _log->debug(p) << " └─ yOffset: " << yOffset << std::endl;

  // The line starts a new block if it is positioned in the north-east of the previous line.
  if (xOffset > 0 && yOffset < -2 * prevLine->position->getHeight()) {
    _log->debug(p) << "\033[1mstarts new block (assuming a column break).\033[0m" << std::endl;
    return true;
  }

  _log->debug(p) << "\033[1mcontinues block (no rule applied).\033[0m" << std::endl;
  return false;
}

// _________________________________________________________________________________________________
bool TextBlocksDetector::startsTextBlock(const PdfTextLine* prevLine, const PdfTextLine* currLine,
      const PdfTextLine* nextLine, const std::unordered_set<std::string>* potentialFootnoteMarkers,
      double hangingIndent) {
  if (!currLine) {
    return false;
  }

  int p = currLine->position->pageNum;
  _log->debug(p) << "=========================" << std::endl;
  _log->debug(p) << "\033[1mLine: page:\033[0m " << currLine->position->pageNum
      << "; \033[1mleftX:\033[0m " << currLine->position->leftX
      << "; \033[1mupperY:\033[0m " << currLine->position->upperY
      << "; \033[1mrightX:\033[0m " << currLine->position->rightX
      << "; \033[1mlowerY:\033[0m " << currLine->position->lowerY
      << "; \033[1mhangingIndent:\033[0m " << hangingIndent
      << "; \033[1mtext:\033[0m \"" << currLine->text << "\"" << std::endl;
  if (currLine->position->rotation != 0) {
    _log->debug(p) << "\033[1mrot:\033[0m " << currLine->position->rotation
        << "; \033[1mrotUpperY:\033[0m " << currLine->position->getRotUpperY()
        << "; \033[1mrotRightX:\033[0m " << currLine->position->getRotRightX()
        << "; \033[1mrotLeftX:\033[0m " << currLine->position->getRotLeftX()
        << "; \033[1mrotLowerY:\033[0m " << currLine->position->getRotLowerY() << std::endl;
  }
  _log->debug(p) << "---------------" << std::endl;

  if (!prevLine) {
    _log->debug(p) << "\033[1mstarts new block (no previous line).\033[0m" << std::endl;
    return true;
  }

  _log->debug(p) << "Checking right margin..." << std::endl;
  _log->debug(p) << " └─ \% lines, no right margin: " << _percZeroRightMarginTextLines << std::endl;
  // The line starts a new block if the computed percentage of text lines with right margin == 0
  // is larger than a given threshold, and if the right margin of the previous line is > 0. This
  // rule exists, to identify the last line of a paragraph, which is often shorter (that is: right
  // margin > 0) than the rest of the text lines. The first condition should ensure that the text
  // in the document is justified.
  if (_percZeroRightMarginTextLines > 0.5 && larger(prevLine->rightMargin, 0, 10 * _doc->avgGlyphWidth)) {
    _log->debug(p) << "\033[1mstarts new block (prevLine.rightMargin > 0).\033[0m" << std::endl;
    return true;
  }

  // The line starts a new block if it is the first line of an enumeration item.
  // The line does not start a new block if it is a continuation of an enumeration item.
  _log->debug(p) << "Checking for enumeration..." << std::endl;
  std::string pStr = currLine->parentTextLine ? currLine->parentTextLine->text : "-";
  std::string psStr = currLine->prevSiblingTextLine ? currLine->prevSiblingTextLine->text : "-";
  std::string nsStr = currLine->nextSiblingTextLine ? currLine->nextSiblingTextLine->text : "-";
  _log->debug(p) << " └─ parent:   " << pStr << std::endl;
  _log->debug(p) << " └─ prev sibling: " << psStr << std::endl;
  _log->debug(p) << " └─ next sibling: " << nsStr << std::endl;
  bool firstLineOfItemPrev = isFirstLineOfItem(prevLine);
  bool contLineOfItemPrev = isContinuationLineOfItem(prevLine);
  bool firstLineOfItem = isFirstLineOfItem(currLine);
  bool contLineOfItem = isContinuationLineOfItem(currLine);
  _log->debug(p) << " └─ currLine.isFirstLineOfItem: " << firstLineOfItem << std::endl;
  _log->debug(p) << " └─ currLine.isContinuationOfItem: " << contLineOfItem << std::endl;
  if (firstLineOfItem) {
    _log->debug(p) << "\033[1mstarts new block (first line of item).\033[0m" << std::endl;
    return true;
  }
  if (contLineOfItem) {
    _log->debug(p) << "\033[1mcontinues block (continuation of item).\033[0m" << std::endl;
    return false;
  }
  if ((firstLineOfItemPrev || contLineOfItemPrev) && !firstLineOfItem && !contLineOfItem) {
    _log->debug(p) << "\033[1mstarts new block (prev line is enumeration, but curr line not.\033[0m"
        << std::endl;
    return true;
  }

  // The line starts a new text block if it is the first line of a footnote.
  // The line does not start a new block if it is a continuation of a footnote.
  _log->debug(p) << "Checking for footnote..." << std::endl;
  bool firstLineOfFootnote = isFirstLineOfFootnote(currLine, potentialFootnoteMarkers);
  bool contLineOfFootnote = isContinuationLineOfFootnote(currLine, potentialFootnoteMarkers);
  _log->debug(p) << " └─ currLine.isFirstLineOfFootnote: " << firstLineOfFootnote << std::endl;
  _log->debug(p) << " └─ currLine.isContinuationOfFootnote: " << contLineOfFootnote << std::endl;
  if (firstLineOfFootnote) {
    _log->debug(p) << "\033[1mstarts new block (first line of footnote).\033[0m" << std::endl;
    return true;
  }
  if (contLineOfFootnote) {
    _log->debug(p) << "\033[1mcontinues block (continuation of footnote).\033[0m" << std::endl;
    return false;
  }

  // The line does not start a new block if the previous line and the current line are emphasized,
  // and if both lines exhibits the same font and the same font size. This rule exists to not split
  // titles and headings, which are often centered (which means that the left margin of the text
  // lines are > 0), in two parts in the next rule (which assumes the start of a new block if the
  // left margin of the current line is > 0).
  _log->debug(p) << "Checking for emphasis..." << std::endl;
  bool isPrevLineEmphasized = isTextLineEmphasized(prevLine);
  bool isCurrLineEmphasized = isTextLineEmphasized(currLine);
  _log->debug(p) << " └─ prevLine.isEmphasized: " << isPrevLineEmphasized << std::endl;
  _log->debug(p) << " └─ currLine.isEmphasized: " << isCurrLineEmphasized << std::endl;
  _log->debug(p) << " └─ prevLine.fontName: " << prevLine->fontName << std::endl;
  _log->debug(p) << " └─ currLine.fontName: " << currLine->fontName << std::endl;
  _log->debug(p) << " └─ prevLine.fontSize: " << prevLine->fontSize << std::endl;
  _log->debug(p) << " └─ currLine.fontSize: " << currLine->fontSize << std::endl;
  if (isPrevLineEmphasized && isCurrLineEmphasized && prevLine->fontName == currLine->fontName &&
        equal(prevLine->fontSize, currLine->fontSize, 0.1)) {
    _log->debug(p) << "\033[1mcontinues block (same emphasized font style).\033[0m" << std::endl;
    return false;
  }

  _log->debug(p) << "Checking left margin..." << std::endl;
  _log->debug(p) << " └─ most freq. left margin: " << _mostFreqLineLeftMargin << std::endl;
  _log->debug(p) << " └─ doc.avgGlyphWidth:      " << _doc->avgGlyphWidth << std::endl;
  _log->debug(p) << " └─ prevLine.leftMargin:    " << prevLine->leftMargin << std::endl;
  _log->debug(p) << " └─ currLine.leftMargin:    " << currLine->leftMargin << std::endl;
  _log->debug(p) << " └─ prevLine.rightMargin:      " << prevLine->rightMargin << std::endl;
  _log->debug(p) << " └─ currLine.rightMargin:      " << currLine->rightMargin << std::endl;

  // The line does not start a new text block if the current line and the previous line is centered,
  // that is: when leftMargin == rightMargin.
  bool prevLineCentered = equal(prevLine->leftMargin, prevLine->rightMargin, 0.1);
  bool currLineCentered = equal(currLine->leftMargin, currLine->rightMargin, 0.1);
  bool prevLineLargeLeftMargin = larger(prevLine->leftMargin, 0, _doc->avgGlyphWidth);
  bool currLineLargeLeftMargin = larger(currLine->leftMargin, 0, _doc->avgGlyphWidth);
  if (prevLineCentered && currLineCentered && (prevLineLargeLeftMargin || currLineLargeLeftMargin)) {
    _log->debug(p) << "\033[1mcontinues block (prev and curr line centered).\033[0m" << std::endl;
    return false;
  }

  // The line starts a new text block if (1) the left margin of the current line is == 0, and (2)
  // the left margin of the previous line is larger than the most frequent left margin. This rule
  // exists to split body text paragraph following a display formula apart. The second condition
  // exists to not detect the second line of a body text paragraph from the (indented) first line.
  if (larger(prevLine->leftMargin - currLine->leftMargin, 6 * _doc->avgGlyphWidth, 0)) {
  // if (equal(currLine->leftMargin, 0, 2 * _doc->avgGlyphWidth) &&
  //       equalOrLarger(prevLine->leftMargin, 6 * _doc->avgGlyphWidth, 0)) {
    _log->debug(p) << "\033[1mstarts new block (line not indented, but previous line).\033[0m"
        << std::endl;
    return true;
  }

  if (hangingIndent > 0) {
    bool notIndented = smaller(currLine->leftMargin, hangingIndent, _doc->avgGlyphWidth);
    if (notIndented) {
      _log->debug(p) << "\033[1mstarts block (hanging indent & no indent).\033[0m" << std::endl;
      return true;
    }
    bool indented = equal(currLine->leftMargin, hangingIndent, _doc->avgGlyphWidth);
    if (indented) {
      _log->debug(p) << "\033[1mcontinues block (hanging indent & indent).\033[0m" << std::endl;
      return false;
    }
  } else {
    bool notIndented = smaller(currLine->leftMargin, _doc->avgGlyphWidth, 0);
    if (notIndented) {
      _log->debug(p) << "\033[1mcontinues block (no hanging indent & no indent).\033[0m" << std::endl;
      return false;
    }
    bool indented = smaller(currLine->leftMargin, 6 * _doc->avgGlyphWidth, 0);
    if (indented) {
      _log->debug(p) << "\033[1mstarts block (no hanging indent & indent).\033[0m" << std::endl;
      return true;
    }
  }

  // The line starts a new block if its left margin is > 0. This rule exists, because the first
  // line of a paragraph is often intended by a certain amount. It also exists to detect display
  // formulas, which are often centered (and thus, have a left margin > 0), as the start of a new
  // block.
  // if (larger(currLine->leftMargin, 0, _doc->avgGlyphWidth)) {
  if (equalOrLarger(currLine->leftMargin, 6 * _doc->avgGlyphWidth, 0)) {
    _log->debug(p) << "\033[1mstarts new block (left margin > mostFreqLineMargin).\033[0m" << std::endl;
    return true;
  }

  _log->debug(p) << "\033[1mcontinues block (no rule applied).\033[0m" << std::endl;
  return false;
}

// =================================================================================================
// Methods to compute some statistics.

// _________________________________________________________________________________________________
void TextBlocksDetector::computeMostFreqTextLineDistance() {
  std::unordered_map<double, int> lineDistanceCounts;
  std::unordered_map<double, std::unordered_map<double, int>> lineDistanceCountsPerFontSize;

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
        lineDistance = std::max(0.0, lineDistance);
        lineDistanceCounts[lineDistance]++;

        // For computing line distances per font size, ignore the lines if their font sizes differ.
        double prevFontSize = round(prevLine->fontSize, 1);
        double currFontSize = round(currLine->fontSize, 1);
        if (equal(prevFontSize, currFontSize, 0.01)) {
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
  std::unordered_map<double, int> mostFreqLineDistanceCountPerFontSize;
  for (const auto& doubleMapPair : lineDistanceCountsPerFontSize) {
    const double fontSize = doubleMapPair.first;
    const std::unordered_map<double, int>& lineDistanceFreqs = doubleMapPair.second;
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
  std::unordered_map<double, int> leftMarginFreqs;
  std::unordered_map<double, int> rightMarginFreqs;
  double numZeroRightMarginTextLines = 0;
  double numBodyTextLines = 0;

  for (auto* page : _doc->pages) {
    for (auto* block : page->blocks) {
      for (size_t i = 0; i < block->lines.size(); i++) {
        PdfTextLine* prevLine = i > 0 ? block->lines.at(i - 1) : nullptr;
        PdfTextLine* currLine = block->lines.at(i);
        PdfTextLine* nextLine = i < block->lines.size() - 1 ? block->lines.at(i + 1) : nullptr;

        currLine->leftMargin = round(currLine->position->getRotLeftX() - block->position->getRotLeftX());
        currLine->rightMargin = round(block->position->getRotRightX() - currLine->position->getRotRightX());

        // Make sure that the indent is measured only for lines from body text paragraphs.
        // Reason: Lines from the bibliography could have other indents.
        if (prevLine) {
          if (prevLine->fontName != _doc->mostFreqFontName ||
                !equal(prevLine->fontSize, _doc->mostFreqFontSize, 1)) {
            continue;
          }
        }
        if (currLine) {
          if (currLine->fontName != _doc->mostFreqFontName ||
                !equal(currLine->fontSize, _doc->mostFreqFontSize, 1)) {
            continue;
          }
        }
        if (nextLine) {
          if (nextLine->fontName != _doc->mostFreqFontName ||
                !equal(nextLine->fontSize, _doc->mostFreqFontSize, 1)) {
            continue;
          }
        }

        // Count the number of justified text lines.
        if (equal(currLine->rightMargin, 0, _doc->avgGlyphWidth)) {
          numZeroRightMarginTextLines++;
        }
        numBodyTextLines++;

        double prevLineLeftMargin = 0.0;
        if (prevLine) {
          prevLineLeftMargin = prevLine->leftMargin;
        }

        double nextLineLeftMargin = 0.0;
        if (nextLine) {
          nextLineLeftMargin = round(nextLine->position->getRotLeftX() - block->position->getRotLeftX());
        }

        if (!equal(prevLineLeftMargin, 0, _doc->avgGlyphWidth)) {
          continue;
        }

        if (!equal(nextLineLeftMargin, 0, _doc->avgGlyphWidth)) {
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

// =================================================================================================
// Methods to compute text line properties.

// _________________________________________________________________________________________________
PdfFigure* TextBlocksDetector::isPartOfFigure(const PdfTextLine* line) const {
  PdfPage* page = _doc->pages[line->position->pageNum - 1];
  for (auto* figure : page->figures) {
    std::pair<double, double> xOverlapRatios = computeXOverlapRatios(line, figure);
    std::pair<double, double> yOverlapRatios = computeYOverlapRatios(line, figure);

    if (xOverlapRatios.first > 0.5 && yOverlapRatios.first > 0.5) {
      return figure;
    }
  }
  return nullptr;
}

// _________________________________________________________________________________________________
bool TextBlocksDetector::isFirstLineOfItem(const PdfTextLine* line) const {
  if (!line) {
    return false;
  }

  if (line->text.size() == 0) {
    return false;
  }

  std::smatch m1;
  std::smatch m2;
  std::smatch m3;

  for (const auto& regex : item_anchor_regexes) {
    bool startsWithAnchor = std::regex_search(line->text, m1, regex);
    bool prevSiblingStartsWithAnchor = line->prevSiblingTextLine
      && std::regex_search(line->prevSiblingTextLine->text, m2, regex);
    bool nextSiblingStartsWithAnchor = line->nextSiblingTextLine
      && std::regex_search(line->nextSiblingTextLine->text, m3, regex);

    if (startsWithAnchor && (prevSiblingStartsWithAnchor || nextSiblingStartsWithAnchor)) {
      return true;
    }
  }

  return false;
}

// _________________________________________________________________________________________________
bool TextBlocksDetector::isContinuationLineOfItem(const PdfTextLine* line) const {
  return line->parentTextLine && isFirstLineOfItem(line->parentTextLine);
}

// _________________________________________________________________________________________________
bool TextBlocksDetector::isFirstLineOfFootnote(const PdfTextLine* line,
      const std::unordered_set<std::string>* potentialFootnoteMarkers) const {
  if (!line) {
    return false;
  }

  if (line->text.size() == 0) {
    return false;
  }

  // if (!smaller(line->fontSize, _doc->mostFreqFontSize, .9)) { // TODO
  //   return false;
  // }

  std::string superScriptPrefix;
  PdfWord* firstWord = line->words[0];
  for (auto* glyph : firstWord->glyphs) {
    if (!glyph->isSuperscript) {
      break;
    }
    superScriptPrefix += glyph->text;
  }

  if (potentialFootnoteMarkers->count(superScriptPrefix) > 0) {
    return true;
  }

  // if (std::regex_search(firstWord->text, m, isFootnoteMarkerRegex)) {
  //   return true;
  // }

  return false;
}

// _________________________________________________________________________________________________
bool TextBlocksDetector::isContinuationLineOfFootnote(const PdfTextLine* line,
      const std::unordered_set<std::string>* potentialFootnoteMarkers) const {
  return line->parentTextLine && isFirstLineOfFootnote(line->parentTextLine, potentialFootnoteMarkers);
}

// _________________________________________________________________________________________________
bool TextBlocksDetector::isTextBlockEmphasized(const std::vector<PdfTextLine*>& lines) {
  for (size_t i = 0; i < lines.size(); i++) {
    if (!isTextLineEmphasized(lines[i])) {
      return false;
    }
  }
  return true;
}

// _________________________________________________________________________________________________
bool TextBlocksDetector::isTextLineEmphasized(const PdfTextLine* line) {
  const PdfFontInfo* docFontInfo = _doc->fontInfos.at(_doc->mostFreqFontName);
  const PdfFontInfo* lineFontInfo = _doc->fontInfos.at(line->fontName);

  // The line is emphasized if ...

  // ... its font size is significantly larger than the most frequent font size in the document.
  if (line->fontSize - _doc->mostFreqFontSize > 0.5) {
    return true;
  }

  // ... its font weight is larger than the most frequent font weight.
  if (line->fontSize - _doc->mostFreqFontSize >= -1
      && lineFontInfo->weight > docFontInfo->weight) {
    return true;
  }

  // ... the line is printed in italics.
  if (line->fontSize - _doc->mostFreqFontSize >= -1 && lineFontInfo->isItalic) {
    return true;
  }

  // ... the line contains at least one alphabetic characters and all alphabetic characters are
  // upper case.
  bool containsAlpha = false;
  bool isAllAlphaUpper = true;
  for (size_t j = 0; j < line->text.size(); j++) {
    if (isalpha(line->text[j])) {
      containsAlpha = true;
      if (islower(line->text[j])) {
        isAllAlphaUpper = false;
        break;
      }
    }
  }
  if (containsAlpha && isAllAlphaUpper) {
    return true;
  }

  return false;
}

// _________________________________________________________________________________________________
void TextBlocksDetector::computeTextLineIndentHierarchies() {
  for (auto* page : _doc->pages) {
    std::stack<PdfTextLine*> lineStack;
    for (auto* segment : page->segments) {
      for (auto* line : segment->lines) {
        // Compute the indentation, relatively to the segment boundaries.
        line->leftMargin = round(line->position->getRotLeftX() - segment->position->getRotLeftX(), 1);

        while (!lineStack.empty()) {
          if (!larger(lineStack.top()->leftMargin, line->leftMargin, _doc->avgGlyphWidth)) {
            break;
          }
          lineStack.pop();
        }

        if (lineStack.empty()) {
          lineStack.push(line);
          continue;
        }

        std::pair<double, double> xOverlapRatios = computeXOverlapRatios(lineStack.top(), line);
        double maxXOVerlapRatio = std::max(xOverlapRatios.first, xOverlapRatios.second);
        if (maxXOVerlapRatio > 0) {
          if (equal(lineStack.top()->leftMargin, line->leftMargin, _doc->avgGlyphWidth)) {
            lineStack.top()->nextSiblingTextLine = line;
            line->prevSiblingTextLine = lineStack.top();
            line->parentTextLine = lineStack.top()->parentTextLine;
            lineStack.pop();
            lineStack.push(line);
            continue;
          }

          if (smaller(lineStack.top()->leftMargin, line->leftMargin, _doc->avgGlyphWidth)) {
            line->parentTextLine = lineStack.top();

            lineStack.push(line);
            continue;
          }
        }
      }
    }
  }
}

// _________________________________________________________________________________________________
double TextBlocksDetector::computeHangingIndent(const PdfTextBlock* block) const {
  // Compute the most frequent left margin > 0.
  int numLines = 0;
  int numLeftMarginLines = 0;
  std::unordered_map<double, int> leftMarginCounts;
  for (const auto* line : block->lines) {
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

  // Abort if there are no more than one line.
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
  bool isAllOtherLinesIndented = true;
  int numLowercasedNotIndentedLines = 0;
  int numLowercasedIndentedLines = 0;
  int numNotIndentedLines = 0;
  int numIndentedLines = 0;

  for (size_t i = 0; i < block->lines.size(); i++) {
    const PdfTextLine* line = block->lines[i];

    bool isNotIndented = equal(line->leftMargin, 0, _doc->avgGlyphWidth);
    bool isIndented = equal(line->leftMargin, mostFreqLeftMargin, _doc->avgGlyphWidth);
    bool isLower = !line->text.empty() && islower(line->text[0]);

    if (i == 0) {
      isFirstLineIndented = isIndented;
    } else {
      isAllOtherLinesIndented &= isIndented;
    }

    if (isLower && isNotIndented) {
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

  // The block is in hanging indent format if the first line is not indented, but all other lines.
  // This should identify single enumeration items, e.g., in the format:
  // Dynamics: The low energy behavior of
  //    a physical system depends on its
  //    dynamics.
  if (!isFirstLineIndented && isAllOtherLinesIndented) {
    return mostFreqLeftMargin;
  }

  // The block is *not* in hanging indent format if there is at least one non-indented line that
  // starts with a lowercase character.
  if (numLowercasedNotIndentedLines > 0) {
    return 0.0;
  }

  // The block is in hanging indent format if all non-indented lines start with an uppercase
  // character and if the number of non-indented lines exceed a certain threshold.
  if (numNotIndentedLines >= 10 && numLowercasedNotIndentedLines == 0) {
    return mostFreqLeftMargin;
  }

  // The block is in hanging indent format if there is at least one indented line that starts with
  // a lowercase character.
  if (numLowercasedIndentedLines > 0) {
    return mostFreqLeftMargin;
  }

  return 0.0;
}

// _________________________________________________________________________________________________
void TextBlocksDetector::computePotentialFootnoteMarkers(const PdfPage* page,
      std::unordered_set<std::string>* footnoteMarkers) const {
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
        std::string footnoteMarker;
        for (const auto* glyph : word->glyphs) {
          if (!nonSubSuperscriptSeen && !glyph->isSubscript && !glyph->isSuperscript) {
            nonSubSuperscriptSeen = true;
            continue;
          }
          if (!nonSubSuperscriptSeen) {
            continue;
          }

          bool isMarker = glyph->isSuperscript && !glyph->text.empty() && isalnum(glyph->text[0]);
          isMarker |= !glyph->text.empty() && specialFootnoteMarkers.count(glyph->text) > 0;

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
void TextBlocksDetector::createTextBlock(const std::vector<PdfTextLine*>& lines,
    std::vector<PdfTextBlock*>* blocks) {
  // Do nothing if no words are given.
  if (lines.size() == 0) {
    return;
  }

  PdfTextBlock* block = new PdfTextBlock();
  block->id = createRandomString(8, "tb-");

  std::unordered_map<std::string, int> fontNameFreqs;
  std::unordered_map<double, int> fontSizeFreqs;
  for (const auto* line : lines) {
    double lineMinX = std::min(line->position->leftX, line->position->rightX);
    double lineMinY = std::min(line->position->upperY, line->position->lowerY);
    double lineMaxX = std::max(line->position->leftX, line->position->rightX);
    double lineMaxY = std::max(line->position->upperY, line->position->lowerY);

    // Update the x,y-coordinates.
    block->position->leftX = std::min(block->position->leftX, lineMinX);
    block->position->upperY = std::min(block->position->upperY, lineMinY);
    block->position->rightX = std::max(block->position->rightX, lineMaxX);
    block->position->lowerY = std::max(block->position->lowerY, lineMaxY);

    // Count the font names and font sizes, for computing the most frequent font name / font size.
    fontNameFreqs[line->fontName]++;
    fontSizeFreqs[line->fontSize]++;
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

  block->isEmphasized = isTextBlockEmphasized(lines);

  block->lines = lines;

  // Set the rank.
  block->rank = blocks->size();

  blocks->push_back(block);
}
