/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <string>
#include <utility>  // std::pair
#include <vector>

#include "../../src/Config.h"
#include "../../src/PdfDocument.h"
#include "../../src/PdfToTextPlusPlus.h"
#include "../../src/utils/PdfElementsUtils.h"

using std::pair;

using ppp::utils::elements::computeHasEqualFont;
using ppp::utils::elements::computeHasEqualFontSize;
using ppp::utils::elements::computeHasEqualLeftX;
using ppp::utils::elements::computeHasEqualLowerY;
using ppp::utils::elements::computeHasEqualRightX;
using ppp::utils::elements::computeHasEqualUpperY;
using ppp::utils::elements::computeHorizontalGap;
using ppp::utils::elements::computeLeftXOffset;
using ppp::utils::elements::computeMaxXOverlapRatio;
using ppp::utils::elements::computeMaxYOverlapRatio;
using ppp::utils::elements::computeOverlapRatios;
using ppp::utils::elements::computeRightXOffset;
using ppp::utils::elements::computeVerticalGap;
using ppp::utils::elements::computeXOverlapRatios;
using ppp::utils::elements::computeYOverlapRatios;

// =================================================================================================

// The tolerance to use on comparing two float values.
static const double TOL = ppp::config::DEFAULT_DOUBLE_EQUAL_TOLERANCE;

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeHorizontalGap) {
  PdfWord word1(1, 20.0, 240.5, 25.5, 250.0, 0, 0);
  PdfWord word2(1, 27.0, 240.5, 32.2, 250.0, 0, 0);
  PdfWord word3(1, 35.4, 240.5, 40.1, 250.0, 0, 0);

  // Input: one or more nullptrs.
  ASSERT_DEATH(computeHorizontalGap(nullptr, nullptr), "");
  ASSERT_DEATH(computeHorizontalGap(&word1, nullptr), "");
  ASSERT_DEATH(computeHorizontalGap(nullptr, &word2), "");

  ASSERT_NEAR(computeHorizontalGap(&word1, &word2), 1.5, TOL);
  ASSERT_NEAR(computeHorizontalGap(&word2, &word1), 1.5, TOL);
  ASSERT_NEAR(computeHorizontalGap(&word1, &word3), 9.9, TOL);
  ASSERT_NEAR(computeHorizontalGap(&word3, &word1), 9.9, TOL);
  ASSERT_NEAR(computeHorizontalGap(&word2, &word3), 3.2, TOL);
  ASSERT_NEAR(computeHorizontalGap(&word3, &word2), 3.2, TOL);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeVerticalGap) {
  PdfWord word1(1, 20.0, 240.5, 25.5, 245.1, 0, 0);
  PdfWord word2(1, 27.0, 247.5, 32.2, 250.5, 0, 0);
  PdfWord word3(1, 35.4, 253.5, 40.1, 257.8, 0, 0);

  // Input: one or more nullptrs.
  ASSERT_DEATH(computeVerticalGap(nullptr, nullptr), "");
  ASSERT_DEATH(computeVerticalGap(&word1, nullptr), "");
  ASSERT_DEATH(computeVerticalGap(nullptr, &word2), "");

  ASSERT_NEAR(computeVerticalGap(&word1, &word2), 2.4, TOL);
  ASSERT_NEAR(computeVerticalGap(&word2, &word1), 2.4, TOL);
  ASSERT_NEAR(computeVerticalGap(&word1, &word3), 8.4, TOL);
  ASSERT_NEAR(computeVerticalGap(&word3, &word1), 8.4, TOL);
  ASSERT_NEAR(computeVerticalGap(&word2, &word3), 3.0, TOL);
  ASSERT_NEAR(computeVerticalGap(&word3, &word2), 3.0, TOL);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeOverlapRatios) {
  // Input: two zero-length intervals.
  pair<double, double> ratios = computeOverlapRatios(0.0, 0.0, 0.0, 0.0);
  ASSERT_NEAR(ratios.first, 0.0, TOL);
  ASSERT_NEAR(ratios.second, 0.0, TOL);

  // Input: two identical intervals.
  ratios = computeOverlapRatios(5.0, 10.0, 5.0, 10.0);
  ASSERT_NEAR(ratios.first, 1.0, TOL);
  ASSERT_NEAR(ratios.second, 1.0, TOL);
  ratios = computeOverlapRatios(10.0, 5.0, 10.0, 5.0);
  ASSERT_NEAR(ratios.first, 1.0, TOL);
  ASSERT_NEAR(ratios.second, 1.0, TOL);

  // Input: two intervals that do not overlap.
  ratios = computeOverlapRatios(12.0, 34.0, 37.0, 40.0);
  ASSERT_NEAR(ratios.first, 0.0, TOL);
  ASSERT_NEAR(ratios.second, 0.0, TOL);
  ratios = computeOverlapRatios(34.0, 12.0, 40.0, 37.0);
  ASSERT_NEAR(ratios.first, 0.0, TOL);
  ASSERT_NEAR(ratios.second, 0.0, TOL);

  // Input: two intervals that partially overlap.
  ratios = computeOverlapRatios(1.0, 7.0, 4.0, 16.0);
  ASSERT_NEAR(ratios.first, 0.5, TOL);
  ASSERT_NEAR(ratios.second, 0.25, TOL);
  ratios = computeOverlapRatios(7.0, 1.0, 16.0, 4.0);
  ASSERT_NEAR(ratios.first, 0.5, TOL);
  ASSERT_NEAR(ratios.second, 0.25, TOL);
  ratios = computeOverlapRatios(4.0, 16.0, 1.0, 7.0);
  ASSERT_NEAR(ratios.first, 0.25, TOL);
  ASSERT_NEAR(ratios.second, 0.5, TOL);

  ratios = computeOverlapRatios(5.0, 15.0, 10.0, 20.0);
  ASSERT_NEAR(ratios.first, 0.5, TOL);
  ASSERT_NEAR(ratios.second, 0.5, TOL);
  ratios = computeOverlapRatios(15.0, 5.0, 20.0, 10.0);
  ASSERT_NEAR(ratios.first, 0.5, TOL);
  ASSERT_NEAR(ratios.second, 0.5, TOL);

  // Input: two intervals, with one interval completely falling into the other.
  ratios = computeOverlapRatios(10.0, 35.0, 0.0, 100.0);
  ASSERT_NEAR(ratios.first, 1.0, TOL);
  ASSERT_NEAR(ratios.second, 0.25, TOL);
  ratios = computeOverlapRatios(35.0, 10.0, 100.0, 0.0);
  ASSERT_NEAR(ratios.first, 1.0, TOL);
  ASSERT_NEAR(ratios.second, 0.25, TOL);

  ratios = computeOverlapRatios(0.0, 100.0, 10.0, 85.0);
  ASSERT_NEAR(ratios.first, 0.75, TOL);
  ASSERT_NEAR(ratios.second, 1.0, TOL);
  ratios = computeOverlapRatios(100.0, 0.0, 85.0, 10.0);
  ASSERT_NEAR(ratios.first, 0.75, TOL);
  ASSERT_NEAR(ratios.second, 1.0, TOL);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeXOverlapRatios) {
  PdfWord word1(1, 12.0, 75.0, 18.0, 80.5, 0, 0);
  PdfWord word2(1, 20.0, 75.0, 28.0, 80.5, 0, 0);
  PdfWord word3(1, 15.0, 75.0, 27.0, 80.5, 0, 0);
  PdfWord word4(1, 10.0, 75.0, 20.0, 80.5, 0, 0);

  // Input: one or more nullptrs.
  ASSERT_DEATH(computeXOverlapRatios(nullptr, nullptr), "");
  ASSERT_DEATH(computeXOverlapRatios(&word1, nullptr), "");
  ASSERT_DEATH(computeXOverlapRatios(nullptr, &word2), "");

  // Input: two words that do not overlap horizontally.
  pair<double, double> pair = computeXOverlapRatios(&word1, &word2);
  ASSERT_NEAR(pair.first, 0.0, TOL);
  ASSERT_NEAR(pair.second, 0.0, TOL);
  pair = computeXOverlapRatios(&word2, &word1);
  ASSERT_NEAR(pair.first, 0.0, TOL);
  ASSERT_NEAR(pair.second, 0.0, TOL);

  // Input: two words that partially overlap horizontally.
  pair = computeXOverlapRatios(&word1, &word3);
  ASSERT_NEAR(pair.first, 0.5, TOL);
  ASSERT_NEAR(pair.second, 0.25, TOL);
  pair = computeXOverlapRatios(&word3, &word1);
  ASSERT_NEAR(pair.first, 0.25, TOL);
  ASSERT_NEAR(pair.second, 0.5, TOL);

  // Input: two words, with one word completely overlapping the other.
  pair = computeXOverlapRatios(&word1, &word4);
  ASSERT_NEAR(pair.first, 1.0, TOL);
  ASSERT_NEAR(pair.second, 0.6, TOL);
  pair = computeXOverlapRatios(&word4, &word1);
  ASSERT_NEAR(pair.first, 0.6, TOL);
  ASSERT_NEAR(pair.second, 1.0, TOL);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeYOverlapRatios) {
  PdfWord word1(1, 7.5, 17.0, 12.0, 27.0, 0, 0);
  PdfWord word2(1, 7.5, 30.0, 12.0, 37.0, 0, 0);
  PdfWord word3(1, 7.5, 24.0, 12.0, 29.0, 0, 0);
  PdfWord word4(1, 7.5, 11.0, 12.0, 36.0, 0, 0);

  // Input: one or more nullptrs.
  ASSERT_DEATH(computeYOverlapRatios(nullptr, nullptr), "");
  ASSERT_DEATH(computeYOverlapRatios(&word1, nullptr), "");
  ASSERT_DEATH(computeYOverlapRatios(nullptr, &word2), "");

  // Input: two words that do not overlap vertically.
  pair<double, double> pair = computeYOverlapRatios(&word1, &word2);
  ASSERT_NEAR(pair.first, 0.0, TOL);
  ASSERT_NEAR(pair.second, 0.0, TOL);
  pair = computeYOverlapRatios(&word2, &word1);
  ASSERT_NEAR(pair.first, 0.0, TOL);
  ASSERT_NEAR(pair.second, 0.0, TOL);

  // Input: two words that partially overlap vertically.
  pair = computeYOverlapRatios(&word1, &word3);
  ASSERT_NEAR(pair.first, 0.3, TOL);
  ASSERT_NEAR(pair.second, 0.6, TOL);
  pair = computeYOverlapRatios(&word3, &word1);
  ASSERT_NEAR(pair.first, 0.6, TOL);
  ASSERT_NEAR(pair.second, 0.3, TOL);

  // Input: two words, with one word completely overlapping the other.
  pair = computeYOverlapRatios(&word1, &word4);
  ASSERT_NEAR(pair.first, 1.0, TOL);
  ASSERT_NEAR(pair.second, 0.4, TOL);
  pair = computeYOverlapRatios(&word4, &word1);
  ASSERT_NEAR(pair.first, 0.4, TOL);
  ASSERT_NEAR(pair.second, 1.0, TOL);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeMaxXOverlapRatio) {
  PdfWord word1(1, 12.0, 75.0, 18.0, 80.5, 0, 0);
  PdfWord word2(1, 20.0, 75.0, 28.0, 80.5, 0, 0);
  PdfWord word3(1, 15.0, 75.0, 27.0, 80.5, 0, 0);
  PdfWord word4(1, 10.0, 75.0, 20.0, 80.5, 0, 0);

  // Input: one or more nullptrs.
  ASSERT_DEATH(computeMaxXOverlapRatio(nullptr, nullptr), "");
  ASSERT_DEATH(computeMaxXOverlapRatio(&word1, nullptr), "");
  ASSERT_DEATH(computeMaxXOverlapRatio(nullptr, &word2), "");

  ASSERT_NEAR(computeMaxXOverlapRatio(&word1, &word2), 0.0, TOL);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word2, &word1), 0.0, TOL);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word1, &word3), 0.5, TOL);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word3, &word1), 0.5, TOL);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word1, &word4), 1.0, TOL);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word4, &word1), 1.0, TOL);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word2, &word3), 7/8., TOL);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word3, &word2), 7/8., TOL);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word2, &word4), 0.0, TOL);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word4, &word2), 0.0, TOL);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word3, &word4), 0.5, TOL);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word4, &word3), 0.5, TOL);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeMaxYOverlapRatio) {
  PdfWord word1(1, 7.5, 17.0, 12.0, 27.0, 0, 0);
  PdfWord word2(1, 7.5, 30.0, 12.0, 37.0, 0, 0);
  PdfWord word3(1, 7.5, 24.0, 12.0, 29.0, 0, 0);
  PdfWord word4(1, 7.5, 11.0, 12.0, 36.0, 0, 0);

  // Input: one or more nullptrs.
  ASSERT_DEATH(computeMaxYOverlapRatio(nullptr, nullptr), "");
  ASSERT_DEATH(computeMaxYOverlapRatio(&word1, nullptr), "");
  ASSERT_DEATH(computeMaxYOverlapRatio(nullptr, &word2), "");

  ASSERT_NEAR(computeMaxYOverlapRatio(&word1, &word2), 0.0, TOL);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word2, &word1), 0.0, TOL);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word1, &word3), 0.6, TOL);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word3, &word1), 0.6, TOL);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word1, &word4), 1.0, TOL);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word4, &word1), 1.0, TOL);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word2, &word3), 0.0, TOL);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word3, &word2), 0.0, TOL);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word2, &word4), 6/7., TOL);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word4, &word2), 6/7., TOL);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word3, &word4), 1.0, TOL);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word4, &word3), 1.0, TOL);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeHasEqualLeftX) {
  PdfWord word1(1, 7.5, 17.0, 12.0, 27.0, 0, 0);
  PdfWord word2(1, 7.5, 30.0, 12.0, 37.0, 0, 0);
  PdfWord word3(1, 7.6, 24.0, 12.0, 29.0, 0, 0);
  PdfWord word4(1, 8.2, 11.0, 12.0, 36.0, 0, 0);

  // Input: one or more nullptrs.
  ASSERT_DEATH(computeHasEqualLeftX(nullptr, nullptr, TOL), "");
  ASSERT_DEATH(computeHasEqualLeftX(&word1, nullptr, TOL), "");
  ASSERT_DEATH(computeHasEqualLeftX(nullptr, &word2, TOL), "");

  ASSERT_TRUE(computeHasEqualLeftX(&word1, &word1, TOL));
  ASSERT_TRUE(computeHasEqualLeftX(&word1, &word2, TOL));
  ASSERT_TRUE(computeHasEqualLeftX(&word2, &word1, TOL));

  ASSERT_FALSE(computeHasEqualLeftX(&word1, &word3, TOL));
  ASSERT_FALSE(computeHasEqualLeftX(&word3, &word1, TOL));
  ASSERT_TRUE(computeHasEqualLeftX(&word1, &word3, 0.2));
  ASSERT_TRUE(computeHasEqualLeftX(&word3, &word1, 0.2));

  ASSERT_FALSE(computeHasEqualLeftX(&word1, &word4, TOL));
  ASSERT_FALSE(computeHasEqualLeftX(&word4, &word1, TOL));
  ASSERT_FALSE(computeHasEqualLeftX(&word1, &word4, 0.2));
  ASSERT_FALSE(computeHasEqualLeftX(&word4, &word1, 0.2));
  ASSERT_TRUE(computeHasEqualLeftX(&word1, &word4, 1.0));
  ASSERT_TRUE(computeHasEqualLeftX(&word4, &word1, 1.0));
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeHasEqualUpperY) {
  PdfWord word1(1, 2.1, 17.0, 12.0, 27.0, 0, 0);
  PdfWord word2(1, 2.5, 17.0, 12.0, 37.0, 0, 0);
  PdfWord word3(1, 7.6, 17.1, 12.0, 29.0, 0, 0);
  PdfWord word4(1, 9.2, 18.0, 12.0, 36.0, 0, 0);

  // Input: one or more nullptrs.
  ASSERT_DEATH(computeHasEqualUpperY(nullptr, nullptr, TOL), "");
  ASSERT_DEATH(computeHasEqualUpperY(&word1, nullptr, TOL), "");
  ASSERT_DEATH(computeHasEqualUpperY(nullptr, &word2, TOL), "");

  ASSERT_TRUE(computeHasEqualUpperY(&word1, &word1, TOL));
  ASSERT_TRUE(computeHasEqualUpperY(&word1, &word2, TOL));
  ASSERT_TRUE(computeHasEqualUpperY(&word2, &word1, TOL));

  ASSERT_FALSE(computeHasEqualUpperY(&word1, &word3, TOL));
  ASSERT_FALSE(computeHasEqualUpperY(&word3, &word1, TOL));
  ASSERT_TRUE(computeHasEqualUpperY(&word1, &word3, 0.2));
  ASSERT_TRUE(computeHasEqualUpperY(&word3, &word1, 0.2));

  ASSERT_FALSE(computeHasEqualUpperY(&word1, &word4, TOL));
  ASSERT_FALSE(computeHasEqualUpperY(&word4, &word1, TOL));
  ASSERT_FALSE(computeHasEqualUpperY(&word1, &word4, 0.2));
  ASSERT_FALSE(computeHasEqualUpperY(&word4, &word1, 0.2));
  ASSERT_TRUE(computeHasEqualUpperY(&word1, &word4, 1.0));
  ASSERT_TRUE(computeHasEqualUpperY(&word4, &word1, 1.0));
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeHasEqualRightX) {
  PdfWord word1(1, 2.1, 10.1, 12.0, 27.0, 0, 0);
  PdfWord word2(1, 2.5, 12.2, 12.0, 37.0, 0, 0);
  PdfWord word3(1, 7.6, 17.1, 12.1, 29.0, 0, 0);
  PdfWord word4(1, 9.2, 18.6, 13.0, 36.0, 0, 0);

  // Input: one or more nullptrs.
  ASSERT_DEATH(computeHasEqualRightX(nullptr, nullptr, TOL), "");
  ASSERT_DEATH(computeHasEqualRightX(&word1, nullptr, TOL), "");
  ASSERT_DEATH(computeHasEqualRightX(nullptr, &word2, TOL), "");

  ASSERT_TRUE(computeHasEqualRightX(&word1, &word1, TOL));
  ASSERT_TRUE(computeHasEqualRightX(&word1, &word2, TOL));
  ASSERT_TRUE(computeHasEqualRightX(&word2, &word1, TOL));

  ASSERT_FALSE(computeHasEqualRightX(&word1, &word3, TOL));
  ASSERT_FALSE(computeHasEqualRightX(&word3, &word1, TOL));
  ASSERT_TRUE(computeHasEqualRightX(&word1, &word3, 0.2));
  ASSERT_TRUE(computeHasEqualRightX(&word3, &word1, 0.2));

  ASSERT_FALSE(computeHasEqualRightX(&word1, &word4, TOL));
  ASSERT_FALSE(computeHasEqualRightX(&word4, &word1, TOL));
  ASSERT_FALSE(computeHasEqualRightX(&word1, &word4, 0.2));
  ASSERT_FALSE(computeHasEqualRightX(&word4, &word1, 0.2));
  ASSERT_TRUE(computeHasEqualRightX(&word1, &word4, 1.0));
  ASSERT_TRUE(computeHasEqualRightX(&word4, &word1, 1.0));
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeHasEqualLowerY) {
  PdfWord word1(1, 2.1, 10.1, 7.8, 27.0, 0, 0);
  PdfWord word2(1, 2.5, 12.2, 8.1, 27.0, 0, 0);
  PdfWord word3(1, 7.6, 17.1, 9.9, 27.2, 0, 0);
  PdfWord word4(1, 9.2, 18.6, 9.9, 30.0, 0, 0);

  // Input: one or more nullptrs.
  ASSERT_DEATH(computeHasEqualLowerY(nullptr, nullptr, TOL), "");
  ASSERT_DEATH(computeHasEqualLowerY(&word1, nullptr, TOL), "");
  ASSERT_DEATH(computeHasEqualLowerY(nullptr, &word2, TOL), "");

  ASSERT_TRUE(computeHasEqualLowerY(&word1, &word1, TOL));
  ASSERT_TRUE(computeHasEqualLowerY(&word1, &word2, TOL));
  ASSERT_TRUE(computeHasEqualLowerY(&word2, &word1, TOL));

  ASSERT_FALSE(computeHasEqualLowerY(&word1, &word3, TOL));
  ASSERT_FALSE(computeHasEqualLowerY(&word3, &word1, TOL));
  ASSERT_TRUE(computeHasEqualLowerY(&word1, &word3, 0.5));
  ASSERT_TRUE(computeHasEqualLowerY(&word3, &word1, 0.5));

  ASSERT_FALSE(computeHasEqualLowerY(&word1, &word4, TOL));
  ASSERT_FALSE(computeHasEqualLowerY(&word4, &word1, TOL));
  ASSERT_FALSE(computeHasEqualLowerY(&word1, &word4, 0.5));
  ASSERT_FALSE(computeHasEqualLowerY(&word4, &word1, 0.5));
  ASSERT_TRUE(computeHasEqualLowerY(&word1, &word4, 5.0));
  ASSERT_TRUE(computeHasEqualLowerY(&word4, &word1, 5.0));
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeLeftXOffset) {
  PdfWord word1(1, 2.1, 10.1, 7.8, 27.0, 0, 0);
  PdfWord word2(1, 2.1, 12.2, 8.1, 27.0, 0, 0);
  PdfWord word3(1, 7.6, 17.1, 9.9, 27.2, 0, 0);
  PdfWord word4(1, 9.2, 18.6, 9.9, 30.0, 0, 0);

  // Input: one or more nullptrs.
  ASSERT_DEATH(computeLeftXOffset(nullptr, nullptr), "");
  ASSERT_DEATH(computeLeftXOffset(&word1, nullptr), "");
  ASSERT_DEATH(computeLeftXOffset(nullptr, &word2), "");

  ASSERT_NEAR(computeLeftXOffset(&word1, &word1), 0.0, TOL);
  ASSERT_NEAR(computeLeftXOffset(&word1, &word2), 0.0, TOL);
  ASSERT_NEAR(computeLeftXOffset(&word2, &word1), 0.0, TOL);
  ASSERT_NEAR(computeLeftXOffset(&word1, &word3), -5.5, TOL);
  ASSERT_NEAR(computeLeftXOffset(&word3, &word1), 5.5, TOL);
  ASSERT_NEAR(computeLeftXOffset(&word1, &word4), -7.1, TOL);
  ASSERT_NEAR(computeLeftXOffset(&word4, &word1), 7.1, TOL);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeRightXOffset) {
  PdfWord word1(1, 2.1, 10.1, 7.8, 27.0, 0, 0);
  PdfWord word2(1, 2.1, 12.2, 8.1, 27.0, 0, 0);
  PdfWord word3(1, 7.6, 17.1, 9.9, 27.2, 0, 0);
  PdfWord word4(1, 1.2, 18.6, 5.5, 30.0, 0, 0);

  // Input: one or more nullptrs.
  ASSERT_DEATH(computeRightXOffset(nullptr, nullptr), "");
  ASSERT_DEATH(computeRightXOffset(&word1, nullptr), "");
  ASSERT_DEATH(computeRightXOffset(nullptr, &word2), "");

  ASSERT_NEAR(computeRightXOffset(&word1, &word1), 0.0, TOL);
  ASSERT_NEAR(computeRightXOffset(&word1, &word2), -0.3, TOL);
  ASSERT_NEAR(computeRightXOffset(&word2, &word1), 0.3, TOL);
  ASSERT_NEAR(computeRightXOffset(&word1, &word3), -2.1, TOL);
  ASSERT_NEAR(computeRightXOffset(&word3, &word1), 2.1, TOL);
  ASSERT_NEAR(computeRightXOffset(&word1, &word4), 2.3, TOL);
  ASSERT_NEAR(computeRightXOffset(&word4, &word1), -2.3, TOL);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeHasEqualFont) {
  PdfWord word1;
  PdfWord word2;
  PdfWord word3;
  word1.fontName = "Arial";
  word2.fontName = "Times New Roman";
  word3.fontName = "Times New Roman";

  // Input: one or more nullptrs.
  ASSERT_DEATH(computeHasEqualFont(nullptr, nullptr), "");
  ASSERT_DEATH(computeHasEqualFont(&word1, nullptr), "");
  ASSERT_DEATH(computeHasEqualFont(nullptr, &word2), "");

  ASSERT_TRUE(computeHasEqualFont(&word1, &word1));
  ASSERT_FALSE(computeHasEqualFont(&word1, &word2));
  ASSERT_FALSE(computeHasEqualFont(&word2, &word1));
  ASSERT_TRUE(computeHasEqualFont(&word2, &word3));
  ASSERT_TRUE(computeHasEqualFont(&word3, &word2));
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeHasEqualFontSize) {
  PdfWord word1;
  PdfWord word2;
  PdfWord word3;
  word1.fontSize = 12.0;
  word2.fontSize = 12.0;
  word3.fontSize = 15.0;

  // Input: one or more nullptrs.
  ASSERT_DEATH(computeHasEqualFontSize(nullptr, nullptr, TOL), "");
  ASSERT_DEATH(computeHasEqualFontSize(&word1, nullptr, TOL), "");
  ASSERT_DEATH(computeHasEqualFontSize(nullptr, &word2, TOL), "");

  ASSERT_TRUE(computeHasEqualFontSize(&word1, &word1, TOL));
  ASSERT_TRUE(computeHasEqualFontSize(&word1, &word2, TOL));
  ASSERT_TRUE(computeHasEqualFontSize(&word2, &word1, TOL));
  ASSERT_FALSE(computeHasEqualFontSize(&word2, &word3, TOL));
  ASSERT_FALSE(computeHasEqualFontSize(&word3, &word2, TOL));
  ASSERT_TRUE(computeHasEqualFontSize(&word2, &word3, 5.0));
  ASSERT_TRUE(computeHasEqualFontSize(&word3, &word2, 5.0));
}
