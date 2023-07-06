/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_COUNTER_H_
#define UTILS_COUNTER_H_

#include <string>
#include <unordered_map>
#include <utility>  // std::pair

using std::pair;
using std::string;
using std::unordered_map;

// =================================================================================================

namespace ppp::utils::counter {

/**
 * This class is for counting given double values and for determining the most frequent double
 * value or the maximum double value. It is implemented as a subclass of unordered_map. It stores
 * the double values as keys and the respective counts as values. To initialize a counter and
 * change the count associated with a double D, you can type something like:
 *
 * DoubleCounter counter;
 * counter[D] = 3;
 * counter[D]++;
 */
class DoubleCounter : public unordered_map<double, unsigned int> {
 public:
  /**
   * This method returns the most frequent double value stored in this counter (that is: the key
   * that is associated with the largest value in this map).
   *
   * @return
   *    The most frequent double value in this counter.
   *
   * @throws std::runtime_error
   *    When the counter is empty or all counts are == 0.
   */
  double mostFreq() const;

  /**
   * This method returns the count of the most frequent double value stored in this counter (that
   * is: the largest value in this map).
   *
   * @return
   *    The count of the most frequent double value in this counter.
   *
   * @throws std::runtime_error
   *    When the counter is empty or all counts are == 0.
   */
  unsigned int mostFreqCount() const;

  /**
   * This method returns the most frequent double value stored in this counter *and* the respective
   * count.
   *
   * @return
   *    A pair containing the most frequent double value and the respective count.
   *
   * @throws std::runtime_error
   *    When the counter is empty or all counts are <= 0.
   */
  pair<double, unsigned int> mostFreqAndCount() const;

  /**
   * This method returns the maximum double value which is associated with a count > 0 in this
   * counter.
   *
   * @return
   *    The maximum double value stored in this counter.
   *
   * @throws std::runtime_error
   *    When the counter is empty or all counts are == 0.
   */
  double max() const;

  /**
   * This method returns the sum of all counts.
   *
   * @return
   *    The sum of all counts.
   */
  unsigned int sumCounts() const;
};

// =================================================================================================

/**
 * This class is for counting given string values and determining the most frequent string. It is
 * implemented as a subclass of unordered_map. It stores the strings as keys and the respective
 * counts as values. To initialize a counter and change the count associated with a string S, you
 * can type something like:
 *
 * StringCounter counter;
 * counter[S] = 3;
 * counter[S]++;
 */
class StringCounter : public unordered_map<string, unsigned int> {
 public:
  /**
   * This method returns the most frequent string stored in this counter (that is: the key that is
   * associated with the largest value in this map).
   *
   * @return
   *    The most frequent string in this counter.
   *
   * @throws std::runtime_error
   *    When the counter is empty or all counts are == 0.
   */
  string mostFreq() const;

  /**
   * This method returns the count of the most frequent string stored in this counter (that is: the
   * largest value in this map).
   *
   * @return
   *    The count of the most frequent string in this counter.
   *
   * @throws std::runtime_error
   *    When the counter is empty or all counts are == 0.
   */
  unsigned int mostFreqCount() const;

  /**
   * This method returns the most frequent string stored in this counter *and* the respective count.
   *
   * @return
   *    A pair of the most frequent string and the respective count.
   *
   * @throws std::runtime_error
   *    When the counter is empty or all counts are <= 0.
   */
  pair<string, unsigned int> mostFreqAndCount() const;

  /**
   * This method returns the sum of all counts stored in this counter.
   *
   * @return
   *    The sum of all counts stored in this counter.
   */
  unsigned int sumCounts() const;
};

}  // namespace ppp::utils::counter

#endif  // UTILS_COUNTER_H_
