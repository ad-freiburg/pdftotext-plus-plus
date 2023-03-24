/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_FIXEDPRIORITYQUEUE_H_
#define UTILS_FIXEDPRIORITYQUEUE_H_

#include <algorithm>  // std::sort
#include <queue>
#include <vector>

using std::priority_queue;
using std::vector;

// =================================================================================================

/**
 * A priority queue with a fixed capacity. If the size of this queue is larger than the given
 * capacity after inserting an element, the element referenced by top() is removed, so that the
 * priority queue never contains more than <capacity>-many elements.
 */
template <typename T, typename Comparator>
class FixedPriorityQueue : public priority_queue<T, vector<T>, Comparator> {
 public:
  /**
   * This constructor creates and initializes a new instance of this class.
   *
   * @param capacity.
   *    The capacity of this priority queue.
   */
  explicit FixedPriorityQueue(int capacity) {
    _capacity = capacity;
  }

  /**
   * This method pushes the given value to this priority queue. If the size of the priority queue
   * is larger than the capacity afterwards, the top element is removed, so that the priority queue
   * never contains more than <capacity>-many elements.
   *
   * @param value
   *   The value to push.
   */
  void push(const T& value) {
    priority_queue<T, vector<T>, Comparator>::push(value);
    if (this->size() > _capacity) {
      this->pop();
    }
  }

  /**
   * This method sorts the elements contained in the queue using the given comparator and append
   * the elements to the given result vector (in sorted order).
   *
   * @param cmp
   *    The comparator.
   * @param result
   *    The result vector to which the sorted elements should be appended.
   */
  template <class Compare>
  void sort(const Compare& cmp, vector<T>* result) const {
    assert(result);

    result->resize(this->size());
    partial_sort_copy(begin(this->c), end(this->c), begin(*result), end(*result), cmp);
  }

 private:
  // The capacity of this priority queue.
  size_t _capacity;

  friend class FixedPriorityQueue_pushpop_Test;  // same: FRIEND_TEST(FixedPriorityQueue, pushpop);
};

#endif  // UTILS_FIXEDPRIORITYQUEUE_H_
