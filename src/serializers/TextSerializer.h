/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef SERIALIZERS_TEXTSERIALIZER_H_
#define SERIALIZERS_TEXTSERIALIZER_H_

#include <string>

#include "../PdfDocument.h"

using std::ostream;
using std::string;

// =================================================================================================

/**
 * This class writes the text extracted from a PDF to a given file or stdout. The format is one
 * text block per line, with the text block separated by blank lines.
 */
class TextSerializer {
 public:
  /**
   * This constructor creates and initializes a new instance of this class.
   *
   * @param doc
   *   The document to process.
   * @param addControlCharacters
   *   Whether or not to add the following control characters to the text:
   *     "^A" (start of heading) in front of each emphasized text block
   *     "^L" (form feed) at each page break.
   * @param addSemanticRoles
   *   Whether or not to prepend each text block with its semantic role.
   * @param excludeSubSuperscripts
   *   Whether or not sub- and subperscripts should be written to the output.
   */
  TextSerializer(PdfDocument* doc, bool addControlCharacters, bool addSemanticRoles,
    bool excludeSubSuperscripts);

  /** The deconstructor. */
  ~TextSerializer();

  /**
   * This method writes the text extracted from the given PDF document to the file given by
   * `targetPath`. If `targetPath` is specified as "-", the text is written to stdout instead.
   *
   * @param targetPath
   *   The path to the file to which the text should be written. If specified as "-", the text is
   *   written to stdout instead.
   */
  void serialize(const string& targetPath);

 private:
  /**
   * This method writes the text extracted from a PDF file to the given stream.
   *
   * @param out The stream to which the text should be written.
   */
  void serializeToStream(ostream& out);

  // The document to process.
  PdfDocument* _doc;

  // Whether or not to prepend each emphasized text block with "^A" (start of heading) and mark
  // each page break with "^L" (form feed).
  bool _addControlCharacters;

  // Whether or not to prepend each text block with its semantic role.
  bool _addSemanticRoles;

  // Whether or not sub- and superscripts should be serialized.
  bool _excludeSubSuperscripts;
};

#endif  // SERIALIZERS_TEXTSERIALIZER_H_
