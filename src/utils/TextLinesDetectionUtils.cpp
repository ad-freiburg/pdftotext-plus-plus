/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <stack>
#include <tuple>
#include <utility>  // std::pair

#include "./Counter.h"
#include "./MathUtils.h"
#include "./PdfElementsUtils.h"
#include "./TextLinesDetectionUtils.h"
#include "../PdfDocument.h"

using std::make_tuple;
using std::pair;
using std::stack;

using ppp::types::PdfDocument;
using ppp::types::PdfPage;
using ppp::types::PdfPageSegment;
using ppp::types::PdfTextLine;
using ppp::utils::counter::DoubleCounter;
using ppp::utils::elements::computeVerticalGap;
using ppp::utils::math::equal;
using ppp::utils::math::equalOrLarger;
using ppp::utils::math::larger;
using ppp::utils::math::round;
using ppp::utils::math::smaller;

// =================================================================================================

namespace ppp::utils {

// _________________________________________________________________________________________________
TextLinesDetectionUtils::TextLinesDetectionUtils(const TextLinesDetectionConfig& config) {
  _config = config;
}

// _________________________________________________________________________________________________
TextLinesDetectionUtils::~TextLinesDetectionUtils() = default;

// _________________________________________________________________________________________________
void TextLinesDetectionUtils::computeTextLineHierarchy(const PdfPage* page) const {
  assert(page);

  // Do nothing if the page does not contain any segments.
  if (page->segments.empty()) {
    return;
  }

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
        bool hasSameRotation = prevLine->pos->rotation == line->pos->rotation;
        bool hasSameWMode = prevLine->pos->wMode == line->pos->wMode;
        if (hasSameRotation && hasSameWMode) {
          double absLineDistance = abs(computeVerticalGap(prevLine, line));
          if (larger(absLineDistance, _config.lineHierarchyMaxLineDist,
              _config.coordsEqualTolerance)) {
            lineStack = stack<PdfTextLine*>();
          }
        }
      }
      prevLine = line;

      const PdfDocument* doc = page->segments[0]->doc;
      // TODO(korzen): Move to config.
      double leftXOffsetThreshold =
        _config.textLineHierarchyLeftXOffsetThresholdFactor * doc->avgCharWidth;

      // Remove all lines from the stack with a larger leftX than the current line, because
      // they can't be a parent line or any sibling line of the current line.
      while (!lineStack.empty()) {
        double topStackLeftX = lineStack.top()->pos->leftX;
        double lineLeftX = line->pos->leftX;
        if (!larger(topStackLeftX, lineLeftX, leftXOffsetThreshold)) {
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
      double topStackLowerY = lineStack.top()->pos->lowerY;
      double lineLowerY = line->pos->lowerY;
      if (equalOrLarger(topStackLowerY, lineLowerY, _config.coordsEqualTolerance)) {
        continue;
      }

      // Check if the topmost line in the stack has the same leftX than the current line
      // (under consideration of the given tolerance). If so, the following is true:
      // (1) the current line is the next sibling line of the topmost line in the stack;
      // (2) the topmost line in the stack is the previous sibling line of the current line;
      // (3) the parent line of the topmost line in the stack is also the parent line of the
      //     current line.
      double topStackLeftX = lineStack.top()->pos->leftX;
      double lineLeftX = line->pos->leftX;
      if (equal(topStackLeftX, lineLeftX, leftXOffsetThreshold)) {
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
      if (smaller(topStackLeftX, lineLeftX, leftXOffsetThreshold)) {
        line->parentLine = lineStack.top();

        lineStack.push(line);
        continue;
      }
    }
  }
}

// _________________________________________________________________________________________________
tuple<double, double, double, double> TextLinesDetectionUtils::computeTrimBox(
    const PdfPageSegment* segment) const {
  assert(segment);

  // Initialize the coordinates of the trim box with the respective coordinates of the bounding box.
  double trimLeftX = segment->pos->leftX;
  double trimUpperY = segment->pos->upperY;
  double trimRightX = segment->pos->rightX;
  double trimLowerY = segment->pos->lowerY;

  // Compute the most frequent rightX among the text lines.
  DoubleCounter rightXCounter;
  for (auto* line : segment->lines) {
    double rightX = round(line->pos->getRotRightX(), _config.trimBoxCoordsPrec);
    rightXCounter[rightX]++;
  }

  if (rightXCounter.sumCounts() > 0) {
    pair<double, double> mostFreqRightXPair = rightXCounter.mostFreqAndCount();
    double mostFreqRightX = mostFreqRightXPair.first;
    int mostFreqRightXCount = mostFreqRightXPair.second;

    // Compute the percentage of lines exhibiting the most frequent rightX.
    double nLines = segment->lines.size();
    double mostFreqRightXRatio = nLines > 0 ? mostFreqRightXCount / nLines: 0.0;

    // If the percentage is larger or equal to the given threshold, set trimRightX to this value.
    if (equalOrLarger(mostFreqRightXRatio, _config.minPrecLinesSameRightX)) {
      trimRightX = mostFreqRightX;
    }
  }

  return make_tuple(trimLeftX, trimUpperY, trimRightX, trimLowerY);
}

}  // namespace ppp::utils
