/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <string.h>

#include <iostream>
#include <cassert>  // assert
#include <iomanip>  // std::setw, std::setfill
#include <sstream>
#include <string>
#include <utility>  // std::move
#include <vector>

#include "./StringUtils.h"

using std::string;
using std::stringstream;
using std::vector;
using std::wstring;

// _________________________________________________________________________________________________
void ppp::string_utils::splitIntoWords(const wstring& text, vector<wstring>* words) {
  assert(words);

  size_t n = text.length();
  const string delimiters = config::WORD_DELIMITERS_ALPHABET;
  // The following works because all characters are single-byte.
  const wstring wdelimiters(delimiters.begin(), delimiters.end());
  size_t start = text.find_first_not_of(wdelimiters);

  while (start < n) {
    size_t stop = text.find_first_of(wdelimiters, start);
    if (stop > n) { stop = n; }
    words->push_back(text.substr(start, stop - start));
    start = text.find_first_not_of(wdelimiters, stop + 1);
  }
}

// _________________________________________________________________________________________________
void ppp::string_utils::splitIntoWords(const string& text, vector<string>* words) {
  assert(words);

  size_t n = text.length();
  const string delimiters = config::WORD_DELIMITERS_ALPHABET;
  size_t start = text.find_first_not_of(delimiters);

  while (start < n) {
    size_t stop = text.find_first_of(delimiters, start);
    if (stop > n) { stop = n; }
    words->push_back(text.substr(start, stop - start));
    start = text.find_first_not_of(delimiters, stop + 1);
  }
}

// _________________________________________________________________________________________________
string ppp::string_utils::createRandomString(size_t len, const string& prefix) {
  // Append the prefix.
  string tmp_s = prefix;
  tmp_s.reserve(prefix.length() + len);

  // Append <len>-many random characters from our alphabet of alphanumerical characters.
  int alphabetSize = strlen(config::ALPHA_NUM_ALPHABET);
  for (size_t i = 0; i < len; i++) {
    tmp_s += config::ALPHA_NUM_ALPHABET[rand() % (alphabetSize - 1)];
  }

  return tmp_s;
}

// =================================================================================================

// _________________________________________________________________________________________________
string ppp::string_utils::escapeJson(const string& str) {
  // Disclaimer: this code is stolen from https://stackoverflow.com/questions/7724448
  stringstream o;
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
          o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(str[i]);
        } else {
          o << str[i];
        }
    }
  }
  return std::move(o).str();
}

// _________________________________________________________________________________________________
string ppp::string_utils::shorten(const string& str, size_t len) {
  if (str.size() <= len) {
    return str;
  }

  return str.substr(0, len) + "...";
}

// _________________________________________________________________________________________________
string ppp::string_utils::strip(const string& str) {
  auto start_it = str.begin();
  auto end_it = str.rbegin();

  while (std::isspace(*start_it)) { ++start_it; }
  while (std::isspace(*end_it)) { ++end_it; }

  return std::string(start_it, end_it.base());
}

// _________________________________________________________________________________________________
string ppp::string_utils::wrap(const string& str, size_t width, size_t indent) {
  std::string result = "";

  // Split the string into lines.
  size_t lineStart = 0;
  size_t lineEnd = 0;
  while (true) {
    // Check if the string needs to be wrapped because it contains an explicit newline character.
    lineEnd = str.rfind("\n", lineStart + width - indent);
    if (lineEnd >= lineStart && lineEnd <= str.size()) {
      for (size_t i = 0; i < indent; i++) { result += " "; }
      result += str.substr(lineStart, lineEnd - lineStart);
      result += "\n";
      lineStart = lineEnd + 1;
      continue;
    }

    // Check if the string needs to be wrapped because it is too long.
    lineEnd = str.rfind(" ", lineStart + width - indent);
    if (lineEnd < lineStart || lineEnd > str.size()) {
      break;
    }
    // Don't wrap if the string would be split in two parts whose accumulated width is <= width.
    if ((lineEnd - lineStart) + (str.size() - lineEnd) <= width) {
      break;
    }
    // Append the indent and the line to the result.
    for (size_t i = 0; i < indent; i++) { result += " "; }
    result += str.substr(lineStart, lineEnd - lineStart);
    result += "\n";
    lineStart = lineEnd + 1;
  }
  // Append the indent and the rest of the string to the result.
  for (size_t i = 0; i < indent; i++) { result += " "; }
  result += str.substr(lineStart, str.size() - lineStart);

  return result;
}

// _________________________________________________________________________________________________
std::string ppp::string_utils::join(const vector<string>& strings, const string& sep) {
  std::string resultStr = "";
  for (const auto& s : strings) {
      if (resultStr.size() > 0) {
        resultStr += sep;
      }
      resultStr += s;
  }
  return resultStr;
}
