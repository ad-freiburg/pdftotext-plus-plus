/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TEXTOUTPUTDEV_H_
#define TEXTOUTPUTDEV_H_

#include <poppler/GfxState.h>
#include <poppler/OutputDev.h>

#include <codecvt>  // std::codecvt_utf8_utf16
#include <locale>  // std::wstring_convert

#include "./utils/Log.h"

#include "./PdfDocument.h"

// =================================================================================================

/**
 * This class is an implementation of Poppler's OutputDev. It is responsible for (1) handling the
 * different events triggered by Poppler while parsing the content streams of a PDF (for example:
 * "start a new page", or "draw a character", or "update font", or "draw a graphic", or "draw a
 * shape") and (2) storing the information required by pdftotext++ about, for example,
 * the characters, graphics, figures and shapes in form of a `PdfDocument`.
 */
class TextOutputDev : public OutputDev {
 public:
  /**
   * This constructor creates and initializes a new instance of this class.
   *
   * @param noEmbeddedFontFilesParsing
   *   A boolean flag indicating whether or not to parse the font files embedded into the current
   *   PDF file. Setting this parameter to true disables the parsing; setting it to false enables
   *   the parsing. Parsing the font files can enable more accurate bounding boxes of the chars
   *   (in particular, when the chars represent mathematical symbols). It also can enable more
   *   correct information about the style of a font (for example, whether or not the font is a
   *   bold font), for the following reason: actually, the PDF standard specifies several font
   *   flags that describe the style of a font. These flags are however often not set, even if they
   *   are supposed to be (for example, there is an isBold flag for a font, but this flag is often
   *   not set, even if the font is actually a bold font). Instead, the missing information is
   *   often stored in the embedded font file (if the font is actually embedded). The consequence
   *   of disabling the parsing of the font files is a faster extraction process, but a lower
   *   accuracy of the extracted text.
   * @param doc
   *   The `PdfDocument` to which the extracted information should be stored.
   * @param logLevel
   *   The logging level.
   * @param logPageFilter
   *   If set to a value > 0, only the logging messages produced while processing the
   *   <logPageFilter>-th page of the current PDF file will be printed to the console.
   */
  TextOutputDev(bool noEmbeddedFontFilesParsing, PdfDocument* doc, LogLevel logLevel = ERROR,
      int logPageFilter = -1);

  /** The deconstructor. */
  ~TextOutputDev() override;

 private:
  /**
   * This method returns true if this device uses upside-down coordinates (meaning that (0,0)
   * is the top left corner of the page), and false otherwise (meaning that (0,0) is the lower
   * left corner of the page).
   *
   * @return
   *    True if this device uses upside-down coordinates; false otherwise.
   */
  bool upsideDown() override { return true; }

  /**
   * This method returns true if this device uses drawChar() (that is: if it processes the text
   * char-wise) instead of drawString(), and false otherwise.
   *
   * @return
   *    True if this device uses drawChar(); false otherwise.
   */
  bool useDrawChar() override { return true; }

  /**
   * This method returns true if this device uses beginType3Char()/endType3Char() to process
   * characters printed in a Type-3 font. Otherwise it returns false, meaning that characters in
   * Type-3 fonts will be drawn with the "normal" drawChar()/drawString() methods.
   *
   * @return
   *    True if this device uses beginType3Char()/endType3Char(); false otherwise.
   */
  bool interpretType3Chars() override { return false; }

  /**
   * This method returns true if this instance requires information about the non-text elements
   * (like graphics and shapes) contained in the PDF; false otherwise.
   *
   * @return
   *    True if this instance requires information about non-text elements, false otherwise.
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
   *   The number of the page.
   * @param state
   *   The current graphics state.
   * @param xref
   *   The xref table of the page.
   */
  void startPage(int pageNum, GfxState* state, XRef* xref) override;

  /**
   * This method handles the event "end of page".
   */
  void endPage() override;

  /**
   * This method handles the event "update the current font" by setting `_fontInfo` to the related
   * `PdfFontInfo` object stored in `_doc->fontInfos` (providing further information about the font,
   * for example: the font name, or whether or not the font is a bold font or an italic font). If
   * no such object exists yet, this method gathers this font information from the `GfxFont` object
   * stored in `state->getFont()`, stores it in a new `PdfFontInfo` object and writes it to
   * `_doc->fontInfos[state->getFont()->getName()]`.
   *
   * @param state
   *   The current graphics state.
   */
  void updateFont(GfxState* state) override;

