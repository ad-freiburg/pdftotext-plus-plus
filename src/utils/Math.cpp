/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cassert>  // assert
#include <cmath>  // fabs

#include "./Math.h"

// =================================================================================================

namespace ppp::utils::math {

// _________________________________________________________________________________________________
bool equal(double d1, double d2, double tolerance) {
  return fabs(d1 - d2) <= tolerance;
}

// _________________________________________________________________________________________________
bool larger(double d1, double d2, double tolerance) {
  return d1 > d2 + tolerance;
}

// _________________________________________________________________________________________________
bool smaller(double d1, double d2, double tolerance) {
  return d1 < d2 - tolerance;
}

// _________________________________________________________________________________________________
bool equalOrLarger(double d1, double d2, double tolerance) {
  return d1 >= d2 - tolerance;
}

// _________________________________________________________________________________________________
bool equalOrSmaller(double d1, double d2, double tolerance) {
  return d1 <= d2 + tolerance;
}

// _________________________________________________________________________________________________
bool between(double d, double low, double up, double tol) {
  return equalOrLarger(d, low, tol) && equalOrSmaller(d, up, tol);
}

// _________________________________________________________________________________________________
double round(double d, int numDecimals) {
  assert(numDecimals >= 0);

  if (numDecimals == 0) {
    return std::round(d);
  }

  double divisor = std::pow(10.0, numDecimals);
  return std::round(d * divisor) / divisor;
}

}  // namespace ppp::utils::math
