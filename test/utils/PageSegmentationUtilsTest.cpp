/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <limits>
#include <vector>

#include "../../src/Config.h"
#include "../../src/PdfDocument.h"
#include "../../src/utils/PageSegmentationUtils.h"

using std::numeric_limits;
using std::vector;

using ppp::config::PageSegmentationConfig;
using ppp::utils::PageSegmentationUtils;

// =================================================================================================

// The allowed tolerance on comparing two float values.
static const double TOL = ppp::config::DEFAULT_DOUBLE_EQUAL_TOLERANCE;

// Define some words.
static PdfWord word1(3, 20.0, 720.0, 24.5, 725.0, 0, 0);
static PdfWord word2(3, 25.0, 719.1, 27.8, 724.9, 0, 0);
static PdfWord word3(3, 28.2, 720.0, 32.5, 724.9, 0, 0);
static PdfWord word4(3, 33.0, 720.1, 37.7, 724.8, 0, 0);

// _________________________________________________________________________________________________
TEST(PageSegmentationUtilsTest, createPageSegment) {
  PageSegmentationConfig config;
  PageSegmentationUtils utils(config);

  // Input: empty vector.
  vector<PdfElement*> elements;
  PdfPageSegment* segment = utils.createPageSegment(elements);
  ASSERT_FALSE(segment->id.empty());
  ASSERT_EQ(segment->pos->pageNum, -1);
  ASSERT_EQ(segment->pos->leftX, numeric_limits<double>::max());
  ASSERT_EQ(segment->pos->upperY, numeric_limits<double>::max());
  ASSERT_EQ(segment->pos->rightX, numeric_limits<double>::min());
  ASSERT_EQ(segment->pos->lowerY, numeric_limits<double>::min());
  ASSERT_EQ(segment->elements, elements);

  // Input: vector of four words.
  elements = { &word1, &word2, &word3, &word4 };
  segment = utils.createPageSegment(elements);
  ASSERT_FALSE(segment->id.empty());
  ASSERT_EQ(segment->pos->pageNum, 3);
  ASSERT_NEAR(segment->pos->leftX, 20.0, TOL);
  ASSERT_NEAR(segment->pos->upperY, 719.1, TOL);
  ASSERT_NEAR(segment->pos->rightX, 37.7, TOL);
  ASSERT_NEAR(segment->pos->lowerY, 725.0, TOL);
  ASSERT_EQ(segment->elements, elements);
}


// _________________________________________________________________________________________________
// TEST(PageSegmentsUtilsTest, computeTrimBoxPdf1) {
//   PdfPage* page0 = pdf1->pages[0];

//   Test a segment composed from the lines of the third enumeration. Since there is no clear most
//   // common rightX value, the rightX of the trim box should be equal to the largest rightX.
//   std::vector<PdfTextLine*> lines;
//   lines.push_back(page0->segments[1]->lines[9]);  // "(a) This is an item ..."
//   lines.push_back(page0->segments[1]->lines[10]);
//   lines.push_back(page0->segments[1]->lines[11]);

//   // Create a segment from the lines and compute the bounding box.
//   PdfPageSegment* segment = new PdfPageSegment();
//   segment->lines = lines;
//   for (auto* line : lines) {
//     segment->pos->leftX = std::min(segment->pos->leftX, line->pos->leftX);
//     segment->pos->upperY = std::min(segment->pos->upperY, line->pos->upperY);
//     segment->pos->rightX = std::max(segment->pos->rightX, line->pos->rightX);
//     segment->pos->lowerY = std::max(segment->pos->lowerY, line->pos->lowerY);
//   }
//   std::tuple<double, double, double, double> trimBox =
//       page_segment_utils::computeTrimBox(segment, 0, 0.5);

//   // The rightX of the trimBox (the third value) should be the largest rightX.
//   ASSERT_NEAR(std::get<0>(trimBox), 312.3, TOL);
//   ASSERT_NEAR(std::get<1>(trimBox), 243.9, TOL);
//   ASSERT_NEAR(std::get<2>(trimBox), 539.2, TOL);
//   ASSERT_NEAR(std::get<3>(trimBox), 284.3, TOL);

//   // ===============
//   // Test a segment composed from the lines of the last but one block of page 1. Notice that the
//   // first of these lines extends beyond the actual column boundaries. The rightX of the trim box
//   // should be equal to the rightX of all other lines.

//   lines.clear();
//   // Starting at "Namliber tempor cum ..."
//   for (size_t i = 30; i < 37; i++) { lines.push_back(page0->segments[1]->lines[i]); }

//   // Create a segment from the lines and compute the bounding box.
//   segment = new PdfPageSegment();
//   segment->lines = lines;
//   for (auto* line : lines) {
//     segment->pos->leftX = std::min(segment->pos->leftX, line->pos->leftX);
//     segment->pos->upperY = std::min(segment->pos->upperY, line->pos->upperY);
//     segment->pos->rightX = std::max(segment->pos->rightX, line->pos->rightX);
//     segment->pos->lowerY = std::max(segment->pos->lowerY, line->pos->lowerY);
//   }
//   trimBox = computeTrimBox(segment, 0, 0.5);

//   // The rightX of the trim box should be equal to the most frequent right X among the lines.
//   ASSERT_NEAR(std::get<0>(trimBox), 310.6, TOL);
//   ASSERT_NEAR(std::get<1>(trimBox), 531.5, TOL);
//   ASSERT_NEAR(std::get<2>(trimBox), 539.0, TOL);
//   ASSERT_NEAR(std::get<3>(trimBox), 616.2, TOL);
// }
