/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <string>
#include <vector>

#include "./serialization/Serialization.h"
#include "./Types.h"
#include "./Validators.h"

using ppp::types::SerializationFormat;
using std::string;
using std::vector;

namespace po = boost::program_options;

// _________________________________________________________________________________________________
void ppp::types::validate(boost::any& v, const vector<string>& vals, SerializationFormat* f, int) {
  // Make sure no previous assignment to the option was made.
  po::validators::check_first_occurrence(v);

  // Extract the first value from 'vals'.
  // NOTE: If there is more than one token, it's an error, and an exception will be thrown.
  const string& token = po::validators::get_single_string(vals);

  for (const auto& entry : ppp::serialization::SERIALIZERS) {
    if (token == ppp::types::getName(entry.first)) {
      v = entry.first;
      return;
    }
  }

  throw po::validation_error(po::validation_error::kind_t::invalid_option_value);
}

// _________________________________________________________________________________________________
void ppp::types::validate(boost::any& v, const vector<string>& vals, SemanticRole* r, int) {
  // Make sure no previous assignment to the option was made.
  po::validators::check_first_occurrence(v);

  // Extract the first value from 'values'.
  // NOTE: If there is more than one token, it's an error, and an exception will be thrown.
  const string& token = po::validators::get_single_string(vals);

  for (size_t i = 0; i < ppp::types::SEMANTIC_ROLE_NAMES.size(); i++) {
    if (token == ppp::types::SEMANTIC_ROLE_NAMES[i]) {
      v = SemanticRole(i);
      return;
    }
  }

  throw po::invalid_option_value(token);
}