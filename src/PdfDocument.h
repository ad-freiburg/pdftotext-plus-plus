/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef PDFDOCUMENT_H_
#define PDFDOCUMENT_H_

#include <limits>  // numeric_limits
#include <string>
#include <unordered_map>
#include <vector>

#include "./PdfFontInfo.h"
#include "./Types.h"

using ppp::types::SemanticRole;
using std::numeric_limits;
using std::string;
using std::unordered_map;
using std::vector;

class PdfDocument;
class PdfElement;
class PdfGraphic;
class PdfPageSegment;
class PdfShape;
class PdfTextBlock;
class PdfTextElement;
class PdfTextLine;
class PdfWord;

// =================================================================================================

/**
 * The directions of a cut used by the `XYCut` class.
 *
 * If the direction is X, the cut is represented by a vertical line (cutting the X-dimension).
 * If the direction is Y, the cut is represented by a horizontal line (cutting the Y-dimension).
 */
enum CutDir { X, Y };

/**
 * This class represents an X-cut (a vertical line splitting given elements in two halves) or an
 * Y-cut (a horizontal line splitting given elements in two halves). It is primarily used by the
 * `XYCut` class.
 */
class Cut {
 public:
  /**
   * This constructor creates a new instance of this class and initializes it with the given cut
   * direction.
   *
   * @param dir
   *   The direction of this cut.
   */
  explicit Cut(const CutDir dir);

  /**
   * This constructor creates a new instance of this class and initializes it with the given cut
   * direction and position in the vector of elements to divide.
   *
   * @param dir
   *   The direction of this cut.
   * @param id
   *   The id of this cut.
   * @param posInElements
   *   The page elements to be split are given in a vector. The parameter `posInElements` is the
   *   position of the cut in this vector. It can be understood as follows: If set to value i, the
   *   cut splits the elements in vector V between V[i-1] and V[i].
   */
  explicit Cut(const CutDir dir, const string& id, int posInElements);

  /** The deconstructor. */
  ~Cut();

  // The direction of this cut.
  CutDir dir;

  // The id of this cut, needed for debugging purposes (for example, for matching a cut mentioned
  // in the debug output to the respective cut in the visualization).
  string id;

  // The number of the page on which this cut is located.
  int pageNum = -1;

  // The x,y-coordinates of this cut. The (x1,y1) pair describes the start point; the (x2,y2) pair
  // describes the end point of this cut.
  // NOTE: X-cuts represent vertical lines, so x1 and x2 should be equal for X-cuts;
  // Y-cuts represent horizontal lines, so y1 and y2 should be equal for Y-cuts.
  double x1 = numeric_limits<double>::max();
  double y1 = numeric_limits<double>::max();
  double x2 = numeric_limits<double>::min();
  double y2 = numeric_limits<double>::min();

  // A boolean flag indicating whether or not this cut was actually chosen by the `PageSegmentator`
  // class or `ReadingOrderDetector` class.
  // NOTE: Here are some background information. The `XYCut` class computes cut candidates (that
  // is, basically, cuts that do not overlap any elements of the page currently processed) and
  // passes these candidates to the `PageSegmentator` class (resp. `ReadingOrderDetector` class).
  // These classes choose one or more cut candidates which should be actually made to segment the
  // page (resp. to detect the reading order).
  // As long as `isChosen` is set to false, the cut is considered to be a candidate cut.
  bool isChosen = false;

  // The element before this cut. When this cut is an X-cut, this is the element with the largest
  // rightX that is located to the left of this cut. When this cut is an Y-cut, this is the element
  // with the largest lowerY that is located above this cut.
  const PdfElement* elementBefore = nullptr;

  // The element after the cut. When this cut is an X-cut, this is the element with the smallest
  // leftX that is located to the right of this cut. When this cut is an Y-cut, this is the element
  // with the smallest upperY that is located below this cut.
  const PdfElement* elementAfter = nullptr;

  // The gap width, that is: the horizontal gap between elementBefore and elementAfter
  // (= elementAfter.leftX - elementBefore.rightX) when the cut is an X-cut; 0.0 otherwise.
  double gapWidth = 0.0;

