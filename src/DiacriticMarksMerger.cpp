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

  // Iterate through the characters of each page. For each character, check if it represents a diacritic
  // mark. If so, compute the horizontal overlap with the respective previous and next character.
  // Merge the diacritic mark with the character yielding the larger horizontal overlap. If the
  // horizontal overlap with both character is zero, do not merge the diacritic mark with any character.
  for (const auto* page : _doc->pages) {
    int p = page->pageNum;
    _log->debug(p) << "=======================================" << std::endl;
    _log->debug(p) << "\033[1mPROCESSING PAGE " << p << "\033[0m" << std::endl;
    _log->debug(p) << " └─ # characters: " << page->characters.size() << std::endl;

    for (size_t i = 0; i < page->characters.size(); i++) {
      PdfCharacter* prevChar = i > 0 ? page->characters[i - 1] : nullptr;
      PdfCharacter* currChar = page->characters[i];
      PdfCharacter* nextChar = i < page->characters.size() - 1 ? page->characters[i + 1] : nullptr;

      _log->debug(p) << "---------------------------------------" << std::endl;
      _log->debug(p) << "\033[1mchar: page:\033[0m " << currChar->position->pageNum
          << "; \033[1mleftX:\033[0m " << currChar->position->leftX
          << "; \033[1mupperY:\033[0m " << currChar->position->upperY
          << "; \033[1mrightX:\033[0m " << currChar->position->rightX
          << "; \033[1mlowerY:\033[0m " << currChar->position->lowerY
          << "; \033[1mtext:\033[0m \"" << currChar->text << "\"" << std::endl;

      // The character is a diacritic mark when its unicode falls into one of the categories:
      //   - "Spacing Modifier Letters" (that is: into the range 02B0 - 02FF),
      //   - "Combining Diacritic Marks" (that is: into the range 0300 - 036F)
      bool isDiacriticMark = false;
      unsigned int unicode = 0;
      if (currChar->unicodes.size() == 1) {
        unicode = currChar->unicodes[0];

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
      _log->debug(p) << " └─ char.isDiacriticMark: " << isDiacriticMark << std::endl;

      // Do nothing if the character does not represent a diacritic mark.
      if (!isDiacriticMark) {
        _log->debug(p) << "\033[1mSkipping char (no diacritic mark).\033[0m" << std::endl;
        continue;
      }

      // Compute the horizontal overlap with the previous and the next char.
      double prevOverlapX = 0;
      double nextOverlapX = 0;
      if (prevChar) {
        _log->debug(p) << "\033[1mprev char: page:\033[0m " << prevChar->position->pageNum
            << "; \033[1mleftX:\033[0m " << prevChar->position->leftX
            << "; \033[1mupperY:\033[0m " << prevChar->position->upperY
            << "; \033[1mrightX:\033[0m " << prevChar->position->rightX
            << "; \033[1mlowerY:\033[0m " << prevChar->position->lowerY
            << "; \033[1mtext:\033[0m \"" << prevChar->text << "\"" << std::endl;

        double prevMinMaxX = std::min(prevChar->position->rightX, currChar->position->rightX);
        double prevMaxMinX = std::max(prevChar->position->leftX, currChar->position->leftX);
        prevOverlapX = std::max(0.0, prevMinMaxX - prevMaxMinX);
      } else {
        _log->debug(p) << "\033[1mprev char: -" << std::endl;
      }

      if (nextChar) {
         _log->debug(p) << "\033[1mnext char: page:\033[0m " << nextChar->position->pageNum
            << "; \033[1mleftX:\033[0m " << nextChar->position->leftX
            << "; \033[1mupperY:\033[0m " << nextChar->position->upperY
            << "; \033[1mrightX:\033[0m " << nextChar->position->rightX
            << "; \033[1mlowerY:\033[0m " << nextChar->position->lowerY
            << "; \033[1mtext:\033[0m \"" << nextChar->text << "\"" << std::endl;

        double nextMinMaxX = std::min(nextChar->position->rightX, currChar->position->rightX);
        double nextMaxMinX = std::max(nextChar->position->leftX, currChar->position->leftX);
        nextOverlapX = std::max(0.0, nextMinMaxX - nextMaxMinX);
      } else {
        _log->debug(p) << "\033[1mnext char: -" << std::endl;
      }
      _log->debug(p) << " └─ x-overlap prev/current char: " << prevOverlapX << std::endl;
      _log->debug(p) << " └─ x-overlap current/next char: " << nextOverlapX << std::endl;

      // Skip the char if both overlaps are equal to zero.
      if (math_utils::equal(prevOverlapX, 0, 0.1) && math_utils::equal(nextOverlapX, 0, 0.1)) {
        _log->debug(p) << "\033[1mSkipping char (both overlaps == 0).\033[0m" << std::endl;
        continue;
      }

      // Merge the diacritic mark with the char yielding the larger overlap.
      PdfCharacter* mark = currChar;
      PdfCharacter* base;
      if (prevOverlapX > nextOverlapX) {
        _log->debug(p) << "\033[1mMerge with previous char.\033[0m" << std::endl;
        base = prevChar;
      } else {
        _log->debug(p) << "\033[1mMerge with next char.\033[0m" << std::endl;
        base = nextChar;
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

      mark->isDiacriticMarkOfBaseChar = base;
      base->isBaseCharOfDiacriticMark = mark;

      base->textWithDiacriticMark = reinterpret_cast<char*>(output);
      base->position->leftX = std::min(base->position->leftX, mark->position->leftX);
      base->position->upperY = std::min(base->position->upperY, mark->position->upperY);
      base->position->rightX = std::max(base->position->rightX, mark->position->rightX);
      base->position->lowerY = std::max(base->position->lowerY, mark->position->lowerY);
      _log->debug(p) << " └─ base.textWithDiacMark: " << base->textWithDiacriticMark << std::endl;
      _log->debug(p) << " └─ base.position: leftX: " << currChar->position->leftX
          << "; upperY: " << currChar->position->upperY
          << "; rightX: " << currChar->position->rightX
          << "; lowerY: " << currChar->position->lowerY << std::endl;
    }
  }
}
