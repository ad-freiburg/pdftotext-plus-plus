/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <string>

#include "./PdfDocumentSerialization.h"
#include "./Types.h"
#include "./utils/PdfElementsUtils.h"

using std::string;

using ppp::types::SerializationFormat;
using ppp::utils::elements::getSerializationFormatName;

// =================================================================================================

namespace ppp::serialization {

// _________________________________________________________________________________________________
string getSerializationFormatChoicesStr() {
  string resultStr = "";
  for (const auto& entry : SERIALIZERS) {
    if (resultStr.size() > 0) {
      resultStr += ", ";
    }
    resultStr += getSerializationFormatName(entry.first);
  }
  return resultStr;
}

// _________________________________________________________________________________________________
Serializer* getSerializer(SerializationFormat format) {
  return SERIALIZERS.find(format) != SERIALIZERS.end() ? SERIALIZERS.at(format) : nullptr;
}

}  // namespace ppp::serialization
