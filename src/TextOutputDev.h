/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TEXTOUTPUTDEV_H_
#define TEXTOUTPUTDEV_H_

#include <codecvt>
#include <locale>  // std::wstring_convert
#include <string>
#include <unordered_map>
#include <vector>

#include <poppler/GfxState.h>
#include <poppler/OutputDev.h>

#include "./utils/LogUtils.h"
#include "./PdfDocument.h"


/**
 * This class is an implementation of Poppler's OutputDev. It is responsible for (1) handling the
 * different events triggered by Poppler while parsing the PDF (for example: "start a new page", or
 * "draw a character", or "update font", or "draw an image", or "draw a shape") and (2) storing the
 * information required by pdftotext++ about, for example, the glyphs, figures and, shapes in form
 * of a `PdfDocument`.
 */
class TextOutputDev : public OutputDev {
 public:
  /**
   * This constructor creates and initializes a new instance of this `TextOutputDev` class.
   *
   * @param parseEmbeddedFontFiles
   *   A boolean flag indicating whether or not to parse the font files embedded into the PDF.
   *   Setting this flag to true enables more accurate font information, for example: the weight of
   *   a font or the exact bounding boxes of the glyphs (without parsing the font files, the font
   *   weights are often 0 and the heights of the bounding boxes are guessed).
   *   Setting this flag to false results in a faster extraction process, but less accurate
   *   extraction results.
   * @param doc
   *   The `PdfDocument` in which the extracted information should be stored.
   * @param debug
   *   Whether or not this instance should print debug information to the console.
   * @param debugPageFilter
   *   The number of the page to which the debug information should be reduced. If specified as a
   *   value > 0, only those messages that relate to the given page will be printed to the console.
   */
  TextOutputDev(bool parseEmbeddedFontFiles, PdfDocument* doc, bool debug=false,
      int debugPageFilter=-1);

  /** The deconstructor. */
  ~TextOutputDev() override;

 private:
  /**
   * This method returns true if this device uses upside-down coordinates (meaning that (0,0)
   * is the top left corner of the page), and false otherwise.
   *
   * @return True if this device uses upside-down coordinates; false otherwise.
   */
  bool upsideDown() override { return true; }

  /**
   * This method returns true if this device uses drawChar() (that is: if it processes the text
   * char-wise) instead of drawString(), and false otherwise.
   *
   * @return True if this device uses drawChar(); false otherwise.
   */
  bool useDrawChar() override { return true; }

  /**
   * This method returns true if this device uses beginType3Char()/endType3Char() to process glyphs
   * printed in a Type-3 font. Otherwise it returns false, meaning that glyphs in Type-3 fonts will
   * be drawn with the "normal" drawChar()/drawString() methods.
   *
   * @return True if this device uses beginType3Char()/endType3Char(); false otherwise.
   */
  bool interpretType3Chars() override { return false; }

  /**
   * This method returns true if this instance requires information about the non-text elements
   * (like figures and shapes) contained in the PDF; false otherwise.
   *
   * @return True if this instance requires requires information about the non-text elements, false
   *    otherwise.
   */
  bool needNonText() override { return true; }

  // ===============================================================================================
  // Handler methods.

  /**
   * This method handles the event "start of a new page" by gathering all information required
   * by pdftotext++ about the page (for example, the width and the height) and storing this
   * information in form of a `PdfPage` to `_doc->pages`.
   *
   * @param pageNum
   *   The page number of the page.
   * @param state
   *   The current graphics state.
   * @param xref
   *   The xref table of the page.
   */
  void startPage(int pageNum, GfxState* state, XRef* xref) override;

  /**
   * This method handles the event "update the current font" by setting `_fontInfo` to the related
   * `PdfFontInfo` object stored in `_doc->fontInfos`. If no such object does not exist yet, this
   * method gathers all information required by pdftotext++ about the font (for example, the font
   * name, or the information whether or not the font is a bold font or an italic font) from the
   * `GfxFont` object stored in `state->getFont()`. The gathered font information is stored in
   * form of a `PdfFontInfo` object and is written to `_doc->fontInfos`.
   *
   * @param state
   *   The current graphics state.
   */
  void updateFont(GfxState* state) override;

