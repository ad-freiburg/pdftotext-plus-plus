/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cmath>  // fabs
#include <iostream>
#include <sstream>

#include <poppler/GfxFont.h>
#include <poppler/GfxState.h>
#include <poppler/GlobalParams.h>
#include <goo/GooString.h>

#include "./PdfDocument.h"
#include "./TextOutputDev.h"

#include "./utils/GlyphMap.h"
#include "./utils/Log.h"
#include "./utils/MathUtils.h"
#include "./utils/StringUtils.h"


// _________________________________________________________________________________________________
TextOutputDev::TextOutputDev(bool noEmbeddedFontFilesParsing, PdfDocument* doc,
      bool debug, int debugPageFilter) {
  _noEmbeddedFontFilesParsing = noEmbeddedFontFilesParsing;
  _doc = doc;
  _log = new Logger(debug ? DEBUG : INFO, debugPageFilter);

  _log->debug() << "=======================================" << std::endl;
  _log->debug() << "\033[1mDEBUG MODE | PDF Parsing\033[0m" << std::endl;
  _log->debug() << " └─ parse font files:  " << noEmbeddedFontFilesParsing << std::endl;
  _log->debug() << " └─ debug page filter: " << debugPageFilter << std::endl;
}

// _________________________________________________________________________________________________
TextOutputDev::~TextOutputDev() {
  delete _log;
}

// _________________________________________________________________________________________________
void TextOutputDev::startPage(int pageNum, GfxState* state, XRef* xref) {
  _log->debug(pageNum) << "=======================================" << std::endl;
  _log->debug(pageNum) << "\033[1mEvent: START PAGE\033[0m " << std::endl;

  _page = new PdfPage();
  _page->pageNum = _p = pageNum;
  state->getClipBBox(&_page->clipLeftX, &_page->clipUpperY, &_page->clipRightX, &_page->clipLowerY);
  _doc->pages.push_back(_page);
  _xref = xref;

  _log->debug(pageNum) << " └─ page.pageNum: " << _page->pageNum << std::endl;
  _log->debug(pageNum) << " └─ page.clipLeftX: " << _page->clipLeftX << std::endl;
  _log->debug(pageNum) << " └─ page.clipUpperY: " << _page->clipUpperY << std::endl;
  _log->debug(pageNum) << " └─ page.clipRightX: " << _page->clipRightX << std::endl;
  _log->debug(pageNum) << " └─ page.clipLowerY: " << _page->clipLowerY << std::endl;
}

// _________________________________________________________________________________________________
void TextOutputDev::updateFont(GfxState* state) {
  _log->debug(_p) << "=======================================" << std::endl;
  _log->debug(_p) << "\033[1mEvent: UPDATE FONT\033[0m " << std::endl;

  // Update the current font info.
  _fontInfo = nullptr;

  // Skip the event if the state does not contain any font.
  GfxFont* gfxFont = state->getFont();
  if (!gfxFont) {
    _log->debug(_p) << " └─ gfxFont: -" << std::endl;
    return;
  }

  // Get the font name. In some cases (e.g., if the type of the font is "type-3"), the gfxFont
  // may not provide a font name. If this is the case, use the pointer address of the font instead.
  std::stringstream gfxFontAddress;
  gfxFontAddress << (void const *) gfxFont;
  std::string fontName = gfxFontAddress.str();
  const GooString* gooFontName = gfxFont->getName();
  if (gooFontName) {
    fontName = gooFontName->toStr();
  }

  // Check if the info about the current font was already computed. If not, compute it.
  if (_doc->fontInfos.count(fontName) == 0) {
    _doc->fontInfos[fontName] = PdfFontInfo::create(state, _xref, !_noEmbeddedFontFilesParsing);
  }
  _fontInfo = _doc->fontInfos[fontName];

  _log->debug(_p) << " └─ font.name: " << _fontInfo->fontName << std::endl;
  _log->debug(_p) << " └─ font.basename: " << _fontInfo->fontBaseName << std::endl;
  _log->debug(_p) << " └─ font.normFontName: " << _fontInfo->normFontName << std::endl;
  _log->debug(_p) << " └─ font.ascent: " << _fontInfo->ascent << std::endl;
  _log->debug(_p) << " └─ font.descent: " << _fontInfo->descent << std::endl;
  _log->debug(_p) << " └─ font.isItalic: " << _fontInfo->isItalic << std::endl;
  _log->debug(_p) << " └─ font.isSerif: " << _fontInfo->isSerif << std::endl;
  _log->debug(_p) << " └─ font.isSymbolic: " << _fontInfo->isSymbolic << std::endl;
  _log->debug(_p) << " └─ font.isType3: " << _fontInfo->isType3 << std::endl;
  _log->debug(_p) << " └─ font.weight: " << _fontInfo->weight << std::endl;
}