  // The gap height, that is: the vertical gap between elementBefore and elementAfter
  // (= elementAfter.upperY - elementBefore.lowerY) when the cut is an Y-cut; 0.0 otherwise.
  double gapHeight = 0.0;

  // The page elements overlapped by this cut.
  // NOTE: Initially, we required that a cut must *not* overlap any page elements in order to be
  // considered as a cut candidate. Normally, this is a reasonable requirement, since we do not
  // want to split a page through the middle of a text block. However, there are PDFs with
  // multi-column layouts that contain text lines which accidentally extend beyond the actual
  // column boundaries (and extend into another column). The consequence was that the respective
  // page could not be split into columns, since there is no cut candidate that does *not* overlap
  // a page element. For this reason, we now allow a cut to overlap a certain number of page
  // elements. The exact number depends on the cut length.
  vector<PdfElement*> overlappingElements;

  // The page elements to be split are given in a vector. The parameter `posInElements` is the
  // position of the cut in this vector. It can be understood as follows: If set to value i, the
  // cut splits the elements in vector V between V[i-1] and V[i].
  size_t posInElements = 0;
};

// =================================================================================================

/**
 * This class represents the position of an element in a PDF.
 */
class PdfPosition {
 public:
  /** This constructor creates and initializes a new instance of this class. */
  PdfPosition();

  /** The deconstructor. */
  ~PdfPosition();

  // The number of the page on which the element is located.
  double pageNum = -1;

  // The coordinates of the bounding box of the element, relatively to the page's upper left.
  // NOTE: The bounding box is the smallest rectangle that completely surrounds the element.
  double leftX = numeric_limits<double>::max();
  double upperY = numeric_limits<double>::max();
  double rightX = numeric_limits<double>::min();
  double lowerY = numeric_limits<double>::min();

  // The rotation of the element.
  int rotation = 0;

  // The writing mode of the element.
  int wMode = 0;

  /**
   * This method returns the width of the bounding box.
   *
   * @return
   *    The width of the bounding box.
   */
  double getWidth() const;

  /**
   * This method returns the height of the bounding box.
   *
   * @return
   *    The height of the bounding box.
   */
  double getHeight() const;

  /**
   * This method returns the (logically correct) leftX coordinate of the element, under
   * consideration of the rotation. The returned coordinate is relatively to an alternative
   * coordinate system that is rotated compared to the original coordinate system of the page such
   * that the element appears to be *not* rotated in the alternative coordinate system. For
   * example, if the element is rotated by 90 degrees clockwise, the alternative coordinate system
   * is rotated by 90 degrees *counter*-clockwise. The leftX of the element in the alternative
   * coordinate system is equal to the upperY in the original coordinate system.
   *
   * @return
   *    The (logically correct) leftX of the element, under consideration of the rotation.
   */
  double getRotLeftX() const;

  /**
   * This method returns the (logically correct) upperY coordinate of the element, under
   * consideration of the rotation. The returned coordinate is relatively to an alternative
   * coordinate system that is rotated compared to the original coordinate system of the page such
   * that the element appears to be *not* rotated in the alternative coordinate system. For
   * example, if the element is rotated by 90 degrees clockwise, the alternative coordinate system
   * is rotated by 90 degrees *counter*-clockwise. The upperY of the element in the alternative
   * coordinate system is equal to the rightX in the original coordinate system.
   *
   * @return
   *    The (logically correct) upperY of the element, under consideration of the rotation.
   */
  double getRotUpperY() const;

  /**
   * This method returns the (logically correct) rightX coordinate of the element, under
   * consideration of the rotation. The returned coordinate is relatively to an alternative
   * coordinate system that is rotated compared to the original coordinate system of the page such
   * that the element appears to be *not* rotated in the alternative coordinate system. For
   * example, if the element is rotated by 90 degrees clockwise, the alternative coordinate system
   * is rotated by 90 degrees *counter*-clockwise. The rightX of the element in the alternative
   * coordinate system is equal to the lowerY in the original coordinate system.
   *
   * @return
   *    The (logically correct) rightX of the element, under consideration of the rotation.
   */
  double getRotRightX() const;

