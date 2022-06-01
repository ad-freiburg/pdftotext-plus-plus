/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <regex>
#include <iostream>
#include <unordered_set>

#include "./MathUtils.h"
#include "./PdfElementUtils.h"
#include "./TextLinesUtils.h"

using namespace std;

// _________________________________________________________________________________________________
double text_lines_utils::computeTextLineDistance(const PdfTextLine* l1, const PdfTextLine* l2) {
  assert(l1);
  assert(l2);
  assert(l1->position->pageNum == l2->position->pageNum);
  assert(l1->position->rotation == l2->position->rotation);
  assert(l1->position->wMode == l2->position->wMode);

  // Determine which of the two lines is the first line (which of the two is positioned "above" the
  // other by checking which of the two lines has the lower upperY value.
  const PdfTextLine* upperLine;
  const PdfTextLine* lowerLine;
  if (l1->position->upperY < l2->position->upperY) {
    upperLine = l1;
    lowerLine = l2;
  } else {
    upperLine = l2;
    lowerLine = l1;
  }

  switch(l1->position->rotation) {
    case 0:
    case 1:
    default:
      return lowerLine->position->upperY - upperLine->position->lowerY;
    case 2:
    case 3:
      return upperLine->position->lowerY - lowerLine->position->upperY;
  }
}

