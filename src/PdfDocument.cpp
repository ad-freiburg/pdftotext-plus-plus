/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <sstream>  // std::stringstream
#include <string>

#include "./utils/MathUtils.h"

#include "./Constants.h"
#include "./PdfDocument.h"

using global_config::COORDS_PREC;
using std::string;
using std::stringstream;

// =================================================================================================
// Cut

// _________________________________________________________________________________________________
Cut::Cut(const CutDir dirA) {
  dir = dirA;
}

// _________________________________________________________________________________________________
Cut::Cut(const CutDir dirA, const string& idA, int posInElementsA) {
  dir = dirA;
  id = idA;
  posInElements = posInElementsA;
}

// _________________________________________________________________________________________________
Cut::~Cut() = default;

// =================================================================================================
// PdfPosition

// _________________________________________________________________________________________________
PdfPosition::PdfPosition() = default;

// _________________________________________________________________________________________________
PdfPosition::~PdfPosition() = default;

// _________________________________________________________________________________________________
double PdfPosition::getWidth() const {
  return rightX - leftX;
}

// _________________________________________________________________________________________________
double PdfPosition::getHeight() const {
  return lowerY - upperY;
}

// _________________________________________________________________________________________________
double PdfPosition::getRotLeftX() const {
  switch (rotation) {
    case 0:
    default:
      return leftX;
    case 1:
      return upperY;
    case 2:
      return rightX;
    case 3:
      return lowerY;
  }
}

// _________________________________________________________________________________________________
double PdfPosition::getRotUpperY() const {
  switch (rotation) {
    case 0:
    default:
      return upperY;
    case 1:
      return rightX;
    case 2:
      return lowerY;
    case 3:
      return leftX;
  }
}

// _________________________________________________________________________________________________
double PdfPosition::getRotRightX() const {
  switch (rotation) {
    case 0:
    default:
      return rightX;
    case 1:
      return lowerY;
    case 2:
      return leftX;
    case 3:
      return upperY;
  }
}

// _________________________________________________________________________________________________
double PdfPosition::getRotLowerY() const {
  switch (rotation) {
    case 0:
    default:
      return lowerY;
    case 1:
      return leftX;
    case 2:
      return upperY;
    case 3:
      return rightX;
  }
}

// _________________________________________________________________________________________________
double PdfPosition::getRotWidth() const {
  switch (rotation) {
    case 0:
    case 1:
    default:
      return getRotRightX() - getRotLeftX();
    case 2:
    case 3:
      return getRotLeftX() - getRotRightX();
  }
}

// _________________________________________________________________________________________________
double PdfPosition::getRotHeight() const {
  switch (rotation) {
    case 0:
    case 3:
    default:
      return getRotLowerY() - getRotUpperY();
    case 1:
    case 2:
      return getRotUpperY() - getRotLowerY();
  }
}

// _________________________________________________________________________________________________
string PdfPosition::toString() const {
  stringstream ss;
  ss << "PdfPosition("
     << "page=" << pageNum << "; "
     << "leftX=" << math_utils::round(leftX, COORDS_PREC) << "; "
     << "upperY=" << math_utils::round(upperY, COORDS_PREC) << "; "
     << "rightX=" << math_utils::round(rightX, COORDS_PREC) << "; "
     << "lowerY=" << math_utils::round(lowerY, COORDS_PREC) << "; "
     << "rotation=" << rotation << "; "
     << "wMode=" << wMode << ")";
  return ss.str();
}

// _________________________________________________________________________________________________
string PdfPosition::toShortString() const {
  stringstream ss;
  ss << "p=" << pageNum
     << "; leftX=" << math_utils::round(leftX, COORDS_PREC)
     << "; upperY=" << math_utils::round(upperY, COORDS_PREC)
     << "; rightX=" << math_utils::round(rightX, COORDS_PREC)
     << "; lowerY=" << math_utils::round(lowerY, COORDS_PREC);
  return ss.str();
}

// =================================================================================================
// PdfElement

// _________________________________________________________________________________________________
PdfElement::PdfElement() {
  position = new PdfPosition();
}

// _________________________________________________________________________________________________
PdfElement::~PdfElement() {
  delete position;
}

// =================================================================================================
// PdfTextElement

// _________________________________________________________________________________________________
PdfTextElement::PdfTextElement() = default;

// _________________________________________________________________________________________________
PdfTextElement::~PdfTextElement() = default;

// =================================================================================================
// PdfNonTextElement

