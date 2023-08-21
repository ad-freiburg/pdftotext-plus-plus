/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cassert>  // assert
#include <iomanip>  // std::setw, std::setfill
#include <sstream>  // std::stringstream
#include <string>
#include <utility>  // std::move
#include <vector>

#include "./TextUtils.h"
#include "../Config.h"

using std::hex;
using std::isspace;
using std::move;
using std::setfill;
using std::setw;
using std::string;
using std::stringstream;
using std::vector;
using std::wstring;

// =================================================================================================

namespace ppp::utils::text {

// _________________________________________________________________________________________________
void splitIntoWords(const wstring& text, vector<wstring>* words, const char* const wordDelimiters) {
  assert(words);

  // TODO(korzen): Is there a more elegant solution for converting char* to wstring?
  // Do we need to convert it at all, or is there another solution?
  const string delimiters = string(wordDelimiters);
  const wstring wdelimiters(delimiters.begin(), delimiters.end());

  unsigned int start = text.find_first_not_of(wdelimiters);
  while (start < text.length()) {
    unsigned int stop = text.find_first_of(wdelimiters, start);
    if (stop > text.length()) { stop = text.length(); }
    words->push_back(text.substr(start, stop - start));
    start = text.find_first_not_of(wdelimiters, stop + 1);
  }
}

// _________________________________________________________________________________________________
void splitIntoWords(const string& text, vector<string>* words, const char* const wordDelimiters) {
  assert(words);

  // TODO(korzen): Is there a more elegant solution for converting char* to string?
  // Do we need to convert it at all, or is there another solution?
  const string delimiters = string(wordDelimiters);

  unsigned int start = text.find_first_not_of(delimiters);
  while (start < text.length()) {
    unsigned int stop = text.find_first_of(delimiters, start);
    if (stop > text.length()) { stop = text.length(); }
    words->push_back(text.substr(start, stop - start));
    start = text.find_first_not_of(delimiters, stop + 1);
  }
}

// _________________________________________________________________________________________________
bool endsWithSentenceDelimiter(const string& text, const char* const sentenceDelimiters) {
  if (text.empty()) {
    return false;
  }

  // TODO(korzen): Is there a more elegant solution for converting char* to string?
  // Do we need to convert it at all, or is there another solution?
  const string delimiters = string(sentenceDelimiters);

  return delimiters.find(text.back()) != std::string::npos;
}

// _________________________________________________________________________________________________
bool startsWithUpper(const string& str) {
  return !str.empty() ? isupper(str[0]) : false;
}


// _________________________________________________________________________________________________
string createRandomString(unsigned int len, const string& prefix, const char* const alphabet) {
  // Append the prefix.
  string str = prefix;
  str.reserve(prefix.length() + len);

  // Append <len>-many random alphanumerical characters.
  int alphabetSize = strlen(alphabet);
  for (unsigned int i = 0; i < len; i++) {
    str += alphabet[rand() % (alphabetSize - 1)];
  }

  return str;
}

// _________________________________________________________________________________________________
string escapeJson(const string& str) {
  // Disclaimer: this code is stolen from https://stackoverflow.com/questions/7724448
  stringstream o;
  for (unsigned int i = 0; i < str.size(); i++) {
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
          o << "\\u" << hex << setw(4) << setfill('0') << static_cast<int>(str[i]);
        } else {
          o << str[i];
        }
    }
  }
  return move(o).str();
}

// _________________________________________________________________________________________________
string shorten(const string& str, unsigned int len) {
  if (str.size() <= len) {
    return str;
  }

  return str.substr(0, len) + "...";
}

// _________________________________________________________________________________________________
string strip(const string& str) {
  auto start_it = str.begin();
  auto end_it = str.rbegin();

  while (isspace(*start_it)) { start_it++; }
  while (isspace(*end_it)) { end_it++; }

  return string(start_it, end_it.base());
}

// _________________________________________________________________________________________________
string wrap(const string& str, unsigned int width, unsigned int indent) {
  string result = "";

  // Split the string into lines.
  unsigned int lineStart = 0;
  unsigned int lineEnd = 0;
  while (true) {
    // Check if the string needs to be wrapped because it contains an explicit newline character.
    lineEnd = str.rfind("\n", lineStart + width - indent);
    if (lineEnd >= lineStart && lineEnd <= str.size()) {
      for (unsigned int i = 0; i < indent; i++) { result += " "; }
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
    for (unsigned int i = 0; i < indent; i++) { result += " "; }
    result += str.substr(lineStart, lineEnd - lineStart);
    result += "\n";
    lineStart = lineEnd + 1;
  }
  // Append the indent and the rest of the string to the result.
  for (unsigned int i = 0; i < indent; i++) { result += " "; }
  result += str.substr(lineStart, str.size() - lineStart);

  return result;
}

// _________________________________________________________________________________________________
string join(const vector<string>& strings, const string& sep) {
  string resultStr = "";
  for (const auto& s : strings) {
      if (resultStr.size() > 0) {
        resultStr += sep;
      }
      resultStr += s;
  }
  return resultStr;
}

}  // namespace ppp::utils::text
