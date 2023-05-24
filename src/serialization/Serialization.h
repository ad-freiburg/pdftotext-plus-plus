/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef SERIALIZATION_SERIALIZATION_H_
#define SERIALIZATION_SERIALIZATION_H_

#include <boost/program_options.hpp>

#include <string>
#include <unordered_map>
#include <vector>

#include "./Serializer.h"
#include "./TextSerializer.h"

using std::string;
using std::unordered_map;
using std::vector;

namespace ppp::serialization {

// The available serialization formats.
enum struct SerializationFormat { TXT, JSONL };

// The names of the serialization formats.
const vector<string> SERIALIZATION_FORMAT_NAMES { "txt", "jsonl" };

// The mapping of serialization formats to serializers.
const unordered_map<SerializationFormat, Serializer*> SERIALIZERS {
  { SerializationFormat::TXT, new TextSerializer() }
};

/**
 * This method creates a string containing all formats into which pdftotext++ allows to serialize
 * text extracted from a PDF file. The serialization formats are separated from each other by
 * commas.
 *
 * @return
 *     A string containing all formats into which pdftotext++ allows to serialize text extracted
 *     from a PDF file.
 */
std::string getSerializationFormatChoicesStr();

/**
 * This function returns the name of the given serialization format.
 *
 * @param format
 *    The serialization format.
 *
 * @return
 *    The name of the serialization format.
 */
string getName(SerializationFormat format);

/**
 * This function returns the serializer associated with the given serialization format.
 *
 * @param format
 *    The serialization format.
 *
 * @return
 *    The serializer associated with the given serialization format.
 */
Serializer* getSerializer(SerializationFormat format);

/**
 * This function validates the value specified by the user via the command-line to choose a
 * serialization format (e.g., by typing "--format <value>") and returns the serialization format
 * associated with this value. Throws a validation error when there is no serialization format
 * associated with the given value.
 *
 * NOTE: This method allows to write something like "( 'format',
 * po::value<SerializationFormat>(&format)" when defining the command-line options, see
 * pdftotext++.cpp for an example.
 */
void validate(boost::any& v, const vector<string>& values, SerializationFormat* format, int);  // NOLINT

}  // namespace ppp::serialization

#endif  // SERIALIZATION_SERIALIZATION_H_
