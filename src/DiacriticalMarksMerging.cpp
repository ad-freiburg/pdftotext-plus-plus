/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <poppler/GlobalParams.h>
#include <utf8proc.h>

#include <cassert>  // assert
#include <vector>

#include "./DiacriticalMarksMerging.h"
#include "./PdfDocument.h"
#include "./utils/Log.h"
#include "./utils/MathUtils.h"
#include "./utils/PdfElementsUtils.h"

using std::endl;
using std::vector;

using ppp::config::DiacriticalMarksMergingConfig;
using ppp::types::PdfCharacter;
using ppp::types::PdfDocument;
using ppp::utils::elements::computeMaxXOverlapRatio;
using ppp::utils::log::BOLD;
using ppp::utils::log::OFF;
using ppp::utils::log::Logger;
using ppp::utils::math::equal;
using ppp::utils::math::larger;
using ppp::utils::math::maximum;
using ppp::utils::math::minimum;

// =================================================================================================

namespace ppp::modules {

// _________________________________________________________________________________________________
DiacriticalMarksMerging::DiacriticalMarksMerging(
    PdfDocument* doc,
    const DiacriticalMarksMergingConfig* config) {
  _doc = doc;
  _config = config;
  _log = new Logger(_config->logLevel, _config->logPageFilter);
}

// _________________________________________________________________________________________________
DiacriticalMarksMerging::~DiacriticalMarksMerging() {
  delete _log;
}

// _________________________________________________________________________________________________
void DiacriticalMarksMerging::process() const {
  assert(_doc);

  _log->info() << "Merging diacritical marks..." << endl;
  _log->debug() << "=======================================" << endl;
  _log->debug() << BOLD << "DEBUG MODE" << OFF << endl;

  // TODO(korzen): Try to avoid the need of Poppler's globalParams.
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

    for (size_t i = 0; i < page->characters.size(); i++) {
      PdfCharacter* prevChar = i > 0 ? page->characters[i - 1] : nullptr;
      PdfCharacter* currChar = page->characters[i];
      PdfCharacter* nextChar = i < page->characters.size() - 1 ? page->characters[i + 1] : nullptr;

      _log->debug(p) << "=======================================" << endl;
      _log->debug(p) << BOLD << "char: \"" << currChar->text << "\"" << OFF << endl;
      _log->debug(p) << " • char.leftX:  " << currChar->pos->leftX << endl;
      _log->debug(p) << " • char.upperY: " << currChar->pos->upperY << endl;
      _log->debug(p) << " • char.rightX: " << currChar->pos->rightX << endl;
      _log->debug(p) << " • char.lowerY: " << currChar->pos->lowerY << endl;
      if (currChar->pos->rotation != 0) {
        _log->debug(p) << " • char.rotation:  " << currChar->pos->rotation << endl;
        _log->debug(p) << " • char.rotLeftX:  " << currChar->pos->getRotLeftX() << endl;
        _log->debug(p) << " • char.rotUpperY: " << currChar->pos->getRotUpperY() << endl;
        _log->debug(p) << " • char.rotRightX: " << currChar->pos->getRotRightX() << endl;
        _log->debug(p) << " • char.rotLowerY: " << currChar->pos->getRotLowerY() << endl;
      }

      // Skip the character if it does not contain exactly one unicode.
      if (currChar->unicodes.size() != 1) {
         _log->debug(p) << BOLD << "Skipping character (more than one unicode)." << OFF << endl;
        continue;
      }

      // Get the unicode of the character. If it is contained in combininMap, replace the unicode
      // by its combining equivalent.
      unsigned int unicode = currChar->unicodes[0];
      if (_config->combiningMap.count(unicode) > 0) {
        unicode = _config->combiningMap.at(unicode);
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
      _log->debug(p) << " • char.isDiacriticMark: " << isDiacriticMark << endl;

      // Skip the character if it does not represent a diacritic mark.
      if (!isDiacriticMark) {
        _log->debug(p) << BOLD << "Skipping character (no diacritical mark)." << OFF << endl;
        continue;
      }

      // Compute the horizontal overlap with the previous character.
      double prevXOverlapRatio = 0;
      _log->debug(p) << "---------------------------------------" << endl;
      if (prevChar) {
        _log->debug(p) << BOLD << "prevChar: \"" << prevChar->text << "\"" << OFF << endl;
        _log->debug(p) << " • prevChar.leftX:  " << prevChar->pos->leftX << endl;
        _log->debug(p) << " • prevChar.upperY: " << prevChar->pos->upperY << endl;
        _log->debug(p) << " • prevChar.rightX: " << prevChar->pos->rightX << endl;
        _log->debug(p) << " • prevChar.lowerY: " << prevChar->pos->lowerY << endl;

        prevXOverlapRatio = computeMaxXOverlapRatio(prevChar, currChar);
      } else {
        _log->debug(p) << BOLD << "prevChar: -" << OFF << endl;
      }

      // Compute the horizontal overlap with the next character.
      double nextXOverlapRatio = 0;
      _log->debug(p) << "---------------------------------------" << endl;
      if (nextChar) {
        _log->debug(p) << BOLD << "nextChar: \"" << nextChar->text << "\"" << OFF << endl;
        _log->debug(p) << " • nextChar.leftX:  " << nextChar->pos->leftX << endl;
        _log->debug(p) << " • nextChar.upperY: " << nextChar->pos->upperY << endl;
        _log->debug(p) << " • nextChar.rightX: " << nextChar->pos->rightX << endl;
        _log->debug(p) << " • nextChar.lowerY: " << nextChar->pos->lowerY << endl;

        nextXOverlapRatio = computeMaxXOverlapRatio(currChar, nextChar);
      } else {
        _log->debug(p) << BOLD << "nextChar: -" << OFF << endl;
      }
      _log->debug(p) << "---------------------------------------" << endl;

      _log->debug(p) << "xOverlapRatio prev/current char: " << prevXOverlapRatio << endl;
      _log->debug(p) << "xOverlapRatio current/next char: " << nextXOverlapRatio << endl;

      // Skip the character if both overlap ratios are equal to zero.
      if (equal(prevXOverlapRatio, 0) &&
          equal(nextXOverlapRatio, 0)) {
        _log->debug(p) << BOLD << "Skipping char (both overlaps == 0)." << OFF << endl;
        continue;
      }

      // Consider the character that yields the larger overlap ratio to be the base character.
      PdfCharacter* mark = currChar;
      PdfCharacter* base;
      if (larger(prevXOverlapRatio, nextXOverlapRatio)) {
        _log->debug(p) << BOLD << "Merge diacritic with previous character." << OFF << endl;
        base = prevChar;
      } else {
        _log->debug(p) << BOLD << "Merge diacritic with next character." << OFF << endl;
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
      base->pos->leftX = minimum(base->pos->leftX, mark->pos->leftX);
      base->pos->upperY = minimum(base->pos->upperY, mark->pos->upperY);
      base->pos->rightX = maximum(base->pos->rightX, mark->pos->rightX);
      base->pos->lowerY = maximum(base->pos->lowerY, mark->pos->lowerY);

      _log->debug(p) << " • base.textWithDiacMark: " << base->textWithDiacriticMark << endl;
      _log->debug(p) << " • base.leftX: " << base->pos->leftX << endl;
      _log->debug(p) << " • base.upperY: " << base->pos->upperY << endl;
      _log->debug(p) << " • base.rightX: " << base->pos->rightX << endl;
      _log->debug(p) << " • base.lowerY: " << base->pos->lowerY << endl;
    }
    _log->debug() << "=======================================" << endl;
  }
}

}  // namespace ppp::modules
