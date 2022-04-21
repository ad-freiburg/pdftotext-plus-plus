/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <iostream>  // std::cout
#include <sstream>  // std::stringstream
#include <tuple>  // std::make_tuple

#include <goo/GooString.h>

#include "./PdfDocument.h"

// =================================================================================================
// Cut

// _________________________________________________________________________________________________
Cut::Cut(const CutDir dirA, int pageNumA, double x1A, double y1A, double x2A, double y2A) {
  dir = dirA;
  pageNum = pageNumA;
  x1 = x1A;
  y1 = y1A;
  x2 = x2A;
  y2 = y2A;
}

// =================================================================================================
// PdfPosition

// _________________________________________________________________________________________________
double PdfPosition::getWidth() const {
  return rightX - leftX;
};

// _________________________________________________________________________________________________
double PdfPosition::getHeight() const {
  return lowerY - upperY;
};

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
};

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
};

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
};

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
};

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
};

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
};

// _________________________________________________________________________________________________
std::string PdfPosition::toString() const {
  std::stringstream ss;
  ss << "PdfPosition("
     << "page=" << pageNum << "; "
     << "leftX=" << leftX << "; "
     << "upperY=" << upperY << "; "
     << "rightX=" << rightX << "; "
     << "lowerY=" << lowerY << ")";
  return ss.str();
}

// =================================================================================================
// PdfElement

// _________________________________________________________________________________________________
PdfElement::PdfElement() {
  position = new PdfPosition();
};

// _________________________________________________________________________________________________
PdfElement::~PdfElement() {
  delete position;
};

// =================================================================================================
// PdfGlyph

// _________________________________________________________________________________________________
PdfGlyph::PdfGlyph() = default;

// _________________________________________________________________________________________________
PdfGlyph::~PdfGlyph() = default;

// _________________________________________________________________________________________________
std::string PdfGlyph::toString() const {
  std::stringstream ss;
  ss << "PdfGlyph("
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

// =================================================================================================
// PdfNonTextElement

// _________________________________________________________________________________________________
PdfNonTextElement::PdfNonTextElement() = default;

// _________________________________________________________________________________________________
PdfNonTextElement::~PdfNonTextElement() = default;

// _________________________________________________________________________________________________
std::string PdfNonTextElement::toString() const {
  std::stringstream ss;
  ss << "PdfNonTextElement(pos=" << position->toString() << ")";
  return ss.str();
}

// =================================================================================================
// PdfFigure

// _________________________________________________________________________________________________
PdfFigure::PdfFigure() = default;

// _________________________________________________________________________________________________
PdfFigure::~PdfFigure() = default;

// _________________________________________________________________________________________________
std::string PdfFigure::toString() const {
  std::stringstream ss;
  ss << "PdfFigure(pos=" << position->toString() << ")";
  return ss.str();
}

// =================================================================================================
// PdfShape

// _________________________________________________________________________________________________
PdfShape::PdfShape() = default;

// _________________________________________________________________________________________________
PdfShape::~PdfShape() = default;

// _________________________________________________________________________________________________
std::string PdfShape::toString() const {
  std::stringstream ss;
  ss << "PdfShape(pos=" << position->toString() << ")";
  return ss.str();
}

// =================================================================================================
// PdfWord

// _________________________________________________________________________________________________
PdfWord::PdfWord() = default;

// _________________________________________________________________________________________________
PdfWord::~PdfWord() {
  if (isFirstPartOfHyphenatedWord != nullptr) { delete isFirstPartOfHyphenatedWord; }
}

// _________________________________________________________________________________________________
std::string PdfWord::toString() const {
  std::stringstream ss;
  ss << "PdfWord("
     << "pos=" << position->toString() << "; "
     << "fontName=" << fontName << "; "
     << "fontSize=" << fontSize << "; "
     << "text=\"" << text << "\")";
  return ss.str();
}

// =================================================================================================
// PdfTextLine

// _________________________________________________________________________________________________
PdfTextLine::PdfTextLine() = default;

// _________________________________________________________________________________________________
PdfTextLine::~PdfTextLine() = default;

// _________________________________________________________________________________________________
std::string PdfTextLine::toString() const {
  std::stringstream ss;
  ss << "PdfTextLine("
     << "text=\"" << text << "\"; "
     << "pos=" << position->toString() << "; "
     << "indent=" << indent << "; "
     << "fontName=" << fontName << "; "
     << "fontSize=" << fontSize << ")";

  return ss.str();
}

// =================================================================================================
// PdfTextBlock

// _________________________________________________________________________________________________
PdfTextBlock::PdfTextBlock() = default;

// _________________________________________________________________________________________________
PdfTextBlock::~PdfTextBlock() = default;

// _________________________________________________________________________________________________
std::string PdfTextBlock::toString() const {
  std::stringstream ss;
  ss << "PdfTextBlock("
     << "pos=" << position->toString() << "; "
     << "role=" << role << "; "
     << "text=\"" << text << "\")";
  return ss.str();
}

// =================================================================================================
// PdfPage

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
std::string PdfPageSegment::toString() const {
  std::stringstream ss;
  ss << "PdfPageSegment("
     << "pos=" << position->toString() << "; ";
  return ss.str();
}

// =================================================================================================
// PdfPage

// _________________________________________________________________________________________________
PdfPage::PdfPage() = default;

// _________________________________________________________________________________________________
PdfPage::~PdfPage() {
  // Delete the glyphs.
  for (const auto* glyph : glyphs) {
    delete glyph;
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
  // Delete the text block detection cuts.
  for (const auto* cut : blockDetectionCuts) {
    delete cut;
  }
  // Delete the reading order cuts.
  for (const auto* cut : readingOrderCuts) {
    delete cut;
  }
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
