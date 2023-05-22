/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <iostream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "./Serialization.h"

namespace po = boost::program_options;

// _________________________________________________________________________________________________
std::string Serialization::getSerializationFormatChoicesStr() {
  std::string choicesStr = "";
  for (const auto &entry : Serialization::NAMES_FORMAT_MAP) {
      if (choicesStr.size() > 0) {
        choicesStr += ", ";
      }
      choicesStr += entry.first;
  }
  return choicesStr;
}

// _________________________________________________________________________________________________
void Serialization::validate(boost::any& v, const std::vector<std::string>& values,
      Serialization::SerializationFormat* format, int) {
  // Make sure no previous assignment to 'format' was made.
  po::validators::check_first_occurrence(v);

  // Extract the first value from 'values'. If there is more than one token, it's an error, and an
  // exception will be thrown.
  const std::string& token = po::validators::get_single_string(values);

  if (Serialization::NAMES_FORMAT_MAP.find(token) != Serialization::NAMES_FORMAT_MAP.end()) {
    v = Serialization::NAMES_FORMAT_MAP.at(token);
    return;
  }

  throw po::validation_error(po::validation_error::kind_t::invalid_option_value);
}