  /**
   * This method handles the event "draw a character" by gathering all information required by
   * pdftotext++ about the character (for example, the position, the font, the font size, or
   * the text). The information is stored in form of a `PdfCharacter` object, which is appended to
   * `page->characters` if the current clip box is equal to the page's clip box or to
   * `figure->characters` otherwise (`figure` is the `PdfFigure` object related to the current
   * clip box; if no such object exists yet, this method will create it).
   *
   * @param state
   *   The current graphics state.
   * @param x
   *   The x-coordinate of the character's lower-left.
   * @param y
   *   The y-coordinate of the character's lower-left.
   * @param dx
   *   The x-offset by which the cursor should be shifted after printing the character.
   * @param dy
   *   The y-offset by which the cursor should be shifted after printing the character.
   * @param originX
   *   TODO: Not sure what this parameter means, it is introduced by Poppler.
   * @param originY
   *   TODO: Not sure what this parameter means, it is introduced by Poppler.
   * @param c
   *   The character code.
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
   * by pdftotext++ about the path (for example, the position and the stroking color) and storing
   * this information in form of a `PdfShape`. If the current clip box is equal to the page's clip
   * box, the shape is added to `_page->shapes`. Otherwise, the shape is added to `figure->shapes`,
   * where figure is the `PdfFigure` object associated with the current clip box.
   *
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

  /**
   * This method handles the event "draw an image mask" by invoking the `drawGraphic(state)` method
   * below. See the comment given for this method for more information.
   */
  void drawImageMask(GfxState* state, Object* ref, Stream* str, int width, int height,
      bool invert, bool interpolate, bool inlineImg) override;

  /**
   * This method handles the event "draw an image" by invoking the `drawGraphic(state)` method
   * below. See the comment given for this method for more information.
   */
  void drawImage(GfxState* state, Object* ref, Stream* str, int width, int height,
      GfxImageColorMap* colorMap, bool interpolate, const int* maskColors,
      bool inlineImg) override;

  /**
   * This method handles the event "draw a masked image" by invoking the `drawGraphic(state)`
   * method below. See the comment given for this method for more information.
   */
  void drawMaskedImage(GfxState* state, Object* ref, Stream* str, int width, int height,
      GfxImageColorMap* colorMap, bool interpolate, Stream* maskStr, int maskWidth,
      int maskHeight, bool maskInvert, bool maskInterpolate) override;

  /**
   * This method handles the event "draw a soft masked image" by invoking the `drawGraphic(state)`
   * method below. See the comment given for this method for more information.
   */
  void drawSoftMaskedImage(GfxState* state, Object* ref, Stream* str, int width,
      int height, GfxImageColorMap* colorMap, bool interpolate, Stream* maskStr,
      int maskWidth, int maskHeight, GfxImageColorMap* maskColorMap,
      bool maskInterpolate) override;

  /**
   * This method is a generic method to handle the events "draw an image mask", "draw an image"
   * "draw a masked image", and "draw a soft masked image" in the exact same way. It gathers all
   * information required by pdftotext++ about the image (for example, the position) and stores
   * this information in form of a `PdfGraphic`. If the current clip box is equal to the page's
   * clip box, the graphic is added to `_page->graphics`. Otherwise, the graphic is added to
   * `figure->graphics`, where figure is the `PdfFigure` object associated with the current clip
   * box.
   *
   * @param state
   *    The current graphics state.
   */
  void drawGraphic(GfxState* state);

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

  // A boolean flag indicating whether or not to parse embedded font files.
  bool _parseEmbeddedFontFiles;
  // The PDF document to process.
  PdfDocument* _doc;
  // The current page.
  PdfPage* _page;
  // The current page number.
  int _p;
  // The xref table of the current page.
  XRef* _xref;
  // The information about the current font.
  PdfFontInfo* _fontInfo = nullptr;
  // The current font size.
  double _fontSize = 0.0;
  // The number of elements (characters, shapes, graphics) already processed.
  int _numElements = 0;
  // An object that converts a std::string to std::wstring.
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> _wStringConverter;
  // The logger.
  const Logger* _log;
};

#endif  // TEXTOUTPUTDEV_H_
