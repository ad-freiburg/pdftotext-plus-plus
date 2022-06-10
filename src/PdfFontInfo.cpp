/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include "./PdfFontInfo.h"

#include <fofi/FoFiType1C.h>  // convertToType1
#include <regex>

#include <climits>  // INT_MIN, INT_MAX
#include <iostream>  // std::stringstream
#include <sstream>  // std::stringstream
#include <unordered_map>  // std::stringstream

#include <poppler/GfxFont.h>
#include <poppler/GfxState.h>
#include <poppler/Object.h>
#include <poppler/XRef.h>

// _________________________________________________________________________________________________
PdfFontInfo::PdfFontInfo() = default;

// _________________________________________________________________________________________________
PdfFontInfo::~PdfFontInfo() = default;

// _________________________________________________________________________________________________
PdfFontInfo* PdfFontInfo::create(const GfxState* state, XRef* xref, bool parseEmbeddedFontFiles) {
  GfxFont* gfxFont = state->getFont();
  // Do nothing if the given graphics state does not provide a font.
  if (!gfxFont) {
    return nullptr;
  }

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

  PdfFontInfo* fontInfo = new PdfFontInfo();

  // Set the font name, for example: "LTSL+Nimbus12-Bold".
  fontInfo->fontName = fontName;

  // Compute and set the normalized font name (= the original font name translated to lower cases
  // and without the prefix ending with "+", for example: "nimbus12-bold").
  GooString gooFontNameLower(fontInfo->fontName);

  gooFontNameLower.lowerCase();
  std::string fontNameLower = gooFontNameLower.toStr();
  std::size_t plusPos = fontNameLower.find("+");
  fontInfo->normFontName = plusPos != std::string::npos ? fontNameLower.substr(plusPos + 1) :
      fontNameLower;

  // Compute and set the font base name (= the normalized font name without the suffix starting
  // with "-" and without digits).
  std::string baseName = fontInfo->normFontName;
  std::size_t endPos = std::min(baseName.find("-"), baseName.size());
  int pos = 0;
  for (size_t i = 0; i < endPos; i++) {
    if (!isdigit(baseName[i])) {
      baseName[pos] = baseName[i];
      pos++;
    }
  }
  fontInfo->fontBaseName = baseName.substr(0, pos);

  // Set the ascent and descent.
  fontInfo->ascent = 0.95;  // fontInfo->ascent = std::max(gfxFont->getAscent(), 0.95);
  fontInfo->descent = -0.35;  // fontInfo->descent = std::min(gfxFont->getDescent(), -0.35);

  int fontFlags = gfxFont->getFlags();
  fontInfo->isFixedWidth = fontFlags & fontFixedWidth;
  fontInfo->isSerif = fontFlags & fontSerif;
  fontInfo->isSymbolic = fontFlags & fontSymbolic;
  fontInfo->isType3 = gfxFont->getType() == fontType3;

  // Compute whether or not the font is an italic font. This is surprisingly difficult, for the
  // following reasons:
  // (1) The font may provide an "isItalic" flag, but this flag can be unset even if the font is
  //     actually an italic font.
  // (2) The font name may contain hints about the italicness of the font (e.g., when it contains
  //     the word "italic"), but this is not always the case.
  // (3) If the font is an embedded font, the embedded font file may provide a full name of the
  //     font, which can contain the word "Italic". This is checked in a later step, on reading
  //     the embedded font file.
  fontInfo->isItalic = fontFlags & fontItalic;
  fontInfo->isItalic |= fontNameLower.find("italic") != std::string::npos;

  // Compute the font weight. This is also surprisingly difficult, for the following reasons:
  // (1) The font may provide the weight explicitly, but this weight is often unset.
  // (2) The font may provide an "isBold" flag, but this flags is also often unset, even if the
  //     font is actually a bold font.
  // (3) The font name can contain hints about the boldness of a font (e.g., when it contains the
  //     substring "bold" or "black").
  // (4) The embedded font file can contain information about the font weight (i.e., in the /Weight
  //     attribute). It can also contain the full name of the font (which is an extended version of
  //     the above font name) which can contain the word "Bold". This is checked in a later step,
  //     on reading the embedded font file.
  fontInfo->weight = gfxFont->getWeight() > 0 ? gfxFont->getWeight() : fontInfo->weight;
  fontInfo->weight = (fontFlags & fontBold) ? 700 : fontInfo->weight;
  fontInfo->weight = fontNameLower.find("bold") != std::string::npos ? 700 : fontInfo->weight;
  fontInfo->weight = fontNameLower.find("black") != std::string::npos ? 800 : fontInfo->weight;

  // Set the font matrix. If the font is embedded, this value will be overwritten by the font
  // matrix stored in the embedded font file.
  const double* gfxFontMatrix = gfxFont->getFontMatrix();
  fontInfo->fontMatrix[0] = gfxFontMatrix[0];
  fontInfo->fontMatrix[1] = gfxFontMatrix[1];
  fontInfo->fontMatrix[2] = gfxFontMatrix[2];
  fontInfo->fontMatrix[3] = gfxFontMatrix[3];
  fontInfo->fontMatrix[4] = gfxFontMatrix[4];
  fontInfo->fontMatrix[5] = gfxFontMatrix[5];

  if (parseEmbeddedFontFiles) {
    // Check if the font is embedded. If so, read the embedded font file. It can contains further
    // information about the font that are not read by pdftotext by default, for example: the font
    // weight, the italicness, or the exact bounding boxes of the characters.
    GfxFontLoc* fontLoc = state->getFont()->locateFont(xref, nullptr);

    if (fontLoc != nullptr) {
      switch (fontLoc->locType) {
        case gfxFontLocEmbedded:
          switch (fontLoc->fontType) {
            case fontType1:
            case fontType1C:
              {
                Type1FontFileParser* parser = new Type1FontFileParser();
                parser->parse(fontLoc->embFontID, xref, fontInfo);
                delete parser;
              }
              break;
            case fontType1COT:
            case fontTrueType:
            case fontTrueTypeOT:
            case fontCIDType0C:
            case fontCIDType2:
            case fontCIDType2OT:
            case fontCIDType0COT:
            default:
              break;
          }
        case gfxFontLocExternal:
        case gfxFontLocResident:
        default:
          break;
      }
    }

    delete fontLoc;
  }

  return fontInfo;
}

