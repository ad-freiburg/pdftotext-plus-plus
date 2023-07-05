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

#include "../../src/utils/FixedCapacityPriorityQueue.h"

using std::string;
using std::vector;

// =================================================================================================
// Define some comparators needed to test FixedCapacityPriorityQueue.

// A comparator for sorting int values in ascending order.
class IntAscComparator {
 public:
  // A method that returns true if the two given elements need to be swapped.
  bool operator() (int i1, int i2) const {
    return i1 <= i2;
  }
};

// A comparator for sorting int values in descending order.
class IntDescComparator {
 public:
  // A method that returns true if the two given elements need to be swapped.
  bool operator() (int i1, int i2) const {
    return i1 >= i2;
  }
};

// A comparator for sorting strings in ascending order.
class StringAscComparator {
 public:
  // A method that returns true if the two given elements need to be swapped.
  bool operator() (const string& s1, const string& s2) const {
    return s1 <= s2;
  }
};

// _________________________________________________________________________________________________
TEST(FixedCapacityPriorityQueueTest, constructor) {
  FixedCapacityPriorityQueue<string, StringAscComparator> queue1(1);
  ASSERT_EQ(queue1._capacity, static_cast<unsigned int>(1));
  ASSERT_EQ(queue1.size(), static_cast<unsigned int>(0));

  FixedCapacityPriorityQueue<string, StringAscComparator> queue2(6);
  ASSERT_EQ(queue2._capacity, static_cast<unsigned int>(6));
  ASSERT_EQ(queue2.size(), static_cast<unsigned int>(0));
}

// _________________________________________________________________________________________________
TEST(FixedCapacityPriorityQueueTest, pushPopInt) {
  // Create a priority queue with capacity 3.
  FixedCapacityPriorityQueue<int, IntDescComparator> queue(3);

  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(3));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(0));

  queue.push(2);
  queue.push(8);
  queue.push(3);
  // PQ: 2 3 8
  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(3));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(3));
  ASSERT_EQ(queue.top(), 2);

  queue.push(5);
  queue.push(6);
  // PQ: 5 6 8
  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(3));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(3));
  ASSERT_EQ(queue.top(), 5);

  queue.push(1);
  // PQ: 1 6 8
  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(3));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(3));
  ASSERT_EQ(queue.top(), 1);

  queue.push(9);
  // PQ: 6 8 9
  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(3));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(3));
  ASSERT_EQ(queue.top(), 6);

  queue.pop();
  // PQ: 8 9
  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(3));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(2));
  ASSERT_EQ(queue.top(), 8);

  queue.pop();
  // PQ: 9
  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(3));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(1));
  ASSERT_EQ(queue.top(), 9);

  queue.push(12);
  // PQ: 9 12
  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(3));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(2));
  ASSERT_EQ(queue.top(), 9);

  queue.pop();
  // PQ: 12
  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(3));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(1));
  ASSERT_EQ(queue.top(), 12);

  queue.pop();
  // PQ: <empty>
  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(3));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(0));
}

// _________________________________________________________________________________________________
TEST(FixedCapacityPriorityQueueTest, pushPopString) {
  string abc = "abc";
  string bcd = "bcd";
  string ghi = "ghi";
  string klm = "klm";
  string tuv = "tuv";
  string vwx = "vwx";

  // Create a priority queue with capacity 4.
  FixedCapacityPriorityQueue<string, StringAscComparator> queue(4);

  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(4));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(0));

  queue.push(tuv);
  queue.push(ghi);
  queue.push(klm);
  // PQ: tuv klm ghi
  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(4));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(3));
  ASSERT_EQ(queue.top(), tuv);

  queue.push(abc);
  // PQ: tuv klm ghi abc
  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(4));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(4));
  ASSERT_EQ(queue.top(), tuv);

  queue.push(abc);
  // PQ: klm ghi abc abc
  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(4));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(4));
  ASSERT_EQ(queue.top(), klm);

  queue.pop();
  queue.push(vwx);
  queue.push(bcd);
  // PQ: ghi bcd abc abc
  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(4));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(4));
  ASSERT_EQ(queue.top(), ghi);

  queue.pop();
  // PQ: bcd abc abc
  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(4));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(3));
  ASSERT_EQ(queue.top(), bcd);

  queue.pop();
  // PQ: abc abc
  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(4));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(2));
  ASSERT_EQ(queue.top(), abc);

  queue.pop();
  // PQ: abc
  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(4));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(1));
  ASSERT_EQ(queue.top(), abc);

  queue.pop();
  // PQ: <empty>
  ASSERT_EQ(queue._capacity, static_cast<unsigned int>(4));
  ASSERT_EQ(queue.size(), static_cast<unsigned int>(0));
}

// _________________________________________________________________________________________________
TEST(FixedCapacityPriorityQueueTest, sort) {
  // Create a priority queue with capacity 4.
  FixedCapacityPriorityQueue<int, IntDescComparator> queue(4);

  // Test the empty queue.
  vector<int> sorted;
  queue.sort(IntAscComparator(), &sorted);
  ASSERT_EQ(sorted.size(), static_cast<unsigned int>(0));

  queue.push(2);
  queue.push(8);
  queue.sort(IntAscComparator(), &sorted);
  ASSERT_EQ(sorted.size(), static_cast<unsigned int>(2));
  ASSERT_EQ(sorted[0], 2);
  ASSERT_EQ(sorted[1], 8);

  sorted.clear();
  queue.push(7);
  queue.push(1);
  queue.push(4);
  queue.push(6);
  queue.sort(IntAscComparator(), &sorted);
  ASSERT_EQ(sorted.size(), static_cast<unsigned int>(4));
  ASSERT_EQ(sorted[0], 4);
  ASSERT_EQ(sorted[1], 6);
  ASSERT_EQ(sorted[2], 7);
  ASSERT_EQ(sorted[3], 8);

  sorted.clear();
  queue.sort(IntDescComparator(), &sorted);
  ASSERT_EQ(sorted.size(), static_cast<unsigned int>(4));
  ASSERT_EQ(sorted[0], 8);
  ASSERT_EQ(sorted[1], 7);
  ASSERT_EQ(sorted[2], 6);
  ASSERT_EQ(sorted[3], 4);

  sorted.clear();
  queue.pop();
  queue.sort(IntAscComparator(), &sorted);
  ASSERT_EQ(sorted.size(), static_cast<unsigned int>(3));
  ASSERT_EQ(sorted[0], 6);
  ASSERT_EQ(sorted[1], 7);
  ASSERT_EQ(sorted[2], 8);

  sorted.clear();
  queue.pop();
  queue.sort(IntAscComparator(), &sorted);
  ASSERT_EQ(sorted.size(), static_cast<unsigned int>(2));
  ASSERT_EQ(sorted[0], 7);
  ASSERT_EQ(sorted[1], 8);
}
