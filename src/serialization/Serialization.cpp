/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <string>
#include <vector>

#include "./Serialization.h"

namespace po = boost::program_options;

// _________________________________________________________________________________________________
std::string ppp::serialization::getSerializationFormatChoicesStr() {
  std::string resultStr = "";
  for (const auto& entry : SERIALIZERS) {
    if (resultStr.size() > 0) {
      resultStr += ", ";
    }
    resultStr += ppp::serialization::getName(entry.first);
  }
  return resultStr;
}

// _________________________________________________________________________________________________
std::string ppp::serialization::getName(SerializationFormat format) {
  return SERIALIZATION_FORMAT_NAMES[static_cast<int>(format)];
}

// _________________________________________________________________________________________________
Serializer* ppp::serialization::getSerializer(SerializationFormat format) {
  return SERIALIZERS.find(format) != SERIALIZERS.end() ? SERIALIZERS.at(format) : nullptr;
}

// _________________________________________________________________________________________________
void ppp::serialization::validate(boost::any& v, const std::vector<std::string>& values,
    SerializationFormat* format, int) {
  // Make sure no previous assignment to 'format' was made.
  po::validators::check_first_occurrence(v);

  // Extract the first value from 'values'. If there is more than one token, it's an error, and an
  // exception will be thrown.
  const std::string& token = po::validators::get_single_string(values);

  for (size_t i = 0; i < ppp::serialization::SERIALIZATION_FORMAT_NAMES.size(); i++) {
    if (token == ppp::serialization::SERIALIZATION_FORMAT_NAMES[i]) {
      v = ppp::serialization::SerializationFormat(i);
      return;
    }
  }

  throw po::validation_error(po::validation_error::kind_t::invalid_option_value);
}
