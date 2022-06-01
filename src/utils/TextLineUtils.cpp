/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <regex>
#include <unordered_set>

#include "./MathUtils.h"
#include "./PdfElementUtils.h"
#include "./TextLineUtils.h"

using namespace std;

// _________________________________________________________________________________________________
double text_line_utils::computeTextLineDistance(const PdfTextLine* prevLine,
      const PdfTextLine* line) {
  assert(prevLine);
  assert(line);
  assert(prevLine->position->pageNum == line->position->pageNum);
  assert(prevLine->position->rotation == line->position->rotation);
  assert(prevLine->position->wMode == line->position->wMode);

  switch(prevLine->position->rotation) {
    case 0:
    case 1:
    default:
      return line->position->getRotUpperY() - prevLine->position->getRotLowerY();
    case 2:
    case 3:
      return prevLine->position->getRotLowerY() - line->position->getRotUpperY();
      break;
  }
}

// _________________________________________________________________________________________________
bool text_line_utils::computeIsFirstLineOfItem(const PdfTextLine* line,
      const unordered_set<string>* potentialFootnoteLabels) {
  assert(line);

  if (line->words.empty()) {
    return false;
  }

  // The line is the first line of an item if it is prefixed by an item label, and there is a
  // previous and/or next sibling text line that is also prefixed by an item label.
  bool isPrefixedByItemLabel = text_line_utils::computeIsPrefixedByItemLabel(line);
  if (!isPrefixedByItemLabel) {
    return false;
  }

  // EXPERIMENTAL: The line is not the first line of a footnote when (1) the font of the line is
  // equal to the font of the previous line, (2) the line distance to the previous line is <= 0;
  // and (3) the previous line does not end with a punctuation mark. This should avoid to detect
  // lines that occasionally starts with a superscripted number as a footnote. Example:
  // 0901.4737:11.
  if (line->prevLine) {
    bool isPrevPrefixedByItemLabel = text_line_utils::computeIsPrefixedByItemLabel(line->prevLine);
    bool hasEqualFont = text_element_utils::computeHasEqualFont(line->prevLine, line);
    bool hasEqualFontSize = text_element_utils::computeHasEqualFontSize(line->prevLine, line, 0.5);  // TODO
    double distance = text_line_utils::computeTextLineDistance(line->prevLine, line);
    bool hasNegativeDistance = math_utils::equalOrSmaller(distance, 0, 0.0001);  // TODO
    bool hasSentenceDelim = text_element_utils::computeEndsWithSentenceDelimiter(line->prevLine);
    bool hasEqualLeftX = element_utils::computeHasEqualLeftX(line->prevLine, line);

    if (!isPrevPrefixedByItemLabel && hasEqualFont && hasEqualFontSize
          && hasNegativeDistance && !hasSentenceDelim && hasEqualLeftX) {
      return false;
    }
  }

  PdfWord* firstWord = line->words[0];

  const PdfTextLine* prevSibling = line->prevSiblingLine;
  if (prevSibling && !prevSibling->words.empty()) {
    PdfWord* prevFirstWord = prevSibling->words[0];
    bool prevIsPrefixedByItemLabel = text_line_utils::computeIsPrefixedByItemLabel(prevSibling);
    bool hasEqualFont = text_element_utils::computeHasEqualFont(prevFirstWord, firstWord);
    bool hasEqualFontSize = text_element_utils::computeHasEqualFontSize(prevFirstWord, firstWord, 0.5);  // TODO
    bool isPrevSiblingItem = prevIsPrefixedByItemLabel && hasEqualFont && hasEqualFontSize;
    if (isPrevSiblingItem) {
      return true;
    }
  }

  const PdfTextLine* nextSibling = line->nextSiblingLine;
  if (nextSibling && !nextSibling->words.empty()) {
    PdfWord* nextFirstWord = nextSibling->words[0];
    bool nextIsPrefixedByItemLabel = text_line_utils::computeIsPrefixedByItemLabel(nextSibling);
    bool hasEqualFont = text_element_utils::computeHasEqualFont(nextFirstWord, firstWord);
    bool hasEqualFontSize = text_element_utils::computeHasEqualFontSize(nextFirstWord, firstWord, 0.5);  // TODO
    bool isNextSiblingItem = nextIsPrefixedByItemLabel && hasEqualFont && hasEqualFontSize;
    if (isNextSiblingItem) {
      return true;
    }
  }

  // The line is the first line of an item if it starts with a footnote label.
  if (text_line_utils::computeIsPrefixedByFootnoteLabel(line, potentialFootnoteLabels)) {
    return true;
  }

  return false;
}

// _________________________________________________________________________________________________
bool text_line_utils::computeIsContinuationOfItem(const PdfTextLine* line,
      const unordered_set<string>* potentialFootnoteLabels) {
  assert(line);

  const PdfTextLine* parentLine = line->parentLine;
  if (!parentLine) {
    return false;
  }

  return text_line_utils::computeIsFirstLineOfItem(parentLine, potentialFootnoteLabels)
      || text_line_utils::computeIsContinuationOfItem(parentLine, potentialFootnoteLabels);
}

