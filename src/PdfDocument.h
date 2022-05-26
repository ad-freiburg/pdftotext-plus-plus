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
#include <unordered_set>
#include <vector>

#include <poppler/GfxFont.h>
#include <poppler/GfxState.h>
#include <poppler/PDFDoc.h>

#include "./PdfFontInfo.h"

class PdfElement;
class PdfTextElement;
class PdfWord;
class PdfShape;
class PdfGraphic;
class PdfTextLine;
class PdfTextBlock;
class PdfPageSegment;
class PdfDocument;

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
  Cut(const CutDir dir);

  /** The direction of the cut. */
  CutDir dir;

  /** The id of this cut, needed for debugging purposes. */
  std::string id;

  /** Whether or not this cut was chosen on page segmentation. */
  bool isChosen = false;

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

  /** The gap width and gap height. */
  double gapWidth = 0.0;
  double gapHeight = 0.0;

  PdfElement* elementBefore = nullptr;
  PdfElement* elementAfter = nullptr;
  std::vector<PdfElement*> cuttingElements;

  size_t posInElements;
};

// =================================================================================================

/**
 * The different text unit.
 */
enum TextUnit { GLYPHS = 1, WORDS = 2, TEXT_LINES = 3, TEXT_BLOCKS = 4, PARAGRAPHS = 5 };

// =================================================================================================

class PdfPosition {
 public:
  /** The page number. */
  double pageNum = -1;

  /** The minimum and maximum x,y-coordinates of this element. */
  double leftX = std::numeric_limits<double>::max();
  double upperY = std::numeric_limits<double>::max();
  double rightX = std::numeric_limits<double>::min();
  double lowerY = std::numeric_limits<double>::min();

  /** The rotation of this element. */
  int rotation = 0;
  /** The writing mode of this element. */
  int wMode = 0;

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

  double getRotLeftX() const;
  double getRotUpperY() const;
  double getRotRightX() const;
  double getRotLowerY() const;
  double getRotHeight() const;
  double getRotWidth() const;

  /**
   * This method returns a string representation of this position for debugging purposes.
   *
   * @return A string representation of this position.
   */
  std::string toString() const;
};

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

  /** The unique id of this element. **/
  std::string id;

  /** The position of this element in the PDF. */
  PdfPosition* position;

  /** The position of this element in the extraction order. */
  int rank = -1;

  PdfDocument* doc;

  /**
   * This method returns a string representation of this element for debugging purposes.
   *
   * @return A string representation of this element.
   */
  virtual std::string toString() const = 0;
};

/**
 * This class represents an abstract class for all text elements in a PDF, for example: glyphs,
 * words, text blocks.
 */
class PdfTextElement : public PdfElement {
 public:
  /** This constructor creates and initializes a new `PdfTextElement`. */
  PdfTextElement();

  /** The deconstructor. */
  virtual ~PdfTextElement() = 0;

  /** The text of this element. */
  std::string text;

  /** The font size of this element. */
  double fontSize = -1;

  /** The font name of this element. */
  std::string fontName;

  /** The RGB stroking color of this element. */
  double color[3];

  /** The opacity of this element. */
  double opacity;

  /** The baseline of this element. */
  double base;
};

// =================================================================================================

/**
 * This class represents any non-text element in a PDF document, for example: a figure or a shape.
 */
class PdfNonTextElement : public PdfElement {
 public:
  /** This constructor creates and initalizes a new `PdfNonTextElement` object. */
  PdfNonTextElement();

  /** The deconstructor. */
  ~PdfNonTextElement();

  /**
   * This method returns a string representation of this element for debugging purposes.
   *
   * @return A string representation of this element.
   */
  std::string toString() const override;
};

// =================================================================================================

/**
 * This class represents a single glyph in a PDF document.
 */
class PdfGlyph : public PdfTextElement {
 public:
  /** This constructor creates and initalizes a new `PdfGlyph`. */
  PdfGlyph();

  /** The deconstructor. */
  ~PdfGlyph();

  /**
   * The name of the character which is represented by this glyph, for example: "A" or
   * "summationdisplay".
   */
  std::string charName;

  /** The unicode codepoints of the character(s) this glyph represents. */
  std::vector<unsigned int> unicodes;

