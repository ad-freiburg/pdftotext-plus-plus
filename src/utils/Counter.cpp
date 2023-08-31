/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <limits>  // std::numeric_limits
#include <string>
#include <utility>  // std::pair

#include "./Counter.h"
#include "./MathUtils.h"

using std::make_pair;
using std::numeric_limits;
using std::pair;
using std::runtime_error;
using std::string;

using ppp::utils::math::larger;

// =================================================================================================

namespace ppp::utils::counter {

// _________________________________________________________________________________________________
double DoubleCounter::mostFreq() const {
  return mostFreqAndCount().first;
}

// _________________________________________________________________________________________________
unsigned int DoubleCounter::mostFreqCount() const {
  return mostFreqAndCount().second;
}

// _________________________________________________________________________________________________
pair<double, unsigned int> DoubleCounter::mostFreqAndCount() const {
  double mostFreq = 0.0;
  unsigned int mostFreqCount = 0;

  for (auto it = begin(); it != end(); it++) {
    if (it->second > mostFreqCount) {
      mostFreq = it->first;
      mostFreqCount = it->second;
    }
  }

  if (mostFreqCount == 0) {
    throw runtime_error("The counter is empty or all counts are == 0.");
  }

  return make_pair(mostFreq, mostFreqCount);
}

// _________________________________________________________________________________________________
double DoubleCounter::max() const {
  double max = numeric_limits<double>::min();
  bool entryFound = false;

  for (auto it = begin(); it != end(); it++) {
    if (it->second > 0 && larger(it->first, max)) {
      max = it->first;
      entryFound = true;
    }
  }

  if (!entryFound) {
    throw runtime_error("The counter is empty or all counts are == 0.");
  }

  return max;
}

// _________________________________________________________________________________________________
unsigned int DoubleCounter::sumCounts() const {
  unsigned int sum = 0;

  for (auto it = begin(); it != end(); it++) {
    sum += it->second;
  }

  return sum;
}

// =================================================================================================
// StringCounter

// _________________________________________________________________________________________________
string StringCounter::mostFreq() const {
  return mostFreqAndCount().first;
}

// _________________________________________________________________________________________________
unsigned int StringCounter::mostFreqCount() const {
  return mostFreqAndCount().second;
}

// _________________________________________________________________________________________________
pair<string, unsigned int> StringCounter::mostFreqAndCount() const {
  string mostFreq;
  unsigned int mostFreqCount = 0;

  for (auto it = begin(); it != end(); it++) {
    if (it->second > mostFreqCount) {
      mostFreq = it->first;
      mostFreqCount = it->second;
    }
  }

  if (mostFreqCount == 0) {
    throw runtime_error("The counter is empty or all counts are <= 0.");
  }

  return make_pair(mostFreq, mostFreqCount);
}

// _________________________________________________________________________________________________
unsigned int StringCounter::sumCounts() const {
  unsigned int sum = 0;

  for (auto it = begin(); it != end(); it++) {
    sum += it->second;
  }

  return sum;
}

}  // namespace ppp::utils::counter
