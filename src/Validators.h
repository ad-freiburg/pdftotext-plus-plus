/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef VALIDATORS_H_
#define VALIDATORS_H_

#include <boost/program_options.hpp>

#include <string>
#include <vector>

#include "./Types.h"

using std::string;
using std::vector;

namespace ppp::types {

/**
 * This function validates the value specified by the user via the command-line to choose a
 * serialization format (e.g., by typing "--format <value>") and returns the serialization format
 * associated with this value.
 *
 * Throws a validation error when there is no serialization format associated with the given value.
 *
 * NOTE: This method allows to write something like "( 'format',
 * po::value<SerializationFormat>(&format)" when defining the command-line options, see
 * pdftotext++.cpp for an example.
 */
void validate(boost::any& v, const vector<string>& values, SerializationFormat* f, int);  // NOLINT

/**
 * This function validates the value specified by the user via the command-line to choose a
 * serialization format (e.g., by typing "--format <value>") and returns the serialization format
 * associated with this value.
 *
 * Throws a validation error when there is no serialization format associated with the given value.
 *
 * NOTE: This method allows to write something like "( 'format',
 * po::value<SerializationFormat>(&format)" when defining the command-line options, see
 * pdftotext++.cpp for an example.
 */
void validate(boost::any& v, const vector<string>& values, SemanticRole* r, int);  // NOLINT

}  // namespace ppp::types

#endif  // VALIDATORS_H_
