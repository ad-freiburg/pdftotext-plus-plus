/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <iomanip> // std::setw, std::setfill
#include <sstream>
#include <string>
#include <vector>

#include "./StringUtils.h"

// _________________________________________________________________________________________________
void string_utils::splitIntoWords(const std::wstring& text, std::vector<std::wstring>* words) {
  size_t n = text.length();
  const std::wstring delimiters = L" \t\r\n\f\v";
  size_t start = text.find_first_not_of(delimiters);

  while (start < n) {
    size_t stop = text.find_first_of(delimiters, start);
    if (stop > n) { stop = n; }
    words->push_back(text.substr(start, stop - start));
    start = text.find_first_not_of(delimiters, stop + 1);
  }
}

// _________________________________________________________________________________________________
void string_utils::splitIntoWords(const std::string& text, std::vector<std::string>* words) {
  size_t n = text.length();
  const std::string delimiters = " \t\r\n\f\v";
  size_t start = text.find_first_not_of(delimiters);

  while (start < n) {
    size_t stop = text.find_first_of(delimiters, start);
    if (stop > n) { stop = n; }
    words->push_back(text.substr(start, stop - start));
    start = text.find_first_not_of(delimiters, stop + 1);
  }
}

// _________________________________________________________________________________________________
std::string string_utils::createRandomString(size_t len, const std::string& prefix) {
  static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  std::string tmp_s = prefix;
  tmp_s.reserve(prefix.length() + len);

  for (size_t i = 0; i < len; i++) {
    tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  return tmp_s;
}

// =================================================================================================

// _________________________________________________________________________________________________
std::string string_utils::escapeJson(const std::string& str) {
  // This code is stolen from https://stackoverflow.com/questions/7724448
  std::stringstream o;
  for (size_t i = 0; i < str.size(); i++) {
    switch (str[i]) {
      case '"':
        o << "\\\"";
        break;
      case '\\':
        o << "\\\\";
        break;
      case '\b':
        o << "\\b";
        break;
      case '\f':
        o << "\\f";
        break;
      case '\n':
        o << "\\n";
        break;
      case '\r':
        o << "\\r";
        break;
      case '\t':
        o << "\\t";
        break;
      default:
        if ('\x00' <= str[i] && str[i] <= '\x1f') {
          o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int) str[i];
        } else {
          o << str[i];
        }
    }
  }
  return o.str();
}