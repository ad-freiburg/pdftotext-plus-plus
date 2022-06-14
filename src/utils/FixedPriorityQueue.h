/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef FIXEDPRIORITYQUEUE_H_
#define FIXEDPRIORITYQUEUE_H_

#include <iostream>
#include <queue>

// =================================================================================================

/**
 * A priority queue with a fixed capacity.
 */
template <typename T, typename Comparator>
class FixedPriorityQueue : public std::priority_queue<T, std::vector<T>, Comparator> {
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
    std::priority_queue<T, std::vector<T>, Comparator>::push(value);
    if (this->size() > _capacity) {
      this->pop();
    }
  }

  template <class Compare>
  void sort(Compare cmp, std::vector<T>* result) {
    result->resize(this->size());
    partial_sort_copy(begin(this->c), end(this->c), begin(*result), end(*result), cmp);
  }

 private:
  // The capacity of this priority queue.
  size_t _capacity;
};

#endif  // FIXEDPRIORITYQUEUE_H_