/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <string>

#include "../Types.h"
#include "./Serialization.h"

using ppp::types::SerializationFormat;
using std::string;

// =================================================================================================

namespace ppp::serialization {

// _________________________________________________________________________________________________
string getSerializationFormatChoicesStr() {
  string resultStr = "";
  for (const auto& entry : SERIALIZERS) {
    if (resultStr.size() > 0) {
      resultStr += ", ";
    }
    resultStr += ppp::types::getName(entry.first);
  }
  return resultStr;
}

// _________________________________________________________________________________________________
Serializer* getSerializer(SerializationFormat format) {
  return SERIALIZERS.find(format) != SERIALIZERS.end() ? SERIALIZERS.at(format) : nullptr;
}

}  // namespace ppp::serialization
