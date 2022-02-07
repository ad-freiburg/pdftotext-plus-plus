/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include "./DiacriticMarksCombiner.h"

#include <utf8proc.h>

#include <algorithm>
#include <iostream>

#include <poppler/GlobalParams.h>


// _________________________________________________________________________________________________
DiacriticMarksCombiner::DiacriticMarksCombiner(PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
DiacriticMarksCombiner::~DiacriticMarksCombiner() = default;

// _________________________________________________________________________________________________
void DiacriticMarksCombiner::combine() {
  const UnicodeMap* uMap = globalParams->getTextEncoding();
  if (!uMap) {
    return;
  }

  for (const auto* page : _doc->pages) {
    for (size_t i = 0; i < page->glyphs.size(); i++) {
      PdfGlyph* prevGlyph = i > 0 ? page->glyphs[i - 1] : nullptr;
      PdfGlyph* currGlyph = page->glyphs[i];
      PdfGlyph* nextGlyph = i < page->glyphs.size() - 1 ? page->glyphs[i + 1] : nullptr;

      // Check if the current glyph represents a diacritic mark. This is the case when the glyph
      // consists of a single unicode that falls into one of the following categories:
      //  - "Spacing Modifier Letters" (that is: into the range 02B0 - 02FF),
      //  - "Combining Diacritical Marks" (that is: into the range 0300 - 036F).
      bool isDiacriticMark = false;
      if (currGlyph->unicodes.size() == 1) {
        unsigned int unicode = currGlyph->unicodes[0];

        if (combiningMap.count(unicode)) {
          unicode = combiningMap.at(unicode);
        }

        if (unicode >= 0x02B0 && unicode <= 0x02FF) {
          isDiacriticMark = true;
        }
        if (unicode >= 0x0300 && unicode <= 0x036f) {
          isDiacriticMark = true;
        }

        if (!isDiacriticMark) {
          continue;
        }

        // Merge the diacritic mark either with the previous glyph or the next glyph, depending on
        // with which of the two glyphs the diacritic mark has the largest horizontal overlap.
        double prevOverlapX = 0;
        if (prevGlyph) {
          double prevMinMaxX = std::min(prevGlyph->maxX, currGlyph->maxX);
          double prevMaxMinX = std::max(prevGlyph->minX, currGlyph->minX);
          prevOverlapX = std::max(0.0, prevMinMaxX - prevMaxMinX);
        }

        double nextOverlapX = 0;
        if (nextGlyph) {
          double nextMinMaxX = std::min(nextGlyph->maxX, currGlyph->maxX);
          double nextMaxMinX = std::max(nextGlyph->minX, currGlyph->minX);
          nextOverlapX = std::max(0.0, nextMinMaxX - nextMaxMinX);
        }

        std::vector<Unicode> unicodesMerged;
        PdfGlyph* diacriticMark = currGlyph;
        PdfGlyph* baseGlyph = nullptr;
        if (prevOverlapX > 0 && prevOverlapX > nextOverlapX) {
          baseGlyph = prevGlyph;
          for (const auto& u : baseGlyph->unicodes) { unicodesMerged.push_back(u); }
          unicodesMerged.push_back(unicode);
        } else if (nextOverlapX > 0 && nextOverlapX > prevOverlapX) {
          baseGlyph = nextGlyph;
          for (const auto& u : baseGlyph->unicodes) { unicodesMerged.push_back(u); }
          unicodesMerged.push_back(unicode);
        }

        if (!baseGlyph) {
          continue;
        }

        std::vector<uint8_t> bytesMerged;
        char buf[8];
        for (const auto& u : unicodesMerged) {
          int len = uMap->mapUnicode(u, buf, sizeof(buf));
          for (int j = 0; j < len; j++) {
            bytesMerged.push_back(buf[j] & 0xff);
          }
        }
        uint8_t* output;
        int options = UTF8PROC_STABLE | UTF8PROC_COMPOSE;
        utf8proc_map(&bytesMerged[0], bytesMerged.size(), &output, (utf8proc_option_t) options);

        diacriticMark->isDiacriticMarkOfBaseGlyph = baseGlyph;
        baseGlyph->isBaseGlyphOfDiacriticMark = diacriticMark;
        baseGlyph->textWithDiacriticMark = reinterpret_cast<char*>(output);

        baseGlyph->minX = std::min(baseGlyph->minX, diacriticMark->minX);
        baseGlyph->minY = std::min(baseGlyph->minY, diacriticMark->minY);
        baseGlyph->maxX = std::max(baseGlyph->maxX, diacriticMark->maxX);
        baseGlyph->maxY = std::max(baseGlyph->maxY, diacriticMark->maxY);
      }
    }
  }
}

// // _____________________________________________________________________________________________
// std::vector<uint8_t> DiacriticMarksCombiner::unicodesToBytes(
  // const std::vector<Unicode> unicodes) {
//   std::vector<uint8_t> bytes(8);

//   // for (const auto& unicode : unicodes) {
//   //   256 65536 16777216
//   // }

//   return bytes;
// }
