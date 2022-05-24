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
  std::regex("^([a-z0-9][0-9]{0,1})\\)\\s+", std::regex_constants::icase),
  // A regex to find items starting with "PACS" (1011.5073).
  std::regex("^PACS\\s+", std::regex_constants::icase)
};

const std::string FOOTNOTE_LABEL_ALPHABET = "*∗†‡?";
const std::string ITEM_LABEL_SUPERSCRIPT_ALPHABET = "abcdefghijklmnopqrstuvwxyz01234567890()";
const std::string FORMULA_ID_ALPHABET = "=+";

const std::unordered_set<std::string> lastNamePrefixes = { "van", "von", "de" };

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
  if (!_doc) {
    return;
  }

  if (_doc->pages.empty()) {
    return;
  }

  // Compute some statistics needed for detecting text blocks.
  // TODO: Compute this in the statistician.
  computeMostFreqTextLineDistance();

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
  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      std::vector<PdfTextLine*> currBlockLines;
      for (size_t i = 0; i < segment->lines.size(); i++) {
        PdfTextLine* prevLine = i > 0 ? segment->lines[i - 1] : nullptr;
        PdfTextLine* currLine = segment->lines[i];
        PdfTextLine* nextLine = i < segment->lines.size() - 1 ? segment->lines[i + 1] : nullptr;

        if (startsPreliminaryTextBlock(prevLine, currLine, nextLine)) {
          if (!currBlockLines.empty()) {
            createTextBlock(currBlockLines, &segment->blocks);
            currBlockLines.clear();
          }
        }

        currBlockLines.push_back(currLine);
      }
      if (!currBlockLines.empty()) {
        createTextBlock(currBlockLines, &segment->blocks);
      }
    }
  }

  computeTextBlockTrimBoxes();
  computeTextLineIndentHierarchies();
  computeTextLineMargins();
  computeHangingIndents();

  for (auto* page : _doc->pages) {
    // Compute the potential footnote markers.
    std::unordered_set<std::string> potentialFootnoteMarkers;
    computePotentialFootnoteMarkers(page, &potentialFootnoteMarkers);

    for (auto* segment : page->segments) {
      for (auto* block : segment->blocks) {
        double percNoRightMarginLines = computePercentageNoRightMarginLines(block);
        bool isCentered = computeIsCentered(block);

        std::vector<PdfTextLine*> currBlockLines;
        for (size_t i = 0; i < block->lines.size(); i++) {
          PdfTextLine* prevLine = i > 0 ? block->lines[i - 1] : nullptr;
          PdfTextLine* currLine = block->lines[i];
          PdfTextLine* nextLine = i < block->lines.size() - 1 ? block->lines[i + 1] : nullptr;

          if (startsTextBlock(prevLine, currLine, nextLine, &potentialFootnoteMarkers,
                block->hangingIndent, percNoRightMarginLines, isCentered) && !currBlockLines.empty()) {
            createTextBlock(currBlockLines, &page->blocks);
            currBlockLines.clear();
          }

          currBlockLines.push_back(currLine);
        }
        if (!currBlockLines.empty()) {
          createTextBlock(currBlockLines, &page->blocks);
        }
      }
    }
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

  // ------------
  // The line starts a new block if there is no previous line.
  if (!prevLine) {
    _log->debug(p) << "\033[1mstarts new block (no previous line).\033[0m" << std::endl;
    return true;
  }

  // ------------
  // The line does *not* start a new block if the line and prev line are part of the same figure.
  PdfFigure* isPrevLinePartOfFigure = isPartOfFigure(prevLine);
  PdfFigure* isCurrLinePartOfFigure = isPartOfFigure(currLine);

  _log->debug(p) << "Checking figure overlappings ..." << std::endl;
  _log->debug(p) << " └─ prevLine.isPartOfFigure: " << isPrevLinePartOfFigure << std::endl;
  _log->debug(p) << " └─ currLine.isPartOfFigure: " << isCurrLinePartOfFigure << std::endl;

  if (isCurrLinePartOfFigure && isCurrLinePartOfFigure == isPrevLinePartOfFigure) {
    _log->debug(p) << "\033[1mcontinues block (part of the same figure).\033[0m" << std::endl;
    return false;
  }

  // ------------
  // The line starts a new block if it has another rotation than the previous line.
  _log->debug(p) << "Checking rotations ..." << std::endl;
  _log->debug(p) << " └─ prevLine.rotation: " << prevLine->position->rotation << std::endl;
  _log->debug(p) << " └─ currLine.rotation: " << currLine->position->rotation << std::endl;

  if (prevLine->position->rotation != currLine->position->rotation) {
    _log->debug(p) << "\033[1mstarts new block (rotations differ).\033[0m" << std::endl;
    return true;
  }

  // ------------
  // The line starts a new block if it has another writing mode than the previous line.
  _log->debug(p) << "Checking writing modes..." << std::endl;
  _log->debug(p) << " └─ prevLine.wMode: " << prevLine->position->wMode << std::endl;
  _log->debug(p) << " └─ currLine.wMode: " << currLine->position->wMode << std::endl;

  if (prevLine->position->wMode != currLine->position->wMode) {
    _log->debug(p) << "\033[1mstarts new block (writing modes differ).\033[0m" << std::endl;
    return true;
  }

  // ------------
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
  actualLineDistance = round(actualLineDistance, 1);

  _log->debug(p) << "Checking line distances..." << std::endl;
  _log->debug(p) << " └─ expected line distance: " << expectedLineDistance << std::endl;
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
      const PdfTextLine* nextLine, const std::unordered_set<std::string>* potentialFootnoteLabels,
      double hangingIndent, double percNoRightMarginLines, bool isCentered) {

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
  std::string pStr = currLine->parentTextLine ? currLine->parentTextLine->text : "-";
  std::string psStr = currLine->prevSiblingTextLine ? currLine->prevSiblingTextLine->text : "-";
  std::string nsStr = currLine->nextSiblingTextLine ? currLine->nextSiblingTextLine->text : "-";
  _log->debug(p) << " └─ line.parent: " << pStr << std::endl;
  _log->debug(p) << " └─ line.prevSibling: " << psStr << std::endl;
  _log->debug(p) << " └─ line.nextSibling: " << nsStr << std::endl;
  _log->debug(p) << "---------------" << std::endl;

  if (!prevLine) {
    _log->debug(p) << "\033[1mstarts new block (no previous line).\033[0m" << std::endl;
    return true;
  }

  // ==========
  // rightX
  double currLineFirstWordWidth = currLine->words[0]->position->getWidth();
  double isUnexpectedRightX = smaller(currLineFirstWordWidth, prevLine->rightMargin, 2 * _doc->avgGlyphWidth);

  // leftX
  bool isMoreIndented = larger(currLine->leftMargin, 6 * _doc->avgGlyphWidth, 0);
  bool prevIsMoreIndented = larger(prevLine->leftMargin, 6 * _doc->avgGlyphWidth, 0);
  bool isUnexpectedLeftX = false;
  if (isMoreIndented) {
    isUnexpectedLeftX |= !equal(currLine->leftMargin, prevLine->leftMargin, _doc->avgGlyphWidth);
  }
  if (prevIsMoreIndented) {
    isUnexpectedLeftX |= !equal(currLine->leftMargin, prevLine->leftMargin, _doc->avgGlyphWidth);
  }
  // ==========

  if (isCentered) {
    // Centered author info
    bool currFirstLineOfItem = isFirstLineOfItem(currLine);

    if (currFirstLineOfItem) {
      _log->debug(p) << "\033[1mstarts block (centered + item start).\033[0m" << std::endl;
      return true;
    }

    _log->debug(p) << "\033[1mcontinues block (centered).\033[0m" << std::endl;
    return false;
  }

  // ---------------
  // The line starts a new block if it is the first line of an enumeration item.
  bool prevFirstLineOfItem = isFirstLineOfItem(prevLine, potentialFootnoteLabels);
  bool currFirstLineOfItem = isFirstLineOfItem(currLine, potentialFootnoteLabels);

  _log->debug(p) << "Checking for first line of an item..." << std::endl;
  _log->debug(p) << " └─ prev sibling: " << psStr << std::endl;
  _log->debug(p) << " └─ next sibling: " << nsStr << std::endl;
  _log->debug(p) << " └─ prevLine.isFirstLineOfItem: " << prevFirstLineOfItem << std::endl;
  _log->debug(p) << " └─ currLine.isFirstLineOfItem: " << currFirstLineOfItem << std::endl;

  if (currFirstLineOfItem) {
    _log->debug(p) << "\033[1mstarts new block (first line of item).\033[0m" << std::endl;
    return true;
  }

  // ---------------
  // The line continues a new block if it is a continuation of an enumeration item.
  bool prevContLineOfItem = isContinuationLineOfItem(prevLine, potentialFootnoteLabels);
  bool currContLineOfItem = isContinuationLineOfItem(currLine, potentialFootnoteLabels);
  bool prevPartOfItem = prevFirstLineOfItem || prevContLineOfItem;
  bool currPartOfItem = currFirstLineOfItem || currContLineOfItem;

  _log->debug(p) << "Checking for continuation line of enumeration item..." << std::endl;
  _log->debug(p) << " └─ parent:   " << pStr << std::endl;
  _log->debug(p) << " └─ prevLine.isContinuationOfItem: " << prevContLineOfItem << std::endl;
  _log->debug(p) << " └─ currLine.isContinuationOfItem: " << currContLineOfItem << std::endl;

  if (currContLineOfItem) {
    if (isCentered) {
      _log->debug(p) << "\033[1mcontinues block (centered).\033[0m" << std::endl;
      return false;
    } else if (isUnexpectedRightX) {
      _log->debug(p) << "\033[1mstarts new block (unexpected right x).\033[0m" << std::endl;
      return true;
    } else if (prevFirstLineOfItem) {
      return false;
    } else if (prevContLineOfItem) {
      double xOffset = prevLine->position->leftX - currLine->position->leftX;
      _log->debug(p) << xOffset << " " << _doc->avgGlyphWidth << std::endl;
      return !between(xOffset, -_doc->avgGlyphWidth, 6 * _doc->avgGlyphWidth);
    } else {
      _log->debug(p) << "\033[1mcontinues block (continuation of item).\033[0m" << std::endl;
      return false;
    }
  }

  if (prevPartOfItem && !currPartOfItem) {
    if (isUnexpectedRightX) {
      _log->debug(p) << "\033[1mstarts new block (unexpected right x).\033[0m" << std::endl;
      return true;
    }

    // There could be an item in the following format:
    //    (i) This is an item that continues in the next
    //  line. Note the smaller leftX of the second line.
    if (!endsWithSentenceDelimiter(prevLine->text) && !startsWithUpper(currLine->text)) {
      _log->debug(p) << "\033[1mcontinues block (punct & lower).\033[0m" << std::endl;
      return false;
    }

    // _log->debug(p) << "\033[1mstarts block (prev part of item).\033[0m" << std::endl;
    // return true;
  }

  // // ---------------
  // // The line starts a new text block if it is the first line of a footnote.
  // bool prevFirstLineOfFootnote = isFirstLineOfFootnote(currLine, potentialFootnoteLabels);
  // bool currFirstLineOfFootnote = isFirstLineOfFootnote(currLine, potentialFootnoteLabels);

  // _log->debug(p) << "Checking for first line of footnote..." << std::endl;
  // _log->debug(p) << " └─ prevLine.isFirstLineOfFootnote: " << prevFirstLineOfFootnote << std::endl;
  // _log->debug(p) << " └─ currLine.isFirstLineOfFootnote: " << currFirstLineOfFootnote << std::endl;

  // if (currFirstLineOfFootnote) {
  //   _log->debug(p) << "\033[1mstarts new block (first line of footnote).\033[0m" << std::endl;
  //   return true;
  // }

  // // ---------------
  // // The line continues a new block if it is a continuation of a footnote.
  // bool prevContLineOfFootnote = isContinuationLineOfFootnote(prevLine, potentialFootnoteLabels);
  // bool currContLineOfFootnote = isContinuationLineOfFootnote(currLine, potentialFootnoteLabels);
  // bool prevPartOfFootnote = prevFirstLineOfFootnote || prevContLineOfFootnote;
  // bool currPartOfFootnote = currFirstLineOfFootnote || currContLineOfFootnote;

  // _log->debug(p) << "Checking for continuation line of footnote..." << std::endl;
  // _log->debug(p) << " └─ prevLine.isContOfFootnote: " << prevContLineOfFootnote << std::endl;
  // _log->debug(p) << " └─ currLine.isContOfFootnote: " << currContLineOfFootnote << std::endl;

  // if (currContLineOfFootnote) {
  //   if (prevContLineOfFootnote) {
  //     double xOffset = prevLine->position->leftX - currLine->position->leftX;
  //     return !between(xOffset, -_doc->avgGlyphWidth, 6 * _doc->avgGlyphWidth);
  //   } else if (isUnexpectedRightX) {
  //     _log->debug(p) << "\033[1mstarts new block (unexpected right x).\033[0m" << std::endl;
  //     return true;
  //   } else {
  //     _log->debug(p) << "\033[1mcontinues block (continuation of item).\033[0m" << std::endl;
  //     return false;
  //   }
  // }

  // if (prevPartOfFootnote && !currPartOfFootnote) {
  //   _log->debug(p) << "\033[1mstarts block (prev part of footnote).\033[0m" << std::endl;
  //   return true;
  // }

  // ---------------
  // The line does not start a new block if the previous line and the current line are emphasized,
  // and if both lines exhibits the same font and the same font size. This rule exists to not split
  // titles and headings, which are often centered (which means that the left margin of the text
  // lines are > 0), in two parts in the next rule (which assumes the start of a new block if the
  // left margin of the current line is > 0).
  bool isPrevLineEmphasized = isTextLineEmphasized(prevLine);
  bool isCurrLineEmphasized = isTextLineEmphasized(currLine);
  bool hasEqualFontName = prevLine->fontName == currLine->fontName;
  bool hasEqualFontSize = equal(prevLine->fontSize, currLine->fontSize, 0.1);

  _log->debug(p) << "Checking for emphasis..." << std::endl;
  _log->debug(p) << " └─ prevLine.isEmphasized: " << isPrevLineEmphasized << std::endl;
  _log->debug(p) << " └─ currLine.isEmphasized: " << isCurrLineEmphasized << std::endl;
  _log->debug(p) << " └─ prevLine.fontName: " << prevLine->fontName << std::endl;
  _log->debug(p) << " └─ currLine.fontName: " << currLine->fontName << std::endl;
  _log->debug(p) << " └─ prevLine.fontSize: " << prevLine->fontSize << std::endl;
  _log->debug(p) << " └─ currLine.fontSize: " << currLine->fontSize << std::endl;

  if (isPrevLineEmphasized && isCurrLineEmphasized && hasEqualFontName && hasEqualFontSize) {
    _log->debug(p) << "\033[1mcontinues block (same emphasized font style).\033[0m" << std::endl;
    return false;
  }

  // TODO
  // The line starts a new text block if (1) the left margin of the current line is == 0, and (2)
  // the left margin of the previous line is larger than the most frequent left margin. This rule
  // exists to split body text paragraph following a display formula apart. The second condition
  // exists to not detect the second line of a body text paragraph from the (indented) first line.
  // if (larger(prevLine->leftMargin - currLine->leftMargin, 6 * _doc->avgGlyphWidth, 0)) {
  //   return true;
  // }

  _log->debug(p) << "Checking indentation..." << std::endl;
  _log->debug(p) << " └─ most freq. left margin: " << _mostFreqLineLeftMargin << std::endl;
  _log->debug(p) << " └─ doc.avgGlyphWidth: " << _doc->avgGlyphWidth << std::endl;
  _log->debug(p) << " └─ prevLine.leftMargin: " << prevLine->leftMargin << std::endl;
  _log->debug(p) << " └─ currLine.leftMargin: " << currLine->leftMargin << std::endl;
  _log->debug(p) << " └─ prevLine.rightMargin: " << prevLine->rightMargin << std::endl;
  _log->debug(p) << " └─ currLine.rightMargin: " << currLine->rightMargin << std::endl;
  _log->debug(p) << " └─ hangingIndent: " << hangingIndent << std::endl;

  if (hangingIndent > 0) {
    // -----
    bool prevNotIndented = smaller(prevLine->leftMargin, hangingIndent, _doc->avgGlyphWidth);
    bool currNotIndented = smaller(currLine->leftMargin, hangingIndent, _doc->avgGlyphWidth);

    _log->debug(p) << " └─ prevLine.notIndented: " << prevNotIndented << std::endl;
    _log->debug(p) << " └─ currLine.notIndented: " << currNotIndented << std::endl;

    if (currNotIndented) {
      _log->debug(p) << "\033[1mstarts block (hanging indent & no indent).\033[0m" << std::endl;
      return true;
    }

    // ---------------
    // The line continues a new block if it is a continuation of a footnote.
    bool prevIndented = equal(prevLine->leftMargin, hangingIndent, _doc->avgGlyphWidth);
    bool currIndented = equal(currLine->leftMargin, hangingIndent, _doc->avgGlyphWidth);
    bool prevMoreIndented = larger(prevLine->leftMargin, hangingIndent, _doc->avgGlyphWidth);
    bool currMoreIndented = larger(currLine->leftMargin, hangingIndent, _doc->avgGlyphWidth);

    _log->debug(p) << " └─ prevLine.indented: " << prevIndented << std::endl;
    _log->debug(p) << " └─ currLine.indented: " << currIndented << std::endl;
    _log->debug(p) << " └─ prevLine.moreIndented: " << prevMoreIndented << std::endl;
    _log->debug(p) << " └─ currLine.moreIndented: " << currMoreIndented << std::endl;

    if (currIndented) {
      if (prevMoreIndented) {
        double xOffset = prevLine->position->leftX - currLine->position->leftX;
        return !between(xOffset, -_doc->avgGlyphWidth, 3 * _doc->avgGlyphWidth);
      } else if (isUnexpectedRightX) {
        return true;
      } else {
        return false;
      }
    }

    if (currMoreIndented) {
      if (prevMoreIndented) {
        double xOffset = prevLine->position->leftX - currLine->position->leftX;
        return !between(xOffset, -_doc->avgGlyphWidth, _doc->avgGlyphWidth);
      } else if (isUnexpectedRightX) {
        return true;
      } else {
        return true;
      }
    }

    // bool notIndented = smaller(currLine->leftMargin, hangingIndent, _doc->avgGlyphWidth);
    // if (notIndented) {
    //   _log->debug(p) << "\033[1mstarts block (hanging indent & no indent).\033[0m" << std::endl;
    //   return true;
    // }
    // bool indented = equal(currLine->leftMargin, hangingIndent, _doc->avgGlyphWidth);
    // if (indented) {
    //   if (isUnexpectedRightX) {
    //     _log->debug(p) << "\033[1mstarts new block (prev line is short).\033[0m" << std::endl;
    //     return true;
    //   } else {
    //     _log->debug(p) << "\033[1mcontinues block (hanging indent & indent).\033[0m" << std::endl;
    //     return false;
    //   }
    // }
    // bool moreIndented = larger(currLine->leftMargin, hangingIndent, _doc->avgGlyphWidth);
    // if (moreIndented) {
    //   _log->debug(p) << "\033[1mstarts new block (is more indented).\033[0m" << std::endl;
    //   return true;
    // }

    _log->debug(p) << "\033[1mcontinues block (hanging indent, no rule applied).\033[0m" << std::endl;
    return false;
  }

  if (isMoreIndented) {
    _log->debug(p) << "\033[1mstarts new block (left margin > expected indent).\033[0m" << std::endl;
    return !equal(currLine->position->leftX, prevLine->position->leftX, _doc->avgGlyphWidth);
  }
  if (prevIsMoreIndented) {
    _log->debug(p) << "\033[1mstarts block (prev is more indented, curr line not).\033[0m" << std::endl;
    return !equal(currLine->position->leftX, prevLine->position->leftX, _doc->avgGlyphWidth);
  }


  bool isIndented = between(currLine->leftMargin, _doc->avgGlyphWidth, 6 * _doc->avgGlyphWidth);

  if (isIndented) {
    _log->debug(p) << "\033[1mstarts block (no hanging indent & indent).\033[0m" << std::endl;
    return true;
  }

  if (isUnexpectedRightX) {
    _log->debug(p) << "\033[1mstarts new block (prev line has capacity).\033[0m" << std::endl;
    return true;
  }

  if (percNoRightMarginLines >= 0.75 && larger(prevLine->rightMargin, 0, 5 * _doc->avgGlyphWidth)) {
    _log->debug(p) << "\033[1mAstarts new block (prevLine.rightMargin > 0).\033[0m" << std::endl;
    return true;
  }
  if (_percZeroRightMarginTextLines > 0.5 && larger(prevLine->rightMargin, 0, 10 * _doc->avgGlyphWidth)) {
    _log->debug(p) << "\033[1mBstarts new block (prevLine.rightMargin > 0).\033[0m" << std::endl;
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
          double isCentered = equal(leftMargin, rightMargin, _doc->avgGlyphWidth);
          if (!isCentered) {
            if (prevBlock) { blockTrimRightX = std::max(blockTrimRightX, prevBlock->trimRightX); }
            if (nextBlock) { blockTrimRightX = std::max(blockTrimRightX, nextBlock->trimRightX); }
          }
        }

        for (size_t j = 0; j < currBlock->lines.size(); j++) {
          PdfTextLine* prevLine = j > 0 ? currBlock->lines.at(j - 1) : nullptr;
          PdfTextLine* currLine = currBlock->lines.at(j);
          PdfTextLine* nextLine = j < currBlock->lines.size() - 1 ? currBlock->lines.at(j + 1) : nullptr;

          // double trimLeftX = std::max(segment->trimLeftX, block->position->leftX);
          // double trimRightX = std::min(segment->trimRightX, block->position->rightX);
          // double trimRightX = segment->trimRightX;
          currLine->leftMargin = round(currLine->position->leftX - currBlock->trimLeftX);
          currLine->rightMargin = round(blockTrimRightX - currLine->position->rightX);

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
            nextLineLeftMargin = round(nextLine->position->getRotLeftX() - currBlock->position->getRotLeftX());
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
bool TextBlocksDetector::isFirstLineOfItem(const PdfTextLine* line,
      const std::unordered_set<std::string>* potentialFootnoteLabels) const {
  if (!line) {
    return false;
  }

  if (line->text.empty()) {
    return false;
  }

  // The line is the first line of an item if it starts with an item label, and there is a previous
  // and/or next sibling text line that also starts with an item label.
  bool currStartsWithItemLabel = startsWithItemLabel(line);

  bool existsPrevItem = false;
  if (line->prevSiblingTextLine && !line->prevSiblingTextLine->words.empty()) {
    bool prevStartsWithItemLabel = startsWithItemLabel(line->prevSiblingTextLine);
    PdfWord* prevFirstWord = line->prevSiblingTextLine->words[0];
    PdfWord* currFirstWord = line->words[0];
    bool isSameFont = prevFirstWord->fontName == currFirstWord->fontName;
    bool isSameFontsize = equal(prevFirstWord->fontSize, currFirstWord->fontSize, 0.5);
    existsPrevItem = prevStartsWithItemLabel && isSameFont && isSameFontsize;
  }

  bool existsNextItem = false;
  if (line->nextSiblingTextLine && !line->nextSiblingTextLine->words.empty()) {
    bool nextStartsWithItemLabel = startsWithItemLabel(line->nextSiblingTextLine);
    PdfWord* nextFirstWord = line->nextSiblingTextLine->words[0];
    PdfWord* currFirstWord = line->words[0];
    bool isSameFont = nextFirstWord->fontName == currFirstWord->fontName;
    bool isSameFontsize = equal(nextFirstWord->fontSize, currFirstWord->fontSize, 0.5);
    existsNextItem = nextStartsWithItemLabel && isSameFont && isSameFontsize;
  }

  // EXPERIMENTAL: The line is not the first line of a footnote when (1) the font of the line is
  // equal to the font of the previous line, (2) the line distance to the previous line is <= 0; and
  // and (3) the previous line does not end with a punctuation mark. This should avoid to detect
  // lines that occasionally starts with a superscripted number as a footnote. Example:
  // 0901.4737:11.
  if (line->prevLine) {
    bool prevStartsWithItemLabel = startsWithItemLabel(line->prevLine);
    bool isEqualFont = line->prevLine->fontName == line->fontName;
    bool isEqualFontSize = equal(line->prevLine->fontSize, line->fontSize, 0.5);
    double distance = line->position->upperY - line->prevLine->position->lowerY;
    bool isSentenceDelimiter = endsWithSentenceDelimiter(line->prevLine->text);
    bool isEqualLeftX = equal(line->prevLine->position->leftX, line->position->leftX, _doc->avgGlyphWidth);

    if (!prevStartsWithItemLabel && isEqualFont && isEqualFontSize && distance <= 0 && !isSentenceDelimiter && isEqualLeftX) {
      return false;
    }
  }


  if (currStartsWithItemLabel && (existsPrevItem || existsNextItem)) {
    return true;
  }

  // The line is the first line of an item if it starts with a footnote label.
  if (startsWithFootnoteLabel(line, potentialFootnoteLabels)) {
    return true;
  }

  return false;
}

// _________________________________________________________________________________________________
bool TextBlocksDetector::isContinuationLineOfItem(const PdfTextLine* line,
      const std::unordered_set<std::string>* potentialFootnoteLabels) const {
  if (!line) {
    return false;
  }

  return isFirstLineOfItem(line->parentTextLine, potentialFootnoteLabels)
      || isContinuationLineOfItem(line->parentTextLine, potentialFootnoteLabels);
}

// _________________________________________________________________________________________________
bool TextBlocksDetector::startsWithItemLabel(const PdfTextLine* line) const {
  if (!line) {
    return false;
  }

  if (line->text.empty()) {
    return false;
  }

  // The line starts with an item label if its first word is a superscripted 1,2,3... (or a,b,c...).
  PdfGlyph* firstGlyph = line->words[0]->glyphs[0];
  if (firstGlyph->isSuperscript && ITEM_LABEL_SUPERSCRIPT_ALPHABET.find(firstGlyph->text[0]) != std::string::npos) {
    return true;
  }

  std::smatch m;
  for (const auto& regex : item_anchor_regexes) {
    if (std::regex_search(line->text, m, regex)) {
      return true;
    }
  }

  return false;
}

// _________________________________________________________________________________________________
bool TextBlocksDetector::startsWithFootnoteLabel(const PdfTextLine* line,
      const std::unordered_set<std::string>* potentialFootnoteLabels) const {
  if (!line) {
    return false;
  }

  if (line->text.empty()) {
    return false;
  }

  std::string superScriptPrefix;
  PdfWord* firstWord = line->words[0];
  for (auto* glyph : firstWord->glyphs) {
    if (!glyph->isSuperscript) {
      break;
    }
    superScriptPrefix += glyph->text;
  }

  if (potentialFootnoteLabels) {
    return potentialFootnoteLabels->count(superScriptPrefix) > 0;
  }

  return !superScriptPrefix.empty();
}



// // _________________________________________________________________________________________________
// bool TextBlocksDetector::isContinuationLineOfItem(const PdfTextLine* line) const {
//   if (!line) {
//     return false;
//   }

//   if (!line->parentTextLine) {
//     return false;
//   }

//   if (line->parentTextLine->words.empty()) {
//     return false;
//   }

//   // The line is a continuation of an item, if the parent text line is an continuation of the item.
//   if (isContinuationLineOfItem(line->parentTextLine)) {
//     return true;
//   }

//   // The line is not a continuation of an item when the parent line is not the 1st line of an item.
//   if (!isFirstLineOfItem(line->parentTextLine)) {
//     return false;
//   }

//   // The line is not a continuation of an item when its leftX value is smaller than the rightX value
//   // of the first word of the parent text line. The assumption made here is that the item label
//   // consists of only one word and the second word is the first word of the item body.
//   const PdfWord* firstParentWord = line->parentTextLine->words[0];
//   if (smaller(line->position->leftX, firstParentWord->position->rightX)) {
//     return false;
//   }

//   return true;
// }

// _________________________________________________________________________________________________
// bool TextBlocksDetector::isFirstLineOfFootnote(const PdfTextLine* line,
//       const std::unordered_set<std::string>* potentialFootnoteMarkers) const {
//   if (!line) {
//     return false;
//   }

//   if (line->text.size() == 0) {
//     return false;
//   }

//   // if (!smaller(line->fontSize, _doc->mostFreqFontSize, .9)) { // TODO
//   //   return false;
//   // }

//   std::string superScriptPrefix;
//   PdfWord* firstWord = line->words[0];
//   for (auto* glyph : firstWord->glyphs) {
//     if (!glyph->isSuperscript) {
//       break;
//     }
//     superScriptPrefix += glyph->text;
//   }

//   if (potentialFootnoteMarkers->count(superScriptPrefix) > 0) {
//     return true;
//   }

//   // if (std::regex_search(firstWord->text, m, isFootnoteMarkerRegex)) {
//   //   return true;
//   // }

//   return false;
// }

// _________________________________________________________________________________________________
// bool TextBlocksDetector::isContinuationLineOfFootnote(const PdfTextLine* line,
//       const std::unordered_set<std::string>* potentialFootnoteMarkers) const {
//   return line->parentTextLine &&
//       (isFirstLineOfFootnote(line->parentTextLine, potentialFootnoteMarkers)
//         || isContinuationLineOfFootnote(line->parentTextLine, potentialFootnoteMarkers));
// }

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
      && lineFontInfo->weight - docFontInfo->weight > 100) {
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
void TextBlocksDetector::computeTextBlockTrimBoxes() const {
  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      // Compute the most frequent rightX coordinates among the text lines in the segment.
      std::unordered_map<double, int> rightXFreqs;

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
        block->trimRightX = std::min(block->position->rightX, mostFreqRightX);
      }
    }
  }
}

