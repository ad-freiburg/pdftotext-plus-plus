/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cmath>  // fabs

# include "./MathUtils.h"

// _________________________________________________________________________________________________
bool math_utils::equal(double d1, double d2, double delta) {
  return fabs(d1 - d2) <= delta;
}

// _________________________________________________________________________________________________
bool math_utils::larger(double d1, double d2, double delta) {
  return d1 - d2 > delta;
}

// _________________________________________________________________________________________________
bool math_utils::equalOrLarger(double d1, double d2, double delta) {
  return math_utils::equal(d1, d2, delta) || math_utils::larger(d1, d2, delta);
}

// _________________________________________________________________________________________________
bool math_utils::smaller(double d1, double d2, double delta) {
  return d1 - d2 < -1 * delta;
}

// _________________________________________________________________________________________________
bool math_utils::equalOrSmaller(double d1, double d2, double delta) {
  return math_utils::equal(d1, d2, delta) || math_utils::smaller(d1, d2, delta);
}

// _________________________________________________________________________________________________
bool math_utils::between(double d, double low, double up) {
  return math_utils::equalOrLarger(d, low, 0.0001) && math_utils::equalOrSmaller(d, up, 0.0001);
}