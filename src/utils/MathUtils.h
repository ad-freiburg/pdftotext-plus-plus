/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_MATHUTILS_H_
#define UTILS_MATHUTILS_H_

#include "../Config.h"

using ppp::config::DEFAULT_DOUBLE_EQUAL_TOLERANCE;

// =================================================================================================

/**
 * A collection of some useful and commonly used math functions.
 */
namespace ppp::utils::math {

/**
 * This method returns true if the two given values d1 and d2 are (approximately) equal.
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
 *    The tolerance.
 *
 * @return
 *    True if d1 and d2 are (approximately) equal, false otherwise.
 */
bool equal(double d1, double d2, double tolerance = DEFAULT_DOUBLE_EQUAL_TOLERANCE);

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
 *    The tolerance.
 *
 * @return
 *    True if d1 is larger than d2 by the given tolerance, false otherwise.
 */
bool larger(double d1, double d2, double tolerance = DEFAULT_DOUBLE_EQUAL_TOLERANCE);

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
 *    The tolerance.
 *
 * @return
 *    True if d1 is smaller than d2 by the given tolerance, false otherwise.
 */
bool smaller(double d1, double d2, double tolerance = DEFAULT_DOUBLE_EQUAL_TOLERANCE);

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
 *    The tolerance.
 *
 * @return
 *    True if d1 is equal or larger than d2 by the given tolerance, false otherwise.
 */
bool equalOrLarger(double d1, double d2, double tolerance = DEFAULT_DOUBLE_EQUAL_TOLERANCE);

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
 *    The tolerance.
 *
 * @return
 *    True if d1 is equal or smaller than d2 by the given tolerance; false otherwise.
 */
bool equalOrSmaller(double d1, double d2, double tolerance = DEFAULT_DOUBLE_EQUAL_TOLERANCE);

/**
 * This method returns true, if the given value is inside the interval [lower, upper], under
 * consideration of the given tolerance. To be more precise, it returns true if the value is equal
 * or larger than lower by the given tolerance, and if it is equal or smaller than upper by the
 * given tolerance.
 *
 * Formally, this method returns true if: lower - tolerance <= d <= upper + tolerance.
 *
 * @param d
 *    The value to check if it falls into the interval [lower, upper].
 * @param lower
 *    The lower bound of the interval.
 * @param upper
 *    The upper bound of the interval.
 * @param tolerance
 *    The tolerance.
 *
 * @return
 *    True if lower - tolerance <= d <= upper + tolerance, false otherwise.
 */
bool between(double d, double lower, double upper,
    double tolerance = DEFAULT_DOUBLE_EQUAL_TOLERANCE);

/**
 * This method rounds the given value to <numDecimals> decimal points after the decimal point.
 *
 * @param d
 *    The value to round.
 * @param numDecimals
 *    A number >= 0 denoting the number of decimals to use when rounding the number.
 *
 * @return
 *    The rounded value.
 */
double round(double d, unsigned int numDecimals = 0);

/**
 * This method returns the minimum of the two double values.
 *
 * @param d1
 *    The first double value.
 * @param d2
 *    The second double value.
 *
 * @return
 *    The minimum of the two double values.
 */
double minimum(double d1, double d2);

/**
 * This method returns the minimum of the three double values.
 *
 * @param d1
 *    The first double value.
 * @param d2
 *    The second double value.
 * @param d3
 *    The third double value.
 *
 * @return
 *    The minimum of the three double values.
 */
double minimum(double d1, double d2, double d3);

/**
 * This method returns the minimum of the four double values.
 *
 * @param d1
 *    The first double value.
 * @param d2
 *    The second double value.
 * @param d3
 *    The third double value.
 * @param d4
 *    The fourth double value.
 *
 * @return
 *    The minimum of the four double values.
 */
double minimum(double d1, double d2, double d3, double d4);

/**
 * This method returns the maximum of the two double values.
 *
 * @param d1
 *    The first double value.
 * @param d2
 *    The second double value.
 *
 * @return
 *    The maximum of the two double values.
 */
double maximum(double d1, double d2);

/**
 * This method returns the maximum of the three double values.
 *
 * @param d1
 *    The first double value.
 * @param d2
 *    The second double value.
 * @param d3
 *    The third double value.
 *
 * @return
 *    The maximum of the two double values.
 */
double maximum(double d1, double d2, double d3);

/**
 * This method returns the maximum of the three double values.
 *
 * @param d1
 *    The first double value.
 * @param d2
 *    The second double value.
 * @param d3
 *    The third double value.
 * @param d4
 *    The fourth double value.
 *
 * @return
 *    The maximum of the four double values.
 */
double maximum(double d1, double d2, double d3, double d4);

}  // namespace ppp::utils::math

#endif  // UTILS_MATHUTILS_H_
