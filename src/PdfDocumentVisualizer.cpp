/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cmath>  // fabs
#include <iostream>
#include <string>

#include <poppler/Annot.h>
#include <goo/GooString.h>
#include <poppler/PDFDoc.h>
#include <poppler/PDFDocEncoding.h>
#include <poppler/PDFDocFactory.h>

#include "./PdfDocument.h"
#include "./PdfDocumentVisualizer.h"
#include "./TextOutputDev.h"


/** The resolution in DPI. */
static double resolution = 72.0;

/**
 * The appearance of a semantic role.
 * "/Helv" is the font name (= Helvetica), "7" is the font size, "0 0 1" is the color (= blue).
 */
static GooString semanticRoleAppearance("/Helv 7 Tf 0 0 1 rg");

/** The width of a line that connects consecutive text blocks (wrt. the reading order). */
static double readingOrderLineWidth = 4.0;

/** The radius of a circle containing a reading order index. */
static double readingOrderCircleRadius = 5;

/** The appearance of a reading order index (= the number in a reading order circle). */
static GooString readingOrderIndexAppearance("/Helv 7 Tf 1 1 1 rg");

/** The width of a line that represents a XY-cut. */
static double cutWidth = 2.0;

/** The font appearance of a cut index. */
static GooString cutIndexAppearance("/Helv 7 Tf 1 1 1 rg");

/** The radius of a square containing a cut index. */
static double cutSquareRadius = 5;

// =================================================================================================

// _________________________________________________________________________________________________
PdfDocumentVisualizer::PdfDocumentVisualizer(std::string pdfFilePath) {
  // Load the PDF document.
  GooString gooPdfFilePath(pdfFilePath);
  _pdfDoc = PDFDocFactory().createPDFDoc(gooPdfFilePath);

  _doc = new PdfDocument();
  _out = new TextOutputDev(true, _doc);

  // Create a Gfx for each PDF page.
  _gfxs.push_back(nullptr);  // Make the vector 1-based.
  for (int i = 1; i <= _pdfDoc->getNumPages(); i++) {
    Page* page = _pdfDoc->getPage(i);
    Gfx* gfx = page->createGfx(_out, resolution, resolution, 0, true, false, -1, -1, -1, -1,
        true, nullptr, nullptr, nullptr);
    _gfxs.push_back(gfx);
  }
}

