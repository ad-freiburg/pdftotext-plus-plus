/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_TEXT_H_
#define UTILS_TEXT_H_

#include <string>
#include <vector>

#include "../Config.h"

using std::string;
using std::vector;
using std::wstring;

using ppp::config::ALPHA_NUM;

// =================================================================================================

/**
 * A collection of some useful and commonly used functions in context of strings.
 */
namespace ppp::utils::text {

/**
 * This method splits the given text (given as a wstring) into words and appends the words to the
 * given vector.
 *
 * @param text
 *   The text to split into words.
 * @param words
 *   The vector to which the computed words should be appended.
 */
void splitIntoWords(const wstring& text, vector<wstring>* words);

/**
 * This method splits the given text (given as a string) into words and appends the words to the
 * given vector.
 *
 * @param text
 *   The text to split into words.
 * @param words
 *   The vector to which the computed words should be appended.
 */
void splitIntoWords(const string& text, vector<string>* words);

/**
 * This method creates a random string of the given length, consisting of alpha-numerical
 * characters. Prepends the given prefix to the created string.
 *
 * This method is used to, for example, create the unique ids of the extracted text elements.
 *
 * @param len
 *    The length of the string to create.
 * @param prefix
 *    The prefix to prepend to the string.
 *
 * @return
 *    The created string.
 */
string createRandomString(unsigned int len, const string& prefix = "");

/**
 * This method escapes the given string to a valid JSON string. For example, it escapes double-
 * quotes and curly-braces (which have a special meaning in JSON).
 *
 * @param str
 *    The string to escape.
 *
 * @return
 *    The escaped string.
 */
string escapeJson(const string& str);

/**
 * If the given string is longer than the given length, this method shortens the string to the
 * given length and appends "..." to it. Otherwise, this method returns a copy of the string, with
 * its content untouched.
 *
 * @param str
 *    The string to shorten.
 * @param len
 *    The target length.
 *
 * @return
 *    The shortened string.
 */
string shorten(const string& str, unsigned int len = 40);

/**
 * This method removes all leading and trailing whitespaces from the given string.
 *
 * @param str
 *   The string to process.
 *
 * @return
 *   The string without leading and trailing whitespaces.
 */
string strip(const string& str);

/**
 * This method wraps the specified string so that every line is indented by <indent>-many
 * whitespaces and the length of each line (+ the length of the indent) is not larger than <width>.
 *
 * @param str
 *    The string to wrap.
 * @param width
 *    The maximal length of each line (and its indent).
 * @param indent
 *    The amount by which each line is to be indented.
 *
 * @return
 *    A string containing <str> wrapped into lines, with each lines separated by a single newline
 *    character.
 */
string wrap(const string& str, unsigned int width = 100, unsigned int indent = 0);

/**
 * This method concatenates all strings in the specified vector, using the specified character(s)
 * as separator.
 *
 * @param strings
 *    The strings to concatenate.
 * @param separator
 *    The separator.
 *
 * @return
 *    The string containing all strings in the specified vector concatenated.
 */
string join(const vector<string>& strings, const string& separator = ", ");

}  // namespace ppp::utils::text

#endif  // UTILS_TEXT_H_
