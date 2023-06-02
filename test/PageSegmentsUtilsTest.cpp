/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <algorithm>  // min, max
#include <limits>
#include <tuple>
#include <vector>

#include "../src/Config.h"
#include "../src/PdfDocument.h"
#include "../src/PdfToTextPlusPlus.h"
#include "../src/utils/PageSegmentsUtils.h"
#include "../src/utils/PdfElementsUtils.h"

using page_segment_utils::computeTrimBox;
using page_segment_utils::createPageSegment;

// The allowed tolerance on comparing two float values.
const double TOL = 0.1;

// _________________________________________________________________________________________________
class PageSegmentsUtilsTest : public ::testing::Test {
 protected:
  // This method is called before the first test of this test suite.
  static void SetUpTestSuite() {
    ppp::Config config;
    config.semanticRolesDetectionModelsDir = CONFIG_SEMANTIC_ROLES_DETECTION_MODELS_DIR;
    PdfToTextPlusPlus engine(&config);

    if (pdf1 == nullptr) {
      pdf1 = new PdfDocument();
      engine.process("./test/pdfs/1-article-two-columns.pdf", pdf1);
    }

    if (pdf2 == nullptr) {
      pdf2 = new PdfDocument();
      engine.process("./test/pdfs/2-article-one-column.pdf", pdf2);
    }
  }

  // This method is called after the last test of this test suite.
  static void TearDownTestSuite() {
    delete pdf1;
    pdf1 = nullptr;
    delete pdf2;
    pdf2 = nullptr;
  }

  static PdfDocument* pdf1;
  static PdfDocument* pdf2;
};

PdfDocument* PageSegmentsUtilsTest::pdf1 = nullptr;
PdfDocument* PageSegmentsUtilsTest::pdf2 = nullptr;

// _________________________________________________________________________________________________
TEST_F(PageSegmentsUtilsTest, createPageSegmentPdf1) {
  PdfPage* page1 = pdf1->pages[1];

  // Test a segment composed from an empty vector of elements.
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

  // Test a segment composed from the lines of the first text block of the second page and the
  // image underneath.
  elements.clear();
  elements.push_back(page1->segments[0]->lines[0]);
  elements.push_back(page1->segments[0]->lines[1]);
  elements.push_back(page1->segments[0]->lines[2]);
  elements.push_back(page1->graphics[0]);
  segment = page_segment_utils::createPageSegment(elements);
  ASSERT_TRUE(!segment->id.empty());
  ASSERT_EQ(segment->pos->pageNum, 2);
  ASSERT_EQ(segment->pos->rotation, 0);
  ASSERT_EQ(segment->pos->wMode, 0);
  ASSERT_NEAR(segment->pos->leftX, 72.0, TOL);
  ASSERT_NEAR(segment->pos->upperY, 125.3, TOL);
  ASSERT_NEAR(segment->pos->rightX, 300.6, TOL);
  ASSERT_NEAR(segment->pos->lowerY, 357.1, TOL);
  ASSERT_EQ(segment->elements, elements);
}

// _________________________________________________________________________________________________
TEST_F(PageSegmentsUtilsTest, computeTrimBoxPdf1) {
  PdfPage* page0 = pdf1->pages[0];

  // Test a segment composed from the lines of the third enumeration. Since there is no clear most
  // common rightX value, the rightX of the trim box should be equal to the largest rightX.
  std::vector<PdfTextLine*> lines;
  lines.push_back(page0->segments[1]->lines[9]);  // "(a) This is an item ..."
  lines.push_back(page0->segments[1]->lines[10]);
  lines.push_back(page0->segments[1]->lines[11]);

  // Create a segment from the lines and compute the bounding box.
  PdfPageSegment* segment = new PdfPageSegment();
  segment->lines = lines;
  for (auto* line : lines) {
    segment->pos->leftX = std::min(segment->pos->leftX, line->pos->leftX);
    segment->pos->upperY = std::min(segment->pos->upperY, line->pos->upperY);
    segment->pos->rightX = std::max(segment->pos->rightX, line->pos->rightX);
    segment->pos->lowerY = std::max(segment->pos->lowerY, line->pos->lowerY);
  }
  std::tuple<double, double, double, double> trimBox = page_segment_utils::computeTrimBox(segment);

  // The rightX of the trimBox (the third value) should be the largest rightX.
  ASSERT_NEAR(std::get<0>(trimBox), 312.3, TOL);
  ASSERT_NEAR(std::get<1>(trimBox), 243.9, TOL);
  ASSERT_NEAR(std::get<2>(trimBox), 539.2, TOL);
  ASSERT_NEAR(std::get<3>(trimBox), 284.3, TOL);

  // ===============
  // Test a segment composed from the lines of the last but one block of page 1. Notice that the
  // first of these lines extends beyond the actual column boundaries. The rightX of the trim box
  // should be equal to the rightX of all other lines.

  lines.clear();
  // Starting at "Namliber tempor cum ..."
  for (size_t i = 30; i < 37; i++) { lines.push_back(page0->segments[1]->lines[i]); }

  // Create a segment from the lines and compute the bounding box.
  segment = new PdfPageSegment();
  segment->lines = lines;
  for (auto* line : lines) {
    segment->pos->leftX = std::min(segment->pos->leftX, line->pos->leftX);
    segment->pos->upperY = std::min(segment->pos->upperY, line->pos->upperY);
    segment->pos->rightX = std::max(segment->pos->rightX, line->pos->rightX);
    segment->pos->lowerY = std::max(segment->pos->lowerY, line->pos->lowerY);
  }
  trimBox = computeTrimBox(segment);

  // The rightX of the trim box should be equal to the most frequent right X among the lines.
  ASSERT_NEAR(std::get<0>(trimBox), 310.6, TOL);
  ASSERT_NEAR(std::get<1>(trimBox), 531.5, TOL);
  ASSERT_NEAR(std::get<2>(trimBox), 539.0, TOL);
  ASSERT_NEAR(std::get<3>(trimBox), 616.2, TOL);
}
