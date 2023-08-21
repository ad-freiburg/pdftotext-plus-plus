/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <string>
#include <tuple>

#include "../../src/Config.h"
#include "../../src/PdfDocument.h"
#include "../../src/PdfToTextPlusPlus.h"
#include "../../src/utils/MathUtils.h"
#include "../../src/utils/TextLinesDetectionUtils.h"

using std::string;
using std::tuple;

using ppp::PdfToTextPlusPlus;
using ppp::config::Config;
using ppp::config::TextLinesDetectionConfig;
using ppp::types::PdfDocument;
using ppp::types::PdfPage;
using ppp::types::PdfPageSegment;
using ppp::types::PdfTextLine;
using ppp::utils::TextLinesDetectionUtils;
using ppp::utils::math::round;

// =================================================================================================

// The path to the PDF file to process in the test cases below.
static const char* PDF_FILE_PATH = "./test/pdfs/TextLinesDetectionUtilsTest.pdf";

// The tolerance to use on comparing two float values.
static const double TOL = ppp::config::DEFAULT_DOUBLE_EQUAL_TOLERANCE;


class TextLinesDetectionUtilsTest : public ::testing::Test {
 protected:
  // This method is called before the first test case of this test suite is called.
  static void SetUpTestSuite() {
    Config config;
    config.subSuperScriptsDetection.disable = true;
    config.textBlocksDetection.disable = true;
    config.readingOrderDetection.disable = true;
    config.semanticRolesPrediction.disable = true;
    config.wordsDehyphenation.disable = true;

    PdfToTextPlusPlus engine(&config);
    pdf = new PdfDocument();
    string pdfFilePathStr = PDF_FILE_PATH;
    engine.process(&pdfFilePathStr, pdf);
  }

  // This method is called after the last test case of this test suite is called.
  static void TearDownTestSuite() {
    delete pdf;
  }

  static PdfDocument* pdf;
};

PdfDocument* TextLinesDetectionUtilsTest::pdf = nullptr;

