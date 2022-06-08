/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef COUNTER_H_
#define COUNTER_H_

#include <unordered_map>
#include <utility>  // pair

using std::pair;

// =================================================================================================

/**
 * A subclass of unordered_map for counting double values. It stores the double values as keys and
 * their counts as values. To initialize a counter and change the count associated with a double
 * value D, type something like:
 *
 * DoubleCounter counter;
 * counter[D] = 3;
 * counter[D]++;
 */
class DoubleCounter : public std::unordered_map<double, int> {
 public:
  /**
   * This method returns the most frequent double stored in this counter (that is: the key that is
   * associated with the largest value in this unordered_map).
   *
   * @return The most frequent double in this counter.
   */
  double mostFreq();

  /**
   * This method returns the count of the most frequent double stored in this counter (that is: the
   * largest value in this unordered_map).
   *
   * @return The count of the most frequent double in this counter.
   */
  int mostFreqCount();

  /**
   * This method returns a pair consisting of the most frequent double stored in this counter and
   * the respective count.
   *
   * @return A pair of the most frequent double and the respective count.
   */
  pair<double, int> mostFreqAndCount();
};

#endif  // COUNTER_H_