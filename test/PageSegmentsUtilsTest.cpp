/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <limits>
#include <tuple>
#include <vector>

#include "../src/utils/PageSegmentsUtils.h"
#include "../src/utils/PdfElementsUtils.h"

#include "../src/Constants.h"
#include "../src/PdfDocument.h"

using global_config::DOUBLE_EQUAL_TOLERANCE;

// _________________________________________________________________________________________________
TEST(PageSegmentsUtils, createPageSegment) {
  std::vector<PdfElement*> elements;
  PdfPageSegment* segment = page_segment_utils::createPageSegment(elements);
  ASSERT_TRUE(!segment->id.empty());
  ASSERT_EQ(segment->pos->pageNum, -1);
  ASSERT_EQ(segment->pos->rotation, 0);
  ASSERT_EQ(segment->pos->wMode, 0);
  ASSERT_EQ(segment->pos->leftX, numeric_limits<double>::max());
  ASSERT_EQ(segment->pos->upperY, numeric_limits<double>::max());
  ASSERT_EQ(segment->pos->rightX, numeric_limits<double>::min());
  ASSERT_EQ(segment->pos->lowerY, numeric_limits<double>::min());
  ASSERT_EQ(segment->elements, elements);

  PdfWord word1(3, 34.1, 250.1, 112.7, 261.4, 0, 0);
  PdfWord word2(3, 34.1, 264.0, 112.7, 275.9, 0, 0);
  PdfWord word3(3, 33.9, 278.3, 122.3, 289.4, 0, 0);
  PdfWord word4(3, 34.1, 292.1, 119.2, 306.7, 0, 0);
  PdfWord word5(3, 34.1, 309.3, 112.7, 320.2, 0, 0);
  elements.push_back(&word1);
  elements.push_back(&word2);
  elements.push_back(&word3);
  elements.push_back(&word4);
  elements.push_back(&word5);

  PdfPageSegment* segment2 = page_segment_utils::createPageSegment(elements);
  ASSERT_TRUE(!segment2->id.empty());
  ASSERT_EQ(segment2->pos->pageNum, 3);
  ASSERT_EQ(segment2->pos->rotation, 0);
  ASSERT_EQ(segment2->pos->wMode, 0);
  ASSERT_NEAR(segment2->pos->leftX, 33.9, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(segment2->pos->upperY, 250.1, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(segment2->pos->rightX, 122.3, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(segment2->pos->lowerY, 320.2, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_EQ(segment2->elements, elements);
}

// _________________________________________________________________________________________________
TEST(PageSegmentsUtils, computeTrimBox) {
  // ------------
  // Create a segment with five lines; all but two having the same rightX value.
  std::vector<PdfTextLine*> lines;
  PdfTextLine line1(1, 34.1, 250.1, 112.7, 261.4, 0, 0);
  PdfTextLine line2(1, 34.1, 264.0, 112.7, 275.9, 0, 0);
  PdfTextLine line3(1, 33.9, 278.3, 122.3, 289.4, 0, 0);
  PdfTextLine line4(1, 34.1, 292.1, 119.2, 306.7, 0, 0);
  PdfTextLine line5(1, 34.1, 309.3, 112.7, 320.2, 0, 0);
  lines.push_back(&line1);
  lines.push_back(&line2);
  lines.push_back(&line3);
  lines.push_back(&line4);
  lines.push_back(&line5);

  PdfPageSegment* segment = new PdfPageSegment();
  segment->lines = lines;
  for (auto* line : lines) {
    segment->pos->leftX = std::min(segment->pos->leftX, line->pos->leftX);
    segment->pos->upperY = std::min(segment->pos->upperY, line->pos->upperY);
    segment->pos->rightX = std::max(segment->pos->rightX, line->pos->rightX);
    segment->pos->lowerY = std::max(segment->pos->lowerY, line->pos->lowerY);
  }
  std::tuple<double, double, double, double> trimBox = page_segment_utils::computeTrimBox(segment);

  // The rightX of the trimBox (the third value) should be the most frequent rightX (112.7).
  ASSERT_NEAR(std::get<0>(trimBox), 33.9, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(std::get<1>(trimBox), 250.1, DOUBLE_EQUAL_TOLERANCE);
  // NOTE: The rightX is rounded to 0 decimals.
  ASSERT_NEAR(std::get<2>(trimBox), 113.0, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(std::get<3>(trimBox), 320.2, DOUBLE_EQUAL_TOLERANCE);

  // ------------
  // Create a segment with five lines; all with different rightX values.
  std::vector<PdfTextLine*> lines2;
  PdfTextLine line6(1, 34.1, 250.1, 117.7, 261.4, 0, 0);
  PdfTextLine line7(1, 34.1, 264.0, 122.7, 275.9, 0, 0);
  PdfTextLine line8(1, 33.9, 278.3, 131.3, 289.4, 0, 0);
  PdfTextLine line9(1, 34.1, 292.1, 142.7, 306.7, 0, 0);
  lines2.push_back(&line6);
  lines2.push_back(&line7);
  lines2.push_back(&line8);
  lines2.push_back(&line9);

  PdfPageSegment* segment2 = new PdfPageSegment();
  segment2->lines = lines2;
  for (auto* line : lines2) {
    segment2->pos->leftX = std::min(segment2->pos->leftX, line->pos->leftX);
    segment2->pos->upperY = std::min(segment2->pos->upperY, line->pos->upperY);
    segment2->pos->rightX = std::max(segment2->pos->rightX, line->pos->rightX);
    segment2->pos->lowerY = std::max(segment2->pos->lowerY, line->pos->lowerY);
  }
  trimBox = page_segment_utils::computeTrimBox(segment2);

  // The rightX of the trimBox (the third value) should be the largest rightX (142.7).
  ASSERT_NEAR(std::get<0>(trimBox), 33.9, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(std::get<1>(trimBox), 250.1, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(std::get<2>(trimBox), 142.7, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(std::get<3>(trimBox), 306.7, DOUBLE_EQUAL_TOLERANCE);
}