// ================================================================================================

// _________________________________________________________________________________________________
void Type1FontFileParser::parse(const Ref& embFontId, XRef* xref, PdfFontInfo* fontInfo) {
  // std::cout << "===== Parsing font " << fontInfo->fontName << std::endl;

  // Fetch the stream object belonging to the embedded font file to read.
  Object refObj(embFontId);
  Object strObj = refObj.fetch(xref);

  // Do nothing if the object is not a stream.
  if (!strObj.isStream()) {
    // TODO(korzen): Print warning.
    return;
  }

  // Do nothing if the stream does not contain the font file stream dictionary.
  Dict* dict = strObj.streamGetDict();
  if (!dict) {
    // TODO(korzen): Print warning.
    return;
  }

  // Read the lengths of the different parts of the font file from the font file stream dictionary.
  Object length1Obj = dict->lookup("Length1");
  Object length2Obj = dict->lookup("Length2");
  if (!length1Obj.isInt() || !length2Obj.isInt()) {
    // TODO(korzen): Print warning.
    return;
  }
  // Read the length of the clear-text portion (in bytes).
  int length1 = length1Obj.getInt();
  strObj.streamReset();
  if (strObj.streamGetChar() == 0x80 && strObj.streamGetChar() == 1) {
    length1 = strObj.streamGetChar() | (strObj.streamGetChar() << 8)
        | (strObj.streamGetChar() << 16) | (strObj.streamGetChar() << 24);
  } else {
    strObj.streamReset();
  }

  // Read the length of the encrypted portion (in bytes).
  int length2 = length2Obj.getInt();

  if (length1 <= 0 || length2 <= 0) {
    // TODO(korzen): Print warning.
    return;
  }

  // Parse the clear-text portion.
  parseAsciiPart(&strObj, length1, fontInfo);

  // Parse the encrypted part.
  parseEncryptedPart(&strObj, length2, fontInfo);
}

