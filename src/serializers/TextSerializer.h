/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef SERIALIZERS_TEXTSERIALIZER_H_
#define SERIALIZERS_TEXTSERIALIZER_H_

#include <string>

#include <poppler/PDFDoc.h>

#include "../PdfDocument.h"


/**
 * This class writes the text extracted from a PDF to a given file or stdout. The format is one
 * text block per line, with the lines separated by blank lines.
 */
class TextSerializer {
 public:
  /**
   * This constructor creates and initializes a new `TextSerializer`.
   *
   * @param doc
   *   The document to process.
   */
  TextSerializer(PdfDocument* doc, bool addControlCharacters, bool addSemanticRoles,
    bool excludeSubSuperscripts);

  /** The deconstructor. */
  ~TextSerializer();

  /**
   * This method writes the text extracted from the given document to the file given by
   * `targetPath`. If `targetPath` is not specified, the text is written to stdout.
   *
   * @param targetPath
   *   The path to the file to which the text should be written.
   */
  void serialize(const std::string& targetPath = nullptr);

 private:
  /**
   * This method writes the text extracted from a PDF file to the given stream.
   *
   * @param out The stream to which the text should be written.
   */
  void serializeToStream(std::ostream& out);

  /** The document to process. **/
  PdfDocument* _doc;

  /**
   * Whether or not to prepend each emphasized text block with "^A" (start of heading) and mark
   * each page break with "^L" (form feed).
   */
  bool _addControlCharacters;

  /**
   * Whether or not to prepend each emphasized text block with its semantic role.
   */
  bool _addSemanticRoles;

  /** Whether or not sub- and superscripts should be serialized. **/
  bool _excludeSubSuperscripts;
};

#endif  // SERIALIZERS_TEXTSERIALIZER_H_
