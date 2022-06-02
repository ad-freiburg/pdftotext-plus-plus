/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef MATHUTILS_H_
#define MATHUTILS_H_

// =================================================================================================

namespace math_utils {

/**
 * This method returns true, if the two given values d1 and d2 are (approximately) equal.
 *
 * Whether or not the two values are considered equal depends on the given tolerance, which
 * represents the maximum allowed difference between d1 and d2.
 *
 * Formally, this method returns true if: abs(d1 - d2) <= tolerance.
 *
 * @param d1
 *    The first value.
 * @param d2
 *    The second value.
 * @param tolerance
 *    The (absolute) tolerance.
 *
 * @return
 *    True if d1 and d2 are (approximately) equal, false otherwise.
 */
bool equal(double d1, double d2, double tolerance = 0.0001);

/**
 * This method returns true, if the first value is larger than the second value by the given
 * tolerance.
 *
 * Formally, this method returns true if: d1 > d2 + tolerance.
 *
 * @param d1
 *    The first value.
 * @param d2
 *    The second value.
 * @param tolerance
 *    The (absolute) tolerance.
 *
 * @return
 *    True if d1 is larger than d2 by the given tolerance, false otherwise.
 */
bool larger(double d1, double d2, double tolerance = 0.0001);

/**
 * This method returns true, if the first value is smaller than the second value by the given
 * tolerance.
 *
 * Formally, this method returns true if: d1 < d2 - tolerance.
 *
 * @param d1
 *    The first value.
 * @param d2
 *    The second value.
 * @param tolerance
 *    The (absolute) tolerance.
 *
 * @return
 *    True if d1 is smaller than d2 by the given tolerance, false otherwise.
 */
bool smaller(double d1, double d2, double tolerance = 0.0001);

/**
 * This method returns true, if the first value is equal or larger than the second value by the
 * given tolerance.
 *
 * Formally, this method returns true if: d1 >= d2 - tolerance.
 *
 * @param d1
 *    The first value.
 * @param d2
 *    The second value.
 * @param tolerance
 *    The (absolute) tolerance.
 *
 * @return
 *    True if d1 is equal or larger than d2 by the given tolerance, false otherwise.
 */
bool equalOrLarger(double d1, double d2, double tolerance = 0.0001);

/**
 * This method returns true, if the first value is equal or smaller than the second value by the
 * given tolerance.
 *
 * Formally, this method returns true if: d1 <= d2 + tolerance.
 *
 * @param d1
 *    The first value.
 * @param d2
 *    The second value.
 * @param tolerance
 *    The (absolute) tolerance.
 *
 * @return
 *    True if d1 is equal or smaller than d2 by the given tolerance; false otherwise.
 */
bool equalOrSmaller(double d1, double d2, double tolerance = 0.0001);

/**
 * This method returns true, if the given value is equal or larger than the given lower bound by
 * the given tolerance, or if it is equal or smaller than the given upper bound by the given
 * tolerance.
 *
 * Formally, this method returns true if: lower - tolerance <= d <= upper + tolerance.
 *
 * @param d
 *    The value.
 * @param lower
 *    The lower bound.
 * @param upper
 *    The upper bound.
 * @param tolerance
 *    The (absolute) tolerance.
 *
 * @return
 *    True if lower - tolerance <= d <= upper + tolerance, false otherwise.
 */
bool between(double d, double lower, double upper, double tolerance = 0.0001);

}  // namespace math_utils

#endif  // MATHUTILS_H_