  /**
   * This method returns the (logically correct) lowerY coordinate of the element, under
   * consideration of the rotation. The returned coordinate is relatively to an alternative
   * coordinate system that is rotated compared to the original coordinate system of the page such
   * that the element appears to be *not* rotated in the alternative coordinate system. For
   * example, if the element is rotated by 90 degrees clockwise, the alternative coordinate system
   * is rotated by 90 degrees *counter*-clockwise. The lowerY of the element in the alternative
   * coordinate system is equal to the leftX in the original coordinate system.
   *
   * @return
   *    The (logically correct) lowerY of the element, under consideration of the rotation.
   */
  double getRotLowerY() const;

  /**
   * This method returns the width of the element, under consideration of the rotation. Formally,
   * it returns getRotRightX() - getRotLeftX().
   *
   * @return
   *    The width of the element, under consideration of the rotation.
   */
  double getRotWidth() const;

  /**
   * This method returns the height of the element, under consideration of the rotation. Formally,
   * it returns getRotLowerY() - getRotUpperY().
   *
   * @return
   *    The height of the element, under consideration of the rotation.
   */
  double getRotHeight() const;

  /**
   * This method returns a string representation of this position, for debugging purposes.
   *
   * @return
   *    A string representation of this position.
   */
  string toString() const;

  /**
   * This method returns a short string representation of this position, for debugging purposes.
   *
   * @return
   *    A short string representation of this position.
   */
  string toShortString() const;
};

// =================================================================================================

/**
 * This class is an abstract class for all visible elements in a PDF, for example: characters,
 * words, text blocks, graphics, or shapes.
 */
class PdfElement {
 public:
  /** This constructor creates and initializes a new instance of this class. */
  PdfElement();

  /** The (virtual) deconstructor, to be overwritten by the respective subclass. */
  virtual ~PdfElement() = 0;

  // The (unique) id of this element.
  string id;

  // The position of this element in the PDF.
  PdfPosition* pos = nullptr;

  // The rank of this element, that is: its position in the extraction order.
  int rank = -1;

  // A reference to the PDF document of which this element is a part.
  const PdfDocument* doc = nullptr;

  /**
   * This method returns a string representation of this element, for debugging purposes.
   *
   * @return
   *    A string representation of this element.
   */
  virtual string toString() const = 0;

  /**
   * This method returns a short string representation of this element, for debugging purposes.
   *
   * @return
   *    A short string representation of this element.
   */
  virtual string toShortString() const = 0;
};

// =================================================================================================

/**
 * This class is an abstract class for all text elements in a PDF, for example: characters, words,
 * text lines or text blocks.
 */
class PdfTextElement : public PdfElement {
 public:
  /** This constructor creates and initializes a new instance of this class. */
  PdfTextElement();

  /** The (virtual) deconstructor, to be overwritten by the respective subclass. */
  virtual ~PdfTextElement() = 0;

  // The text of this element.
  string text;

  // The font size of this element.
  double fontSize = -1;

  // The font name of this element.
  string fontName;

  // The stroking color of this element, in RGB format (an array of three doubles between 0 and 1).
  double color[3];

  // The opacity of this element, that is: a double in [0,1], with 0="not visible" and 1="opaque".
  double opacity = 1.0;
};

// =================================================================================================

/**
 * This class is an abstract class for all non-text elements in a PDF, for example: graphics and
 * shapes.
 */
class PdfNonTextElement : public PdfElement {
 public:
  /** This constructor creates and initializes a new instance of this class. */
  PdfNonTextElement();

  /** The deconstructor. */
  virtual ~PdfNonTextElement() = 0;
};

// =================================================================================================

/**
 * This class represents a single character of a PDF.
 */
class PdfCharacter : public PdfTextElement {
 public:
  /** This constructor creates and initializes a new instance of this class. */
  PdfCharacter();

