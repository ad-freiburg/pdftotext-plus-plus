/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef SERIALIZERS_JSONLSERIALIZER_H_
#define SERIALIZERS_JSONLSERIALIZER_H_

#include <iostream>  // std::ostream
#include <string>
#include <vector>

#include "../PdfDocument.h"

using std::ostream;
using std::string;
using std::vector;

// =================================================================================================

/**
 * This class is responsible for writing the (text) elements extracted from a PDF to a specified
 * file in JSONL format.
 */
class JsonlSerializer {
 public:
  /**
   * This constructor creates and initializes a new instance of this class.
   *
   * @param doc
   *   The document to process.
   * @param serializePages
   *   Whether or not this serializer should write the information about the pages to the output
   *   file (one line per page, each providing, for example, the width and height of the page).
   * @param serializeChars
   *   Whether or not this serializer should write the information about the characters to the
   *   output file (one line per character, each providing, for example, the position, the text,
   *   the font name, or the font size of the character).
   * @param serializeFigures
   *   Whether or not this serializer should write the information about the figures to the output
   *   file (one line per figure, each providing, for example, the position of the figure).
   * @param serializeShapes
   *   Whether or not this serializer should write the information about the shapes to the output
   *   file (one line per shape, each providing, for example, the position of the shape).
   * @param serializeWords
   *   Whether or not this serializer should write the information about the words to the output
   *   file (one line per word, each providing, for example, the position, the text, the most
   *   frequent font name and font size of the word).
   * @param serializeTextBlocks
   *   Whether or not this serializer should write the information about the text blocks to the
   *   output file (one line per block, each providing, for example, the position, the text, the
   *   most frequent font name and font size of the block).
   */
  JsonlSerializer(const PdfDocument* doc, bool serializePages, bool serializeChars,
      bool serializeFigures, bool serializeShapes, bool serializeWords, bool serializeTextBlocks);

  /** The deconstructor. */
  ~JsonlSerializer();

  /**
   * This method writes the (text) elements extracted from the given PDF document to the file
   * specified by `targetPath`. If the target path is specified as "-", the elements are written to
   * the console instead. The output will contain one line per element, each in JSONL format. The
   * exact format of a single line depends on the element type and is described in the comment of
   * the respective serialize* method below.
   *
   * @param targetPath
   *   The path to the file to which the elements should be written. If specified as "-", the JSONL
   *   will be printed to std::cout instead.
   */
  void serialize(const string& targetPath) const;

 private:
  /**
   * This method writes the elements extracted from the given PDF document to the given stream
   * (which could be a file output stream, if the elements should be written to a file, or
   * std::cout, if the elements should be written to the console).
   *
   * @param out
   *    The stream to which the elements should be written.
   */
  void serializeToStream(ostream& out) const;

  /**
   * This method writes the information about the given page in the following format to the given
   * output stream:
   *
   * { "type": "page", "num": 1, "width": 120.1, "height": 345.2 }
   *
   *  The file will contain one line per element, each in the following JSON format:
   *
   * @param page
   *    The page to write to the stream.
   * @param stream
   *    The output stream.
   */
  void serializePage(const PdfPage* page, ostream& stream) const;

  /**
   * This method writes the information about the given characters to the given output stream.
   * For each character, a line in the following format will be written:
   *
   * {"type": "char", "id\": "abc", "rank": 1, "page": 2, "minX": 12.1, "minY": 54.1,
   *    "maxX": 432.4, "maxY": 125.2, "wMode": 0, "rotation": 0, "font": "arial",
   *    "fontSize": 12.0, "weight": 100, "italic": true, "type-3": false,
   *    "color": [1, 1, 1], opacity": 1, "text": "x", "origin": "pdftotext++" }
   *
   * @param characters
   *    The characters to write to the stream.
   * @param stream
   *    The output stream.
   */
  void serializeChars(const vector<PdfCharacter*>& chars, ostream& stream) const;

  /**
   * This method writes the information about the given figures to the given output stream.
   * For each figure, a line in the following format will be written:
   *
   * {"type": "figure", "id\": "abc", "rank": 1, "page": 2, "minX": 12.1, "minY": 54.1,
   *    "maxX": 432.4, "maxY": 125.2, "origin": "pdftotext++" }
   *
   * @param figures
   *    The figures to write to the stream.
   * @param stream
   *    The output stream.
   */
  void serializeFigures(const vector<PdfFigure*>& figs, ostream& stream) const;

  /**
   * This method writes the information about the given shapes to the given output stream.
   * For each shape, a line in the following format will be written:
   *
   * {"type": "shape", "id\": "abc", "rank": 1, "page": 2, "minX": 12.1, "minY": 54.1,
   *    "maxX": 432.4, "maxY": 125.2, "origin": "pdftotext++" }
   *
   * @param shapes
   *    The shapes to write to the stream.
   * @param stream
   *    The output stream.
   */
  void serializeShapes(const vector<PdfShape*>& shapes, ostream& stream) const;

  /**
   * This method writes the information about the given words to the given output stream.
   * For each word, a line in the following format will be written:
   *
   * {"type": "word", "id\": "abc", "rank": 1, "page": 2, "minX": 12.1, "minY": 54.1,
   *    "maxX": 432.4, "maxY": 125.2, "font": "arial", "fontSize": 12.0, "text": "xyz",
   *    "origin": "pdftotext++" }
   *
   * @param words
   *    The words to write to the stream.
   * @param stream
   *    The output stream.
   */
  void serializeWords(const vector<PdfWord*>& words, ostream& stream) const;

  /**
   * This method writes the information about the given text blocks to the given output stream.
   * For each text block, a line in the following format will be written:
   *
   * {"type": "block", "id\": "abc", "rank": 1, "page": 2, "minX": 12.1, "minY": 54.1,
   *    "maxX": 432.4, "maxY": 125.2, "font": "arial", "fontSize": 12.0, "text": "xyz",
   *    "origin": "pdftotext++" }
   *
   *
   * @param blocks
   *    The text blocks to write to the stream.
   * @param stream
   *    The output stream.
   */
  void serializeTextBlocks(const vector<PdfTextBlock*>& blocks, ostream& stream) const;

  // The document to process.
  const PdfDocument* _doc;

  // Whether or not to serialize the pages.
  bool _serializePages;

  // Whether or not to serialize the characters.
  bool _serializeChars;

  // Whether or not to serialize the figures.
  bool _serializeFigures;

  // Whether or not to serialize the shapes.
  bool _serializeShapes;

  // Whether or not to serialize the words.
  bool _serializeWords;

  // Whether or not to serialize the text blocks.
  bool _serializeTextBlocks;
};

#endif  // SERIALIZERS_JSONLSERIALIZER_H_