// _________________________________________________________________________________________________
PdfNonTextElement::PdfNonTextElement() = default;

// _________________________________________________________________________________________________
PdfNonTextElement::~PdfNonTextElement() = default;

// =================================================================================================
// PdfCharacter

// _________________________________________________________________________________________________
PdfCharacter::PdfCharacter() = default;

// _________________________________________________________________________________________________
PdfCharacter::~PdfCharacter() = default;

// _________________________________________________________________________________________________
string PdfCharacter::toString() const {
  stringstream ss;
  ss << "PdfCharacter("
     << "pos=" << position->toString() << "; "
     << "fontName=" << fontName << "; "
     << "fontSize=" << fontSize << "; "
     << "color=(" << color[0] << ", " << color[1] << ", " << color[2] << "); "
     << "opacity=" << opacity << "; "
     << "unicodes=[";
  for (size_t i = 0; i < unicodes.size(); i++) {
    ss << unicodes[i];
    if (i < unicodes.size() - 1) {
      ss << ", ";
    }
  }
  ss << "]; text=\"" << text << "\")";
  return ss.str();
}

// _________________________________________________________________________________________________
string PdfCharacter::toShortString() const {
  stringstream ss;
  ss << "type: char; "
     << position->toShortString()
     << "; fn=" << fontName
     << "; fs=" << fontSize
     << "; text=\"" << text << "\"";
  return ss.str();
}

// =================================================================================================
// PdfWord

// _________________________________________________________________________________________________
PdfWord::PdfWord() = default;

// _________________________________________________________________________________________________
PdfWord::~PdfWord() {
  // if (isFirstPartOfHyphenatedWord != nullptr) { delete isFirstPartOfHyphenatedWord; }
  // if (isSecondPartOfHyphenatedWord != nullptr) { delete isSecondPartOfHyphenatedWord; }
  // if (isPartOfStackedMathSymbol != nullptr) { delete isPartOfStackedMathSymbol; }
}

// _________________________________________________________________________________________________
string PdfWord::toString() const {
  stringstream ss;
  ss << "PdfWord("
     << "pos=" << position->toString() << "; "
     << "fontName=" << fontName << "; "
     << "fontSize=" << fontSize << "; "
     << "text=\"" << text << "\")";
  return ss.str();
}

// _________________________________________________________________________________________________
string PdfWord::toShortString() const {
  stringstream ss;
  ss << "type: word; "
     << position->toShortString()
     << "; fn=" << fontName
     << "; fs=" << fontSize
     << "; text=\"" << text << "\"";
  return ss.str();
}

// =================================================================================================
// PdfTextLine

// _________________________________________________________________________________________________
PdfTextLine::PdfTextLine() = default;

// _________________________________________________________________________________________________
PdfTextLine::~PdfTextLine() = default;

// _________________________________________________________________________________________________
string PdfTextLine::toString() const {
  stringstream ss;
  ss << "PdfTextLine("
     << "text=\"" << text << "\"; "
     << "pos=" << position->toString() << "; "
     << "leftMargin=" << leftMargin << "; "
     << "rightMargin=" << rightMargin << "; "
     << "fontName=" << fontName << "; "
     << "fontSize=" << fontSize << ")";

  return ss.str();
}

// _________________________________________________________________________________________________
string PdfTextLine::toShortString() const {
  stringstream ss;
  ss << "type: line; "
     << position->toShortString()
     << "; fn=" << fontName
     << "; fs=" << fontSize
     << "; text=\"" << text << "\"";
  return ss.str();
}

// =================================================================================================
// PdfTextBlock

// _________________________________________________________________________________________________
PdfTextBlock::PdfTextBlock() = default;

// _________________________________________________________________________________________________
PdfTextBlock::~PdfTextBlock() = default;

// _________________________________________________________________________________________________
string PdfTextBlock::toString() const {
  stringstream ss;
  ss << "PdfTextBlock("
     << "pos=" << position->toString() << "; "
     << "role=" << role << "; "
     << "isCentered=" << isLinesCentered << "; "
     << "isEmphasized=" << isEmphasized << "; "
     << "text=\"" << text << "\")";
  return ss.str();
}

// _________________________________________________________________________________________________
string PdfTextBlock::toShortString() const {
  stringstream ss;
  ss << "type: block; "
     << position->toShortString()
     << "; fn=" << fontName
     << "; fs=" << fontSize
     << "; text=\"" << text << "\"";
  return ss.str();
}

