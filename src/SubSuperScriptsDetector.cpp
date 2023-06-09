/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // min, max

#include "./utils/Log.h"
#include "./utils/MathUtils.h"

#include "./PdfDocument.h"
#include "./SubSuperScriptsDetector.h"

using ppp::math_utils::larger;
using ppp::math_utils::smaller;
using std::endl;
using std::max;
using std::min;

namespace config = sub_super_scripts_detector::config;

// _________________________________________________________________________________________________
SubSuperScriptsDetector::SubSuperScriptsDetector(const PdfDocument* doc, LogLevel logLevel,
      int logPageFilter) {
  _log = new Logger(logLevel, logPageFilter);
  _doc = doc;
}

// _________________________________________________________________________________________________
SubSuperScriptsDetector::~SubSuperScriptsDetector() {
  delete _log;
}

// _________________________________________________________________________________________________
void SubSuperScriptsDetector::process() const {
  assert(_doc);

  _log->info() << "Detecting sub-/superscripts..." << endl;
  _log->debug() << "=======================================" << endl;
  _log->debug() << BOLD << "DEBUG MODE" << OFF << endl;

  for (auto* page : _doc->pages) {
    int p = page->pageNum;

    for (auto* segment : page->segments) {
      for (auto* line : segment->lines) {
        _log->debug(p) << "=======================================" << endl;
        _log->debug(p) << BOLD << "line: \"" << line->text << "\"" << OFF << endl;
        _log->debug(p) << "---------------------------------------" << endl;

        for (auto* word : line->words) {
          for (auto* character : word->characters) {
            // Consider a character to be superscripted, if its font size is smaller than the
            // most frequent font size (under consideration of the given tolerance) and its base
            // line is higher than the base line of the text line. Consider a character to be
            // subscripted if its base line is lower than the base line of the text line.
            _log->debug(p) << BOLD << "char: " << character->text << OFF << endl;
            _log->debug(p) << " └─ char.fontSize: " << character->fontSize << endl;
            _log->debug(p) << " └─ doc.mostFrequentFontSize: " << _doc->mostFreqFontSize << endl;
            _log->debug(p) << " └─ tolerance font-size: " << config::FSIZE_EQUAL_TOLERANCE << endl;
            _log->debug(p) << " └─ char.base: " << character->base << endl;
            _log->debug(p) << " └─ line.base: " << line->base << endl;
            _log->debug(p) << " └─ tolerance base-line: " << config::BASE_EQUAL_TOLERANCE << endl;

            if (smaller(character->fontSize, _doc->mostFreqFontSize,
                  config::FSIZE_EQUAL_TOLERANCE)) {
              if (smaller(character->base, line->base, config::BASE_EQUAL_TOLERANCE)) {
                _log->debug(p) << BOLD << " superscript (char.base < line.base)" << OFF << endl;
                character->isSuperscript = true;
                continue;
              }

              if (larger(character->base, line->base, config::BASE_EQUAL_TOLERANCE)) {
                _log->debug(p) << BOLD << " subscript (char.base > line.base)" << OFF << endl;
                character->isSubscript = true;
                continue;
              }
            }

            // Compute the coordinates of the base bounding box of the line.
            line->baseBBoxLeftX = min(line->baseBBoxLeftX, character->pos->leftX);
            line->baseBBoxUpperY = min(line->baseBBoxUpperY, character->pos->upperY);
            line->baseBBoxRightX = max(line->baseBBoxRightX, character->pos->rightX);
            line->baseBBoxLowerY = max(line->baseBBoxLowerY, character->pos->lowerY);
          }
        }
      }
    }

    _log->debug(p) << "=======================================" << endl;
  }
}
