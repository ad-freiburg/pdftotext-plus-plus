/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef PDFDOCUMENTVISUALIZER_H_
#define PDFDOCUMENTVISUALIZER_H_

#include <memory>
#include <string>
#include <vector>

#include <poppler/Annot.h>
#include <poppler/PDFDoc.h>

#include "./PdfDocument.h"
#include "./TextOutputDev.h"

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
const ColorScheme red(r1, r2, r3);
const ColorScheme green(g1, g2, g3);
const ColorScheme blue(b1, b2, b3);
const ColorScheme gray(gr1, gr2, gr3);

// =================================================================================================

/**
 * This class creates a visualization of a `PdfDocument`, that is: a copy of the belonging PDF
 * file, with annotations added for debugging purposes; for example: the bounding boxes of
 * the extracted character, words or text blocks; or the semantic roles of the text blocks.
 */
class PdfDocumentVisualizer {
 public:
  /**
   * TODO This constructor creates and initializes a new `PdfDocumentVisualizer` object.
   *
   * @param doc
   *   The `PdfDocument` providing the elements (for example, the characters, words and text blocks)
   *   extracted from the given PDF document.
   */
  explicit PdfDocumentVisualizer(std::string pdfFilePath);

  /**
   * The deconstructor.
   */
  ~PdfDocumentVisualizer();

  /**
   * TODO This method visualizes the extracted characters by drawing their bounding boxes to the
   * visualization.
   */
  void visualizeCharacters(const PdfDocument& doc, const ColorScheme& cs);

  /**
   * TODO This method visualizes the extracted characters by drawing their bounding boxes to the
   * visualization.
   */
  void visualizeCharacters(const std::vector<PdfCharacter*>& chars, const ColorScheme& cs);

  /**
   * TODO This method visualizes the extracted non-text elements by drawing their bounding boxes to
   * the visualization.
   */
  void visualizeFigures(const PdfDocument& doc, const ColorScheme& cs);

  /**
   * TODO This method visualizes the extracted non-text elements by drawing their bounding boxes to
   * the visualization.
   */
  void visualizeFigures(const std::vector<PdfFigure*>& figures, const ColorScheme& cs);

  /**
   * TODO This method visualizes the extracted non-text elements by drawing their bounding boxes to
   * the visualization.
   */
  void visualizeShapes(const PdfDocument& doc, const ColorScheme& cs);

  /**
   * TODO This method visualizes the extracted non-text elements by drawing their bounding boxes to
   * the visualization.
   */
  void visualizeShapes(const std::vector<PdfShape*>& shapes, const ColorScheme& cs);

  /**
   * TODO This method visualizes the extracted non-text elements by drawing their bounding boxes to
   * the visualization.
   */
  void visualizeGraphics(const PdfDocument& doc, const ColorScheme& cs);

  /**
   * TODO This method visualizes the extracted non-text elements by drawing their bounding boxes to
   * the visualization.
   */
  void visualizeGraphics(const std::vector<PdfGraphic*>& graphics, const ColorScheme& cs);

  /**
   * TODO This method visualizes the extracted words by drawing their bounding boxes to the
   * visualization.
   */
  void visualizeWords(const PdfDocument& doc, const ColorScheme& cs);

  /**
   * TODO This method visualizes the extracted words by drawing their bounding boxes to the
   * visualization.
   */
  void visualizeWords(const std::vector<PdfWord*>& words, const ColorScheme& cs);

  /**
   * TODO This method visualizes the extracted words by drawing their bounding boxes to the
   * visualization.
   */
  void visualizeTextLines(const PdfDocument& doc, const ColorScheme& cs);

  /**
   * TODO This method visualizes the extracted words by drawing their bounding boxes to the
   * visualization.
   */
  void visualizeTextLines(const std::vector<PdfTextLine*>& lines, const ColorScheme& cs);

  /**
   * TODO This method visualizes the extracted text blocks by drawing (1) their bounding boxes, (2)
   * their semantic roles and (3) the XY-cuts made to detect the text blocks to the visualization.
   */
  void visualizeTextBlocks(const PdfDocument& doc, const ColorScheme& cs);

  /**
   * TODO This method visualizes the extracted text blocks by drawing (1) their bounding boxes, (2)
   * their semantic roles and (3) the XY-cuts made to detect the text blocks to the visualization.
   */
  void visualizeTextBlocks(const std::vector<PdfTextBlock*>& blocks, const ColorScheme& cs);

  /**
   * TODO This method visualizes the extracted text blocks by drawing (1) their bounding boxes, (2)
   * their semantic roles and (3) the XY-cuts made to detect the text blocks to the visualization.
   */
  void visualizePageSegments(const PdfDocument& doc, const ColorScheme& cs);

  /**
   * TODO This method visualizes the extracted text blocks by drawing (1) their bounding boxes, (2)
   * their semantic roles and (3) the XY-cuts made to detect the text blocks to the visualization.
   */
  void visualizePageSegments(const std::vector<PdfPageSegment*>& blocks, const ColorScheme& cs);

  /**
   * TODO This method visualizes the detected reading order by drawing (1) lines between
   * consecutive text blocks and (2) the index of each text block in the reading order.
   */
  void visualizeReadingOrder(const PdfDocument& doc, const ColorScheme& cs);