// _________________________________________________________________________________________________
void TextOutputDev::drawChar(GfxState* state, double x, double y, double dx, double dy,
    double originX, double originY, CharCode c, int nBytes, const Unicode* u, int uLen) {
  _log->debug(_p) << "=======================================" << std::endl;
  _log->debug(_p) << "\033[1mEvent: DRAW CHAR\033[0m " << std::endl;

  if (!state) {
    _log->debug(_p) << " └─ state: -" << std::endl;
    return;
  }

  if (!_fontInfo) {
    _log->debug(_p) << " └─ fontInfo: -" << std::endl;
    return;
  }

  const Gfx8BitFont* gfx8BitFont = dynamic_cast<Gfx8BitFont*>(state->getFont());
  const GfxCIDFont* gfxCidFont = dynamic_cast<GfxCIDFont*>(state->getFont());

  // Parse the information about the character and store it in form of a `PdfGlyph`.
  PdfGlyph* glyph = new PdfGlyph();
  glyph->id = string_utils::createRandomString(8, "g-");
  glyph->doc = _doc;
  _log->debug(_p) << " └─ glyph.id: \"" << glyph->id << "\"" << std::endl;

  // ----------------------------------
  // The character name (e.g., "summationdisplay" for "Σ").

  std::string charName;
  GfxFont* gfxFont = state->getFont();
  if (gfx8BitFont) {
    const char* charNameArray = gfx8BitFont->getCharName(c);
    if ((charNameArray != nullptr) && (charNameArray[0] != '\0')) {
      charName = charNameArray;
    }
  }
  glyph->charName = charName;
  _log->debug(_p) << " └─ glyph.charName: \"" << glyph->charName << "\"" << std::endl;

  // ----------------------------------
  // The text of the glyph.

  std::string text;

  // If the glyph map contains an entry for the given char name, use the text stored in this map.
  // Otherwise, map the character code(s) to Unicode.
  if (uLen == 1 && glyphMap.count(charName) > 0) {
    glyph->unicodes.push_back(glyphMap.at(charName).first);
    text = glyphMap.at(charName).second;
  } else if (u) {
    const UnicodeMap* uMap;
    char buf[8];
    int n;

    if ((uMap = globalParams->getTextEncoding())) {
      // Usually, uLen == 1 (meaning that the glyph represents a single character.
      // But it may uLen > 1, for example for ligatures.
      for (int i = 0; i < uLen; i++) {
        n = uMap->mapUnicode(u[i], buf, sizeof(buf));
        text.append(buf, n);
      }
    }

    for (int i = 0; i < uLen; i++) {
      glyph->unicodes.push_back(u[i]);
    }
  }
  glyph->text = text;
  _log->debug(_p) << " └─ glyph.text: \"" << glyph->text << "\"" << std::endl;

  // Ignore the glyph if it represents a whitespace. We want to consider the "non-breaking space"
  // character (\u00a0) as a whitespace, but std::isspace does not. So we have to check the glyph
  // for a non-breaking space manually. To do so, convert the text to a wide character, as the
  // non-breaking space is a 2-byte character.
  std::wstring wText = _wStringConverter.from_bytes(text);
  bool isWhitespace = !wText.empty();
  for (wchar_t& ch : wText) {
    if (!std::iswspace(ch) && ch != 0x00a0) {
      isWhitespace = false;
      break;
    }
  }

  if (isWhitespace) {
    _log->debug(_p) << "\033[1mSkipping glyph (is a whitespace).\033[0m" << std::endl;
    delete glyph;
    return;
  }

  // ----------------------------------
  // The page number.

  glyph->position->pageNum = _page->pageNum;
  _log->debug(_p) << " └─ glyph.pageNum: " << glyph->position->pageNum << std::endl;

  // ----------------------------------
  // The rotation (this code is adopted from Poppler).

  const double* fontm;
  double m[4], m2[4];
  state->getFontTransMat(&m[0], &m[1], &m[2], &m[3]);
  if (gfxFont && gfxFont->getType() == fontType3) {
    fontm = state->getFont()->getFontMatrix();
    m2[0] = fontm[0] * m[0] + fontm[1] * m[2];
    m2[1] = fontm[0] * m[1] + fontm[1] * m[3];
    m2[2] = fontm[2] * m[0] + fontm[3] * m[2];
    m2[3] = fontm[2] * m[1] + fontm[3] * m[3];
    m[0] = m2[0];
    m[1] = m2[1];
    m[2] = m2[2];
    m[3] = m2[3];
  }
  if (fabs(m[0] * m[3]) > fabs(m[1] * m[2])) {
    glyph->position->rotation = (m[0] > 0 || m[3] < 0) ? 0 : 2;
  } else {
    glyph->position->rotation = (m[2] > 0) ? 1 : 3;
  }
  // In vertical writing mode, the lines are effectively rotated by 90 degrees.
  if (gfxFont && gfxFont->getWMode()) {
    glyph->position->rotation = (glyph->position->rotation + 1) & 3;
  }
  _log->debug(_p) << " └─ glyph.rotation: " << glyph->position->rotation << std::endl;

  // ----------------------------------
  // The writing mode.

  glyph->position->wMode = gfxFont->getWMode();
  _log->debug(_p) << " └─ glyph.wMode: " << glyph->position->wMode << std::endl;

  // ----------------------------------
  // The bounding box.

  // Compute the current text rendering matrix.
  double fontSize = state->getFontSize();
  double horizScaling = state->getHorizScaling();
  double rise = state->getRise();
  double tm0 = state->getTextMat()[0];
  double tm1 = state->getTextMat()[1];
  double tm2 = state->getTextMat()[2];
  double tm3 = state->getTextMat()[3];
  x = state->getCurX();
  y = state->getCurY();
  const double* ctm = state->getCTM();
  double params[6] = { fontSize * horizScaling, 0, 0, fontSize, 0, rise };
  double tm[6] = { tm0, tm1, tm2, tm3, x, y };
  double paramsXtm[6];
  concat(params, tm, paramsXtm);
  double trm[6];
  concat(paramsXtm, ctm, trm);

  // Compute the text rendering matrix of the next glyph.
  double width = 0;
  if (gfx8BitFont) {
    width = gfx8BitFont->getWidth(c);
  } else if (gfxCidFont) {
    if (nBytes > 0) {
      char* bytes = new char[nBytes];
      for (int k = 0; k < nBytes; k++) {
        bytes[k] = (c >> (8 * (nBytes - k - 1))) & 0xFF;
      }
      width = gfxCidFont->getWidth(bytes, nBytes);
      delete[] bytes;
    }
  }

  double td[6] = { 1, 0, 0, 1, width * fontSize * horizScaling, 0 };
  double tdXtm[6];
  concat(td, tm, tdXtm);
  double nextTrm[6];
  concat(tdXtm, ctm, nextTrm);

  double x0 = math_utils::round(trm[4], 1);
  double y0 = math_utils::round(trm[5], 1);
  double x1 = math_utils::round(nextTrm[4], 1);
  double y1 = math_utils::round(nextTrm[5], 1);
  double transformedFontSize = state->getTransformedFontSize();

  // Compute the ascent, that is: the maximum extent of the font above the base line.
  double ascent = _fontInfo ? _fontInfo->ascent * transformedFontSize : 0;
  // Compute the descent, that is: the maximum extent of the font below the base line.
  double descent = _fontInfo ? _fontInfo->descent * transformedFontSize : 0;

  glyph->position->leftX = x0 - transformedFontSize;
  glyph->position->upperY = y0 - transformedFontSize;
  glyph->position->rightX = x0;
  glyph->position->lowerY = y0;

  int wMode = gfxFont->getWMode();
  if (wMode) {  // vertical writing mode
    switch (glyph->position->rotation) {
      case 0:
        break;
      case 1:
        glyph->position->leftX = x0;
        glyph->position->upperY = y0 - transformedFontSize;
        glyph->position->rightX = x0 + transformedFontSize;
        glyph->position->lowerY = y0;
        break;
      case 2:
        glyph->position->leftX = x0;
        glyph->position->upperY = y0;
        glyph->position->rightX = x0 + transformedFontSize;
        glyph->position->lowerY = y0 + transformedFontSize;
        break;
      case 3:
        glyph->position->leftX = x0 - transformedFontSize;
        glyph->position->upperY = y0;
        glyph->position->rightX = x0;
        glyph->position->lowerY = y0 + transformedFontSize;
        break;
    }
  } else {  // horizontal writing mode
    switch (glyph->position->rotation) {
      case 0:
        glyph->position->leftX = x0;
        glyph->position->upperY = y0 - ascent;
        glyph->position->rightX = x0 + (x1 - x0);
        glyph->position->lowerY = y0 - descent;
        glyph->base = y0;
        break;
      case 1:
        glyph->position->leftX = x0 + descent;
        glyph->position->upperY = y0;
        glyph->position->rightX = x0 + ascent;
        glyph->position->lowerY = y0 + (y1 - y0);
        glyph->base = x0;
        break;
      case 2:
        glyph->position->leftX = x0;
        glyph->position->upperY = y0 + descent;
        glyph->position->rightX = x0 + (x1 - x0);
        glyph->position->lowerY = y0 + ascent;
        glyph->base = y0;
        break;
      case 3:
        glyph->position->leftX = x0 - ascent;
        glyph->position->upperY = y0 + (y1 - y0);
        glyph->position->rightX = x0 - descent;
        glyph->position->lowerY = y0;
        glyph->base = x0;
        break;
    }
  }

  // If the current font info contains exact bounding boxes (parsed from the embedded font files),
  // compute the exact bounding box for the current glyph and write it to `glyph->position` if the
  // vertical extent of the exact bounding box is smaller than of the bounding box computed above.
  // This should especially update the bounding boxes of math symbols like the summation symbol or
  // the product symbol, for which the computed bounding boxes are often by far too high.
  if (_fontInfo && _fontInfo->glyphBoundingBoxes.count(glyph->charName)) {
    auto& boundingBox = _fontInfo->glyphBoundingBoxes[glyph->charName];  // tuple of four doubles.
    double leftX = std::get<0>(boundingBox);
    double upperY = std::get<1>(boundingBox);
    double rightX = std::get<2>(boundingBox);
    double lowerY = std::get<3>(boundingBox);

    double* fm = _fontInfo->fontMatrix;
    double leftX2 = leftX * fm[0] + upperY * fm[2] + fm[4];
    double upperY2 = leftX * fm[1] + upperY * fm[3] + fm[5];
    double rightX2 = rightX * fm[0] + lowerY * fm[2] + fm[4];
    double lowerY2 = rightX * fm[1] + lowerY * fm[3] + fm[5];

    double leftX3 = leftX2 * trm[0] + upperY2 * trm[2] + trm[4];
    double upperY3 = leftX2 * trm[1] + upperY2 * trm[3] + trm[5];
    double rightX3 = rightX2 * trm[0] + lowerY2 * trm[2] + trm[4];
    double lowerY3 = rightX2 * trm[1] + lowerY2 * trm[3] + trm[5];

    leftX = std::min(leftX3, rightX3);
    upperY = std::min(upperY3, lowerY3);
    rightX = std::max(leftX3, rightX3);
    lowerY = std::max(upperY3, lowerY3);

    if (upperY < glyph->position->upperY || lowerY > glyph->position->lowerY) {
      glyph->position->leftX = leftX;
      glyph->position->upperY = upperY;
      glyph->position->rightX = rightX;
      glyph->position->lowerY = lowerY;
      glyph->base = lowerY;
    }
  }

  _log->debug(_p) << " └─ glyph.leftX: " << glyph->position->leftX << std::endl;
  _log->debug(_p) << " └─ glyph.upperY: " << glyph->position->upperY << std::endl;
  _log->debug(_p) << " └─ glyph.rightX: " << glyph->position->rightX << std::endl;
  _log->debug(_p) << " └─ glyph.lowerY: " << glyph->position->lowerY << std::endl;
  _log->debug(_p) << " └─ glyph.base: " << glyph->base << std::endl;
  if (glyph->position->rotation > 0) {
    _log->debug(_p) << " └─ glyph.rotLeftX: " << glyph->position->getRotLeftX() << std::endl;
    _log->debug(_p) << " └─ glyph.rotUpperY: " << glyph->position->getRotUpperY() << std::endl;
    _log->debug(_p) << " └─ glyph.rotRightX: " << glyph->position->getRotRightX() << std::endl;
    _log->debug(_p) << " └─ glyph.rotLowerY: " << glyph->position->getRotLowerY() << std::endl;
  }

  // ----------------------------------
  // The font name.

  glyph->fontName = _fontInfo ? _fontInfo->fontName : "";
  _log->debug(_p) << " └─ glyph.fontName: " << glyph->fontName << std::endl;

  // ----------------------------------
  // The font size (rounded to one decimal).

  glyph->fontSize = static_cast<double>(static_cast<int>(transformedFontSize * 10.)) / 10.;
  _log->debug(_p) << " └─ glyph.fontSize: " << glyph->fontSize << std::endl;

  // ----------------------------------
  // The extraction rank.

  glyph->rank = _numElements++;
  _log->debug(_p) << " └─ glyph.rank: " << glyph->rank << std::endl;

  // ----------------------------------
  // The opacity.

  glyph->opacity = state->getStrokeOpacity();
  _log->debug(_p) << " └─ glyph.opacity: " << glyph->opacity << std::endl;

  // ----------------------------------
  // The color.

  GfxRGB rgb;
  state->getStrokeRGB(&rgb);
  glyph->color[0] = colToDbl(rgb.r);
  glyph->color[1] = colToDbl(rgb.g);
  glyph->color[2] = colToDbl(rgb.b);
  _log->debug(_p) << " └─ glyph.color: [" << glyph->color[0] << ", " << glyph->color[1] << ", "
      << glyph->color[2] << "]" << std::endl;

  // ----------------------------------
  // Add the glyph to the page or to a figure, depending on the current clipbox. If the current
  // clipbox is equal to the page's clipbox, append the glyph to `page->glyphs`.
  // Otherwise, append the glyph to `figure->glyphs`, where `figure` is the `PdfFigure` object
  // related to the current clipbox. If no such object exists yet, create it.

  double clipLeftX, clipUpperY, clipRightX, clipLowerY;
  state->getClipBBox(&clipLeftX, &clipUpperY, &clipRightX, &clipLowerY);
  _log->debug(_p) << " └─ clipbox: leftX: " << clipLeftX << "; upperY: " << clipUpperY
      << "; rightX: " << clipRightX << "; lowerY: " << clipLowerY << std::endl;

  if (math_utils::equal(clipLeftX, _page->clipLeftX, 0.1) && math_utils::equal(clipUpperY, _page->clipUpperY, 0.1)
        && math_utils::equal(clipRightX, _page->clipRightX, 0.1) && math_utils::equal(clipLowerY, _page->clipLowerY, 0.1)) {
    _page->glyphs.push_back(glyph);
    _log->debug(_p) << "\033[1mAppended glyph to page.\033[0m" << std::endl;
    return;
  }

  // Iterate through the figures to check if there is a figure with the same current clipbox.
  for (auto* fig : _page->figures) {
    if (math_utils::equal(clipLeftX, fig->clipLeftX, 0.1) && math_utils::equal(clipUpperY, fig->clipUpperY, 0.1)
        && math_utils::equal(clipRightX, fig->clipRightX, 0.1) && math_utils::equal(clipLowerY, fig->clipLowerY, 0.1)) {
      // Update the bounding box of the figure.
      fig->position->leftX = std::min(fig->position->leftX, glyph->position->leftX);
      fig->position->upperY = std::min(fig->position->upperY, glyph->position->upperY);
      fig->position->rightX = std::max(fig->position->rightX, glyph->position->rightX);
      fig->position->lowerY = std::max(fig->position->lowerY, glyph->position->lowerY);
      fig->glyphs.push_back(glyph);
      _log->debug(_p) << "\033[1mAppended glyph to figure " << fig->id << ".\033[0m" << std::endl;
      return;
    }
  }

  // No figure exists for the current clipbox. Create a new figure and append the glyph to it.
  PdfFigure* figure = new PdfFigure();
  figure->id = string_utils::createRandomString(8, "fig-");
  figure->doc = _doc;
  figure->clipLeftX = clipLeftX;
  figure->clipUpperY = clipUpperY;
  figure->clipRightX = clipRightX;
  figure->clipLowerY = clipLowerY;
  figure->position->pageNum = _page->pageNum;
  figure->position->leftX = glyph->position->leftX;
  figure->position->upperY = glyph->position->upperY;
  figure->position->rightX = glyph->position->rightX;
  figure->position->lowerY = glyph->position->lowerY;
  figure->glyphs.push_back(glyph);

  _log->debug(_p) << "\033[1mAppended glyph to a new figure.\033[0m" << std::endl;
  _log->debug(_p) << " └─ figure.id: " << figure->id << std::endl;
  _log->debug(_p) << " └─ figure.clipLeftX: " << figure->clipLeftX << std::endl;
  _log->debug(_p) << " └─ figure.clipUpperY: " << figure->clipUpperY << std::endl;
  _log->debug(_p) << " └─ figure.clipRightX: " << figure->clipRightX << std::endl;
  _log->debug(_p) << " └─ figure.clipLowerY: " << figure->clipLowerY << std::endl;
  _log->debug(_p) << " └─ figure.pageNum: " << figure->position->pageNum << std::endl;
  _log->debug(_p) << " └─ figure.leftX: " << figure->position->leftX << std::endl;
  _log->debug(_p) << " └─ figure.upperY: " << figure->position->upperY << std::endl;
  _log->debug(_p) << " └─ figure.rightX: " << figure->position->rightX << std::endl;
  _log->debug(_p) << " └─ figure.lowerY: " << figure->position->lowerY << std::endl;

  _page->figures.push_back(figure);
}