  /**
   * This constructor creates and initializes a new instance of this class.
   *
   * @param pageNum
   *    The page number.
   * @param leftX
   *    The leftX coordinate.
   * @param upperY
   *    The upperY coordinate.
   * @param rightX
   *    The rightX coordinate.
   * @param lowerY
   *    The lowerY coordinate.
   * @param rotation
   *    The rotation.
   * @param wMode
   *    The writing mode.
   */
  PdfCharacter(int pageNum, double leftX, double upperY, double rightX, double lowerY,
      int rotation, int wMode);

  /** The deconstructor. */
  ~PdfCharacter();

  // A reference to the word of which this character is a part.
  const PdfWord* word = nullptr;

  // The name of this character, as it is provided by PDF, for example: "A" or "summationdisplay".
  string name;

  // The unicode codepoints of the characters actually represented by this character.
  // NOTE: Usually, there is only one codepoint per character, but there can be more codepoints
  // when the character represents a ligature.
  vector<unsigned int> unicodes;

  // The baseline of this character, that is (according to Wikipedia): the line upon which the
  // character sits and below which a descender character (like "g" and "j") extend.
  double base = 0.0;

  // A boolean flag indicating whether or not this character is subscripted.
  bool isSubscript = false;

  // A boolean flag indicating whether or not this character is superscripted.
  bool isSuperscript = false;

  // A reference to the respective base character, if this character represents a combining
  // diacritical mark.
  const PdfCharacter* isDiacriticMarkOfBaseChar = nullptr;

  // A reference to the combining diacritical mark.
  const PdfCharacter* isBaseCharOfDiacriticMark = nullptr;

  // The text of the character resulted by combining it with the combining diacritical mark.
  string textWithDiacriticMark;

  /**
   * This method returns a string representation of this character, for debugging purposes.
   *
   * @return
   *    A string representation of this character.
   */
  string toString() const override;

  /**
   * This method returns a short string representation of this character, for debugging purposes.
   *
   * @return
   *    A short string representation of this character.
   */
  string toShortString() const override;
};

// =================================================================================================

/**
 * This class represents a single word in a PDF.
 */
class PdfWord : public PdfTextElement {
 public:
  /** This constructor creates and initializes a new instance of this class. */
  PdfWord();

  /**
   * This constructor creates and initializes a new instance of this class.
   *
   * @param pageNum
   *    The page number.
   * @param leftX
   *    The leftX coordinate.
   * @param upperY
   *    The upperY coordinate.
   * @param rightX
   *    The rightX coordinate.
   * @param lowerY
   *    The lowerY coordinate.
   * @param rotation
   *    The rotation.
   * @param wMode
   *    The writing mode.
   */
  PdfWord(int pageNum, double leftX, double upperY, double rightX, double lowerY,
      int rotation, int wMode);

  /** The deconstructor. */
  ~PdfWord();

  // The characters of this word.
  vector<PdfCharacter*> characters;

  // A reference to the text line of which this word is a part.
  const PdfTextLine* line = nullptr;

  // The baseline of this word, that is (according to Wikipedia): the line upon which most of the
  // characters sit and below which descender characters extend.
  double base = 0.0;

  // A reference to the base word, if the word is part of a "stacked math symbol" and the word
  // itself is not the base word. By a "stacked math symbol" we mean a set of words that logically
  // belong together and that are stacked on top of each other. A typical example for a stacked
  // math symbol is a summation symbol with limits positioned below and above the symbol.
  // In "∑_(i=0)^(n)", the "∑" word is the base word of a stacked math symbol, and the "i=0" and
  // "n" words are part of the same stacked math symbol.
  const PdfWord* isPartOfStackedMathSymbol = nullptr;

  // The references to the words that are part of the same stacked math symbol as this word, if
  // this word is the base word of the stacked math symbol. For the example above, if this word is
  // the "∑" word, the vector contains references to the "i=0" and "n" words.
  vector<PdfWord*> isBaseOfStackedMathSymbol;

  // A reference to a word that represents the second part of a hyphenated word, if this word
  // represents the first part of the same hyphenated word.
  // NOTE: Each part of a hyphenated word is first detected as a separate word. The words belonging
  // to the same hyphenated word are merged in a later step.
  const PdfWord* isFirstPartOfHyphenatedWord = nullptr;

