/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <vector>

#include "../PdfDocument.h"

const int FONT_SIZE_PREC = 1;
const int LINE_DIST_PREC = 1;
const int LEFT_MARGIN_PREC = 0;
const double FS_EQUAL_TOLERANCE = 1;

// =================================================================================================

enum Trool { None = -1, False, True };

// =================================================================================================

/**
 * Splits the given text (given as a std::wstring) into words.
 *
 * @param text
 *   The text to split into words.
 * @param words
 *   The vector to which the computed words should be appended.
 */
void splitIntoWords(const std::wstring& text, std::vector<std::wstring>* words);

/**
 * Splits the given text (given as a std::string) into words.
 *
 * @param text
 *   The text to split into words.
 * @param words
 *   The vector to which the computed words should be appended.
 */
void splitIntoWords(const std::string& text, std::vector<std::string>* words);

/**
 * This method is the equivalent of std::string::substr() for UTF-8 encoded strings (for which the
 * standard substr() method does not work because the positions are related to the number of bytes
 * in the string, and not to the number of characters).
 *
 * @param str
 *   The string to process.
 * @param start
 *   The position of the first character of the substring.
 * @param len
 *   The number of characters to include in the substring.
 *
 * @return The substring.
 */
// std::string utf8_substr(const std::string& str, unsigned int start, unsigned int len);

/**
 * This method is the equivalent of `std::string::length()` for UTF-8 encoded strings (for which
 * the standard length() method does not work because the returned length is related to the number
 * of bytes in the string, and not to the number of characters).
 *
 * @param str
 *   The string to process.
 *
 * @return The length of the string.
 */
// size_t utf8_length(const std::string& str);

// =================================================================================================

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

// =================================================================================================

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

// =================================================================================================

double computeHorizontalGap(const PdfElement* element1, const PdfElement* element2);
double computeVerticalGap(const PdfElement* element1, const PdfElement* element2);

// =================================================================================================

#endif  // UTILS_H_
