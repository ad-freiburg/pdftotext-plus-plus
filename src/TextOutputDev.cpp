/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <goo/GooString.h>
#include <poppler/GfxFont.h>
#include <poppler/GfxState.h>
#include <poppler/GlobalParams.h>

#include <algorithm>  // min, max
#include <cmath>  // fabs
#include <limits>  // numeric_limits
#include <sstream>  // std::stringstream
#include <string>

#include "./utils/CharMap.h"
#include "./utils/Log.h"
#include "./utils/MathUtils.h"
#include "./utils/StringUtils.h"

#include "./PdfDocument.h"
#include "./TextOutputDev.h"

using std::endl;
using std::get;
using std::max;
using std::min;
using std::numeric_limits;
using std::string;
using std::stringstream;
using std::wstring;

// _________________________________________________________________________________________________
TextOutputDev::TextOutputDev(bool parseEmbeddedFontFiles, PdfDocument* doc, bool debug,
      int debugPageFilter) {
  _log = new Logger(debug ? DEBUG : INFO, debugPageFilter);
  _parseEmbeddedFontFiles = parseEmbeddedFontFiles;
  _doc = doc;

  _log->debug() << BOLD << "PDF parsing - DEBUG MODE" << OFF << endl;
  _log->debug() << " └─ parse embedded font files: " << parseEmbeddedFontFiles << endl;
}

// _________________________________________________________________________________________________
TextOutputDev::~TextOutputDev() {
  delete _log;
}

// _________________________________________________________________________________________________
void TextOutputDev::startPage(int pageNum, GfxState* state, XRef* xref) {
  assert(state);
  assert(xref);

  _page = new PdfPage();
  _page->pageNum = _p = pageNum;
  state->getClipBBox(&_page->clipLeftX, &_page->clipUpperY, &_page->clipRightX, &_page->clipLowerY);
  _doc->pages.push_back(_page);
  _xref = xref;

  _log->debug(_p) << "=======================================" << endl;
  _log->debug(_p) << BOLD << "Event: START PAGE" << OFF << endl;
  _log->debug(_p) << " └─ page.pageNum: " << _page->pageNum << endl;
  _log->debug(_p) << " └─ page.clipLeftX: " << _page->clipLeftX << endl;
  _log->debug(_p) << " └─ page.clipUpperY: " << _page->clipUpperY << endl;
  _log->debug(_p) << " └─ page.clipRightX: " << _page->clipRightX << endl;
  _log->debug(_p) << " └─ page.clipLowerY: " << _page->clipLowerY << endl;
  _log->debug(_p) << " └─ page.width: " << _page->getWidth() << endl;
  _log->debug(_p) << " └─ page.height: " << _page->getHeight() << endl;
}

// _________________________________________________________________________________________________
void TextOutputDev::updateFont(GfxState* state) {
  assert(state);

  _log->debug(_p) << "=======================================" << endl;
  _log->debug(_p) << BOLD << "Event: UPDATE FONT" << OFF << endl;

  // Revoke the current font info.
  _fontInfo = nullptr;

  // Skip the event if the state does not contain any font.
  GfxFont* gfxFont = state->getFont();
  if (!gfxFont) {
    _log->debug(_p) << " └─ gfxFont: -" << endl;
    return;
  }

  // Get the font name. In some cases (e.g., if the type of the font is "type-3"), the gfxFont
  // may not provide a font name. In this case, use the pointer address of the font instead.
  stringstream gfxFontAddress;
  gfxFontAddress << (void const*) gfxFont;
  string fontName = gfxFontAddress.str();
  const GooString* gooFontName = gfxFont->getName();
  if (gooFontName) {
    fontName = gooFontName->toStr();
  }

  // Check if the info about the current font was already computed. If not, compute it.
  if (_doc->fontInfos.count(fontName) == 0) {
    _doc->fontInfos[fontName] = PdfFontInfo::create(state, _xref, _parseEmbeddedFontFiles);
  }
  _fontInfo = _doc->fontInfos[fontName];

  _log->debug(_p) << " └─ font.name: " << _fontInfo->fontName << endl;
  _log->debug(_p) << " └─ font.basename: " << _fontInfo->fontBaseName << endl;
  _log->debug(_p) << " └─ font.normFontName: " << _fontInfo->normFontName << endl;
  _log->debug(_p) << " └─ font.ascent: " << _fontInfo->ascent << endl;
  _log->debug(_p) << " └─ font.descent: " << _fontInfo->descent << endl;
  _log->debug(_p) << " └─ font.isItalic: " << _fontInfo->isItalic << endl;
  _log->debug(_p) << " └─ font.isSerif: " << _fontInfo->isSerif << endl;
  _log->debug(_p) << " └─ font.isSymbolic: " << _fontInfo->isSymbolic << endl;
  _log->debug(_p) << " └─ font.isType3: " << _fontInfo->isType3 << endl;
  _log->debug(_p) << " └─ font.weight: " << _fontInfo->weight << endl;
}

