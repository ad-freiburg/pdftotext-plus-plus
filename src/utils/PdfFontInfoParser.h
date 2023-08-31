/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_PDFFONTINFOPARSER_H_
#define UTILS_PDFFONTINFOPARSER_H_

#include <poppler/GfxState.h>
#include <poppler/Object.h>  // Ref
#include <poppler/XRef.h>

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "../Types.h"

using std::string;
using std::tuple;
using std::unordered_map;
using std::vector;

using ppp::types::PdfFontInfo;

// =================================================================================================

namespace ppp::utils::fonts {

/**
 * TODO
 */
class PdfFontInfoParser {
 public:
  /**
   * TODO
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
  PdfFontInfo* parse(const GfxState* state, XRef* xref, bool parseEmbeddedFontFiles);
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

}  // namespace ppp::utils::fonts

#endif  // UTILS_PDFFONTINFOPARSER_H_