// _________________________________________________________________________________________________
void Type1FontFileParser::parseAsciiPart(Object* strObj, int length, PdfFontInfo* fontInfo) {
  std::string asciiPartStr;
  int c;
  for (int i = 0; i < length && (c = strObj->streamGetChar()) != EOF; i++) {
    asciiPartStr.push_back(c);
  }

  // Parse the ASCII part line by line.
  std::istringstream ascii(asciiPartStr);
  std::string line;
  bool fontMatrixFound = false;
  bool italicAngleFound = false;
  bool weightFound = false;
  while (std::getline(ascii, line)) {
    // /FontMatrix [0.001 0 0 0.001 0 0 ]readonly def
    if (!fontMatrixFound) {
      size_t prefixPos = line.find("/FontMatrix");
      if (prefixPos != std::string::npos) {
        size_t lSquarePos = line.find("[", prefixPos + 11);
        if (lSquarePos != std::string::npos) {
          lSquarePos += 1;
          size_t rSquarePos = line.find("]", lSquarePos);
          if (rSquarePos != std::string::npos) {
            std::istringstream lineSS(line.substr(lSquarePos, rSquarePos - lSquarePos));
            double d;
            int k = 0;
            while (lineSS >> d) {
              fontInfo->fontMatrix[k++] = d;
            }
          }
        }
      }
    }

    // Find the "/ItalicAngle" entry of form: "/ItalicAngle 0 def". A value != 0 means that the
    // font is an italic font.
    if (!italicAngleFound) {
      size_t pos = line.find("/ItalicAngle");
      if (pos != std::string::npos) {
        std::istringstream lineSS(line);
        double d;
        lineSS >> d;
        fontInfo->isItalic |= (d != 0);
        italicAngleFound = true;
      }
    }

    if (!weightFound) {
      size_t startPos = line.find("/Weight (");
      if (startPos != std::string::npos) {
        startPos += 9;
        size_t endPos = line.find(")", startPos);
        if (endPos != std::string::npos) {
          std::string weight = line.substr(startPos, endPos - startPos);
          if (weight == "Regular") {
            fontInfo->weight = 400;
          } else if (weight == "Medium") {
            fontInfo->weight = 500;
          } else if (weight == "Bold") {
            fontInfo->weight = 700;
          }
          weightFound = true;
        }
      }
    }
  }
}

