/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <regex>
#include <string>
#include <unordered_set>
#include <vector>
#include <stack>

#include "../Constants.h"

#include "./MathUtils.h"
#include "./PdfElementsUtils.h"
#include "./TextLinesUtils.h"

using std::string;
using std::unordered_set;
using std::vector;

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

  double avgGlyphWidth = line->doc->avgGlyphWidth;

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
    bool isPrevPrefixedByLabel = text_lines_utils::computeIsPrefixedByItemLabel(line->prevLine);
    bool hasEqualFont = text_element_utils::computeHasEqualFont(line->prevLine, line);
    bool hasEqualFontSize = text_element_utils::computeHasEqualFontSize(line->prevLine, line);
    double distance = element_utils::computeVerticalGap(line->prevLine, line);
    bool hasNegativeDistance = math_utils::equalOrSmaller(distance, 0);
    bool hasSentenceDelim = text_element_utils::computeEndsWithSentenceDelimiter(line->prevLine);
    bool hasEqualLeftX = element_utils::computeHasEqualLeftX(line->prevLine, line, avgGlyphWidth);

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
  const vector<PdfWord*>& words = line->words;
  if (words.empty()) {
    return false;
  }

  // The line is not prefixed by an enumeration item label if the first word is empty.
  const vector<PdfGlyph*>& firstWordGlyphs = words[0]->glyphs;
  if (firstWordGlyphs.empty()) {
    return false;
  }

  // The line is prefixed by an enumeration item label if the first glyph is superscripted and if
  // it is contained in our alphabet we defined for identifying superscripted item labels.
  // TODO(korzen): Instead of analyzing only the first glyph, we should analyze the first *word*.
  // This would identify also lines that are prefixed by something like "a)".
  PdfGlyph* glyph = firstWordGlyphs[0];
  string glyphStr = glyph->text;
  if (glyph->isSuperscript && strstr(SUPER_ITEM_LABEL_ALPHABET, glyphStr.c_str()) != nullptr) {
    return true;
  }

  // The line is prefixed by an enumeration item label if it matches one of our regexes we defined
  // for identifying item labels. The matching parts must not be superscripted.
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
  const vector<PdfWord*>& words = line->words;
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
void text_lines_utils::computeTextLineHierarchy(const PdfPage* page) {
  assert(page);

  // Do nothing if the page does not contain any segments.
  if (page->segments.empty()) {
    return;
  }

  // ----------
  // CONSTANTS

  const double MAX_LINE_DIST = 10.0;
  const double LEFT_X_OFFSET_TOLERANCE = page->segments[0]->doc->avgGlyphWidth;

  // ----------

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
          double absLineDistance = abs(element_utils::computeVerticalGap(prevLine, line));
          if (math_utils::larger(absLineDistance, MAX_LINE_DIST)) {
            lineStack = stack<PdfTextLine*>();
          }
        }
      }
      prevLine = line;

      // Remove all lines from the stack with a larger leftX than the current line, because
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

      // Ignore the current line if its lowerY is smaller than the lowerY of the
      // topmost line in the stack (that is: if the current line is positioned above the topmost
      // line in the stack). This should prevent to consider a line to be the parent line or a
      // sibling line of a line in a different column.
      double topStackLowerY = lineStack.top()->position->lowerY;
      double lineLowerY = line->position->lowerY;
      if (math_utils::equalOrLarger(topStackLowerY, lineLowerY)) {
        continue;
      }

      // Check if the topmost line in the stack has the same leftX than the current line
      // (under consideration of the given tolerance). If so, the following is true:
      // (1) the current line is the next sibling line of the topmost line in the stack;
      // (2) the topmost line in the stack is the previous sibling line of the current line;
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

      // Check if the topmost line in the stack has a smaller leftX than the current line
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
  assert(result);

  // Iterate through the glyphs of the word. For each glyph, check if it is a label that
  // potentially reference a footnote, that is: if it is a superscipted alphanumerical or if it
  // occurs in our alphabet we defined to identify special footnote labels. Merge each consecutive
  // glyph that is part of such a label and that are positioned behind the word (we don't want to
  // consider labels that are positioned in front of a word, since footnote labels are usually
  // positioned behind words).
  // TODO(korzen): We do not store the info about whether a superscript is positioned before or
  // after a word. As a workaround, consider a superscript as part of a potential footnote marker
  // only when a non-subscript and non-superscript was already seen.
  for (const auto* word : line->words) {
    string label;
    bool nonSubSuperscriptSeen = false;
    for (const auto* glyph : word->glyphs) {
      // Ignore sub- and superscripts that are positioned before the word.
      if (!nonSubSuperscriptSeen && !glyph->isSubscript && !glyph->isSuperscript) {
        nonSubSuperscriptSeen = true;
        continue;
      }
      // Ignore the glyph when no glyph which is not a subscript and superscript was seen yet.
      if (!nonSubSuperscriptSeen) {
        continue;
      }
      // Ignore the glyph when it does not contain any text.
      if (glyph->text.empty()) {
        continue;
      }

      // The glyph is part of a potential footnote label when it occurs in our alphabet we defined
      // to identify special (= non-alphanumerical) footnote labels.
      bool isLabel = strchr(SPECIAL_FOOTNOTE_LABELS_ALPHABET, glyph->text[0]) != nullptr;

      // The glyph is also a potential footnote label when it is a superscripted alphanumerical.
      if (glyph->isSuperscript && isalnum(glyph->text[0])) {
        isLabel = true;
      }

      // When the glyph is part of a potential footnote label, add it to the current label string.
      if (isLabel) {
        label += glyph->text;
        continue;
      }

      // Otherwise the end of a potential label is reached. When the current label string is not
      // empty, append it to the result list.
      if (!label.empty()) {
        result->insert(label);
        label.clear();
      }
    }

    // Don't forget to add the last label string to the result list (if it is not empty).
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
