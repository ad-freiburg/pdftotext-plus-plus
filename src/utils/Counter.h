/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_COUNTER_H_
#define UTILS_COUNTER_H_

#include <string>
#include <unordered_map>
#include <utility>  // pair

using std::pair;
using std::string;
using std::unordered_map;

// =================================================================================================
// DoubleCounter

/**
 * This class is for counting double values and determining the most frequent double or the max
 * double. It is implemented as a subclass of unordered_map. It stores the double values as keys
 * and the respective counts as values. To initialize a counter and change the count associated
 * with a double D, you can type something like:
 *
 * DoubleCounter counter;
 * counter[D] = 3;
 * counter[D]++;
 */
class DoubleCounter : public unordered_map<double, int> {
 public:
  /**
   * This method returns the most frequent double stored in this counter (that is: the key that is
   * associated with the largest value in this unordered_map).
   *
   * @return
   *    The most frequent double in this counter.
   */
  double mostFreq() const;

  /**
   * This method returns the count of the most frequent double stored in this counter (that is: the
   * largest value in this unordered_map).
   *
   * @return
   *    The count of the most frequent double in this counter.
   */
  int mostFreqCount() const;

  /**
   * This method returns a pair consisting of the most frequent double stored in this counter and
   * the respective count.
   *
   * @return
   *    A pair of the most frequent double and the respective count.
   */
  pair<double, int> mostFreqAndCount() const;

  /**
   * This method returns the maximum double stored in this counter.
   *
   * @return
   *    The maximum double stored in this counter.
   */
  double max() const;
};

// =================================================================================================
// StringCounter

/**
 * This class is for counting string values and determining the most frequent string. It is
 * implemented as a subclass of unordered_map. It stores the strings as keys and the respective
 * counts as values. To initialize a counter and change the count associated with a string S, you
 * can type something like:
 *
 * StringCounter counter;
 * counter[S] = 3;
 * counter[S]++;
 */
class StringCounter : public unordered_map<string, int> {
 public:
  /**
   * This method returns the most frequent string stored in this counter (that is: the key that is
   * associated with the largest value in this unordered_map).
   *
   * @return
   *    The most frequent string in this counter.
   */
  string mostFreq() const;

  /**
   * This method returns the count of the most frequent string stored in this counter (that is: the
   * largest value in this unordered_map).
   *
   * @return
   *    The count of the most frequent string in this counter.
   */
  int mostFreqCount() const;

  /**
   * This method returns a pair consisting of the most frequent string stored in this counter and
   * the respective count.
   *
   * @return
   *    A pair of the most frequent string and the respective count.
   */
  pair<string, int> mostFreqAndCount() const;
};

#endif  // UTILS_COUNTER_H_
