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
// PdfElement

// _________________________________________________________________________________________________
PdfElement::PdfElement() = default;

// _________________________________________________________________________________________________
PdfElement::~PdfElement() = default;

// _________________________________________________________________________________________________
double PdfElement::getWidth() const {
  return maxX - minX;
}

// _________________________________________________________________________________________________
double PdfElement::getHeight() const {
  return maxY - minY;
}


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
     << "page=" << pageNum << "; "
     << "minX=" << minX << "; "
     << "minY=" << minY << "; "
     << "maxX=" << maxX << "; "
     << "maxY=" << maxY << "; "
     << "fontName=" << fontName << "; "
     << "fontSize=" << fontSize << "; "
     << "rotation=" << rotation << "; "
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
// PdfNonText

// _________________________________________________________________________________________________
PdfNonText::PdfNonText() = default;

// _________________________________________________________________________________________________
PdfNonText::~PdfNonText() = default;

// _________________________________________________________________________________________________
std::string PdfNonText::toString() const {
  std::stringstream ss;
  ss << "PdfNonText("
     << "page=" << pageNum << "; "
     << "minX=" << minX << "; "
     << "minY=" << minY << "; "
     << "maxX=" << maxX << "; "
     << "maxY=" << maxY << ")";
  return ss.str();
}

// =================================================================================================
// PdfWord

// _________________________________________________________________________________________________
PdfWord::PdfWord() = default;

// _________________________________________________________________________________________________
PdfWord::~PdfWord() {
  if (leftSubscript != nullptr) { delete leftSubscript; }
  if (leftSuperscript != nullptr) { delete leftSuperscript; }
  if (leftPunctuation != nullptr) { delete leftPunctuation; }
  if (rightSubscript != nullptr) { delete rightSubscript; }
  if (rightSuperscript != nullptr) { delete rightSuperscript; }
  if (rightPunctuation != nullptr) { delete rightPunctuation; }
  if (isFirstPartOfHyphenatedWord != nullptr) { delete isFirstPartOfHyphenatedWord; }
}

// _________________________________________________________________________________________________
std::string PdfWord::toString() const {
  std::stringstream ss;
  ss << "PdfWord("
     << "page=" << pageNum << "; "
     << "minX=" << minX << "; "
     << "minY=" << minY << "; "
     << "maxX=" << maxX << "; "
     << "maxY=" << maxY << "; "
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
     << "page=" << pageNum << "; "
     << "minX=" << minX << "; "
     << "minY=" << minY << "; "
     << "maxX=" << maxX << "; "
     << "maxY=" << maxY << "; "
     << "fontName=" << fontName << "; "
     << "fontSize=" << fontSize << "; "
     << "text=\"" << text << "\")";
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
     << "page=" << pageNum << "; "
     << "minX=" << minX << "; "
     << "minY=" << minY << "; "
     << "maxX=" << maxX << "; "
     << "maxY=" << maxY << "; "
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
     << "page=" << pageNum << "; "
     << "minX=" << minX << "; "
     << "minY=" << minY << "; "
     << "maxX=" << maxX << "; "
     << "maxY=" << maxY << ")";
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
  // Delete the non-text elements.
  for (const auto* nonText : nonTexts) {
    delete nonText;
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