  // A reference to a word that represents the first part of a hyphenated word, if this word
  // represents the second part of the same hyphenated word.
  // NOTE: Each part of a hyphenated word is first detected as a separate word. The words belonging
  // to the same hyphenated word are merged in a later step.
  const PdfWord* isSecondPartOfHyphenatedWord = nullptr;

  /**
   * This method returns a string representation of this word, for debugging purposes.
   *
   * @return
   *    A string representation of this word.
   */
  string toString() const override;

  /**
   * This method returns a short string representation of this character, for debugging purposes.
   *
   * @return
   *    A short string representation of this character.
   */
  string toShortString() const override;
};

// =================================================================================================

/**
 * This class represents a single text line in a PDF.
 */
class PdfTextLine : public PdfTextElement {
 public:
  /** This constructor creates and initializes a new instance of this class. */
  PdfTextLine();

  /**
   * This constructor creates and initializes a new instance of this class.
   *
   * @param pageNum
   *    The page number.
   * @param leftX
   *    The leftX coordinate.
   * @param upperY
   *    The upperY coordinate.
   * @param rightX
   *    The rightX coordinate.
   * @param lowerY
   *    The lowerY coordinate.
   * @param rotation
   *    The rotation.
   * @param wMode
   *    The writing mode.
   */
  PdfTextLine(int pageNum, double leftX, double upperY, double rightX, double lowerY,
      int rotation, int wMode);

  /** The deconstructor. */
  ~PdfTextLine();

  // The words of this text line.
  vector<PdfWord*> words;

  // A reference to the segment of which this text line is a part.
  const PdfPageSegment* segment = nullptr;

  // A reference to the text block of which this text line is a part.
  const PdfTextBlock* block = nullptr;

  // The coordinates of the "base bounding box" of this text line.
  // NOTE: The base bounding box is the smallest rectangle that surrounds the characters of this
  // text line that are *not* subscripted or superscripted.
  double baseBBoxLeftX = numeric_limits<double>::max();
  double baseBBoxUpperY = numeric_limits<double>::max();
  double baseBBoxRightX = numeric_limits<double>::min();
  double baseBBoxLowerY = numeric_limits<double>::min();

  // The maximum font size among the characters in this text line.
  double maxFontSize = 0.0;

  // The baseline of this text line, that is (according to Wikipedia): the line upon which most of
  // the characters sit and below which descender characters extend.
  double base = 0.0;

  // The left margin of this text line, that is: the horizontal gap between the left boundary of
  // the text line and the left boundary of the segment.
  double leftMargin = 0.0;

  // The right margin of this text line, that is: the horizontal gap between the right boundary of
  // the text line and the right boundary of the segment.
  double rightMargin = 0.0;

  // A reference to the previous text line in the segment.
  const PdfTextLine* prevLine = nullptr;

  // A reference to the next text line in the segment.
  const PdfTextLine* nextLine = nullptr;

  // A reference to the previous sibling text line. A text line L is the previous sibling text line
  // of text line M if:
  //  (a) L is the nearest previous text line of M with L.leftX == M.leftX
  //  (b) there is no other text line K between L and M with K.leftX < M.leftX.
  //  (c) the line distance between L and M is smaller than a given threshold.
  //  (d) L.lowerY < M.lowerY (meaning that M must be positioned below L).
  // NOTE: The previous sibling line is primarily used to detect enumeration items and references.
  const PdfTextLine* prevSiblingLine = nullptr;

  // A reference to the next sibling text line. A text line L is the next sibling text line of text
  // line M if:
  //  (a) L is the nearest next text line of M with L.leftX == M.leftX
  //  (b) there is no other text line K between M and L with K.leftX < M.leftX.
  //  (c) the line distance between L and M is smaller than a given threshold.
  //  (d) L.lowerY < M.lowerY (meaning that M must be positioned below L).
  // NOTE: The next sibling line is primarily used to detect enumeration items and references.
  const PdfTextLine* nextSiblingLine = nullptr;