// _________________________________________________________________________________________________
void TextOutputDev::stroke(GfxState* state) {
  _log->debug(_p) << "=======================================" << std::endl;
  _log->debug(_p) << "\033[1mEvent: STROKE PATH\033[0m " << std::endl;

  // Get the current clip box (= a rectangle defining the visible part of the path; a path not
  // falling into this rectangle is not visible to the reader of the PDF).
  double clipLeftX, clipUpperY, clipRightX, clipLowerY;
  state->getClipBBox(&clipLeftX, &clipUpperY, &clipRightX, &clipLowerY);

  // Iterate through each subpath and each point to compute the bounding box of the path.
  double x, y;
  double leftX = std::numeric_limits<double>::max();
  double upperY = std::numeric_limits<double>::max();
  double rightX = std::numeric_limits<double>::min();
  double lowerY = std::numeric_limits<double>::min();
  const GfxPath* path = state->getPath();
  for (int i = 0; i < path->getNumSubpaths(); i++) {
    const GfxSubpath* subpath = path->getSubpath(i);

    for (int j = 0; j < subpath->getNumPoints(); j++) {
      state->transform(subpath->getX(j), subpath->getY(j), &x, &y);
      // Ignore points that lies outside the clip box (since points outside the clip box are
      // not visible).
      // TODO: This is dangerous, since we may ignore a path that is actually visible, for example,
      // when the first endpoint of a line lies left to the clip box and the second endpoint lies
      // right to the clip box (and the connecting line goes straight through the clip box).
      if (x >= clipRightX || y >= clipLowerY || x <= clipLeftX || y <= clipUpperY) {
        continue;
      }
      leftX = std::max(std::min(leftX, x), clipLeftX);
      upperY = std::max(std::min(upperY, y), clipUpperY);
      rightX = std::min(std::max(rightX, x), clipRightX);
      lowerY = std::min(std::max(lowerY, y), clipLowerY);
    }
  }

  // Store the information about the path in form of a `PdfShape`.
  PdfShape* shape = new PdfShape();
  shape->id = string_utils::createRandomString(8, "shape-");
  shape->doc = _doc;
  shape->position->pageNum = _page->pageNum;
  shape->position->leftX = leftX;
  shape->position->upperY = upperY;
  shape->position->rightX = rightX;
  shape->position->lowerY = lowerY;
  shape->rank = _numElements++;

  _log->debug(_p) << " └─ shape.id: " << shape->id << std::endl;
  _log->debug(_p) << " └─ shape.pageNum: " << shape->position->pageNum << std::endl;
  _log->debug(_p) << " └─ shape.leftX: " << shape->position->leftX << std::endl;
  _log->debug(_p) << " └─ shape.upperY: " << shape->position->upperY << std::endl;
  _log->debug(_p) << " └─ shape.rightX: " << shape->position->rightX << std::endl;
  _log->debug(_p) << " └─ shape.lowerY: " << shape->position->lowerY << std::endl;
  _log->debug(_p) << " └─ shape.rank: " << shape->rank << std::endl;
  _log->debug(_p) << " └─ clipBox: leftX: " << clipLeftX << "; upperY: " << clipUpperY
      << "; rightX: " << clipRightX << "; lowerY: " << clipLowerY << std::endl;

  // Add the shape to the page or to a figure, depending on the current clipbox. If the current
  // clipbox is equal to the page's clipbox, append the shape to `page->shapes`.
  // Otherwise, append the shape to `figure->shapes`, where `figure` is the `PdfFigure` object
  // related to the current clipbox. If no such object exists yet, create it.
  if (math_utils::equal(clipLeftX, _page->clipLeftX, 0.1) && math_utils::equal(clipUpperY, _page->clipUpperY, 0.1)
        && math_utils::equal(clipRightX, _page->clipRightX, 0.1) && math_utils::equal(clipLowerY, _page->clipLowerY, 0.1)) {
    _page->shapes.push_back(shape);
    _log->debug(_p) << "\033[1mAppended shape to page.\033[0m" << std::endl;
    return;
  }

  for (auto* fig : _page->figures) {
    if (math_utils::equal(clipLeftX, fig->clipLeftX, 0.1) && math_utils::equal(clipUpperY, fig->clipUpperY, 0.1)
        && math_utils::equal(clipRightX, fig->clipRightX, 0.1) && math_utils::equal(clipLowerY, fig->clipLowerY, 0.1)) {
      // Update the bounding box of the figure.
      fig->position->leftX = std::min(fig->position->leftX, shape->position->leftX);
      fig->position->upperY = std::min(fig->position->upperY, shape->position->upperY);
      fig->position->rightX = std::max(fig->position->rightX, shape->position->rightX);
      fig->position->lowerY = std::max(fig->position->lowerY, shape->position->lowerY);
      fig->shapes.push_back(shape);
      _log->debug(_p) << "\033[1mAppended shape to figure " << fig->id << ".\033[0m" << std::endl;
      _log->debug(_p) << fig->position->toString() << std::endl;
      return;
    }
  }

  // No figure exists for the current clipbox. Create a new figure and append the shape to it.
  PdfFigure* figure = new PdfFigure();
  figure->id = string_utils::createRandomString(8, "fig-");
  figure->doc = _doc;
  figure->clipLeftX = clipLeftX;
  figure->clipUpperY = clipUpperY;
  figure->clipRightX = clipRightX;
  figure->clipLowerY = clipLowerY;
  figure->position->pageNum = _page->pageNum;
  figure->position->leftX = shape->position->leftX;
  figure->position->upperY = shape->position->upperY;
  figure->position->rightX = shape->position->rightX;
  figure->position->lowerY = shape->position->lowerY;
  figure->shapes.push_back(shape);

  _log->debug(_p) << "\033[1mAppended shape to a new figure.\033[0m" << std::endl;
  _log->debug(_p) << " └─ figure.id: " << figure->id << std::endl;
  _log->debug(_p) << " └─ figure.clipLeftX: " << figure->clipLeftX << std::endl;
  _log->debug(_p) << " └─ figure.clipUpperY: " << figure->clipUpperY << std::endl;
  _log->debug(_p) << " └─ figure.clipRightX: " << figure->clipRightX << std::endl;
  _log->debug(_p) << " └─ figure.clipLowerY: " << figure->clipLowerY << std::endl;
  _log->debug(_p) << " └─ figure.pageNum: " << figure->position->pageNum << std::endl;
  _log->debug(_p) << " └─ figure.leftX: " << figure->position->leftX << std::endl;
  _log->debug(_p) << " └─ figure.upperY: " << figure->position->upperY << std::endl;
  _log->debug(_p) << " └─ figure.rightX: " << figure->position->rightX << std::endl;
  _log->debug(_p) << " └─ figure.lowerY: " << figure->position->lowerY << std::endl;

  _page->figures.push_back(figure);
}