  /**
   * TODO This method visualizes the detected reading order by drawing (1) lines between
   * consecutive text blocks and (2) the index of each text block in the reading order.
   */
  void visualizeReadingOrder(const std::vector<PdfTextBlock*>& blocks, const ColorScheme& cs);

  /**
   * TODO This method visualizes the detected reading order by drawing (1) lines between
   * consecutive text blocks and (2) the index of each text block in the reading order.
   */
  void visualizeTextBlockDetectionCuts(const PdfDocument& doc, const ColorScheme& cs);

  /**
   * TODO This method visualizes the detected reading order by drawing (1) lines between
   * consecutive text blocks and (2) the index of each text block in the reading order.
   */
  void visualizeTextBlockDetectionCuts(const std::vector<Cut*>& cuts, const ColorScheme& cs);

  /**
   * TODO This method visualizes the detected reading order by drawing (1) lines between
   * consecutive text blocks and (2) the index of each text block in the reading order.
   */
  void visualizeReadingOrderCuts(const PdfDocument& doc, const ColorScheme& cs);

  /**
   * TODO This method visualizes the detected reading order by drawing (1) lines between
   * consecutive text blocks and (2) the index of each text block in the reading order.
   */
  void visualizeReadingOrderCuts(const std::vector<Cut*>& cuts, const ColorScheme& cs);

  /**
   * This method writes the visualization (= the PDF with the added annotations) to the given file
   * path.
   *
   * @param targetPath
   *    The path to the file to which the visualization should be written.
   */
  void save(const std::string& targetPath);

 private:
  /**
   * TODO(korzen): This method draws the bounding boxes of the non-text elements (e.g., figures and
   * shapes) stored in `_doc` to the visualization.
   */
  void drawCharBoundingBoxes(const std::vector<PdfCharacter*>& chars, const ColorScheme& cs);

  /**
   * TODO(korzen): This method draws the bounding boxes of the non-text elements (e.g., figures and
   * shapes) stored in `_doc` to the visualization.
   */
  void drawFigureBoundingBoxes(const std::vector<PdfFigure*>& figures, const ColorScheme& cs);

  /**
   * TODO(korzen): This method draws the bounding boxes of the non-text elements (e.g., figures and
   * shapes) stored in `_doc` to the visualization.
   */
  void drawShapeBoundingBoxes(const std::vector<PdfShape*>& shapes, const ColorScheme& cs);

  /**
   * TODO(korzen): This method draws the bounding boxes of the non-text elements (e.g., figures and
   * shapes) stored in `_doc` to the visualization.
   */
  void drawGraphicBoundingBoxes(const std::vector<PdfGraphic*>& graphics, const ColorScheme& cs);

  /**
   * TODO(korzen): This method draws the bounding boxes of the words stored in `_doc` to the
   * visualization.
   */
  void drawWordBoundingBoxes(const std::vector<PdfWord*>& words, const ColorScheme& cs);

  /**
   * TODO(korzen): This method draws the bounding boxes of the non-text elements (e.g., figures and
   * shapes) stored in `_doc` to the visualization.
   */
  void drawTextLineBoundingBoxes(const std::vector<PdfTextLine*>& lines, const ColorScheme& cs);

  /**
   * TODO This method draws the bounding boxes of the text blocks stored in `_doc` to the
   * visualization.
   */
  void drawTextBlockBoundingBoxes(const std::vector<PdfTextBlock*>& blocks, const ColorScheme& cs);

  /**
   * TODO This method draws the bounding boxes of the text blocks stored in `_doc` to the
   * visualization.
   */
  void drawPageSegmentBoundingBoxes(const std::vector<PdfPageSegment*>& segments,
      const ColorScheme& cs);

  /**
   * TODO(korzen): This method draws the bounding boxes of the characters stored in `_doc` to the
   * visualization.
   */
  void drawBoundingBox(const PdfElement* element, const ColorScheme& cs);

  /**
   * TODO This method draws the semantic roles of the text blocks stored in `_doc` to the
   * visualization.
   */
  void drawTextBlockSemanticRoles(const std::vector<PdfTextBlock*>& blocks, const ColorScheme& cs);

  /**
   * This methods draws (1) lines between consecutive text blocks (with respect to the reading
   * order and (2) the index of each text block in the reading order.
   */
  void drawReadingOrder(const std::vector<PdfTextBlock*>& blocks, const ColorScheme& cs);

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
   */
  void drawReadingOrderIndexCircle(Page* page, Gfx* gfx, double x, double y, int blockIndex,
      const ColorScheme& cs);

  /**
   * This method draws the given XY-cuts (made to detect the text blocks or the reading order of
   * text blocks) to the visualization.
   */
  void drawCuts(const std::vector<Cut*>& cuts, const ColorScheme& cs);

  /**
   * This method converts the given string to an UTF-16 string, which is required so that a string
   * of an AnnotText or AnnotFreeText is drawn correctly to the PDF.
   *
   * @param The string to convert.
   *
   * @return The string converted to UTF-16.
   */
  GooString* convertToUtf16(GooString* str);

  /** The PDF document to process. */
  std::unique_ptr<PDFDoc> _pdfDoc;
  /** The document to process. */
  PdfDocument* _doc;

  std::vector<Gfx*> _gfxs;

  TextOutputDev* _out;
};

#endif  // PDFDOCUMENTVISUALIZER_H_
