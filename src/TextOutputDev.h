/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TextOutputDev_H_
#define TextOutputDev_H_

#include <codecvt>
#include <locale>  // std::wstring_convert
#include <string>
#include <unordered_map>
#include <vector>

#include <poppler/GfxState.h>
#include <poppler/OutputDev.h>

#include "./PdfDocument.h"


/**
 * This class is an implementation of Poppler's OutputDev. It is responsible for (1) handling the
 * different events triggered by Poppler while parsing the PDF (for example: "start a new page", or
 * "draw a character", or "update font", or "draw an image") and (2) storing the information
 * required by pdftotext++ in form of a `PdfDocument`.
 */
class TextOutputDev : public OutputDev {
 public:
  /**
   * This constructor creates and initializes a new `TextOutputDev` object.
   *
   * @param parseEmbeddedFontFiles
   *   A boolean flag indicating whether or not to parse the embedded font files for more accurate
   *   font information, for example: the weight of a font or the exact bounding boxes of the
   *   glyphs. Setting this flag to true results in a faster extraction process but less accurate
   *   extraction results.
   * @param doc
   *  The `PdfDocument` in which the extracted information should be stored.
   */
  explicit TextOutputDev(bool parseEmbeddedFontFiles, PdfDocument* doc);

  /** The deconstructor. */
  ~TextOutputDev() override;

  /**
   * This method returns true if this object was successfully initialized, false otherwise.
   *
   * @return True, if this object was successfully initialized, false otherwise.
   */
  bool isOk() { return _ok; }

 private:
  /**
   * This method returns true if this device uses upside-down coordinates (upside-down means (0,0)
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
   * This method returns true if this device requires non-text content (like figures and shapes).
   *
   * @return True if this device requires non-text content, false otherwise.
   */
  bool needNonText() override { return true; }

  // ===============================================================================================
  // Handler methods.

  /**
   * This method handles the event "start of a new page" by gathering all information required
   * by pdftotext++ about the page (for example, the width and height) and storing this
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
   * This method handles the event "update the current font" by gathering all information required
   * by pdftotext++ about the font (for example, the font name, or the information whether
   * or not the font is a bold font or an italic font) and storing this information in form of a
   * `PdfFontInfo` to `_doc->fontInfos`.
   *
   * @param state
   *   The current graphics state.
   */
  void updateFont(GfxState* state) override;

  /**
   * This method handles the event "draw a character" by gathering all information required by
   * pdftotext++ about the character (for example, the position, the font, the font size, or
   * the text) and storing this information in form of a `PdfGlyph` to `page->glyphs`, where `page`
   * denotes the current `PdfPage`.
   */
  void drawChar(GfxState* state, double x, double y, double dx, double dy, double originX,
      double originY, CharCode c, int nBytes, const Unicode* u, int uLen) override;

  /**
   * This method handles the event "stroke a path" by gathering all information required
   * by pdftotext++ about the path (for example, the position) and storing this information
   * in form of a `PdfShape` to `page->shapes`, where `page` denotes the current `PdfPage`.
   *
   * @param state
   *   The current graphics state.
   */
  void stroke(GfxState* state) override;

  /**
   * This method handles the event "fill a path" by gathering all information required
   * by pdftotext++ about the path (for example, the position) and storing this information
   * in form of a `PdfShape` to `page->shapes`, where `page` denotes the current `PdfPage`.
   *
   * @param state
   *   The current graphics state.
   */
  void fill(GfxState* state) override;

  /**
   * This method handles the event "draw an image mask" by invoking the `drawImage(state, width,
   * height)` method described below.
   */
  void drawImageMask(GfxState* state, Object* ref, Stream* str, int width, int height,
      bool invert, bool interpolate, bool inlineImg) override;

  /**
   * This method handles the event "draw an image" by invoking the `drawImage(state, width, height)`
   * method described below.
   */
  void drawImage(GfxState* state, Object* ref, Stream* str, int width, int height,
      GfxImageColorMap* colorMap, bool interpolate, const int* maskColors,
      bool inlineImg) override;

  /**
   * This method handles the event "draw a masked image" by invoking the `drawImage(state, width,
   * height)` method described below.
   */
  void drawMaskedImage(GfxState* state, Object* ref, Stream* str, int width, int height,
      GfxImageColorMap* colorMap, bool interpolate, Stream* maskStr, int maskWidth,
      int maskHeight, bool maskInvert, bool maskInterpolate) override;

  /**
   * This method handles the event "draw a soft masked image" by invoking the `drawImage(state,
   * width, height)` method described below.
   */
  void drawSoftMaskedImage(GfxState* state, Object* ref, Stream* str, int width,
      int height, GfxImageColorMap* colorMap, bool interpolate, Stream* maskStr,
      int maskWidth, int maskHeight, GfxImageColorMap* maskColorMap,
      bool maskInterpolate) override;

  /**
   * This method is a generic method to handle all events belonging to the `drawImageMask()`,
   * `drawImage()`, `drawMaskedImage()`, and `drawSoftMaskedImage()` methods in the exact same way
   * (it is internally invoked by each of the mentioned methods). It gathers all information
   * required by pdftotext++ about the image (for example, the position) and stores this
   * information in form of a `PdfFigure` to `page->figures`, where `page` denotes the current
   * `PdfPage`.
   */
  void drawImage(GfxState* state, int width, int height);

  /** This method handles the event "end of the current page". */
  void endPage() override;

  /**
   * This method handles the event "restore the graphics state".
   *
   * @param state The graphics state to restore.
   */
  void restoreState(GfxState* state) override;

 private:
  void concat(const double* m1, const double* m2, double* res) const;

  /** The document to process. */
  PdfDocument* _doc;
  /** The current page to process. */
  PdfPage* _page;
  /** The xref table of the current PDF page. */
  XRef* _xref;
  /** The information about the current font. */
  PdfFontInfo* _fontInfo;
  /** The current font size. */
  double _fontSize;
  /** The number of glyphs already processed. */
  int _numGlyphs = 0;
  /** A boolean flag indicating whether or not this device was successfully initialized. */
  bool _ok;
  /** A boolean flag indicating whether or not to parse embedded font files. */
  bool _parseEmbeddedFontFiles;
  /** A mapping of common glyph names to the text the respective glyphs actual represent.*/
  std::unordered_map<std::string, std::string> _glyphMap;

  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> _wStringConverter;
};

#endif  // TextOutputDev_H_
