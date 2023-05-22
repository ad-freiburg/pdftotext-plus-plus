/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef SERIALIZATION_FORMAT_H_
#define SERIALIZATION_FORMAT_H_

#include <string>
#include <vector>

#include <boost/program_options.hpp>

// =================================================================================================

namespace Serialization {

// The formats into which text extracted from a PDF file can be serialized.
enum class SerializationFormat {
  TXT,
  XML,
  JSON
};

// A mapping of names to serialization formats.
const std::unordered_map<std::string, Serialization::SerializationFormat> NAMES_FORMAT_MAP = {
  { "txt", Serialization::SerializationFormat::TXT },
  { "xml", Serialization::SerializationFormat::XML },
  { "json", Serialization::SerializationFormat::JSON }
};

/**
 * This function returns a string containing all serialization formats separated from each other by
 * a comma.
 *
 * @return
 *    A string containing all serialization formats.
 */
std::string getSerializationFormatChoicesStr();

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
void validate(boost::any& v, const std::vector<std::string>& values,
    Serialization::SerializationFormat* format, int);

} // namespace Serialization

#endif  // SERIALIZATION_FORMAT_H_
