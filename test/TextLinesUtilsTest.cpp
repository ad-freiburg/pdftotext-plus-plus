/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <unordered_set>

#include "../src/PdfToTextPlusPlus.h"
#include "../src/utils/TextLinesUtils.h"

using text_lines_utils::computeHasPrevLineCapacity;
using text_lines_utils::computeIsCentered;
using text_lines_utils::computeIsContinuationOfItem;
using text_lines_utils::computeIsFirstLineOfItem;
using text_lines_utils::computeIsPrefixedByItemLabel;
using text_lines_utils::computeIsPrefixedByFootnoteLabel;
using text_lines_utils::computePotentialFootnoteLabels;

// _________________________________________________________________________________________________
class TextLinesUtilsTest : public ::testing::Test {
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

PdfDocument* TextLinesUtilsTest::pdf1 = nullptr;
PdfDocument* TextLinesUtilsTest::pdf2 = nullptr;

// _________________________________________________________________________________________________
TEST_F(TextLinesUtilsTest, computeIsFirstLineOfItemPdf1) {
  // Test a line with no words.
  PdfTextLine* line = new PdfTextLine();
  ASSERT_FALSE(computeIsFirstLineOfItem(line));

  PdfPageSegment* segment = pdf1->pages[0]->segments[0];

  // Test the heading of the Introduction.
  line = segment->lines[0];
  ASSERT_FALSE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();

  // Test the first five lines of the first block of the Introduction.
  line = segment->lines[1];
  ASSERT_FALSE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[2];
  ASSERT_FALSE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[3];
  ASSERT_FALSE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[4];
  ASSERT_FALSE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[5];
  ASSERT_FALSE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();

  // Test the sixth lines of the first block of the Introduction (starting with "1. scriptorem...").
  // Should return false, since the line is not part of an enumeration, but the body.
  line = segment->lines[6];
  ASSERT_FALSE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();

  // Test the three lines of the first enumeration (starting with "1.", "2.", "3.").
  line = segment->lines[16];
  ASSERT_TRUE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[17];
  ASSERT_TRUE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[18];
  ASSERT_TRUE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();

  // Test the third line of the block after the first enumeration (starting with a superscripted 2).
  // The method should return false since it is not a footnote but part of the body.
  line = segment->lines[21];
  ASSERT_FALSE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();

  // Test the three first item lines of the second enumeration (starting with "-").
  line = segment->lines[26];
  ASSERT_TRUE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[30];
  ASSERT_TRUE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[33];
  ASSERT_TRUE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();

  // Test the footnote at the end of the left column ("1 This is a footnote").
  line = segment->lines[40];
  ASSERT_TRUE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();

  segment = pdf1->pages[0]->segments[1];

  // Test the two lines of the third enumeration (starting with "(a)", "(b)").
  line = segment->lines[9];
  ASSERT_TRUE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[10];
  ASSERT_TRUE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();

  segment = pdf1->pages[1]->segments[0];

  // Test the two footnotes at the end of the left column of the second page.
  line = segment->lines[24];
  ASSERT_TRUE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[25];
  ASSERT_TRUE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();
}

// _________________________________________________________________________________________________
TEST_F(TextLinesUtilsTest, computeIsFirstLineOfItemPdf2) {
  PdfPageSegment* segment = pdf2->pages[0]->segments[0];

  // Iterate through all lines of the first segment, for computing the potential footnote labels.
  std::unordered_set<std::string> labels;
  for (auto* line : segment->lines) {
    computePotentialFootnoteLabels(line, &labels);
  }

  // Test the footnote at the end of the left column (starting with "§") two times: once without
  // passing potential footnote labels, once with passing. Both variants should return true.
  PdfTextLine* line = segment->lines[31];
  ASSERT_TRUE(computeIsFirstLineOfItem(line)) << "Line: " << line->toString();
  ASSERT_TRUE(computeIsFirstLineOfItem(line, &labels)) << "Line: " << line->toString();
}

// _________________________________________________________________________________________________
TEST_F(TextLinesUtilsTest, computeIsContinuationOfItemPdf1) {
  // Test a line with no words.
  PdfTextLine* line = new PdfTextLine();
  ASSERT_FALSE(computeIsContinuationOfItem(line));

  PdfPageSegment* segment = pdf1->pages[0]->segments[0];

  // Test the heading of the Introduction.
  line = segment->lines[0];
  ASSERT_FALSE(computeIsContinuationOfItem(line)) << "Line: " << line->toString();

  // Test the first five lines of the first block of the Introduction.
  line = segment->lines[1];
  ASSERT_FALSE(computeIsContinuationOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[2];
  ASSERT_FALSE(computeIsContinuationOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[3];
  ASSERT_FALSE(computeIsContinuationOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[4];
  ASSERT_FALSE(computeIsContinuationOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[5];
  ASSERT_FALSE(computeIsContinuationOfItem(line)) << "Line: " << line->toString();

  // Test the three lines of the first enumeration (starting with "1.", "2.", "3.").
  line = segment->lines[16];
  ASSERT_FALSE(computeIsContinuationOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[17];
  ASSERT_FALSE(computeIsContinuationOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[18];
  ASSERT_FALSE(computeIsContinuationOfItem(line)) << "Line: " << line->toString();

  // Test the continuation lines of the second enumeration (starting with "-").
  line = segment->lines[27];
  ASSERT_TRUE(computeIsContinuationOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[28];
  ASSERT_TRUE(computeIsContinuationOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[29];
  ASSERT_TRUE(computeIsContinuationOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[31];
  ASSERT_TRUE(computeIsContinuationOfItem(line)) << "Line: " << line->toString();
  line = segment->lines[32];
  ASSERT_TRUE(computeIsContinuationOfItem(line)) << "Line: " << line->toString();

  // Test the footnote at the end of the left column.
  line = segment->lines[40];
  ASSERT_FALSE(computeIsContinuationOfItem(line)) << "Line: " << line->toString();

  segment = pdf1->pages[0]->segments[1];

  // Test the continuation lines of the third enumeration (starting with "(a)", "(b)").
  line = segment->lines[11];
  ASSERT_TRUE(computeIsContinuationOfItem(line)) << "Line: " << line->toString();
}

// _________________________________________________________________________________________________
TEST_F(TextLinesUtilsTest, computeIsPrefixedByItemLabelPdf1) {
  // Test a line with no words.
  PdfTextLine* line = new PdfTextLine();
  ASSERT_FALSE(computeIsPrefixedByItemLabel(line));

  // Test a line with a word with no characters.
  PdfWord* word = new PdfWord();
  line->words.push_back(word);
  ASSERT_FALSE(computeIsPrefixedByItemLabel(line));

  PdfPageSegment* segment = pdf1->pages[0]->segments[0];

  // Test the heading of the Introduction. The method should return false, since "1" is not a valid
  // item label.
  line = segment->lines[0];
  ASSERT_FALSE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();

  // Test the first five lines of the first block of the Introduction.
  line = segment->lines[1];
  ASSERT_FALSE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();
  line = segment->lines[2];
  ASSERT_FALSE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();
  line = segment->lines[3];
  ASSERT_FALSE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();
  line = segment->lines[4];
  ASSERT_FALSE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();
  line = segment->lines[5];
  ASSERT_FALSE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();

  // Test the sixth lines of the first block of the Introduction (starting with "1.").
  // The method should return true, since it starts with an item label.
  line = segment->lines[6];
  ASSERT_TRUE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();

  // Test the three lines of the first enumeration (starting with "1.", "2.", "3.").
  line = segment->lines[16];
  ASSERT_TRUE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();
  line = segment->lines[17];
  ASSERT_TRUE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();
  line = segment->lines[18];
  ASSERT_TRUE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();

  // Test the third line of the block after the first enumeration (starting with a superscripted 2).
  // The method should return true since a superscripted number is a valid label.
  line = segment->lines[21];
  ASSERT_TRUE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();

  // Test the three lines of the second enumeration (starting with "-").
  line = segment->lines[26];
  ASSERT_TRUE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();
  line = segment->lines[30];
  ASSERT_TRUE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();
  line = segment->lines[33];
  ASSERT_TRUE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();

  // Test the footnote at the end of the left column. The method should return true since it starts
  // with an item label.
  line = segment->lines[40];
  ASSERT_TRUE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();

  segment = pdf1->pages[0]->segments[1];

  // Test the two lines of the third enumeration (the enumeration starting with "(a)", "(b)").
  line = segment->lines[9];
  ASSERT_TRUE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();
  line = segment->lines[10];
  ASSERT_TRUE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();

  segment = pdf1->pages[1]->segments[0];

  // Test the two footnotes at the end of the left column of the second page. The method should
  // return true since they start with an item label.
  line = segment->lines[24];
  ASSERT_TRUE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();
  line = segment->lines[25];
  ASSERT_TRUE(computeIsPrefixedByItemLabel(line)) << "Line: " << line->toString();
}

// _________________________________________________________________________________________________
TEST_F(TextLinesUtilsTest, computeIsPrefixedByFootnoteLabelPdf1) {
  // Test a line with no words.
  PdfTextLine* line = new PdfTextLine();
  ASSERT_FALSE(computeIsPrefixedByFootnoteLabel(line));

  PdfPageSegment* segment = pdf1->pages[0]->segments[0];

  // Test the heading of the Introduction.
  line = segment->lines[0];
  ASSERT_FALSE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();

  // Test the first five lines of the first block of the Introduction.
  line = segment->lines[1];
  ASSERT_FALSE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();
  line = segment->lines[2];
  ASSERT_FALSE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();
  line = segment->lines[3];
  ASSERT_FALSE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();
  line = segment->lines[4];
  ASSERT_FALSE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();
  line = segment->lines[5];
  ASSERT_FALSE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();

  // Test the sixth lines of the first block of the Introduction (starting with "1. scriptorem...").
  line = segment->lines[6];
  ASSERT_FALSE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();

  // Test the three lines of the first enumeration (starting with "1.", "2.", "3.").
  line = segment->lines[16];
  ASSERT_FALSE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();
  line = segment->lines[17];
  ASSERT_FALSE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();
  line = segment->lines[18];
  ASSERT_FALSE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();

  // Test the third line of the block after the first enumeration (starting with a superscripted 2).
  // The method should return true a superscripted number is a valid fn label.
  line = segment->lines[21];
  ASSERT_TRUE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();

  // Test the three lines of the second enumeration (starting with "-").
  line = segment->lines[26];
  ASSERT_FALSE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();
  line = segment->lines[30];
  ASSERT_FALSE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();
  line = segment->lines[33];
  ASSERT_FALSE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();

  // Test the footnote at the end of the left column. The method should return true since it starts
  // with a footnote label.
  line = segment->lines[40];
  ASSERT_TRUE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();

  segment = pdf1->pages[0]->segments[1];

  // Test the two first item lines of the third enumeration (starting with "(a)", "(b)").
  line = segment->lines[9];
  ASSERT_FALSE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();
  line = segment->lines[10];
  ASSERT_FALSE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();

  segment = pdf1->pages[1]->segments[0];

  // Test the two footnotes at the end of the left column of the second page. The method should
  // return true since they start with a footnote label.
  line = segment->lines[24];
  ASSERT_TRUE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();
  line = segment->lines[25];
  ASSERT_TRUE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();
}

// _________________________________________________________________________________________________
TEST_F(TextLinesUtilsTest, computeIsPrefixedByFootnoteLabelPdf2) {
  PdfPageSegment* segment = pdf2->pages[0]->segments[0];

  // Iterate through all lines of the first segment, for computing the potential footnote labels.
  std::unordered_set<std::string> labels;
  for (auto* line : segment->lines) {
    computePotentialFootnoteLabels(line, &labels);
  }

  // Test the first line of the first block of the Introduction two times: once without passing
  // potential footnote labels, once with passing. Both variants should return false.
  PdfTextLine* line = segment->lines[4];
  ASSERT_FALSE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();
  ASSERT_FALSE(computeIsPrefixedByFootnoteLabel(line, &labels)) << "Line: " << line->toString();

  // Test the footnote at the end of the left column (starting with "§") two times: once without
  // passing potential footnote labels, once with passing. Both variants should return true.
  line = segment->lines[31];
  ASSERT_TRUE(computeIsPrefixedByFootnoteLabel(line)) << "Line: " << line->toString();
  ASSERT_TRUE(computeIsPrefixedByFootnoteLabel(line, &labels)) << "Line: " << line->toString();
}

// _________________________________________________________________________________________________
TEST_F(TextLinesUtilsTest, computeHasPrevLineCapacityPdf1) {
  // Test a line with no words.
  PdfTextLine* line = new PdfTextLine();
  ASSERT_FALSE(computeHasPrevLineCapacity(line));

  PdfPageSegment* segment = pdf1->pages[0]->segments[0];

  // Test the first line of the second block of the Introduction ("Sed at eirmod...").
  // TODO(korzen): Need to update the prevLine reference, since it's overriden by TextBlockDetector.
  PdfTextLine* prevLine = segment->lines[8];
  line = segment->lines[9];
  line->prevLine = prevLine;
  ASSERT_TRUE(computeHasPrevLineCapacity(line)) << "Line: " << line->toString();

  // Test the second line of the second block of the Introduction ("tam, utinam...").
  prevLine = segment->lines[9];
  line = segment->lines[10];
  line->prevLine = prevLine;
  ASSERT_FALSE(computeHasPrevLineCapacity(line)) << "Line: " << line->toString();

  // Test the first line of the second enumeration ("- This is an item...").
  prevLine = segment->lines[25];
  line = segment->lines[26];
  line->prevLine = prevLine;
  ASSERT_TRUE(computeHasPrevLineCapacity(line)) << "Line: " << line->toString();

  // Test the first line of the second item of the second enumeration ("- This is the second...").
  prevLine = segment->lines[29];
  line = segment->lines[30];
  line->prevLine = prevLine;
  ASSERT_TRUE(computeHasPrevLineCapacity(line)) << "Line: " << line->toString();

  segment = pdf1->pages[0]->segments[1];

  // Test the third line of the right column ("Sed at eirmod...").
  prevLine = segment->lines[1];
  line = segment->lines[2];
  line->prevLine = prevLine;
  ASSERT_TRUE(computeHasPrevLineCapacity(line)) << "Line: " << line->toString();

  prevLine = segment->lines[2];
  line = segment->lines[3];
  line->prevLine = prevLine;
  ASSERT_FALSE(computeHasPrevLineCapacity(line)) << "Line: " << line->toString();

  // Test the first line of last block in the first page ("Namliber tempor..."). The method should
  // return false, because the capacity of the previous line is not large enough for the first
  // word of the line.
  prevLine = segment->lines[29];
  line = segment->lines[30];
  line->prevLine = prevLine;
  ASSERT_FALSE(computeHasPrevLineCapacity(line)) << "Line: " << line->toString();
}

// _________________________________________________________________________________________________
TEST_F(TextLinesUtilsTest, computeTextLineHierarchyPdf1) {
  PdfPage* page1 = pdf1->pages[0];
  text_lines_utils::computeTextLineHierarchy(page1);

  PdfPageSegment* segment = page1->segments[0];

  // Test the heading of the Introduction.
  PdfTextLine* line = segment->lines[0];
  ASSERT_EQ(line->parentLine, nullptr) << "Line: " << line->toString();
  ASSERT_EQ(line->prevSiblingLine, nullptr) << "Line: " << line->toString();
  ASSERT_EQ(line->nextSiblingLine, segment->lines[1]) << "Line: " << line->toString();

  // Test the first line of the first block of the Introduction.
  line = segment->lines[1];
  ASSERT_EQ(line->parentLine, nullptr) << "Line: " << line->toString();
  ASSERT_EQ(line->prevSiblingLine, segment->lines[0]) << "Line: " << line->toString();
  ASSERT_EQ(line->nextSiblingLine, segment->lines[2]) << "Line: " << line->toString();

  // Test the first line of the second block of the Introduction ("Sed at eirmod...").
  line = segment->lines[9];
  ASSERT_EQ(line->parentLine, segment->lines[8]) << "Line: " << line->toString();
  ASSERT_EQ(line->prevSiblingLine, nullptr) << "Line: " << line->toString();
  ASSERT_EQ(line->nextSiblingLine, nullptr) << "Line: " << line->toString();

  // Test the second item of the first enumeration ("2. This is the second...").
  line = segment->lines[17];
  ASSERT_EQ(line->parentLine, segment->lines[15]) << "Line: " << line->toString();
  ASSERT_EQ(line->prevSiblingLine, segment->lines[16]) << "Line: " << line->toString();
  ASSERT_EQ(line->nextSiblingLine, segment->lines[18]) << "Line: " << line->toString();

  // Test the third line of the first item of the second enumeration ("the item is...").
  line = segment->lines[28];
  ASSERT_EQ(line->parentLine, segment->lines[26]) << "Line: " << line->toString();
  ASSERT_EQ(line->prevSiblingLine, segment->lines[27]) << "Line: " << line->toString();
  ASSERT_EQ(line->nextSiblingLine, segment->lines[29]) << "Line: " << line->toString();

  // Test the second line of the second item of the second enumeration ("usual. How...").
  line = segment->lines[31];
  ASSERT_EQ(line->parentLine, segment->lines[30]) << "Line: " << line->toString();
  ASSERT_EQ(line->prevSiblingLine, nullptr) << "Line: " << line->toString();
  ASSERT_EQ(line->nextSiblingLine, segment->lines[32]) << "Line: " << line->toString();

  segment = page1->segments[1];

  // Test the last line of the third enumeration ("point as the first ...")
  line = segment->lines[11];
  ASSERT_EQ(line->parentLine, segment->lines[10]) << "Line: " << line->toString();
  ASSERT_EQ(line->prevSiblingLine, nullptr) << "Line: " << line->toString();
  ASSERT_EQ(line->nextSiblingLine, nullptr) << "Line: " << line->toString();
}

// _________________________________________________________________________________________________
TEST_F(TextLinesUtilsTest, computePotentialFootnoteLabelsPdf1) {
  PdfPageSegment* segment = pdf1->pages[0]->segments[0];

  // Test the heading of the Introduction.
  std::unordered_set<string> labels;
  PdfTextLine* line = segment->lines[0];
  computePotentialFootnoteLabels(line, &labels);
  ASSERT_EQ(size_t(0), labels.size()) << "Line: " << segment->lines[0]->toString();

  // Test the fifth line of the first text block in the Introduction ("tas iriure...").
  line = segment->lines[5];
  computePotentialFootnoteLabels(line, &labels);
  ASSERT_EQ(size_t(1), labels.size()) << "Line: " << line->toString();
  ASSERT_GT(labels.count("1"), size_t(0)) << "Line: " << line->toString();

  // Test the first line of the first enumeration ("1. This is the first...").
  labels.clear();
  line = segment->lines[16];
  computePotentialFootnoteLabels(line, &labels);
  ASSERT_EQ(size_t(0), labels.size()) << "Line: " << line->toString();

  // Test the third line in the block after the first enumeration ("2Id, vis at..."). Should return
  // no labels, since a superscript should be ignored when it is a prefix of a line.
  labels.clear();
  line = segment->lines[21];
  computePotentialFootnoteLabels(line, &labels);
  ASSERT_EQ(size_t(0), labels.size()) << "Line: " << line->toString();

  // Test the 3rd line in the last block of the left column ("phaedrum te...").
  labels.clear();
  line = segment->lines[36];
  computePotentialFootnoteLabels(line, &labels);
  ASSERT_EQ(size_t(2), labels.size()) << "Line: " << line->toString();
  ASSERT_GT(labels.count("†"), size_t(0)) << "Line: " << line->toString();
  ASSERT_GT(labels.count("‡"), size_t(0)) << "Line: " << line->toString();

  // Test the footnote at the end of the left column. Should return no label, since a superscript
  // should be ignored when it is a prefix of a line.
  labels.clear();
  line = segment->lines[40];
  computePotentialFootnoteLabels(line, &labels);
  ASSERT_EQ(size_t(0), labels.size()) << "Line: " << line->toString();
}

// _________________________________________________________________________________________________
TEST_F(TextLinesUtilsTest, computeIsCenteredPdf1) {
  PdfPageSegment* segment = pdf1->pages[0]->segments[0];

  // Test the heading of the Introduction and the next line.
  PdfTextLine* l1 = segment->lines[0];
  PdfTextLine* l2 = segment->lines[1];
  ASSERT_FALSE(computeIsCentered(l1, l2)) << "L1: " << l1->toString() << "\nL2: " << l2->toString();

  // Test the first five lines of the first block in the Introduction.
  l1 = segment->lines[1];
  l2 = segment->lines[2];
  ASSERT_TRUE(computeIsCentered(l1, l2)) << "L1: " << l1->toString() << "\nL2: " << l2->toString();
  l1 = segment->lines[2];
  l2 = segment->lines[3];
  ASSERT_TRUE(computeIsCentered(l1, l2)) << "L1: " << l1->toString() << "\nL2: " << l2->toString();
  l1 = segment->lines[3];
  l2 = segment->lines[4];
  ASSERT_TRUE(computeIsCentered(l1, l2)) << "L1: " << l1->toString() << "\nL2: " << l2->toString();
  l1 = segment->lines[4];
  l2 = segment->lines[5];
  ASSERT_TRUE(computeIsCentered(l1, l2)) << "L1: " << l1->toString() << "\nL2: " << l2->toString();

  // Test the last line of the first block ("altera interpretaris...") and the next line.
  l1 = segment->lines[8];
  l2 = segment->lines[9];
  ASSERT_FALSE(computeIsCentered(l1, l2)) << "L1: " << l1->toString() << "\nL2: " << l2->toString();

  // Test the last line of the second block ("argumentum at...") and the next line.
  l1 = segment->lines[15];
  l2 = segment->lines[16];
  ASSERT_FALSE(computeIsCentered(l1, l2)) << "L1: " << l1->toString() << "\nL2: " << l2->toString();

  segment = pdf1->pages[0]->segments[1];

  // Test the centered lines in the middle of the right column.
  l1 = segment->lines[19];
  l2 = segment->lines[20];
  ASSERT_FALSE(computeIsCentered(l1, l2)) << "L1: " << l1->toString() << "\nL2: " << l2->toString();
  l1 = segment->lines[20];
  l2 = segment->lines[21];
  ASSERT_TRUE(computeIsCentered(l1, l2)) << "L1: " << l1->toString() << "\nL2: " << l2->toString();
  l1 = segment->lines[21];
  l2 = segment->lines[22];
  ASSERT_TRUE(computeIsCentered(l1, l2)) << "L1: " << l1->toString() << "\nL2: " << l2->toString();
  l1 = segment->lines[22];
  l2 = segment->lines[23];
  ASSERT_TRUE(computeIsCentered(l1, l2)) << "L1: " << l1->toString() << "\nL2: " << l2->toString();
}
