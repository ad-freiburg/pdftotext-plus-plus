/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "../src/PdfDocument.h"
#include "../src/PdfToTextPlusPlus.h"
#include "../src/utils/PdfElementsUtils.h"

using element_utils::computeHasEqualLeftX;
using element_utils::computeHasEqualLowerY;
using element_utils::computeHasEqualRightX;
using element_utils::computeHasEqualUpperY;
using element_utils::computeHorizontalGap;
using element_utils::computeLeftXOffset;
using element_utils::computeMaxXOverlapRatio;
using element_utils::computeMaxYOverlapRatio;
using element_utils::computeOverlapRatios;
using element_utils::computeOverlapsFigure;
using element_utils::computeRightXOffset;
using element_utils::computeVerticalGap;
using element_utils::computeXOverlapRatios;
using element_utils::computeYOverlapRatios;
using text_element_utils::computeEndsWithSentenceDelimiter;
using text_element_utils::computeHasEqualFont;
using text_element_utils::computeHasEqualFontSize;
using text_element_utils::computeIsEmphasized;
using text_element_utils::computeStartsWithUpper;

// The allowed tolerance on comparing two float values.
const double TOL = 0.01;

// _________________________________________________________________________________________________
class PdfElementsUtilsTest : public ::testing::Test {
 protected:
  // This method is called before the first test of this test suite.
  static void SetUpTestSuite() {
    ppp::Config config;
    config.rolesPrediction.modelsDir = CONFIG_SEMANTIC_ROLES_DETECTION_MODELS_DIR;
    PdfToTextPlusPlus engine(config);

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

PdfDocument* PdfElementsUtilsTest::pdf1 = nullptr;
PdfDocument* PdfElementsUtilsTest::pdf2 = nullptr;

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeHorizontalGapPdf1) {
  PdfPage* page0 = pdf1->pages[0];

  // Test the gap between "1" and "Introduction" in the first line of the first page.
  PdfWord* w1 = page0->words[0];
  PdfWord* w2 = page0->words[1];
  double gap = computeHorizontalGap(w1, w2);
  ASSERT_NEAR(gap, 16.14, TOL) << "Word 1: " << w1->toString() << "\nWord 2: " << w2->toString();
  gap = computeHorizontalGap(w2, w1);
  ASSERT_NEAR(gap, 16.14, TOL) << "Word 1: " << w1->toString() << "\nWord 2: " << w2->toString();

  // Test the gap between "Lorem" and "Ipsum" in the second line of the first page.
  w1 = page0->words[2];
  w2 = page0->words[3];
  gap = computeHorizontalGap(w1, w2);
  ASSERT_NEAR(gap, 3.99, TOL) << "Word 1: " << w1->toString() << "\nWord 2: " << w2->toString();
  gap = computeHorizontalGap(w2, w1);
  ASSERT_NEAR(gap, 3.99, TOL) << "Word 1: " << w1->toString() << "\nWord 2: " << w2->toString();
}

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeVerticalGapPdf1) {
  PdfPage* page0 = pdf1->pages[0];

  // Test the gap between "Introduction" (in the first line) and "Lorem" (in the second line).
  PdfWord* w1 = page0->words[1];
  PdfWord* w2 = page0->words[2];
  double gap = computeVerticalGap(w1, w2);
  ASSERT_NEAR(gap, 7.33, TOL) << "Word 1: " << w1->toString() << "\nWord 2: " << w2->toString();
  gap = computeVerticalGap(w2, w1);
  ASSERT_NEAR(gap, 7.33, TOL) << "Word 1: " << w1->toString() << "\nWord 2: " << w2->toString();

  // Test the gap between "ad" (in the second line) and "eos," (in the third line).
  w1 = page0->words[9];
  w2 = page0->words[10];
  gap = computeVerticalGap(w1, w2);
  ASSERT_NEAR(gap, -0.99, TOL) << "Word 1: " << w1->toString() << "\nWord 2: " << w2->toString();
  gap = computeVerticalGap(w2, w1);
  ASSERT_NEAR(gap, -0.99, TOL) << "Word 1: " << w1->toString() << "\nWord 2: " << w2->toString();
}

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeOverlapRatios) {
  auto pair = computeOverlapRatios(12.1, 34.5, 37.1, 40.8);
  ASSERT_NEAR(pair.first, 0.0, TOL);
  ASSERT_NEAR(pair.second, 0.0, TOL);

  pair = computeOverlapRatios(5.0, 15.0, 10.0, 20.0);
  ASSERT_NEAR(pair.first, 0.5, TOL);
  ASSERT_NEAR(pair.second, 0.5, TOL);

  pair = computeOverlapRatios(5.0, 10.0, 5.0, 10.0);
  ASSERT_NEAR(pair.first, 1.0, TOL);
  ASSERT_NEAR(pair.second, 1.0, TOL);

  pair = computeOverlapRatios(10.0, 35.0, 0.0, 100.0);
  ASSERT_NEAR(pair.first, 1.0, TOL);
  ASSERT_NEAR(pair.second, 0.25, TOL);

  pair = computeOverlapRatios(0.0, 100.0, 10.0, 85.0);
  ASSERT_NEAR(pair.first, 0.75, TOL);
  ASSERT_NEAR(pair.second, 1.0, TOL);
}

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeXOverlapRatiosPdf1) {
  PdfPage* page0 = pdf1->pages[0];

  // Test the ratio between "Introduction" (in the first line) and "ad" (in the second line).
  PdfWord* w1 = page0->words[1];
  PdfWord* w2 = page0->words[9];
  auto pair = computeXOverlapRatios(w1, w2);
  ASSERT_NEAR(pair.first, 0.0, TOL)  << "W1: " << w1->toString() << "\nW2: " << w2->toString();
  ASSERT_NEAR(pair.second, 0.0, TOL) << "W1: " << w1->toString() << "\nW2: " << w2->toString();

  // Test the ratio between "Introduction" (in the first line) and "ipsum" (in the second line).
  w1 = page0->words[1];
  w2 = page0->words[3];
  pair = computeXOverlapRatios(w1, w2);
  ASSERT_NEAR(pair.first, 0.29, TOL) << "W1: " << w1->toString() << "\nW2: " << w2->toString();
  ASSERT_NEAR(pair.second, 1.0, TOL) << "W1: " << w1->toString() << "\nW2: " << w2->toString();

  // Test the ratio between "Sed" (in the first line of the second block) and "tam," (in the second
  // line of the second block).
  w1 = page0->segments[0]->lines[9]->words[0];
  w2 = page0->segments[0]->lines[10]->words[0];
  pair = computeXOverlapRatios(w1, w2);
  ASSERT_NEAR(pair.first, 0.64, TOL) << "W1: " << w1->toString() << "\nW2: " << w2->toString();
  ASSERT_NEAR(pair.second, 0.49, TOL) << "W1: " << w1->toString() << "\nW2: " << w2->toString();
}

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeYOverlapRatiosPdf1) {
  PdfPage* page0 = pdf1->pages[0];

  // Test the ratio between "Introduction" (in the first line) and "Lorem" (in the second line).
  PdfWord* w1 = page0->words[1];
  PdfWord* w2 = page0->words[2];
  auto pair = computeYOverlapRatios(w1, w2);
  ASSERT_NEAR(pair.first, 0.0, TOL) << "W1: " << w1->toString() << "\nW2: " << w2->toString();
  ASSERT_NEAR(pair.second, 0.0, TOL) << "W1: " << w1->toString() << "\nW2: " << w2->toString();

  // Test the ratio between "Lorem" and "ipsum" (in the second line).
  w1 = page0->words[2];
  w2 = page0->words[3];
  pair = computeYOverlapRatios(w1, w2);
  ASSERT_NEAR(pair.first, 1.0, TOL) << "W1: " << w1->toString() << "\nW2: " << w2->toString();
  ASSERT_NEAR(pair.second, 1.0, TOL) << "W1: " << w1->toString() << "\nW2: " << w2->toString();

  // Test the ratio between "ad" (in the second line) and "eos" (in the third line).
  w1 = page0->words[9];
  w2 = page0->words[10];
  pair = computeYOverlapRatios(w1, w2);
  ASSERT_NEAR(pair.first, 0.07, TOL) << "W1: " << w1->toString() << "\nW2: " << w2->toString();
  ASSERT_NEAR(pair.second, 0.07, TOL) << "W1: " << w1->toString() << "\nW2: " << w2->toString();
}

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeMaxXOverlapRatioPdf1) {
  PdfPage* page0 = pdf1->pages[0];

  // Test the ratio between "Introduction" (in the first line) and "ad" (in the second line).
  PdfWord* w1 = page0->words[1];
  PdfWord* w2 = page0->words[9];
  double ratio = computeMaxXOverlapRatio(w1, w2);
  ASSERT_NEAR(ratio, 0.0, TOL) << "W1: " << w1->toString() << "\nW2: " << w2->toString();

  // Test the ratio between "Introduction" (in the first line) and "ipsum" (in the second line).
  w1 = page0->words[1];
  w2 = page0->words[3];
  ratio = computeMaxXOverlapRatio(w1, w2);
  ASSERT_NEAR(ratio, 1.0, TOL) << "W1: " << w1->toString() << "\nW2: " << w2->toString();

  // Test the ratio between "Sed" (in the first line of the second block) and the "tam," (in the
  // second line of the second block).
  w1 = page0->segments[0]->lines[9]->words[0];
  w2 = page0->segments[0]->lines[10]->words[0];
  ratio = computeMaxXOverlapRatio(w1, w2);
  ASSERT_NEAR(ratio, 0.64, TOL) << "W1: " << w1->toString() << "\nW2: " << w2->toString();
}

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeMaxYOverlapRatioPdf1) {
  PdfPage* page0 = pdf1->pages[0];

  // Test the ratio between "Introduction" (in the first line) and "Lorem" (in the second line).
  PdfWord* w1 = page0->words[1];
  PdfWord* w2 = page0->words[2];
  double ratio = computeMaxYOverlapRatio(w1, w2);
  ASSERT_NEAR(ratio, 0.0, TOL) << "W1: " << w1->toString() << "\nW2: " << w2->toString();

  // Test the ratio between "Lorem" and "ipsum" (in the second line).
  w1 = page0->words[2];
  w2 = page0->words[3];
  ratio = computeMaxYOverlapRatio(w1, w2);
  ASSERT_NEAR(ratio, 1.0, TOL) << "W1: " << w1->toString() << "\nW2: " << w2->toString();

  // Test the ratio between "ad" (in the second line) and "eos," (in the third line).
  w1 = page0->segments[0]->lines[1]->words[7];
  w2 = page0->segments[0]->lines[2]->words[0];
  ratio = computeMaxYOverlapRatio(w1, w2);
  ASSERT_NEAR(ratio, 0.07, TOL) << "W1: " << w1->toString() << "\nW2: " << w2->toString();
}

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeHasEqualLeftXPdf1) {
  PdfPage* page0 = pdf1->pages[0];

  // Test "1" (in the first line) and "Lorem" (in the second line).
  PdfWord* w1 = page0->words[0];
  PdfWord* w2 = page0->words[2];
  ASSERT_TRUE(computeHasEqualLeftX(w1, w2))
      << "W1: " << w1->toString() << "\nW2: " << w2->toString();

  // Test "Lorem" and "ipsum" (in the second line).
  w1 = page0->words[2];
  w2 = page0->words[3];
  ASSERT_FALSE(computeHasEqualLeftX(w1, w2))
      << "W1: " << w1->toString() << "\nW2: " << w2->toString();
}

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeHasEqualUpperYPdf1) {
  PdfPage* page0 = pdf1->pages[0];

  // Test "1" (in the first line) and "Introduction" (in the second line).
  PdfWord* w1 = page0->words[0];
  PdfWord* w2 = page0->words[1];
  ASSERT_TRUE(computeHasEqualUpperY(w1, w2))
      << "W1: " << w1->toString() << "\nW2: " << w2->toString();

  // Test "Introduction" (in the first line) and "Lorem" (in the second line).
  w1 = page0->words[1];
  w2 = page0->words[2];
  ASSERT_FALSE(computeHasEqualUpperY(w1, w2))
      << "W1: " << w1->toString() << "\nW2: " << w2->toString();
}

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeHasEqualRightXPdf1) {
  PdfPage* page0 = pdf1->pages[0];

  // Test "ad" (int the second line) and "phae-" (in the third line).
  PdfWord* w1 = page0->words[9];
  PdfWord* w2 = page0->words[18];
  ASSERT_TRUE(computeHasEqualRightX(w1, w2))
      << "W1: " << w1->toString() << "\nW2: " << w2->toString();

  // Test "Introduction" (in the first line) and "Lorem" (in the second line).
  w1 = page0->words[1];
  w2 = page0->words[2];
  ASSERT_FALSE(computeHasEqualRightX(w1, w2))
      << "W1: " << w1->toString() << "\nW2: " << w2->toString();
}

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeHasEqualLowerYPdf1) {
  PdfPage* page0 = pdf1->pages[0];

  // Test "1" (in the first line) and "Introduction" (in the second line).
  PdfWord* w1 = page0->words[0];
  PdfWord* w2 = page0->words[1];
  ASSERT_TRUE(computeHasEqualLowerY(w1, w2))
      << "W1: " << w1->toString() << "\nW2: " << w2->toString();

  // Test "Introduction" (in the first line) and "Lorem" (in the second line).
  w1 = page0->words[1];
  w2 = page0->words[2];
  ASSERT_FALSE(computeHasEqualLowerY(w1, w2))
      << "W1: " << w1->toString() << "\nW2: " << w2->toString();
}

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeLeftXOffsetPdf1) {
  PdfPage* page0 = pdf1->pages[0];

  // Test "1" and "Introduction" (in the first line).
  PdfWord* w1 = page0->words[0];
  PdfWord* w2 = page0->words[1];
  ASSERT_NEAR(computeLeftXOffset(w2, w1), 24.2, TOL)
      << "W1: " << w1->toString() << "\nW2: " << w2->toString();

  // Test "Lorem" (in the second line) and "eos," (in the third line).
  w1 = page0->words[2];
  w2 = page0->words[10];
  ASSERT_NEAR(computeLeftXOffset(w2, w1), 0.0, TOL)
      << "W1: " << w1->toString() << "\nW2: " << w2->toString();
}

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeRightXOffsetPdf1) {
  PdfPage* page0 = pdf1->pages[0];

  // Test "1" (in the first line) and "Lorem" (in the second line).
  PdfWord* w1 = page0->words[0];
  PdfWord* w2 = page0->words[1];
  ASSERT_NEAR(computeRightXOffset(w2, w1), 104.95, TOL)
      << "W1: " << w1->toString() << "\nW2: " << w2->toString();

  // Test "ad" (in the second line) and "phae-" (in the third line).
  w1 = page0->segments[0]->lines[1]->words[7];
  w2 = page0->segments[0]->lines[2]->words[8];
  ASSERT_NEAR(computeRightXOffset(w2, w1), 0.0, TOL)
      << "W1: " << w1->toString() << "\nW2: " << w2->toString();
}

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeOverlapsFigurePdf1) {
  // TODO(korzen): Read from config
  double minXOverlapRatio = 0.5;
  double minYOverlapRatio = 0.5;

  PdfPage* page1 = pdf1->pages[1];
  std::vector<PdfFigure*>& figures = page1->figures;

  // Test the first line of the second page ("Lorem ipsum...").
  PdfTextLine* line = page1->segments[0]->lines[0];
  ASSERT_EQ(computeOverlapsFigure(line, minXOverlapRatio, minYOverlapRatio, figures), nullptr)
      << "Line: " << line->toString();

  // Test the second line of the second page ("vel ne dolore...").
  line = page1->segments[0]->lines[1];
  ASSERT_EQ(computeOverlapsFigure(line, minXOverlapRatio, minYOverlapRatio, figures), nullptr)
      << "Line: " << line->toString();

  // Test the first character ("f") in Figure 1 on the second page.
  PdfCharacter* ch = figures[0]->characters[0];
  ASSERT_EQ(computeOverlapsFigure(ch, minXOverlapRatio, minYOverlapRatio, figures), figures[0])
      << "Character: " << ch->toString();

  // Test the second character ("o") in Figure 1 on the second page.
  ch = figures[0]->characters[1];
  ASSERT_EQ(computeOverlapsFigure(ch, minXOverlapRatio, minYOverlapRatio, figures), figures[0])
      << "Character: " << ch->toString();

  // Test the third character ("o") in Figure 1 on the second page.
  ch = figures[0]->characters[2];
  ASSERT_EQ(computeOverlapsFigure(ch, minXOverlapRatio, minYOverlapRatio, figures), figures[0])
      << "Character: " << ch->toString();
}

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeHasEqualFontPdf1) {
  PdfPage* page0 = pdf1->pages[0];

  // Test "Introduction" (in the first line) and "Lorem" (in the second line).
  PdfWord* w1 = page0->words[1];
  PdfWord* w2 = page0->words[2];
  ASSERT_FALSE(computeHasEqualFont(w1, w2))
      << "W1: " << w1->toString() << "\nW2: " << w2->toString();

  // Test "Lorem" and "ipsum" (in the second line).
  w1 = page0->words[2];
  w2 = page0->words[3];
  ASSERT_TRUE(computeHasEqualFont(w1, w2))
      << "W1: " << w1->toString() << "\nW2: " << w2->toString();
}

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeHasEqualFontSizePdf1) {
  // TODO(korzen): Read from config.
  double fontSizeEqualTolerance = 1.0;

  PdfPage* page0 = pdf1->pages[0];

  // Test "Introduction" (in the first line) and "Lorem" (in the second line).
  PdfWord* w1 = page0->words[1];
  PdfWord* w2 = page0->words[2];
  ASSERT_FALSE(computeHasEqualFontSize(w1, w2, fontSizeEqualTolerance))
      << "W1: " << w1->toString() << "\nW2: " << w2->toString();

  // Test "Lorem" and "ipsum" (in the second line).
  w1 = page0->words[2];
  w2 = page0->words[3];
  ASSERT_TRUE(computeHasEqualFontSize(w1, w2, fontSizeEqualTolerance))
      << "W1: " << w1->toString() << "\nW2: " << w2->toString();
}

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeEndsWithSentenceDelimiterPdf1) {
  // TODO(korzen): Read from config.
  string delimAlphabet = "?!.);";

  PdfPage* page0 = pdf1->pages[0];

  // Test "Introduction" (in the first line).
  PdfWord* w = page0->words[1];
  ASSERT_FALSE(computeEndsWithSentenceDelimiter(w, delimAlphabet)) << "Word: " << w->toString();

  // Test "Lorem" (in the second line).
  w = page0->words[2];
  ASSERT_FALSE(computeEndsWithSentenceDelimiter(w, delimAlphabet)) << "Word: " << w->toString();

  // Test "laboramus." (in the third line).
  w = page0->segments[0]->lines[2]->words[4];
  ASSERT_TRUE(computeEndsWithSentenceDelimiter(w, delimAlphabet)) << "Word: " << w->toString();

  // Test "eum." (in the fourth line).
  w = page0->segments[0]->lines[3]->words[5];
  ASSERT_TRUE(computeEndsWithSentenceDelimiter(w, delimAlphabet)) << "Word: " << w->toString();

  // Test "laboramus?" (in the fifth line).
  w = page0->segments[0]->lines[4]->words[5];
  ASSERT_TRUE(computeEndsWithSentenceDelimiter(w, delimAlphabet)) << "Word: " << w->toString();
}

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeStartsWithUpperPdf1) {
  PdfPage* page0 = pdf1->pages[0];

  // Test "Introduction" (in the first line).
  PdfWord* w = page0->words[1];
  ASSERT_TRUE(computeStartsWithUpper(w)) << "Word: " << w->toString();

  // Test "Lorem" (in the second line).
  w = page0->words[2];
  ASSERT_TRUE(computeStartsWithUpper(w)) << "Word: " << w->toString();

  // Test "ipsum" (in the second line).
  w = page0->words[3];
  ASSERT_FALSE(computeStartsWithUpper(w)) << "Word: " << w->toString();
}