  PdfGlyph* isDiacriticMarkOfBaseGlyph = nullptr;
  PdfGlyph* isBaseGlyphOfDiacriticMark = nullptr;
  std::string textWithDiacriticMark;

  bool isSubscript = false;
  bool isSuperscript = false;

  /**
   * The reference to the word of which this glyph is a part of.
   */
  PdfWord* word = nullptr;

  /**
   * This method returns a string representation of this glyph for debugging purposes.
   *
   * @return A string representation of this glyph.
   */
  std::string toString() const override;
};

// =================================================================================================

/**
 * This class represents a single word in a PDF document.
 */
class PdfWord : public PdfTextElement {
 public:
  /** This constructor creates and initalizes a new `PdfWord`. */
  PdfWord();

  /** The constructor creates a copy of the given word. */
  PdfWord(const PdfWord& word);

  /** The deconstructor. */
  ~PdfWord();

  /** The glyphs of this word. */
  std::vector<PdfGlyph*> glyphs;

  // Stacked words: words that logically belong together and overlap horizontally, for example:
  // a summation symbol with its sub- and superscripts.
  PdfWord* isPartOfStackedMathSymbol = nullptr;
  std::vector<PdfWord*> isBaseOfStackedMathSymbol;

  PdfWord* isFirstPartOfHyphenatedWord = nullptr;
  PdfWord* isSecondPartOfHyphenatedWord = nullptr;

  /**
   * The reference to the text line of which this word is a part of.
   */
  PdfTextLine* line = nullptr;

  /**
   * This method returns a string representation of this word for debugging purposes.
   *
   * @return A string representation of this word.
   */
  std::string toString() const override;
};

// =================================================================================================

/**
 * This class represents a single figure in a PDF document.
 */
class PdfFigure : public PdfNonTextElement {
 public:
  /** This constructor creates and initalizes a new `PdfFigure`. */
  PdfFigure();

  /** The constructor creates a copy of the given figure. */
  PdfFigure(const PdfFigure& figure);

  /** The deconstructor. */
  ~PdfFigure();

  /** The glyphs of this figure. */
  std::vector<PdfGlyph*> glyphs;

  /** The shapes of this figure. */
  std::vector<PdfShape*> shapes;

  /** The graphics of this figure. */
  std::vector<PdfGraphic*> graphics;

  /** The coordinates of the clip box of this figure. */
  double clipLeftX = std::numeric_limits<double>::max();
  double clipUpperY = std::numeric_limits<double>::max();
  double clipRightX = std::numeric_limits<double>::min();
  double clipLowerY = std::numeric_limits<double>::min();

  /**
   * This method returns a string representation of this figure for debugging purposes.
   *
   * @return A string representation of this figure.
   */
  std::string toString() const override;
};

/**
 * This class represents a single shape in a PDF document.
 */
class PdfShape : public PdfNonTextElement {
 public:
  /** This constructor creates and initalizes a new `PdfShape`. */
  PdfShape();

  /** The constructor creates a copy of the given shape. */
  PdfShape(const PdfShape& shape);

  /** The deconstructor. */
  ~PdfShape();

  /**
   * This method returns a string representation of this shape for debugging purposes.
   *
   * @return A string representation of this shape.
   */
  std::string toString() const override;
};

/**
 * This class represents a single graphic in a PDF document.
 */
class PdfGraphic : public PdfNonTextElement {
 public:
  /** This constructor creates and initalizes a new `PdfGraphic`. */
  PdfGraphic();

  /** The deconstructor. */
  ~PdfGraphic();

  /**
   * This method returns a string representation of this graphic for debugging purposes.
   *
   * @return A string representation of this graphic.
   */
  std::string toString() const override;
};

// =================================================================================================

// enum PdfTextLineAlignment { LEFT, RIGHT, CENTERED, JUSTIFIED };

/**
 * This class represents a single text line in a PDF document.
 */
class PdfTextLine : public PdfTextElement {
 public:
  /** This constructor creates and initalizes a new `PdfTextLine` object. **/
  PdfTextLine();

  /** The deconstructor. */
  ~PdfTextLine();

  /** The words of this line. */
  std::vector<PdfWord*> words;

  /**
   * The reference to the text block of which this text line is a part of.
   */
  PdfTextBlock* block = nullptr;

