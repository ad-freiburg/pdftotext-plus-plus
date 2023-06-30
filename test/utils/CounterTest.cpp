/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <string>
#include <utility>

#include "../../src/utils/Counter.h"
#include "../../src/Config.h"

using std::pair;
using std::string;

using ppp::utils::counter::DoubleCounter;
using ppp::utils::counter::StringCounter;

static const double TOLERANCE = ppp::config::DEFAULT_DOUBLE_EQUAL_TOLERANCE;

// _________________________________________________________________________________________________
TEST(DoubleCounter, mostFreq) {
  DoubleCounter counter;

  // Test the empty counter.
  ASSERT_THROW(counter.mostFreq(), std::runtime_error);

  counter[0.4] = 1;
  counter[1.7] = 7;
  counter[2.3] = 3;
  ASSERT_NEAR(counter.mostFreq(), 1.7, TOLERANCE);

  counter[2.3] += 3;
  ASSERT_NEAR(counter.mostFreq(), 1.7, TOLERANCE);

  counter[2.3] += 2;
  ASSERT_NEAR(counter.mostFreq(), 2.3, TOLERANCE);

  counter[6.1] = 5;
  ASSERT_NEAR(counter.mostFreq(), 2.3, TOLERANCE);

  counter[6.1] = 9;
  ASSERT_NEAR(counter.mostFreq(), 6.1, TOLERANCE);

  counter[6.1] -= 5;
  ASSERT_NEAR(counter.mostFreq(), 2.3, TOLERANCE);

  counter[1.7] = 0;
  counter[2.3] = 0;
  counter[6.1] = 0;
  ASSERT_NEAR(counter.mostFreq(), 0.4, TOLERANCE);

  counter[0.4] = 0;
  ASSERT_THROW(counter.mostFreq(), std::runtime_error);
}

// _________________________________________________________________________________________________
TEST(DoubleCounter, mostFreqCount) {
  DoubleCounter counter;

  // Test the empty counter.
  ASSERT_THROW(counter.mostFreqCount(), std::runtime_error);

  counter[0.7] = 5;
  counter[2.5] = 2;
  counter[3.1] = 1;
  ASSERT_EQ(counter.mostFreqCount(), 5);

  counter[2.5] += 2;
  ASSERT_EQ(counter.mostFreqCount(), 5);

  counter[2.5] += 4;
  ASSERT_EQ(counter.mostFreqCount(), 8);

  counter[6.3] = 9;
  ASSERT_EQ(counter.mostFreqCount(), 9);

  counter[6.3] -= 5;
  ASSERT_EQ(counter.mostFreqCount(), 8);

  counter[0.7] = 0;
  counter[2.5] = 0;
  counter[6.3] = 0;
  ASSERT_EQ(counter.mostFreqCount(), 1);

  counter[3.1] = 0;
  ASSERT_THROW(counter.mostFreqCount(), std::runtime_error);
}

// _________________________________________________________________________________________________
TEST(DoubleCounter, mostFreqAndCount) {
  DoubleCounter counter;

  // Test the empty counter.
  ASSERT_THROW(counter.mostFreqAndCount(), std::runtime_error);

  counter[4.5] = 3;
  counter[2.1] = 6;
  counter[4.7] = 7;
  pair<double, int> entry = counter.mostFreqAndCount();
  ASSERT_NEAR(entry.first, 4.7, TOLERANCE);
  ASSERT_EQ(entry.second, 7);

  counter[4.5] += 6;
  entry = counter.mostFreqAndCount();
  ASSERT_NEAR(entry.first, 4.5, TOLERANCE);
  ASSERT_EQ(entry.second, 9);

  counter[4.5] -= 1;
  entry = counter.mostFreqAndCount();
  ASSERT_NEAR(entry.first, 4.5, TOLERANCE);
  ASSERT_EQ(entry.second, 8);

  counter[4.5] -= 5;
  entry = counter.mostFreqAndCount();
  ASSERT_NEAR(entry.first, 4.7, TOLERANCE);
  ASSERT_EQ(entry.second, 7);

  counter[2.2] = 9;
  entry = counter.mostFreqAndCount();
  ASSERT_NEAR(entry.first, 2.2, TOLERANCE);
  ASSERT_EQ(entry.second, 9);

  counter[2.1] = 0;
  counter[4.7] = 0;
  counter[2.2] = 0;
  entry = counter.mostFreqAndCount();
  ASSERT_NEAR(entry.first, 4.5, TOLERANCE);
  ASSERT_EQ(entry.second, 3);

  counter[4.5] = 0;
  ASSERT_THROW(counter.mostFreqAndCount(), std::runtime_error);
}

