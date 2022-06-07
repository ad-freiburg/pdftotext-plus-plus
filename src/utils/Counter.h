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

class DoubleCounter : public std::unordered_map<double, int> {
 public:
  double mostFreq();
  int mostFreqCount();
  pair<double, int> mostFreqAndCount();
};

#endif  // COUNTER_H_