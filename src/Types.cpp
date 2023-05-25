/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <string>

#include "./Types.h"

using std::string;

// _________________________________________________________________________________________________
string ppp::types::getSerializationFormatsStr(const string& separator) {
  string resultStr = "";
  for (const auto& name : SERIALIZATION_FORMAT_NAMES) {
    if (resultStr.size() > 0) {
      resultStr += separator;
    }
    resultStr += name;
  }
  return resultStr;
}

// _________________________________________________________________________________________________
string ppp::types::getSemanticRolesStr(const string& separator) {
  string resultStr = "";
  for (const auto& name : SEMANTIC_ROLE_NAMES) {
    if (resultStr.size() > 0) {
      resultStr += separator;
    }
    resultStr += name;
  }
  return resultStr;
}

// _________________________________________________________________________________________________
string ppp::types::getName(SerializationFormat format) {
  return SERIALIZATION_FORMAT_NAMES[static_cast<int>(format)];
}

// _________________________________________________________________________________________________
string ppp::types::getName(SemanticRole role) {
  return SEMANTIC_ROLE_NAMES[static_cast<int>(role)];
}