// _________________________________________________________________________________________________
TEST(DoubleCounter, max) {
  DoubleCounter counter;

  // Test the empty counter.
  ASSERT_THROW(counter.max(), std::runtime_error);

  counter[4.5] = 3;
  counter[2.1] = 6;
  counter[4.7] = 2;
  ASSERT_NEAR(counter.max(), 4.7, TOLERANCE);

  counter[4.7] += 6;
  ASSERT_NEAR(counter.max(), 4.7, TOLERANCE);

  counter[4.7] = 1;
  ASSERT_NEAR(counter.max(), 4.7, TOLERANCE);

  counter[4.7]--;
  ASSERT_NEAR(counter.max(), 4.5, TOLERANCE);

  counter[5.2]++;
  ASSERT_NEAR(counter.max(), 5.2, TOLERANCE);

  counter[5.2] = 0;
  counter[2.1] = 0;
  ASSERT_NEAR(counter.max(), 4.5, TOLERANCE);

  counter[4.5] = 0;
  ASSERT_THROW(counter.max(), std::runtime_error);
}

// =================================================================================================

// _________________________________________________________________________________________________
TEST(StringCounter, mostFreq) {
  StringCounter counter;

  // Test the empty counter.
  ASSERT_THROW(counter.mostFreq(), std::runtime_error);

  counter["Iron Man"] = 2;
  counter["Pepper Potts"] = 7;
  counter["Captain Marvel"] = 3;
  ASSERT_EQ(counter.mostFreq(), "Pepper Potts");

  counter["Captain Marvel"] += 3;
  ASSERT_EQ(counter.mostFreq(), "Pepper Potts");

  counter["Captain Marvel"] += 2;
  ASSERT_EQ(counter.mostFreq(), "Captain Marvel");

  counter["Iron Man"] = 5;
  ASSERT_EQ(counter.mostFreq(), "Captain Marvel");

  counter["Iron Man"] += 4;
  ASSERT_EQ(counter.mostFreq(), "Iron Man");

  counter["Iron Man"] -= 5;
  ASSERT_EQ(counter.mostFreq(), "Captain Marvel");

  counter["Black Widow"] = 9;
  ASSERT_EQ(counter.mostFreq(), "Black Widow");

  counter["Black Widow"] = 0;
  counter["Pepper Potts"] = 0;
  counter["Captain Marvel"] = 0;
  ASSERT_EQ(counter.mostFreq(), "Iron Man");

  counter["Iron Man"] = 0;
  ASSERT_THROW(counter.mostFreq(), std::runtime_error);
}

// _________________________________________________________________________________________________
TEST(StringCounter, mostFreqCount) {
  StringCounter counter;

  // Test the empty counter.
  ASSERT_THROW(counter.mostFreqCount(), std::runtime_error);

  counter["Buche"] = 3;
  counter["Eiche"] = 5;
  counter["Linde"] = 2;
  ASSERT_EQ(counter.mostFreqCount(), 5);

  counter["Linde"] += 2;
  ASSERT_EQ(counter.mostFreqCount(), 5);

  counter["Linde"] += 2;
  ASSERT_EQ(counter.mostFreqCount(), 6);

  counter["Buche"] = 8;
  ASSERT_EQ(counter.mostFreqCount(), 8);

  counter["Buche"] -= 5;
  ASSERT_EQ(counter.mostFreqCount(), 6);

  counter["Ahorn"] = 12;
  ASSERT_EQ(counter.mostFreqCount(), 12);

  counter["Buche"] = 0;
  counter["Eiche"] = 0;
  counter["Linde"] = 0;
  counter["Ahorn"] = 0;
  ASSERT_THROW(counter.mostFreqCount(), std::runtime_error);
}

// _________________________________________________________________________________________________
TEST(StringCounter, mostFreqAndCount) {
  StringCounter counter;

  // Test the empty counter.
  ASSERT_THROW(counter.mostFreqAndCount(), std::runtime_error);

  counter["USA"] = 3;
  counter["Germany"] = 6;
  counter["Sweden"] = 1;
  counter["France"] = 8;
  pair<string, int> entry = counter.mostFreqAndCount();
  ASSERT_EQ(entry.first, "France");
  ASSERT_EQ(entry.second, 8);

  counter["USA"] += 6;
  entry = counter.mostFreqAndCount();
  ASSERT_EQ(entry.first, "USA");
  ASSERT_EQ(entry.second, 9);

  counter["Germany"] += 5;
  entry = counter.mostFreqAndCount();
  ASSERT_EQ(entry.first, "Germany");
  ASSERT_EQ(entry.second, 11);

  counter["Germany"] -= 9;
  entry = counter.mostFreqAndCount();
  ASSERT_EQ(entry.first, "USA");
  ASSERT_EQ(entry.second, 9);

  counter["USA"] = 0;
  entry = counter.mostFreqAndCount();
  ASSERT_EQ(entry.first, "France");
  ASSERT_EQ(entry.second, 8);

  counter["France"] = 0;
  counter["Sweden"] = 0;
  counter["Germany"] = 0;
  ASSERT_THROW(counter.mostFreqAndCount(), std::runtime_error);
}
