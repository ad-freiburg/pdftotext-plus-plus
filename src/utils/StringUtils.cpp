/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cassert>  // assert
#include <iomanip>  // std::setw, std::setfill
#include <sstream>
#include <string>
#include <utility>  // std::move
#include <vector>

#include "../Constants.h"

#include "./StringUtils.h"

using std::string;
using std::stringstream;
using std::vector;
using std::wstring;

// _________________________________________________________________________________________________
void string_utils::splitIntoWords(const wstring& text, vector<wstring>* words) {
  assert(words);

  size_t n = text.length();
  const wstring delimiters = L" \t\r\n\f\v";  // TODO(korzen): Move this to Constants.h
  size_t start = text.find_first_not_of(delimiters);

  while (start < n) {
    size_t stop = text.find_first_of(delimiters, start);
    if (stop > n) { stop = n; }
    words->push_back(text.substr(start, stop - start));
    start = text.find_first_not_of(delimiters, stop + 1);
  }
}

// _________________________________________________________________________________________________
void string_utils::splitIntoWords(const string& text, vector<string>* words) {
  assert(words);

  size_t n = text.length();
  const string delimiters = " \t\r\n\f\v";  // TODO(korzen): Move this to Constants.h
  size_t start = text.find_first_not_of(delimiters);

  while (start < n) {
    size_t stop = text.find_first_of(delimiters, start);
    if (stop > n) { stop = n; }
    words->push_back(text.substr(start, stop - start));
    start = text.find_first_not_of(delimiters, stop + 1);
  }
}

// _________________________________________________________________________________________________
string string_utils::createRandomString(size_t len, const string& prefix) {
  // Append the prefix.
  string tmp_s = prefix;
  tmp_s.reserve(prefix.length() + len);

  // Append <len>-many random characters from our alphabet of alphanumerical characters.
  int alphabetSize = sizeof(ALPHA_NUM_ALPHABET);
  for (size_t i = 0; i < len; i++) {
    tmp_s += ALPHA_NUM_ALPHABET[rand() % (alphabetSize - 1)];
  }

  return tmp_s;
}

// =================================================================================================

// _________________________________________________________________________________________________
string string_utils::escapeJson(const string& str) {
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
