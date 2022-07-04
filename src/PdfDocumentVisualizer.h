/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef PDFDOCUMENTVISUALIZER_H_
#define PDFDOCUMENTVISUALIZER_H_

#include <poppler/Annot.h>
#include <poppler/PDFDoc.h>

#include <memory>  // std::unique_ptr
#include <string>
#include <vector>

#include "./PdfDocument.h"
#include "./TextOutputDev.h"

using std::string;
using std::unique_ptr;
using std::vector;

// =================================================================================================

// TODO(korzen): XXX
struct ColorScheme {
  ColorScheme(const AnnotColor& primaryColorA, const AnnotColor& secondaryColorA,
      const AnnotColor& tertiaryColorA) {
    primaryColor = primaryColorA;
    secondaryColor = secondaryColorA;
    tertiaryColor = tertiaryColorA;
  }

  AnnotColor primaryColor;
  AnnotColor secondaryColor;
  AnnotColor tertiaryColor;
};

// =================================================================================================
// CONFIG

namespace visualizer::config {

// The resolution in DPI.
const double RESOLUTION = 72.0;

// The appearance of a semantic role.
// "/Helv" is the font name (= Helvetica), "7" is the font size, "0 0 1" is the color (= blue).
const GooString SEMANTIC_ROLE_APPEARANCE("/Helv 7 Tf 0 0 1 rg");

// The width of a line that connects consecutive text blocks (wrt. the reading order). */
const double READING_ORDER_LINE_WIDTH = 4.0;

// The radius of a circle containing a reading order index.
const double READING_ORDER_CIRCLE_RADIUS = 5;

// The appearance of a reading order index (= the number in a reading order circle).
const GooString READING_ORDER_INDEX_APPEARANCE("/Helv 7 Tf 1 1 1 rg");

// The width of a line that represents an XY-cut.
const double CUT_WIDTH = 2.0;

// The font appearance of a cut index.
const GooString CUT_INDEX_APPEARANCE("/Helv 7 Tf 1 1 1 rg");

// The radius of a square containing a cut index.
const double CUT_SQUARE_RADIUS = 5;

// The font appearance of a cut id.
const GooString CUT_ID_APPEARANCE("/Helv 3 Tf .9 .9 .9 rg");

}  // namespace visualizer::config

// =================================================================================================

namespace visualizer::colors {

// TODO(korzen): XXX
const AnnotColor r1(1, 0, 0);
const AnnotColor r2(1, 0.5, 0.5);
const AnnotColor r3(1, 0.7, 0.7);
const AnnotColor g1(0, 1, 0);
const AnnotColor g2(0.5, 1, 0.5);
const AnnotColor g3(0.7, 1, 0.7);
const AnnotColor b1(0, 0, 1);
const AnnotColor b2(0.5, 0.5, 1);
const AnnotColor b3(0.7, 0.7, 1);
const AnnotColor gr1(0.7, 0.7, 0.7);
const AnnotColor gr2(0.8, 0.8, 0.8);
const AnnotColor gr3(0.9, 0.9, 0.9);

}  // namespace visualizer::colors

// =================================================================================================

namespace visualizer::color_schemes {

const ColorScheme red(colors::r1, colors::r2, colors::r3);
const ColorScheme green(colors::g1, colors::g2, colors::g3);
const ColorScheme blue(colors::b1, colors::b2, colors::b3);
const ColorScheme gray(colors::gr1, colors::gr2, colors::gr3);

}  // namespace visualizer::color_schemes

// =================================================================================================

/**
 * This class creates a visualization of a `PdfDocument`, that is: a copy of the belonging PDF
 * file, with annotations added for debugging purposes; for example: the bounding boxes of
 * the extracted character, words or text blocks; or the semantic roles of the text blocks.
 */
class PdfDocumentVisualizer {
 public:
  /**
   * This constructor creates and initializes a new instance of this class
   *
   * @param pdfFilePath
   *   The path to the PDF file to which the annotations should be added.
   */
  explicit PdfDocumentVisualizer(const string& pdfFilePath);

  /** The deconstructor. */
  ~PdfDocumentVisualizer();