// _________________________________________________________________________________________________
void Type1FontFileParser::parseEncryptedPart(Object* strObj, int length,
    PdfFontInfo* fontInfo) {

  // Figure out if the encrypted part is in PFA format (ascii) or PFB format (binary) by inspecting
  // the first four characters.
  bool isPfb = false;
  int start[4];
  for (int i = 0; i < 4; i++) {
    start[i] = strObj->streamGetChar();
    if (start[i] == EOF) {
      // std::cout << "Unexpected end of file in embedded font stream" << std::endl;
      return;
    }
    if (!((start[i] >= '0' && start[i] <= '9') ||
          (start[i] >= 'A' && start[i] <= 'F') ||
          (start[i] >= 'a' && start[i] <= 'f'))) {
      isPfb = true;
    }
  }

  int numBytes = isPfb ? length : length / 2;
  char* bytes = new char[numBytes];
  int b = 0;
  int c;

  if (isPfb) {
    if (start[0] == 0x80 && start[1] == 2) {
      length = start[2] | (start[3] << 8) | (strObj->streamGetChar() << 16) |
          (strObj->streamGetChar() << 24);
    } else {
      for (int i = 0; i < 4; i++) {
        char c1 = _hexToChar[(start[i] >> 4) & 0x0f];
        char c2 = _hexToChar[start[i] & 0x0f];
        bytes[b] = 16 * _charToHex[c1] + _charToHex[c2];
        b++;
      }
    }

    for (int i = b; i < length; i++) {
      if ((c = strObj->streamGetChar()) == EOF) {
        break;
      }
      char c1 = _hexToChar[(c >> 4) & 0x0f];
      char c2 = _hexToChar[c & 0x0f];
      bytes[b] = 16 * _charToHex[c1] + _charToHex[c2];
      b++;
    }
  } else {
    for (int i = 0; i < 4; i += 2) {
      bytes[i] = 16 * _charToHex[start[i]] + _charToHex[start[i+1]];
      b++;
    }
    for (int i = 4; i < length; i += 2) {
      char c1;
      if ((c1 = strObj->streamGetChar()) == EOF) {
        break;
      }
      char c2;
      if ((c2 = strObj->streamGetChar()) == EOF) {
        break;
      }
      bytes[i] = 16 * _charToHex[c1] + _charToHex[c2];
      b++;
    }
  }

  // std::cout << "is PFB: " << isPfb << std::endl;

  std::string decrypted;
  decrypt(bytes, b, 55665, 4, &decrypted);
  delete[] bytes;

  // Parse the "/lenIV" part.
  int lenIV = 4;
  std::size_t lenIVStart = decrypted.find("/lenIV ");
  if (lenIVStart != std::string::npos) {
    lenIVStart += 7;
    std::size_t lenIVEnd = decrypted.find(" ", lenIVStart);
    if (lenIVEnd != std::string::npos) {
      lenIV = std::stoi(decrypted.substr(lenIVStart, lenIVEnd - lenIVStart));
    }
  }

  // Parse the "/Subrs" part.
  std::size_t subrsStart = decrypted.find("/Subrs");
  if (subrsStart == std::string::npos) {
    // TODO(korzen): Print warning.
    return;
  }
  subrsStart += 6;

  std::size_t subrsEnd = decrypted.find("ND", subrsStart);
  if (subrsEnd == std::string::npos) {
    // TODO(korzen): Print warning.
    return;
  }

  std::string subrs = decrypted.substr(subrsStart, subrsEnd - subrsStart);
  std::regex dupRegex("dup\\s(\\d+)\\s(\\d+)\\sRD\\s");
  auto dupBegin = std::sregex_iterator(subrs.begin(), subrs.end(), dupRegex);
  auto dupEnd = std::sregex_iterator();

  std::unordered_map<int, std::string> subrsMap;
  for (std::sregex_iterator i = dupBegin; i != dupEnd; i++) {
    std::smatch match = *i;
    int index = std::stoi(match.str(1));
    int nnumBytes = std::stoi(match.str(2));

    std::string dupstring = subrs.substr(match.position() + match.length(), nnumBytes);

    std::string decryptedDupString;
    decrypt(dupstring.c_str(), nnumBytes, 4330, lenIV, &decryptedDupString);

    subrsMap[index] = decryptedDupString;
  }

  std::size_t charStringsStart = decrypted.find("/CharStrings");
  if (charStringsStart == std::string::npos) {
    // TODO(korzen): Print warning.
    return;
  }
  charStringsStart += 12;

  std::size_t charStringsEnd = decrypted.find("end", charStringsStart);
  if (charStringsEnd == std::string::npos) {
    // TODO(korzen): Print warning.
    return;
  }

  std::string charstrings = decrypted.substr(charStringsStart);

  std::regex words_regex("\\/(\\S+)\\s+(\\d+)\\sRD\\s");
  auto words_begin = std::sregex_iterator(charstrings.begin(), charstrings.end(), words_regex);
  auto words_end = std::sregex_iterator();

  for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
    std::smatch match = *i;
    std::string charName = match.str(1);
    int nnumBytes = std::stoi(match.str(2));

    // std::cout << "* " << charName << std::endl;

    std::string charstring = charstrings.substr(match.position() + match.length(), nnumBytes);

    std::string decryptedCharString;
    decrypt(charstring.c_str(), nnumBytes, 4330, lenIV, &decryptedCharString);

    int leftX = std::numeric_limits<int>::max();
    int upperY = std::numeric_limits<int>::max();
    int rightX = std::numeric_limits<int>::min();
    int lowerY = std::numeric_limits<int>::min();
    int curX = 0;
    int curY = 0;
    std::vector<int> args;
    std::vector<int> interpreterStack;
    parseCharString(decryptedCharString, subrsMap, &curX, &curY, &leftX, &upperY, &rightX, &lowerY, &args,
        &interpreterStack);
    fontInfo->glyphBoundingBoxes[charName] = std::make_tuple(leftX, upperY, rightX, lowerY);
  }
}

