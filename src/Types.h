/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <boost/program_options.hpp>

#include <string>
#include <vector>

namespace ppp::types {

// The formats into which pdftotext++ allows to serialize the text extracted from a PDF file.
enum struct SerializationFormat { TXT, XML, JSON };

// The names of the serialization formats.
const std::vector<std::string> SERIALIZATION_FORMAT_NAMES { "txt", "xml", "json" };

/**
 * This function validates the value specified by the user via the command-line to choose a
 * serialization format (e.g., by typing "--format <value>") and returns the serialization format
 * associated with this value in NAMES_FORMAT_MAP. Throws a validation error when there is no
 * serialization format associated with the given value.
 *
 * NOTE: This method allows to write something like "( 'format',
 * po::value<SerializationFormat>(&format)" when defining the command-line options, see
 * pdftotext++.cpp for an example.
 */
void validate(boost::any& v, const std::vector<std::string>& values,  // NOLINT
    ppp::types::SerializationFormat* format, int);

}  // namespace ppp::types

// =================================================================================================

#endif  // TYPES_H_
