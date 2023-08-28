/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include "../../src/Config.h"
#include "../../src/utils/MathUtils.h"

using ppp::utils::math::between;
using ppp::utils::math::equal;
using ppp::utils::math::equalOrLarger;
using ppp::utils::math::equalOrSmaller;
using ppp::utils::math::larger;
using ppp::utils::math::maximum;
using ppp::utils::math::minimum;
using ppp::utils::math::round;
using ppp::utils::math::smaller;

// =================================================================================================

// The tolerance to use on comparing two float values.
static const double TOL = ppp::config::DEFAULT_DOUBLE_EQUAL_TOLERANCE;

// _________________________________________________________________________________________________
TEST(MathUtilsTest, equal) {
  ASSERT_TRUE(equal(0.0, 0.0));
  ASSERT_TRUE(equal(2.3, 2.3));
  ASSERT_TRUE(equal(1.1, 1.5, 0.4));
  ASSERT_TRUE(equal(5.3, 5.7, 2.2));

  ASSERT_FALSE(equal(0.0, 0.1));
  ASSERT_FALSE(equal(2.3, 5.3));
  ASSERT_FALSE(equal(1.1, 1.5, 0.39));
  ASSERT_FALSE(equal(5.3, 7.7, 1.2));
}

// _________________________________________________________________________________________________
TEST(MathUtilsTest, larger) {
  ASSERT_TRUE(larger(0.1, 0.0));
  ASSERT_TRUE(larger(3.3, 2.3));
  ASSERT_TRUE(larger(1.8, 1.5, 0.2));
  ASSERT_TRUE(larger(5.3, 1.7, 2.2));

  ASSERT_FALSE(larger(0.0, 0.0));
  ASSERT_FALSE(larger(0.3, 2.8));
  ASSERT_FALSE(larger(1.8, 1.5, 0.3));
  ASSERT_FALSE(larger(5.3, 1.7, 5.0));
}

// _________________________________________________________________________________________________
TEST(MathUtilsTest, smaller) {
  ASSERT_TRUE(smaller(0.0, 0.01));
  ASSERT_TRUE(smaller(3.3, 4.2));
  ASSERT_TRUE(smaller(1.3, 1.8, 0.2));
  ASSERT_TRUE(smaller(5.4, 10.7, 2.2));

  ASSERT_FALSE(smaller(0.01, 0.0));
  ASSERT_FALSE(smaller(4.2, 3.3));
  ASSERT_FALSE(smaller(1.3, 1.8, 0.5));
  ASSERT_FALSE(smaller(9.1, 10.7, 2.2));
}

// _________________________________________________________________________________________________
TEST(MathUtilsTest, equalOrLarger) {
  ASSERT_TRUE(equalOrLarger(1.0, 1.0));
  ASSERT_TRUE(equalOrLarger(3.3, 0.2));
  ASSERT_TRUE(equalOrLarger(1.3, 1.3, 0.2));
  ASSERT_TRUE(equalOrLarger(11.9, 9.0, 2.2));

  ASSERT_FALSE(equalOrLarger(0.9, 1.0));
  ASSERT_FALSE(equalOrLarger(0.0, 0.2));
  ASSERT_FALSE(equalOrLarger(0.9, 1.2, 0.2));
  ASSERT_FALSE(equalOrLarger(2.1, 11.0, 2.2));
}

// _________________________________________________________________________________________________
TEST(MathUtilsTest, equalOrSmaller) {
  ASSERT_TRUE(equalOrSmaller(1.0, 1.0));
  ASSERT_TRUE(equalOrSmaller(0.3, 2.2));
  ASSERT_TRUE(equalOrSmaller(1.3, 1.3, 0.2));
  ASSERT_TRUE(equalOrSmaller(1.2, 1.3, 0.2));

  ASSERT_FALSE(equalOrSmaller(1.1, 1.0));
  ASSERT_FALSE(equalOrSmaller(5.1, 0.2));
  ASSERT_FALSE(equalOrSmaller(1.41, 1.2, 0.2));
  ASSERT_FALSE(equalOrSmaller(11.0, 2.1, 2.2));
}

// _________________________________________________________________________________________________
TEST(MathUtilsTest, between) {
  ASSERT_TRUE(between(0.0, 0.0, 1.0));
  ASSERT_TRUE(between(0.5, 0.0, 1.0));
  ASSERT_TRUE(between(1.7, 1.5, 1.6, 0.1));
  ASSERT_TRUE(between(1.4, 1.5, 1.6, 0.1));

  ASSERT_FALSE(between(1.1, 0.0, 1.0));
  ASSERT_FALSE(between(0.2, 0.5, 1.0));
  ASSERT_FALSE(between(2.0, 1.5, 1.6, 0.2));
  ASSERT_FALSE(between(0.0, 2.0, 3.0, 1.0));
}

// _________________________________________________________________________________________________
TEST(MathUtilsTest, round) {
  ASSERT_NEAR(round(1.46731, 0), 1.0, TOL);
  ASSERT_NEAR(round(1.56731, 0), 2.0, TOL);
  ASSERT_NEAR(round(1.56731, 1), 1.6, TOL);
  ASSERT_NEAR(round(1.56731, 2), 1.57, TOL);
  ASSERT_NEAR(round(1.56731, 3), 1.567, TOL);
  ASSERT_NEAR(round(1.56731, 4), 1.5673, TOL);
  ASSERT_NEAR(round(1.56731, 5), 1.56731, TOL);
}

// _________________________________________________________________________________________________
TEST(MathUtilsTest, minimum) {
  ASSERT_NEAR(minimum(0.0, 0.0), 0.0, TOL);
  ASSERT_NEAR(minimum(-6.3, -5.6), -6.3, TOL);
  ASSERT_NEAR(minimum(-5.6, -6.3), -6.3, TOL);
  ASSERT_NEAR(minimum(-7.2, 2.3), -7.2, TOL);
  ASSERT_NEAR(minimum(2.3, -7.2), -7.2, TOL);
  ASSERT_NEAR(minimum(2.3, 5.6), 2.3, TOL);
  ASSERT_NEAR(minimum(5.6, 2.3), 2.3, TOL);
  ASSERT_NEAR(minimum(1, 2, 3, 4), 1, TOL);
  ASSERT_NEAR(minimum(3, 2, 4), 2, TOL);
  ASSERT_NEAR(minimum(0.6, 0.4, 0.3, 1.2), 0.3, TOL);
}

// _________________________________________________________________________________________________
TEST(MathUtilsTest, maximum) {
  ASSERT_NEAR(maximum(0.0, 0.0), 0.0, TOL);
  ASSERT_NEAR(maximum(-6.3, -5.6), -5.6, TOL);
  ASSERT_NEAR(maximum(-5.6, -6.3), -5.6, TOL);
  ASSERT_NEAR(maximum(-7.2, 2.3), 2.3, TOL);
  ASSERT_NEAR(maximum(2.3, -7.2), 2.3, TOL);
  ASSERT_NEAR(maximum(2.3, 5.6), 5.6, TOL);
  ASSERT_NEAR(maximum(5.6, 2.3), 5.6, TOL);
  ASSERT_NEAR(maximum(1, 2, 3, 4), 4, TOL);
  ASSERT_NEAR(maximum(3, 2, 1), 3, TOL);
  ASSERT_NEAR(maximum(0.6, 0.4, 0.3, 1.2), 1.2, TOL);
}
