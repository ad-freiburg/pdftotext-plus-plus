/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <functional>

#include "../src/utils/FixedPriorityQueue.h"

// _________________________________________________________________________________________________
class IntAscComparator {
 public:
  bool operator() (int i1, int i2) const {
    return i1 < i2;
  }
};

// _________________________________________________________________________________________________
class IntDescComparator {
 public:
  bool operator() (int i1, int i2) const {
    return i1 > i2;
  }
};

// _________________________________________________________________________________________________
class StringDescComparator {
 public:
  bool operator() (std::string& s1, std::string& s2) const {
    return s1 < s2;
  }
};

// _________________________________________________________________________________________________
TEST(FixedPriorityQueue, pushpop) {
  FixedPriorityQueue<int, IntDescComparator> queue(3);
  ASSERT_EQ(queue._capacity, 3);
  ASSERT_EQ(queue.size(), 0);
  queue.push(2);
  queue.push(8);
  queue.push(3);
  ASSERT_EQ(queue.size(), 3);
  queue.push(5);
  queue.push(6);
  ASSERT_EQ(queue.size(), 3);
  ASSERT_EQ(queue.top(), 5);
  queue.pop();
  ASSERT_EQ(queue.top(), 6);
  queue.pop();
  ASSERT_EQ(queue.top(), 8);
  queue.pop();

  std::string abc = "abc";
  std::string bcd = "bcd";
  std::string ghi = "ghi";
  std::string klm = "klm";
  std::string tuv = "tuv";
  std::string vwx = "vwx";
  FixedPriorityQueue<std::string, StringDescComparator> queue2(4);
  ASSERT_EQ(queue2._capacity, 4);
  ASSERT_EQ(queue2.size(), 0);
  queue2.push(tuv);
  queue2.push(ghi);
  queue2.push(klm);
  ASSERT_EQ(queue2.size(), 3);
  queue2.push(abc);
  ASSERT_EQ(queue2.size(), 4);
  ASSERT_EQ(queue2.top(), tuv);
  queue2.pop();
  queue2.push(vwx);
  queue2.push(bcd);
  ASSERT_EQ(queue2.size(), 4);
  ASSERT_EQ(queue2.top(), klm);
  queue2.pop();
  ASSERT_EQ(queue2.top(), ghi);
  queue2.pop();
  ASSERT_EQ(queue2.top(), bcd);
  queue2.pop();
  ASSERT_EQ(queue2.top(), abc);
  queue2.pop();
}

// _________________________________________________________________________________________________
TEST(FixedPriorityQueue, sort) {
  FixedPriorityQueue<int, IntDescComparator> queue(4);

  std::vector<int> sorted;
  queue.sort(IntDescComparator(), &sorted);
  ASSERT_EQ(sorted.size(), 0);

  queue.push(2);
  queue.push(8);

  std::vector<int> sorted2;
  queue.sort(IntDescComparator(), &sorted2);
  ASSERT_EQ(sorted2.size(), 2);
  ASSERT_EQ(sorted2[0], 8);
  ASSERT_EQ(sorted2[1], 2);

  queue.push(7);
  queue.push(1);
  queue.push(4);
  queue.push(6);

  std::vector<int> sorted3;
  queue.sort(IntDescComparator(), &sorted3);
  ASSERT_EQ(sorted3.size(), 4);
  ASSERT_EQ(sorted3[0], 8);
  ASSERT_EQ(sorted3[1], 7);
  ASSERT_EQ(sorted3[2], 6);
  ASSERT_EQ(sorted3[3], 4);

  queue.pop();

  std::vector<int> sorted4;
  queue.sort(IntAscComparator(), &sorted4);
  ASSERT_EQ(sorted4.size(), 3);
  ASSERT_EQ(sorted4[0], 6);
  ASSERT_EQ(sorted4[1], 7);
  ASSERT_EQ(sorted4[2], 8);
}