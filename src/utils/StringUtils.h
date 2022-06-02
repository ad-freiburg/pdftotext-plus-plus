/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef STRINGUTILS_H_
#define STRINGUTILS_H_

#include <string>
#include <vector>

namespace string_utils {

/**
 * This method splits the given text (given as a std::wstring) into words and appends the words
 * to the given vector.
 *
 * @param text
 *   The text to split into words.
 * @param words
 *   The vector to which the computed words should be appended.
 */
void splitIntoWords(const std::wstring& text, std::vector<std::wstring>* words);

/**
 * This method splits the given text (given as a std::string) into words and append the words to
 * the given vector.
 *
 * @param text
 *   The text to split into words.
 * @param words
 *   The vector to which the computed words should be appended.
 */
void splitIntoWords(const std::string& text, std::vector<std::string>* words);

/**
 * This method creates a random string of the given length that consists of alpha-numerical
 * characters and that starts with the given prefix. It is used to, for example, create unique ids
 * for the extracted text elements.
 *
 * @param len
 *    The length of the string to create.
 * @param prefix
 *    The prefix.
 *
 * @return The created string.
 */
std::string createRandomString(size_t len, const std::string& prefix="");

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

}  // namespace string_utils

#endif  // #define STRINGUTILS_H_
