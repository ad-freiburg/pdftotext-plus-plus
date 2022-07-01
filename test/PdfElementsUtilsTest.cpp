/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <vector>

#include "../src/utils/PdfElementsUtils.h"

#include "../src/Constants.h"
#include "../src/PdfDocument.h"

using global_config::DOUBLE_EQUAL_TOLERANCE;

// _________________________________________________________________________________________________
TEST(PdfElementsUtils, computeHorizontalGap) {
  PdfWord* word1 = new PdfWord(1, 23.1, 451.2, 31.8, 475.2, 0, 0);
  PdfWord* word2 = new PdfWord(1, 31.8, 451.2, 47.1, 475.2, 0, 0);
  ASSERT_NEAR(element_utils::computeHorizontalGap(word1, word2), 0.0, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(element_utils::computeHorizontalGap(word2, word1), 0.0, DOUBLE_EQUAL_TOLERANCE);

  PdfWord* word3 = new PdfWord(1, 23.1, 451.2, 31.8, 475.2, 0, 0);
  PdfWord* word4 = new PdfWord(1, 34.2, 451.2, 47.1, 475.2, 0, 0);
  ASSERT_NEAR(element_utils::computeHorizontalGap(word3, word4), 2.4, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(element_utils::computeHorizontalGap(word4, word3), 2.4, DOUBLE_EQUAL_TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtils, computeVerticalGap) {
  PdfWord* word1 = new PdfWord(1, 23.1, 451.2, 31.8, 475.2, 0, 0);
  PdfWord* word2 = new PdfWord(1, 34.2, 475.2, 47.1, 485.3, 0, 0);
  ASSERT_NEAR(element_utils::computeVerticalGap(word1, word2), 0.0, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(element_utils::computeVerticalGap(word2, word1), 0.0, DOUBLE_EQUAL_TOLERANCE);

  PdfWord* word3 = new PdfWord(1, 23.1, 451.2, 31.8, 475.2, 0, 0);
  PdfWord* word4 = new PdfWord(1, 34.2, 480.1, 47.1, 485.3, 0, 0);
  ASSERT_NEAR(element_utils::computeVerticalGap(word3, word4), 4.9, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(element_utils::computeVerticalGap(word4, word3), 4.9, DOUBLE_EQUAL_TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtils, computeOverlapRatios) {
  auto pair = element_utils::computeOverlapRatios(12.1, 34.5, 37.1, 40.8);
  ASSERT_NEAR(pair.first, 0.0, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(pair.second, 0.0, DOUBLE_EQUAL_TOLERANCE);

  pair = element_utils::computeOverlapRatios(5.0, 15.0, 10.0, 20.0);
  ASSERT_NEAR(pair.first, 0.5, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(pair.second, 0.5, DOUBLE_EQUAL_TOLERANCE);

  pair = element_utils::computeOverlapRatios(5.0, 10.0, 5.0, 10.0);
  ASSERT_NEAR(pair.first, 1.0, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(pair.second, 1.0, DOUBLE_EQUAL_TOLERANCE);

  pair = element_utils::computeOverlapRatios(10.0, 35.0, 0.0, 100.0);
  ASSERT_NEAR(pair.first, 1.0, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(pair.second, 0.25, DOUBLE_EQUAL_TOLERANCE);

  pair = element_utils::computeOverlapRatios(0.0, 100.0, 10.0, 85.0);
  ASSERT_NEAR(pair.first, 0.75, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(pair.second, 1.0, DOUBLE_EQUAL_TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtils, computeXOverlapRatios) {
  PdfWord* word1 = new PdfWord(1, 10.0, 0, 20.0, 0, 0, 0);
  PdfWord* word2 = new PdfWord(1, 25.0, 0, 30.0, 0, 0, 0);
  auto pair = element_utils::computeXOverlapRatios(word1, word2);
  ASSERT_NEAR(pair.first, 0.0, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(pair.second, 0.0, DOUBLE_EQUAL_TOLERANCE);

  PdfWord* word3 = new PdfWord(1, 10.0, 0, 20.0, 0, 0, 0);
  PdfWord* word4 = new PdfWord(1, 10.0, 0, 20.0, 0, 0, 0);
  pair = element_utils::computeXOverlapRatios(word3, word4);
  ASSERT_NEAR(pair.first, 1.0, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(pair.second, 1.0, DOUBLE_EQUAL_TOLERANCE);

  PdfWord* word5 = new PdfWord(1, 10.0, 0, 20.0, 0, 0, 0);
  PdfWord* word6 = new PdfWord(1, 15.0, 0, 25.0, 0, 0, 0);
  pair = element_utils::computeXOverlapRatios(word5, word6);
  ASSERT_NEAR(pair.first, 0.5, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(pair.second, 0.5, DOUBLE_EQUAL_TOLERANCE);

  PdfWord* word7 = new PdfWord(1, 10.0, 0, 20.0, 0, 0, 0);
  PdfWord* word8 = new PdfWord(1, 10.0, 0, 60.0, 0, 0, 0);
  pair = element_utils::computeXOverlapRatios(word7, word8);
  ASSERT_NEAR(pair.first, 1, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(pair.second, 0.2, DOUBLE_EQUAL_TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtils, computeYOverlapRatios) {
  PdfWord* word1 = new PdfWord(1, 0, 10.0, 0, 20.0, 0, 0);
  PdfWord* word2 = new PdfWord(1, 0, 25.0, 0, 30.0, 0, 0);
  auto pair = element_utils::computeYOverlapRatios(word1, word2);
  ASSERT_NEAR(pair.first, 0.0, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(pair.second, 0.0, DOUBLE_EQUAL_TOLERANCE);

  PdfWord* word3 = new PdfWord(1, 0, 10.0, 0, 20.0, 0, 0);
  PdfWord* word4 = new PdfWord(1, 0, 10.0, 0, 20.0, 0, 0);
  pair = element_utils::computeYOverlapRatios(word3, word4);
  ASSERT_NEAR(pair.first, 1.0, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(pair.second, 1.0, DOUBLE_EQUAL_TOLERANCE);

  PdfWord* word5 = new PdfWord(1, 0, 10.0, 0, 20.0, 0, 0);
  PdfWord* word6 = new PdfWord(1, 0, 15.0, 0, 25.0, 0, 0);
  pair = element_utils::computeYOverlapRatios(word5, word6);
  ASSERT_NEAR(pair.first, 0.5, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(pair.second, 0.5, DOUBLE_EQUAL_TOLERANCE);

  PdfWord* word7 = new PdfWord(1, 0, 10.0, 0, 20.0, 0, 0);
  PdfWord* word8 = new PdfWord(1, 0, 10.0, 0, 60.0, 0, 0);
  pair = element_utils::computeYOverlapRatios(word7, word8);
  ASSERT_NEAR(pair.first, 1, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(pair.second, 0.2, DOUBLE_EQUAL_TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtils, computeMaxXOverlapRatio) {
  PdfWord* word1 = new PdfWord(1, 10.0, 0, 20.0, 0, 0, 0);
  PdfWord* word2 = new PdfWord(1, 25.0, 0, 30.0, 0, 0, 0);
  ASSERT_NEAR(element_utils::computeMaxXOverlapRatio(word1, word2), 0.0, DOUBLE_EQUAL_TOLERANCE);

  PdfWord* word3 = new PdfWord(1, 10.0, 0, 20.0, 0, 0, 0);
  PdfWord* word4 = new PdfWord(1, 10.0, 0, 20.0, 0, 0, 0);
  ASSERT_NEAR(element_utils::computeMaxXOverlapRatio(word3, word4), 1.0, DOUBLE_EQUAL_TOLERANCE);

  PdfWord* word5 = new PdfWord(1, 10.0, 0, 20.0, 0, 0, 0);
  PdfWord* word6 = new PdfWord(1, 15.0, 0, 25.0, 0, 0, 0);
  ASSERT_NEAR(element_utils::computeMaxXOverlapRatio(word5, word6), 0.5, DOUBLE_EQUAL_TOLERANCE);

  PdfWord* word7 = new PdfWord(1, 10.0, 0, 90.0, 0, 0, 0);
  PdfWord* word8 = new PdfWord(1, 70.0, 0, 170.0, 0, 0, 0);
  ASSERT_NEAR(element_utils::computeMaxXOverlapRatio(word7, word8), 0.25, DOUBLE_EQUAL_TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtils, computeMaxYOverlapRatio) {
  PdfWord* word1 = new PdfWord(1, 0, 10.0, 0, 20.0, 0, 0);
  PdfWord* word2 = new PdfWord(1, 0, 25.0, 0, 30.0, 0, 0);
  ASSERT_NEAR(element_utils::computeMaxYOverlapRatio(word1, word2), 0.0, DOUBLE_EQUAL_TOLERANCE);

  PdfWord* word3 = new PdfWord(1, 0, 10.0, 0, 20.0, 0, 0);
  PdfWord* word4 = new PdfWord(1, 0, 10.0, 0, 20.0, 0, 0);
  ASSERT_NEAR(element_utils::computeMaxYOverlapRatio(word3, word4), 1.0, DOUBLE_EQUAL_TOLERANCE);

  PdfWord* word5 = new PdfWord(1, 0, 10.0, 0, 20.0, 0, 0);
  PdfWord* word6 = new PdfWord(1, 0, 19.0, 0, 24.0, 0, 0);
  ASSERT_NEAR(element_utils::computeMaxYOverlapRatio(word5, word6), 0.2, DOUBLE_EQUAL_TOLERANCE);

  PdfWord* word7 = new PdfWord(1, 0, 10.0, 0, 90.0, 0, 0);
  PdfWord* word8 = new PdfWord(1, 0, 65.0, 0, 170.0, 0, 0);
  ASSERT_NEAR(element_utils::computeMaxYOverlapRatio(word7, word8), 0.3125, DOUBLE_EQUAL_TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtils, computeHasEqualLeftX) {
  PdfWord* word1 = new PdfWord(1, 12.1, 10.0, 17.2, 20.0, 0, 0);
  PdfWord* word2 = new PdfWord(1, 12.1, 25.0, 17.3, 30.0, 0, 0);
  ASSERT_TRUE(element_utils::computeHasEqualLeftX(word1, word2));

  PdfWord* word3 = new PdfWord(1, 12.1, 10.0, 17.2, 20.0, 0, 0);
  PdfWord* word4 = new PdfWord(1, 12.7, 25.0, 17.3, 30.0, 0, 0);
  ASSERT_FALSE(element_utils::computeHasEqualLeftX(word3, word4));
  ASSERT_FALSE(element_utils::computeHasEqualLeftX(word3, word4, 0.5));
  ASSERT_TRUE(element_utils::computeHasEqualLeftX(word3, word4, 0.7));
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtils, computeHasEqualUpperY) {
  PdfWord* word1 = new PdfWord(1, 12.1, 10.0, 17.2, 20.0, 0, 0);
  PdfWord* word2 = new PdfWord(1, 13.1, 10.0, 17.3, 30.0, 0, 0);
  ASSERT_TRUE(element_utils::computeHasEqualUpperY(word1, word2));

  PdfWord* word3 = new PdfWord(1, 12.1, 10.0, 17.2, 20.0, 0, 0);
  PdfWord* word4 = new PdfWord(1, 16.7, 12.5, 17.3, 30.0, 0, 0);
  ASSERT_FALSE(element_utils::computeHasEqualUpperY(word3, word4));
  ASSERT_FALSE(element_utils::computeHasEqualUpperY(word3, word4, 2.4));
  ASSERT_TRUE(element_utils::computeHasEqualUpperY(word3, word4, 2.6));
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtils, computeHasEqualRightX) {
  PdfWord* word1 = new PdfWord(1, 12.1, 10.0, 17.2, 20.0, 0, 0);
  PdfWord* word2 = new PdfWord(1, 15.2, 25.0, 17.2, 30.0, 0, 0);
  ASSERT_TRUE(element_utils::computeHasEqualRightX(word1, word2));

  PdfWord* word3 = new PdfWord(1, 12.1, 10.0, 17.2, 20.0, 0, 0);
  PdfWord* word4 = new PdfWord(1, 12.7, 25.0, 17.9, 30.0, 0, 0);
  ASSERT_FALSE(element_utils::computeHasEqualLeftX(word3, word4));
  ASSERT_FALSE(element_utils::computeHasEqualLeftX(word3, word4, 0.5));
  ASSERT_TRUE(element_utils::computeHasEqualLeftX(word3, word4, 0.7));
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtils, computeHasEqualLowerY) {
  PdfWord* word1 = new PdfWord(1, 12.1, 10.0, 17.2, 20.0, 0, 0);
  PdfWord* word2 = new PdfWord(1, 13.1, 12.3, 17.4, 20.0, 0, 0);
  ASSERT_TRUE(element_utils::computeHasEqualLowerY(word1, word2));

  PdfWord* word3 = new PdfWord(1, 12.1, 10.0, 17.2, 20.0, 0, 0);
  PdfWord* word4 = new PdfWord(1, 13.1, 12.3, 17.4, 21.1, 0, 0);
  ASSERT_FALSE(element_utils::computeHasEqualLowerY(word3, word4));
  ASSERT_FALSE(element_utils::computeHasEqualLowerY(word3, word4, 1.0));
  ASSERT_TRUE(element_utils::computeHasEqualLowerY(word3, word4, 1.2));
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtils, computeLeftXOffset) {
  PdfWord* word1 = new PdfWord(1, 12.1, 10.0, 17.2, 20.0, 0, 0);
  PdfWord* word2 = new PdfWord(1, 12.1, 12.3, 17.4, 20.0, 0, 0);
  ASSERT_NEAR(element_utils::computeLeftXOffset(word1, word2), 0.0, DOUBLE_EQUAL_TOLERANCE);

  PdfWord* word3 = new PdfWord(1, 12.1, 10.0, 17.2, 20.0, 0, 0);
  PdfWord* word4 = new PdfWord(1, 15.2, 12.3, 17.4, 20.0, 0, 0);
  ASSERT_NEAR(element_utils::computeLeftXOffset(word3, word4), -3.1, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(element_utils::computeLeftXOffset(word4, word2), 3.1, DOUBLE_EQUAL_TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtils, computeRightXOffset) {
  PdfWord* word1 = new PdfWord(1, 12.1, 10.0, 17.2, 20.0, 0, 0);
  PdfWord* word2 = new PdfWord(1, 12.5, 12.3, 17.2, 20.0, 0, 0);
  ASSERT_NEAR(element_utils::computeRightXOffset(word1, word2), 0.0, DOUBLE_EQUAL_TOLERANCE);

  PdfWord* word3 = new PdfWord(1, 12.1, 10.0, 17.2, 20.0, 0, 0);
  PdfWord* word4 = new PdfWord(1, 15.2, 12.3, 19.7, 20.0, 0, 0);
  ASSERT_NEAR(element_utils::computeRightXOffset(word3, word4), -2.5, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(element_utils::computeRightXOffset(word4, word2), 2.5, DOUBLE_EQUAL_TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtils, computeOverlapsFigure) {
  PdfFigure figure1(1, 100.0, 200.0, 200.0, 500.0);
  PdfFigure figure2(1, 0.0, 0.0, 100.0, 100.0);
  PdfFigure figure3(1, 300.0, 100.0, 400.0, 200.0);

  std::vector<PdfFigure*> figures;
  figures.push_back(&figure1);
  figures.push_back(&figure2);
  figures.push_back(&figure3);

  PdfWord word1(1, 0.0, 200.0, 10.0, 210.0, 0, 0);
  ASSERT_EQ(element_utils::computeOverlapsFigure(&word1, figures), nullptr);

  PdfWord word2(1, 310.0, 150.0, 320.0, 160.0, 0, 0);
  ASSERT_EQ(element_utils::computeOverlapsFigure(&word2, figures), &figure3);

  PdfWord word3(1, 90.0, 10.0, 101.0, 20.0, 0, 0);
  ASSERT_EQ(element_utils::computeOverlapsFigure(&word3, figures), &figure2);

  PdfWord word4(1, 90.0, 10.0, 140.0, 20.0, 0, 0);
  ASSERT_EQ(element_utils::computeOverlapsFigure(&word4, figures), nullptr);
}

// _________________________________________________________________________________________________
TEST(PdfTextElementsUtils, computeHasEqualFont) {

}

// _________________________________________________________________________________________________
TEST(PdfTextElementsUtils, computeHasEqualFontSize) {

}

// _________________________________________________________________________________________________
TEST(PdfTextElementsUtils, computeEndsWithSentenceDelimiter) {

}

// _________________________________________________________________________________________________
TEST(PdfTextElementsUtils, computeStartsWithUpper) {

}

// _________________________________________________________________________________________________
TEST(PdfTextElementsUtils, computeIsEmphasized) {

}