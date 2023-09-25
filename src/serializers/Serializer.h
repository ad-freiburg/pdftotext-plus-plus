/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef SERIALIZERS_SERIALIZER_H_
#define SERIALIZERS_SERIALIZER_H_

#include <ostream>
#include <string>
#include <unordered_set>

#include "../Types.h"

using std::ostream;
using std::string;
using std::unordered_set;

using ppp::types::PdfElementType;
using ppp::types::PdfDocument;
using ppp::types::SemanticRole;

// =================================================================================================

namespace ppp::serialization {

/**
 * This class is the super class for all serializer classes (each of which is responsible for
 * writing the text extracted from a PDF to a specified file (or to stdout) in a specified format.
 */
class Serializer {
 public:
  /**
   * This method writes the text extracted from the given PDF document to the file given by
   * `targetPath`. If `targetPath` is specified as "-", the text is written to stdout instead.
   *
   * @param doc
   *   The PDF document to process.
   * @param roles
   *   If not empty, only the text of text blocks with the specified roles is written to the file.
   *   If empty, the text of *all* text blocks is written to the file.
   * @param units
   *   If not empty, semantic and layout information about (and the text of) the specified units is
   *   output. If empty, the information and text of *all* text units is written to the file.
   * @param targetPath
   *   The path to the file to which the text should be written.
   *   NOTE: If specified as "-", the text is written to stdout.
   */
  void serialize(const PdfDocument* doc, const unordered_set<SemanticRole>& roles,
      const unordered_set<PdfElementType>& units, const string& targetPath) const;

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
  virtual void serializeToStream(const PdfDocument* doc, const unordered_set<SemanticRole>& roles,
      const unordered_set<PdfElementType>& units, ostream& out) const = 0;
};

}  // namespace ppp::serialization

#endif  // SERIALIZERS_SERIALIZER_H_
