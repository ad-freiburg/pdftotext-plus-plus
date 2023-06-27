/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cassert>  // assert
#include <cmath>  // fabs

#include "./MathUtils.h"

// _________________________________________________________________________________________________
bool ppp::utils::math::equal(double d1, double d2, double tolerance) {
  return fabs(d1 - d2) <= tolerance;
}

// _________________________________________________________________________________________________
bool ppp::utils::math::larger(double d1, double d2, double tolerance) {
  return d1 > d2 + tolerance;
}

// _________________________________________________________________________________________________
bool ppp::utils::math::smaller(double d1, double d2, double tolerance) {
  return d1 < d2 - tolerance;
}

// _________________________________________________________________________________________________
bool ppp::utils::math::equalOrLarger(double d1, double d2, double tolerance) {
  return d1 >= d2 - tolerance;
}

// _________________________________________________________________________________________________
bool ppp::utils::math::equalOrSmaller(double d1, double d2, double tolerance) {
  return d1 <= d2 + tolerance;
}

// _________________________________________________________________________________________________
bool ppp::utils::math::between(double d, double low, double up, double tol) {
  return equalOrLarger(d, low, tol) && equalOrSmaller(d, up, tol);
}

// _________________________________________________________________________________________________
double ppp::utils::math::round(double d, int numDecimals) {
  assert(numDecimals >= 0);

  if (numDecimals == 0) {
    return std::round(d);
  } else {
    double divisor = std::pow(10.0, numDecimals);
    return std::round(d * divisor) / divisor;
  }
}
