/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <utf8proc.h>
#include <algorithm>
#include <iostream>

#include <poppler/GlobalParams.h>

#include "./utils/Log.h"
#include "./utils/MathUtils.h"
#include "./DiacriticMarksMerger.h"

// _________________________________________________________________________________________________
DiacriticMarksMerger::DiacriticMarksMerger(PdfDocument* doc, bool debug, int debugPageFilter) {
  _doc = doc;
  _log = new Logger(debug ? DEBUG : INFO, debugPageFilter);

  _log->debug() << "=======================================" << std::endl;
  _log->debug() << "\033[1mDEBUG MODE | Diacritic Marks Merging\033[0m" << std::endl;
  _log->debug() << " └─ debug page filter: " << debugPageFilter << std::endl;
}

// _________________________________________________________________________________________________
DiacriticMarksMerger::~DiacriticMarksMerger() {
  delete _log;
};

// _________________________________________________________________________________________________
void DiacriticMarksMerger::merge() {
  const UnicodeMap* uMap = globalParams->getTextEncoding();
  if (!uMap) {
    return;
  }

  // Iterate through the glyphs of each page. For each glyph, check if it represents a diacritic
  // mark. If so, compute the horizontal overlap with the respective previous and next glyph.
  // Merge the diacritic mark with the glyph yielding the larger horizontal overlap. If the
  // horizontal overlap with both glyphs is zero, do not merge the diacritic mark with any glyph.
  for (const auto* page : _doc->pages) {
    int p = page->pageNum;
    _log->debug(p) << "=======================================" << std::endl;
    _log->debug(p) << "\033[1mPROCESSING PAGE " << p << "\033[0m" << std::endl;
    _log->debug(p) << " └─ # glyphs: " << page->glyphs.size() << std::endl;

    for (size_t i = 0; i < page->glyphs.size(); i++) {
      PdfGlyph* prevGlyph = i > 0 ? page->glyphs[i - 1] : nullptr;
      PdfGlyph* currGlyph = page->glyphs[i];
      PdfGlyph* nextGlyph = i < page->glyphs.size() - 1 ? page->glyphs[i + 1] : nullptr;

      _log->debug(p) << "---------------------------------------" << std::endl;
      _log->debug(p) << "\033[1mglyph: page:\033[0m " << currGlyph->position->pageNum
          << "; \033[1mleftX:\033[0m " << currGlyph->position->leftX
          << "; \033[1mupperY:\033[0m " << currGlyph->position->upperY
          << "; \033[1mrightX:\033[0m " << currGlyph->position->rightX
          << "; \033[1mlowerY:\033[0m " << currGlyph->position->lowerY
          << "; \033[1mtext:\033[0m \"" << currGlyph->text << "\"" << std::endl;

      // The glyph is a diacritic mark when its unicode falls into one of the categories:
      //   - "Spacing Modifier Letters" (that is: into the range 02B0 - 02FF),
      //   - "Combining Diacritic Marks" (that is: into the range 0300 - 036F)
      bool isDiacriticMark = false;
      unsigned int unicode = 0;
      if (currGlyph->unicodes.size() == 1) {
        unicode = currGlyph->unicodes[0];

        // Map the diacritic mark to its combining equivalent.
        if (combiningMap.count(unicode) > 0) {
          unicode = combiningMap.at(unicode);
        }

        if (unicode >= 0x02B0 && unicode <= 0x02FF) {
          isDiacriticMark = true;
        }
        if (unicode >= 0x0300 && unicode <= 0x036f) {
          isDiacriticMark = true;
        }
      }
      _log->debug(p) << " └─ glyph.isDiacriticMark: " << isDiacriticMark << std::endl;

      // Do nothing if the glyph does not represent a diacritic mark.
      if (!isDiacriticMark) {
        _log->debug(p) << "\033[1mSkipping glyph (no diacritic mark).\033[0m" << std::endl;
        continue;
      }

      // Compute the horizontal overlap with the previous and the next glyph.
      double prevOverlapX = 0;
      double nextOverlapX = 0;
      if (prevGlyph) {
        _log->debug(p) << "\033[1mprev glyph: page:\033[0m " << prevGlyph->position->pageNum
            << "; \033[1mleftX:\033[0m " << prevGlyph->position->leftX
            << "; \033[1mupperY:\033[0m " << prevGlyph->position->upperY
            << "; \033[1mrightX:\033[0m " << prevGlyph->position->rightX
            << "; \033[1mlowerY:\033[0m " << prevGlyph->position->lowerY
            << "; \033[1mtext:\033[0m \"" << prevGlyph->text << "\"" << std::endl;

        double prevMinMaxX = std::min(prevGlyph->position->rightX, currGlyph->position->rightX);
        double prevMaxMinX = std::max(prevGlyph->position->leftX, currGlyph->position->leftX);
        prevOverlapX = std::max(0.0, prevMinMaxX - prevMaxMinX);
      } else {
        _log->debug(p) << "\033[1mprev glyph: -" << std::endl;
      }

      if (nextGlyph) {
         _log->debug(p) << "\033[1mnext glyph: page:\033[0m " << nextGlyph->position->pageNum
            << "; \033[1mleftX:\033[0m " << nextGlyph->position->leftX
            << "; \033[1mupperY:\033[0m " << nextGlyph->position->upperY
            << "; \033[1mrightX:\033[0m " << nextGlyph->position->rightX
            << "; \033[1mlowerY:\033[0m " << nextGlyph->position->lowerY
            << "; \033[1mtext:\033[0m \"" << nextGlyph->text << "\"" << std::endl;

        double nextMinMaxX = std::min(nextGlyph->position->rightX, currGlyph->position->rightX);
        double nextMaxMinX = std::max(nextGlyph->position->leftX, currGlyph->position->leftX);
        nextOverlapX = std::max(0.0, nextMinMaxX - nextMaxMinX);
      } else {
        _log->debug(p) << "\033[1mnext glyph: -" << std::endl;
      }
      _log->debug(p) << " └─ x-overlap prev/current glyph: " << prevOverlapX << std::endl;
      _log->debug(p) << " └─ x-overlap current/next glyph: " << nextOverlapX << std::endl;

      // Skip the glyph if both overlaps are equal to zero.
      if (math_utils::equal(prevOverlapX, 0, 0.1) && math_utils::equal(nextOverlapX, 0, 0.1)) {
        _log->debug(p) << "\033[1mSkipping glyph (both overlaps == 0).\033[0m" << std::endl;
        continue;
      }

      // Merge the diacritic mark with the glyph yielding the larger overlap.
      PdfGlyph* mark = currGlyph;
      PdfGlyph* base;
      if (prevOverlapX > nextOverlapX) {
        _log->debug(p) << "\033[1mMerge with previous glyph.\033[0m" << std::endl;
        base = prevGlyph;
      } else {
        _log->debug(p) << "\033[1mMerge with next glyph.\033[0m" << std::endl;
        base = nextGlyph;
      }


      std::vector<Unicode> unicodesMerged;
      for (const auto& u : base->unicodes) { unicodesMerged.push_back(u); }
      unicodesMerged.push_back(unicode);

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

      mark->isDiacriticMarkOfBaseGlyph = base;
      base->isBaseGlyphOfDiacriticMark = mark;

      base->textWithDiacriticMark = reinterpret_cast<char*>(output);
      base->position->leftX = std::min(base->position->leftX, mark->position->leftX);
      base->position->upperY = std::min(base->position->upperY, mark->position->upperY);
      base->position->rightX = std::max(base->position->rightX, mark->position->rightX);
      base->position->lowerY = std::max(base->position->lowerY, mark->position->lowerY);
      _log->debug(p) << " └─ base.textWithDiacMark: " << base->textWithDiacriticMark << std::endl;
      _log->debug(p) << " └─ base.position: leftX: " << currGlyph->position->leftX
          << "; upperY: " << currGlyph->position->upperY
          << "; rightX: " << currGlyph->position->rightX
          << "; lowerY: " << currGlyph->position->lowerY << std::endl;
    }
  }
}