  /**
   * This method handles the event "draw a character" by gathering all information required by
   * pdftotext++ about the character (for example, the position, the font, the font size, or
   * the text). The information are stored in form of a `PdfGlyph` object, which is appended to
   * `page->glyphs` if the current clipbox is equal to the page's clipbox or to `figure->glyphs`
   * otherwise (`figure` is the `PdfFigure` object related to the current clipbox, if no such
   * object exists, this method will create it).
   *
   * @param state
   *   The current graphics state.
   * @param x
   *   The x-coordinate of the glyph's lower-left, given relatively to the page's upper left.
   * @param y
   *   The y-coordinate of the glyph's lower-left, given relatively to the page's upper left.
   * @param dx
   *   The x-offset by which the cursor should be shifted after printing the character.
   * @param dy
   *   The y-offset by which the cursor should be shifted after printing the character.
   * @param originX
   *   TODO
   * @param originY
   *   TODO
   * @param c
   *   The character code in the content stream.
   * @param nBytes
   *   The number of bytes of the character (a character can consists of either 8-bit or 16-bit).
   * @param u
   *   The UCS-4 mapping used for text extraction.
   * @param uLen
   *   The number of unicode entries in u.  Usually 1 for a single character, but it may also have
   *   a larger value, for example for ligatures.
   */
  void drawChar(GfxState* state, double x, double y, double dx, double dy, double originX,
      double originY, CharCode c, int nBytes, const Unicode* u, int uLen) override;

  /**
   * This method handles the event "stroke a path" by gathering all information required
   * by pdftotext++ about the path (for example, the position) and storing this information in
   * form of a `PdfShape`. If the current clip box is equal to the page's clip box, the shape is
   * added to `_page->shapes`. Otherwise, the shape is added to `figure->shapes`, where figure is
   * the `PdfFigure` object associated with the current clip box.
   * NOTE: Our assumption here is that each clip box, which is different to the page's clip box,
   * represents a separate figure and that all shapes (and text) falling into the same clip box
   * belong to the same figure. We create exactly one figure per clip box. This figure is created
   * when the clip box is "seen" for the first time.
   *
   * @param state
   *   The current graphics state.
   */
  void stroke(GfxState* state) override;

  /**
   * This method handles the event "fill a path" in the same way as the event "stroke a path".
   * This is sufficient, because we only need the position of the path, but not the information
   * whether the path is filled or not.
   *
   * @param state
   *   The current graphics state.
   */
  void fill(GfxState* state) override;

 private:
  /**
   * This method multiplies the given transformation matrices and writes the result to `res`.
   *
   * @param m1
   *    The first transformation matrix (an array of size 6).
   * @param m2
   *    The second transformation matrix (an array of size 6).
   * @param res
   *    An array of size 6, into which the result should be written.
   */
  void concat(const double* m1, const double* m2, double* res) const;

  /** A boolean flag indicating whether or not to parse embedded font files. */
  bool _parseEmbeddedFontFiles;
  /** The PDF document to process. */
  PdfDocument* _doc;

  /** The current page to process. */
  PdfPage* _page;
  /** The current page number. */
  int _p;
  /** The xref table of the current PDF page. */
  XRef* _xref;
  /** The information about the current font. */
  PdfFontInfo* _fontInfo = nullptr;
  /** The current font size. */
  double _fontSize = 0.0;
  /** The number of elements (glyphs, shapes, figures) already processed. */
  int _numElements = 0;
  /** An object that converts a std::string to std::wstring. */
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> _wStringConverter;

  /** The logger. */
  Logger* _log;
};

#endif  // TEXTOUTPUTDEV_H_