  // A reference to the parent text line. A text line L is the parent text line of text line M if:
  //  (a) L is the nearest previous text line of M with L.leftX < M.leftX (meaning that M is
  //      indented compared to L).
  //  (b) the line distance between L and M is smaller than a given threshold.
  //  (c) L.lowerY < M.lowerY (meaning that M must be positioned below L).
  // NOTE: The parent line is primarily used to detect enumerations and references.
  const PdfTextLine* parentLine = nullptr;

  /**
   * This method returns a string representation of this text line, for debugging purposes.
   *
   * @return
   *    A string representation of this text line.
   */
  string toString() const override;

  /**
   * This method returns a short string representation of this character, for debugging purposes.
   *
   * @return
   *    A short string representation of this character.
   */
  string toShortString() const override;
};

// =================================================================================================

/**
 * This class represents a single text block in a PDF.
 *
 * A text block is a group of text that logically belong together, that is recognizably set off
 * from other text blocks, and that play a specific semantic role (e.g., "title", or "heading",
 * or "paragraph", or "footnote").
 */
class PdfTextBlock : public PdfTextElement {
 public:
  /** This constructor creates and initializes a new instance of this class. */
  PdfTextBlock();

  /** The deconstructor. */
  ~PdfTextBlock();

  // The text lines of this text block.
  vector<PdfTextLine*> lines;

  // A reference to the segment of which this text block is a part.
  const PdfPageSegment* segment = nullptr;

  // The semantic role of this text block.
  SemanticRole role;

  // A boolean flag indicating whether or not the text lines in this text block are centered.
  bool isLinesCentered = false;

  // A boolean flag indicating whether or not this text block is emphasized compared to the
  // majority of other text blocks in the same PDF.
  bool isEmphasized = false;

  // If this text block is in hanging indent format (meaning that the first line of the text block
  // is indented and the continuation lines are indented by a certain amount): The amount by which
  // the continuation lines are indented; 0.0 otherwise.
  // NOTE: This can be used to check if the block is in hanging indent format, by checking if
  // hangingIndent > 0.0.
  double hangingIndent = 0.0;

  // The coordinates of the trim box of this text block.
  // NOTE: A text line can accidentally extend beyond the actual segment boundaries (and extend
  // into other segments). The trim box is the bounding box around the lines of the segment that
  // do *not* extend beyond the actual segment boundaries.
  double trimLeftX = numeric_limits<double>::max();
  double trimUpperY = numeric_limits<double>::max();
  double trimRightX = numeric_limits<double>::min();
  double trimLowerY = numeric_limits<double>::min();

  // A reference to the previous text block in the document.
  const PdfTextBlock* prevBlock = nullptr;

  // A reference to the next text block in the document.
  const PdfTextBlock* nextBlock = nullptr;

  /**
   * This method returns a string representation of this text block, for debugging purposes.
   *
   * @return
   *    A string representation of this text block, for debugging purposes.
   */
  string toString() const override;

  /**
   * This method returns a short string representation of this character, for debugging purposes.
   *
   * @return
   *    A short string representation of this character.
   */
  string toShortString() const override;
};

// =================================================================================================

/**
 * This class represents a single figure in a PDF, that is: a collection of characters, graphics,
 * and shapes that logically belong together (and that belong to the same figure).
 *
 * NOTE: A `PdfFigure` is not to be confused with a `PdfGraphic`. A `PdfGraphic` represents a
 * single image in the PDF (e.g., a jpg or eps). A `PdfFigure` can consist of multiple images.
 */
class PdfFigure : public PdfNonTextElement {
 public:
  /** This constructor creates and initializes a new instance of this class. */
  PdfFigure();

  /**
   * This constructor creates and initializes a new instance of this class.
   *
   * @param pageNum
   *    The page number.
   * @param leftX
   *    The leftX coordinate.
   * @param upperY
   *    The upperY coordinate.
   * @param rightX
   *    The rightX coordinate.
   * @param lowerY
   *    The lowerY coordinate.
   */
  PdfFigure(int pageNum, double leftX, double upperY, double rightX, double lowerY);

  /** The deconstructor. */
  ~PdfFigure();