// _________________________________________________________________________________________________
void TextOutputDev::drawChar(GfxState* state, double x, double y, double dx, double dy,
    double originX, double originY, CharCode c, int nBytes, const Unicode* u, int uLen) {
  assert(state);

  _log->debug(_p) << "=======================================" << endl;
  _log->debug(_p) << BOLD << "Event: DRAW CHAR" << OFF << endl;

  if (!_fontInfo) {
    _log->debug(_p) << " └─ fontInfo: -" << endl;
    return;
  }

  // Parse the information about the character and store it in form of a `PdfCharacter`.
  PdfCharacter* ch = new PdfCharacter();
  ch->doc = _doc;

  // ----------------------------------
  // Create a (unique) id.

  ch->id = string_utils::createRandomString(global_config::IDS_LENGTH, "char-");
  _log->debug(_p) << " └─ char.id: \"" << ch->id << "\"" << endl;

  // ----------------------------------
  // Get the character name provided by the PDF (e.g., "summationdisplay" for "Σ").

  const Gfx8BitFont* gfx8BitFont = dynamic_cast<Gfx8BitFont*>(state->getFont());
  if (gfx8BitFont) {
    const char* charNameArray = gfx8BitFont->getCharName(c);
    if ((charNameArray != nullptr) && (charNameArray[0] != '\0')) {
      ch->name = charNameArray;
    }
  }
  _log->debug(_p) << " └─ char.name: \"" << ch->name << "\"" << endl;

  // ----------------------------------
  // Get the text of the character.

  // If the character map contains an entry for the given char name, use the text provided by this
  // entry. Otherwise, map the character code(s) to Unicode.
  if (uLen == 1 && charMap.count(ch->name) > 0) {
    ch->unicodes.push_back(charMap.at(ch->name).first);
    ch->text = charMap.at(ch->name).second;
  } else if (u) {
    const UnicodeMap* uMap;
    char buf[8];
    int n;

    if ((uMap = globalParams->getTextEncoding())) {
      // Usually, uLen == 1 (meaning that the character represents a single character.
      // But it may uLen > 1, for example for ligatures.
      for (int i = 0; i < uLen; i++) {
        n = uMap->mapUnicode(u[i], buf, sizeof(buf));
        ch->text.append(buf, n);
      }
    }

    for (int i = 0; i < uLen; i++) {
      ch->unicodes.push_back(u[i]);
    }
  }
  _log->debug(_p) << " └─ char.text: \"" << ch->text << "\"" << endl;

  // Ignore the character if it represents a whitespace. We also want to consider the "non-breaking
  // space" character (\u00a0) as a whitespace, but std::isspace does not. So we have to check the
  // char for a non-breaking space manually. To do so, convert the text to a wide character, as the
  // non-breaking space is a 2-byte character.
  wstring wText = _wStringConverter.from_bytes(ch->text);
  bool isWhitespace = !wText.empty();
  for (wchar_t& ch : wText) {
    if (!std::iswspace(ch) && ch != 0x00a0) {
      isWhitespace = false;
      break;
    }
  }
  if (isWhitespace) {
    _log->debug(_p) << BOLD << "Skipping the character (is a whitespace)." << OFF << endl;
    delete ch;
    return;
  }

  // ----------------------------------
  // Get the page number.

  ch->position->pageNum = _page->pageNum;
  _log->debug(_p) << " └─ char.pageNum: " << ch->position->pageNum << endl;

  // ----------------------------------
  // Get the rotation (this code is stolen from Poppler).

  const double* fontm;
  double m[4], m2[4];
  state->getFontTransMat(&m[0], &m[1], &m[2], &m[3]);
  GfxFont* gfxFont = state->getFont();
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
    ch->position->rotation = (m[0] > 0 || m[3] < 0) ? 0 : 2;
  } else {
    ch->position->rotation = (m[2] > 0) ? 1 : 3;
  }
  // In vertical writing mode, the lines are effectively rotated by 90 degrees.
  if (gfxFont && gfxFont->getWMode()) {
    ch->position->rotation = (ch->position->rotation + 1) & 3;
  }
  _log->debug(_p) << " └─ char.rotation: " << ch->position->rotation << endl;

  // ----------------------------------
  // Get the writing mode.

  ch->position->wMode = gfxFont->getWMode();
  _log->debug(_p) << " └─ char.wMode: " << ch->position->wMode << endl;

  // ----------------------------------
  // Compute the bounding box.
  // NOTE: There are two methods to compute the bounding box:
  // (1) Computing the bounding box from the text rendering matrix and the ascent, and descent.
  //     This results in bounding boxes that are usually taller than the actual character, because
  //     they respect the maximum ascent and descent of the font. The bounding boxes are accurate
  //     in "most cases". For some characters (for example, for mathematical symbols like the
  //     summation symbol, the bounding boxes are *not* accurate, since they are shifted in
  //     y-direction (typically, they don't even overlap the character).
  // (2) Computing the bounding box from the "glyph bounding boxes" parsed from the embedded font
  //     file (if the font is actually embedded). This results in "tight" bounding boxes, meaning
  //     that they exactly fit the width and height of the character (and do not respect the
  //     maximum ascent and descent of the font). The bounding boxes are very accurate and the
  //     effect that bounding boxes are shifted (like in the first method) does not exist.
  // Because of the "shifted bounding box" issue of the first method, we use both methods to
  // compute both bounding boxes for each character. To bypass the issue, we use the bounding box
  // of the second method if has a larger vertical extent than the bounding box of the first method
  // (that is: if its upperY is smaller or its lowerY is larger). Otherwise, we use the bounding
  // box of the first method.

  // Compute the bounding box via the text rendering matrix; this code is stolen from Poppler.
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

  // Compute the text rendering matrix of the next char.
  const GfxCIDFont* gfxCidFont = dynamic_cast<GfxCIDFont*>(state->getFont());
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

  // Compute leftX, upperY, rightX, lowerY dependent on the writing mode and rotation.
  ch->position->leftX = x0 - transformedFontSize;
  ch->position->upperY = y0 - transformedFontSize;
  ch->position->rightX = x0;
  ch->position->lowerY = y0;

  int wMode = gfxFont->getWMode();
  if (wMode) {  // vertical writing mode
    switch (ch->position->rotation) {
      case 0:
        break;
      case 1:
        ch->position->leftX = x0;
        ch->position->upperY = y0 - transformedFontSize;
        ch->position->rightX = x0 + transformedFontSize;
        ch->position->lowerY = y0;
        break;
      case 2:
        ch->position->leftX = x0;
        ch->position->upperY = y0;
        ch->position->rightX = x0 + transformedFontSize;
        ch->position->lowerY = y0 + transformedFontSize;
        break;
      case 3:
        ch->position->leftX = x0 - transformedFontSize;
        ch->position->upperY = y0;
        ch->position->rightX = x0;
        ch->position->lowerY = y0 + transformedFontSize;
        break;
    }
  } else {  // horizontal writing mode
    switch (ch->position->rotation) {
      case 0:
        ch->position->leftX = x0;
        ch->position->upperY = y0 - ascent;
        ch->position->rightX = x0 + (x1 - x0);
        ch->position->lowerY = y0 - descent;
        ch->base = y0;
        break;
      case 1:
        ch->position->leftX = x0 + descent;
        ch->position->upperY = y0;
        ch->position->rightX = x0 + ascent;
        ch->position->lowerY = y0 + (y1 - y0);
        ch->base = x0;
        break;
      case 2:
        ch->position->leftX = x0;
        ch->position->upperY = y0 + descent;
        ch->position->rightX = x0 + (x1 - x0);
        ch->position->lowerY = y0 + ascent;
        ch->base = y0;
        break;
      case 3:
        ch->position->leftX = x0 - ascent;
        ch->position->upperY = y0 + (y1 - y0);
        ch->position->rightX = x0 - descent;
        ch->position->lowerY = y0;
        ch->base = x0;
        break;
    }
  }

  // Compute the bounding box from the "glyph bounding boxes" parsed from the embedded font file.
  if (_fontInfo && _fontInfo->glyphBoundingBoxes.count(ch->name)) {
    auto& boundingBox = _fontInfo->glyphBoundingBoxes[ch->name];  // tuple of four doubles.
    double leftX = get<0>(boundingBox);
    double upperY = get<1>(boundingBox);
    double rightX = get<2>(boundingBox);
    double lowerY = get<3>(boundingBox);

    double* fm = _fontInfo->fontMatrix;
    double leftX2 = leftX * fm[0] + upperY * fm[2] + fm[4];
    double upperY2 = leftX * fm[1] + upperY * fm[3] + fm[5];
    double rightX2 = rightX * fm[0] + lowerY * fm[2] + fm[4];
    double lowerY2 = rightX * fm[1] + lowerY * fm[3] + fm[5];

    double leftX3 = leftX2 * trm[0] + upperY2 * trm[2] + trm[4];
    double upperY3 = leftX2 * trm[1] + upperY2 * trm[3] + trm[5];
    double rightX3 = rightX2 * trm[0] + lowerY2 * trm[2] + trm[4];
    double lowerY3 = rightX2 * trm[1] + lowerY2 * trm[3] + trm[5];

    leftX = min(leftX3, rightX3);
    upperY = min(upperY3, lowerY3);
    rightX = max(leftX3, rightX3);
    lowerY = max(upperY3, lowerY3);

    // Update the bounding box when the alternative bounding box has a larger vertical extent.
    if (upperY < ch->position->upperY || lowerY > ch->position->lowerY) {
      ch->position->leftX = leftX;
      ch->position->upperY = upperY;
      ch->position->rightX = rightX;
      ch->position->lowerY = lowerY;
      ch->base = lowerY;
    }
  }

  _log->debug(_p) << " └─ char.leftX: " << ch->position->leftX << endl;
  _log->debug(_p) << " └─ char.upperY: " << ch->position->upperY << endl;
  _log->debug(_p) << " └─ char.rightX: " << ch->position->rightX << endl;
  _log->debug(_p) << " └─ char.lowerY: " << ch->position->lowerY << endl;
  _log->debug(_p) << " └─ char.base: " << ch->base << endl;
  if (ch->position->rotation > 0) {
    _log->debug(_p) << " └─ char.rotLeftX: " << ch->position->getRotLeftX() << endl;
    _log->debug(_p) << " └─ char.rotUpperY: " << ch->position->getRotUpperY() << endl;
    _log->debug(_p) << " └─ char.rotRightX: " << ch->position->getRotRightX() << endl;
    _log->debug(_p) << " └─ char.rotLowerY: " << ch->position->getRotLowerY() << endl;
  }

  // ----------------------------------
  // Get the font name.

  ch->fontName = _fontInfo ? _fontInfo->fontName : "";
  _log->debug(_p) << " └─ char.fontName: " << ch->fontName << endl;

  // ----------------------------------
  // Get the font size.

  ch->fontSize = math_utils::round(transformedFontSize, FONT_SIZE_PREC);
  _log->debug(_p) << " └─ char.fontSize: " << ch->fontSize << endl;

  // ----------------------------------
  // Get the extraction rank.

  ch->rank = _numElements++;
  _log->debug(_p) << " └─ char.rank: " << ch->rank << endl;

  // ----------------------------------
  // Get the opacity.

  ch->opacity = state->getStrokeOpacity();
  _log->debug(_p) << " └─ char.opacity: " << ch->opacity << endl;

  // ----------------------------------
  // Get the stroking color in RGB format (three doubles between 0 and 1).

  GfxRGB rgb;
  state->getStrokeRGB(&rgb);
  ch->color[0] = colToDbl(rgb.r);
  ch->color[1] = colToDbl(rgb.g);
  ch->color[2] = colToDbl(rgb.b);
  _log->debug(_p) << " └─ char.color: [" << ch->color[0] << ", " << ch->color[1] << ", "
      << ch->color[2] << "]" << endl;

  // ----------------------------------
  // Add the character to the page or to a figure, depending on the current clip box. If the
  // current clip box is equal to the page's clip box, append the character to `page->characters`.
  // Otherwise, append the character to `figure->characters`, where `figure` is the `PdfFigure`
  // with a clip box equal to the current clip box. If no such figure exists yet, create it.
  // NOTE: Our assumption here is that each clip box, which is different to the page's clip box,
  // represents a separate figure and that all text falling into the same clip box belong to the
  // same figure. We create exactly one figure per clip box. This figure is created when the clip
  // box is "seen" for the first time.

  double clipLeftX, clipUpperY, clipRightX, clipLowerY;
  state->getClipBBox(&clipLeftX, &clipUpperY, &clipRightX, &clipLowerY);
  _log->debug(_p) << " └─ clipbox: leftX: " << clipLeftX << "; upperY: " << clipUpperY
      << "; rightX: " << clipRightX << "; lowerY: " << clipLowerY << endl;

  // Check if the clip box is equal to the page's clip box. If so, add the character to the page.
  if (math_utils::equal(clipLeftX, _page->clipLeftX)
        && math_utils::equal(clipUpperY, _page->clipUpperY)
        && math_utils::equal(clipRightX, _page->clipRightX)
        && math_utils::equal(clipLowerY, _page->clipLowerY)) {
    _page->characters.push_back(ch);
    _log->debug(_p) << BOLD << "Appended char to page." << OFF << endl;
    return;
  }

  // Iterate through the figures to check if there is a figure with a clip box equal to the current
  // clip box. If so, append the character to this figure.
  for (auto* figure : _page->figures) {
    if (math_utils::equal(clipLeftX, figure->clipLeftX)
          && math_utils::equal(clipUpperY, figure->clipUpperY)
          && math_utils::equal(clipRightX, figure->clipRightX)
          && math_utils::equal(clipLowerY, figure->clipLowerY)) {
      // Update the bounding box of the figure.
      figure->position->leftX = min(figure->position->leftX, ch->position->leftX);
      figure->position->upperY = min(figure->position->upperY, ch->position->upperY);
      figure->position->rightX = max(figure->position->rightX, ch->position->rightX);
      figure->position->lowerY = max(figure->position->lowerY, ch->position->lowerY);
      figure->characters.push_back(ch);
      _log->debug(_p) << BOLD << "Appended char to figure " << figure->id << "." << OFF << endl;
      return;
    }
  }

  // There is no figure with a clip box equal to the current clip box yet. Create one.
  PdfFigure* figure = new PdfFigure();
  figure->id = string_utils::createRandomString(global_config::IDS_LENGTH, "figure-");
  figure->doc = _doc;
  figure->position->pageNum = _page->pageNum;
  figure->position->leftX = ch->position->leftX;
  figure->position->upperY = ch->position->upperY;
  figure->position->rightX = ch->position->rightX;
  figure->position->lowerY = ch->position->lowerY;
  figure->clipLeftX = clipLeftX;
  figure->clipUpperY = clipUpperY;
  figure->clipRightX = clipRightX;
  figure->clipLowerY = clipLowerY;
  figure->characters.push_back(ch);
  _page->figures.push_back(figure);

  _log->debug(_p) << BOLD << "Created new figure and appended the char to it." << OFF << endl;
  _log->debug(_p) << " └─ figure.id: " << figure->id << endl;
  _log->debug(_p) << " └─ figure.pageNum: " << figure->position->pageNum << endl;
  _log->debug(_p) << " └─ figure.leftX: " << figure->position->leftX << endl;
  _log->debug(_p) << " └─ figure.upperY: " << figure->position->upperY << endl;
  _log->debug(_p) << " └─ figure.rightX: " << figure->position->rightX << endl;
  _log->debug(_p) << " └─ figure.lowerY: " << figure->position->lowerY << endl;
  _log->debug(_p) << " └─ figure.clipLeftX: " << figure->clipLeftX << endl;
  _log->debug(_p) << " └─ figure.clipUpperY: " << figure->clipUpperY << endl;
  _log->debug(_p) << " └─ figure.clipRightX: " << figure->clipRightX << endl;
  _log->debug(_p) << " └─ figure.clipLowerY: " << figure->clipLowerY << endl;
}

