/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef PDFFONTINFO_H_
#define PDFFONTINFO_H_

#include <poppler/GfxState.h>
#include <poppler/Object.h>  // Ref
#include <poppler/XRef.h>

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

using std::string;
using std::tuple;
using std::unordered_map;
using std::vector;

// =================================================================================================

class Type1FontFileParser;

/**
 * This class represents a font in a PDF document.
 */
class PdfFontInfo {
 public:
  /** This constructor creates and initializes a new `PdfFontInfo` object. */
  PdfFontInfo();

  /** The deconstructor. */
  ~PdfFontInfo();

  /**
   * This method creates a new `PdfFontInfo` object from the given graphics state and xref table.
   *
   * @param state
   *    The current graphics state.
   * @param xref
   *    The XRef table of the current page.
   * @param parseEmbeddedFontFiles
   *    A boolean flag indicating whether or not to parse the embedded font files for more accurate
   *    font information, for example: the weight of a font or the exact bounding boxes of the
   *    characters. Setting this flag to true results in a faster extraction process but less
   *    accurate extraction results.
   *
   * @return
   *    The created `PdfFontInfo` object
   */
  static PdfFontInfo* create(const GfxState* state, XRef* xref, bool parseEmbeddedFontFiles);

  /**
   * The ascent of the font, that is: the maximum extent above the base line.
   */
  double ascent;

  /**
   * The descent of the font, that is: the maximum extent below the base line.
   */
  double descent;

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
   * and without digits.
   */
  string fontBaseName;

  /**
   * The boolean flag indicating whether or not this font is a fixed width font.
   */
  bool isFixedWidth;

  /**
   * The boolean flag indicating whether or not the font is a serif font.
   */
  bool isSerif;

  /**
   * The boolean flag indicating whether or not the font is a symbolic font.
   */
  bool isSymbolic;

  /**
   * The boolean flag indicating whether or not the font is an italic font.
   */
  bool isItalic;


  /**
   * This method returns the font weight, that is: one of the values [100, 200, ..., 900] which
   * describes the boldness of the font's characters. Here is the meaning of the different values:
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
   *
   * @return
   *    The font weight.
   */
  int weight = 400;

  /**
   * The boolean flag indicating whether or not the font is a type-3 font.
   */
  bool isType3;

  /** The font matrix. */
  double fontMatrix[6];

  /**
   * A mapping of character names to bounding boxes (each given by its leftX, upperY, rightX, lowerY
   * coordinates in character space).
   */
  unordered_map<string, tuple<double, double, double, double>> glyphBoundingBoxes;
};

// =================================================================================================

/**
 * TODO
 * A class for parsing an embedded font file in Type-1 format. An embedded Type-1 font file is
 * stored in a "normal" PDF stream object. Here is an example:
 *
 * 12 0 obj
 * <<
 *   /Filter  /ASCII85Decode
 *   /Length  41116
 *   /Length1 2526
 *   /Length2 32393
 *   /Length3 570
 * >>
 * stream
 * ,p>`rDKJj'E+LaU0eP.@+AH9dBOu$hFD55nC
 * ... [omitted data] ...
 * endstream
 * endobj
 *
 * The part in << ... >> is the font file stream dictionary that provides metadata about the font
 * file. The font file is stored in the "stream ... endstream" part (in encrypted form).
 * A standard Type-1 font file, as described in the Adobe Type-1 Font Format specification,
 * consists of three parts: a clear-text (ASCII) portion , an encrypted portion, and a
 * fixed-content portion.
 * The clear-text portion contains the *font dictionary* that provides metadata about the font (
 * for example: the font name or the font weight) in PostScript format. Here is an example font
 * dictionary:
 *
 * %!PS-AdobeFont-1.0: CMEX10 003.002
 * %%Title: CMEX10
 * /FontType 1 def
 * /FontMatrix [0.001 0 0 0.001 0 0 ]readonly def
 * /FontName /YQJSDJ+CMEX10 def
 * /FontBBox {-24 -2960 1454 772 }readonly def
 * ...
 * currentdict end
 *
 * The encrypted portion contains
 * The fixed-content portion contains 512 ASCII zeros followed by a cleartomark operator.
 */
class Type1FontFileParser {
 public:
  /**
   * This method parses the embedded font file referenced by the given id.  for the following
   * information: TODO
   * Stores the parsed information to the given `PdfFontInfo` object.
   */
  void parse(const Ref& embFontId, XRef* xref, PdfFontInfo* fontInfo);

 private:
  void parseAsciiPart(Object* streamObj, int length, PdfFontInfo* fontInfo);

  void parseEncryptedPart(Object* streamObj, int length, PdfFontInfo* fontInfo);

  void parseCharString(const string& charString,
    const unordered_map<int, string>& subrs, int* curX, int* curY,
    int* leftX, int* upperY, int* rightX, int* lowerY, vector<int>* args,
    vector<int>* interpreterStack) const;

  /** TODO(korzen) */
  void decrypt(const char* bytes, int numBytes, int r, int n, string* resultStr) const;

  /** The mapping of hex-codes to their respective integer representations. */
  unordered_map<char, int> _charToHex{ { '0', 0 }, { '1', 1 }, { '2', 2 }, { '3', 3 },
      { '4', 4 }, { '5', 5 }, { '6', 6 }, { '7', 7 }, { '8', 8 }, { '9', 9 }, { 'a', 10 },
      { 'b', 11 }, { 'c', 12 }, { 'd', 13 }, { 'e', 14 }, { 'f', 15 } };

  /** The reverse mapping of `charToHex`. */
  unordered_map<int, char> _hexToChar{ { 0, '0' }, { 1, '1' }, { 2, '2' }, { 3, '3' },
      { 4, '4' }, { 5, '5' }, { 6, '6' }, { 7, '7' }, { 8, '8' }, { 9, '9' }, { 10, 'a' },
      { 11, 'b' }, { 12, 'c' }, { 13, 'd' }, { 14, 'e' }, { 15, 'f' } };
};

#endif  // PDFFONTINFO_H_
