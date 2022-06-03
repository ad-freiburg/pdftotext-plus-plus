/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cassert>  // assert
#include <cmath>  // fabs

#include "./MathUtils.h"

// _________________________________________________________________________________________________
bool math_utils::equal(double d1, double d2, double tolerance) {
  return fabs(d1 - d2) <= tolerance;
}

// _________________________________________________________________________________________________
bool math_utils::larger(double d1, double d2, double tolerance) {
  return d1 > d2 + tolerance;
}

// _________________________________________________________________________________________________
bool math_utils::smaller(double d1, double d2, double tolerance) {
  return d1 < d2 - tolerance;
}

// _________________________________________________________________________________________________
bool math_utils::equalOrLarger(double d1, double d2, double tolerance) {
  return d1 >= d2 - tolerance;
}

// _________________________________________________________________________________________________
bool math_utils::equalOrSmaller(double d1, double d2, double tolerance) {
  return d1 <= d2 + tolerance;
}

// _________________________________________________________________________________________________
bool math_utils::between(double d, double low, double up, double tol) {
  return math_utils::equalOrLarger(d, low, tol) && math_utils::equalOrSmaller(d, up, tol);
}

// _________________________________________________________________________________________________
double math_utils::round(double d, int numDecimals) {
  assert(numDecimals >= 0);

  if (numDecimals == 0) {
    return std::round(d);
  } else {
    double divisor = 10.0 * numDecimals;
    return static_cast<double>(static_cast<int>(d * divisor)) / divisor;
  }
}