// _________________________________________________________________________________________________
void TextOutputDev::fill(GfxState* state) {
  stroke(state);
}

// _________________________________________________________________________________________________
void TextOutputDev::drawImageMask(GfxState* state, Object* ref, Stream* str, int width, int height,
    bool invert, bool interpolate, bool inlineImg) {
  drawGraphic(state);
}

// _________________________________________________________________________________________________
void TextOutputDev::drawImage(GfxState* state, Object* ref, Stream* str, int width, int height,
      GfxImageColorMap* colorMap, bool interpolate, const int* maskColors, bool inlineImg) {
  drawGraphic(state);
}

// _________________________________________________________________________________________________
void TextOutputDev::drawMaskedImage(GfxState* state, Object* ref, Stream* str, int width, int height,
    GfxImageColorMap* colorMap, bool interpolate, Stream* maskStr, int maskWidth, int maskHeight,
    bool maskInvert, bool maskInterpolate) {
  drawGraphic(state);
}

// _________________________________________________________________________________________________
void TextOutputDev::drawSoftMaskedImage(GfxState* state, Object* ref, Stream* str, int width,
    int height, GfxImageColorMap* colorMap, bool interpolate, Stream* maskStr, int maskWidth,
    int maskHeight, GfxImageColorMap* maskColorMap, bool maskInterpolate) {
  drawGraphic(state);
}