  /**
   * TODO This method visualizes the extracted characters by drawing their bounding boxes to the
   * visualization.
   */
  void visualizeCharacters(const PdfDocument& doc, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the extracted characters by drawing their bounding boxes to the
   * visualization.
   */
  void visualizeCharacters(const vector<PdfCharacter*>& chars, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the extracted non-text elements by drawing their bounding boxes to
   * the visualization.
   */
  void visualizeFigures(const PdfDocument& doc, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the extracted non-text elements by drawing their bounding boxes to
   * the visualization.
   */
  void visualizeFigures(const vector<PdfFigure*>& figures, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the extracted non-text elements by drawing their bounding boxes to
   * the visualization.
   */
  void visualizeShapes(const PdfDocument& doc, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the extracted non-text elements by drawing their bounding boxes to
   * the visualization.
   */
  void visualizeShapes(const vector<PdfShape*>& shapes, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the extracted non-text elements by drawing their bounding boxes to
   * the visualization.
   */
  void visualizeGraphics(const PdfDocument& doc, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the extracted non-text elements by drawing their bounding boxes to
   * the visualization.
   */
  void visualizeGraphics(const vector<PdfGraphic*>& graphics, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the extracted words by drawing their bounding boxes to the
   * visualization.
   */
  void visualizeWords(const PdfDocument& doc, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the extracted words by drawing their bounding boxes to the
   * visualization.
   */
  void visualizeWords(const vector<PdfWord*>& words, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the extracted words by drawing their bounding boxes to the
   * visualization.
   */
  void visualizeTextLines(const PdfDocument& doc, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the extracted words by drawing their bounding boxes to the
   * visualization.
   */
  void visualizeTextLines(const vector<PdfTextLine*>& lines, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the extracted text blocks by drawing (1) their bounding boxes, (2)
   * their semantic roles and (3) the XY-cuts made to detect the text blocks to the visualization.
   */
  void visualizeTextBlocks(const PdfDocument& doc, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the extracted text blocks by drawing (1) their bounding boxes, (2)
   * their semantic roles and (3) the XY-cuts made to detect the text blocks to the visualization.
   */
  void visualizeTextBlocks(const vector<PdfTextBlock*>& blocks, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the extracted text blocks by drawing (1) their bounding boxes, (2)
   * their semantic roles and (3) the XY-cuts made to detect the text blocks to the visualization.
   */
  void visualizePageSegments(const PdfDocument& doc, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the extracted text blocks by drawing (1) their bounding boxes, (2)
   * their semantic roles and (3) the XY-cuts made to detect the text blocks to the visualization.
   */
  void visualizePageSegments(const vector<PdfPageSegment*>& blocks, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the detected reading order by drawing (1) lines between
   * consecutive text blocks and (2) the index of each text block in the reading order.
   */
  void visualizeReadingOrder(const PdfDocument& doc, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the detected reading order by drawing (1) lines between
   * consecutive text blocks and (2) the index of each text block in the reading order.
   */
  void visualizeReadingOrder(const vector<PdfTextBlock*>& blocks, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the detected reading order by drawing (1) lines between
   * consecutive text blocks and (2) the index of each text block in the reading order.
   */
  void visualizeSegmentCuts(const PdfDocument& doc, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the detected reading order by drawing (1) lines between
   * consecutive text blocks and (2) the index of each text block in the reading order.
   */
  void visualizeTextBlockDetectionCuts(const vector<Cut*>& cuts, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the detected reading order by drawing (1) lines between
   * consecutive text blocks and (2) the index of each text block in the reading order.
   */
  void visualizeReadingOrderCuts(const PdfDocument& doc, const ColorScheme& cs) const;

  /**
   * TODO This method visualizes the detected reading order by drawing (1) lines between
   * consecutive text blocks and (2) the index of each text block in the reading order.
   */
  void visualizeReadingOrderCuts(const vector<Cut*>& cuts, const ColorScheme& cs) const;

  /**
   * This method writes the visualization (= the PDF with the added annotations) to the given file
   * path.
   *
   * @param targetPath
   *    The path to the file to which the visualization should be written.
   */
  void save(const string& targetPath) const;

 private:
  /**
   * TODO(korzen): This method draws the bounding boxes of the non-text elements (e.g., figures and
   * shapes) stored in `_doc` to the visualization.
   */
  void drawCharBoundingBoxes(const vector<PdfCharacter*>& chars, const ColorScheme& cs) const;

  /**
   * TODO(korzen): This method draws the bounding boxes of the non-text elements (e.g., figures and
   * shapes) stored in `_doc` to the visualization.
   */
  void drawFigureBoundingBoxes(const vector<PdfFigure*>& figures, const ColorScheme& cs) const;

  /**
   * TODO(korzen): This method draws the bounding boxes of the non-text elements (e.g., figures and
   * shapes) stored in `_doc` to the visualization.
   */
  void drawShapeBoundingBoxes(const vector<PdfShape*>& shapes, const ColorScheme& cs) const;

  /**
   * TODO(korzen): This method draws the bounding boxes of the non-text elements (e.g., figures and
   * shapes) stored in `_doc` to the visualization.
   */
  void drawGraphicBoundingBoxes(const vector<PdfGraphic*>& graphics, const ColorScheme& cs) const;

  /**
   * TODO(korzen): This method draws the bounding boxes of the words stored in `_doc` to the
   * visualization.
   */
  void drawWordBoundingBoxes(const vector<PdfWord*>& words, const ColorScheme& cs) const;

  /**
   * TODO(korzen): This method draws the bounding boxes of the non-text elements (e.g., figures and
   * shapes) stored in `_doc` to the visualization.
   */
  void drawTextLineBoundingBoxes(const vector<PdfTextLine*>& lines, const ColorScheme& cs) const;

  /**
   * TODO This method draws the bounding boxes of the text blocks stored in `_doc` to the
   * visualization.
   */
  void drawTextBlockBoundingBoxes(const vector<PdfTextBlock*>& blocks, const ColorScheme& cs) const;

  /**
   * TODO This method draws the bounding boxes of the text blocks stored in `_doc` to the
   * visualization.
   */
  void drawPageSegmentBoundingBoxes(const vector<PdfPageSegment*>& segments,
      const ColorScheme& cs) const;

  /**
   * TODO(korzen): This method draws the bounding boxes of the characters stored in `_doc` to the
   * visualization.
   */
  void drawBoundingBox(const PdfElement* element, const ColorScheme& cs) const;

  /**
   * TODO This method draws the semantic roles of the text blocks stored in `_doc` to the
   * visualization.
   */
  void drawTextBlockSemanticRoles(const vector<PdfTextBlock*>& blocks, const ColorScheme& cs) const;

  /**
   * This methods draws (1) lines between consecutive text blocks (with respect to the reading
   * order and (2) the index of each text block in the reading order.
   */
  void drawReadingOrder(const vector<PdfTextBlock*>& blocks, const ColorScheme& cs) const;

  /**
   * This methods draws the circle at the end of a reading order line, containing the index of the
   * respective text block in the reading order.
   *
   * @param page
   *   The page to which the circle should be added.
   * @param gfx
   *   The Gfx engine of the page.
   * @param x
   *   The x-coordinate of the midpoint of the circle to draw.
   * @param y
   *   The y-coordinate of the midpoint of the circle to draw.
   * @param blockIndex
   *   The index of the respective text block in the reading order.
   * @param cs
   *   TODO
   */
  void drawReadingOrderIndexCircle(Page* page, Gfx* gfx, double x, double y, int blockIndex,
      const ColorScheme& cs) const;

  /**
   * TODO This method draws the given XY-cuts (made to detect the text blocks or the reading order of
   * text blocks) to the visualization.
   */
  void drawCuts(const vector<Cut*>& cuts, const ColorScheme& cs) const;

  /**
   * This method converts the given string to an UTF-16 string, which is required so that a string
   * of an AnnotText or AnnotFreeText is drawn correctly to the PDF.
   *
   * @param str
   *    The string to convert.
   *
   * @return
   *    The string converted to UTF-16.
   */
  GooString* convertToUtf16(GooString* str) const;

  // The PDF document to process.
  unique_ptr<PDFDoc> _pdfDoc;

  // The document to process.
  PdfDocument* _doc;

  // TODO(korzen): XXX
  vector<Gfx*> _gfxs;

  // TODO(korzen): XXX
  TextOutputDev* _out;
};

#endif  // PDFDOCUMENTVISUALIZER_H_