// _________________________________________________________________________________________________
void Type1FontFileParser::parseCharString(const std::string& charString,
    const std::unordered_map<int, std::string>& subrs, int* curX, int* curY,
    int* leftX, int* upperY, int* rightX, int* lowerY,
    std::vector<int>* args, std::vector<int>* interpreterStack) const {

  // for (size_t t = 0; t < charString.size(); t++) {
  //   char byte = charString[t] & 0xff;
  //   std::cout << (int) byte << " ";
  // }
  // std::cout << std::endl;

  for (size_t t = 0; t < charString.size(); t++) {
    unsigned char byte = charString[t] & 0xff;

    if (byte < 32) {
      // std::cout << "CMD: " << (int) byte << " ARGS: ";
      // for (auto a : *args) {
      //   std::cout << ((int) a) << " ";
      // }
      // std::cout << std::endl;

      switch (byte) {
        case 1:  // y dy hstem
          if (args->size() >= 2) {
            //   int dy = args->back();
            //   args->pop_back();
            //   int y = args->back();
            //   args->pop_back();

            //   *upperY = std::min(*upperY, std::min(y, y + dy));
            //   *lowerY = std::max(*lowerY, std::max(y, y + dy));
            args->pop_back();
            args->pop_back();
          }
          break;
        case 3:  // x dx vstem
          if (args->size() >= 2) {
            //   int dx = args->back();
            //   args->pop_back();
            //   int x = args->back();
            //   args->pop_back();

            //   *leftX = std::min(*leftX, std::min(x, x + dx));
            //   *rightX = std::max(*rightX, std::max(x, x + dx));
            args->pop_back();
            args->pop_back();
          }
          break;
        case 4:  // dy vmoveto
          if (args->size() >= 1) {
            int dy = args->back();
            args->pop_back();

            *curY += dy;
            *upperY = std::min(*upperY, *curY);
            *lowerY = std::max(*lowerY, *curY);
          }
          break;
        case 5:  // dx dy rlineto
          if (args->size() >= 2) {
            int dy = args->back();
            args->pop_back();
            int dx = args->back();
            args->pop_back();

            *curX += dx;
            *curY += dy;
            *leftX = std::min(*leftX, *curX);
            *rightX = std::max(*rightX, *curX);
            *upperY = std::min(*upperY, *curY);
            *lowerY = std::max(*lowerY, *curY);
          }
          break;
        case 6:  // dx hlineto
          if (args->size() >= 1) {
            int dx = args->back();
            args->pop_back();

            *curX += dx;
            *leftX = std::min(*leftX, *curX);
            *rightX = std::max(*rightX, *curX);
          }
          break;
        case 7:  // dy vlineto
          if (args->size() >= 1) {
            int dy = args->back();
            args->pop_back();

            *curY += dy;
            *upperY = std::min(*upperY, *curY);
            *lowerY = std::max(*lowerY, *curY);
          }
          break;
        case 8:  // dx1 dy1 dx2 dy2 dx3 dy3 rrcurveto
          if (args->size() >= 6) {
            int dy3 = args->back();
            args->pop_back();
            int dx3 = args->back();
            args->pop_back();
            int dy2 = args->back();
            args->pop_back();
            int dx2 = args->back();
            args->pop_back();
            int dy1 = args->back();
            args->pop_back();
            int dx1 = args->back();
            args->pop_back();

            int x1 = *curX + dx1;
            int y1 = *curY + dy1;
            int x2 = x1 + dx2;
            int y2 = y1 + dy2;
            int x3 = x2 + dx3;
            int y3 = y2 + dy3;

            *curX = x3;
            *curY = y3;
            *leftX = std::min(*leftX, std::min(x1, std::min(x2, x3)));
            *rightX = std::max(*rightX, std::max(x1, std::max(x2, x3)));
            *upperY = std::min(*upperY, std::min(y1, std::min(y2, y3)));
            *lowerY = std::max(*lowerY, std::max(y1, std::max(y2, y3)));
          }
          break;
        case 10:  // subr# callsubr
          if (args->size() >= 1) {
            int index = args->back();
            args->pop_back();

            if (!subrs.count(index)) {
              break;
            }

            parseCharString(subrs.at(index), subrs, curX, curY, leftX, upperY, rightX, lowerY, args,
                interpreterStack);
          }
          break;
        case 12:  // escape
          {
            unsigned char byte2 = charString[++t] & 0xff;

            // std::cout << " Byte2: " << (int) byte2 << " ";

            switch (byte2) {
              case 0:  // dotsection
                break;
              case 1:  // x0 dx0 x1 dx1 x2 dx2 vstem3
                if (args->size() >= 6) {
                  args->pop_back();
                  args->pop_back();
                  args->pop_back();
                  args->pop_back();
                  args->pop_back();
                  args->pop_back();
                }
                break;
              case 2:  // y0 dy0 y1 dy1 y2 dy2 hstem3
                if (args->size() >= 6) {
                  args->pop_back();
                  args->pop_back();
                  args->pop_back();
                  args->pop_back();
                  args->pop_back();
                  args->pop_back();
                }
                break;
              case 6:  // asb adx ady bchar achar seac
                if (args->size() >= 5) {
                  args->pop_back();
                  args->pop_back();
                  args->pop_back();
                  args->pop_back();
                  args->pop_back();
                }
                break;
              case 7:  // sbx sby wx wy sbw
                if (args->size() >= 4) {
                  args->pop_back();
                  args->pop_back();
                  int sby = args->back();
                  args->pop_back();
                  int sbx = args->back();
                  args->pop_back();

                  *curX = sbx;
                  *curY = sby;
                  *leftX = std::min(*leftX, *curX);
                  *rightX = std::max(*rightX, *curX);
                  *upperY = std::min(*upperY, *curY);
                  *lowerY = std::max(*lowerY, *curY);
                }
                break;
              case 12:  // num1 num2 div
                if (args->size() >= 2) {
                  int num2 = args->back();
                  args->pop_back();
                  int num1 = args->back();
                  args->pop_back();

                  int result = num1 / num2;  // TODO(korzen): float

                  args->push_back(result);
                }
                break;
              case 16:  // arg1 ... argn n othersubr# callothersubr
                if (args->size() >= 2) {
                  int othersubr = args->back();
                  args->pop_back();
                  int numArgs = args->back();
                  args->pop_back();


                  interpreterStack->clear();
                  switch (othersubr) {
                    case 0:
                      {
                        int n1 = args->back();
                        args->pop_back();
                        interpreterStack->push_back(n1);

                        int n2 = args->back();
                        args->pop_back();
                        interpreterStack->push_back(n2);

                        args->pop_back();
                      }
                    case 1:
                      break;
                    case 3:
                      {
                        int n1 = args->back();
                        args->pop_back();
                        interpreterStack->push_back(n1);
                        break;
                      }
                    default:
                      {
                        for (int i = 0; i < numArgs; i++) {
                          int n1 = args->back();
                          args->pop_back();
                          interpreterStack->push_back(n1);
                        }
                        break;
                      }
                  }
                }
                break;
              case 17:  // pop
                {
                  if (interpreterStack->size() > 0) {
                    args->push_back(interpreterStack->back());
                    interpreterStack->pop_back();
                  }
                  break;
                }
              case 33:  // x y setcurrentpoint
                if (args->size() >= 2) {
                  int y = args->back();
                  args->pop_back();
                  int x = args->back();
                  args->pop_back();

                  *curX = x;
                  *curY = y;
                  *leftX = std::min(*leftX, *curX);
                  *rightX = std::max(*rightX, *curX);
                  *upperY = std::min(*upperY, *curY);
                  *lowerY = std::max(*lowerY, *curY);
                }
                break;
            }
          }
          break;
        case 13:  // sbx wx hsbw
          if (args->size() >= 2) {
            // int wx = args->back();
            args->pop_back();
            int sbx = args->back();
            args->pop_back();

            *curX = sbx;
            *curY = 0;
            *leftX = std::min(*leftX, *curX);
            *rightX = std::max(*rightX, *curX);
            *upperY = std::min(*upperY, *curY);
            *lowerY = std::max(*lowerY, *curY);
          }
          break;
        case 21:  // dx dy rmoveto
          if (args->size() >= 2) {
            int dy = args->back();
            args->pop_back();
            int dx = args->back();
            args->pop_back();

            *curX += dx;
            *curY += dy;
            *leftX = std::min(*leftX, *curX);
            *rightX = std::max(*rightX, *curX);
            *upperY = std::min(*upperY, *curY);
            *lowerY = std::max(*lowerY, *curY);
          }
          break;
        case 22:  // dx hmoveto
          if (args->size() >= 1) {
            int dx = args->back();
            args->pop_back();

            *curX += dx;
            *leftX = std::min(*leftX, *curX);
            *rightX = std::max(*rightX, *curX);
          }
          break;
        case 30:  // dy1 dx2 dy2 dx3 vhcurveto
          if (args->size() >= 4) {
            int dx3 = args->back();
            args->pop_back();
            int dy2 = args->back();
            args->pop_back();
            int dx2 = args->back();
            args->pop_back();
            int dy1 = args->back();
            args->pop_back();

            int x1 = *curX;
            int y1 = *curY + dy1;
            int x2 = x1 + dx2;
            int y2 = y1 + dy2;
            int x3 = x2 + dx3;
            int y3 = y2;

            *curX = x3;
            *curY = y3;
            *leftX = std::min(*leftX, std::min(x1, std::min(x2, x3)));
            *rightX = std::max(*rightX, std::max(x1, std::max(x2, x3)));
            *upperY = std::min(*upperY, std::min(y1, std::min(y2, y3)));
            *lowerY = std::max(*lowerY, std::max(y1, std::max(y2, y3)));
          }
          break;
        case 31:  // dx1 dx2 dy2 dy3 hvcurveto
          if (args->size() >= 4) {
            int dy3 = args->back();
            args->pop_back();
            int dy2 = args->back();
            args->pop_back();
            int dx2 = args->back();
            args->pop_back();
            int dx1 = args->back();
            args->pop_back();

            int x1 = *curX + dx1;
            int y1 = *curY;
            int x2 = x1 + dx2;
            int y2 = y1 + dy2;
            int x3 = x2;
            int y3 = y2 + dy3;

            *curX = x3;
            *curY = y3;
            *leftX = std::min(*leftX, std::min(x1, std::min(x2, x3)));
            *rightX = std::max(*rightX, std::max(x1, std::max(x2, x3)));
            *upperY = std::min(*upperY, std::min(y1, std::min(y2, y3)));
            *lowerY = std::max(*lowerY, std::max(y1, std::max(y2, y3)));
          }
          break;
        case 0:  // error
        case 9:  // closepath
        case 11:  // return
        case 14:  // endchar
          break;
      }
    } else {
      if (byte <= 246) {
        args->push_back(byte - 139);
      } else if (byte >= 247 && byte <= 250) {
        unsigned char bbyte = charString[++t] & 0xff;
        args->push_back((byte - 247) * 256 + bbyte + 108);
      } else if (byte >= 251 && byte <= 254) {
        unsigned char bbyte = charString[++t] & 0xff;
        args->push_back(-(byte - 251) * 256 - bbyte - 108);
      } else if (byte == 255) {
        unsigned char b1 = charString[++t] & 0xff;
        unsigned char b2 = charString[++t] & 0xff;
        unsigned char b3 = charString[++t] & 0xff;
        unsigned char b4 = charString[++t] & 0xff;
        args->push_back(b1 << 24 | b2 << 16 | b3 << 8 | b4);
      }
    }
  }
}


// _________________________________________________________________________________________________
void Type1FontFileParser::decrypt(const char* bytes, int numBytes, int r, int n,
    std::string* resultStr) const {
  int c1 = 52845;
  int c2 = 22719;

  for (int i = 0; i < numBytes; i++) {
    int cipher = bytes[i] & 0xff;
    int plain = cipher ^ (r >> 8);

    if (i >= n) {
      resultStr->push_back(static_cast<char>(plain) );
    }

    r = ((cipher + r) * c1 + c2) & ((1 << 16) - 1);
  }
}
