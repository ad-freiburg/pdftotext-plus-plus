/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <limits>  // std::numeric_limits
#include <vector>

#include "../../src/Config.h"
#include "../../src/PdfDocument.h"
#include "../../src/utils/PageSegmentationUtils.h"

using std::numeric_limits;
using std::vector;

using ppp::config::PageSegmentationConfig;
using ppp::types::PdfDocument;
using ppp::types::PdfElement;
using ppp::types::PdfPageSegment;
using ppp::types::PdfWord;
using ppp::utils::PageSegmentationUtils;

// =================================================================================================

// The tolerance to use on comparing two float values.
static const double TOL = ppp::config::DEFAULT_DOUBLE_EQUAL_TOLERANCE;

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
  ASSERT_EQ(segment->doc, nullptr);

  // Input: vector of four words.
  PdfDocument pdf;
  PdfWord word1(3, 20.0, 720.0, 24.5, 725.0, 0, 0);
  PdfWord word2(3, 25.0, 719.1, 27.8, 724.9, 0, 0);
  PdfWord word3(3, 28.2, 720.0, 32.5, 724.9, 0, 0);
  PdfWord word4(3, 33.0, 720.1, 37.7, 724.8, 0, 0);
  word1.doc = &pdf;
  word2.doc = &pdf;
  word3.doc = &pdf;
  word4.doc = &pdf;
  elements = { &word1, &word2, &word3, &word4 };
  segment = utils.createPageSegment(elements);
  ASSERT_FALSE(segment->id.empty());
  ASSERT_EQ(segment->pos->pageNum, 3);
  ASSERT_NEAR(segment->pos->leftX, 20.0, TOL);
  ASSERT_NEAR(segment->pos->upperY, 719.1, TOL);
  ASSERT_NEAR(segment->pos->rightX, 37.7, TOL);
  ASSERT_NEAR(segment->pos->lowerY, 725.0, TOL);
  ASSERT_EQ(segment->elements, elements);
  ASSERT_EQ(segment->doc, &pdf);
}
