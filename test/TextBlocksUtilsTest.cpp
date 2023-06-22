/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <string>
#include <unordered_set>
#include <vector>

#include "../src/PdfDocument.h"
#include "../src/PdfDocumentVisualizer.h"
#include "../src/PdfToTextPlusPlus.h"
#include "../src/utils/MathUtils.h"
#include "../src/utils/TextBlocksUtils.h"

using text_blocks_utils::computeHangingIndent;
using text_blocks_utils::computeIsTextLinesCentered;
using text_blocks_utils::computeTextLineMargins;

// The allowed tolerance on comparing two float values. TODO(korzen): Read from config.
const double TOL = 0.1;

// _________________________________________________________________________________________________
class TextBlocksUtilsTest : public ::testing::Test {
 protected:
  // This method is called before the first test of this test suite.
  static void SetUpTestSuite() {
    ppp::Config config;
    config.rolesPrediction.modelsDir = CONFIG_SEMANTIC_ROLES_DETECTION_MODELS_DIR;
    PdfToTextPlusPlus engine(config);

    if (pdf1 == nullptr) {
      pdf1 = new PdfDocument();
      engine.process("./test/pdfs/1-article-two-columns.pdf", pdf1);

      // Create a visualization of the text blocks (that will help to address specific text blocks
      // in the test cases below).
      // PdfDocumentVisualizer visualizer(pdf1->pdfFilePath);
      // visualizer.visualizeTextBlocks(*pdf1, visualizer::color_schemes::red);
      // visualizer.save("./test/pdfs/1-article-two-columns.vis-blocks.pdf");
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

PdfDocument* TextBlocksUtilsTest::pdf1 = nullptr;
PdfDocument* TextBlocksUtilsTest::pdf2 = nullptr;

// _________________________________________________________________________________________________
TEST_F(TextBlocksUtilsTest, computeIsTextLinesCenteredPdf1) {
  // TODO(korzen): Read the parameter from the config.
  const char* formulaIdAlphabet = "=+";
  double centeringXOverlapRatioThreshold = 0.99;
  double centeringXOffsetEqualToleranceFactor = 2.0;
  int centeringMaxNumJustifiedLines = 5;

  PdfPage* page0 = pdf1->pages[0];
  PdfPage* page1 = pdf1->pages[1];

  // Test an empty block.
  PdfTextBlock* block = new PdfTextBlock();
  ASSERT_FALSE(computeIsTextLinesCentered(
    block,
    formulaIdAlphabet,
    centeringXOverlapRatioThreshold,
    centeringXOffsetEqualToleranceFactor,
    centeringMaxNumJustifiedLines)) << "Block: " << block->toString();

  // Test the first text block of the Introduction. The lines are not centered compared to each
  // other, so the method should return false.
  block = page0->blocks[1];
  ASSERT_FALSE(computeIsTextLinesCentered(
    block,
    formulaIdAlphabet,
    centeringXOverlapRatioThreshold,
    centeringXOffsetEqualToleranceFactor,
    centeringMaxNumJustifiedLines)) << "Block: " << block->toString();

  // Test the sixth block of the right column of the first page (the block with the centered lines).
  // The method should return true.
  block = page0->blocks[17];
  ASSERT_TRUE(computeIsTextLinesCentered(
    block,
    formulaIdAlphabet,
    centeringXOverlapRatioThreshold,
    centeringXOffsetEqualToleranceFactor,
    centeringMaxNumJustifiedLines)) << "Block: " << block->toString();

  // Test the first text block on the second page. The lines are not centered compared to each
  // other, so the method should return false.
  block = page1->blocks[0];
  ASSERT_FALSE(computeIsTextLinesCentered(
    block,
    formulaIdAlphabet,
    centeringXOverlapRatioThreshold,
    centeringXOffsetEqualToleranceFactor,
    centeringMaxNumJustifiedLines)) << "Block: " << block->toString();

  // Test a block composed of the display formula in the left column of the second page and the
  // respective next line. Although the lines are centered compared to each other, the method
  // should return false, because one of the lines is a formula.
  block = new PdfTextBlock();
  block->doc = pdf1;
  block->lines.push_back(page1->segments[0]->lines[19]);  // formula
  block->lines.push_back(page1->segments[0]->lines[20]);  // "This equation..."
  ASSERT_FALSE(computeIsTextLinesCentered(
    block,
    formulaIdAlphabet,
    centeringXOverlapRatioThreshold,
    centeringXOffsetEqualToleranceFactor,
    centeringMaxNumJustifiedLines)) << "Block: " << block->toString();

  // Test the block after the display formula in the left column of the second page. It consists
  // of two justified text lines, so the method should return false.
  block = page1->blocks[6];
  ASSERT_FALSE(computeIsTextLinesCentered(
    block,
    formulaIdAlphabet,
    centeringXOverlapRatioThreshold,
    centeringXOffsetEqualToleranceFactor,
    centeringMaxNumJustifiedLines)) << "Block: " << block->toString();

  // Test a block composed of the lines of the centered block in the right column of the first
  // page, and the respective three following lines. The method should return true.
  block = new PdfTextBlock();
  block->doc = pdf1;
  block->lines = page0->blocks[17]->lines;  // "This is a centered ..."
  for (size_t i = 0; i < 3; i++) { block->lines.push_back(page0->blocks[18]->lines[i]); }
  ASSERT_TRUE(computeIsTextLinesCentered(
    block,
    formulaIdAlphabet,
    centeringXOverlapRatioThreshold,
    centeringXOffsetEqualToleranceFactor,
    centeringMaxNumJustifiedLines)) << "Block: " << block->toString();

  // Test a block composed of the lines of the centered block in the right column of the first
  // page, and the respective seven following lines. The method should return false, because the
  // number of justified lines exceeds the threshold.
  block = new PdfTextBlock();
  block->doc = pdf1;
  block->lines = page0->blocks[17]->lines;  // "This is a centered ..."
  for (size_t i = 0; i < 7; i++) { block->lines.push_back(page0->blocks[18]->lines[i]); }
  ASSERT_FALSE(computeIsTextLinesCentered(
    block,
    formulaIdAlphabet,
    centeringXOverlapRatioThreshold,
    centeringXOffsetEqualToleranceFactor,
    centeringMaxNumJustifiedLines)) << "Block: " << block->toString();
}

// _________________________________________________________________________________________________
TEST_F(TextBlocksUtilsTest, computeHangingIndentPdf1) {
  // TODO(korzen): Read value from config.
  const unordered_set<string>& lastNamePrefixes =  { "van", "von", "de" };
  int hangIndentMinLengthLongLines = 3;
  double prevTextLineCapacityThresholdFactor = 2.0;
  double hangIndentMinPercLinesSameLeftMargin = 0.5;
  int hangIndentNumNonIndentedLinesThreshold = 10;
  double hangIndentMarginThresholdFactor = 1.0;
  int hangIndentNumLowerNonIndentedLinesThreshold = 0;
  int hangIndentNumLongLinesThreshold = 4;
  int hangIndentNumLowerIndentedLinesThreshold = 1;


  PdfPage* page0 = pdf1->pages[0];
  PdfPage* page1 = pdf1->pages[1];

  // Test a block with no lines.
  PdfTextBlock* block = new PdfTextBlock();
  block->doc = pdf1;
  ASSERT_NEAR(computeHangingIndent(
    block,
    lastNamePrefixes,
    hangIndentMinLengthLongLines,
    prevTextLineCapacityThresholdFactor,
    hangIndentMinPercLinesSameLeftMargin,
    hangIndentNumNonIndentedLinesThreshold,
    hangIndentMarginThresholdFactor,
    hangIndentNumLowerNonIndentedLinesThreshold,
    hangIndentNumLongLinesThreshold,
    hangIndentNumLowerIndentedLinesThreshold), 0.0, TOL) << "Block: " << block->toString();

  // Test the first text block of the Introduction (not in hanging indent format).
  block = page0->blocks[1];
  ASSERT_NEAR(computeHangingIndent(
    block,
    lastNamePrefixes,
    hangIndentMinLengthLongLines,
    prevTextLineCapacityThresholdFactor,
    hangIndentMinPercLinesSameLeftMargin,
    hangIndentNumNonIndentedLinesThreshold,
    hangIndentMarginThresholdFactor,
    hangIndentNumLowerNonIndentedLinesThreshold,
    hangIndentNumLongLinesThreshold,
    hangIndentNumLowerIndentedLinesThreshold), 0.0, TOL) << "Block: " << block->toString();

  // Test the centered text block in the right column of the first page.
  block = page0->blocks[17];
  ASSERT_NEAR(computeHangingIndent(
    block,
    lastNamePrefixes,
    hangIndentMinLengthLongLines,
    prevTextLineCapacityThresholdFactor,
    hangIndentMinPercLinesSameLeftMargin,
    hangIndentNumNonIndentedLinesThreshold,
    hangIndentMarginThresholdFactor,
    hangIndentNumLowerNonIndentedLinesThreshold,
    hangIndentNumLongLinesThreshold,
    hangIndentNumLowerIndentedLinesThreshold), 0.0, TOL) << "Block: " << block->toString();

  // Test a block composed of the text lines of the second section ("Movie Listing").
  block = new PdfTextBlock();
  block->doc = pdf1;
  for (size_t i = 8; i < 33; i++) {
    block->lines.push_back(page1->segments[1]->lines[i]);
  }
  ASSERT_NEAR(computeHangingIndent(
    block,
    lastNamePrefixes,
    hangIndentMinLengthLongLines,
    prevTextLineCapacityThresholdFactor,
    hangIndentMinPercLinesSameLeftMargin,
    hangIndentNumNonIndentedLinesThreshold,
    hangIndentMarginThresholdFactor,
    hangIndentNumLowerNonIndentedLinesThreshold,
    hangIndentNumLongLinesThreshold,
    hangIndentNumLowerIndentedLinesThreshold), 19.0, TOL) << "Block: " << block->toString();
}

// _________________________________________________________________________________________________
TEST_F(TextBlocksUtilsTest, computeHangingIndentPdf2) {
  // TODO(korzen): Read value from config.
  const unordered_set<string>& lastNamePrefixes =  { "van", "von", "de" };
  int hangIndentMinLengthLongLines = 3;
  double prevTextLineCapacityThresholdFactor = 2.0;
  double hangIndentMinPercLinesSameLeftMargin = 0.5;
  int hangIndentNumNonIndentedLinesThreshold = 10;
  double hangIndentMarginThresholdFactor = 1.0;
  int hangIndentNumLowerNonIndentedLinesThreshold = 0;
  int hangIndentNumLongLinesThreshold = 4;
  int hangIndentNumLowerIndentedLinesThreshold = 1;

  PdfPage* page0 = pdf2->pages[0];

  // Test the six text blocks of the Introduction.
  PdfTextBlock* block = page0->blocks[3];
  ASSERT_NEAR(computeHangingIndent(
    block,
    lastNamePrefixes,
    hangIndentMinLengthLongLines,
    prevTextLineCapacityThresholdFactor,
    hangIndentMinPercLinesSameLeftMargin,
    hangIndentNumNonIndentedLinesThreshold,
    hangIndentMarginThresholdFactor,
    hangIndentNumLowerNonIndentedLinesThreshold,
    hangIndentNumLongLinesThreshold,
    hangIndentNumLowerIndentedLinesThreshold), 0.0, TOL) << "Block: " << block->toString();

  block = page0->blocks[4];
  ASSERT_NEAR(computeHangingIndent(
    block,
    lastNamePrefixes,
    hangIndentMinLengthLongLines,
    prevTextLineCapacityThresholdFactor,
    hangIndentMinPercLinesSameLeftMargin,
    hangIndentNumNonIndentedLinesThreshold,
    hangIndentMarginThresholdFactor,
    hangIndentNumLowerNonIndentedLinesThreshold,
    hangIndentNumLongLinesThreshold,
    hangIndentNumLowerIndentedLinesThreshold), 0.0, TOL) << "Block: " << block->toString();

  block = page0->blocks[5];
  ASSERT_NEAR(computeHangingIndent(
    block,
    lastNamePrefixes,
    hangIndentMinLengthLongLines,
    prevTextLineCapacityThresholdFactor,
    hangIndentMinPercLinesSameLeftMargin,
    hangIndentNumNonIndentedLinesThreshold,
    hangIndentMarginThresholdFactor,
    hangIndentNumLowerNonIndentedLinesThreshold,
    hangIndentNumLongLinesThreshold,
    hangIndentNumLowerIndentedLinesThreshold), 0.0, TOL) << "Block: " << block->toString();

  block = page0->blocks[6];
  ASSERT_NEAR(computeHangingIndent(
    block,
    lastNamePrefixes,
    hangIndentMinLengthLongLines,
    prevTextLineCapacityThresholdFactor,
    hangIndentMinPercLinesSameLeftMargin,
    hangIndentNumNonIndentedLinesThreshold,
    hangIndentMarginThresholdFactor,
    hangIndentNumLowerNonIndentedLinesThreshold,
    hangIndentNumLongLinesThreshold,
    hangIndentNumLowerIndentedLinesThreshold), 0.0, TOL) << "Block: " << block->toString();

  block = page0->blocks[7];
  ASSERT_NEAR(computeHangingIndent(
    block,
    lastNamePrefixes,
    hangIndentMinLengthLongLines,
    prevTextLineCapacityThresholdFactor,
    hangIndentMinPercLinesSameLeftMargin,
    hangIndentNumNonIndentedLinesThreshold,
    hangIndentMarginThresholdFactor,
    hangIndentNumLowerNonIndentedLinesThreshold,
    hangIndentNumLongLinesThreshold,
    hangIndentNumLowerIndentedLinesThreshold), 0.0, TOL) << "Block: " << block->toString();

  block = page0->blocks[8];
  ASSERT_NEAR(computeHangingIndent(
    block,
    lastNamePrefixes,
    hangIndentMinLengthLongLines,
    prevTextLineCapacityThresholdFactor,
    hangIndentMinPercLinesSameLeftMargin,
    hangIndentNumNonIndentedLinesThreshold,
    hangIndentMarginThresholdFactor,
    hangIndentNumLowerNonIndentedLinesThreshold,
    hangIndentNumLongLinesThreshold,
    hangIndentNumLowerIndentedLinesThreshold), 0.0, TOL) << "Block: " << block->toString();

  // Test the three references of the Bibliography.
  block = page0->blocks[10];
  ASSERT_NEAR(computeHangingIndent(
    block,
    lastNamePrefixes,
    hangIndentMinLengthLongLines,
    prevTextLineCapacityThresholdFactor,
    hangIndentMinPercLinesSameLeftMargin,
    hangIndentNumNonIndentedLinesThreshold,
    hangIndentMarginThresholdFactor,
    hangIndentNumLowerNonIndentedLinesThreshold,
    hangIndentNumLongLinesThreshold,
    hangIndentNumLowerIndentedLinesThreshold), 11.0, TOL) << "Block: " << block->toString();

  block = page0->blocks[11];
  ASSERT_NEAR(computeHangingIndent(
    block,
    lastNamePrefixes,
    hangIndentMinLengthLongLines,
    prevTextLineCapacityThresholdFactor,
    hangIndentMinPercLinesSameLeftMargin,
    hangIndentNumNonIndentedLinesThreshold,
    hangIndentMarginThresholdFactor,
    hangIndentNumLowerNonIndentedLinesThreshold,
    hangIndentNumLongLinesThreshold,
    hangIndentNumLowerIndentedLinesThreshold), 11.0, TOL) << "Block: " << block->toString();

  block = page0->blocks[12];
  ASSERT_NEAR(computeHangingIndent(
    block,
    lastNamePrefixes,
    hangIndentMinLengthLongLines,
    prevTextLineCapacityThresholdFactor,
    hangIndentMinPercLinesSameLeftMargin,
    hangIndentNumNonIndentedLinesThreshold,
    hangIndentMarginThresholdFactor,
    hangIndentNumLowerNonIndentedLinesThreshold,
    hangIndentNumLongLinesThreshold,
    hangIndentNumLowerIndentedLinesThreshold), 11.0, TOL) << "Block: " << block->toString();
}

// _________________________________________________________________________________________________
TEST_F(TextBlocksUtilsTest, computeTextLineMarginsPdf1) {
  PdfPage* page0 = pdf1->pages[0];

  // Test the second text block of the Introduction.
  PdfTextBlock* block = page0->blocks[2];
  computeTextLineMargins(block);
  ASSERT_NEAR(block->lines[0]->leftMargin, 10.0, TOL) << "Line: " << block->lines[0]->toString();
  ASSERT_NEAR(block->lines[0]->rightMargin, 0.0, TOL) << "Line: " << block->lines[0]->toString();
  for (size_t i = 1; i < 6; i++) {
    ASSERT_NEAR(block->lines[i]->leftMargin, 0.0, TOL) << "Line: " << block->lines[i]->toString();
    ASSERT_NEAR(block->lines[i]->rightMargin, 0.0, TOL) << "Line: " << block->lines[i]->toString();
  }
  ASSERT_NEAR(block->lines[6]->leftMargin, 0.0, TOL) << "Line: " << block->lines[6]->toString();
  ASSERT_NEAR(block->lines[6]->rightMargin, 139.0, TOL) << "Line: " << block->lines[6]->toString();

  // Test the centered block in the right column of the first page.
  block = page0->blocks[17];
  ASSERT_NEAR(block->lines[0]->leftMargin, 0.0, TOL) << "Line: " << block->lines[0]->toString();
  ASSERT_NEAR(block->lines[0]->rightMargin, 0.0, TOL) << "Line: " << block->lines[0]->toString();
  ASSERT_NEAR(block->lines[1]->leftMargin, 24.0, TOL) << "Line: " << block->lines[1]->toString();
  ASSERT_NEAR(block->lines[1]->rightMargin, 24.0, TOL) << "Line: " << block->lines[1]->toString();
  ASSERT_NEAR(block->lines[2]->leftMargin, 9.0, TOL) << "Line: " << block->lines[2]->toString();
  ASSERT_NEAR(block->lines[2]->rightMargin, 9.0, TOL) << "Line: " << block->lines[2]->toString();

  // Test the last but one block of the first page (the one with the line extending the column
  // boundary.
  block = page0->blocks[19];
  ASSERT_NEAR(block->lines[0]->leftMargin, 10.0, TOL) << "Line: " << block->lines[0]->toString();
  ASSERT_NEAR(block->lines[0]->rightMargin, -25.0, TOL) << "Line: " << block->lines[0]->toString();
  for (size_t i = 1; i < 7; i++) {
    ASSERT_NEAR(block->lines[i]->leftMargin, 0.0, TOL) << "Line: " << block->lines[i]->toString();
    ASSERT_NEAR(block->lines[i]->rightMargin, 0.0, TOL) << "Line: " << block->lines[i]->toString();
  }
  ASSERT_NEAR(block->lines[7]->leftMargin, 0.0, TOL) << "Line: " << block->lines[2]->toString();
  ASSERT_NEAR(block->lines[7]->rightMargin, 48.0, TOL) << "Line: " << block->lines[2]->toString();
}

// _________________________________________________________________________________________________
TEST_F(TextBlocksUtilsTest, createTextBlockPdf1) {
  // TODO(korzen): Read the values from the config.
  int idLength = 8;
  double centeringXOverlapRatioThreshold = 0.99;
  double centeringXOffsetEqualToleranceFactor = 2.0;
  double prevTextLineCapacityThresholdFactor = 2.0;
  const char* formulaIdAlphabet = "=+";
  const int centeringMaxNumJustifiedLines = 5;
  unordered_set<string> lastNamePrefixes = { "van", "von", "de" };
  int hangIndentMinLengthLongLines = 3;
  double hangIndentMinPercLinesSameLeftMargin = 0.5;
  int hangIndentNumNonIndentedLinesThreshold = 10;
  double hangIndentMarginThresholdFactor = 1.0;
  int hangIndentNumLowerNonIndentedLinesThreshold = 0;
  int hangIndentNumLongLinesThreshold = 4;
  int hangIndentNumLowerIndentedLinesThreshold = 1;
  double fontSizeEqualTolerance = 1.0;
  double fontWeightEqualTolerance = 100;

  PdfPage* page0 = pdf1->pages[0];
  PdfPage* page1 = pdf1->pages[1];

  // Test a text block composed of the three text lines of the centered block in the right column
  // of the first page.
  std::vector<PdfTextLine*> lines;
  PdfTextLine* line0 = page0->segments[1]->lines[20];
  PdfTextLine* line1 = page0->segments[1]->lines[21];
  PdfTextLine* line2 = page0->segments[1]->lines[22];
  lines.push_back(line0);
  lines.push_back(line1);
  lines.push_back(line2);

  std::vector<PdfTextBlock*> blocks;
  text_blocks_utils::createTextBlock(
    lines,
    idLength,
    formulaIdAlphabet,
    centeringXOverlapRatioThreshold,
    centeringXOffsetEqualToleranceFactor,
    centeringMaxNumJustifiedLines,
    prevTextLineCapacityThresholdFactor,
    lastNamePrefixes,
    hangIndentMinLengthLongLines,
    hangIndentMinPercLinesSameLeftMargin,
    hangIndentNumNonIndentedLinesThreshold,
    hangIndentMarginThresholdFactor,
    hangIndentNumLowerNonIndentedLinesThreshold,
    hangIndentNumLongLinesThreshold,
    hangIndentNumLowerIndentedLinesThreshold,
    fontSizeEqualTolerance,
    fontWeightEqualTolerance,
    &blocks);

  PdfTextBlock* block = blocks.back();
  ASSERT_EQ(block->id.size(), size_t(idLength + 6));  // +6 for "block-"
  ASSERT_EQ(block->doc, pdf1);
  ASSERT_EQ(block->segment, line0->segment);
  ASSERT_EQ(block->lines, lines);
  ASSERT_EQ(block->pos->pageNum, line0->pos->pageNum);
  ASSERT_NEAR(block->pos->leftX, 349.1, TOL);
  ASSERT_NEAR(block->pos->rightX, 500.7, TOL);
  ASSERT_NEAR(block->pos->upperY, 400.9, TOL);
  ASSERT_NEAR(block->pos->lowerY, 437.7, TOL);
  ASSERT_NEAR(block->trimLeftX, 349.1, TOL);
  ASSERT_NEAR(block->trimRightX, 500.7, TOL);
  ASSERT_NEAR(block->trimUpperY, 400.9, TOL);
  ASSERT_NEAR(block->trimLowerY, 437.7, TOL);
  ASSERT_EQ(block->pos->wMode, line0->pos->wMode);
  ASSERT_EQ(block->pos->rotation, line0->pos->rotation);
  ASSERT_EQ(block->rank, 0);
  ASSERT_EQ(line0->prevLine, nullptr);
  ASSERT_EQ(line0->nextLine, line1);
  ASSERT_EQ(line0->block, block);
  ASSERT_EQ(line1->prevLine, line0);
  ASSERT_EQ(line1->nextLine, line2);
  ASSERT_EQ(line1->block, block);
  ASSERT_EQ(line2->prevLine, line1);
  ASSERT_EQ(line2->nextLine, nullptr);
  ASSERT_EQ(line2->block, block);

  // ================
  // Test a text block composed of the first three text lines of the second page.

  lines.clear();
  line0 = page1->segments[0]->lines[0];
  line1 = page1->segments[0]->lines[1];
  line2 = page1->segments[0]->lines[2];
  lines.push_back(line0);
  lines.push_back(line1);
  lines.push_back(line2);

  blocks.clear();
  text_blocks_utils::createTextBlock(
    lines,
    idLength,
    formulaIdAlphabet,
    centeringXOverlapRatioThreshold,
    centeringXOffsetEqualToleranceFactor,
    centeringMaxNumJustifiedLines,
    prevTextLineCapacityThresholdFactor,
    lastNamePrefixes,
    hangIndentMinLengthLongLines,
    hangIndentMinPercLinesSameLeftMargin,
    hangIndentNumNonIndentedLinesThreshold,
    hangIndentMarginThresholdFactor,
    hangIndentNumLowerNonIndentedLinesThreshold,
    hangIndentNumLongLinesThreshold,
    hangIndentNumLowerIndentedLinesThreshold,
    fontSizeEqualTolerance,
    fontWeightEqualTolerance,
    &blocks);

  block = blocks.back();
  ASSERT_EQ(block->id.size(), size_t(idLength + 6));  // +6 for "block-"
  ASSERT_EQ(block->doc, pdf1);
  ASSERT_EQ(block->segment, line0->segment);
  ASSERT_EQ(block->lines, lines);
  ASSERT_EQ(block->pos->pageNum, line0->pos->pageNum);
  ASSERT_NEAR(block->pos->leftX, 72.0, TOL);
  ASSERT_NEAR(block->pos->rightX, 300.6, TOL);
  ASSERT_NEAR(block->pos->upperY, 125.3, TOL);
  ASSERT_NEAR(block->pos->lowerY, 162.1, TOL);
  ASSERT_NEAR(block->trimLeftX, 72.0, TOL);
  ASSERT_NEAR(block->trimRightX, 300.6, TOL);
  ASSERT_NEAR(block->trimUpperY, 125.3, TOL);
  ASSERT_NEAR(block->trimLowerY, 162.1, TOL);
  ASSERT_EQ(block->pos->wMode, line0->pos->wMode);
  ASSERT_EQ(block->pos->rotation, line0->pos->rotation);
  ASSERT_EQ(block->rank, 0);
  ASSERT_EQ(line0->prevLine, nullptr);
  ASSERT_EQ(line0->nextLine, line1);
  ASSERT_EQ(line0->block, block);
  ASSERT_EQ(line1->prevLine, line0);
  ASSERT_EQ(line1->nextLine, line2);
  ASSERT_EQ(line1->block, block);
  ASSERT_EQ(line2->prevLine, line1);
  ASSERT_EQ(line2->nextLine, nullptr);
  ASSERT_EQ(line2->block, block);

  // ================
  // Test if the trim box of the text block composed of the lines of the last but one text block
  // (the one with the text line extending beyond the column boundaries) is computed correctly.

  lines.clear();
  for (size_t i = 30; i < 37; i++) {
    lines.push_back(page0->segments[1]->lines[i]);
  }
  blocks.clear();
  text_blocks_utils::createTextBlock(
    lines,
    idLength,
    formulaIdAlphabet,
    centeringXOverlapRatioThreshold,
    centeringXOffsetEqualToleranceFactor,
    centeringMaxNumJustifiedLines,
    prevTextLineCapacityThresholdFactor,
    lastNamePrefixes,
    hangIndentMinLengthLongLines,
    hangIndentMinPercLinesSameLeftMargin,
    hangIndentNumNonIndentedLinesThreshold,
    hangIndentMarginThresholdFactor,
    hangIndentNumLowerNonIndentedLinesThreshold,
    hangIndentNumLongLinesThreshold,
    hangIndentNumLowerIndentedLinesThreshold,
    fontSizeEqualTolerance,
    fontWeightEqualTolerance,
    &blocks);

  block = blocks.back();
  ASSERT_EQ(block->pos->pageNum, lines[0]->pos->pageNum);
  ASSERT_NEAR(block->pos->leftX, 310.6, TOL);
  ASSERT_NEAR(block->pos->rightX, 564.2, TOL);
  ASSERT_NEAR(block->pos->upperY, 531.5, TOL);
  ASSERT_NEAR(block->pos->lowerY, 616.2, TOL);
  ASSERT_NEAR(block->trimLeftX, 310.6, TOL);
  ASSERT_NEAR(block->trimRightX, 539.0, TOL);
  ASSERT_NEAR(block->trimUpperY, 531.5, TOL);
  ASSERT_NEAR(block->trimLowerY, 616.2, TOL);
}
