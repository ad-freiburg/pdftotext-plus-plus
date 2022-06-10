/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <utility>  // pair

#include "./Counter.h"

using std::make_pair;
using std::pair;

// _________________________________________________________________________________________________
double DoubleCounter::mostFreq() const {
  pair<double, int> pair = mostFreqAndCount();
  return pair.first;
}

// _________________________________________________________________________________________________
int DoubleCounter::mostFreqCount() const {
  pair<double, int> pair = mostFreqAndCount();
  return pair.second;
}

// _________________________________________________________________________________________________
pair<double, int> DoubleCounter::mostFreqAndCount() const {
  double mostFreq = 0.0;
  int mostFreqCount = 0;
  for (auto it = begin(); it != end(); ++it) {
    if (it->second > mostFreqCount) {
      mostFreq = it->first;
      mostFreqCount = it->second;
    }
  }
  return make_pair(mostFreq, mostFreqCount);
}

// =================================================================================================

// _________________________________________________________________________________________________
string StringCounter::mostFreq() const {
  pair<string, int> pair = mostFreqAndCount();
  return pair.first;
}

// _________________________________________________________________________________________________
int StringCounter::mostFreqCount() const {
  pair<string, int> pair = mostFreqAndCount();
  return pair.second;
}

// _________________________________________________________________________________________________
pair<string, int> StringCounter::mostFreqAndCount() const {
  string mostFreq;
  int mostFreqCount = 0;
  for (auto it = begin(); it != end(); ++it) {
    if (it->second > mostFreqCount) {
      mostFreq = it->first;
      mostFreqCount = it->second;
    }
  }
  return make_pair(mostFreq, mostFreqCount);
}