  // The characters of this figure.
  vector<PdfCharacter*> characters;

  // The shapes of this figure.
  vector<PdfShape*> shapes;

  // The graphics of this figure.
  vector<PdfGraphic*> graphics;

  // The coordinates of the clip box of this figure.
  // NOTE: The clip box of a figure can be seen as the "visible part" of an figure in the PDF.
  // All elements (or parts of elements) outside the clip box are not visible in the PDF.
  // The clip box is usually larger than the bounding box, since the visible parts do not
  // necessarily have to be adjacent to the clip box.
  double clipLeftX = numeric_limits<double>::max();
  double clipUpperY = numeric_limits<double>::max();
  double clipRightX = numeric_limits<double>::min();
  double clipLowerY = numeric_limits<double>::min();

  /**
   * This method returns a string representation of this figure, for debugging purposes.
   *
   * @return
   *    A string representation of this figure.
   */
  string toString() const override;

  /**
   * This method returns a short string representation of this character, for debugging purposes.
   *
   * @return
   *    A short string representation of this character.
   */
  string toShortString() const override;
};

// =================================================================================================

/**
 * This class represents a single shape in a PDF, for example: a line or a curve.
 */
class PdfShape : public PdfNonTextElement {
 public:
  /** This constructor creates and initializes a new instance of this class. */
  PdfShape();

  /** The deconstructor. */
  ~PdfShape();

  /**
   * This method returns a string representation of this shape, for debugging purposes.
   *
   * @return
   *    A string representation of this shape.
   */
  string toString() const override;

  /**
   * This method returns a short string representation of this character, for debugging purposes.
   *
   * @return
   *    A short string representation of this character.
   */
  string toShortString() const override;
};

// =================================================================================================

/**
 * This class represents a single image in a PDF, for example: a jpg or eps.
 *
 * NOTE: A `PdfGraphic` is not to be confused with a `PdfFigure`. A `PdfFigure` is a collection of
 * multiple elements (e.g., multiple images). A `PdfGraphic` is a single image.
 */
class PdfGraphic : public PdfNonTextElement {
 public:
  /** This constructor creates and initializes a new instance of this class. */
  PdfGraphic();

  /** The deconstructor. */
  ~PdfGraphic();

  /**
   * This method returns a string representation of this graphic, for debugging purposes.
   *
   * @return
   *    A string representation of this graphic.
   */
  string toString() const override;

  /**
   * This method returns a short string representation of this character, for debugging purposes.
   *
   * @return
   *    A short string representation of this character.
   */
  string toShortString() const override;
};

// =================================================================================================

/**
 * This class represents a single segment of a page in a PDF. A segment is created by the
 * `PageSegmentator` class and contains all elements of a page that the `PageSegmentator`
 * considers to be part of the same column.
 *
 * The difference between a `PdfPageSegment` and a `PdfTextBlock` is that a `PdfPageSegment` can
 * contain the text elements belonging to multiple text blocks. For example, if the heading and the
 * paragraphs of a section is arranged in the same column, the respective `PageSegment` consists of
 * the text elements of all these text blocks. A `PdfTextBlock` contains only those text elements
 * that logically belong together and that play a specific role in the document.
 */
class PdfPageSegment : public PdfElement {
 public:
  /** This constructor creates and initializes a new instance of this class. */
  PdfPageSegment();

  /** The deconstructor. */
  ~PdfPageSegment();

  // The elements of this segment.
  vector<PdfElement*> elements;

  // The text lines of this segment, detected from the elements.
  vector<PdfTextLine*> lines;

  // The text blocks of this segment, detected from the text lines.
  vector<PdfTextBlock*> blocks;

  // The coordinates of the trim box of this segment.
  // NOTE: A text line can accidentally extend beyond the actual segment boundaries. The trim box is
  // the bounding box around the lines of the segment that do *not* extend beyond the actual
  // segment boundaries.
  double trimLeftX = numeric_limits<double>::max();
  double trimUpperY = numeric_limits<double>::max();
  double trimRightX = numeric_limits<double>::min();
  double trimLowerY = numeric_limits<double>::min();

