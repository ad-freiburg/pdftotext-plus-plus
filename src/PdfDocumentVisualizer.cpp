/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <goo/GooString.h>
#include <poppler/Annot.h>
#include <poppler/PDFDoc.h>
#include <poppler/PDFDocEncoding.h>
#include <poppler/PDFDocFactory.h>

#include <cmath>  // fabs
#include <memory>  // std::unique_ptr
#include <string>
#include <utility>  // std::move
#include <vector>

#include "./PdfDocument.h"
#include "./PdfDocumentVisualizer.h"
#include "./TextOutputDev.h"

using std::make_unique;
using std::move;
using std::string;
using std::unique_ptr;
using std::vector;


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

/** The font appearance of a cut id. */
static GooString cutIdAppearance("/Helv 3 Tf .9 .9 .9 rg");

// =================================================================================================

// _________________________________________________________________________________________________
PdfDocumentVisualizer::PdfDocumentVisualizer(const string& pdfFilePath) {
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
void PdfDocumentVisualizer::visualizeCharacters(const PdfDocument& doc, const ColorScheme& cs)
    const {
  for (const auto* page : doc.pages) {
    drawCharBoundingBoxes(page->characters, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeCharacters(const vector<PdfCharacter*>& chars,
    const ColorScheme& cs) const {
  drawCharBoundingBoxes(chars, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeFigures(const PdfDocument& doc, const ColorScheme& cs) const {
  for (const auto* page : doc.pages) {
    drawFigureBoundingBoxes(page->figures, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeFigures(const vector<PdfFigure*>& figures,
    const ColorScheme& cs) const {
  drawFigureBoundingBoxes(figures, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeShapes(const PdfDocument& doc, const ColorScheme& cs) const {
  for (const auto* page : doc.pages) {
    drawShapeBoundingBoxes(page->shapes, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeShapes(const vector<PdfShape*>& shapes,
    const ColorScheme& cs) const {
  drawShapeBoundingBoxes(shapes, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeGraphics(const PdfDocument& doc, const ColorScheme& cs) const {
  for (const auto* page : doc.pages) {
    drawGraphicBoundingBoxes(page->graphics, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeGraphics(const vector<PdfGraphic*>& graphics,
    const ColorScheme& cs) const {
  drawGraphicBoundingBoxes(graphics, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeWords(const PdfDocument& doc, const ColorScheme& cs) const {
  for (const auto* page : doc.pages) {
    drawWordBoundingBoxes(page->words, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeWords(const vector<PdfWord*>& words,
    const ColorScheme& cs) const {
  drawWordBoundingBoxes(words, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeTextLines(const PdfDocument& doc, const ColorScheme& cs)
    const {
  for (const auto* page : doc.pages) {
    for (const auto* segment : page->segments) {
      drawTextLineBoundingBoxes(segment->lines, cs);
    }
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeTextLines(const vector<PdfTextLine*>& lines,
    const ColorScheme& cs) const {
  drawTextLineBoundingBoxes(lines, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeTextBlocks(const PdfDocument& doc, const ColorScheme& cs)
      const {
  for (const auto* page : doc.pages) {
    drawTextBlockBoundingBoxes(page->blocks, cs);
    drawTextBlockSemanticRoles(page->blocks, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeTextBlocks(const vector<PdfTextBlock*>& blocks,
    const ColorScheme& cs) const {
  drawTextBlockBoundingBoxes(blocks, cs);
  drawTextBlockSemanticRoles(blocks, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizePageSegments(const PdfDocument& doc,
    const ColorScheme& cs) const {
  for (const auto* page : doc.pages) {
    drawPageSegmentBoundingBoxes(page->segments, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizePageSegments(const vector<PdfPageSegment*>& segments,
    const ColorScheme& cs) const {
  drawPageSegmentBoundingBoxes(segments, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeReadingOrder(const PdfDocument& doc,
    const ColorScheme& cs) const {
  for (const auto* page : doc.pages) {
    drawTextBlockBoundingBoxes(page->blocks, cs);
    drawTextBlockSemanticRoles(page->blocks, cs);
    drawReadingOrder(page->blocks, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeReadingOrder(const vector<PdfTextBlock*>& blocks,
    const ColorScheme& cs) const {
  drawTextBlockBoundingBoxes(blocks, cs);
  drawTextBlockSemanticRoles(blocks, cs);
  drawReadingOrder(blocks, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeSegmentCuts(const PdfDocument& doc,
    const ColorScheme& cs) const {
  for (const auto* page : doc.pages) {
    drawCuts(page->blockDetectionCuts, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeTextBlockDetectionCuts(const vector<Cut*>& cuts,
    const ColorScheme& cs) const {
  drawCuts(cuts, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeReadingOrderCuts(const PdfDocument& doc,
    const ColorScheme& cs) const {
  for (const auto* page : doc.pages) {
    drawCuts(page->readingOrderCuts, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::visualizeReadingOrderCuts(const vector<Cut*>& cuts,
    const ColorScheme& cs) const {
  drawCuts(cuts, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::save(const string& targetPath) const {
  GooString gooTargetPath(targetPath);
  _pdfDoc->saveAs(&gooTargetPath);
}

// =================================================================================================

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawCharBoundingBoxes(const vector<PdfCharacter*>& characters,
    const ColorScheme& cs) const {
  for (const auto* ch : characters) {
    drawBoundingBox(ch, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawFigureBoundingBoxes(const vector<PdfFigure*>& figures,
    const ColorScheme& cs) const {
  for (const auto* figure : figures) {
    drawBoundingBox(figure, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawShapeBoundingBoxes(const vector<PdfShape*>& shapes,
    const ColorScheme& cs) const {
  for (const auto* shape : shapes) {
    drawBoundingBox(shape, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawGraphicBoundingBoxes(const vector<PdfGraphic*>& graphics,
    const ColorScheme& cs) const {
  for (const auto* graphic : graphics) {
    drawBoundingBox(graphic, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawWordBoundingBoxes(const vector<PdfWord*>& words,
    const ColorScheme& cs) const {
  for (const auto* word : words) {
    drawBoundingBox(word, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawTextLineBoundingBoxes(const vector<PdfTextLine*>& lines,
    const ColorScheme& cs) const {
  for (const auto* line : lines) {
    drawBoundingBox(line, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawTextBlockBoundingBoxes(const vector<PdfTextBlock*>& blocks,
    const ColorScheme& cs) const {
  for (const auto* block : blocks) {
    drawBoundingBox(block, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawPageSegmentBoundingBoxes(
    const vector<PdfPageSegment*>& segments, const ColorScheme& cs) const {
  for (const auto* segment : segments) {
    drawBoundingBox(segment, cs);

    // Draw the trim box of the segment.
    const ColorScheme& cs1 = green;
    Page* pdfPage = _pdfDoc->getPage(segment->position->pageNum);
    Gfx* gfx = _gfxs[segment->position->pageNum];
    double leftX = segment->trimLeftX;
    double upperY = pdfPage->getMediaHeight() - segment->trimLowerY;
    double rightX = segment->trimRightX;
    double lowerY = pdfPage->getMediaHeight() - segment->trimUpperY;
    // Vertical/horizontal lines can have a width/height of zero, in which case they are not
    // visible in the visualization. So ensure a minimal width/height of 1.
    if (fabs(leftX - rightX) < 1) { rightX += 1; }
    if (fabs(upperY - lowerY) < 1) { lowerY += 1; }
    PDFRectangle rect(leftX, upperY, rightX, lowerY);

    // Create the bounding box.
    AnnotGeometry* annot = new AnnotGeometry(_pdfDoc.get(), &rect, Annot::AnnotSubtype::typeSquare);

    // Define the color of the bounding box.
    std::unique_ptr<AnnotColor> color = make_unique<AnnotColor>(cs1.primaryColor);
    annot->setColor(move(color));

    // Draw the bounding box.
    pdfPage->addAnnot(annot);
    annot->draw(gfx, false);

    // Draw the (preliminary) text blocks stored in the segment
    for (const auto* block : segment->blocks) {
      const ColorScheme& cs2 = red;
      Page* pdfPage = _pdfDoc->getPage(segment->position->pageNum);
      Gfx* gfx = _gfxs[segment->position->pageNum];
      double leftX = block->position->leftX;
      double upperY = pdfPage->getMediaHeight() - block->position->lowerY;
      double rightX = block->position->rightX;
      double lowerY = pdfPage->getMediaHeight() - block->position->upperY;
      // Vertical/horizontal lines can have a width/height of zero, in which case they are not
      // visible in the visualization. So ensure a minimal width/height of 1.
      if (fabs(leftX - rightX) < 1) { rightX += 1; }
      if (fabs(upperY - lowerY) < 1) { lowerY += 1; }
      PDFRectangle rect(leftX, upperY, rightX, lowerY);

      // Create the bounding box.
      AnnotGeometry* a = new AnnotGeometry(_pdfDoc.get(), &rect, Annot::AnnotSubtype::typeSquare);

      // Define the color of the bounding box.
      std::unique_ptr<AnnotColor> color = make_unique<AnnotColor>(cs2.primaryColor);
      a->setColor(move(color));

      // Draw the bounding box.
      pdfPage->addAnnot(a);
      a->draw(gfx, false);
    }
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawBoundingBox(const PdfElement* element, const ColorScheme& cs)
      const {
  Page* pdfPage = _pdfDoc->getPage(element->position->pageNum);
  Gfx* gfx = _gfxs[element->position->pageNum];

  // Define the coordinates of the bounding box to draw. Make the y-coordinates relatively to the
  // lower left of the page.
  double leftX = element->position->leftX;
  double upperY = pdfPage->getMediaHeight() - element->position->lowerY;
  double rightX = element->position->rightX;
  double lowerY = pdfPage->getMediaHeight() - element->position->upperY;
  // Vertical/horizontal lines can have a width/height of zero, in which case they are not
  // visible in the visualization. So ensure a minimal width/height of 1.
  if (fabs(leftX - rightX) < 1) { rightX += 1; }
  if (fabs(upperY - lowerY) < 1) { lowerY += 1; }
  PDFRectangle rect(leftX, upperY, rightX, lowerY);

  // Create the bounding box.
  AnnotGeometry* annot = new AnnotGeometry(_pdfDoc.get(), &rect, Annot::AnnotSubtype::typeSquare);

  // Define the color of the bounding box.
  std::unique_ptr<AnnotColor> color = make_unique<AnnotColor>(cs.primaryColor);
  annot->setColor(move(color));

  // Draw the bounding box.
  pdfPage->addAnnot(annot);
  annot->draw(gfx, false);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawTextBlockSemanticRoles(const vector<PdfTextBlock*>& blocks,
    const ColorScheme& cs) const {
  // Iterate through the text blocks and draw the semantic role of each.
  for (const auto* block : blocks) {
    Page* pdfPage = _pdfDoc->getPage(block->position->pageNum);
    Gfx* gfx = _gfxs[block->position->pageNum];

    // Define the position of the semantic role. Make the lowerY relatively to the lower left of
    // the page.
    double leftX = block->position->leftX;
    double lowerY = pdfPage->getMediaHeight() - block->position->upperY;
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
    annot->setBorder(move(border));

    // Draw the annotation.
    pdfPage->addAnnot(annot);
    annot->draw(gfx, false);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawReadingOrder(const vector<PdfTextBlock*>& blocks,
    const ColorScheme& cs) const {
  // Iterate through the text blocks and draw a line between the current and previous block.
  for (size_t i = 1; i < blocks.size(); i++) {
    PdfTextBlock* prevBlock = blocks[i - 1];
    PdfTextBlock* currBlock = blocks[i];

    Page* pdfPage = _pdfDoc->getPage(currBlock->position->pageNum);
    Gfx* gfx = _gfxs[currBlock->position->pageNum];

    // Compute the coordinates of the midpoints of the previous and current text block.
    double prevMinX = prevBlock->position->leftX;
    double prevMaxX = prevBlock->position->rightX;
    double prevMinY = pdfPage->getMediaHeight() - prevBlock->position->lowerY;
    double prevMaxY = pdfPage->getMediaHeight() - prevBlock->position->upperY;
    double prevMidX = prevBlock->position->leftX + ((prevMaxX - prevMinX) / 2.0);
    double prevMidY = prevMinY + ((prevMaxY - prevMinY) / 2.0);
    double currMinX = currBlock->position->leftX;
    double currMaxX = currBlock->position->rightX;
    double currMinY = pdfPage->getMediaHeight() - currBlock->position->lowerY;
    double currMaxY = pdfPage->getMediaHeight() - currBlock->position->upperY;
    double currMidX = currBlock->position->leftX + ((currMaxX - currMinX) / 2.0);
    double currMidY = currMinY + ((currMaxY - currMinY) / 2.0);

    // Define the position of the reading order line.
    PDFRectangle lineRect(prevMidX, prevMidY, currMidX, currMidY);
    AnnotLine* lineAnnot = new AnnotLine(_pdfDoc.get(), &lineRect);
    lineAnnot->setVertices(prevMidX, prevMidY, currMidX, currMidY);

    // Define the width of the reading order line.
    std::unique_ptr<AnnotBorder> lineBorder(new AnnotBorderArray());
    lineBorder->setWidth(readingOrderLineWidth);
    lineAnnot->setBorder(move(lineBorder));

    // Define the color of the reading order line.
    auto lineColor = make_unique<AnnotColor>(cs.secondaryColor);
    lineAnnot->setColor(move(lineColor));

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
    double y, int readingOrderIndex, const ColorScheme& cs) const {
  // Define the position of the circle.
  PDFRectangle circleRect(
    x - readingOrderCircleRadius, y - readingOrderCircleRadius,
    x + readingOrderCircleRadius, y + readingOrderCircleRadius);
  AnnotGeometry* circleAnnot = new AnnotGeometry(_pdfDoc.get(), &circleRect,
    Annot::AnnotSubtype::typeCircle);

  // Define the stroking color of the circle.
  auto circleStrokingColor = make_unique<AnnotColor>(cs.primaryColor);
  circleAnnot->setColor(move(circleStrokingColor));

  // Define the filling color of the circle.
  auto circleFillingColor = make_unique<AnnotColor>(cs.primaryColor);
  circleAnnot->setInteriorColor(move(circleFillingColor));

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
  indexAnnot->setBorder(move(indexBorder));

  // Draw the reading order index.
  page->addAnnot(indexAnnot);
  indexAnnot->draw(gfx, false);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualizer::drawCuts(const vector<Cut*>& cuts, const ColorScheme& cs) const {
  size_t chosenCutIndex = 0;
  // Iterate through the cuts and visualize each.
  for (size_t i = 0; i < cuts.size(); i++) {
    const auto* cut = cuts[i];

    const ColorScheme& cos = cut->isChosen ? cs : gray;

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
    lineAnnot->setBorder(move(lineBorder));

    // Define the line color.
    auto lineColor = make_unique<AnnotColor>(cos.tertiaryColor);
    lineAnnot->setColor(move(lineColor));

    // Draw the line.
    pdfPage->addAnnot(lineAnnot);
    lineAnnot->draw(gfx, false);

    if (cut->isChosen) {
      // ==========
      // Draw a square at the beginning of the line, containing the cut index.

      // Define the position of the square.
      PDFRectangle squareRect(
        x1 - cutSquareRadius, y1 - cutSquareRadius,
        x1 + cutSquareRadius, y1 + cutSquareRadius);
      AnnotGeometry* squareAnnot = new AnnotGeometry(_pdfDoc.get(), &squareRect,
          Annot::AnnotSubtype::typeSquare);

      // Define the stroking color of the square.

      auto squareStrokingColor = make_unique<AnnotColor>(cos.secondaryColor);
      squareAnnot->setColor(move(squareStrokingColor));

      // Define the filling color of the square.
      auto squareFillingColor = make_unique<AnnotColor>(cos.secondaryColor);
      squareAnnot->setInteriorColor(move(squareFillingColor));

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

      // Define the text of the annot (= the how many-th chosen cut the index is).
      GooString index(std::to_string(++chosenCutIndex));
      indexAnnot->setContents(convertToUtf16(&index));
      // Center the text horizontally.
      indexAnnot->setQuadding(AnnotFreeText::AnnotFreeTextQuadding::quaddingCentered);

      // Remove the default border around the cut index.
      std::unique_ptr<AnnotBorder> indexBorder(new AnnotBorderArray());
      indexBorder->setWidth(0);
      indexAnnot->setBorder(move(indexBorder));

      // Draw the cut index.
      pdfPage->addAnnot(indexAnnot);
      indexAnnot->draw(gfx, false);
    }

    // ==========
    // Draw the id of the cut.

    // Define the appearance of the id.
    const DefaultAppearance idAppearance(&cutIdAppearance);

    // Define the position of the id.
    double rectWidth = 30;
    double rectHeight = 15;
    double rectMinX = cut->dir == CutDir::X ? x2 - (rectWidth / 2.0) : x2 - rectWidth;
    double rectMinY = cut->dir == CutDir::X ? y2 - rectHeight : y2;
    double rectMaxX = rectMinX + rectWidth;
    double rectMaxY = rectMinY + rectHeight;
    PDFRectangle idRect(rectMinX, rectMinY, rectMaxX, rectMaxY);
    AnnotFreeText* idAnnot = new AnnotFreeText(_pdfDoc.get(), &idRect, idAppearance);

    // Define the text of the annot (= the id).
    GooString id(cut->id);
    idAnnot->setContents(convertToUtf16(&id));
    // Center the text horizontally.
    idAnnot->setQuadding(AnnotFreeText::AnnotFreeTextQuadding::quaddingCentered);

    // Remove the default border around the cut id.
    // std::unique_ptr<AnnotBorder> idBorder(new AnnotBorderArray());
    // idBorder->setWidth(0);
    // idAnnot->setBorder(move(idBorder));

    // Draw the id.
    pdfPage->addAnnot(idAnnot);
    idAnnot->draw(gfx, false);
  }
}

// _________________________________________________________________________________________________
GooString* PdfDocumentVisualizer::convertToUtf16(GooString* str) const {
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
