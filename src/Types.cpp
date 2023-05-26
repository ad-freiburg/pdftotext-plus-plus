/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <string>
#include <vector>

#include "./Types.h"

using std::string;
using std::vector;

namespace ppp::types {

// =================================================================================================
// Document units.

// _________________________________________________________________________________________________
string getName(DocumentUnit unit) {
  return DOCUMENT_UNIT_NAMES[static_cast<int>(unit)];
}

// _________________________________________________________________________________________________
vector<DocumentUnit> getDocumentUnits() {
  vector<DocumentUnit> result;
  for (size_t i = 0; i < DOCUMENT_UNIT_NAMES.size(); i++) {
    result.push_back(DocumentUnit(i));
  }
  return result;
}

// _________________________________________________________________________________________________
string getDocumentUnitsStr(const string& separator) {
  string resultStr = "";
  for (const auto& name : DOCUMENT_UNIT_NAMES) {
    if (resultStr.size() > 0) {
      resultStr += separator;
    }
    resultStr += name;
  }
  return resultStr;
}

// =================================================================================================
// Serialization formats.

// _________________________________________________________________________________________________
string getName(SerializationFormat format) {
  return SERIALIZATION_FORMAT_NAMES[static_cast<int>(format)];
}

// _________________________________________________________________________________________________
vector<SerializationFormat> getSerializationFormats() {
  vector<SerializationFormat> result;
  for (size_t i = 0; i < SERIALIZATION_FORMAT_NAMES.size(); i++) {
    result.push_back(SerializationFormat(i));
  }
  return result;
}

// _________________________________________________________________________________________________
string getSerializationFormatsStr(const string& separator) {
  string resultStr = "";
  for (const auto& name : SERIALIZATION_FORMAT_NAMES) {
    if (resultStr.size() > 0) {
      resultStr += separator;
    }
    resultStr += name;
  }
  return resultStr;
}

// =================================================================================================
// Semantic roles.

// _________________________________________________________________________________________________
string getName(SemanticRole role) {
  return SEMANTIC_ROLE_NAMES[static_cast<int>(role)];
}

// _________________________________________________________________________________________________
vector<SemanticRole> getSemanticRoles() {
  vector<SemanticRole> result;
  for (size_t i = 0; i < SEMANTIC_ROLE_NAMES.size(); i++) {
    result.push_back(SemanticRole(i));
  }
  return result;
}

// _________________________________________________________________________________________________
string getSemanticRolesStr(const string& separator) {
  string resultStr = "";
  for (const auto& name : SEMANTIC_ROLE_NAMES) {
    if (resultStr.size() > 0) {
      resultStr += separator;
    }
    resultStr += name;
  }
  return resultStr;
}

}  // namespace ppp::types