// _________________________________________________________________________________________________
bool text_line_utils::computeIsPrefixedByItemLabel(const PdfTextLine* line) {
  assert(line);

  const std::vector<PdfWord*>& words = line->words;
  if (words.empty()) {
    return false;
  }

  const std::vector<PdfGlyph*>& firstWordGlyphs = words[0]->glyphs;
  if (firstWordGlyphs.empty()) {
    return false;
  }

  // The line is prefixed by an item label if the first glyph is superscripted and it is contained
  // in our alphabet we defined for identifying superscripted item labels.
  // TODO: Instead of analyzing only the first glyph, should we analyze the first *word*? This
  // would identify also lines that are prefixed by something like "a)".
  PdfGlyph* firstGlyph = firstWordGlyphs[0];
  string firstGlyphStr = firstGlyph->text;
  if (firstGlyph->isSuperscript && SUPER_ITEM_LABEL_ALPHABET.find(firstGlyphStr) != string::npos) {
    return true;
  }

  // The line is also prefixed by an item label if it matches one of our regexes we defined for
  // identifying item labels.
  smatch m;
  for (const auto& regex : ITEM_LABEL_REGEXES) {
    if (regex_search(line->text, m, regex)) {
      return true;
    }
  }

  return false;
}

// _________________________________________________________________________________________________
bool text_line_utils::computeIsPrefixedByFootnoteLabel(const PdfTextLine* line,
      const unordered_set<string>* potentialFootnoteLabels) {
  assert(line);

  const std::vector<PdfWord*>& words = line->words;
  if (words.empty()) {
    return false;
  }

  const PdfWord* firstWord = words[0];
  string superScriptPrefix;
  for (const auto* glyph : firstWord->glyphs) {
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

// _________________________________________________________________________________________________
bool text_line_utils::computeHasPrevLineCapacity(const PdfTextLine* line) {
  assert(line);

  if (!line->prevLine) {
    return false;
  }

  if (line->words.empty()) {
    return false;
  }

  const PdfWord* firstWord = line->words[0];
  double firstWordWidth = firstWord->position->getWidth();

  // TODO: Parameterize the 2.
  return math_utils::larger(line->prevLine->rightMargin, firstWordWidth, 2 * line->doc->avgGlyphWidth);
}

// _________________________________________________________________________________________________
void text_line_utils::computeTextLineIndentHierarchies(const PdfPage* page) {
  stack<PdfTextLine*> lineStack;
  PdfTextLine* prevLine = nullptr;
  for (auto* segment : page->segments) {
    for (auto* line : segment->lines) {
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
        if (math_utils::larger(abs(actualLineDistance), max(10.0, 3 * line->doc->mostFreqLineDistance))) {
          lineStack = stack<PdfTextLine*>();
        }
      }
      prevLine = line;

      while (!lineStack.empty()) {
        if (!math_utils::larger(lineStack.top()->position->leftX, line->position->leftX, line->doc->avgGlyphWidth)) {
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
        if (math_utils::equal(lineStack.top()->position->leftX, line->position->leftX, line->doc->avgGlyphWidth)) {
          lineStack.top()->nextSiblingLine = line;
          line->prevSiblingLine = lineStack.top();
          line->parentLine = lineStack.top()->parentLine;
          lineStack.pop();
          lineStack.push(line);
          continue;
        }

        if (math_utils::smaller(lineStack.top()->position->leftX, line->position->leftX, line->doc->avgGlyphWidth)) {
          line->parentLine = lineStack.top();

          lineStack.push(line);
          continue;
        }
      }
    }
  }
}

// _________________________________________________________________________________________________
void text_line_utils::computePotentialFootnoteLabels(const PdfTextLine* line,
      unordered_set<string>* result) {
  assert(line);

  // Iterate through the glyphs of the word and merge each adjacent superscripts which are
  // positioned before the word (we don't want to consider superscript that are positioned
  // behind the word). Consider each merged superscript string as a potential footnote marker.
  // TODO: We do not store the info about whether a superscript is positioned before or after
  // a word. As a workaround, consider a superscript as part of a potential footnote marker
  // only when a non-subscript and non-superscript was already seen.
  for (const auto* word : line->words) {
    bool nonSubSuperscriptSeen = false;
    string label;
    for (const auto* glyph : word->glyphs) {
      if (!nonSubSuperscriptSeen && !glyph->isSubscript && !glyph->isSuperscript) {
        nonSubSuperscriptSeen = true;
        continue;
      }
      if (!nonSubSuperscriptSeen) {
        continue;
      }

      bool isLabel = glyph->isSuperscript && !glyph->text.empty() && isalnum(glyph->text[0]);
      isLabel |= !glyph->text.empty() && FOOTNOTE_LABEL_ALPHABET.find(glyph->text[0]) != string::npos;

      if (!isLabel) {
        if (!label.empty()) {
          result->insert(label);
          label.clear();
        }
        continue;
      }

      label += glyph->text;
    }
    if (!label.empty()) {
      result->insert(label);
    }
  }
}

// _________________________________________________________________________________________________
bool text_line_utils::computeIsCentered(const PdfTextLine* line1, const PdfTextLine* line2,
      double xOffsetToleranceFactor) {
  assert(line1);
  assert(line2);

  // The lines are not centered when neither the first line nor the second line is fully
  // overlapped horizontally by the respective other line.
  double maxXOverlapRatio = element_utils::computeMaxXOverlapRatio(line1, line2);
  if (math_utils::smaller(maxXOverlapRatio, 1, 0.01)) {
    return false;
  }

  // The lines are not centered when when the leftX-offset and the rightX-offset between the lines
  // is not equal).
  double absLeftXOffset = abs(element_utils::computeLeftXOffset(line1, line2));
  double absRightXOffset = abs(element_utils::computeRightXOffset(line1, line2));
  double tolerance = xOffsetToleranceFactor * line1->doc->avgGlyphWidth;
  if (!math_utils::equal(absLeftXOffset, absRightXOffset, tolerance)) {
    return false;
  }

  return true;
}