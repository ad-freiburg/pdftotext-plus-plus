/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <string>
#include <utility>  // pair

#include "../src/utils/Counter.h"

#include "../src/Constants.h"

using global_config::DOUBLE_EQUAL_TOLERANCE;

// _________________________________________________________________________________________________
TEST(DoubleCounter, mostFreq) {
  DoubleCounter counter;

  counter[1.2]++;
  ASSERT_NEAR(1.2, counter.mostFreq(), DOUBLE_EQUAL_TOLERANCE);

  counter[1.1]++;
  counter[1.1]++;
  ASSERT_NEAR(1.1, counter.mostFreq(), DOUBLE_EQUAL_TOLERANCE);

  counter[1.2]++;
  counter[1.2]++;
  ASSERT_NEAR(1.2, counter.mostFreq(), DOUBLE_EQUAL_TOLERANCE);

  counter[1.2]++;
  ASSERT_NEAR(1.2, counter.mostFreq(), DOUBLE_EQUAL_TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(DoubleCounter, mostFreqCount) {
  DoubleCounter counter;

  counter[1.2]++;
  ASSERT_EQ(1, counter.mostFreqCount());

  counter[1.1]++;
  counter[1.1]++;
  ASSERT_EQ(2, counter.mostFreqCount());

  counter[1.2]++;
  counter[1.2]++;
  ASSERT_EQ(3, counter.mostFreqCount());

  counter[1.2]++;
  ASSERT_EQ(4, counter.mostFreqCount());
}

// _________________________________________________________________________________________________
TEST(DoubleCounter, mostFreqCountAndCount) {
  DoubleCounter counter;

  counter[1.2]++;
  std::pair<double, int> p = counter.mostFreqAndCount();
  ASSERT_NEAR(1.2, p.first, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_EQ(1, p.second);

  counter[1.1]++;
  counter[1.1]++;
  p = counter.mostFreqAndCount();
  ASSERT_NEAR(1.1, p.first, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_EQ(2, p.second);

  counter[1.2]++;
  counter[1.2]++;
  p = counter.mostFreqAndCount();
  ASSERT_NEAR(1.2, p.first, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_EQ(3, p.second);

  counter[1.2]++;
  p = counter.mostFreqAndCount();
  ASSERT_NEAR(1.2, p.first, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_EQ(4, p.second);
}

// _________________________________________________________________________________________________
TEST(DoubleCounter, max) {
  DoubleCounter counter;

  counter[1.2]++;
  ASSERT_NEAR(1.2, counter.max(), DOUBLE_EQUAL_TOLERANCE);

  counter[1.1]++;
  counter[1.1]++;
  ASSERT_NEAR(1.2, counter.max(), DOUBLE_EQUAL_TOLERANCE);

  counter[1.2]++;
  counter[1.2]++;
  ASSERT_NEAR(1.2, counter.max(), DOUBLE_EQUAL_TOLERANCE);

  counter[3.2]++;
  ASSERT_NEAR(3.2, counter.max(), DOUBLE_EQUAL_TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(StringCounter, mostFreq) {
  StringCounter counter;

  counter["foo"]++;
  ASSERT_EQ("foo", counter.mostFreq());

  counter["bar"]++;
  counter["bar"]++;
  ASSERT_EQ("bar", counter.mostFreq());

  counter["foo"]++;
  counter["foo"]++;
  ASSERT_EQ("foo", counter.mostFreq());

  counter["foo"]++;
  ASSERT_EQ("foo", counter.mostFreq());
}

// _________________________________________________________________________________________________
TEST(StringCounter, mostFreqCount) {
  StringCounter counter;

  counter["foo"]++;
  ASSERT_EQ(1, counter.mostFreqCount());

  counter["bar"]++;
  counter["bar"]++;
  ASSERT_EQ(2, counter.mostFreqCount());

  counter["foo"]++;
  counter["foo"]++;
  ASSERT_EQ(3, counter.mostFreqCount());

  counter["foo"]++;
  ASSERT_EQ(4, counter.mostFreqCount());
}

// _________________________________________________________________________________________________
TEST(StringCounter, mostFreqCountAndCount) {
  StringCounter counter;

  counter["foo"]++;
  std::pair<std::string, int> p = counter.mostFreqAndCount();
  ASSERT_EQ("foo", p.first);
  ASSERT_EQ(1, p.second);

  counter["bar"]++;
  counter["bar"]++;
  p = counter.mostFreqAndCount();
  ASSERT_EQ("bar", p.first);
  ASSERT_EQ(2, p.second);

  counter["foo"]++;
  counter["foo"]++;
  p = counter.mostFreqAndCount();
  ASSERT_EQ("foo", p.first);
  ASSERT_EQ(3, p.second);

  counter["foo"]++;
  p = counter.mostFreqAndCount();
  ASSERT_EQ("foo", p.first);
  ASSERT_EQ(4, p.second);
}
