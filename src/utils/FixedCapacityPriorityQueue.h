/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_FIXEDCAPACITYPRIORITYQUEUE_H_
#define UTILS_FIXEDCAPACITYPRIORITYQUEUE_H_

#include <algorithm>  // std::sort
#include <queue>
#include <vector>

using std::priority_queue;
using std::vector;

// =================================================================================================

/**
 * A priority queue with a fixed capacity. If on pushing a new element the queue is already full
 * (i.e., _capacity == _size), the element referenced by top() is removed beforehand (so that the
 * priority queue never contains more than <capacity>-many elements).
 */
template <typename T, typename Comparator>
class FixedCapacityPriorityQueue : public priority_queue<T, vector<T>, Comparator> {
 public:
  /**
   * The default constructor.
   *
   * @param capacity.
   *    The capacity of this priority queue.
   */
  explicit FixedCapacityPriorityQueue(int capacity) {
    _capacity = capacity;
  }

  /** The deconstructor. */
  ~FixedCapacityPriorityQueue() = default;

  /**
   * This method pushes the given value to this priority queue. If the priority queue is already
   * full (meaning that _size == _capacity), the top element is removed beforehand (so that the
   * queue never contains more than <_capacity>-many elements).
   *
   * @param value
   *   The value to push.
   */
  void push(const T& value) {
    if (this->size() == _capacity) {
      this->pop();
    }
    priority_queue<T, vector<T>, Comparator>::push(value);
  }

  /**
   * This method sorts the elements contained in the queue using the given comparator and appends
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
  unsigned int _capacity;

  friend class FixedCapacityPriorityQueueTest_constructor_Test;
  friend class FixedCapacityPriorityQueueTest_pushPopInt_Test;
  friend class FixedCapacityPriorityQueueTest_pushPopString_Test;
};

#endif  // UTILS_FIXEDCAPACITYPRIORITYQUEUE_H_