// _________________________________________________________________________________________________
void TextBlocksDetector::computeTextLineIndentHierarchies() {
  for (auto* page : _doc->pages) {
    std::stack<PdfTextLine*> lineStack;
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
            if (larger(abs(actualLineDistance), std::max(10.0, 3 * _mostFreqLineDistance))) {
              lineStack = std::stack<PdfTextLine*>();
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

          // std::pair<double, double> xOverlapRatios = computeXOverlapRatios(lineStack.top(), line);
          // double maxXOVerlapRatio = std::max(xOverlapRatios.first, xOverlapRatios.second);
          // if (maxXOVerlapRatio > 0) {
          if (lineStack.top()->position->lowerY < line->position->lowerY) {
            if (equal(lineStack.top()->position->leftX, line->position->leftX, _doc->avgGlyphWidth)) {
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

          bool isCentered = equal(line->leftMargin, line->rightMargin, _doc->avgGlyphWidth) &&
              larger(line->leftMargin, _doc->avgGlyphWidth, 0);
          bool isNotIndented = equal(line->leftMargin, 0, _doc->avgGlyphWidth);
          bool isIndented = equal(line->leftMargin, mostFreqLeftMargin, _doc->avgGlyphWidth);
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
    if (equal(line->rightMargin, 0, _doc->avgGlyphWidth)) {
      numNoRightMarginLines++;
    }
    numLines++;
  }
  return numLines > 0 ? numNoRightMarginLines / numLines : 0;
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
          isMarker |= !glyph->text.empty() && FOOTNOTE_LABEL_ALPHABET.find(glyph->text[0]) != std::string::npos;

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
  for (size_t i = 0; i < lines.size(); i++) {
    PdfTextLine* prevLine = i > 0 ? lines[i-1] : nullptr;
    PdfTextLine* currLine = lines[i];
    PdfTextLine* nextLine = i < lines.size() - 1 ? lines[i+1] : nullptr;

    double lineMinX = std::min(currLine->position->leftX, currLine->position->rightX);
    double lineMinY = std::min(currLine->position->upperY, currLine->position->lowerY);
    double lineMaxX = std::max(currLine->position->leftX, currLine->position->rightX);
    double lineMaxY = std::max(currLine->position->upperY, currLine->position->lowerY);

    // Update the x,y-coordinates.
    block->position->leftX = std::min(block->position->leftX, lineMinX);
    block->position->upperY = std::min(block->position->upperY, lineMinY);
    block->position->rightX = std::max(block->position->rightX, lineMaxX);
    block->position->lowerY = std::max(block->position->lowerY, lineMaxY);

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

  block->isEmphasized = isTextBlockEmphasized(lines);

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

    std::pair<double, double> ratios = computeXOverlapRatios(prevLine, currLine);
    double maxXOverlapRatio = std::max(ratios.first, ratios.second);

    if (smaller(maxXOverlapRatio, 1, 0.01)) {
      return false;
    }

    double leftXOffset = abs(prevLine->position->leftX - currLine->position->leftX);
    double rightXOffset = abs(prevLine->position->rightX - currLine->position->rightX);
    bool isEqualOffset = equal(leftXOffset, rightXOffset, 2 * _doc->avgGlyphWidth);

    if (!isEqualOffset) {
      return false;
    }

    bool isLargeLeftXOffset = larger(leftXOffset, 0, 2 * _doc->avgGlyphWidth);
    bool isLargeRightXOffset = larger(rightXOffset, 0, 2 * _doc->avgGlyphWidth);
    bool isLargeXOffset = isLargeLeftXOffset || isLargeRightXOffset;
    bool prevIsFormula = false;
    for (char c : FORMULA_ID_ALPHABET) {
      if (prevLine->text.find(c) != std::string::npos) {
        prevIsFormula = true;
        break;
      }
    }
    bool currIsFormula = false;
    for (char c : FORMULA_ID_ALPHABET) {
      if (currLine->text.find(c) != std::string::npos) {
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
  bool isEqualOffset = equal(leftXOffset, rightXOffset, 2 * _doc->avgGlyphWidth);




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

  _log->debug(prevLine->position->pageNum) << "YYY " << isLargeMargin << " " << currLine->text << std::endl;
  _log->debug(prevLine->position->pageNum) << "ZZZ " << isLargeXOffset << " " << currLine->text << std::endl;

  // (3) The height of the previous line and current line is almost the same. This requirement
  // exists to not accidentally consider a centered formula and a subsequent text line, which is
  // actually part of a justified block, to be part of a centered block.
  // TODO: This assumes that the height of formulas are larger than the height of text lines, which
  // is not true in any case.
  double prevLineHeight = prevLine->position->getHeight();
  double currLineHeight = currLine->position->getHeight();
  bool isSameHeight = equal(prevLineHeight, currLineHeight, 0.25 * _doc->mostFreqWordHeight);

  bool isCentered = isEqualOffset && (isLargeMargin || isLargeXOffset) && isSameHeight;

  if (verbose) {
    int p = currLine->position->pageNum;
    _log->debug(p) << "Checking for centered block..." << std::endl;
    _log->trace(p) << " └─ leftXOffset: " << leftXOffset << std::endl;
    _log->trace(p) << " └─ rightXOffset: " << rightXOffset << std::endl;
    _log->debug(p) << " └─ isEqualOffset: " << isEqualOffset << std::endl;
    _log->trace(p) << " └─ isLargePrevLeftMargin: " << isLargePrevLeftMargin << std::endl;
    _log->trace(p) << " └─ isLargePrevRightMargin: " << isLargePrevRightMargin << std::endl;
    _log->trace(p) << " └─ isLargeCurrLeftMargin: " << isLargeCurrLeftMargin << std::endl;
    _log->trace(p) << " └─ isLargeCurrRightMargin: " << isLargeCurrRightMargin << std::endl;
    _log->trace(p) << " └─ isLargePrevMargin: " << isLargePrevMargin << std::endl;
    _log->trace(p) << " └─ isLargeCurrMargin: " << isLargeCurrMargin << std::endl;
    _log->trace(p) << " └─ isLargeMargin: " << isLargeMargin << std::endl;
    _log->trace(p) << " └─ isLargeLeftXOffset: " << isLargeLeftXOffset << std::endl;
    _log->trace(p) << " └─ isLargeRightXOffset: " << isLargeRightXOffset << std::endl;
    _log->debug(p) << " └─ isLargeXOffset: " << isLargeXOffset << std::endl;
    _log->trace(p) << " └─ prevLine.height: " << prevLineHeight << std::endl;
    _log->trace(p) << " └─ currLine.height: " << currLineHeight << std::endl;
    _log->trace(p) << " └─ _doc->mostFreqWordHeight: " << _doc->mostFreqWordHeight << std::endl;
    _log->debug(p) << " └─ isSameHeight: " << isSameHeight << std::endl;
    _log->debug(p) << "is centered block: " << isCentered << std::endl;
  }

  return isCentered;
}