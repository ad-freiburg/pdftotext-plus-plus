/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_MATHUTILS_H_
#define UTILS_MATHUTILS_H_

#include "../Constants.h"

using global_config::DOUBLE_EQUAL_TOLERANCE;

// =================================================================================================

/**
 * A collection of some useful and commonly used math functions.
 */
namespace math_utils {

/**
 * This method returns true, if the two given values d1 and d2 are (approximately) equal.
 *
 * Whether or not the two values are considered equal depends on the given tolerance, which
 * denote the maximum allowed difference between d1 and d2.
 *
 * Formally, this method returns true if: abs(d1 - d2) <= tolerance.
 *
 * @param d1
 *    The first value to compare.
 * @param d2
 *    The second value to compare.
 * @param tolerance
 *    The (absolute) tolerance.
 *
 * @return
 *    True if d1 and d2 are (approximately) equal, false otherwise.
 */
bool equal(double d1, double d2, double tolerance = DOUBLE_EQUAL_TOLERANCE);

/**
 * This method returns true, if the first value is larger than the second value by the given
 * tolerance.
 *
 * Formally, this method returns true if: d1 > d2 + tolerance.
 *
 * @param d1
 *    The first value to compare.
 * @param d2
 *    The second value to compare.
 * @param tolerance
 *    The (absolute) tolerance.
 *
 * @return
 *    True if d1 is larger than d2 by the given tolerance, false otherwise.
 */
bool larger(double d1, double d2, double tolerance = DOUBLE_EQUAL_TOLERANCE);

/**
 * This method returns true, if the first value is smaller than the second value by the given
 * tolerance.
 *
 * Formally, this method returns true if: d1 < d2 - tolerance.
 *
 * @param d1
 *    The first value to compare.
 * @param d2
 *    The second value to compare.
 * @param tolerance
 *    The (absolute) tolerance.
 *
 * @return
 *    True if d1 is smaller than d2 by the given tolerance, false otherwise.
 */
bool smaller(double d1, double d2, double tolerance = DOUBLE_EQUAL_TOLERANCE);

/**
 * This method returns true, if the first value is (approximately) equal to the second value, or
 * if it is larger than the second value by the given tolerance.
 *
 * Formally, this method returns true if: d1 >= d2 - tolerance.
 *
 * @param d1
 *    The first value to compare.
 * @param d2
 *    The second value to compare.
 * @param tolerance
 *    The (absolute) tolerance.
 *
 * @return
 *    True if d1 is equal or larger than d2 by the given tolerance, false otherwise.
 */
bool equalOrLarger(double d1, double d2, double tolerance = DOUBLE_EQUAL_TOLERANCE);

/**
 * This method returns true, if the first value is (approximately) equal to the second value, or
 * if it is smaller than the second value by the given tolerance.
 *
 * Formally, this method returns true if: d1 <= d2 + tolerance.
 *
 * @param d1
 *    The first value to compare.
 * @param d2
 *    The second value to compare.
 * @param tolerance
 *    The (absolute) tolerance.
 *
 * @return
 *    True if d1 is equal or smaller than d2 by the given tolerance; false otherwise.
 */
bool equalOrSmaller(double d1, double d2, double tolerance = DOUBLE_EQUAL_TOLERANCE);

/**
 * This method returns true, if the given value is inside the given interval, under consideration
 * of the given tolerance. To be more precise, it returns true if the value is equal or larger than
 * the given lower bound by the given tolerance, or if it is equal or smaller than the given upper
 * bound by the given tolerance.
 *
 * Formally, this method returns true if: lower - tolerance <= d <= upper + tolerance.
 *
 * @param d
 *    The value to check if it is inside the interval.
 * @param lower
 *    The lower bound of the interval.
 * @param upper
 *    The upper bound of the interval.
 * @param tolerance
 *    The (absolute) tolerance.
 *
 * @return
 *    True if lower - tolerance <= d <= upper + tolerance, false otherwise.
 */
bool between(double d, double lower, double upper, double tolerance = DOUBLE_EQUAL_TOLERANCE);

/**
 * This method rounds the given value to <numDecimals> precision after the decimal point.
 *
 * @param d
 *    The value to round.
 * @param numDecimals
 *    A number >= 0 denoting the number of decimals to use when rounding the number.
 *
 * @return
 *    The rounded value.
 */
double round(double d, int numDecimals = 0);

}  // namespace math_utils

#endif  // UTILS_MATHUTILS_H_
