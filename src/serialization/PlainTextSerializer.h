/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef SERIALIZATION_PLAINTEXTSERIALIZER_H_
#define SERIALIZATION_PLAINTEXTSERIALIZER_H_

#include <ostream>
#include <string>
#include <unordered_set>

#include "../PdfDocument.h"
#include "../Types.h"
#include "./Serializer.h"

using ppp::types::SemanticRole;
using std::ostream;
using std::string;
using std::unordered_set;

// =================================================================================================

namespace ppp::serialization {

/**
 * This class outputs the text extracted from a PDF in plain text and writes it to a given file or
 * stdout. The format is one text block per line, with the text blocks separated by blank lines.
 */
class PlainTextSerializer : public Serializer {
 public:
  /** The default constructor. */
  PlainTextSerializer();

  /** The deconstructor. */
  ~PlainTextSerializer();

 protected:
  /**
   * This method outputs the text extracted from the given PDF document and writes it to the given
   * stream.
   *
   * @param doc
   *   The PDF document to process.
   * @param roles
   *   If not empty, only the text of text blocks with the specified roles is written to the file.
   *   If empty, the text of *all* text blocks is written to the file.
   * @param units
   *   If not empty, semantic and layout information about (and the text of) the specified units is
   *   output. If empty, the information and text of *all* text units is written to the file.
   * @param out
   *   The stream to which the text should be written.
   */
  void serializeToStream(const PdfDocument* doc, const unordered_set<SemanticRole>& roles,
      const unordered_set<DocumentUnit>& units, ostream& out) const;
};

}  // namespace ppp::serialization

#endif  // SERIALIZATION_PLAINTEXTSERIALIZER_H_
