/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // min, max

#include "./utils/MathUtils.h"

#include "./PdfDocument.h"
#include "./SubSuperScriptsDetector.h"

using global_config::COORDS_EQUAL_TOLERANCE;
using std::max;
using std::min;

// _________________________________________________________________________________________________
SubSuperScriptsDetector::SubSuperScriptsDetector(const PdfDocument* doc) {
  _config = new SubSuperScriptsDetectorConfig();
  _doc = doc;
}

// _________________________________________________________________________________________________
SubSuperScriptsDetector::~SubSuperScriptsDetector() {
  delete _config;
}

// _________________________________________________________________________________________________
void SubSuperScriptsDetector::process() const {
  assert(_doc);

  double fsTolerance = _config->getFontSizeTolerance(_doc);

  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      for (auto* line : segment->lines) {
        for (auto* word : line->words) {
          for (auto* character : word->characters) {
            // Consider a character to be superscripted, if its font size is smaller than the
            // most frequent font size (under consideration of the given tolerance) and its base
            // line is higher than the base line of the text line. Consider a character to be
            // subscripted if its base line is lower than the base line of the text line.
            if (math_utils::smaller(character->fontSize, _doc->mostFreqFontSize, fsTolerance)) {
              if (math_utils::smaller(character->base, line->base, COORDS_EQUAL_TOLERANCE)) {
                character->isSuperscript = true;
                continue;
              }

              if (math_utils::larger(character->base, line->base, COORDS_EQUAL_TOLERANCE)) {
                character->isSubscript = true;
                continue;
              }
            }

            // Compute the coordinates of the base bounding box of the line.
            line->baseBBoxLeftX = min(line->baseBBoxLeftX, character->position->leftX);
            line->baseBBoxUpperY = min(line->baseBBoxUpperY, character->position->upperY);
            line->baseBBoxRightX = max(line->baseBBoxRightX, character->position->rightX);
            line->baseBBoxLowerY = max(line->baseBBoxLowerY, character->position->lowerY);
          }
        }
      }
    }
  }
}
