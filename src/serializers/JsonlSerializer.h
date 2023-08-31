/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef SERIALIZERS_JSONLSERIALIZER_H_
#define SERIALIZERS_JSONLSERIALIZER_H_

#include <ostream>
#include <unordered_set>

#include "./Serializer.h"
#include "../Types.h"

using std::ostream;
using std::unordered_set;

using ppp::types::PdfDocument;
using ppp::types::SemanticRole;

// =================================================================================================

namespace ppp::serialization {

/**
 * This class outputs the text extracted from a PDF in JSONL format and writes it to a given file
 * or stdout. The output contains one line per document unit, each of which represents valid JSON
 * of its own. How the JSON actually look like is specific to the respective document unit and is
 * described in the comment of the respective serialize* method below.
 */
class JsonlSerializer : public Serializer {
 public:
  /**
   * The default constructor.
   *
   * @param coordsPrecision
   *    The precision with which this serializer should output the coordinates.
   */
  explicit JsonlSerializer(size_t coordsPrecision);

  /** The deconstructor. */
  ~JsonlSerializer();

 protected:
  /**
   * This method outputs the text extracted from the given PDF document in JSONL format and writes
   * it to the given stream.
   *
   * @param doc
   *   The PDF document to process.
   * @param roles
   *   Output only entries for text that is part of a text block whose roles is in this set.
   * @param units
   *   Output only entries which relate to one of the document units in this unordered_set.
   * @param out
   *   The stream to which the entries should be written.
   */
  void serializeToStream(const PdfDocument* doc, const unordered_set<SemanticRole>& roles,
      const unordered_set<PdfElementType>& units, ostream& out) const;

  /**
   * This method writes the information about the pages of the given PDF document to the given
   * stream. For each page, a line in the following format is written:
   *
   * { "type": "page", "num": 1, "width": 120.1, "height": 345.2 }
   *
   * @param page
   *    The page to write to the stream.
   * @param roles
   *   Output only entries for text that is part of a text block whose roles is in this set.
   * @param stream
   *    The output stream.
   */
  void serializePages(const PdfDocument* pages, const unordered_set<SemanticRole>& roles,
      ostream& stream) const;

  /**
   * This method writes the information about the characters of the given PDF document to the given
   * stream. For each character, a line in the following format is written:
   *
   * {"type": "char", "id": "abc", "rank": 1, "page": 2, "minX": 12.1, "minY": 54.1,
   *    "maxX": 432.4, "maxY": 125.2, "wMode": 0, "rotation": 0, "font": "arial",
   *    "fontSize": 12.0, "weight": 100, "italic": true, "type-3": false,
   *    "color": [1, 1, 1], opacity": 1, "text": "x", "origin": "pdftotext++" }
   *
   * @param doc
   *    The PDF document to process.
   * @param roles
   *   Output only entries for text that is part of a text block whose roles is in this set.
   * @param stream
   *    The output stream.
   */
  void serializeCharacters(const PdfDocument* doc, const unordered_set<SemanticRole>& roles,
      ostream& stream) const;

  /**
   * This method writes the information about the figures of the given PDF document to the given
   * stream. For each figure, a line in the following format is written:
   *
   * {"type": "figure", "id": "abc", "rank": 1, "page": 2, "minX": 12.1, "minY": 54.1,
   *    "maxX": 432.4, "maxY": 125.2, "origin": "pdftotext++" }
   *
   * @param doc
   *    The PDF document to process.
   * @param roles
   *   Output only entries for text that is part of a text block whose roles is in this set.
   * @param stream
   *    The output stream.
   */
  void serializeFigures(const PdfDocument* doc, const unordered_set<SemanticRole>& roles,
      ostream& stream) const;

  /**
   * This method writes the information about the shapes of the given PDF document to the given
   * stream. For each character, a line in the following format is written:
   *
   * {"type": "shape", "id\": "abc", "rank": 1, "page": 2, "minX": 12.1, "minY": 54.1,
   *    "maxX": 432.4, "maxY": 125.2, "origin": "pdftotext++" }
   *
   * @param doc
   *    The PDF document to process.
   * @param roles
   *   Output only entries for text that is part of a text block whose roles is in this set.
   * @param stream
   *    The output stream.
   */
  void serializeShapes(const PdfDocument* doc, const unordered_set<SemanticRole>& roles,
      ostream& stream) const;

  /**
   * This method writes the information about the words of the given PDF document to the given
   * stream. For each word, a line in the following format is written:
   *
   * {"type": "word", "id\": "abc", "rank": 1, "page": 2, "minX": 12.1, "minY": 54.1,
   *    "maxX": 432.4, "maxY": 125.2, "font": "arial", "fontSize": 12.0, "text": "xyz",
   *    "origin": "pdftotext++" }
   *
   * @param doc
   *    The PDF document to process.
   * @param roles
   *   Output only entries for text that is part of a text block whose roles is in this set.
   * @param stream
   *    The output stream.
   */
  void serializeWords(const PdfDocument* doc, const unordered_set<SemanticRole>& roles,
      ostream& stream) const;

  /**
   * This method writes the information about the text blocks of the given PDF document to the
   * given stream. For each block, a line in the following format is written:
   *
   * {"type": "block", "id\": "abc", "rank": 1, "page": 2, "minX": 12.1, "minY": 54.1,
   *    "maxX": 432.4, "maxY": 125.2, "font": "arial", "fontSize": 12.0, "text": "xyz",
   *    "origin": "pdftotext++" }
   *
   * @param doc
   *    The PDF document to process.
   * @param roles
   *   Output only entries for text that is part of a text block whose roles is in this set.
   * @param stream
   *    The output stream.
   */
  void serializeTextBlocks(const PdfDocument* doc, const unordered_set<SemanticRole>& roles,
      ostream& stream) const;

 private:
  // The precision with which this serializer should output the coordinates.
  size_t _coordsPrecision;
};

}  // namespace ppp::serialization

#endif  // SERIALIZERS_JSONLSERIALIZER_H_

