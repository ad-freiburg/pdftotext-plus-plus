/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef PDFDOCUMENT_H_
#define PDFDOCUMENT_H_

#include <limits>  // std::numeric_limits
#include <string>
#include <unordered_map>
#include <vector>

#include <poppler/GfxFont.h>
#include <poppler/GfxState.h>
#include <poppler/PDFDoc.h>

#include "./PdfFontInfo.h"


struct Timing {
  std::string description;
  int64_t time = 0;

  Timing(const std::string& descriptionA, int64_t timeA) {
    description = descriptionA;
    time = timeA;
  }
};

// =================================================================================================


/**
 * The direction of a cut used in context of the XY-cut algorithm.
 */
enum CutDir { X, Y };

/**
 * This class represents an X-cut or Y-cut used in context of the XY-cut algorithm.
 */
class Cut {
 public:
  /**
   * This constructor creates and initializes a new `Cut`.
   *
   * @param dir
   *   The direction of the cut (x or y).
   * @param pageNum
   *   The page number of the cut.
   * @param x1
   *   The x-coordinate of the start point of the cut.
   * @param y1
   *   The y-coordinate of the start point of the cut.
   * @param x2
   *   The x-coordinate of the end point of the cut.
   * @param y2
   *   The y-coordinate of the end point of the cut.
   */
  Cut(const CutDir dir, int pageNum, double x1, double y1, double x2, double y2);

  /** The direction of the cut. */
  CutDir dir;

  /** The page number. */
  int pageNum = -1;

  /**
   * The x,y-coordinates of the cut. x1, y1 are the coordinates of the start point of the cut and
   * x2, y2 are the coordinates of the end point.
   */
  double x1 = std::numeric_limits<double>::max();
  double y1 = std::numeric_limits<double>::max();
  double x2 = std::numeric_limits<double>::min();
  double y2 = std::numeric_limits<double>::min();
};

// =================================================================================================

/**
 * The different text unit.
 */
enum TextUnit { CHARACTERS = 1, WORDS = 2, TEXT_LINES = 3, TEXT_BLOCKS = 4, PARAGRAPHS = 5 };

/**
 * This class represents an abstract class for all visible elements in a PDF, for example: glyphs,
 * words, text blocks, figures and shapes.
 */
class PdfElement {
 public:
  /** This constructor creates and initializes a new `PdfElement`. */
  PdfElement();

  /** The deconstructor. */
  virtual ~PdfElement() = 0;

  /**
   * This method returns the width of this element.
   *
   * @return The width of this element.
   */
  double getWidth() const;

  /**
   * This method returns the height of this element.
   *
   * @return The height of this element.
   */
  double getHeight() const;

  /**
   * This method returns a string representation of this element for debugging purposes.
   *
   * @return A string representation of this element.
   */
  virtual std::string toString() const = 0;

  /** The unique id of this element. **/
  std::string id;

  /** The minimum and maximum x,y-coordinates of this element. */
  double minX = std::numeric_limits<double>::max();
  double minY = std::numeric_limits<double>::max();
  double maxX = std::numeric_limits<double>::min();
  double maxY = std::numeric_limits<double>::min();

  /** The page number. */
  double pageNum = -1;
};

// =================================================================================================

/**
 * This class represents a single glyph in a PDF document.
 */
class PdfGlyph : public PdfElement {
 public:
  /** This constructor creates and initalizes a new `PdfGlyph`. */
  PdfGlyph();

  /** The deconstructor. */
  ~PdfGlyph();

  /** The position of this glyph in the extraction order. */
  int rank = -1;

  /** The text of this glyph. */
  std::string text;

  /**
   * The name of the character which is represented by this glyph, for example: "A" or
   * "summationdisplay".
   */
  std::string charName;

  /** The font size of this glyph. */
  double fontSize = -1;

  /** The font name of this glyph. */
  std::string fontName;

  /** The RGB stroking color of this glyph. */
  double color[3];

  /** The opacity of this glyph. */
  double opacity;

  /**
   * The writing mode of this glyph, given as an integer 0 or 1.
   * The meaning of this integer is as follows:
   * 0: vertical writing mode; 1: horizontal writing mode.
   */
  int wMode = -1;

  /**
   * The rotation of this glyph, given as an integer 0, 1, 2, or 3.
   * The meaning of this integer is as follows:
   * 0: the glyph is not rotated;
   * 1: the glyph is rotated by 90 degress (clock-wise);
   * 2: the glyph is rotated by 180 degress (clock-wise);
   * 3: the glyph is rotated by 270 degress (clock-wise).
   */
  int rotation = -1;

