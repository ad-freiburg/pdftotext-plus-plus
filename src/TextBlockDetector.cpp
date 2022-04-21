/**
 * Copyright 2021, University of Freiburg,
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
#include "./TextBlockDetector.h"
#include "./utils/Utils.h"

std::regex item_anchor_regexes[] = {
  // A regex to find items starting with "•" or "-" or "+".
  std::regex("^(•|-|–|\\+)\\s+"),
  // A regex to find items starting with "I.", "II.", etc.
  std::regex("^(X{0,1}(IX|IV|V?I{0,3}))\\.\\s+", std::regex_constants::icase),
  // A regex to find items starting with "(I)", "(II)", "(III)", etc.
  std::regex("^\\((X{0,1}(IX|IV|V?I{0,3}))\\)\\s+", std::regex_constants::icase),
  // A regex to find items that start with "a.", "b.", "c.", 0., 1., etc.
  std::regex("^([a-z0-9])\\.\\s+"),
  // A regex to find items that start with (A), (1), (C1), or [1], [2], etc.
  std::regex("^(\\(|\\[)([a-z0-9][0-9]{0,1})(\\)|\\])\\s+", std::regex_constants::icase),
  // A regex to find items that start with A) or 1) or a1).
  std::regex("^([a-z0-9][0-9]{0,1})\\)\\s+", std::regex_constants::icase)
};

// A regex to find footnotes starting with a digit.
std::regex isDigitRegex("^\\d+");
// A regex to find footnotes starting with "*", "†", or "‡", or "?". The "?" is for symbols
// that couldn't be translated to text (e.g., because of a missing encoding).
std::regex isFootnoteMarkerRegex("^(\\*|†|‡|\\?)");

// A regex to find the marker at the end of a display formula, which are of form "(1)", or "(2.12)".
std::regex formula_marker_regex("\\(\\d{1,2}(\\.\\d{1,2})?\\)$");

// _________________________________________________________________________________________________
TextBlockDetector::TextBlockDetector(PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
TextBlockDetector::~TextBlockDetector() = default;

// _________________________________________________________________________________________________
void TextBlockDetector::detect() {
  // Do nothing if no document is given.
  if (!_doc) {
    return;
  }

  // Do nothing if no pages are given.
  if (_doc->pages.size() == 0) {
    return;
  }

  computeMostFrequentLinePitch();
  computeMostFrequentLinePitchPerPage();
  computeMostFrequentLinePitchPerFontSize();
  computeTextLineIndentHierarchies();

  for (auto* page : _doc->pages) {
    std::vector<PdfTextBlock*> textBlocks;
    for (auto* segment : page->segments) {
      std::vector<PdfTextLine*> currentTextBlockLines;
      for (size_t i = 0; i < segment->lines.size(); i++) {
        PdfTextLine* prevLine = i > 0 ? segment->lines.at(i - 1) : nullptr;
        PdfTextLine* line = segment->lines.at(i);
        PdfTextLine* nextLine = i < segment->lines.size() - 1 ? segment->lines.at(i + 1) : nullptr;

        if (startsNewTextBlock(prevLine, line, nextLine)) {
          // createTextBlock(currentTextBlockLines, &page->blocks);
          createTextBlock(currentTextBlockLines, &textBlocks);
          currentTextBlockLines.clear();
        }
        currentTextBlockLines.push_back(line);
      }
      // Don't forget to process the remaining text lines of the segment.
      // createTextBlock(currentTextBlockLines, &page->blocks);
      createTextBlock(currentTextBlockLines, &textBlocks);
    }

    computeTextLineAlignments(textBlocks);

    for (auto* block : textBlocks) {
      std::vector<PdfTextLine*> currentTextBlockLines;
      PdfTextLine* prevLine = nullptr;
      for (auto* line : block->lines) {
        if (startsNewTextBlock2(prevLine, line)) {
          createTextBlock(currentTextBlockLines, &page->blocks);
          currentTextBlockLines.clear();
        }
        currentTextBlockLines.push_back(line);
        prevLine = line;
      }
      // Don't forget to process the remaining text lines of the segment.
      createTextBlock(currentTextBlockLines, &page->blocks);
    }
  }
}

// _________________________________________________________________________________________________
bool TextBlockDetector::startsNewTextBlock(const PdfTextLine* prevLine, const PdfTextLine* line,
      const PdfTextLine* nextLine) {
  std::cout << "==========" << std::endl;
  std::cout << "\033[1m" << line->toString() << "\033[0m" << std::endl;

  if (!prevLine) {
    return true;
  }

  // The line doesn't start a new text block if the line and the previous line is part of a figure.
  std::cout << "Checking if lines are part of a figure..." << std::endl;
  bool isPrevLinePartOfFigure = isPartOfFigure(prevLine);
  bool isLinePartOfFigure = isPartOfFigure(line);
  std::cout << " └─ prevLine.isPartOfFigure: " << isPrevLinePartOfFigure << std::endl;
  std::cout << " └─ line.isPartOfFigure: " << isLinePartOfFigure << std::endl;
  if (isPrevLinePartOfFigure && isLinePartOfFigure) {
    std::cout << "\033[1mContinuation of block (lines are part of figure).\033[0m" << std::endl;
    return false;
  }

  // The line starts a new text block if it has another rotation than the previous line.
  std::cout << "Checking rotations..." << std::endl;
  std::cout << " └─ prevLine.rotation: " << prevLine->position->rotation << std::endl;
  std::cout << " └─ currLine.rotation: " << line->position->rotation << std::endl;
  if (prevLine->position->rotation != line->position->rotation) {
    std::cout << "\033[1mBegin of new block (rotations don't match).\033[0m" << std::endl;
    return true;
  }

  // The line starts a new text block if it has another writing mode than the previous line.
  std::cout << "Checking writing modes..." << std::endl;
  std::cout << " └─ prevLine.wMode: " << prevLine->position->wMode << std::endl;
  std::cout << " └─ currLine.wMode: " << line->position->wMode << std::endl;
  if (prevLine->position->wMode != line->position->wMode) {
    std::cout << "\033[1mBegin of new block (writing modes don't match).\033[0m" << std::endl;
    return true;
  }

  // The line starts a new text block if the difference between its font size and the font size of
  // the previous line is larger than a threshold.
  std::cout << "Checking font sizes..." << std::endl;
  std::cout << " └─ prevLine.fontSize: " << prevLine->fontSize << std::endl;
  std::cout << " └─ currLine.fontSize: " << line->fontSize << std::endl;
  if (!equal(prevLine->fontSize, line->fontSize, 1)) {
    std::cout << "\033[1mBegin of new block (font sizes don't match).\033[0m" << std::endl;
    return true;
  }

  std::cout << "Checking line pitches..." << std::endl;
  double fontSize = round(line->fontSize, 1);
  double expectedLinePitch = 0;
  if (_mostFreqLinePitchPerFontSize.count(fontSize) > 0) {
    double lp = _mostFreqLinePitchPerFontSize.at(fontSize);
    std::cout << " └─ expected line pitch regarding fontsize: " << lp << std::endl;
    expectedLinePitch = std::max(expectedLinePitch, lp);
  }
  // if (_mostFreqLinePitchPerPage.count(line->position->pageNum) > 0) {
  //   double lp = _mostFreqLinePitchPerPage.at(line->position->pageNum);
  //   std::cout << " └─ expected line pitch regarding page: " << lp << std::endl;
  //   expectedLinePitch = std::max(expectedLinePitch, lp);
  // }
  std::cout << " └─ expected line pitch regarding doc: " << _mostFreqLinePitch << std::endl;
  expectedLinePitch = std::max(expectedLinePitch, _mostFreqLinePitch);
  std::cout << " └─ expected line pitch: " << expectedLinePitch << std::endl;

  double linePitch = 0;
  if (line->position->rotation == 0 || line->position->rotation == 1) {
    linePitch = line->position->getRotUpperY() - prevLine->position->getRotLowerY();
  } else {
    linePitch = prevLine->position->getRotLowerY() - line->position->getRotUpperY();
  }
  std::cout << " └─ actual line pitch: " << linePitch << std::endl;

  // The line does not start a new new text block if the line pitch is smaller equal 0.
  if (equalOrSmaller(linePitch, 0)) {
    std::cout << "\033[1mContinuation of block (line pitch too small).\033[0m" << std::endl;
    return false;
  }

  // The line starts a new text block if the line pitch between the line and the previous line is
  // larger than the computed most frequent line pitch, under consideration of a small threshold.
  if (larger(linePitch, expectedLinePitch, std::max(1.0, 0.1 * expectedLinePitch))) {
    std::cout << "\033[1mBegin of new block (line pitch too large).\033[0m" << std::endl;
    return true;
  }

  std::cout << "Checking for column break..." << std::endl;
  double lineXOffset = 0;
  if (line->position->rotation == 0 || line->position->rotation == 1) {
    lineXOffset = line->position->getRotLeftX() - prevLine->position->getRotLeftX();
  } else {
    lineXOffset = prevLine->position->getRotLeftX() - line->position->getRotLeftX();
  }

  double lineYOffset = 0;
  if (line->position->rotation == 0 || line->position->rotation == 1) {
    lineYOffset = line->position->lowerY - prevLine->position->lowerY;
  } else {
    lineYOffset = prevLine->position->lowerY - line->position->lowerY;
  }

  if (lineXOffset > 0 && lineYOffset < -2 * prevLine->position->getHeight()) {
    std::cout << "\033[1mBegin of new block (column break).\033[0m" << std::endl;
    return true;
  }

  return false;
}

// _________________________________________________________________________________________________
bool TextBlockDetector::startsNewTextBlock2(const PdfTextLine* prevLine, const PdfTextLine* line) {
  // std::cout << "\033[1m" << line->toString() << "\033[0m" << std::endl;
  // std::cout << "\033[1m" << "Parent: " << (line->parentTextLine ? line->parentTextLine->toString() : "-") << "\033[0m" << std::endl;
  // std::cout << "\033[1m" << "Next: " << (line->nextSiblingTextLine ? line->nextSiblingTextLine->toString() : "-") << "\033[0m" << std::endl;
  // std::cout << "\033[1m" << isFirstLineOfItem(line) << "\033[0m" << std::endl;
  // std::cout << "\033[1m" << isContinuationLineOfItem(line) << "\033[0m" << std::endl;

  std::cout << "==========" << std::endl;
  std::cout << "\033[1m" << line->toString() << "\033[0m" << std::endl;

  if (!prevLine) {
    return true;
  }

  // The line starts a new text block if it is the first line of an enumeration item.
  std::cout << "Checking for enumeration item..." << std::endl;
  bool firstLineOfItem = isFirstLineOfItem(line);
  bool continuationLineOfItem = isContinuationLineOfItem(line);
  bool prevFirstLineOfItem = isFirstLineOfItem(prevLine);
  bool prevContinuationLineOfItem = isContinuationLineOfItem(prevLine);
  std::cout << "  is first: " << firstLineOfItem << std::endl;
  std::cout << "  prev is first: " << prevFirstLineOfItem << std::endl;
  std::cout << "  is cont: " << continuationLineOfItem << std::endl;
  std::cout << "  prev is cont: " << prevContinuationLineOfItem << std::endl;
  if (firstLineOfItem) {
    std::cout << "\033[1mBegin of new block (line is first line of item).\033[0m" << std::endl;
    return true;
  }
  // The line does not start a new text block if it is the continuation of an item.
  if (continuationLineOfItem) {
    std::cout << "\033[1mContinuation of block (line is continuation of item).\033[0m" << std::endl;
    return false;
  }
  // if (!(firstLineOfItem || continuationLineOfItem) && (prevFirstLineOfItem || prevContinuationLineOfItem)) {
  //   std::cout << "\033[1mContinuation of block (change between enumeration and non-enumeration).\033[0m" << std::endl;
  //   return true;
  // }

  // The line starts a new text block if the previous line or the current line is part of a display
  // formula (but not both).
  std::cout << "Checking for display formula..." << std::endl;
  bool isPrevLineDisplayFormula = isDisplayFormula(prevLine);
  bool isLineDisplayFormula = isDisplayFormula(line);
  std::cout << "  is prev line display formula: " << isPrevLineDisplayFormula << std::endl;
  std::cout << "  is line display formula: " << isLineDisplayFormula << std::endl;
  if (isPrevLineDisplayFormula != isLineDisplayFormula) {
    std::cout << "\033[1mbegin of new block (change between display formula).\033[0m" << std::endl;
    return true;
  }

  // The line starts a new text block if it is the first line of a footnote.
  std::cout << "Checking for footnote..." << std::endl;
  bool firstLineOfFootnote = isFirstLineOfFootnote(line);
  bool continuationLineOfFootnote = isContinuationLineOfFootnote(line);
  std::cout << "  is first: " << firstLineOfFootnote << std::endl;
  std::cout << "  is cont: " << continuationLineOfFootnote << std::endl;
  if (firstLineOfFootnote) {
    std::cout << "\033[1mBegin of new block (line is first line of footnote).\033[0m" << std::endl;
    return true;
  }
  // The line does not start a new text block if it is the continuation of a footnote.
  if (continuationLineOfFootnote) {
    std::cout << "\033[1mContinuation of block (line continues footnote).\033[0m" << std::endl;
    return false;
  }

  // The line starts a new text block if it is indented.
  std::cout << "Checking indentation..." << std::endl;
  std::cout << " └─ most freq. line indent: " << _doc->mostFreqLineIndent << std::endl;
  std::cout << " └─ actual line indent:     " << line->indent << std::endl;
  if (isIndented(line)) {
    std::cout << "\033[1mBegin of new block (line is indented).\033[0m" << std::endl;
    return true;
  }

  // The line starts a new text block if it has a different alignment than the previous line.
  std::cout << "Checking alignments..." << std::endl;
  std::cout << " └─ prevLine.alignments:";
  for (const auto& a : prevLine->alignments) { std::cout << " " << a; }
  std::cout << std::endl << " └─ currLine.alignments:";
  for (const auto& a : line->alignments) { std::cout << " " << a; }
  std::cout << std::endl;
  bool hasEqualAlignment = prevLine->alignments.empty() && line->alignments.empty();

  if (!hasEqualAlignment) {
    for (const auto& alignment : line->alignments) {
      if (prevLine->alignments.count(alignment) > 0) {
        hasEqualAlignment = true;
        break;
      }
    }
  }
  if (!hasEqualAlignment) {
    bool prevLineIndented = larger(prevLine->indent, 0, _doc->avgGlyphWidth) && equalOrSmaller(prevLine->indent, _doc->mostFreqLineIndent, _doc->avgGlyphWidth);
    hasEqualAlignment = prevLineIndented && line->alignments.count(PdfTextLineAlignment::LEFT) > 0;
  }

  if (!hasEqualAlignment) {
    std::cout << "\033[1mBegin of new block (no common alignments).\033[0m" << std::endl;
    return true;
  }
  return false;
}

// _________________________________________________________________________________________________
void TextBlockDetector::createTextBlock(const std::vector<PdfTextLine*>& lines,
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

  block->isEmphasized = computeIsTextBlockEmphasized(lines);

  block->lines = lines;

  // Set the rank.
  block->rank = blocks->size();

  blocks->push_back(block);
}

// =================================================================================================

// _________________________________________________________________________________________________
bool TextBlockDetector::computeIsTextBlockEmphasized(const std::vector<PdfTextLine*>& lines) {
  const PdfFontInfo* docFontInfo = _doc->fontInfos.at(_doc->mostFreqFontName);

  for (size_t i = 0; i < lines.size(); i++) {
    PdfTextLine* line = lines[i];
    const PdfFontInfo* lineFontInfo = _doc->fontInfos.at(line->fontName);

    // The line is emphasized if ...
    bool isLineEmphasized = false;

    // ... its font size is significantly larger than the most frequent font size in the document.
    if (line->fontSize - _doc->mostFreqFontSize > 0.5) {
      isLineEmphasized = true;
    }

    // ... its font weight is larger than the most frequent font weight.
    if (line->fontSize - _doc->mostFreqFontSize >= -1
        && lineFontInfo->weight > docFontInfo->weight) {
      isLineEmphasized = true;
    }

    // ... the line is printed in italics.
    if (line->fontSize - _doc->mostFreqFontSize >= -1 && lineFontInfo->isItalic) {
      isLineEmphasized = true;
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
      isLineEmphasized = true;
    }

    if (!isLineEmphasized) {
      return false;
    }
  }

  return true;
}

// ______________________________________________________________________________________________
void TextBlockDetector::computeMostFrequentLinePitch() {
  std::unordered_map<double, int> linePitchCounts;

  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      for (size_t i = 1; i < segment->lines.size(); i++) {
        PdfTextLine* prevLine = segment->lines.at(i - 1);
        PdfTextLine* currLine = segment->lines.at(i);

        if (!prevLine || !currLine) {
          continue;
        }

        if (prevLine->position->pageNum != currLine->position->pageNum) {
          continue;
        }

        if (prevLine->position->wMode != 0 || currLine->position->wMode != 0) {
          continue;
        }

        if (prevLine->position->rotation != 0 || currLine->position->rotation != 0) {
          continue;
        }

        double linePitch = round(currLine->baseBBoxUpperY - prevLine->baseBBoxLowerY, 1);
        linePitch = std::max(0.0, linePitch);

        linePitchCounts[linePitch]++;
      }
    }
  }

  int mostFreqLinePitchCount = 0;
  for (const auto& pair : linePitchCounts) {
    if (pair.second > mostFreqLinePitchCount) {
      _mostFreqLinePitch = pair.first;
      mostFreqLinePitchCount = pair.second;
    }
  }
}

// ______________________________________________________________________________________________
void TextBlockDetector::computeMostFrequentLinePitchPerPage() {
  for (auto* page : _doc->pages) {
    std::unordered_map<double, int> linePitchCountsOfPage;
    for (auto* segment : page->segments) {
      for (size_t i = 1; i < segment->lines.size(); i++) {
        PdfTextLine* prevLine = segment->lines.at(i - 1);
        PdfTextLine* currLine = segment->lines.at(i);

        if (!prevLine || !currLine) {
          continue;
        }

        if (prevLine->position->pageNum != currLine->position->pageNum) {
          continue;
        }

        if (prevLine->position->wMode != 0 || currLine->position->wMode != 0) {
          continue;
        }

        if (prevLine->position->rotation != 0 || currLine->position->rotation != 0) {
          continue;
        }

        double linePitch = round(currLine->baseBBoxUpperY - prevLine->baseBBoxLowerY, 1);
        linePitch = std::max(0.0, linePitch);

        linePitchCountsOfPage[linePitch]++;
      }
    }
    double mostFreqLinePitch = 0;
    int mostFreqLinePitchCount = 0;
    for (const auto& pair : linePitchCountsOfPage) {
      if (pair.second > mostFreqLinePitchCount and pair.second > 1) {
        mostFreqLinePitch = pair.first;
        mostFreqLinePitchCount = pair.second;
      }
    }

    _mostFreqLinePitchPerPage[page->pageNum] = mostFreqLinePitch;
  }
}

// _________________________________________________________________________________________________
void TextBlockDetector::computeMostFrequentLinePitchPerFontSize() {
  // Compute the most frequent height-pitch ratio, per font size.
  std::unordered_map<double, std::unordered_map<double, int>> linePitchCountsPerFontSize;

  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      for (size_t i = 1; i < segment->lines.size(); i++) {
        PdfTextLine* prevLine = segment->lines.at(i - 1);
        PdfTextLine* currLine = segment->lines.at(i);

        if (!prevLine || !currLine) {
          continue;
        }

        if (prevLine->position->pageNum != currLine->position->pageNum) {
          continue;
        }

        if (prevLine->position->wMode != 0 || currLine->position->wMode != 0) {
          continue;
        }

        if (prevLine->position->rotation != 0 || currLine->position->rotation != 0) {
          continue;
        }

        if (!equal(prevLine->fontSize, currLine->fontSize, 0.01)) {
          continue;
        }

        double fontSize = round(currLine->fontSize, 1);
        double linePitch = round(currLine->baseBBoxUpperY - prevLine->baseBBoxLowerY, 1);
        linePitch = std::max(0.0, linePitch);

        linePitchCountsPerFontSize[fontSize][linePitch]++;
      }
    }
  }

  std::unordered_map<double, int> mostFreqLinePitchCountPerFontSize;
  for (const auto& doubleMapPair : linePitchCountsPerFontSize) {
    const double fontSize = doubleMapPair.first;
    const std::unordered_map<double, int>& linePitchFreqs = doubleMapPair.second;
    for (const auto& doubleIntPair : linePitchFreqs) {
      double linePitch = doubleIntPair.first;
      double count = doubleIntPair.second;
      int mostFreqCount = 0;
      if (mostFreqLinePitchCountPerFontSize.count(fontSize) > 0) {
        mostFreqCount = mostFreqLinePitchCountPerFontSize.at(fontSize);
      }
      if (count > mostFreqCount) {
        _mostFreqLinePitchPerFontSize[fontSize] = linePitch;
        mostFreqLinePitchCountPerFontSize[fontSize] = count;
      }
    }
  }
}

bool TextBlockDetector::isFirstLineOfItem(const PdfTextLine* line) const {
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

bool TextBlockDetector::isContinuationLineOfItem(const PdfTextLine* line) const {
  return line->parentTextLine && isFirstLineOfItem(line->parentTextLine);
}

bool TextBlockDetector::isFirstLineOfFootnote(const PdfTextLine* line) const {
  if (!line) {
    return false;
  }

  if (line->text.size() == 0) {
    return false;
  }

  if (!smaller(line->fontSize, _doc->mostFreqFontSize, .9)) {
    return false;
  }

  PdfGlyph* firstGlyph = line->words[0]->glyphs[0];
  std::smatch m1;
  if (firstGlyph->isSuperscript && std::regex_search(firstGlyph->text, m1, isDigitRegex)) {
    return true;
  }

  std::smatch m2;
  bool startsWithAnchor = std::regex_search(firstGlyph->text, m2, isFootnoteMarkerRegex);
  if (startsWithAnchor) {
    return true;
  }

  return false;
}

bool TextBlockDetector::isContinuationLineOfFootnote(const PdfTextLine* line) const {
  return line->parentTextLine && isFirstLineOfFootnote(line->parentTextLine);
}

bool TextBlockDetector::isIndented(const PdfTextLine* line) const {
  return larger(line->indent, 0, _doc->avgGlyphWidth)
      && equalOrSmaller(line->indent, _doc->mostFreqLineIndent, _doc->avgGlyphWidth);
}

bool TextBlockDetector::isPartOfFigure(const PdfTextLine* line) const {
  PdfPage* page = _doc->pages.at(line->position->pageNum - 1);
  for (const auto* figure : page->figures) {
    double minRightX = std::min(figure->position->rightX, line->position->rightX);
    double maxLeftX = std::max(figure->position->leftX, line->position->leftX);
    double xOverlapLength = std::max(0.0, minRightX - maxLeftX);
    double width = line->position->getWidth();
    double xOverlapRatio = width > 0 ? xOverlapLength / width : 0;

    double minLowerY = std::min(figure->position->lowerY, line->position->lowerY);
    double maxUpperY = std::max(figure->position->upperY, line->position->upperY);
    double yOverlapLength = std::max(0.0, minLowerY - maxUpperY);
    double height = line->position->getHeight();
    double yOverlapRatio = height > 0 ? yOverlapLength / height : 0;

    if (xOverlapRatio > 0.5 && yOverlapRatio > 0.5) {
      return true;
    }
    // if (contains(figure, line, _doc->avgGlyphHeight)) {
    //   return true;
    // }
  }
  return false;
}

bool TextBlockDetector::isDisplayFormula(const PdfTextLine* line) const {
  std::unordered_set<std::string> displayMathGlyphNames({
    "summationdisplay",
    "summationssdisplay",
    "circledotdisplay",
    "circlemultiplydisplay",
    "circleplusdisplay",
    "contintegraldisplay",
    "coproductdisplay",
    "integraldisplay",
    "intersectiondisplay",
    "logicalanddisplay",
    "logicalordisplay",
    "productdisplay",
    "summationdisplay",
    "uniondisplay",
    "unionmultidisplay",
    "unionsqdisplay"
  });

  // Check if the line contains a glyph that hints at a display formula.
  for (auto* word : line->words) {
    for (auto* glyph : word->glyphs) {
      if (displayMathGlyphNames.count(glyph->charName)) {
        return true;
      }
    }
  }

  // Check if the line ends with a formula marker of form (1), (2), (3), etc., and contains at
  // least one symbol that hints unambiguously at a formula.
  std::smatch m;
  bool hasFormulaMarker = std::regex_search(line->text, m, formula_marker_regex);
  bool hasMathSymbol = line->text.find("=") != std::string::npos;

  return hasFormulaMarker && hasMathSymbol;
}

// =================================================================================================

void TextBlockDetector::computeTextLineAlignments(const std::vector<PdfTextBlock*>& blocks) {
  std::stack<PdfTextLine*> lineStack;
  for (auto* block : blocks) {
    for (auto* line : block->lines) {
      // Compute the alignments.
      double leftMargin = round(line->position->getRotLeftX() - block->position->getRotLeftX(), 1);
      double rightMargin = round(block->position->getRotRightX() - line->position->getRotRightX(), 1);

      if (equal(leftMargin, 0, 2 * _doc->avgGlyphWidth) && equal(rightMargin, 0, 2 * _doc->avgGlyphWidth)) {
        line->alignments.insert(PdfTextLineAlignment::JUSTIFIED);
      }

      if (equal(leftMargin, rightMargin, 2 * _doc->avgGlyphWidth)) {
        line->alignments.insert(PdfTextLineAlignment::CENTERED);
      }

      if (larger(leftMargin, 2 * _doc->avgGlyphWidth, 0) && larger(rightMargin, 2 * _doc->avgGlyphWidth, 0)) {
        line->alignments.insert(PdfTextLineAlignment::CENTERED);
      }

      if (equal(leftMargin, 0, 2 * _doc->avgGlyphWidth)) {
        line->alignments.insert(PdfTextLineAlignment::LEFT);
      }

      if (equal(rightMargin, 0, 2 * _doc->avgGlyphWidth)) {
        line->alignments.insert(PdfTextLineAlignment::RIGHT);
      }

      // Compute the indentation.
      line->indent = !equal(leftMargin, rightMargin, 1) ? leftMargin : 0;
    }
  }
}

void TextBlockDetector::computeTextLineIndentHierarchies() {
  std::stack<PdfTextLine*> lineStack;
  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      for (auto* line : segment->lines) {
        // Compute the indentation, relatively to the segment boundaries.
        line->indent = round(line->position->getRotLeftX() - segment->position->getRotLeftX(), 1);

        while (!lineStack.empty()) {
          double tolerance = 1.0;
          if (lineStack.top()->position->pageNum != line->position->pageNum) {
            tolerance = 7.5;
          }
          if (!larger(lineStack.top()->indent, line->indent, tolerance)) {
            break;
          }
          lineStack.pop();
        }

        if (lineStack.empty()) {
          lineStack.push(line);
          continue;
        }

        // If the line is positioned on another page than the topmost page, allow a larger
        // threshold. This is because there could be a 2-page layout, where the left margin
        // could differ between even pages and odd pages (hep-ex0205091:9/10).
        double tolerance = 1.0;
        if (lineStack.top()->position->pageNum != line->position->pageNum) {
          tolerance = 7.5;
        }
        if (equal(lineStack.top()->indent, line->indent, tolerance)) {
          lineStack.top()->nextSiblingTextLine = line;
          line->prevSiblingTextLine = lineStack.top();
          line->parentTextLine = lineStack.top()->parentTextLine;
          lineStack.pop();
          lineStack.push(line);
          continue;
        }

        tolerance = 1.0;
        if (lineStack.top()->position->pageNum != line->position->pageNum) {
          tolerance = 7.5;
        }
        if (smaller(lineStack.top()->indent, line->indent, tolerance)) {
          line->parentTextLine = lineStack.top();

          lineStack.push(line);
          continue;
        }
      }
    }
  }
}
