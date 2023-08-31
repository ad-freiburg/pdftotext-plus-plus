/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <string>
#include <vector>

#include "./PdfDocumentSerialization.h"
#include "./Types.h"
#include "./Validators.h"
#include "./utils/PdfElementsUtils.h"

using std::string;
using std::vector;

using ppp::serialization::SERIALIZERS;
using ppp::types::SerializationFormat;
using ppp::types::SemanticRole;
using ppp::types::PDF_ELEMENT_TYPE_NAMES;
using ppp::types::SEMANTIC_ROLE_NAMES;
using ppp::utils::elements::getSerializationFormatName;

namespace po = boost::program_options;

// =================================================================================================

namespace ppp::types {

// _________________________________________________________________________________________________
void validate(boost::any& v, const vector<string>& vals, SerializationFormat* f, int) {  // NOLINT
  // Make sure no previous assignment to the option was made.
  po::validators::check_first_occurrence(v);

  // Extract the first value from 'vals'.
  // NOTE: If there is more than one token, it's an error, and an exception will be thrown.
  const string& token = po::validators::get_single_string(vals);

  for (const auto& entry : SERIALIZERS) {
    if (token == getSerializationFormatName(entry.first)) {
      v = entry.first;
      return;
    }
  }

  throw po::invalid_option_value(token);
}

// _________________________________________________________________________________________________
void validate(boost::any& v, const vector<string>& vals, SemanticRole* r, int) {  // NOLINT
  // Make sure no previous assignment to the option was made.
  po::validators::check_first_occurrence(v);

  // Extract the first value from 'vals'.
  // NOTE: If there is more than one token, it's an error, and an exception will be thrown.
  const string& token = po::validators::get_single_string(vals);

  for (size_t i = 0; i < SEMANTIC_ROLE_NAMES.size(); i++) {
    if (token == SEMANTIC_ROLE_NAMES[i]) {
      v = SemanticRole(i);
      return;
    }
  }

  throw po::invalid_option_value(token);
}

// _________________________________________________________________________________________________
void validate(boost::any& v, const vector<string>& vals, PdfElementType* u, int) {  // NOLINT
  // Make sure no previous assignment to the option was made.
  po::validators::check_first_occurrence(v);

  // Extract the first value from 'vals'.
  // NOTE: If there is more than one token, it's an error, and an exception will be thrown.
  const string& token = po::validators::get_single_string(vals);

  for (size_t i = 0; i < PDF_ELEMENT_TYPE_NAMES.size(); i++) {
    if (token == PDF_ELEMENT_TYPE_NAMES[i]) {
      v = PdfElementType(i);
      return;
    }
  }

  throw po::invalid_option_value(token);
}

}  // namespace ppp::types