  /**
   * The baseline of the glyph.
   */
  double base;

  /** The unicode codepoints of the character(s) this glyph represents. */
  std::vector<unsigned int> unicodes;

  PdfGlyph* isDiacriticMarkOfBaseGlyph = nullptr;
  PdfGlyph* isBaseGlyphOfDiacriticMark = nullptr;
  std::string textWithDiacriticMark;

  bool isSubscript = false;
  bool isSuperscript = false;

  /**
   * This method returns a string representation of this glyph for debugging purposes.
   *
   * @return A string representation of this glyph.
   */
  std::string toString() const override;
};

// =================================================================================================

/**
 * This class represents any non-text element in a PDF document, for example: a figure or a shape.
 */
class PdfNonText : public PdfElement {
 public:
  /** This constructor creates and initalizes a new `PdfNonText` object. */
  PdfNonText();

  /** The deconstructor. */
  ~PdfNonText();

  /**
   * This method returns a string representation of this element for debugging purposes.
   *
   * @return A string representation of this element.
   */
  std::string toString() const override;
};

// =================================================================================================

/**
 * This class represents a single word in a PDF document.
 */
class PdfWord : public PdfElement {
 public:
  /** This constructor creates and initalizes a new `PdfWord`. */
  PdfWord();

  /** The constructor creates a copy of the given word. */
  PdfWord(const PdfWord& word);

  /** The deconstructor. */
  ~PdfWord();

  /** The text of this word. */
  std::string text;

  /** The font size of this word. */
  double fontSize = -1;

  /** The font name of this word. */
  std::string fontName;

  /**
   * The writing mode of this word, given as an integer 0 or 1.
   * The meaning of this integer is as follows:
   * 0: vertical writing mode; 1: horizontal writing mode.
   */
  int wMode = -1;

  /**
   * The rotation of this word, given as an integer 0, 1, 2, or 3.
   * The meaning of this integer is as follows:
   * 0: the glyph is not rotated;
   * 1: the glyph is rotated by 90 degress (clock-wise);
   * 2: the glyph is rotated by 180 degress (clock-wise);
   * 3: the glyph is rotated by 270 degress (clock-wise).
   */
  int rotation = -1;

  /** The glyphs of this word. */
  std::vector<PdfGlyph*> glyphs;

  /**
   * The left subscript of the word.
   */
  PdfWord* leftSubscript = nullptr;

  /**
   * The left superscript of the word.
   */
  PdfWord* leftSuperscript = nullptr;

  /**
   * The left punctuation of the word.
   */
  PdfWord* leftPunctuation = nullptr;

  /**
   * The right subscript of the word.
   */
  PdfWord* rightSubscript = nullptr;

  /**
   * The right superscript of the word.
   */
  PdfWord* rightSuperscript = nullptr;

  /**
   * The right punctuation of the word.
   */
  PdfWord* rightPunctuation = nullptr;

  PdfWord* isFirstPartOfHyphenatedWord = nullptr;
  PdfWord* isSecondPartOfHyphenatedWord = nullptr;

  /**
   * This method returns a string representation of this word for debugging purposes.
   *
   * @return A string representation of this word.
   */
  std::string toString() const override;

  /** The position of this glyph in the extraction sequence. */
  int rank = -1;
};

// =================================================================================================

/**
 * This class represents a single text line in a PDF document.
 */
class PdfTextLine : public PdfElement {
 public:
  /** This constructor creates and initalizes a new `PdfTextLine` object. **/
  PdfTextLine();

  /** The deconstructor. */
  ~PdfTextLine();

  /** The font size of this text line. */
  double fontSize = -1;

  /** The font name of this line. */
  std::string fontName;

  /** The text of this line. */
  std::string text;

  /**
   * The writing mode of this line, given as an integer 0 or 1.
   * The meaning of this integer is as follows:
   * 0: vertical writing mode; 1: horizontal writing mode.
   */
  int wMode = -1;

  /**
   * The rotation of this line, given as an integer 0, 1, 2, or 3.
   * The meaning of this integer is as follows:
   * 0: the glyph is not rotated;
   * 1: the glyph is rotated by 90 degress (clock-wise);
   * 2: the glyph is rotated by 180 degress (clock-wise);
   * 3: the glyph is rotated by 270 degress (clock-wise).
   */
  int rotation = -1;

  /**
   * The baseline of the glyph.
   */
  double base;

  /** The words of this line. */
  std::vector<PdfWord*> words;

