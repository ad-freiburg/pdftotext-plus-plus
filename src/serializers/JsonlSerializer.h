/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef SERIALIZERS_JSONLSERIALIZER_H_
#define SERIALIZERS_JSONLSERIALIZER_H_

#include <string>

#include <poppler/PDFDoc.h>

#include "../PdfDocument.h"


/**
 * This class writes the elements extracted from a PDF to a specified file. The file will
 * contain one line per element, each in the following JSON format:
 *
 * { "id": "14c3x", "rank": 12, "page": 2, "leftX": 12.4, "minY": 42.1, "rightX": 64.1,
 *   "font": "Arial", "fontSize": 12, "text": "Hello ...", "word" "p2322" }
 *
 *  The property "word" provides the id of the parent word of a character.
 */
class JsonlSerializer {
 public:
  /**
   * This constructor creates and initializes a new `JsonlSerializer`.
   *
   * @param doc
   *   The document to process.
   */
  explicit JsonlSerializer(PdfDocument* doc, bool serializePages, bool serializeChars,
      bool serializeFigures, bool serializeShapes, bool serializeWords, bool serializeTextBlocks);

  /** The deconstructor. */
  ~JsonlSerializer();

  /**
   * This method writes the elements extracted from the given document to the file given by
   * `targetPath`. The elements are written in JSONL format, see the comment at the beginning of
   * the file for details about the exact format.
   *
   * @param targetPath
   *   The path to the file to which the elements should be written.
   */
  void serialize(const std::string& targetPath);

 private:
  /**
   * This method writes the elements extracted from the given document to the given stream.
   *
   * @param out
   *    The stream to which the elements should be written.
   */
  void serializeToStream(std::ostream& out);

  void serializePage(const PdfPage* page, std::ostream& stream);

  void serializeChars(const std::vector<PdfCharacter*>& chars, std::ostream& stream);

  void serializeFigures(const std::vector<PdfFigure*>& figs, std::ostream& stream);

  void serializeShapes(const std::vector<PdfShape*>& shapes, std::ostream& stream);

  void serializeWords(const std::vector<PdfWord*>& words, std::ostream& stream);

  void serializeTextBlocks(const std::vector<PdfTextBlock*>& blocks, std::ostream& stream);

  /** The document to process. **/
  PdfDocument* _doc;

  /** Whether to serialize the pages. **/
  bool _serializePages;

  /** Whether to serialize the characters. **/
  bool _serializeChars;

  /** Whether to serialize the figures. **/
  bool _serializeFigures;

  /** Whether to serialize the shapes. **/
  bool _serializeShapes;

  /** Whether to serialize the words. **/
  bool _serializeWords;

  /** Whether to serialize the text blocks. **/
  bool _serializeTextBlocks;
};

#endif  // SERIALIZERS_JSONLSERIALIZER_H_