// _________________________________________________________________________________________________
void TextOutputDev::stroke(GfxState* state) {
  assert(state);

  _log->debug(_p) << "=======================================" << endl;
  _log->debug(_p) << BOLD << "Event: STROKE PATH" << OFF << endl;

  // Get the current clip box (= a rectangle defining the visible part of the path; a path not
  // falling into this rectangle is not visible to the reader of the PDF).
  double clipLeftX, clipUpperY, clipRightX, clipLowerY;
  state->getClipBBox(&clipLeftX, &clipUpperY, &clipRightX, &clipLowerY);

  // Iterate through each subpath and each point, for compute the bounding box of the path.
  double x, y;
  double leftX = numeric_limits<double>::max();
  double upperY = numeric_limits<double>::max();
  double rightX = numeric_limits<double>::min();
  double lowerY = numeric_limits<double>::min();
  const GfxPath* path = state->getPath();
  for (int i = 0; i < path->getNumSubpaths(); i++) {
    const GfxSubpath* subpath = path->getSubpath(i);

    for (int j = 0; j < subpath->getNumPoints(); j++) {
      state->transform(subpath->getX(j), subpath->getY(j), &x, &y);
      // Ignore points that lies outside the clip box (since points outside the clip box are
      // not visible).
      // TODO(korzen): This is dangerous, since we may ignore a path that is actually visible, for
      // example, when the first endpoint of a line lies left to the clip box and the second
      // endpoint lies right to the clip box (and the connecting line goes straight through the
      // clip box).
      if (math_utils::equalOrSmaller(x, clipLeftX) || math_utils::equalOrSmaller(y, clipUpperY) ||
          math_utils::equalOrLarger(x, clipRightX) || math_utils::equalOrLarger(y, clipLowerY)) {
        continue;
      }

      leftX = max(min(leftX, x), clipLeftX);
      upperY = max(min(upperY, y), clipUpperY);
      rightX = min(max(rightX, x), clipRightX);
      lowerY = min(max(lowerY, y), clipLowerY);
    }
  }

  // Store the information about the path in form of a `PdfShape`.
  PdfShape* shape = new PdfShape();
  shape->id = string_utils::createRandomString(global_config::IDS_LENGTH, "shape-");
  shape->doc = _doc;
  shape->position->pageNum = _page->pageNum;
  shape->position->leftX = leftX;
  shape->position->upperY = upperY;
  shape->position->rightX = rightX;
  shape->position->lowerY = lowerY;
  shape->rank = _numElements++;

  _log->debug(_p) << " └─ shape.id: " << shape->id << endl;
  _log->debug(_p) << " └─ shape.pageNum: " << shape->position->pageNum << endl;
  _log->debug(_p) << " └─ shape.leftX: " << shape->position->leftX << endl;
  _log->debug(_p) << " └─ shape.upperY: " << shape->position->upperY << endl;
  _log->debug(_p) << " └─ shape.rightX: " << shape->position->rightX << endl;
  _log->debug(_p) << " └─ shape.lowerY: " << shape->position->lowerY << endl;
  _log->debug(_p) << " └─ shape.rank: " << shape->rank << endl;
  _log->debug(_p) << " └─ clipBox: leftX: " << clipLeftX << "; upperY: " << clipUpperY
      << "; rightX: " << clipRightX << "; lowerY: " << clipLowerY << endl;

  // ----------------------------------
  // Add the shape to the page or to a figure, depending on the current clip box. If the
  // current clip box is equal to the page's clip box, append the shape to `page->shapes`.
  // Otherwise, append the shape to `figure->shapes`, where `figure` is the `PdfFigure`
  // with a clip box equal to the current clip box. If no such figure exists yet, create it.
  // NOTE: Our assumption here is that each clip box, which is different to the page's clip box,
  // represents a separate figure and that all text falling into the same clip box belong to the
  // same figure. We create exactly one figure per clip box. This figure is created when the clip
  // box is "seen" for the first time.

  // Check if the clip box is equal to the page's clip box. If so, add the shape to the page.
  if (math_utils::equal(clipLeftX, _page->clipLeftX)
        && math_utils::equal(clipUpperY, _page->clipUpperY)
        && math_utils::equal(clipRightX, _page->clipRightX)
        && math_utils::equal(clipLowerY, _page->clipLowerY)) {
    _page->shapes.push_back(shape);
    _log->debug(_p) << BOLD << "Appended shape to page." << OFF << endl;
    return;
  }

  // Iterate through the figures to check if there is a figure with a clip box equal to the current
  // clip box. If so, append the shape to this figure.
  for (auto* figure : _page->figures) {
    if (math_utils::equal(clipLeftX, figure->clipLeftX)
          && math_utils::equal(clipUpperY, figure->clipUpperY)
          && math_utils::equal(clipRightX, figure->clipRightX)
          && math_utils::equal(clipLowerY, figure->clipLowerY)) {
      // Update the bounding box of the figure.
      figure->position->leftX = min(figure->position->leftX, shape->position->leftX);
      figure->position->upperY = min(figure->position->upperY, shape->position->upperY);
      figure->position->rightX = max(figure->position->rightX, shape->position->rightX);
      figure->position->lowerY = max(figure->position->lowerY, shape->position->lowerY);
      figure->shapes.push_back(shape);
      _log->debug(_p) << BOLD << "Appended shape to figure " << figure->id << "." << OFF << endl;
      return;
    }
  }

  // There is no figure with a clip box equal to the current clip box yet. Create one.
  PdfFigure* figure = new PdfFigure();
  figure->id = string_utils::createRandomString(global_config::IDS_LENGTH, "figure-");
  figure->doc = _doc;
  figure->position->pageNum = _page->pageNum;
  figure->position->leftX = shape->position->leftX;
  figure->position->upperY = shape->position->upperY;
  figure->position->rightX = shape->position->rightX;
  figure->position->lowerY = shape->position->lowerY;
  figure->clipLeftX = clipLeftX;
  figure->clipUpperY = clipUpperY;
  figure->clipRightX = clipRightX;
  figure->clipLowerY = clipLowerY;
  figure->shapes.push_back(shape);
  _page->figures.push_back(figure);

  _log->debug(_p) << BOLD << "Created new figure and appended the shape to it." << OFF << endl;
  _log->debug(_p) << " └─ figure.id: " << figure->id << endl;
  _log->debug(_p) << " └─ figure.pageNum: " << figure->position->pageNum << endl;
  _log->debug(_p) << " └─ figure.leftX: " << figure->position->leftX << endl;
  _log->debug(_p) << " └─ figure.upperY: " << figure->position->upperY << endl;
  _log->debug(_p) << " └─ figure.rightX: " << figure->position->rightX << endl;
  _log->debug(_p) << " └─ figure.lowerY: " << figure->position->lowerY << endl;
  _log->debug(_p) << " └─ figure.clipLeftX: " << figure->clipLeftX << endl;
  _log->debug(_p) << " └─ figure.clipUpperY: " << figure->clipUpperY << endl;
  _log->debug(_p) << " └─ figure.clipRightX: " << figure->clipRightX << endl;
  _log->debug(_p) << " └─ figure.clipLowerY: " << figure->clipLowerY << endl;
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
void TextOutputDev::drawMaskedImage(GfxState* state, Object* ref, Stream* str, int width,
    int height, GfxImageColorMap* colorMap, bool interpolate, Stream* maskStr, int maskWidth,
    int maskHeight, bool maskInvert, bool maskInterpolate) {
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
  assert(state);

  _log->debug(_p) << "=======================================" << endl;
  _log->debug(_p) << BOLD << "Event: DRAW GRAPHIC" << OFF << endl;

  // Get the current clip box (= a rectangle defining the visible part of the graphic; parts of the
  // graphic not falling into this rectangle is not visible to the reader of the PDF).
  double clipLeftX, clipUpperY, clipRightX, clipLowerY;
  state->getClipBBox(&clipLeftX, &clipUpperY, &clipRightX, &clipLowerY);

  // Compute the bounding box of the graphic from the ctm.
  const double* ctm = state->getCTM();
  double leftX = ctm[4];  // ctm[4] = translateX
  double upperY = ctm[5];  // ctm[5] = translateY
  double rightX = leftX + ctm[0];  // ctm[0] = scaleX
  double lowerY = upperY + ctm[3];  // ctm[3] = scaleY

  // Ignore the graphic if it lies outside the clip box (since graphics outside the clip box are
  // not visible). Example PDF where a graphic lies outside the clip box: 1001.5159
  if (math_utils::equalOrSmaller(leftX, clipLeftX)
      || math_utils::equalOrSmaller(upperY, clipUpperY)
      || math_utils::equalOrLarger(rightX, clipRightX)
      || math_utils::equalOrLarger(lowerY, clipLowerY)) {
    return;
  }

  // Handle each graphic as a `PdfGraphic`.
  PdfGraphic* graphic = new PdfGraphic();
  graphic->id = string_utils::createRandomString(global_config::IDS_LENGTH, "graphic-");
  graphic->doc = _doc;
  graphic->position->pageNum = _page->pageNum;
  graphic->position->leftX = max(min(leftX, rightX), clipLeftX);
  graphic->position->upperY = max(min(upperY, lowerY), clipUpperY);
  graphic->position->rightX = min(max(leftX, rightX), clipRightX);
  graphic->position->lowerY = min(max(upperY, lowerY), clipLowerY);
  graphic->rank = _numElements++;

  _log->debug(_p) << " └─ graphic.id: " << graphic->id << endl;
  _log->debug(_p) << " └─ graphic.pageNum: " << graphic->position->pageNum << endl;
  _log->debug(_p) << " └─ graphic.leftX: " << graphic->position->leftX << endl;
  _log->debug(_p) << " └─ graphic.upperY: " << graphic->position->upperY << endl;
  _log->debug(_p) << " └─ graphic.rightX: " << graphic->position->rightX << endl;
  _log->debug(_p) << " └─ graphic.lowerY: " << graphic->position->lowerY << endl;
  _log->debug(_p) << " └─ graphic.rank: " << graphic->rank << endl;
  _log->debug(_p) << " └─ clipBox: leftX: " << clipLeftX << "; upperY: " << clipUpperY
      << "; rightX: " << clipRightX << "; lowerY: " << clipLowerY << endl;

  // ----------------------------------
  // Add the graphic to the page or to a figure, depending on the current clip box. If the
  // current clip box is equal to the page's clip box, append the graphic to `page->graphics`.
  // Otherwise, append the graphic to `figure->graphics`, where `figure` is the `PdfFigure`
  // with a clip box equal to the current clip box. If no such figure exists yet, create it.
  // NOTE: Our assumption here is that each clip box, which is different to the page's clip box,
  // represents a separate figure and that all text falling into the same clip box belong to the
  // same figure. We create exactly one figure per clip box. This figure is created when the clip
  // box is "seen" for the first time.

  // Check if the clip box is equal to the page's clip box. If so, add the graphic to the page.
  if (math_utils::equal(clipLeftX, _page->clipLeftX)
        && math_utils::equal(clipUpperY, _page->clipUpperY)
        && math_utils::equal(clipRightX, _page->clipRightX)
        && math_utils::equal(clipLowerY, _page->clipLowerY)) {
    _page->graphics.push_back(graphic);
    _log->debug(_p) << BOLD << "Appended graphic to page." << OFF << endl;
    return;
  }

  // Iterate through the figures to check if there is a figure with a clip box equal to the current
  // clip box. If so, append the graphic to this figure.
  for (auto* figure : _page->figures) {
    if (math_utils::equal(clipLeftX, figure->clipLeftX)
          && math_utils::equal(clipUpperY, figure->clipUpperY)
          && math_utils::equal(clipRightX, figure->clipRightX)
          && math_utils::equal(clipLowerY, figure->clipLowerY)) {
      // Update the bounding box of the figure.
      figure->position->leftX = min(figure->position->leftX, graphic->position->leftX);
      figure->position->upperY = min(figure->position->upperY, graphic->position->upperY);
      figure->position->rightX = max(figure->position->rightX, graphic->position->rightX);
      figure->position->lowerY = max(figure->position->lowerY, graphic->position->lowerY);
      figure->graphics.push_back(graphic);
      _log->debug(_p) << BOLD << "Appended graphic to figure " << figure->id << "." << OFF << endl;
      return;
    }
  }

  // There is no figure with a clip box equal to the current clip box yet. Create one.
  PdfFigure* figure = new PdfFigure();
  figure->id = string_utils::createRandomString(global_config::IDS_LENGTH, "figure-");
  figure->doc = _doc;
  figure->position->pageNum = _page->pageNum;
  figure->position->leftX = graphic->position->leftX;
  figure->position->upperY = graphic->position->upperY;
  figure->position->rightX = graphic->position->rightX;
  figure->position->lowerY = graphic->position->lowerY;
  figure->clipLeftX = clipLeftX;
  figure->clipUpperY = clipUpperY;
  figure->clipRightX = clipRightX;
  figure->clipLowerY = clipLowerY;
  figure->graphics.push_back(graphic);
  _page->figures.push_back(figure);

  _log->debug(_p) << BOLD << "Created new figure and appended the graphic to it." << OFF << endl;
  _log->debug(_p) << " └─ figure.id: " << figure->id << endl;
  _log->debug(_p) << " └─ figure.pageNum: " << figure->position->pageNum << endl;
  _log->debug(_p) << " └─ figure.leftX: " << figure->position->leftX << endl;
  _log->debug(_p) << " └─ figure.upperY: " << figure->position->upperY << endl;
  _log->debug(_p) << " └─ figure.rightX: " << figure->position->rightX << endl;
  _log->debug(_p) << " └─ figure.lowerY: " << figure->position->lowerY << endl;
  _log->debug(_p) << " └─ figure.clipLeftX: " << figure->clipLeftX << endl;
  _log->debug(_p) << " └─ figure.clipUpperY: " << figure->clipUpperY << endl;
  _log->debug(_p) << " └─ figure.clipRightX: " << figure->clipRightX << endl;
  _log->debug(_p) << " └─ figure.clipLowerY: " << figure->clipLowerY << endl;
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