  /**
   * This method returns a string representation of this line for debugging purposes.
   *
   * @return A string representation of this line.
   */
  std::string toString() const override;
};

// =================================================================================================

/**
 * This class represents a single text block in PDF document.
 */
class PdfTextBlock : public PdfElement {
 public:
  /** This constructor creates and initalizes a new `PdfTextBlock` object. **/
  PdfTextBlock();

  /** The deconstructor. */
  ~PdfTextBlock();

  /** The semantic role of this block. */
  std::string role;

  /** The text of this block. */
  std::string text;

  /** The font size of this block. */
  double fontSize = -1;

  /** The font name of this block. */
  std::string fontName;

  /**
   * The writing mode of this block, given as an integer 0 or 1.
   * The meaning of this integer is as follows:
   * 0: vertical writing mode; 1: horizontal writing mode.
   */
  int wMode = -1;

  /**
   * The rotation of this block, given as an integer 0, 1, 2, or 3.
   * The meaning of this integer is as follows:
   * 0: the glyph is not rotated;
   * 1: the glyph is rotated by 90 degress (clock-wise);
   * 2: the glyph is rotated by 180 degress (clock-wise);
   * 3: the glyph is rotated by 270 degress (clock-wise).
   */
  int rotation = -1;

  /**
   * Whether or not this text block is emphasized compared to the other blocks.
   */
  bool isEmphasized;

  /** The text lines of this block. */
  std::vector<PdfTextLine*> lines;

  /**
   * This method returns a string representation of this block for debugging purposes.
   *
   * @return A string representation of this block.
   */
  std::string toString() const override;
};

// =================================================================================================

/**
 * TODO
 */
class PdfPageSegment : public PdfElement {
 public:
  /** This constructor creates and initalizes a new `PdfPageSegment`. */
  PdfPageSegment();

  /** The deconstructor. */
  ~PdfPageSegment();

  /** The elements of this segment. */
  std::vector<PdfElement*> elements;

  /** The text lines of this segment. */
  std::vector<PdfTextLine*> lines;

  /** A boolen flag that indicates whether or not the segment is justified. */
  bool isJustified;

  /**
   * This method returns a string representation of this segment for debugging purposes.
   *
   * @return A string representation of this segment.
   */
  std::string toString() const override;
};

// =================================================================================================

/**
 * This class represents a single page of a PDF document.
 */
class PdfPage {
 public:
  /** This constructor creates and initalizes a new `PdfPage`. */
  PdfPage();

  /** The deconstructor. */
  ~PdfPage();

  /** The width of this page. */
  double width = 0;

  /** The height of this page. */
  double height = 0;

  /** The page number. */
  int pageNum = -1;

  /** The glyphs of this page. */
  std::vector<PdfGlyph*> glyphs;

  /** The words of this page. */
  std::vector<PdfWord*> words;

  /** The blocks of this page. */
  std::vector<PdfTextBlock*> blocks;

  /** The segments of this page. */
  std::vector<PdfPageSegment*> segments;

  /** The non-text elements of this page, for example: figures and shapes. */
  std::vector<PdfNonText*> nonTexts;

  /** The XY-cuts made to detect the text blocks. */
  std::vector<Cut*> blockDetectionCuts;

  /** The XY-cuts made to detect the reading order of the text blocks. */
  std::vector<Cut*> readingOrderCuts;
};

// =================================================================================================

/**
 * This class represents a whole PDF document.
 */
class PdfDocument {
 public:
  /** This constructor creates and initializes a new `PdfDocument`. */
  PdfDocument();

  /** The deconstructor. */
  ~PdfDocument();

  /** The pages of this document. */
  std::vector<PdfPage*> pages;

  /**
   * A dictionary that maps font names to respective `PdfFontInfo` objects (each providing
   * further information about a font, for example: whether or not the font is a bold font.
   */
  std::unordered_map<std::string, PdfFontInfo*> fontInfos;

  /** The average glyph width in this document. */
  double avgGlyphWidth = 0;

  /** The average glyph height in this document. */
  double avgGlyphHeight = 0;

  /** The most frequent font size among the glyphs of this document. */
  double mostFreqFontSize = 0;

  /** The name of the most frequent font among the glyphs of this document. */
  std::string mostFreqFontName;

  /** The most frequent line distance in this document. */
  double mostFreqEstimatedLineDistance = 0;

  double mostFreqLineGap = 0;

  /** The most frequent word height in this document. */
  double mostFreqWordHeight = 0;

  /** The most frequent line indent in this document. */
  double mostFreqLineIndent = 0;
};

#endif  // PDFDOCUMENT_H_