  /**
   * This method returns a string representation of this segment, for debugging purposes.
   *
   * @return
   *    A string representation of this segment.
   */
  string toString() const override;

  /**
   * This method returns a short string representation of this character, for debugging purposes.
   *
   * @return
   *    A short string representation of this character.
   */
  string toShortString() const override;
};

// =================================================================================================

/**
 * This class represents a single page of a PDF.
 */
class PdfPage {
 public:
  /** This constructor creates and initializes a new instance of this class. */
  PdfPage();

  /** The deconstructor. */
  ~PdfPage();

  // The characters of this page.
  vector<PdfCharacter*> characters;

  // The figures of this page.
  vector<PdfFigure*> figures;

  // The shapes of this page.
  vector<PdfShape*> shapes;

  // The graphics of this page.
  vector<PdfGraphic*> graphics;

  // The words of this page.
  vector<PdfWord*> words;

  // The text lines of this page.
  vector<PdfTextLine*> textLines;

  // The text blocks of this page.
  vector<PdfTextBlock*> blocks;

  // The segments of this page.
  vector<PdfPageSegment*> segments;

  // The page number.
  int pageNum = -1;

  // The coordinates of the clip box of this page.
  // NOTE: The clip box of a page can be seen as the "visible part" of the page in the PDF.
  // All elements (or parts of elements) outside the clip box are not visible in the PDF.
  double clipLeftX = numeric_limits<double>::max();
  double clipUpperY = numeric_limits<double>::max();
  double clipRightX = numeric_limits<double>::min();
  double clipLowerY = numeric_limits<double>::min();

  // The XY-cuts made to detect the text blocks on this page.
  vector<Cut*> blockDetectionCuts;

  // The XY-cuts made to detect the reading order of the text blocks on this page.
  vector<Cut*> readingOrderCuts;

  /**
   * This method returns the width of this page.
   *
   * @return
   *    The width of this page.
   */
  double getWidth() const;

  /**
   * This method returns the height of this page.
   *
   * @return
   *    The height of this page.
   */
  double getHeight() const;
};

// =================================================================================================

/**
 * This class represents a PDF document.
 */
class PdfDocument {
 public:
  /** This constructor creates and initializes a new instance of this class. */
  PdfDocument();

  /** The deconstructor. */
  ~PdfDocument();

  // The pages of this PDF document.
  vector<PdfPage*> pages;

  // A dictionary that maps font names to their respective `PdfFontInfo` objects (each providing
  // further information about the font, for example: whether or not the font is a bold font.
  unordered_map<string, PdfFontInfo*> fontInfos;

  // The average character width in this PDF document.
  double avgCharWidth = 0.0;

  // The average character height in this PDF document.
  double avgCharHeight = 0.0;

  // The most frequent font size among the characters in this PDF document.
  double mostFreqFontSize = 0.0;

  // The name of the most frequent font among the characters in this PDF document.
  string mostFreqFontName;

  // The most frequent horizontal gap between the words in this PDF document.
  double mostFreqWordDistance = 0.0;

  // The most frequent line distance in this PDF document, estimated by analyzing the vertical
  // gaps between the words (this is needed for tasks that need to be executed before text lines
  // were detected).
  double mostFreqEstimatedLineDistance = 0.0;

  // The most frequent line distance in this PDF document, computed by analyzing the vertical
  // gaps between the text lines (this value is usually more exact than
  // mostFreqEstimatedLineDistance).
  double mostFreqLineDistance = 0.0;

  // The most frequent line distances in this PDF document, per font sizes. The entry stored at
  // mostFreqLineDistancePerFontSize[x] denotes the most frequent line distance between two
  // consecutive lines with font size x.
  unordered_map<double, double> mostFreqLineDistancePerFontSize = {};

  // The most frequent word height in this PDF document.
  double mostFreqWordHeight = 0.0;

  // The most frequent left margin of the text lines in this PDF document.
  double mostFreqLineLeftMargin = 0.0;

  // The path to the PDF file.
  string pdfFilePath;
};

#endif  // PDFDOCUMENT_H_