// _________________________________________________________________________________________________
PdfDocumentVisualizer::~PdfDocumentVisualizer() {
  // TODO(korzen):
  // for (size_t i = 0; i < _gfxs.size(); i++) {
  //   delete _gfxs[i];
  // }
  delete _doc;
  delete _out;
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeGlyphs(const PdfDocument& doc, const ColorScheme& cs) {
  for (const auto* page : doc.pages) {
    drawGlyphBoundingBoxes(page->glyphs, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeGlyphs(const std::vector<PdfGlyph*>& glyphs,
    const ColorScheme& cs) {
  drawGlyphBoundingBoxes(glyphs, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeFigures(const PdfDocument& doc, const ColorScheme& cs) {
  for (const auto* page : doc.pages) {
    drawFigureBoundingBoxes(page->figures, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeFigures(const std::vector<PdfFigure*>& figures,
    const ColorScheme& cs) {
  drawFigureBoundingBoxes(figures, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeShapes(const PdfDocument& doc, const ColorScheme& cs) {
  for (const auto* page : doc.pages) {
    drawShapeBoundingBoxes(page->shapes, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeShapes(const std::vector<PdfShape*>& shapes,
    const ColorScheme& cs) {
  drawShapeBoundingBoxes(shapes, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeWords(const PdfDocument& doc, const ColorScheme& cs) {
  for (const auto* page : doc.pages) {
    drawWordBoundingBoxes(page->words, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeWords(const std::vector<PdfWord*>& words,
    const ColorScheme& cs) {
  drawWordBoundingBoxes(words, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeTextLines(const PdfDocument& doc, const ColorScheme& cs) {
  for (const auto* page : doc.pages) {
    for (const auto* segment : page->segments) {
      drawTextLineBoundingBoxes(segment->lines, cs);
    }
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeTextLines(const std::vector<PdfTextLine*>& lines,
    const ColorScheme& cs) {
  drawTextLineBoundingBoxes(lines, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeTextBlocks(const PdfDocument& doc, const ColorScheme& cs) {
  for (const auto* page : doc.pages) {
    drawTextBlockBoundingBoxes(page->blocks, cs);
    drawTextBlockSemanticRoles(page->blocks, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeTextBlocks(const std::vector<PdfTextBlock*>& blocks,
    const ColorScheme& cs) {
  drawTextBlockBoundingBoxes(blocks, cs);
  drawTextBlockSemanticRoles(blocks, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizePageSegments(const PdfDocument& doc,
    const ColorScheme& cs) {
  for (const auto* page : doc.pages) {
    drawPageSegmentBoundingBoxes(page->segments, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizePageSegments(const std::vector<PdfPageSegment*>& segments,
    const ColorScheme& cs) {
  drawPageSegmentBoundingBoxes(segments, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeReadingOrder(const PdfDocument& doc,
    const ColorScheme& cs) {
  for (const auto* page : doc.pages) {
    drawTextBlockBoundingBoxes(page->blocks, cs);
    drawTextBlockSemanticRoles(page->blocks, cs);
    drawReadingOrder(page->blocks, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeReadingOrder(const std::vector<PdfTextBlock*>& blocks,
    const ColorScheme& cs) {
  drawTextBlockBoundingBoxes(blocks, cs);
  drawTextBlockSemanticRoles(blocks, cs);
  drawReadingOrder(blocks, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeTextBlockDetectionCuts(const PdfDocument& doc,
    const ColorScheme& cs) {
  for (const auto* page : doc.pages) {
    drawCuts(page->blockDetectionCuts, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeTextBlockDetectionCuts(const std::vector<Cut*>& cuts,
    const ColorScheme& cs) {
  drawCuts(cuts, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeReadingOrderCuts(const PdfDocument& doc,
    const ColorScheme& cs) {
  for (const auto* page : doc.pages) {
    drawCuts(page->readingOrderCuts, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeReadingOrderCuts(const std::vector<Cut*>& cuts,
    const ColorScheme& cs) {
  drawCuts(cuts, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::save(const std::string& targetPath) {
  GooString gooTargetPath(targetPath);
  _pdfDoc->saveAs(&gooTargetPath);
}

// =================================================================================================

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawGlyphBoundingBoxes(const std::vector<PdfGlyph*>& glyphs,
    const ColorScheme& cs) {
  for (const auto* glyph : glyphs) {
    drawBoundingBox(glyph, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawFigureBoundingBoxes(const std::vector<PdfFigure*>& figures,
    const ColorScheme& cs) {
  for (const auto* figure : figures) {
    drawBoundingBox(figure, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawShapeBoundingBoxes(const std::vector<PdfShape*>& shapes,
    const ColorScheme& cs) {
  for (const auto* shape : shapes) {
    drawBoundingBox(shape, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawWordBoundingBoxes(const std::vector<PdfWord*>& words,
    const ColorScheme& cs) {
  for (const auto* word : words) {
    drawBoundingBox(word, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawTextLineBoundingBoxes(const std::vector<PdfTextLine*>& lines,
    const ColorScheme& cs) {
  for (const auto* line : lines) {
    drawBoundingBox(line, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawTextBlockBoundingBoxes(const std::vector<PdfTextBlock*>& blocks,
    const ColorScheme& cs) {
  for (const auto* block : blocks) {
    drawBoundingBox(block, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawPageSegmentBoundingBoxes(
    const std::vector<PdfPageSegment*>& segments, const ColorScheme& cs) {
  for (const auto* segment : segments) {
    drawBoundingBox(segment, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawBoundingBox(const PdfElement* element, const ColorScheme& cs) {
  Page* pdfPage = _pdfDoc->getPage(element->position->pageNum);
  Gfx* gfx = _gfxs[element->position->pageNum];

  // Define the coordinates of the bounding box to draw.
  double leftX = element->position->leftX;
  double upperY = pdfPage->getMediaHeight() - element->position->lowerY;  // make it relative to the lower left.
  double rightX = element->position->rightX;
  double lowerY = pdfPage->getMediaHeight() - element->position->upperY;  // make it relative to the lower left.
  // Vertical/horizontal lines can have a width/height of zero, in which case they are not
  // visible in the visualization. So ensure a minimal width/height of 1.
  if (fabs(leftX - rightX) < 1) { rightX += 1; }
  if (fabs(upperY - lowerY) < 1) { lowerY += 1; }
  PDFRectangle rect(leftX, upperY, rightX, lowerY);

  // Create the bounding box.
  AnnotGeometry* annot = new AnnotGeometry(_pdfDoc.get(), &rect, Annot::AnnotSubtype::typeSquare);

  // Define the color of the bounding box.
  std::unique_ptr<AnnotColor> color = std::make_unique<AnnotColor>(cs.primaryColor);
  annot->setColor(std::move(color));

  // Draw the bounding box.
  pdfPage->addAnnot(annot);
  annot->draw(gfx, false);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawTextBlockSemanticRoles(const std::vector<PdfTextBlock*>& blocks,
    const ColorScheme& cs) {
  // Iterate through the text blocks and draw the semantic role of each.
  for (const auto* block : blocks) {
    Page* pdfPage = _pdfDoc->getPage(block->position->pageNum);
    Gfx* gfx = _gfxs[block->position->pageNum];

    // Define the position of the semantic role.
    double leftX = block->position->leftX;
    double lowerY = pdfPage->getMediaHeight() - block->position->upperY;  // make it relative to the lower left.
    PDFRectangle rect(leftX, lowerY, leftX + 100, lowerY + 7);

    // Define the font appearance of the semantic role.
    const DefaultAppearance appearance(&semanticRoleAppearance);

    // Create the annotation.
    AnnotFreeText* annot = new AnnotFreeText(_pdfDoc.get(), &rect, appearance);

    // Define the text of the annotation (= the semantic role).
    GooString role(block->role);
    annot->setContents(convertToUtf16(&role));

    // Remove the default border around the annotation.
    std::unique_ptr<AnnotBorder> border(new AnnotBorderArray());
    border->setWidth(0);
    annot->setBorder(std::move(border));

    // Draw the annotation.
    pdfPage->addAnnot(annot);
    annot->draw(gfx, false);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawReadingOrder(const std::vector<PdfTextBlock*>& blocks,
    const ColorScheme& cs) {
  // Iterate through the text blocks and draw a line between the current and previous block.
  for (size_t i = 1; i < blocks.size(); i++) {
    PdfTextBlock* prevBlock = blocks[i - 1];
    PdfTextBlock* block = blocks[i];

    Page* pdfPage = _pdfDoc->getPage(block->position->pageNum);
    Gfx* gfx = _gfxs[block->position->pageNum];

    // Compute the coordinates of the midpoints of the previous and current text block.
    double prevMinY = pdfPage->getMediaHeight() - prevBlock->position->lowerY;
    double prevMaxY = pdfPage->getMediaHeight() - prevBlock->position->upperY;
    double prevMidX = prevBlock->position->leftX + ((prevBlock->position->rightX - prevBlock->position->leftX) / 2.0);
    double prevMidY = prevMinY + ((prevMaxY - prevMinY) / 2.0);
    double currMinY = pdfPage->getMediaHeight() - block->position->lowerY;
    double currMaxY = pdfPage->getMediaHeight() - block->position->upperY;
    double currMidX = block->position->leftX + ((block->position->rightX - block->position->leftX) / 2.0);
    double currMidY = currMinY + ((currMaxY - currMinY) / 2.0);

    // Define the position of the reading order line.
    PDFRectangle lineRect(prevMidX, prevMidY, currMidX, currMidY);
    AnnotLine* lineAnnot = new AnnotLine(_pdfDoc.get(), &lineRect);
    lineAnnot->setVertices(prevMidX, prevMidY, currMidX, currMidY);

    // Define the width of the reading order line.
    std::unique_ptr<AnnotBorder> lineBorder(new AnnotBorderArray());
    lineBorder->setWidth(readingOrderLineWidth);
    lineAnnot->setBorder(std::move(lineBorder));

    // Define the color of the reading order line.
    auto lineColor = std::make_unique<AnnotColor>(cs.secondaryColor);
    lineAnnot->setColor(std::move(lineColor));

    // Draw the reading order line.
    pdfPage->addAnnot(lineAnnot);
    lineAnnot->draw(gfx, false);

    // -----------

    // Draw the circle at the midpoint of the previous block, with the reading order index.
    drawReadingOrderIndexCircle(pdfPage, gfx, prevMidX, prevMidY, i, cs);

    // Draw the circle at the midpoint of the last block.
    if (i == blocks.size() - 1) {
      drawReadingOrderIndexCircle(pdfPage, gfx, currMidX, currMidY, i + 1, cs);
    }
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawReadingOrderIndexCircle(Page* page, Gfx* gfx, double x,
    double y, int readingOrderIndex, const ColorScheme& cs) {
  // Define the position of the circle.
  PDFRectangle circleRect(
    x - readingOrderCircleRadius, y - readingOrderCircleRadius,
    x + readingOrderCircleRadius, y + readingOrderCircleRadius);
  AnnotGeometry* circleAnnot = new AnnotGeometry(_pdfDoc.get(), &circleRect,
    Annot::AnnotSubtype::typeCircle);

  // Define the stroking color of the circle.
  auto circleStrokingColor = std::make_unique<AnnotColor>(cs.primaryColor);
  circleAnnot->setColor(std::move(circleStrokingColor));

  // Define the filling color of the circle.
  auto circleFillingColor = std::make_unique<AnnotColor>(cs.primaryColor);
  circleAnnot->setInteriorColor(std::move(circleFillingColor));

  // Draw the circle.
  page->addAnnot(circleAnnot);
  circleAnnot->draw(gfx, false);

  // --------

  // Define the appearance of the reading order index within the circle.
  const DefaultAppearance indexAppearance(&readingOrderIndexAppearance);

  // Define the position of the index.
  PDFRectangle indexRect(
    x - readingOrderCircleRadius, y - readingOrderCircleRadius,
    x + readingOrderCircleRadius, y + readingOrderCircleRadius * 0.6);
  AnnotFreeText* indexAnnot = new AnnotFreeText(_pdfDoc.get(), &indexRect, indexAppearance);

  // Define the text of the annot (= the reading order index).
  GooString index(std::to_string(readingOrderIndex));
  indexAnnot->setContents(convertToUtf16(&index));
  // Center the text horizontally.
  indexAnnot->setQuadding(AnnotFreeText::AnnotFreeTextQuadding::quaddingCentered);

  // Remove the default border around the reading order index.
  std::unique_ptr<AnnotBorder> indexBorder(new AnnotBorderArray());
  indexBorder->setWidth(0);
  indexAnnot->setBorder(std::move(indexBorder));

  // Draw the reading order index.
  page->addAnnot(indexAnnot);
  indexAnnot->draw(gfx, false);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawCuts(const std::vector<Cut*>& cuts, const ColorScheme& cs) {
  // Iterate through the cuts and visualize each.
  for (size_t i = 0; i < cuts.size(); i++) {
    const auto* cut = cuts[i];

    Page* pdfPage = _pdfDoc->getPage(cut->pageNum);
    Gfx* gfx = _gfxs[cut->pageNum];

    double x1 = cut->x1;
    double y1 = pdfPage->getMediaHeight() - cut->y1;  // make it relative to the lower left.
    double x2 = cut->x2;
    double y2 = pdfPage->getMediaHeight() - cut->y2;  // make it relative to the lower left.

    // ==========
    // Draw a line representing the cut.

    // Define the position of the line.
    PDFRectangle lineRect(x1, y1, x2, y2);
    AnnotLine* lineAnnot = new AnnotLine(_pdfDoc.get(), &lineRect);
    lineAnnot->setVertices(x1, y1, x2, y2);

    // Define the line width.
    std::unique_ptr<AnnotBorder> lineBorder(new AnnotBorderArray());
    lineBorder->setWidth(cutWidth);
    lineAnnot->setBorder(std::move(lineBorder));

    // Define the line color.
    auto lineColor = std::make_unique<AnnotColor>(cs.tertiaryColor);
    lineAnnot->setColor(std::move(lineColor));

    // Draw the line.
    pdfPage->addAnnot(lineAnnot);
    lineAnnot->draw(gfx, false);

    // ==========
    // Draw a square at the beginning of the line, containing the cut index.

    // Define the position of the square.
    PDFRectangle squareRect(
      x1 - cutSquareRadius, y1 - cutSquareRadius,
      x1 + cutSquareRadius, y1 + cutSquareRadius);
    AnnotGeometry* squareAnnot = new AnnotGeometry(_pdfDoc.get(), &squareRect,
        Annot::AnnotSubtype::typeSquare);

    // Define the stroking color of the square.
    auto squareStrokingColor = std::make_unique<AnnotColor>(cs.secondaryColor);
    squareAnnot->setColor(std::move(squareStrokingColor));

    // Define the filling color of the square.
    auto squareFillingColor = std::make_unique<AnnotColor>(cs.secondaryColor);
    squareAnnot->setInteriorColor(std::move(squareFillingColor));

    // Draw the square.
    pdfPage->addAnnot(squareAnnot);
    squareAnnot->draw(gfx, false);

    // ----------

    // Define the appearance of the cut index.
    const DefaultAppearance indexAppearance(&cutIndexAppearance);

    // Define the position of the cut index.
    PDFRectangle indexRect(
      x1 - cutSquareRadius, y1 - cutSquareRadius,
      x1 + cutSquareRadius, y1 + cutSquareRadius * 0.6);
    AnnotFreeText* indexAnnot = new AnnotFreeText(_pdfDoc.get(), &indexRect, indexAppearance);

    // Define the text of the annot (= the cut index).
    GooString index(std::to_string(i + 1));
    indexAnnot->setContents(convertToUtf16(&index));
    // Center the text horizontally.
    indexAnnot->setQuadding(AnnotFreeText::AnnotFreeTextQuadding::quaddingCentered);

    // Remove the default border around the cut index.
    std::unique_ptr<AnnotBorder> indexBorder(new AnnotBorderArray());
    indexBorder->setWidth(0);
    indexAnnot->setBorder(std::move(indexBorder));

    // Draw the cut index.
    pdfPage->addAnnot(indexAnnot);
    indexAnnot->draw(gfx, false);
  }
}

// _________________________________________________________________________________________________
GooString* PdfDocumentVisualizer::convertToUtf16(GooString* str) {
  int length = 2 + 2 * str->getLength();
  char* result = new char[length];
  // Add unicode markers.
  result[0] = static_cast<char>(0xfe);
  result[1] = static_cast<char>(0xff);
  // Convert to UTF-16.
  for (int i = 2, j = 0; i < length; i += 2, j++) {
    unsigned char character = static_cast<unsigned char>(str->getChar(j));
    Unicode u = pdfDocEncoding[static_cast<unsigned int>(character)] & 0xffff;
    result[i] = (u >> 8) & 0xff;
    result[i + 1] = u & 0xff;
  }
  // delete str;
  str = new GooString(result, length);
  // delete[] result;
  return str;
}
