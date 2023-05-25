/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef SERIALIZATION_TEXTSERIALIZER_H_
#define SERIALIZATION_TEXTSERIALIZER_H_

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

/**
 * This class writes the text extracted from a PDF to a given file or stdout. The format is one
 * text block per line, with the text block separated by blank lines.
 */
class TextSerializer : public Serializer {
 public:
  /** The default deconstructor. */
  TextSerializer();

  /** The deconstructor. */
  ~TextSerializer();

  /**
   * This method writes the text extracted from the given PDF document to the file given by
   * `targetPath`. If `targetPath` is specified as "-", the text is written to stdout instead.
   *
   * @param doc
   *   The PDF document to process.
   * @param roles
   *   If not empty, only the text of text blocks with the specified roles is written to the file.
   *   If empty, all text is written to the file.
   * @param targetPath
   *   The path to the file to which the text should be written. If specified as "-", the text is
   *   written to stdout instead.
   */
  void serialize(PdfDocument* doc, const unordered_set<string>& roles, const string& targetPath);

 private:
  /**
   * This method writes the text extracted from the given PDF file to the given stream.
   *
   * @param doc
   *   The PDF document to process.
   * @param roles
   *   If not empty, only the text of text blocks with the specified roles is written to the stream.
   *   If empty, all text is written to the stream.
   * @param out
   *   The stream to which the text should be written.
   */
  void serializeToStream(PdfDocument* doc, const unordered_set<string>& roles, ostream& out);

  // Whether or not to prepend each emphasized text block with "^A" (start of heading) and mark
  // each page break with "^L" (form feed).
  bool _addControlCharacters;

  // Whether or not to prepend each text block with its semantic role.
  bool _addSemanticRoles;

  // Whether or not sub- and superscripts should be serialized.
  bool _excludeSubSuperscripts;
};

#endif  // SERIALIZATION_TEXTSERIALIZER_H_