// _________________________________________________________________________________________________
TEST_F(TextLinesDetectionUtilsTest, computeTextLineHierarchy) {
  PdfPage* page = pdf->pages[0];
  PdfTextLine* line1 = page->textLines[1];
  PdfTextLine* line2 = page->textLines[2];
  PdfTextLine* line3 = page->textLines[3];
  PdfTextLine* line4 = page->textLines[4];
  PdfTextLine* line5 = page->textLines[5];
  PdfTextLine* line6 = page->textLines[6];
  PdfTextLine* line7 = page->textLines[7];
  PdfTextLine* line8 = page->textLines[8];
  PdfTextLine* line9 = page->textLines[9];
  PdfTextLine* line10 = page->textLines[10];
  PdfTextLine* line11 = page->textLines[11];
  PdfTextLine* line12 = page->textLines[12];
  PdfTextLine* line13 = page->textLines[13];
  PdfTextLine* line14 = page->textLines[14];

  // Make sure we selected the correct text lines.
  ASSERT_TRUE(line1->text.starts_with("1. Things to collect"));
  ASSERT_TRUE(line2->text.starts_with("a) Bottle caps"));
  ASSERT_TRUE(line3->text.starts_with("b) Lost Socks"));
  ASSERT_TRUE(line4->text.starts_with("i. Left Socks"));
  ASSERT_TRUE(line5->text.starts_with("ii. Striped socks"));
  ASSERT_TRUE(line6->text.starts_with("A. Red and white striped socks"));
  ASSERT_TRUE(line7->text.starts_with("of blue polka dots"));
  ASSERT_TRUE(line8->text.starts_with("B. Blue and white striped"));
  ASSERT_TRUE(line9->text.starts_with("iii. Polka dot socks"));
  ASSERT_TRUE(line10->text.starts_with("c) Broken pencils"));
  ASSERT_TRUE(line11->text.starts_with("d) Empty gum wrappers"));
  ASSERT_TRUE(line12->text.starts_with("2. Unusual hobbies"));
  ASSERT_TRUE(line13->text.starts_with("a) Counting raindrops"));
  ASSERT_TRUE(line14->text.starts_with("b) Bubble wrap popping marathon"));

  TextLinesDetectionConfig config;
  TextLinesDetectionUtils utils(config);

  // Input: nullptr.
  ASSERT_DEATH(utils.computeTextLineHierarchy(nullptr), "");

  utils.computeTextLineHierarchy(page);
  ASSERT_EQ(line1->parentLine, nullptr);
  ASSERT_EQ(line1->prevSiblingLine, nullptr);
  ASSERT_EQ(line2->parentLine, line1);
  ASSERT_EQ(line2->prevSiblingLine, nullptr);
  ASSERT_EQ(line2->nextSiblingLine, line3);
  ASSERT_EQ(line3->parentLine, line1);
  ASSERT_EQ(line3->prevSiblingLine, line2);
  ASSERT_EQ(line3->nextSiblingLine, line10);
  ASSERT_EQ(line4->parentLine, line3);
  ASSERT_EQ(line4->prevSiblingLine, nullptr);
  ASSERT_EQ(line4->nextSiblingLine, line5);
  ASSERT_EQ(line5->parentLine, line3);
  ASSERT_EQ(line5->prevSiblingLine, line4);
  ASSERT_EQ(line5->nextSiblingLine, line9);
  ASSERT_EQ(line6->parentLine, line5);
  ASSERT_EQ(line6->prevSiblingLine, nullptr);
  ASSERT_EQ(line6->nextSiblingLine, line8);
  ASSERT_EQ(line7->parentLine, line6);
  ASSERT_EQ(line7->prevSiblingLine, nullptr);
  ASSERT_EQ(line7->nextSiblingLine, nullptr);
  ASSERT_EQ(line8->parentLine, line5);
  ASSERT_EQ(line8->prevSiblingLine, line6);
  ASSERT_EQ(line8->nextSiblingLine, nullptr);
  ASSERT_EQ(line9->parentLine, line3);
  ASSERT_EQ(line9->prevSiblingLine, line5);
  ASSERT_EQ(line9->nextSiblingLine, nullptr);
  ASSERT_EQ(line10->parentLine, line1);
  ASSERT_EQ(line10->prevSiblingLine, line3);
  ASSERT_EQ(line10->nextSiblingLine, line11);
  ASSERT_EQ(line11->parentLine, line1);
  ASSERT_EQ(line11->prevSiblingLine, line10);
  ASSERT_EQ(line11->nextSiblingLine, nullptr);
  ASSERT_EQ(line12->parentLine, nullptr);
  ASSERT_EQ(line12->prevSiblingLine, line1);
  ASSERT_EQ(line12->nextSiblingLine, nullptr);
  ASSERT_EQ(line13->parentLine, line12);
  ASSERT_EQ(line13->prevSiblingLine, nullptr);
  ASSERT_EQ(line13->nextSiblingLine, line14);
  ASSERT_EQ(line14->parentLine, line12);
  ASSERT_EQ(line14->prevSiblingLine, line13);
  ASSERT_EQ(line14->nextSiblingLine, nullptr);
}

// _________________________________________________________________________________________________
// TODO(korzen): Add a test for a "normal" segment, without a line extending the column boundaries.
TEST_F(TextLinesDetectionUtilsTest, computeTrimBox) {
  PdfPage* page = pdf->pages[1];
  PdfPageSegment* segment = page->segments[0];

  // Make sure we selected the correct segment.
  ASSERT_TRUE(segment->lines[1]->text.starts_with("Friedrich Schiller, born on"));
  ASSERT_TRUE(segment->lines[segment->lines.size() - 1]->text.ends_with("with audiences today."));

  TextLinesDetectionConfig config;
  TextLinesDetectionUtils utils(config);

  // Input: nullptr.
  ASSERT_DEATH(utils.computeTrimBox(nullptr), "");

  tuple<double, double, double, double> trimBox = utils.computeTrimBox(segment);
  // TODO(korzen): The coordinates should not be rounded here.
  ASSERT_NEAR(round(std::get<0>(trimBox), 1), 56.7, TOL);
  ASSERT_NEAR(round(std::get<1>(trimBox), 1), 59.1, TOL);
  ASSERT_NEAR(round(std::get<2>(trimBox), 1), 539.0, TOL);
  ASSERT_NEAR(round(std::get<3>(trimBox), 1), 500.4, TOL);
}
