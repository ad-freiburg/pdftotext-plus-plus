/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // std::min, std::max

#include "./utils/Log.h"
#include "./utils/MathUtils.h"
#include "./Config.h"
#include "./PdfDocument.h"
#include "./SubSuperScriptsDetection.h"

using std::endl;
using std::max;
using std::min;

using ppp::config::SubSuperScriptsDetectionConfig;
using ppp::utils::log::Logger;
using ppp::utils::log::BOLD;
using ppp::utils::log::OFF;
using ppp::utils::math::larger;
using ppp::utils::math::smaller;

// =================================================================================================

namespace ppp::modules {

// _________________________________________________________________________________________________
SubSuperScriptsDetection::SubSuperScriptsDetection(PdfDocument* doc,
    const SubSuperScriptsDetectionConfig& config) {
  _doc = doc;
  _config = config;
  _log = new Logger(config.logLevel, config.logPageFilter);
}

// _________________________________________________________________________________________________
SubSuperScriptsDetection::~SubSuperScriptsDetection() {
  delete _log;
}

// _________________________________________________________________________________________________
void SubSuperScriptsDetection::process() const {
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
            _log->debug(p) << " └─ tolerance font-size: " << _config.fsEqualTolerance << endl;
            _log->debug(p) << " └─ char.base: " << character->base << endl;
            _log->debug(p) << " └─ line.base: " << line->base << endl;
            _log->debug(p) << " └─ tolerance base-line: " << _config.baseEqualTolerance << endl;

            if (smaller(character->fontSize, _doc->mostFreqFontSize, _config.fsEqualTolerance)) {
              if (smaller(character->base, line->base, _config.baseEqualTolerance)) {
                _log->debug(p) << BOLD << " superscript (char.base < line.base)" << OFF << endl;
                character->isSuperscript = true;
                continue;
              }

              if (larger(character->base, line->base, _config.baseEqualTolerance)) {
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

}  // namespace ppp::modules
