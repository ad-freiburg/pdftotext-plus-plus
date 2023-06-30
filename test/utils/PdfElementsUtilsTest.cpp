/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "../../src/Config.h"
#include "../../src/PdfDocument.h"
#include "../../src/PdfToTextPlusPlus.h"
#include "../../src/utils/PdfElementsUtils.h"

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
using ppp::utils::elements::computeStartsWithUpper;
using ppp::utils::elements::computeVerticalGap;
using ppp::utils::elements::computeXOverlapRatios;
using ppp::utils::elements::computeYOverlapRatios;

// The allowed tolerance on comparing two float values.
static const double TOLERANCE = ppp::config::DEFAULT_DOUBLE_EQUAL_TOLERANCE;

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeHorizontalGap) {
  PdfWord word1(1, 20.0, 240.5, 25.5, 250.0, 0, 0);
  PdfWord word2(1, 27.0, 240.5, 32.2, 250.0, 0, 0);
  PdfWord word3(1, 35.4, 240.5, 40.1, 250.0, 0, 0);

  // Check if the method dies when a null pointer is passed.
  ASSERT_DEATH(computeHorizontalGap(nullptr, nullptr), "");
  ASSERT_DEATH(computeHorizontalGap(&word1, nullptr), "");
  ASSERT_DEATH(computeHorizontalGap(nullptr, &word2), "");

  // Check the standard behaviour.
  ASSERT_NEAR(computeHorizontalGap(&word1, &word2), 1.5, TOLERANCE);
  ASSERT_NEAR(computeHorizontalGap(&word2, &word1), 1.5, TOLERANCE);
  ASSERT_NEAR(computeHorizontalGap(&word1, &word3), 9.9, TOLERANCE);
  ASSERT_NEAR(computeHorizontalGap(&word3, &word1), 9.9, TOLERANCE);
  ASSERT_NEAR(computeHorizontalGap(&word2, &word3), 3.2, TOLERANCE);
  ASSERT_NEAR(computeHorizontalGap(&word3, &word2), 3.2, TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeVerticalGap) {
  PdfWord word1(1, 20.0, 240.5, 25.5, 245.1, 0, 0);
  PdfWord word2(1, 27.0, 247.5, 32.2, 250.5, 0, 0);
  PdfWord word3(1, 35.4, 253.5, 40.1, 257.8, 0, 0);

  // Check if the method dies when a null pointer is passed.
  ASSERT_DEATH(computeVerticalGap(nullptr, nullptr), "");
  ASSERT_DEATH(computeVerticalGap(&word1, nullptr), "");
  ASSERT_DEATH(computeVerticalGap(nullptr, &word2), "");

  // Check the standard behaviour.
  ASSERT_NEAR(computeVerticalGap(&word1, &word2), 2.4, TOLERANCE);
  ASSERT_NEAR(computeVerticalGap(&word2, &word1), 2.4, TOLERANCE);
  ASSERT_NEAR(computeVerticalGap(&word1, &word3), 8.4, TOLERANCE);
  ASSERT_NEAR(computeVerticalGap(&word3, &word1), 8.4, TOLERANCE);
  ASSERT_NEAR(computeVerticalGap(&word2, &word3), 3.0, TOLERANCE);
  ASSERT_NEAR(computeVerticalGap(&word3, &word2), 3.0, TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeOverlapRatios) {
  // Check two zero-length intervals.
  pair<double, double> pair = computeOverlapRatios(0.0, 0.0, 0.0, 0.0);
  ASSERT_NEAR(pair.first, 0.0, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.0, TOLERANCE);

  // Check two identical intervals.
  pair = computeOverlapRatios(5.0, 10.0, 5.0, 10.0);
  ASSERT_NEAR(pair.first, 1.0, TOLERANCE);
  ASSERT_NEAR(pair.second, 1.0, TOLERANCE);
  pair = computeOverlapRatios(10.0, 5.0, 10.0, 5.0);
  ASSERT_NEAR(pair.first, 1.0, TOLERANCE);
  ASSERT_NEAR(pair.second, 1.0, TOLERANCE);

  // Check two intervals that do not overlap.
  pair = computeOverlapRatios(12.0, 34.0, 37.0, 40.0);
  ASSERT_NEAR(pair.first, 0.0, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.0, TOLERANCE);
  pair = computeOverlapRatios(34.0, 12.0, 40.0, 37.0);
  ASSERT_NEAR(pair.first, 0.0, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.0, TOLERANCE);

  // Check two intervals that partially overlap.
  pair = computeOverlapRatios(1.0, 7.0, 4.0, 16.0);
  ASSERT_NEAR(pair.first, 0.5, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.25, TOLERANCE);
  pair = computeOverlapRatios(7.0, 1.0, 16.0, 4.0);
  ASSERT_NEAR(pair.first, 0.5, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.25, TOLERANCE);
  pair = computeOverlapRatios(4.0, 16.0, 1.0, 7.0);
  ASSERT_NEAR(pair.first, 0.25, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.5, TOLERANCE);

  pair = computeOverlapRatios(5.0, 15.0, 10.0, 20.0);
  ASSERT_NEAR(pair.first, 0.5, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.5, TOLERANCE);
  pair = computeOverlapRatios(15.0, 5.0, 20.0, 10.0);
  ASSERT_NEAR(pair.first, 0.5, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.5, TOLERANCE);

  // Check two intervals where one interval completely falls into another.
  pair = computeOverlapRatios(10.0, 35.0, 0.0, 100.0);
  ASSERT_NEAR(pair.first, 1.0, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.25, TOLERANCE);
  pair = computeOverlapRatios(35.0, 10.0, 100.0, 0.0);
  ASSERT_NEAR(pair.first, 1.0, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.25, TOLERANCE);

  pair = computeOverlapRatios(0.0, 100.0, 10.0, 85.0);
  ASSERT_NEAR(pair.first, 0.75, TOLERANCE);
  ASSERT_NEAR(pair.second, 1.0, TOLERANCE);
  pair = computeOverlapRatios(100.0, 0.0, 85.0, 10.0);
  ASSERT_NEAR(pair.first, 0.75, TOLERANCE);
  ASSERT_NEAR(pair.second, 1.0, TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeXOverlapRatios) {
  PdfWord word1(1, 12.0, 75.0, 18.0, 80.5, 0, 0);
  PdfWord word2(1, 20.0, 75.0, 28.0, 80.5, 0, 0);
  PdfWord word3(1, 15.0, 75.0, 27.0, 80.5, 0, 0);
  PdfWord word4(1, 10.0, 75.0, 20.0, 80.5, 0, 0);

  // Check if the method dies when a null pointer is passed.
  ASSERT_DEATH(computeXOverlapRatios(nullptr, nullptr), "");
  ASSERT_DEATH(computeXOverlapRatios(&word1, nullptr), "");
  ASSERT_DEATH(computeXOverlapRatios(nullptr, &word2), "");

  // Check the standard behaviour.
  pair<double, double> pair = computeXOverlapRatios(&word1, &word2);
  ASSERT_NEAR(pair.first, 0.0, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.0, TOLERANCE);
  pair = computeXOverlapRatios(&word2, &word1);
  ASSERT_NEAR(pair.first, 0.0, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.0, TOLERANCE);

  pair = computeXOverlapRatios(&word1, &word3);
  ASSERT_NEAR(pair.first, 0.5, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.25, TOLERANCE);
  pair = computeXOverlapRatios(&word3, &word1);
  ASSERT_NEAR(pair.first, 0.25, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.5, TOLERANCE);

  pair = computeXOverlapRatios(&word1, &word4);
  ASSERT_NEAR(pair.first, 1.0, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.6, TOLERANCE);
  pair = computeXOverlapRatios(&word4, &word1);
  ASSERT_NEAR(pair.first, 0.6, TOLERANCE);
  ASSERT_NEAR(pair.second, 1.0, TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeYOverlapRatios) {
  PdfWord word1(1, 7.5, 17.0, 12.0, 27.0, 0, 0);
  PdfWord word2(1, 7.5, 30.0, 12.0, 37.0, 0, 0);
  PdfWord word3(1, 7.5, 24.0, 12.0, 29.0, 0, 0);
  PdfWord word4(1, 7.5, 11.0, 12.0, 36.0, 0, 0);

  // Check if the method dies when a null pointer is passed.
  ASSERT_DEATH(computeYOverlapRatios(nullptr, nullptr), "");
  ASSERT_DEATH(computeYOverlapRatios(&word1, nullptr), "");
  ASSERT_DEATH(computeYOverlapRatios(nullptr, &word2), "");

  // Check the standard behaviour.
  pair<double, double> pair = computeYOverlapRatios(&word1, &word2);
  ASSERT_NEAR(pair.first, 0.0, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.0, TOLERANCE);
  pair = computeYOverlapRatios(&word2, &word1);
  ASSERT_NEAR(pair.first, 0.0, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.0, TOLERANCE);

  pair = computeYOverlapRatios(&word1, &word3);
  ASSERT_NEAR(pair.first, 0.3, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.6, TOLERANCE);
  pair = computeYOverlapRatios(&word3, &word1);
  ASSERT_NEAR(pair.first, 0.6, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.3, TOLERANCE);

  pair = computeYOverlapRatios(&word1, &word4);
  ASSERT_NEAR(pair.first, 1.0, TOLERANCE);
  ASSERT_NEAR(pair.second, 0.4, TOLERANCE);
  pair = computeYOverlapRatios(&word4, &word1);
  ASSERT_NEAR(pair.first, 0.4, TOLERANCE);
  ASSERT_NEAR(pair.second, 1.0, TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeMaxXOverlapRatio) {
  PdfWord word1(1, 12.0, 75.0, 18.0, 80.5, 0, 0);
  PdfWord word2(1, 20.0, 75.0, 28.0, 80.5, 0, 0);
  PdfWord word3(1, 15.0, 75.0, 27.0, 80.5, 0, 0);
  PdfWord word4(1, 10.0, 75.0, 20.0, 80.5, 0, 0);

  // Check if the method dies when a null pointer is passed.
  ASSERT_DEATH(computeMaxXOverlapRatio(nullptr, nullptr), "");
  ASSERT_DEATH(computeMaxXOverlapRatio(&word1, nullptr), "");
  ASSERT_DEATH(computeMaxXOverlapRatio(nullptr, &word2), "");

  // Check the standard behaviour.
  ASSERT_NEAR(computeMaxXOverlapRatio(&word1, &word2), 0.0, TOLERANCE);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word2, &word1), 0.0, TOLERANCE);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word1, &word3), 0.5, TOLERANCE);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word3, &word1), 0.5, TOLERANCE);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word1, &word4), 1.0, TOLERANCE);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word4, &word1), 1.0, TOLERANCE);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word2, &word3), 7/8., TOLERANCE);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word3, &word2), 7/8., TOLERANCE);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word2, &word4), 0.0, TOLERANCE);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word4, &word2), 0.0, TOLERANCE);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word3, &word4), 0.5, TOLERANCE);
  ASSERT_NEAR(computeMaxXOverlapRatio(&word4, &word3), 0.5, TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeMaxYOverlapRatio) {
  PdfWord word1(1, 7.5, 17.0, 12.0, 27.0, 0, 0);
  PdfWord word2(1, 7.5, 30.0, 12.0, 37.0, 0, 0);
  PdfWord word3(1, 7.5, 24.0, 12.0, 29.0, 0, 0);
  PdfWord word4(1, 7.5, 11.0, 12.0, 36.0, 0, 0);

  // Check if the method dies when a null pointer is passed.
  ASSERT_DEATH(computeMaxYOverlapRatio(nullptr, nullptr), "");
  ASSERT_DEATH(computeMaxYOverlapRatio(&word1, nullptr), "");
  ASSERT_DEATH(computeMaxYOverlapRatio(nullptr, &word2), "");

  // Check the standard behaviour.
  ASSERT_NEAR(computeMaxYOverlapRatio(&word1, &word2), 0.0, TOLERANCE);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word2, &word1), 0.0, TOLERANCE);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word1, &word3), 0.6, TOLERANCE);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word3, &word1), 0.6, TOLERANCE);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word1, &word4), 1.0, TOLERANCE);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word4, &word1), 1.0, TOLERANCE);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word2, &word3), 0.0, TOLERANCE);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word3, &word2), 0.0, TOLERANCE);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word2, &word4), 6/7., TOLERANCE);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word4, &word2), 6/7., TOLERANCE);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word3, &word4), 1.0, TOLERANCE);
  ASSERT_NEAR(computeMaxYOverlapRatio(&word4, &word3), 1.0, TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeHasEqualLeftX) {
  PdfWord word1(1, 7.5, 17.0, 12.0, 27.0, 0, 0);
  PdfWord word2(1, 7.5, 30.0, 12.0, 37.0, 0, 0);
  PdfWord word3(1, 7.6, 24.0, 12.0, 29.0, 0, 0);
  PdfWord word4(1, 8.2, 11.0, 12.0, 36.0, 0, 0);

  // Check if the method dies when a null pointer is passed.
  ASSERT_DEATH(computeHasEqualLeftX(nullptr, nullptr, TOLERANCE), "");
  ASSERT_DEATH(computeHasEqualLeftX(&word1, nullptr, TOLERANCE), "");
  ASSERT_DEATH(computeHasEqualLeftX(nullptr, &word2, TOLERANCE), "");

  // Check the standard behaviour.
  ASSERT_TRUE(computeHasEqualLeftX(&word1, &word1, TOLERANCE));
  ASSERT_TRUE(computeHasEqualLeftX(&word1, &word2, TOLERANCE));
  ASSERT_TRUE(computeHasEqualLeftX(&word2, &word1, TOLERANCE));

  ASSERT_FALSE(computeHasEqualLeftX(&word1, &word3, TOLERANCE));
  ASSERT_FALSE(computeHasEqualLeftX(&word3, &word1, TOLERANCE));
  ASSERT_TRUE(computeHasEqualLeftX(&word1, &word3, 0.2));
  ASSERT_TRUE(computeHasEqualLeftX(&word3, &word1, 0.2));

  ASSERT_FALSE(computeHasEqualLeftX(&word1, &word4, TOLERANCE));
  ASSERT_FALSE(computeHasEqualLeftX(&word4, &word1, TOLERANCE));
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

  // Check if the method dies when a null pointer is passed.
  ASSERT_DEATH(computeHasEqualUpperY(nullptr, nullptr, TOLERANCE), "");
  ASSERT_DEATH(computeHasEqualUpperY(&word1, nullptr, TOLERANCE), "");
  ASSERT_DEATH(computeHasEqualUpperY(nullptr, &word2, TOLERANCE), "");

  // Check the standard behaviour.
  ASSERT_TRUE(computeHasEqualUpperY(&word1, &word1, TOLERANCE));
  ASSERT_TRUE(computeHasEqualUpperY(&word1, &word2, TOLERANCE));
  ASSERT_TRUE(computeHasEqualUpperY(&word2, &word1, TOLERANCE));

  ASSERT_FALSE(computeHasEqualUpperY(&word1, &word3, TOLERANCE));
  ASSERT_FALSE(computeHasEqualUpperY(&word3, &word1, TOLERANCE));
  ASSERT_TRUE(computeHasEqualUpperY(&word1, &word3, 0.2));
  ASSERT_TRUE(computeHasEqualUpperY(&word3, &word1, 0.2));

  ASSERT_FALSE(computeHasEqualUpperY(&word1, &word4, TOLERANCE));
  ASSERT_FALSE(computeHasEqualUpperY(&word4, &word1, TOLERANCE));
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

  // Check if the method dies when a null pointer is passed.
  ASSERT_DEATH(computeHasEqualRightX(nullptr, nullptr, TOLERANCE), "");
  ASSERT_DEATH(computeHasEqualRightX(&word1, nullptr, TOLERANCE), "");
  ASSERT_DEATH(computeHasEqualRightX(nullptr, &word2, TOLERANCE), "");

  // Check the standard behaviour.
  ASSERT_TRUE(computeHasEqualRightX(&word1, &word1, TOLERANCE));
  ASSERT_TRUE(computeHasEqualRightX(&word1, &word2, TOLERANCE));
  ASSERT_TRUE(computeHasEqualRightX(&word2, &word1, TOLERANCE));

  ASSERT_FALSE(computeHasEqualRightX(&word1, &word3, TOLERANCE));
  ASSERT_FALSE(computeHasEqualRightX(&word3, &word1, TOLERANCE));
  ASSERT_TRUE(computeHasEqualRightX(&word1, &word3, 0.2));
  ASSERT_TRUE(computeHasEqualRightX(&word3, &word1, 0.2));

  ASSERT_FALSE(computeHasEqualRightX(&word1, &word4, TOLERANCE));
  ASSERT_FALSE(computeHasEqualRightX(&word4, &word1, TOLERANCE));
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

  // Check if the method dies when a null pointer is passed.
  ASSERT_DEATH(computeHasEqualLowerY(nullptr, nullptr, TOLERANCE), "");
  ASSERT_DEATH(computeHasEqualLowerY(&word1, nullptr, TOLERANCE), "");
  ASSERT_DEATH(computeHasEqualLowerY(nullptr, &word2, TOLERANCE), "");

  // Check the standard behaviour.
  ASSERT_TRUE(computeHasEqualLowerY(&word1, &word1, TOLERANCE));
  ASSERT_TRUE(computeHasEqualLowerY(&word1, &word2, TOLERANCE));
  ASSERT_TRUE(computeHasEqualLowerY(&word2, &word1, TOLERANCE));

  ASSERT_FALSE(computeHasEqualLowerY(&word1, &word3, TOLERANCE));
  ASSERT_FALSE(computeHasEqualLowerY(&word3, &word1, TOLERANCE));
  ASSERT_TRUE(computeHasEqualLowerY(&word1, &word3, 0.5));
  ASSERT_TRUE(computeHasEqualLowerY(&word3, &word1, 0.5));

  ASSERT_FALSE(computeHasEqualLowerY(&word1, &word4, TOLERANCE));
  ASSERT_FALSE(computeHasEqualLowerY(&word4, &word1, TOLERANCE));
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

  // Check if the method dies when a null pointer is passed.
  ASSERT_DEATH(computeLeftXOffset(nullptr, nullptr), "");
  ASSERT_DEATH(computeLeftXOffset(&word1, nullptr), "");
  ASSERT_DEATH(computeLeftXOffset(nullptr, &word2), "");

  ASSERT_NEAR(computeLeftXOffset(&word1, &word1), 0.0, TOLERANCE);
  ASSERT_NEAR(computeLeftXOffset(&word1, &word2), 0.0, TOLERANCE);
  ASSERT_NEAR(computeLeftXOffset(&word2, &word1), 0.0, TOLERANCE);
  ASSERT_NEAR(computeLeftXOffset(&word1, &word3), -5.5, TOLERANCE);
  ASSERT_NEAR(computeLeftXOffset(&word3, &word1), 5.5, TOLERANCE);
  ASSERT_NEAR(computeLeftXOffset(&word1, &word4), -7.1, TOLERANCE);
  ASSERT_NEAR(computeLeftXOffset(&word4, &word1), 7.1, TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeRightXOffset) {
  PdfWord word1(1, 2.1, 10.1, 7.8, 27.0, 0, 0);
  PdfWord word2(1, 2.1, 12.2, 8.1, 27.0, 0, 0);
  PdfWord word3(1, 7.6, 17.1, 9.9, 27.2, 0, 0);
  PdfWord word4(1, 1.2, 18.6, 5.5, 30.0, 0, 0);

  // Check if the method dies when a null pointer is passed.
  ASSERT_DEATH(computeRightXOffset(nullptr, nullptr), "");
  ASSERT_DEATH(computeRightXOffset(&word1, nullptr), "");
  ASSERT_DEATH(computeRightXOffset(nullptr, &word2), "");

  ASSERT_NEAR(computeRightXOffset(&word1, &word1), 0.0, TOLERANCE);
  ASSERT_NEAR(computeRightXOffset(&word1, &word2), -0.3, TOLERANCE);
  ASSERT_NEAR(computeRightXOffset(&word2, &word1), 0.3, TOLERANCE);
  ASSERT_NEAR(computeRightXOffset(&word1, &word3), -2.1, TOLERANCE);
  ASSERT_NEAR(computeRightXOffset(&word3, &word1), 2.1, TOLERANCE);
  ASSERT_NEAR(computeRightXOffset(&word1, &word4), 2.3, TOLERANCE);
  ASSERT_NEAR(computeRightXOffset(&word4, &word1), -2.3, TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeHasEqualFont) {
  PdfWord word1;
  PdfWord word2;
  PdfWord word3;

  word1.fontName = "Arial";
  word2.fontName = "Times New Roman";
  word3.fontName = "Times New Roman";

  // Check if the method dies when a null pointer is passed.
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

  // Check if the method dies when a null pointer is passed.
  ASSERT_DEATH(computeHasEqualFontSize(nullptr, nullptr, TOLERANCE), "");
  ASSERT_DEATH(computeHasEqualFontSize(&word1, nullptr, TOLERANCE), "");
  ASSERT_DEATH(computeHasEqualFontSize(nullptr, &word2, TOLERANCE), "");

  ASSERT_TRUE(computeHasEqualFontSize(&word1, &word1, TOLERANCE));
  ASSERT_TRUE(computeHasEqualFontSize(&word1, &word2, TOLERANCE));
  ASSERT_TRUE(computeHasEqualFontSize(&word2, &word1, TOLERANCE));
  ASSERT_FALSE(computeHasEqualFontSize(&word2, &word3, TOLERANCE));
  ASSERT_FALSE(computeHasEqualFontSize(&word3, &word2, TOLERANCE));
  ASSERT_TRUE(computeHasEqualFontSize(&word2, &word3, 5.0));
  ASSERT_TRUE(computeHasEqualFontSize(&word3, &word2, 5.0));
}

// _________________________________________________________________________________________________
TEST(PdfElementsUtilsTest, computeStartsWithUpper) {
  PdfWord word0;
  PdfWord word1;
  PdfWord word2;
  PdfWord word3;
  PdfWord word4;

  word1.text = "";
  word2.text = "big";
  word3.text = "Apple";
  word4.text = "123";

  // Check if the method dies when a null pointer is passed.
  ASSERT_DEATH(computeStartsWithUpper(nullptr), "");

  // Check the standard behaviour.
  ASSERT_FALSE(computeStartsWithUpper(&word0));
  ASSERT_FALSE(computeStartsWithUpper(&word1));
  ASSERT_FALSE(computeStartsWithUpper(&word2));
  ASSERT_TRUE(computeStartsWithUpper(&word3));
  ASSERT_FALSE(computeStartsWithUpper(&word4));
}

// // _________________________________________________________________________________________________
// TEST_F(PdfElementsUtilsTest, computeOverlapsFigurePdf1) {
//   // TODO(korzen): Read from config
//   double minXOverlapRatio = 0.5;
//   double minYOverlapRatio = 0.5;

//   PdfPage* page1 = pdf1->pages[1];
//   std::vector<PdfFigure*>& figures = page1->figures;

//   // Test the first line of the second page ("Lorem ipsum...").
//   PdfTextLine* line = page1->segments[0]->lines[0];
//   ASSERT_EQ(computeOverlapsFigure(line, minXOverlapRatio, minYOverlapRatio, figures), nullptr)
//       << "Line: " << line->toString();

//   // Test the second line of the second page ("vel ne dolore...").
//   line = page1->segments[0]->lines[1];
//   ASSERT_EQ(computeOverlapsFigure(line, minXOverlapRatio, minYOverlapRatio, figures), nullptr)
//       << "Line: " << line->toString();

//   // Test the first character ("f") in Figure 1 on the second page.
//   PdfCharacter* ch = figures[0]->characters[0];
//   ASSERT_EQ(computeOverlapsFigure(ch, minXOverlapRatio, minYOverlapRatio, figures), figures[0])
//       << "Character: " << ch->toString();

//   // Test the second character ("o") in Figure 1 on the second page.
//   ch = figures[0]->characters[1];
//   ASSERT_EQ(computeOverlapsFigure(ch, minXOverlapRatio, minYOverlapRatio, figures), figures[0])
//       << "Character: " << ch->toString();

//   // Test the third character ("o") in Figure 1 on the second page.
//   ch = figures[0]->characters[2];
//   ASSERT_EQ(computeOverlapsFigure(ch, minXOverlapRatio, minYOverlapRatio, figures), figures[0])
//       << "Character: " << ch->toString();
// }



// // _________________________________________________________________________________________________
// TEST_F(PdfElementsUtilsTest, computeHasEqualFontSizePdf1) {
//   // TODO(korzen): Read from config.
//   double fontSizeEqualTolerance = 1.0;

//   PdfPage* page0 = pdf1->pages[0];

//   // Test "Introduction" (in the first line) and "Lorem" (in the second line).
//   PdfWord* w1 = page0->words[1];
//   PdfWord* w2 = page0->words[2];
//   ASSERT_FALSE(computeHasEqualFontSize(w1, w2, fontSizeEqualTolerance))
//       << "W1: " << w1->toString() << "\nW2: " << w2->toString();

//   // Test "Lorem" and "ipsum" (in the second line).
//   w1 = page0->words[2];
//   w2 = page0->words[3];
//   ASSERT_TRUE(computeHasEqualFontSize(w1, w2, fontSizeEqualTolerance))
//       << "W1: " << w1->toString() << "\nW2: " << w2->toString();
// }

// // _________________________________________________________________________________________________
// TEST_F(PdfElementsUtilsTest, computeEndsWithSentenceDelimiterPdf1) {
//   // TODO(korzen): Read from config.
//   string delimAlphabet = "?!.);";

//   PdfPage* page0 = pdf1->pages[0];

//   // Test "Introduction" (in the first line).
//   PdfWord* w = page0->words[1];
//   ASSERT_FALSE(computeEndsWithSentenceDelimiter(w, delimAlphabet)) << "Word: " << w->toString();

//   // Test "Lorem" (in the second line).
//   w = page0->words[2];
//   ASSERT_FALSE(computeEndsWithSentenceDelimiter(w, delimAlphabet)) << "Word: " << w->toString();

//   // Test "laboramus." (in the third line).
//   w = page0->segments[0]->lines[2]->words[4];
//   ASSERT_TRUE(computeEndsWithSentenceDelimiter(w, delimAlphabet)) << "Word: " << w->toString();

//   // Test "eum." (in the fourth line).
//   w = page0->segments[0]->lines[3]->words[5];
//   ASSERT_TRUE(computeEndsWithSentenceDelimiter(w, delimAlphabet)) << "Word: " << w->toString();

//   // Test "laboramus?" (in the fifth line).
//   w = page0->segments[0]->lines[4]->words[5];
//   ASSERT_TRUE(computeEndsWithSentenceDelimiter(w, delimAlphabet)) << "Word: " << w->toString();
// }

// // _________________________________________________________________________________________________
// TEST_F(PdfElementsUtilsTest, computeStartsWithUpperPdf1) {
//   PdfPage* page0 = pdf1->pages[0];

//   // Test "Introduction" (in the first line).
//   PdfWord* w = page0->words[1];
//   ASSERT_TRUE(computeStartsWithUpper(w)) << "Word: " << w->toString();

//   // Test "Lorem" (in the second line).
//   w = page0->words[2];
//   ASSERT_TRUE(computeStartsWithUpper(w)) << "Word: " << w->toString();

//   // Test "ipsum" (in the second line).
//   w = page0->words[3];
//   ASSERT_FALSE(computeStartsWithUpper(w)) << "Word: " << w->toString();
// }

// // _________________________________________________________________________________________________
// TEST_F(PdfElementsUtilsTest, computeIsEmphasizedPdf1) {
//   // TODO(korzen): Read from config
//   double fontSizeEqualTolerance = 0.1;
//   double fontWeightEqualTolerance = 1.0;

//   PdfPage* page0 = pdf1->pages[0];
//   PdfPage* page1 = pdf1->pages[1];

//   // Test "1 Introduction" (the first line).
//   PdfTextLine* line = page0->segments[0]->lines[0];
//   ASSERT_TRUE(computeIsEmphasized(line, fontSizeEqualTolerance, fontWeightEqualTolerance))
//       << "Line: " << line->toString();

//   // Test "Lorem ipsum..." (the second line, not emphasized).
//   line = page0->segments[0]->lines[1];
//   ASSERT_FALSE(computeIsEmphasized(line, fontSizeEqualTolerance, fontWeightEqualTolerance))
//       << "Line: " << line->toString();

//   // Test "vel ne dolore..." (the second line of the second page, printed in bold).
//   line = page1->segments[0]->lines[1];
//   ASSERT_TRUE(computeIsEmphasized(line, fontSizeEqualTolerance, fontWeightEqualTolerance))
//       << "Line: " << line->toString();

//   // Test "EIRMOD" (the third word in the fourth line of the second page, printed in uppercase).
//   PdfWord* w = page1->segments[0]->lines[3]->words[2];
//   ASSERT_TRUE(computeIsEmphasized(w, fontSizeEqualTolerance, fontWeightEqualTolerance))
//       << "Word: " << w->toString();

//   // Test "uti deleniti..." (the fifth text line of the second page, printed in larger font size).
//   line = page1->segments[0]->lines[4];
//   ASSERT_TRUE(computeIsEmphasized(line, fontSizeEqualTolerance, fontWeightEqualTolerance))
//       << "Line: " << line->toString();
// }
