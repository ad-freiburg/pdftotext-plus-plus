/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <poppler/GlobalParams.h>
#include <utf8proc.h>

#include <algorithm>  // min
#include <vector>

#include "./utils/Log.h"
#include "./utils/MathUtils.h"
#include "./utils/PdfElementsUtils.h"

#include "./DiacriticalMarksMerger.h"

using std::endl;
using std::max;
using std::min;
using std::vector;

// _________________________________________________________________________________________________
DiacriticalMarksMerger::DiacriticalMarksMerger(const PdfDocument* doc, bool debug,
      int debugPageFilter) {
  _log = new Logger(debug ? DEBUG : INFO, debugPageFilter);
  _doc = doc;
}

// _________________________________________________________________________________________________
DiacriticalMarksMerger::~DiacriticalMarksMerger() {
  delete _log;
}

// _________________________________________________________________________________________________
void DiacriticalMarksMerger::process() const {
  assert(_doc);

  _log->debug() << BOLD << "Diacritical Marks Merging - DEBUG MODE" << OFF << endl;

  const UnicodeMap* uMap = globalParams->getTextEncoding();
  if (!uMap) {
    return;
  }

  // Iterate through the characters of each page. For each character, check if it represents a
  // diacritical mark. If so, compute the horizontal overlap with the respective previous and next
  // character. Merge the diacritic mark with the character yielding the larger horizontal overlap.
  // If the horizontal overlap with both character is zero, do not merge the diacritic mark with
  // any character.
  for (const auto* page : _doc->pages) {
    int p = page->pageNum;
    _log->debug(p) << "=========================" << endl;
    _log->debug(p) << BOLD << "PROCESSING PAGE " << p << OFF << endl;
    _log->debug(p) << " └─ # characters: " << page->characters.size() << endl;

    for (size_t i = 0; i < page->characters.size(); i++) {
      PdfCharacter* prevChar = i > 0 ? page->characters[i - 1] : nullptr;
      PdfCharacter* currChar = page->characters[i];
      PdfCharacter* nextChar = i < page->characters.size() - 1 ? page->characters[i + 1] : nullptr;

      _log->debug(p) << "-------------------------" << endl;
      _log->debug(p) << BOLD << "Char: \"" << currChar->text << "\"" << OFF << endl;
      _log->debug(p) << " └─ leftX:  " << currChar->position->leftX << endl;
      _log->debug(p) << " └─ upperY: " << currChar->position->upperY << endl;
      _log->debug(p) << " └─ rightX: " << currChar->position->rightX << endl;
      _log->debug(p) << " └─ lowerY: " << currChar->position->lowerY << endl;

      // Skip the character if it does not contain exactly one unicode.
      if (currChar->unicodes.size() != 1) {
        continue;
      }

      // Get the unicode of the character. If it is contained in combininMap, replace the unicode
      // by its combining equivalent.
      unsigned int unicode = currChar->unicodes[0];
      if (combiningMap.count(unicode) > 0) {
        unicode = combiningMap.at(unicode);
      }

      // The character is a diacritic mark when its unicode falls into one of the categories:
      //   - "Spacing Modifier Letters" (that is: into the range 02B0 - 02FF),
      //   - "Combining Diacritic Marks" (that is: into the range 0300 - 036F)
      bool isDiacriticMark = false;
      if (unicode >= 0x02B0 && unicode <= 0x02FF) {
        isDiacriticMark = true;
      }
      if (unicode >= 0x0300 && unicode <= 0x036f) {
        isDiacriticMark = true;
      }
      _log->debug(p) << " └─ char.isDiacriticMark: " << isDiacriticMark << endl;

      // Skip the character if it does not represent a diacritic mark.
      if (!isDiacriticMark) {
        _log->debug(p) << BOLD << "Skipping character (no diacritical mark)." << OFF << endl;
        continue;
      }

      // Compute the horizontal overlap with the previous character.
      double prevXOverlapRatio = 0;
      if (prevChar) {
        _log->debug(p) << BOLD << "prevChar: \"" << prevChar->text << "\"" << OFF << endl;
        _log->debug(p) << " └─ prevChar.leftX:  " << prevChar->position->leftX << endl;
        _log->debug(p) << " └─ prevChar.upperY: " << prevChar->position->upperY << endl;
        _log->debug(p) << " └─ prevChar.rightX: " << prevChar->position->rightX << endl;
        _log->debug(p) << " └─ prevChar.lowerY: " << prevChar->position->lowerY << endl;

        prevXOverlapRatio = element_utils::computeMaxXOverlapRatio(prevChar, currChar);
      } else {
        _log->debug(p) << BOLD << "prevChar: -" << OFF << endl;
      }

      double nextXOverlapRatio = 0;
      if (nextChar) {
        _log->debug(p) << BOLD << "nextChar: \"" << nextChar->text << "\"" << OFF << endl;
        _log->debug(p) << " └─ nextChar.leftX:  " << nextChar->position->leftX << endl;
        _log->debug(p) << " └─ nextChar.upperY: " << nextChar->position->upperY << endl;
        _log->debug(p) << " └─ nextChar.rightX: " << nextChar->position->rightX << endl;
        _log->debug(p) << " └─ nextChar.lowerY: " << nextChar->position->lowerY << endl;

        nextXOverlapRatio = element_utils::computeMaxXOverlapRatio(currChar, nextChar);
      } else {
        _log->debug(p) << BOLD << "nextChar: -" << OFF << endl;
      }
      _log->debug(p) << " └─ xOverlapRatio prev/current char: " << prevXOverlapRatio << endl;
      _log->debug(p) << " └─ xOverlapRatio current/next char: " << nextXOverlapRatio << endl;

      // Skip the character if both overlap ratios are equal to zero.
      if (math_utils::equal(prevXOverlapRatio, 0) && math_utils::equal(nextXOverlapRatio, 0)) {
        _log->debug(p) << BOLD << "Skipping char (both overlaps == 0)." << OFF << endl;
        continue;
      }

      // Consider the character yielding the larger overlap ratio to be the base character.
      PdfCharacter* mark = currChar;
      PdfCharacter* base;
      if (prevXOverlapRatio > nextXOverlapRatio) {
        _log->debug(p) << BOLD << "Merge with previous character." << OFF << endl;
        base = prevChar;
      } else {
        _log->debug(p) << BOLD << "Merge with next character." << OFF << endl;
        base = nextChar;
      }
      mark->isDiacriticMarkOfBaseChar = base;
      base->isBaseCharOfDiacriticMark = mark;

      // Compute the text with the base character and the diacritic mark merged to a single
      // character, using the utf8proc library.
      vector<Unicode> unicodesMerged;
      for (const auto& u : base->unicodes) { unicodesMerged.push_back(u); }
      unicodesMerged.push_back(unicode);

      vector<uint8_t> bytesMerged;
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

      base->textWithDiacriticMark = reinterpret_cast<char*>(output);

      // Update the bounding box.
      base->position->leftX = min(base->position->leftX, mark->position->leftX);
      base->position->upperY = min(base->position->upperY, mark->position->upperY);
      base->position->rightX = max(base->position->rightX, mark->position->rightX);
      base->position->lowerY = max(base->position->lowerY, mark->position->lowerY);

      _log->debug(p) << " └─ base.textWithDiacMark: " << base->textWithDiacriticMark << endl;
      _log->debug(p) << " └─ base.leftX: " << base->position->leftX << endl;
      _log->debug(p) << " └─ base.upperY: " << base->position->upperY << endl;
      _log->debug(p) << " └─ base.rightX: " << base->position->rightX << endl;
      _log->debug(p) << " └─ base.lowerY: " << base->position->lowerY << endl;
    }
  }
}
