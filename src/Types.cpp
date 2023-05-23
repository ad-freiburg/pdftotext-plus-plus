/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "./Types.h"

namespace po = boost::program_options;

// _________________________________________________________________________________________________
void ppp::types::validate(boost::any& v, const std::vector<std::string>& values,
      SerializationFormat* format, int) {
  // Make sure no previous assignment to 'format' was made.
  po::validators::check_first_occurrence(v);

  // Extract the first value from 'values'. If there is more than one token, it's an error, and an
  // exception will be thrown.
  const std::string& token = po::validators::get_single_string(values);

  for (size_t i = 0; i < SERIALIZATION_FORMAT_NAMES.size(); i++) {
    if (token == SERIALIZATION_FORMAT_NAMES[i]) {
      v = ppp::types::SerializationFormat(i);
      return;
    }
  }

  throw po::validation_error(po::validation_error::kind_t::invalid_option_value);
}