// =================================================================================================
// PdfFigure

// _________________________________________________________________________________________________
PdfFigure::PdfFigure() = default;

// _________________________________________________________________________________________________
PdfFigure::~PdfFigure() {
  // Delete the characters.
  for (const auto* character : characters) {
    delete character;
  }

  // Delete the shapes.
  for (const auto* shape : shapes) {
    delete shape;
  }

  // Delete the graphics.
  for (const auto* graphic : graphics) {
    delete graphic;
  }
}

// _________________________________________________________________________________________________
string PdfFigure::toString() const {
  stringstream ss;
  ss << "PdfFigure(pos=" << position->toString() << ")";
  return ss.str();
}

// _________________________________________________________________________________________________
string PdfFigure::toShortString() const {
  stringstream ss;
  ss << "type: figure; " << position->toShortString();
  return ss.str();
}

// =================================================================================================
// PdfShape

// _________________________________________________________________________________________________
PdfShape::PdfShape() = default;

// _________________________________________________________________________________________________
PdfShape::~PdfShape() = default;

// _________________________________________________________________________________________________
string PdfShape::toString() const {
  stringstream ss;
  ss << "PdfShape(pos=" << position->toString() << ")";
  return ss.str();
}

// _________________________________________________________________________________________________
string PdfShape::toShortString() const {
  stringstream ss;
  ss << "type: shape; " << position->toShortString();
  return ss.str();
}

// =================================================================================================
// PdfGraphic

// _________________________________________________________________________________________________
PdfGraphic::PdfGraphic() = default;

// _________________________________________________________________________________________________
PdfGraphic::~PdfGraphic() = default;

// _________________________________________________________________________________________________
string PdfGraphic::toString() const {
  stringstream ss;
  ss << "PdfGraphic(pos=" << position->toString() << ")";
  return ss.str();
}

// _________________________________________________________________________________________________
string PdfGraphic::toShortString() const {
  stringstream ss;
  ss << "type: graphic; " << position->toShortString();
  return ss.str();
}

// =================================================================================================
// PdfPageSegment

// _________________________________________________________________________________________________
PdfPageSegment::PdfPageSegment() = default;

// _________________________________________________________________________________________________
PdfPageSegment::~PdfPageSegment() {
  // Delete the lines.
  for (const auto* line : lines) {
    delete line;
  }
}

// _________________________________________________________________________________________________
string PdfPageSegment::toString() const {
  stringstream ss;
  ss << "PdfPageSegment("
     << "pos=" << position->toString() << "; ";
  return ss.str();
}

// _________________________________________________________________________________________________
string PdfPageSegment::toShortString() const {
  stringstream ss;
  ss << "type: segment; " << position->toShortString();
  return ss.str();
}

// =================================================================================================
// PdfPage

// _________________________________________________________________________________________________
PdfPage::PdfPage() = default;

// _________________________________________________________________________________________________
PdfPage::~PdfPage() {
  // Delete the characters.
  for (const auto* character : characters) {
    delete character;
  }
  // Delete the words.
  for (const auto* word : words) {
    delete word;
  }
  // Delete the blocks.
  for (const auto* block : blocks) {
    delete block;
  }
  // Delete the page segments.
  for (const auto* segment : segments) {
    delete segment;
  }
  // Delete the figures.
  for (const auto* figure : figures) {
    delete figure;
  }
  // Delete the shapes.
  for (const auto* shape : shapes) {
    delete shape;
  }
  // Delete the graphics.
  for (const auto* graphic : graphics) {
    delete graphic;
  }
  // Delete the text block detection cuts.
  for (const auto* cut : blockDetectionCuts) {
    delete cut;
  }
  // Delete the reading order cuts.
  for (const auto* cut : readingOrderCuts) {
    delete cut;
  }
}

// _________________________________________________________________________________________________
double PdfPage::getWidth() const {
  return clipRightX - clipLeftX;
}

// _________________________________________________________________________________________________
double PdfPage::getHeight() const {
  return clipLowerY - clipUpperY;
}

// =================================================================================================
// PdfDocument

// _________________________________________________________________________________________________
PdfDocument::PdfDocument() = default;

// _________________________________________________________________________________________________
PdfDocument::~PdfDocument() {
  // Delete the pages.
  for (const auto& page : pages) {
    delete page;
  }
  // Delete the font infos.
  for (const auto& fi : fontInfos) {
    delete fi.second;
  }
}
