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
  PdfWord word1;
  word1.fontName = "Arial";
  PdfWord word2;
  word2.fontName = "Arial";
  PdfWord word3;
  word3.fontName = "Times";
  ASSERT_TRUE(text_element_utils::computeHasEqualFont(&word1, &word2));
  ASSERT_FALSE(text_element_utils::computeHasEqualFont(&word1, &word3));
}

// _________________________________________________________________________________________________
TEST(PdfTextElementsUtils, computeHasEqualFontSize) {
  PdfWord word1;
  word1.fontSize = 11.0;
  PdfWord word2;
  word2.fontSize = 11.2;
  PdfWord word3;
  word3.fontSize = 13.4;
  ASSERT_TRUE(text_element_utils::computeHasEqualFontSize(&word1, &word2));
  ASSERT_FALSE(text_element_utils::computeHasEqualFontSize(&word1, &word3));
}

// _________________________________________________________________________________________________
TEST(PdfTextElementsUtils, computeEndsWithSentenceDelimiter) {
  PdfWord word1;
  word1.text = "foo.";
  PdfWord word2;
  word2.text = "foo?";
  PdfWord word3;
  word3.text = "foo!";
  PdfWord word4;
  word4.text = "foo";
  ASSERT_TRUE(text_element_utils::computeEndsWithSentenceDelimiter(&word1));
  ASSERT_TRUE(text_element_utils::computeEndsWithSentenceDelimiter(&word2));
  ASSERT_TRUE(text_element_utils::computeEndsWithSentenceDelimiter(&word3));
  ASSERT_FALSE(text_element_utils::computeEndsWithSentenceDelimiter(&word4));
}

// _________________________________________________________________________________________________
TEST(PdfTextElementsUtils, computeStartsWithUpper) {
  PdfWord word1;
  word1.text = "foo";
  PdfWord word2;
  word2.text = "Foo";
  ASSERT_FALSE(text_element_utils::computeStartsWithUpper(&word1));
  ASSERT_TRUE(text_element_utils::computeStartsWithUpper(&word2));
}

// _________________________________________________________________________________________________
TEST(PdfTextElementsUtils, computeIsEmphasized) {
  PdfDocument* doc = new PdfDocument();
  doc->mostFreqFontName = "Arial";
  doc->mostFreqFontSize = 11.9;

  PdfFontInfo* arial = new PdfFontInfo();
  arial->fontName = "Arial";
  arial->weight = 400;
  doc->fontInfos[arial->fontName] = arial;

  PdfFontInfo* times = new PdfFontInfo();
  times->fontName = "Times";
  times->weight = 400;
  doc->fontInfos[times->fontName] = times;

  PdfFontInfo* timesBold = new PdfFontInfo();
  timesBold->fontName = "TimesBold";
  timesBold->weight = 600;
  doc->fontInfos[timesBold->fontName] = timesBold;

  PdfFontInfo* arialIt = new PdfFontInfo();
  arialIt->fontName = "ArialItalic";
  arialIt->weight = 400;
  arialIt->isItalic = true;
  doc->fontInfos[arialIt->fontName] = arialIt;

  PdfWord* word1 = new PdfWord();
  word1->fontName = "Arial";
  word1->fontSize = 9.9;
  word1->doc = doc;
  // Not emphasized because smaller font size.
  ASSERT_FALSE(text_element_utils::computeIsEmphasized(word1));

  PdfWord* word2 = new PdfWord();
  word2->fontName = "Arial";
  word2->fontSize = 11.9;
  word2->doc = doc;
  // Not emphasized because equal font size.
  ASSERT_FALSE(text_element_utils::computeIsEmphasized(word2));

  PdfWord* word3 = new PdfWord();
  word3->fontName = "Arial";
  word3->fontSize = 12.4;
  word3->doc = doc;
  // Not emphasized because the difference between the font sizes is smaller than the threshold.
  ASSERT_FALSE(text_element_utils::computeIsEmphasized(word3));

  PdfWord* word4 = new PdfWord();
  word4->fontName = "Arial";
  word4->fontSize = 13.0;
  word4->doc = doc;
  // Emphasized because larger font size.
  ASSERT_TRUE(text_element_utils::computeIsEmphasized(word4));

  PdfWord* word5 = new PdfWord();
  word5->fontName = "Times";
  word5->fontSize = 11.9;
  word5->doc = doc;
  // Not emphasized because same most frequent font weight.
  ASSERT_FALSE(text_element_utils::computeIsEmphasized(word5));

  PdfWord* word6 = new PdfWord();
  word6->fontName = "TimesBold";
  word6->fontSize = 11.9;
  word6->doc = doc;
  // Emphasized because font size is not smaller and font weight is larger.
  ASSERT_TRUE(text_element_utils::computeIsEmphasized(word6));

  PdfWord* word7 = new PdfWord();
  word7->fontName = "TimesBold";
  word7->fontSize = 9.9;
  word7->doc = doc;
  // Not emphasized because font size is smaller.
  ASSERT_FALSE(text_element_utils::computeIsEmphasized(word7));

  PdfWord* word8 = new PdfWord();
  word8->fontName = "ArialItalic";
  word8->fontSize = 11.9;
  word8->doc = doc;
  // Emphasized because font size is not smaller and printed in italics.
  ASSERT_TRUE(text_element_utils::computeIsEmphasized(word8));

  PdfWord* word9 = new PdfWord();
  word9->fontName = "ArialItalic";
  word9->fontSize = 10.0;
  word9->doc = doc;
  // Not emphasized because font size is smaller.
  ASSERT_FALSE(text_element_utils::computeIsEmphasized(word9));

  PdfWord* word0 = new PdfWord();
  word0->fontName = "Arial";
  word0->fontSize = 11.9;
  word0->text = "INTRODUCTION";
  word0->doc = doc;
  // Emphasized because font size is not smaller and text is in uppercase.
  ASSERT_FALSE(text_element_utils::computeIsEmphasized(word0));
}
