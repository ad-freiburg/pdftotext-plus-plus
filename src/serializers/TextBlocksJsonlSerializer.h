/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef SERIALIZERS_TEXTBLOCKSJSONLSERIALIZER_H_
#define SERIALIZERS_TEXTBLOCKSJSONLSERIALIZER_H_

#include <string>

#include <poppler/PDFDoc.h>

#include "../PdfDocument.h"


/**
 * This class writes the text blocks extracted from a PDF to a specified file. The file will
 * contain one line per text block, each in the following JSON format:
 *
 * { "id": "14c3x", "rank": 12, "page": 2, "minX": 12.4, "minY": 42.1, "maxX": 64.1,
 *   "font": "Arial", "fontSize": 12, "text": "Hello ...", "role": "paragraph" }
 * ...
 */
class TextBlocksJsonlSerializer {
 public:
  /**
   * This constructor creates and initializes a new `TextBlocksJsonlSerializer`.
   *
   * @param doc
   *   The document to process.
   */
  explicit TextBlocksJsonlSerializer(PdfDocument* doc);

  /** The deconstructor. */
  ~TextBlocksJsonlSerializer();

  /**
   * This method writes the text blocks extracted from the given document to the file given by
   * `targetPath`. The text blocks are written in JSONL format, see the comment at the beginning of
   * the file for details about the exact format.
   *
   * @param targetPath
   *   The path to the file to which the text blocks should be written.
   */
  void serialize(const std::string& targetPath);

 private:
  /**
   * This method writes the text blocks extracted from the given document to the given stream.
   *
   * @param out
   *    The stream to which the text blocks should be written.
   */
  void serializeToStream(std::ostream& out);

  /** The document to process. **/
  PdfDocument* _doc;
};

#endif  // SERIALIZERS_TEXTBLOCKSJSONLSERIALIZER_H_
