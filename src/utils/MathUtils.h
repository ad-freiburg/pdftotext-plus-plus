/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef MATH_UTILS_H_
#define MATH_UTILS_H_

#include <vector>


// =================================================================================================

namespace math_utils {

bool equal(double d1, double d2, double delta = 0.0001);
bool larger(double d1, double d2, double delta = 0.0001);
bool equalOrLarger(double d1, double d2, double delta = 0.0001);
bool smaller(double d1, double d2, double delta = 0.0001);
bool equalOrSmaller(double d1, double d2, double delta = 0.0001);
bool between(double d, double lower, double upper, double delta = 0.0001);

}

#endif  // MATH_UTILS_H_