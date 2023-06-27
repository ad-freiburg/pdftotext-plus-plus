/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_STRINGUTILS_H_
#define UTILS_STRINGUTILS_H_

#include <string>
#include <vector>

// =================================================================================================

/**
 * A collection of some useful and commonly used functions in context of strings.
 */
namespace ppp::utils::text {

// The alphabet that is used for creating random strings.
const char* const ALPHA_NUM = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

/**
 * This method splits the given text (given as a wstring) into words and appends the words to the
 * given vector.
 *
 * @param text
 *   The text to split into words.
 * @param wordDelimitersAlphabet
 *   The (concatenated) symbols to consider to be a word delimiter.
 * @param words
 *   The vector to which the computed words should be appended.
 */
void splitIntoWords(const std::wstring& text, const std::string& wordDelimitersAlphabet,
    std::vector<std::wstring>* words);

/**
 * This method splits the given text (given as a string) into words and appends the words to the
 * given vector.
 *
 * @param text
 *   The text to split into words.
 * @param wordDelimitersAlphabet
 *   The (concatenated) symbols to consider to be a word delimiter.
 * @param words
 *   The vector to which the computed words should be appended.
 */
void splitIntoWords(const std::string& text, const std::string& wordDelimitersAlphabet,
    std::vector<std::string>* words);

/**
 * This method creates a random string of the given length, consisting of alpha-numerical
 * characters. Prepends the given prefix to the created string.
 *
 * This method is used to, for example, create unique ids for extracted text elements.
 *
 * @param len
 *    The length of the string to create.
 * @param prefix
 *    The prefix to prepend to the string.
 *
 * @return
 *    The created string.
 */
std::string createRandomString(size_t len, const std::string& prefix = "");

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
std::string escapeJson(const std::string& str);

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
std::string shorten(const std::string& str, size_t len = 40);  // TODO(korzen): Parameterize.

/**
 * This method removes all leading and trailing whitespaces from the given string.
 *
 * @param str
 *   The string to process.
 *
 * @return
 *   The string without leading and trailing whitespaces.
 */
std::string strip(const std::string& str);

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
std::string wrap(const std::string& str, size_t width = 100, size_t indent = 0);

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
std::string join(const std::vector<std::string>& strings, const std::string& separator = ", ");

}  // namespace ppp::utils::text

#endif  // UTILS_STRINGUTILS_H_