// _________________________________________________________________________________________________
TEST_F(PdfElementsUtilsTest, computeIsEmphasizedPdf1) {
  // TODO(korzen): Read from config
  double fontSizeEqualTolerance = 0.1;
  double fontWeightEqualTolerance = 1.0;

  PdfPage* page0 = pdf1->pages[0];
  PdfPage* page1 = pdf1->pages[1];

  // Test "1 Introduction" (the first line).
  PdfTextLine* line = page0->segments[0]->lines[0];
  ASSERT_TRUE(computeIsEmphasized(line, fontSizeEqualTolerance, fontWeightEqualTolerance))
      << "Line: " << line->toString();

  // Test "Lorem ipsum..." (the second line, not emphasized).
  line = page0->segments[0]->lines[1];
  ASSERT_FALSE(computeIsEmphasized(line, fontSizeEqualTolerance, fontWeightEqualTolerance))
      << "Line: " << line->toString();

  // Test "vel ne dolore..." (the second line of the second page, printed in bold).
  line = page1->segments[0]->lines[1];
  ASSERT_TRUE(computeIsEmphasized(line, fontSizeEqualTolerance, fontWeightEqualTolerance))
      << "Line: " << line->toString();

  // Test "EIRMOD" (the third word in the fourth line of the second page, printed in uppercase).
  PdfWord* w = page1->segments[0]->lines[3]->words[2];
  ASSERT_TRUE(computeIsEmphasized(w, fontSizeEqualTolerance, fontWeightEqualTolerance))
      << "Word: " << w->toString();

  // Test "uti deleniti..." (the fifth text line of the second page, printed in larger font size).
  line = page1->segments[0]->lines[4];
  ASSERT_TRUE(computeIsEmphasized(line, fontSizeEqualTolerance, fontWeightEqualTolerance))
      << "Line: " << line->toString();
}
