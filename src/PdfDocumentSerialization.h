/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef PDFDOCUMENTSERIALIZATION_H_
#define PDFDOCUMENTSERIALIZATION_H_

#include <string>
#include <unordered_map>

#include "./Types.h"
#include "./serializers/JsonlSerializer.h"
#include "./serializers/PlainTextSerializer.h"
#include "./serializers/PlainTextExtendedSerializer.h"
#include "./serializers/Serializer.h"

using std::string;
using std::unordered_map;

using ppp::types::SerializationFormat;

// =================================================================================================

namespace ppp::serialization {

// The mapping of serialization formats to serializers.
const unordered_map<SerializationFormat, Serializer*> SERIALIZERS {
  { SerializationFormat::TXT, new PlainTextSerializer() },
  { SerializationFormat::TXT_EXTENDED, new PlainTextExtendedSerializer() },
  // { SerializationFormat::JSONL, new JsonlSerializer() }
};

/**
 * This method creates a string containing all formats into which pdftotext++ allows to serialize
 * text extracted from a PDF. The serialization formats are separated from each other by commas.
 *
 * @return
 *     A string containing all formats into which pdftotext++ allows to serialize text extracted
 *     from a PDF.
 */
string getSerializationFormatChoicesStr();

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

}  // namespace ppp::serialization

#endif  // PDFDOCUMENTSERIALIZATION_H_
