/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cmath>
#include <iomanip> // std::setw, std::setfill
#include <string>
#include <sstream>
#include <vector>
#include <iostream>

#include "./Utils.h"
#include "../PdfDocument.h"

// _________________________________________________________________________________________________
void splitIntoWords(const std::wstring& text, std::vector<std::wstring>* words) {
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
void splitIntoWords(const std::string& text, std::vector<std::string>* words) {
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
// size_t utf8_length(const std::string& str) {
//   int len = 0;
//   for (size_t i = 0; i < str.length(); i++) {
//     len += (str[i] & 0xc0) != 0x80;
//   }
//   return len;
// }

// _________________________________________________________________________________________________
// std::string utf8_substr(const std::string& str, unsigned int start, unsigned int len) {
//   // Disclaimer: The following code is stolen from
//   // https://stackoverflow.com/questions/30995246/substring-of-a-stdstring-in-utf-8-c11
//   if (len == 0) {
//     return "";
//   }
//   size_t c, i, ix, q, min = std::string::npos, max = std::string::npos;
//   for (q = 0, i = 0, ix = str.length(); i < ix; i++, q++) {
//     if (q == start) {
//       min = i;
//     }
//     if (q <= start + len || len == std::string::npos) {
//       max = i;
//     }

//     c = (unsigned char) str[i];
//     if (c <= 127) {
//       i += 0;
//     } else if ((c & 0xE0) == 0xC0) {
//       i += 1;
//     } else if ((c & 0xF0) == 0xE0) {
//       i += 2;
//     } else if ((c & 0xF8) == 0xF0) {
//       i += 3;
//     } else {
//       return "";  // invalid utf8
//     }
//   }
//   if (q <= start + len || len == std::string::npos) {
//     max = i;
//   }
//   if (min == std::string::npos || max == std::string::npos) {
//     return "";
//   }
//   return str.substr(min, max);
// }

// =================================================================================================

// _________________________________________________________________________________________________
double computeHorizontalGap(const PdfElement* element1, const PdfElement* element2) {
  if (element1->position->rightX < element2->position->leftX) {
    return element2->position->leftX - element1->position->rightX;
  }
  if (element1->position->leftX > element2->position->rightX) {
    return element1->position->leftX - element2->position->rightX;
  }
  return 0.0;
}

// _________________________________________________________________________________________________
double computeVerticalGap(const PdfElement* element1, const PdfElement* element2) {
  if (element1->position->upperY > element2->position->lowerY) {
    return element1->position->upperY - element2->position->lowerY;
  }
  if (element1->position->lowerY < element2->position->upperY) {
    return element2->position->upperY - element1->position->lowerY;
  }
  return 0.0;
}

// =================================================================================================

// _________________________________________________________________________________________________
std::string createRandomString(size_t len, const std::string& prefix) {
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
std::string escapeJson(const std::string& str) {
  // Stolen from https://stackoverflow.com/questions/7724448/simple-json-string-escape-for-c
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

// _________________________________________________________________________________________________
bool startsWithUpper(const std::string& str) {
  if (str.empty()) {
    return false;
  }

  return isupper(str[0]);
}

// _________________________________________________________________________________________________
bool startsWithLower(const std::string& str) {
  if (str.empty()) {
    return false;
  }

  return islower(str[0]);
}