// _________________________________________________________________________________________________
bool text_lines_utils::computeIsFirstLineOfItem(const PdfTextLine* line,
      const unordered_set<string>* potentialFootnoteLabels) {
  assert(line);

  // The line is not the first line of an item if it does not contain any words.
  if (line->words.empty()) {
    return false;
  }

  // The line is not the first line of an item if it is not prefixed by an item label.
  bool isPrefixedByItemLabel = text_lines_utils::computeIsPrefixedByItemLabel(line);
  if (!isPrefixedByItemLabel) {
    return false;
  }

  // EXPERIMENTAL: The line is not the first line of a footnote when all of the following
  // requirements are fulfilled:
  // (1) the previous line is not prefixed by an item label;
  // (2) the previous line and the current line have the same font;
  // (3) the previous line and the current line have the same font size;
  // (4) the distance between the previous and current line is <= 0
  // (5) the previous line does not end with a sentence delimiter
  // (6) the previous and current line have the same leftX value.
  // This should avoid to detect lines that occasionally start with a footnote label, but that are
  // actually not part of a footnote, as a footnote. Example: 0901.4737, page 11.
  if (line->prevLine) {
    bool isPrevPrefixedByLabel = text_lines_utils::computeIsPrefixedByItemLabel(line->prevLine);
    bool hasEqualFont = text_element_utils::computeHasEqualFont(line->prevLine, line);
    bool hasEqualFontSize = text_element_utils::computeHasEqualFontSize(line->prevLine, line);
    double distance = text_lines_utils::computeTextLineDistance(line->prevLine, line);
    bool hasNegativeDistance = math_utils::equalOrSmaller(distance, 0);
    bool hasSentenceDelim = text_element_utils::computeEndsWithSentenceDelimiter(line->prevLine);
    bool hasEqualLeftX = element_utils::computeHasEqualLeftX(line->prevLine, line);

    if (!isPrevPrefixedByLabel && hasEqualFont && hasEqualFontSize && hasNegativeDistance
          && !hasSentenceDelim && hasEqualLeftX) {
      return false;
    }
  }

  // Check if there is a previous sibling line. The current line is the first line of an item
  // if the previous sibling line is also prefixed by an item label and if it exhibits the same
  // font and font size as the current line.
  const PdfTextLine* prevSibling = line->prevSiblingLine;
  if (prevSibling && !prevSibling->words.empty()) {
    PdfWord* firstWord = line->words[0];
    PdfWord* prevFirstWord = prevSibling->words[0];
    bool prevIsPrefixedByItemLabel = text_lines_utils::computeIsPrefixedByItemLabel(prevSibling);
    bool hasEqualFont = text_element_utils::computeHasEqualFont(prevFirstWord, firstWord);
    bool hasEqualFontSize = text_element_utils::computeHasEqualFontSize(prevFirstWord, firstWord);
    if (prevIsPrefixedByItemLabel && hasEqualFont && hasEqualFontSize) {
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
    bool nextIsPrefixedByItemLabel = text_lines_utils::computeIsPrefixedByItemLabel(nextSibling);
    bool hasEqualFont = text_element_utils::computeHasEqualFont(nextFirstWord, firstWord);
    bool hasEqualFontSize = text_element_utils::computeHasEqualFontSize(nextFirstWord, firstWord);
    if (nextIsPrefixedByItemLabel && hasEqualFont && hasEqualFontSize) {
      return true;
    }
  }

  // The line is the first line of an item if it starts with a footnote label.
  if (text_lines_utils::computeIsPrefixedByFootnoteLabel(line, potentialFootnoteLabels)) {
    return true;
  }

  return false;
}

// _________________________________________________________________________________________________
bool text_lines_utils::computeIsContinuationOfItem(const PdfTextLine* line,
      const unordered_set<string>* potentialFootnoteLabels) {
  assert(line);

  // The line is not a continuation of an item (a footnote) if it does not have a parent line.
  const PdfTextLine* parentLine = line->parentLine;
  if (!parentLine) {
    return false;
  }

  // The line is a continuation of an item (a footnote) if the parent line is the first line or a
  // continuation of an item (a footnote).
  return text_lines_utils::computeIsFirstLineOfItem(parentLine, potentialFootnoteLabels)
      || text_lines_utils::computeIsContinuationOfItem(parentLine, potentialFootnoteLabels);
}

// _________________________________________________________________________________________________
bool text_lines_utils::computeIsPrefixedByItemLabel(const PdfTextLine* line) {
  assert(line);

  // The line is not prefixed by an enumeration item label if it does not contain any words.
  const std::vector<PdfWord*>& words = line->words;
  if (words.empty()) {
    return false;
  }

  // The line is not prefixed by an enumeration item label if the first word is empty.
  const std::vector<PdfGlyph*>& firstWordGlyphs = words[0]->glyphs;
  if (firstWordGlyphs.empty()) {
    return false;
  }

  // The line is prefixed by an enumeration item label if the first glyph is superscripted and if
  // it is contained in our alphabet we defined for identifying superscripted item labels.
  // TODO: Instead of analyzing only the first glyph, we should analyze the first *word*. This
  // would identify also lines that are prefixed by something like "a)".
  PdfGlyph* firstGlyph = firstWordGlyphs[0];
  string firstGlyphStr = firstGlyph->text;
  if (firstGlyph->isSuperscript && SUPER_ITEM_LABEL_ALPHABET.find(firstGlyphStr) != string::npos) {
    return true;
  }

  // The line is prefixed by an enumeration item label if it matches one of our regexes we defined
  // for identifying item labels.
  smatch m;
  for (const auto& regex : ITEM_LABEL_REGEXES) {
    if (regex_search(line->text, m, regex)) {
      return true;
    }
  }

  return false;
}

// _________________________________________________________________________________________________
bool text_lines_utils::computeIsPrefixedByFootnoteLabel(const PdfTextLine* line,
      const unordered_set<string>* potentialFootnoteLabels) {
  assert(line);

  // The line is not prefixed by a footnote label if it does not contain any words.
  const std::vector<PdfWord*>& words = line->words;
  if (words.empty()) {
    return false;
  }

  // Compute the superscripted prefix of the line, that is: the concatenation of all superscripted
  // characters in front of the line.
  const PdfWord* firstWord = words[0];
  string superScriptPrefix;
  for (const auto* glyph : firstWord->glyphs) {
    if (!glyph->isSuperscript) {
      break;
    }
    superScriptPrefix += glyph->text;
  }

  // If potentialFootnoteLabels is specified, it must contain the superscripted prefix.
  if (potentialFootnoteLabels) {
    return potentialFootnoteLabels->count(superScriptPrefix) > 0;
  }

  // The superscripted prefix must not be empty.
  return !superScriptPrefix.empty();
}

// _________________________________________________________________________________________________
bool text_lines_utils::computeHasPrevLineCapacity(const PdfTextLine* line, double toleranceFactor) {
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
  double firstWordWidth = line->words[0]->position->getWidth();

  // The previous line has capacity if its right margin is larger than the width of the first word
  // of the given line, under consideration of a tolerance.
  double tolerance = toleranceFactor * line->doc->avgGlyphWidth;
  return math_utils::larger(line->prevLine->rightMargin, firstWordWidth, tolerance);
}


// _________________________________________________________________________________________________
void text_lines_utils::computeTextLineHierarchies(const PdfPage* page) {
  assert(page);

  // Do nothing if the page does not contain any segments.
  if (page->segments.empty()) {
    return;
  }

  const double MAX_LINE_DIST = 10.0;
  const double LEFT_X_OFFSET_TOLERANCE = page->segments[0]->doc->avgGlyphWidth;

  // Maintain a stack to keep track of the parent and sibling lines.
  stack<PdfTextLine*> lineStack;

  // Iterate through the lines and determine the parent line and the sibling lines for each.
  PdfTextLine* prevLine = nullptr;
  for (auto* segment : page->segments) {
    for (auto* line : segment->lines) {
      // Empty the stack if the distance between the line and the previous line is larger than the
      // threshold. This should prevent to consider a line to be the parent line or a sibling line
      // of another line when the distance between the lines is too large.
      if (prevLine) {
        bool hasSameRotation = prevLine->position->rotation == line->position->rotation;
        bool hasSameWMode = prevLine->position->wMode == line->position->wMode;
        if (hasSameRotation && hasSameWMode) {
          double absLineDistance = abs(text_lines_utils::computeTextLineDistance(prevLine, line));
          if (math_utils::larger(absLineDistance, MAX_LINE_DIST)) {
            lineStack = stack<PdfTextLine*>();
          }
        }
      }
      prevLine = line;

      // Remove all lines from the stack with a larger leftX value than the current line, because
      // they can't be a parent line or any sibling line of the current line.
      while (!lineStack.empty()) {
        double topStackLeftX = lineStack.top()->position->leftX;
        double lineLeftX = line->position->leftX;
        if (!math_utils::larger(topStackLeftX, lineLeftX, LEFT_X_OFFSET_TOLERANCE)) {
          break;
        }
        lineStack.pop();
      }

      // If the stack is empty, the current line does not have any parent line or sibling lines.
      // Push the line to the stack.
      if (lineStack.empty()) {
        lineStack.push(line);
        continue;
      }

      // Ignore the current line if its lowerY value is smaller than the lowerY value of the
      // topmost line in the stack (that is: if the current line is positioned above the topmost
      // line in). This should prevent to consider a line to be the parent line or a sibling
      // line of a line in a different column.
      double topStackLowerY = lineStack.top()->position->lowerY;
      double lineLowerY = line->position->lowerY;
      if (math_utils::equalOrLarger(topStackLowerY, lineLowerY)) {
        continue;
      }

      // Check if the topmost line in the stack has the same leftX value than the current line
      // (under consideration of the given tolerance). If so, the following is true:
      // (1) the current line is the next sibling line of the topmost line in the stack
      // (2) the topmost line in the stack is the previous sibling line of the current line.
      // (3) the parent line of the topmost line in the stack is also the parent line of the
      //     current line.
      double topStackLeftX = lineStack.top()->position->leftX;
      double lineLeftX = line->position->leftX;
      if (math_utils::equal(topStackLeftX, lineLeftX, LEFT_X_OFFSET_TOLERANCE)) {
        lineStack.top()->nextSiblingLine = line;
        line->prevSiblingLine = lineStack.top();
        line->parentLine = lineStack.top()->parentLine;
        lineStack.pop();
        lineStack.push(line);
        continue;
      }

      // Check if the topmost line in the stack has a smaller leftX value than the current line
      // (under consideration of the given tolerance). If so, the topmost line in the stack is the
      // parent line of the current line.
      if (math_utils::smaller(topStackLeftX, lineLeftX, LEFT_X_OFFSET_TOLERANCE)) {
        line->parentLine = lineStack.top();

        lineStack.push(line);
        continue;
      }
    }
  }
}

// _________________________________________________________________________________________________
void text_lines_utils::computePotentialFootnoteLabels(const PdfTextLine* line,
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
bool text_lines_utils::computeIsCentered(const PdfTextLine* line1, const PdfTextLine* line2,
      double xOffsetToleranceFactor) {
  assert(line1);
  assert(line2);

  // The lines are not centered when neither the first line nor the second line is completely
  // overlapped horizontally by the respective other line.
  double maxXOverlapRatio = element_utils::computeMaxXOverlapRatio(line1, line2);
  if (math_utils::smaller(maxXOverlapRatio, 1, 0.01)) {
    return false;
  }

  // The lines are not centered when the leftX-offset and the rightX-offset between the lines
  // is not equal).
  double absLeftXOffset = abs(element_utils::computeLeftXOffset(line1, line2));
  double absRightXOffset = abs(element_utils::computeRightXOffset(line1, line2));
  double tolerance = xOffsetToleranceFactor * line1->doc->avgGlyphWidth;
  if (!math_utils::equal(absLeftXOffset, absRightXOffset, tolerance)) {
    return false;
  }

  return true;
}