// _________________________________________________________________________________________________
void TextOutputDev::drawGraphic(GfxState* state) {
  _log->debug(_p) << "=======================================" << std::endl;
  _log->debug(_p) << "\033[1mEvent: DRAW GRAPHIC\033[0m " << std::endl;

  // Get the current clip box (= a rectangle defining the visible part of the image; parts of the
  // image not falling into this rectangle is not visible to the reader of the PDF).
  double clipLeftX, clipUpperY, clipRightX, clipLowerY;
  state->getClipBBox(&clipLeftX, &clipUpperY, &clipRightX, &clipLowerY);

  // Compute the bounding box of the image from the ctm.
  const double* ctm = state->getCTM();
  double leftX = ctm[4];  // ctm[4] = translateX
  double upperY = ctm[5];  // ctm[5] = translateY
  double rightX = leftX + ctm[0];  // ctm[0] = scaleX
  double lowerY = upperY + ctm[3];  // ctm[3] = scaleY

  // Handle each image as a PdfGraphic.
  PdfGraphic* graphic = new PdfGraphic();
  graphic->id = string_utils::createRandomString(8, "graphic-");
  graphic->doc = _doc;
  graphic->position->pageNum = _page->pageNum;
  graphic->position->leftX = std::max(std::min(leftX, rightX), clipLeftX);
  graphic->position->upperY = std::max(std::min(upperY, lowerY), clipUpperY);
  graphic->position->rightX = std::min(std::max(leftX, rightX), clipRightX);
  graphic->position->lowerY = std::min(std::max(upperY, lowerY), clipLowerY);
  graphic->rank = _numElements++;

  _log->debug(_p) << " └─ graphic.id: " << graphic->id << std::endl;
  _log->debug(_p) << " └─ graphic.pageNum: " << graphic->position->pageNum << std::endl;
  _log->debug(_p) << " └─ graphic.leftX: " << graphic->position->leftX << std::endl;
  _log->debug(_p) << " └─ graphic.upperY: " << graphic->position->upperY << std::endl;
  _log->debug(_p) << " └─ graphic.rightX: " << graphic->position->rightX << std::endl;
  _log->debug(_p) << " └─ graphic.lowerY: " << graphic->position->lowerY << std::endl;
  _log->debug(_p) << " └─ graphic.rank: " << graphic->rank << std::endl;
  _log->debug(_p) << " └─ clipBox: leftX: " << clipLeftX << "; upperY: " << clipUpperY
      << "; rightX: " << clipRightX << "; lowerY: " << clipLowerY << std::endl;

  if (clipLeftX == _page->clipLeftX && clipUpperY == _page->clipUpperY
        && clipRightX == _page->clipRightX && clipLowerY == _page->clipLowerY) {
    _page->graphics.push_back(graphic);
    _log->debug(_p) << "\033[1mAppended graphic to page.\033[0m" << std::endl;
    return;
  }

  for (auto* fig : _page->figures) {
    if (clipLeftX == fig->clipLeftX && clipUpperY == fig->clipUpperY
        && clipRightX == fig->clipRightX && clipLowerY == fig->clipLowerY) {
      fig->graphics.push_back(graphic);
      fig->position->leftX = std::min(fig->position->leftX, graphic->position->leftX);
      fig->position->upperY = std::min(fig->position->upperY, graphic->position->upperY);
      fig->position->rightX = std::max(fig->position->rightX, graphic->position->rightX);
      fig->position->lowerY = std::max(fig->position->lowerY, graphic->position->lowerY);
      _log->debug(_p) << "\033[1mAppended graphic to figure " << fig->id << ".\033[0m" << std::endl;
      return;
    }
  }

  // Create new figure.
  PdfFigure* figure = new PdfFigure();
  figure->id = string_utils::createRandomString(8, "fig-");
  figure->doc = _doc;
  figure->clipLeftX = clipLeftX;
  figure->clipUpperY = clipUpperY;
  figure->clipRightX = clipRightX;
  figure->clipLowerY = clipLowerY;
  figure->position->pageNum = _page->pageNum;
  figure->position->leftX = graphic->position->leftX;
  figure->position->upperY = graphic->position->upperY;
  figure->position->rightX = graphic->position->rightX;
  figure->position->lowerY = graphic->position->lowerY;
  figure->graphics.push_back(graphic);

  _log->debug(_p) << "\033[1mAppended graphic to a new figure.\033[0m" << std::endl;
  _log->debug(_p) << " └─ figure.id: " << figure->id << std::endl;
  _log->debug(_p) << " └─ figure.clipLeftX: " << figure->clipLeftX << std::endl;
  _log->debug(_p) << " └─ figure.clipUpperY: " << figure->clipUpperY << std::endl;
  _log->debug(_p) << " └─ figure.clipRightX: " << figure->clipRightX << std::endl;
  _log->debug(_p) << " └─ figure.clipLowerY: " << figure->clipLowerY << std::endl;
  _log->debug(_p) << " └─ figure.pageNum: " << figure->position->pageNum << std::endl;
  _log->debug(_p) << " └─ figure.leftX: " << figure->position->leftX << std::endl;
  _log->debug(_p) << " └─ figure.upperY: " << figure->position->upperY << std::endl;
  _log->debug(_p) << " └─ figure.rightX: " << figure->position->rightX << std::endl;
  _log->debug(_p) << " └─ figure.lowerY: " << figure->position->lowerY << std::endl;

  _page->figures.push_back(figure);
}

// _________________________________________________________________________________________________
void TextOutputDev::concat(const double* m1, const double* m2, double* res) const {
  res[0] = m1[0] * m2[0] + m1[1] * m2[2];
  res[1] = m1[0] * m2[1] + m1[1] * m2[3];
  res[2] = m1[2] * m2[0] + m1[3] * m2[2];
  res[3] = m1[2] * m2[1] + m1[3] * m2[3];
  res[4] = m1[4] * m2[0] + m1[5] * m2[2] + m2[4];
  res[5] = m1[4] * m2[1] + m1[5] * m2[3] + m2[5];
}