  /**
   * The coordinates of the bounding box of this line, with sub- and superscripts ignored.
   */
  double baseBBoxLeftX = std::numeric_limits<double>::max();
  double baseBBoxUpperY = std::numeric_limits<double>::max();
  double baseBBoxRightX = std::numeric_limits<double>::min();
  double baseBBoxLowerY = std::numeric_limits<double>::min();

  /**
   * The possible alignments of this text line. Since the alignment of a text line can be ambiguous
   * (for example, a justified text line could be considered as a left-aligned or right-aligned
   * text line), we do not compute a single alignment, but all possible alignments of the text line.
   *
   */
  // std::unordered_set<PdfTextLineAlignment> alignments;

  /**
   * Whether or not this line is indented, that is: the gap between the left boundary of the text
   * line and the left boundary of the containing segment is equal to the most frequnet line
   * indentation amount.
   */
  // bool isIndented = false;
  // double indent = 0.0;
  double leftMargin = 0.0;
  double rightMargin = 0.0;

  /**
   * The segment in which this text line is contained.
   */
  const PdfPageSegment* segment = nullptr;

  const PdfTextLine* prevSiblingTextLine = nullptr;
  const PdfTextLine* nextSiblingTextLine = nullptr;
  const PdfTextLine* parentTextLine = nullptr;

  /**
   * This method returns a string representation of this line for debugging purposes.
   *
   * @return A string representation of this line.
   */
  std::string toString() const override;

  double maxFontSize = 0;

  PdfTextLine* prevLine;
  PdfTextLine* nextLine;
};

// =================================================================================================

/**
 * This class represents a single text block in PDF document.
 */
class PdfTextBlock : public PdfTextElement {
 public:
  /** This constructor creates and initalizes a new `PdfTextBlock` object. **/
  PdfTextBlock();

  /** The deconstructor. */
  ~PdfTextBlock();

  /** The semantic role of this block. */
  std::string role;

  /**
   * Whether or not this text block is emphasized compared to the other blocks.
   */
  bool isEmphasized;

  /** The text lines of this block. */
  std::vector<PdfTextLine*> lines;

  double hangingIndent = 0.0;

  double trimLeftX = 0.0;
  double trimUpperY = 0.0;
  double trimRightX = 0.0;
  double trimLowerY = 0.0;

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

  /** The text lines of this segment. */
  std::vector<PdfTextBlock*> blocks;

  /** A boolen flag that indicates whether or not the segment is justified. */
  bool isJustified;

  double trimLeftX;
  double trimUpperY;
  double trimRightX;
  double trimLowerY;

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

  /** The coordinates of the clip box of this page. */
  double clipLeftX = std::numeric_limits<double>::max();
  double clipUpperY = std::numeric_limits<double>::max();
  double clipRightX = std::numeric_limits<double>::min();
  double clipLowerY = std::numeric_limits<double>::min();

  /** The width of this page. */
  double getWidth() const { return clipRightX - clipLeftX; };

  /** The height of this page. */
  double getHeight() const { return clipLowerY - clipUpperY; };

  /** The page number. */
  int pageNum = -1;

  /** The glyphs of this page. */
  std::vector<PdfGlyph*> glyphs;

  /** The words of this page. */
  std::vector<PdfWord*> words;

  /** The text lines of this page. */
  std::vector<PdfTextLine*> textLines;

  /** The blocks of this page. */
  std::vector<PdfTextBlock*> blocks;

  /** The segments of this page. */
  std::vector<PdfPageSegment*> segments;

  /** The XY-cuts made to detect the text blocks. */
  std::vector<Cut*> blockDetectionCuts;

  /** The XY-cuts made to detect the reading order of the text blocks. */
  std::vector<Cut*> readingOrderCuts;

  /** The figures of this page. */
  std::vector<PdfFigure*> figures;

  /** The shapes of this page. */
  std::vector<PdfShape*> shapes;

  /** The graphics of this page. */
  std::vector<PdfGraphic*> graphics;
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
  double mostFreqWordDistance = 0;
  double mostFreqEstimatedLineDistance = 0;

  double mostFreqLineGap = 0;

  /** The most frequent word height in this document. */
  double mostFreqWordHeight = 0;

  /** The most frequent line indent in this document. */
  // double mostFreqLineIndent = 0;
  double mostFreqLineLeftMargin = 0;
};

#endif  // PDFDOCUMENT_H_
