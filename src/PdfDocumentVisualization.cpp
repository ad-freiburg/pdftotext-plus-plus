/**
 * Copyright 2023, University of Freiburg,
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

#include "./Config.h"
#include "./PdfDocumentVisualization.h"
#include "./PdfParsing.h"
#include "./Types.h"
#include "./utils/MathUtils.h"
#include "./utils/PdfElementsUtils.h"

using std::make_unique;
using std::move;
using std::string;
using std::to_string;
using std::unique_ptr;
using std::vector;

using ppp::config::PdfDocumentVisualizationConfig;
using ppp::config::PdfParsingConfig;
using ppp::modules::PdfParsing;
using ppp::types::Cut;
using ppp::types::CutDir;
using ppp::types::PdfCharacter;
using ppp::types::PdfDocument;
using ppp::types::PdfElement;
using ppp::types::PdfFigure;
using ppp::types::PdfGraphic;
using ppp::types::PdfPageSegment;
using ppp::types::PdfShape;
using ppp::types::PdfTextBlock;
using ppp::types::PdfTextLine;
using ppp::types::PdfWord;
using ppp::types::SemanticRole;
using ppp::utils::elements::getSemanticRoleName;
using ppp::utils::math::smaller;

// =================================================================================================

namespace ppp::visualization {

// _________________________________________________________________________________________________
PdfDocumentVisualization::PdfDocumentVisualization(const string& pdfFilePath,
    const PdfDocumentVisualizationConfig& config) {
  // Load the PDF document.
  GooString gooPdfFilePath(pdfFilePath);
  _pdfDoc = PDFDocFactory().createPDFDoc(gooPdfFilePath);

  _doc = new PdfDocument();

  PdfParsingConfig ppConfig;
  ppConfig.skipEmbeddedFontFilesParsing = true;
  _out = new PdfParsing(_doc, ppConfig);

  // Create a Gfx for each PDF page.
  _gfxs.push_back(nullptr);  // Make the vector 1-based.
  for (int i = 1; i <= _pdfDoc->getNumPages(); i++) {
    Page* page = _pdfDoc->getPage(i);
    Gfx* gfx = page->createGfx(_out, config.hDPI, config.vDPI, 0,
        true, false, -1, -1, -1, -1, true, nullptr, nullptr, nullptr);
    _gfxs.push_back(gfx);
  }
  _config = config;
}

// _________________________________________________________________________________________________
PdfDocumentVisualization::~PdfDocumentVisualization() {
  // TODO(korzen):
  // for (size_t i = 0; i < _gfxs.size(); i++) {
  //   delete _gfxs[i];
  // }
  delete _doc;
  delete _out;
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeCharacters(const PdfDocument& doc, const ColorScheme& cs)
    const {
  for (const auto* page : doc.pages) {
    drawCharBoundingBoxes(page->characters, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeCharacters(const vector<PdfCharacter*>& chars,
    const ColorScheme& cs) const {
  drawCharBoundingBoxes(chars, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeFigures(const PdfDocument& doc,
    const ColorScheme& cs) const {
  for (const auto* page : doc.pages) {
    drawFigureBoundingBoxes(page->figures, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeFigures(const vector<PdfFigure*>& figures,
    const ColorScheme& cs) const {
  drawFigureBoundingBoxes(figures, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeShapes(const PdfDocument& doc,
    const ColorScheme& cs) const {
  for (const auto* page : doc.pages) {
    drawShapeBoundingBoxes(page->shapes, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeShapes(const vector<PdfShape*>& shapes,
    const ColorScheme& cs) const {
  drawShapeBoundingBoxes(shapes, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeGraphics(const PdfDocument& doc,
    const ColorScheme& cs) const {
  for (const auto* page : doc.pages) {
    drawGraphicBoundingBoxes(page->graphics, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeGraphics(const vector<PdfGraphic*>& graphics,
    const ColorScheme& cs) const {
  drawGraphicBoundingBoxes(graphics, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeWords(const PdfDocument& doc, const ColorScheme& cs) const {
  for (const auto* page : doc.pages) {
    drawWordBoundingBoxes(page->words, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeWords(const vector<PdfWord*>& words,
    const ColorScheme& cs) const {
  drawWordBoundingBoxes(words, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeTextLines(const PdfDocument& doc, const ColorScheme& cs)
    const {
  for (const auto* page : doc.pages) {
    for (const auto* segment : page->segments) {
      drawTextLineBoundingBoxes(segment->lines, cs);
    }
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeTextLines(const vector<PdfTextLine*>& lines,
    const ColorScheme& cs) const {
  drawTextLineBoundingBoxes(lines, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeTextBlocks(const PdfDocument& doc, const ColorScheme& cs)
      const {
  for (const auto* page : doc.pages) {
    drawTextBlockBoundingBoxes(page->blocks, cs);
    drawTextBlockSemanticRoles(page->blocks, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeTextBlocks(const vector<PdfTextBlock*>& blocks,
    const ColorScheme& cs) const {
  drawTextBlockBoundingBoxes(blocks, cs);
  drawTextBlockSemanticRoles(blocks, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizePageSegments(const PdfDocument& doc,
    const ColorScheme& cs) const {
  for (const auto* page : doc.pages) {
    drawPageSegmentBoundingBoxes(page->segments, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizePageSegments(const vector<PdfPageSegment*>& segments,
    const ColorScheme& cs) const {
  drawPageSegmentBoundingBoxes(segments, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeReadingOrder(const PdfDocument& doc,
    const ColorScheme& cs) const {
  for (const auto* page : doc.pages) {
    drawTextBlockBoundingBoxes(page->blocks, cs);
    drawTextBlockSemanticRoles(page->blocks, cs);
    drawReadingOrder(page->blocks, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeReadingOrder(const vector<PdfTextBlock*>& blocks,
    const ColorScheme& cs) const {
  drawTextBlockBoundingBoxes(blocks, cs);
  drawTextBlockSemanticRoles(blocks, cs);
  drawReadingOrder(blocks, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeSegmentCuts(const PdfDocument& doc,
    const ColorScheme& cs) const {
  for (const auto* page : doc.pages) {
    drawCuts(page->blockDetectionCuts, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeTextBlockDetectionCuts(const vector<Cut*>& cuts,
    const ColorScheme& cs) const {
  drawCuts(cuts, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeReadingOrderCuts(const PdfDocument& doc,
    const ColorScheme& cs) const {
  for (const auto* page : doc.pages) {
    drawCuts(page->readingOrderCuts, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::visualizeReadingOrderCuts(const vector<Cut*>& cuts,
    const ColorScheme& cs) const {
  drawCuts(cuts, cs);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::save(const string& targetPath) const {
  GooString gooTargetPath(targetPath);
  _pdfDoc->saveAs(gooTargetPath);
}

// =================================================================================================

// _________________________________________________________________________________________________
void PdfDocumentVisualization::drawCharBoundingBoxes(const vector<PdfCharacter*>& characters,
    const ColorScheme& cs) const {
  for (const auto* ch : characters) {
    drawBoundingBox(ch, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::drawFigureBoundingBoxes(const vector<PdfFigure*>& figures,
    const ColorScheme& cs) const {
  for (const auto* figure : figures) {
    drawBoundingBox(figure, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::drawShapeBoundingBoxes(const vector<PdfShape*>& shapes,
    const ColorScheme& cs) const {
  for (const auto* shape : shapes) {
    drawBoundingBox(shape, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::drawGraphicBoundingBoxes(const vector<PdfGraphic*>& graphics,
    const ColorScheme& cs) const {
  for (const auto* graphic : graphics) {
    drawBoundingBox(graphic, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::drawWordBoundingBoxes(const vector<PdfWord*>& words,
    const ColorScheme& cs) const {
  for (const auto* word : words) {
    drawBoundingBox(word, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::drawTextLineBoundingBoxes(const vector<PdfTextLine*>& lines,
    const ColorScheme& cs) const {
  for (const auto* line : lines) {
    drawBoundingBox(line, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::drawTextBlockBoundingBoxes(const vector<PdfTextBlock*>& blocks,
    const ColorScheme& cs) const {
  for (const auto* block : blocks) {
    drawBoundingBox(block, cs);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::drawPageSegmentBoundingBoxes(
    const vector<PdfPageSegment*>& segments, const ColorScheme& cs) const {
  for (const auto* segment : segments) {
    drawBoundingBox(segment, cs);

    // Draw the trim box of the segment.
    // const ColorScheme& cs1 = color_schemes::green;
    // Page* pdfPage = _pdfDoc->getPage(segment->pos->pageNum);
    // Gfx* gfx = _gfxs[segment->pos->pageNum];
    // double leftX = segment->trimLeftX;
    // double upperY = pdfPage->getMediaHeight() - segment->trimLowerY;
    // double rightX = segment->trimRightX;
    // double lowerY = pdfPage->getMediaHeight() - segment->trimUpperY;
    // // Vertical/horizontal lines can have a width/height of zero, in which case they are not
    // // visible in the visualization. So ensure a minimal width/height of 1.
    // if (ppp::math_utils::smaller(fabs(leftX - rightX), 1)) { rightX += 1; }
    // if (ppp::math_utils::smaller(fabs(upperY - lowerY), 1)) { lowerY += 1; }
    // PDFRectangle rect(leftX, upperY, rightX, lowerY);

    // // Create the bounding box.
// AnnotGeometry* annot = new AnnotGeometry(_pdfDoc.get(), &rect, Annot::AnnotSubtype::typeSquare);

    // // Define the color of the bounding box.
    // std::unique_ptr<AnnotColor> color = make_unique<AnnotColor>(cs1.primaryColor);
    // annot->setColor(move(color));

    // // Draw the bounding box.
    // pdfPage->addAnnot(annot);
    // annot->draw(gfx, false);

    // Draw the (preliminary) text blocks stored in the segment
    for (const auto* block : segment->blocks) {
      const ColorScheme& cs2 = ppp::visualization::color_schemes::red;
      Page* pdfPage = _pdfDoc->getPage(segment->pos->pageNum);
      Gfx* gfx = _gfxs[segment->pos->pageNum];
      double leftX = block->pos->leftX;
      double upperY = pdfPage->getMediaHeight() - block->pos->lowerY;
      double rightX = block->pos->rightX;
      double lowerY = pdfPage->getMediaHeight() - block->pos->upperY;
      // Vertical/horizontal lines can have a width/height of zero, in which case they are not
      // visible in the visualization. So ensure a minimal width/height of 1.
      if (smaller(fabs(leftX - rightX), 1)) { rightX += 1; }
      if (smaller(fabs(upperY - lowerY), 1)) { lowerY += 1; }
      PDFRectangle rect(leftX, upperY, rightX, lowerY);

      // Create the bounding box.
      AnnotGeometry* a = new AnnotGeometry(_pdfDoc.get(), &rect, Annot::AnnotSubtype::typeSquare);

      // Define the color of the bounding box.
      unique_ptr<AnnotColor> color = make_unique<AnnotColor>(cs2.primaryColor);
      a->setColor(move(color));

      // Draw the bounding box.
      pdfPage->addAnnot(a);
      a->draw(gfx, false);
    }
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::drawBoundingBox(const PdfElement* element, const ColorScheme& cs)
      const {
  Page* pdfPage = _pdfDoc->getPage(element->pos->pageNum);
  Gfx* gfx = _gfxs[element->pos->pageNum];

  // Define the coordinates of the bounding box to draw. Make the y-coordinates relatively to the
  // lower left of the page.
  double leftX = element->pos->leftX;
  double upperY = pdfPage->getMediaHeight() - element->pos->lowerY;
  double rightX = element->pos->rightX;
  double lowerY = pdfPage->getMediaHeight() - element->pos->upperY;

  // Vertical/horizontal lines can have a width/height of zero, in which case they are not
  // visible in the visualization. So ensure a minimal width/height of 1.
  if (smaller(fabs(leftX - rightX), 1)) { rightX += 1; }
  if (smaller(fabs(upperY - lowerY), 1)) { lowerY += 1; }
  PDFRectangle rect(leftX, upperY, rightX, lowerY);

  // Create the bounding box.
  AnnotGeometry* annot = new AnnotGeometry(_pdfDoc.get(), &rect, Annot::AnnotSubtype::typeSquare);

  // Define the color of the bounding box.
  unique_ptr<AnnotColor> color = make_unique<AnnotColor>(cs.primaryColor);
  annot->setColor(move(color));

  // Draw the bounding box.
  pdfPage->addAnnot(annot);
  annot->draw(gfx, false);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::drawTextBlockSemanticRoles(const vector<PdfTextBlock*>& blocks,
    const ColorScheme& cs) const {
  // Iterate through the text blocks and draw the semantic role of each.
  for (const auto* block : blocks) {
    Page* pdfPage = _pdfDoc->getPage(block->pos->pageNum);
    Gfx* gfx = _gfxs[block->pos->pageNum];

    // Define the position of the semantic role. Make the lowerY relatively to the lower left of
    // the page.
    double leftX = block->pos->leftX;
    double lowerY = pdfPage->getMediaHeight() - block->pos->upperY;
    PDFRectangle rect(leftX, lowerY, leftX + 100, lowerY + 7);

    // Define the font appearance of the semantic role.
    const GooString appearanceStr(_config.semanticRoleAppearance);
    const DefaultAppearance appearance(&appearanceStr);

    // Create the annotation.
    AnnotFreeText* annot = new AnnotFreeText(_pdfDoc.get(), &rect);
    annot->setDefaultAppearance(appearance);

    // Define the text of the annotation (= the semantic role).
    annot->setContents(make_unique<GooString>(convertToUtf16(getSemanticRoleName(block->role))));

    // Remove the default border around the annotation.
    unique_ptr<AnnotBorder> border(new AnnotBorderArray());
    border->setWidth(0);
    annot->setBorder(move(border));

    // Draw the annotation.
    pdfPage->addAnnot(annot);
    annot->draw(gfx, false);
  }
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::drawReadingOrder(const vector<PdfTextBlock*>& blocks,
    const ColorScheme& cs) const {
  // Iterate through the text blocks and draw a line between the current and previous block.
  for (size_t i = 1; i < blocks.size(); i++) {
    PdfTextBlock* prevBlock = blocks[i - 1];
    PdfTextBlock* currBlock = blocks[i];

    Page* pdfPage = _pdfDoc->getPage(currBlock->pos->pageNum);
    Gfx* gfx = _gfxs[currBlock->pos->pageNum];

    // Compute the coordinates of the midpoints of the previous and current text block.
    double prevMinX = prevBlock->pos->leftX;
    double prevMaxX = prevBlock->pos->rightX;
    double prevMinY = pdfPage->getMediaHeight() - prevBlock->pos->lowerY;
    double prevMaxY = pdfPage->getMediaHeight() - prevBlock->pos->upperY;
    double prevMidX = prevBlock->pos->leftX + ((prevMaxX - prevMinX) / 2.0);
    double prevMidY = prevMinY + ((prevMaxY - prevMinY) / 2.0);
    double currMinX = currBlock->pos->leftX;
    double currMaxX = currBlock->pos->rightX;
    double currMinY = pdfPage->getMediaHeight() - currBlock->pos->lowerY;
    double currMaxY = pdfPage->getMediaHeight() - currBlock->pos->upperY;
    double currMidX = currBlock->pos->leftX + ((currMaxX - currMinX) / 2.0);
    double currMidY = currMinY + ((currMaxY - currMinY) / 2.0);

    // Define the position of the reading order line.
    PDFRectangle lineRect(prevMidX, prevMidY, currMidX, currMidY);
    AnnotLine* lineAnnot = new AnnotLine(_pdfDoc.get(), &lineRect);
    lineAnnot->setVertices(prevMidX, prevMidY, currMidX, currMidY);

    // Define the width of the reading order line.
    unique_ptr<AnnotBorder> lineBorder(new AnnotBorderArray());
    lineBorder->setWidth(_config.readingOrderLineWidth);
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
void PdfDocumentVisualization::drawReadingOrderIndexCircle(Page* page, Gfx* gfx, double x,
    double y, int readingOrderIndex, const ColorScheme& cs) const {
  // Define the position of the circle.
  double radius = _config.readingOrderCircleRadius;
  PDFRectangle circleRect(x - radius, y - radius, x + radius, y + radius);
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
  const GooString indexAppearanceStr(_config.readingOrderIndexAppearance);
  const DefaultAppearance indexAppearance(&indexAppearanceStr);

  // Define the position of the index.
  PDFRectangle indexRect(x - radius, y - radius, x + radius, y + radius * 0.6);
  AnnotFreeText* indexAnnot = new AnnotFreeText(_pdfDoc.get(), &indexRect);
  indexAnnot->setDefaultAppearance(indexAppearance);

  // Define the text of the annot (= the reading order index).
  indexAnnot->setContents(make_unique<GooString>(convertToUtf16(to_string(readingOrderIndex))));
  // Center the text horizontally.
  indexAnnot->setQuadding(VariableTextQuadding::centered);

  // Remove the default border around the reading order index.
  unique_ptr<AnnotBorder> indexBorder(new AnnotBorderArray());
  indexBorder->setWidth(0);
  indexAnnot->setBorder(move(indexBorder));

  // Draw the reading order index.
  page->addAnnot(indexAnnot);
  indexAnnot->draw(gfx, false);
}

// _________________________________________________________________________________________________
void PdfDocumentVisualization::drawCuts(const vector<Cut*>& cuts, const ColorScheme& cs) const {
  size_t chosenCutIndex = 0;
  // Iterate through the cuts and visualize each.
  for (size_t i = 0; i < cuts.size(); i++) {
    const auto* cut = cuts[i];

    const ColorScheme& cos = cut->isChosen ? cs : color_schemes::gray;

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
    unique_ptr<AnnotBorder> lineBorder(new AnnotBorderArray());
    lineBorder->setWidth(_config.cutWidth);
    lineAnnot->setBorder(move(lineBorder));

    // Define the line color.
    auto lineColor = make_unique<AnnotColor>(cos.primaryColor);
    lineAnnot->setColor(move(lineColor));

    // Draw the line.
    pdfPage->addAnnot(lineAnnot);
    lineAnnot->draw(gfx, false);

    if (cut->isChosen) {
      // ==========
      // Draw a square at the beginning of the line, containing the cut index.

      // Define the position of the square.
      const double radius = _config.cutSquareRadius;
      PDFRectangle squareRect(x1 - radius, y1 - radius, x1 + radius, y1 + radius);
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
      GooString indexAppearanceStr(_config.cutIndexAppearance);
      const DefaultAppearance indexAppearance(&indexAppearanceStr);

      // Define the position of the cut index.
      PDFRectangle indexRect(x1 - radius, y1 - radius, x1 + radius, y1 + radius * 0.6);
      AnnotFreeText* indexAnnot = new AnnotFreeText(_pdfDoc.get(), &indexRect);
      indexAnnot->setDefaultAppearance(indexAppearance);

      // Define the text of the annot (= the how many-th chosen cut the index is).
      indexAnnot->setContents(make_unique<GooString>(convertToUtf16(to_string(++chosenCutIndex))));
      // Center the text horizontally.
      indexAnnot->setQuadding(VariableTextQuadding::centered);

      // Remove the default border around the cut index.
      unique_ptr<AnnotBorder> indexBorder(new AnnotBorderArray());
      indexBorder->setWidth(0);
      indexAnnot->setBorder(move(indexBorder));

      // Draw the cut index.
      pdfPage->addAnnot(indexAnnot);
      indexAnnot->draw(gfx, false);
    }

    // ==========
    // Draw the id of the cut.

    // Define the appearance of the id.
    const GooString idAppearanceStr(_config.cutIdAppearance);
    const DefaultAppearance idAppearance(&idAppearanceStr);

    // Define the position of the id.
    double rectWidth = 20;
    double rectHeight = 10;
    double rectMinX = cut->dir == CutDir::X ? x2 - (rectWidth / 2.0) : x2 - rectWidth;
    double rectMinY = cut->dir == CutDir::X ? y2 - rectHeight : y2;
    double rectMaxX = rectMinX + rectWidth;
    double rectMaxY = rectMinY + rectHeight;
    PDFRectangle idRect(rectMinX, rectMinY, rectMaxX, rectMaxY);
    AnnotFreeText* idAnnot = new AnnotFreeText(_pdfDoc.get(), &idRect);
    idAnnot->setDefaultAppearance(idAppearance);

    // Define the text of the annot (= the id).
    idAnnot->setContents(make_unique<GooString>(convertToUtf16(cut->id)));
    // Center the text horizontally.
    idAnnot->setQuadding(VariableTextQuadding::centered);

    // Remove the default border around the cut id.
    unique_ptr<AnnotBorder> idBorder(new AnnotBorderArray());
    idBorder->setWidth(0);
    idAnnot->setBorder(move(idBorder));

    // Draw the id.
    pdfPage->addAnnot(idAnnot);
    idAnnot->draw(gfx, false);
  }
}

// _________________________________________________________________________________________________
string PdfDocumentVisualization::convertToUtf16(const string& str) const {
  int length = 2 + 2 * str.length();
  char* result = new char[length];
  // Add unicode markers.
  result[0] = static_cast<char>(0xfe);
  result[1] = static_cast<char>(0xff);
  // Convert to UTF-16.
  for (int i = 2, j = 0; i < length; i += 2, j++) {
    unsigned char character = static_cast<unsigned char>(str.at(j));
    Unicode u = pdfDocEncoding[static_cast<unsigned int>(character)] & 0xffff;
    result[i] = (u >> 8) & 0xff;
    result[i + 1] = u & 0xff;
  }
  // delete str;
  string s(result, length);
  // delete[] result;
  return s;
}

}  // namespace ppp::visualization
