/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

using std::string;
using std::tuple;
using std::unordered_map;
using std::vector;

// =================================================================================================

namespace ppp::types {

// -------------------------------------------------------------------------------------------------
// PdfElementType

// The different types of elements of which a PDF document can consist.
enum struct PdfElementType {
  PAGES,
  TEXT_BLOCKS,
  TEXT_LINES,
  WORDS,
  CHARACTERS,
  FIGURES,
  SHAPES
};

// The names of the PDF element types.
const vector<string> PDF_ELEMENT_TYPE_NAMES {
  "pages",
  "blocks",
  "lines",
  "words",
  "characters",
  "figures",
  "shapes"
};

// -------------------------------------------------------------------------------------------------
// SemanticRole

// The available semantic roles.
enum struct SemanticRole {
  PARAGRAPH,
  REFERENCE,
  MARGINAL,
  FOOTNOTE,
  HEADING,
  FORMULA,
  TITLE,
  AUTHOR_INFO,
  ABSTRACT,
  DATE,
  CAPTION,
  TABLE,
  OTHER,
  TOC
};

// The names of the semantic roles.
const vector<string> SEMANTIC_ROLE_NAMES {
  "paragraph",
  "reference",
  "marginal",
  "footnote",
  "heading",
  "formula",
  "title",
  "author-info",
  "abstract",
  "date",
  "caption",
  "table",
  "other",
  "toc",
};

// -------------------------------------------------------------------------------------------------

/**
 * This class represents a font used in a PDF document.
 */
class PdfFontInfo {
 public:
  /**
   * The ascent of the font, that is: the maximum extent above the base line.
   */
  double ascent = 0.0;

  /**
   * The descent of the font, that is: the maximum extent below the base line.
   */
  double descent = 0.0;

  /**
   * The font name as it is provided by PDF, for example: "LTSL+Nimbus12-Bold".
   */
  string fontName;

  /**
   * The normalized font name, that is: the original font name translated to lower cases and
   * without the prefix ending with "+", for example: "nimbus12-bold".
   */
  string normFontName;

  /**
   * The font base name, that is: the normalized font name without the suffix starting with "-"
   * and without digits, for example: nimbus.
   */
  string fontBaseName;

  /**
   * The boolean flag indicating whether or not this font is a fixed width font.
   */
  bool isFixedWidth = false;

  /**
   * The boolean flag indicating whether or not the font is a serif font.
   */
  bool isSerif = false;

  /**
   * The boolean flag indicating whether or not the font is a symbolic font.
   */
  bool isSymbolic = false;

  /**
   * The boolean flag indicating whether or not the font is an italic font.
   */
  bool isItalic = false;

  /**
   * The font weight, that is: one of the values [100, 200, ..., 900]. It describes the boldness of
   * the font's characters. Here is the meaning of the different values:
   *
   * 100: Extra Light or Ultra Light
   * 200: Light or Thin
   * 300: Book or Demi
   * 400: Normal or Regular
   * 500: Medium
   * 600: Semibold, Demibold
   * 700: Bold
   * 800: Black, Extra Bold or Heavy
   * 900: Extra Black, Fat, Poster or Ultra Blacktrue.
   */
  int weight = 400;

  /**
   * The boolean flag indicating whether or not the font is a type-3 font.
   */
  bool isType3 = false;

  /** The font matrix. */
  double fontMatrix[6];

  /**
   * A mapping of character names to bounding boxes (each given by its leftX, upperY, rightX,
   * lowerY coordinates in character space).
   */
  unordered_map<string, tuple<double, double, double, double>> glyphBoundingBoxes;
};

// -------------------------------------------------------------------------------------------------
// Timing.

/**
 * A struct for storing the runtime required to complete a particular action or method.
 */
struct Timing {
  /**
   * This constructor creates and initializes a new instance of this struct.
   *
   * @param nameA
   *    A (short) name describing the action/method.
   * @param timeA
   *    The runtime required to complete the action/method.
   */
  Timing(const string& nameA, int64_t timeA) {
    name = nameA;
    time = timeA;
  }

  // The name of the action/method.
  string name;
  // The runtime required to complete the action/method.
  int64_t time = 0;
};

// -------------------------------------------------------------------------------------------------
// SerializationFormat

// The formats into which pdftotext++ is able to serialize the text extracted from a PDF document.
enum struct SerializationFormat {
  TXT,
  TXT_EXTENDED,
  JSONL
};

// The names of the serialization formats.
const vector<string> SERIALIZATION_FORMAT_NAMES {
  "txt",
  "txt-extended",
  "jsonl"
};

}  // namespace ppp::types

#endif  // TYPES_H_
