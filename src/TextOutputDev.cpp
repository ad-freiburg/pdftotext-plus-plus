/**
 * Copyright 2021, University of Freiburg,
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
#include "./utils/Utils.h"


// _________________________________________________________________________________________________
TextOutputDev::TextOutputDev(bool parseEmbeddedFontFiles, PdfDocument* doc) {
  _doc = doc;
  _parseEmbeddedFontFiles = parseEmbeddedFontFiles;
  _fontInfo = nullptr;
  _fontSize = 0;
  _ok = true;
}

// _________________________________________________________________________________________________
TextOutputDev::~TextOutputDev() = default;

// _________________________________________________________________________________________________
void TextOutputDev::startPage(int pageNum, GfxState* state, XRef* xref) {
  _page = new PdfPage();
  _page->pageNum = pageNum;
  _page->width = state ? state->getPageWidth() : 0;
  _page->height = state ? state->getPageHeight() : 0;
  _doc->pages.push_back(_page);
  _xref = xref;
}

// _________________________________________________________________________________________________
void TextOutputDev::updateFont(GfxState* state) {
  // Get the current font size.
  // _fontSize = state->getTransformedFontSize();
  // _fontSize = state->getFontSize();

  // Get the current font info.
  _fontInfo = nullptr;
  GfxFont* gfxFont = state->getFont();

  if (gfxFont) {
    // Get the font name. In some cases (e.g., if the type of the font is "type-3"), the gfxFont
    // may not provide a font name. So use the pointer address of the font as default font name.
    std::stringstream gfxFontAddress;
    gfxFontAddress << (void const *) gfxFont;
    std::string fontName = gfxFontAddress.str();

    // If the font provide a font name, take this.
    const GooString* gooFontName = gfxFont->getName();
    if (gooFontName) {
      fontName = gooFontName->toStr();
    }

    // Check if the info about the current font was already computed.
    if (_doc->fontInfos.find(fontName) == _doc->fontInfos.end()) {
      // The info about the current font was not computed yet, so compute it.
      _doc->fontInfos[fontName] = PdfFontInfo::create(state, _xref, _parseEmbeddedFontFiles);
    }
    _fontInfo = _doc->fontInfos[fontName];
  }
}

// _________________________________________________________________________________________________
void TextOutputDev::drawChar(GfxState* state, double x, double y, double dx, double dy,
    double originX, double originY, CharCode c, int nBytes, const Unicode* u, int uLen) {
  if (!state) {
    return;
  }

  if (!_fontInfo) {
    return;
  }

  const Gfx8BitFont* gfx8BitFont = dynamic_cast<Gfx8BitFont*>(state->getFont());
  const GfxCIDFont* gfxCidFont = dynamic_cast<GfxCIDFont*>(state->getFont());

  // ----------------------------------
  // Compute and set the character name.

  std::string charName;
  GfxFont* gfxFont = state->getFont();
  if (gfx8BitFont) {
    const char* charNameArray = gfx8BitFont->getCharName(c);
    if ((charNameArray != nullptr) && (charNameArray[0] != '\0')) {
      charName = charNameArray;
    }
  }

  // ----------------------------------
  // Compute and set the text of the glyph.

  std::string text;

  if (glyphMap.count(charName)) {
    text = glyphMap.at(charName);
  } else if (u) {
    const UnicodeMap* uMap;
    char buf[8];
    int n;

    if ((uMap = globalParams->getTextEncoding())) {
      // Usually, uLen == 1 (meaning that the glyph represents a single character.
      // But it may uLen > 1, for example for ligatures.
      for (int i = 0; i < uLen; ++i) {
        n = uMap->mapUnicode(u[i], buf, sizeof(buf));
        text.append(buf, n);
      }
    }
  }

  // Ignore the glyph if it represents a whitespace. We want to consider the "non-breaking space"
  // character (\u00a0) as a whitespace, but std::isspace does not. So we have to check the glyph
  // for a non-breaking space manually. To do so, convert the text to a wide character, as the
  // non-breaking space is a 2-byte character.
  std::wstring wText = _wStringConverter.from_bytes(text);

  bool isWhitespace = true;
  for (wchar_t& ch : wText) {
    if (!std::iswspace(ch) && ch != 0x00a0) {
      isWhitespace = false;
      break;
    }
  }

  if (isWhitespace) {
    return;
  }

  PdfGlyph* glyph = new PdfGlyph();
  glyph->id = createRandomString(8, "g-");
  glyph->charName = charName;
  glyph->text = text;

  // ----------------------------------
  // Set the unicode codepoints of the glyph.

  for (int i = 0; i < uLen; i++) {
    glyph->unicodes.push_back(u[i]);
  }

  // ----------------------------------
  // Set the page number of the glyph.

  glyph->pageNum = _page->pageNum;

  // ----------------------------------
  // Compute and set the rotation of the glyph (this code is adopted from Poppler).

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
    glyph->rotation = (m[0] > 0 || m[3] < 0) ? 0 : 2;
  } else {
    glyph->rotation = (m[2] > 0) ? 1 : 3;
  }
  // In vertical writing mode, the lines are effectively rotated by 90 degrees.
  if (gfxFont && gfxFont->getWMode()) {
    glyph->rotation = (glyph->rotation + 1) & 3;
  }

  // ----------------------------------
  // Set the writing mode of the glyph.

  glyph->wMode = gfxFont->getWMode();

  // ----------------------------------
  // Compute the x,y-coordinates of the bounding box around the glyph.

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

  double x0 = round(trm[4], 1);
  double y0 = round(trm[5], 1);
  double x1 = round(nextTrm[4], 1);
  double y1 = round(nextTrm[5], 1);
  double transformedFontSize = state->getTransformedFontSize();

  // Compute the ascent, that is: the maximum extent of the font above the base line.
  double ascent = _fontInfo ? _fontInfo->ascent * transformedFontSize : 0;
  // Compute the descent, that is: the maximum extent of the font below the base line.
  double descent = _fontInfo ? _fontInfo->descent * transformedFontSize : 0;

  int wMode = gfxFont->getWMode();
  if (wMode) {  // vertical writing mode
    switch (glyph->rotation) {
      case 0:
        glyph->minX = x0 - transformedFontSize;
        glyph->minY = y0 - transformedFontSize;
        glyph->maxX = x0;
        glyph->maxY = y0;
        break;
      case 1:
        glyph->minX = x0;
        glyph->minY = y0 - transformedFontSize;
        glyph->maxX = x0 + transformedFontSize;
        glyph->maxY = y0;
        break;
      case 2:
        glyph->minX = x0;
        glyph->minY = y0;
        glyph->maxX = x0 + transformedFontSize;
        glyph->maxY = y0 + transformedFontSize;
        break;
      case 3:
        glyph->minX = x0 - transformedFontSize;
        glyph->minY = y0;
        glyph->maxX = x0;
        glyph->maxY = y0 + transformedFontSize;
        break;
    }
  } else {  // horizontal writing mode
    switch (glyph->rotation) {
      case 0:
        glyph->minX = x0;
        glyph->minY = y0 - ascent;
        glyph->maxX = x0 + (x1 - x0);
        glyph->maxY = y0 - descent;
        glyph->base = y0;
        break;
      case 1:
        glyph->minX = x0 + descent;
        glyph->minY = y0;
        glyph->maxX = x0 + ascent;
        glyph->maxY = y0 + (y1 - y0);
        glyph->base = x0;
        break;
      case 2:
        glyph->minX = x0;
        glyph->minY = y0 + descent;
        glyph->maxX = x0 + (x1 - x0);
        glyph->maxY = y0 + ascent;
        glyph->base = y0;
        break;
      case 3:
        glyph->minX = x0 - ascent;
        glyph->minY = y0 + (y1 - y0);
        glyph->maxX = x0 - descent;
        glyph->maxY = y0;
        glyph->base = x0;
        break;
    }
  }

  // ===============================================================================================

  // TODO(korzen):
  if (_fontInfo && _fontInfo->glyphBoundingBoxes.count(glyph->charName)) {
    auto boundingBox = _fontInfo->glyphBoundingBoxes[glyph->charName];
    double minX = std::get<0>(boundingBox);
    double minY = std::get<1>(boundingBox);
    double maxX = std::get<2>(boundingBox);
    double maxY = std::get<3>(boundingBox);

    double* fm = _fontInfo->fontMatrix;
    double minX2 = minX * fm[0] + minY * fm[2] + fm[4];
    double minY2 = minX * fm[1] + minY * fm[3] + fm[5];
    double maxX2 = maxX * fm[0] + maxY * fm[2] + fm[4];
    double maxY2 = maxX * fm[1] + maxY * fm[3] + fm[5];

    double minX3 = minX2 * trm[0] + minY2 * trm[2] + trm[4];
    double minY3 = minX2 * trm[1] + minY2 * trm[3] + trm[5];
    double maxX3 = maxX2 * trm[0] + maxY2 * trm[2] + trm[4];
    double maxY3 = maxX2 * trm[1] + maxY2 * trm[3] + trm[5];

    minX = std::min(minX3, maxX3);
    minY = std::min(minY3, maxY3);
    maxX = std::max(minX3, maxX3);
    maxY = std::max(minY3, maxY3);

    if (minY < glyph->minY || maxY > glyph->maxY) {
      glyph->minX = minX;
      glyph->minY = minY;
      glyph->maxX = maxX;
      glyph->maxY = maxY;
    }
  }

  // ----------------------------------
  // Set the font name.

  glyph->fontName = _fontInfo ? _fontInfo->fontName : "";

  // ----------------------------------
  // Set the font size.

  double fs = static_cast<double>(static_cast<int>(transformedFontSize * 10.)) / 10.;
  glyph->fontSize = fs;

  // ----------------------------------
  // Set the extraction rank.

  glyph->rank = _numGlyphs++;

  // if (glyph->pageNum == 1) {
  //   std::cout << glyph->toString() << std::endl;
  // }
  
  _page->glyphs.push_back(glyph);
}

// _________________________________________________________________________________________________
void TextOutputDev::stroke(GfxState* state) {
  // Get the current clip box (= a rectangle defining the visible part of the path; a path not
  // falling into this rectangle is not visible to the reader of the PDF). Note that multiple
  // paths can have the same clip box.
  double clipMinX, clipMinY, clipMaxX, clipMaxY;
  state->getClipBBox(&clipMinX, &clipMinY, &clipMaxX, &clipMaxY);

  double clipBoxWidth = clipMaxX - clipMinX;
  double clipBoxHeight = clipMaxY - clipMinY;

  // A clip box can span the whole page or only a part of the page.
  // If it spans the whole page, we assume that the path does not have a clip box.
  // If it spans only a part of the page, we assume that the path has a clip box and consider the
  // clip box itself as a non-text element - and not the actual path because otherwise we may
  // include (parts of the) paths which are actually not visible in the PDF.

  // Check if the clip box spans the whole page by checking if the width or height of the clip box
  // is smaller than the width/height of the page, allowing a small threshold.
  double xOverlapRatio = clipBoxWidth / _page->width;
  double yOverlapRatio = clipBoxHeight / _page->height;
  if (xOverlapRatio < 0.9 || yOverlapRatio < 0.9) {
    PdfNonText* nonText = new PdfNonText();
    nonText->id = createRandomString(8, "nt-");
    nonText->pageNum = _page->pageNum;
    nonText->minX = clipMinX;
    nonText->minY = clipMinY;
    nonText->maxX = clipMaxX;
    nonText->maxY = clipMaxY;

    _page->nonTexts.push_back(nonText);
    return;
  }

  // The clip box is equal to the bounding box of the page. Instead of the clip box, include the
  // actual path as a non-text element.
  PdfNonText* nonText = new PdfNonText();
  nonText->id = createRandomString(8, "nt-");
  nonText->pageNum = _page->pageNum;

  // Iterate through each subpath and each point to compute the bounding box of the path.
  double x, y;
  const GfxPath* path = state->getPath();
  for (int i = 0; i < path->getNumSubpaths(); i++) {
    const GfxSubpath* subpath = path->getSubpath(i);

    for (int j = 0; j < subpath->getNumPoints(); j++) {
      state->transform(subpath->getX(j), subpath->getY(j), &x, &y);
      nonText->minX = std::min(nonText->minX, x);
      nonText->minY = std::min(nonText->minY, y);
      nonText->maxX = std::max(nonText->maxX, x);
      nonText->maxY = std::max(nonText->maxY, y);
    }
  }

  _page->nonTexts.push_back(nonText);
}

// _________________________________________________________________________________________________
void TextOutputDev::fill(GfxState* state) {
  // Handle a "fill path" event in the same way as a "stroke path" event (this is ok, because we
  // only need the position of the path, but not the information whether the path is filled or not).
  stroke(state);
}

// _________________________________________________________________________________________________
void TextOutputDev::drawImageMask(GfxState* state, Object* ref, Stream* str, int width, int height,
    bool invert, bool interpolate, bool inlineImg) {
  drawImage(state, width, height);
}

// _________________________________________________________________________________________________
void TextOutputDev::drawImage(GfxState* state, Object* ref, Stream* str, int width, int height,
      GfxImageColorMap* colorMap, bool interpolate, const int* maskColors, bool inlineImg) {
  drawImage(state, width, height);
}

// _________________________________________________________________________________________________
void TextOutputDev::drawMaskedImage(GfxState* state, Object* ref, Stream* str, int width, int height,
    GfxImageColorMap* colorMap, bool interpolate, Stream* maskStr, int maskWidth, int maskHeight,
    bool maskInvert, bool maskInterpolate) {
  drawImage(state, width, height);
}

// _________________________________________________________________________________________________
void TextOutputDev::drawSoftMaskedImage(GfxState* state, Object* ref, Stream* str, int width,
    int height, GfxImageColorMap* colorMap, bool interpolate, Stream* maskStr, int maskWidth,
    int maskHeight, GfxImageColorMap* maskColorMap, bool maskInterpolate) {
  drawImage(state, width, height);
}

// _________________________________________________________________________________________________
void TextOutputDev::drawImage(GfxState* state, int width, int height) {
  // Get the current clip box (= a rectangle defining the visible part of the image; parts of the
  // image not falling into this rectangle are not visible to the reader of the PDF).
  double clipMinX, clipMinY, clipMaxX, clipMaxY;
  state->getClipBBox(&clipMinX, &clipMinY, &clipMaxX, &clipMaxY);

  double clipBoxWidth = clipMaxX - clipMinX;
  double clipBoxHeight = clipMaxY - clipMinY;

  // A clip box can span the whole page or only a part of the page.
  // If it spans the whole page, we assume that the image does not have a clip box.
  // If it spans only a part of the page, we assume that the image has a clip box and consider the
  // clip box itself as a non-text element - and not the actual image because otherwise we may
  // include (parts of the) image which are actually not visible in the PDF.

  // Check if the clip box spans the whole page by checking if the width or height of the clip box
  // is smaller than the width/height of the page, allowing a small threshold.
  double xOverlapRatio = clipBoxWidth / _page->width; // TODO: Compute the real overlap ratio.
  double yOverlapRatio = clipBoxHeight / _page->height;
  // std::cout << "IMAGE " << xOverlapRatio << " " << yOverlapRatio;
  if (xOverlapRatio < 0.9 || yOverlapRatio < 0.9) {
    PdfNonText* nonText = new PdfNonText();
    nonText->id = createRandomString(8, "nt-");
    nonText->pageNum = _page->pageNum;
    nonText->minX = clipMinX;
    nonText->minY = clipMinY;
    nonText->maxX = clipMaxX;
    nonText->maxY = clipMaxY;

    // std::cout << nonText->toString() << std::endl;

    _page->nonTexts.push_back(nonText);
    return;
  }

  // The clip box is equal to the bounding box of the page. Instead of the clip box, include the
  // actual image as a non-text element. Compute the bounding box from the ctm.
  const double* ctm = state->getCTM();

  PdfNonText* nonText = new PdfNonText();
  nonText->id = createRandomString(8, "nt-");
  nonText->pageNum = _page->pageNum;
  nonText->minX = ctm[4];  // ctm[4] = translateX
  nonText->minY = ctm[5];  // ctm[5] = translateY
  nonText->maxX = nonText->minX + ctm[0];  // ctm[0] = scaleX
  nonText->maxY = nonText->minY + ctm[3];  // ctm[3] = scaleY

  _page->nonTexts.push_back(nonText);
}

// _________________________________________________________________________________________________
void TextOutputDev::restoreState(GfxState* state) {
  // Nothing to do so far.
}


// _________________________________________________________________________________________________
void TextOutputDev::endPage() {
  // Nothing to do so far.
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
