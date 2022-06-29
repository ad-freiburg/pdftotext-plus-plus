/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include "../src/utils/MathUtils.h"

// _________________________________________________________________________________________________
TEST(MathUtils, equal) {
  ASSERT_TRUE(math_utils::equal(0.0, 0.0));
  ASSERT_TRUE(math_utils::equal(2.3, 2.3));
  ASSERT_TRUE(math_utils::equal(1.1, 1.5, 0.4));
  ASSERT_TRUE(math_utils::equal(5.3, 5.7, 2.2));

  ASSERT_FALSE(math_utils::equal(0.0, 0.1));
  ASSERT_FALSE(math_utils::equal(2.3, 5.3));
  ASSERT_FALSE(math_utils::equal(1.1, 1.5, 0.39));
  ASSERT_FALSE(math_utils::equal(5.3, 7.7, 1.2));
}

// _________________________________________________________________________________________________
TEST(MathUtils, larger) {
  ASSERT_TRUE(math_utils::larger(0.1, 0.0));
  ASSERT_TRUE(math_utils::larger(3.3, 2.3));
  ASSERT_TRUE(math_utils::larger(1.8, 1.5, 0.2));
  ASSERT_TRUE(math_utils::larger(5.3, 1.7, 2.2));

  ASSERT_FALSE(math_utils::larger(0.0, 0.0));
  ASSERT_FALSE(math_utils::larger(0.3, 2.8));
  ASSERT_FALSE(math_utils::larger(1.8, 1.5, 0.3));
  ASSERT_FALSE(math_utils::larger(5.3, 1.7, 5.0));
}

// _________________________________________________________________________________________________
TEST(MathUtils, smaller) {
  ASSERT_TRUE(math_utils::smaller(0.0, 0.01));
  ASSERT_TRUE(math_utils::smaller(3.3, 4.2));
  ASSERT_TRUE(math_utils::smaller(1.3, 1.8, 0.2));
  ASSERT_TRUE(math_utils::smaller(5.4, 10.7, 2.2));

  ASSERT_FALSE(math_utils::smaller(0.01, 0.0));
  ASSERT_FALSE(math_utils::smaller(4.2, 3.3));
  ASSERT_FALSE(math_utils::smaller(1.3, 1.8, 0.5));
  ASSERT_FALSE(math_utils::smaller(9.1, 10.7, 2.2));
}

// _________________________________________________________________________________________________
TEST(MathUtils, equalOrLarger) {
  ASSERT_TRUE(math_utils::equalOrLarger(1.0, 1.0));
  ASSERT_TRUE(math_utils::equalOrLarger(3.3, 0.2));
  ASSERT_TRUE(math_utils::equalOrLarger(1.3, 1.3, 0.2));
  ASSERT_TRUE(math_utils::equalOrLarger(11.9, 9.0, 2.2));

  ASSERT_FALSE(math_utils::equalOrLarger(0.9, 1.0));
  ASSERT_FALSE(math_utils::equalOrLarger(0.0, 0.2));
  ASSERT_FALSE(math_utils::equalOrLarger(0.9, 1.2, 0.2));
  ASSERT_FALSE(math_utils::equalOrLarger(2.1, 11.0, 2.2));
}

// _________________________________________________________________________________________________
TEST(MathUtils, equalOrSmaller) {
  ASSERT_TRUE(math_utils::equalOrSmaller(1.0, 1.0));
  ASSERT_TRUE(math_utils::equalOrSmaller(0.3, 2.2));
  ASSERT_TRUE(math_utils::equalOrSmaller(1.3, 1.3, 0.2));
  ASSERT_TRUE(math_utils::equalOrSmaller(1.2, 1.3, 0.2));

  ASSERT_FALSE(math_utils::equalOrSmaller(1.1, 1.0));
  ASSERT_FALSE(math_utils::equalOrSmaller(5.1, 0.2));
  ASSERT_FALSE(math_utils::equalOrSmaller(1.41, 1.2, 0.2));
  ASSERT_FALSE(math_utils::equalOrSmaller(11.0, 2.1, 2.2));
}

// _________________________________________________________________________________________________
TEST(MathUtils, between) {
  ASSERT_TRUE(math_utils::between(0.0, 0.0, 1.0));
  ASSERT_TRUE(math_utils::between(0.5, 0.0, 1.0));
  ASSERT_TRUE(math_utils::between(1.7, 1.5, 1.6, 0.1));
  ASSERT_TRUE(math_utils::between(1.4, 1.5, 1.6, 0.1));

  ASSERT_FALSE(math_utils::between(1.1, 0.0, 1.0));
  ASSERT_FALSE(math_utils::between(0.2, 0.5, 1.0));
  ASSERT_FALSE(math_utils::between(2.0, 1.5, 1.6, 0.2));
  ASSERT_FALSE(math_utils::between(0.0, 2.0, 3.0, 1.0));
}

// _________________________________________________________________________________________________
TEST(MathUtils, round) {
  ASSERT_NEAR(math_utils::round(1.46731, 0), 1.0, 0.00001);
  ASSERT_NEAR(math_utils::round(1.56731, 0), 2.0, 0.00001);
  ASSERT_NEAR(math_utils::round(1.56731, 1), 1.6, 0.00001);
  ASSERT_NEAR(math_utils::round(1.56731, 2), 1.57, 0.00001);
  ASSERT_NEAR(math_utils::round(1.56731, 3), 1.567, 0.00001);
  ASSERT_NEAR(math_utils::round(1.56731, 4), 1.5673, 0.00001);
  ASSERT_NEAR(math_utils::round(1.56731, 5), 1.56731, 0.00001);
}