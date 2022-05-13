/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cmath>
#include <iostream>
#include <vector>

#include "./utils/Utils.h"
#include "./PdfDocument.h"
#include "./SubSuperScriptsDetector.h"

// _________________________________________________________________________________________________
SubSuperScriptsDetector::SubSuperScriptsDetector(PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
SubSuperScriptsDetector::~SubSuperScriptsDetector() = default;

// _________________________________________________________________________________________________
void SubSuperScriptsDetector::detect() const {
  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      for (auto* line : segment->lines) {
        for (auto* word : line->words) {
          for (auto* glyph : word->glyphs) {
            // Identify superscripts.
            if (smaller(glyph->fontSize, _doc->mostFreqFontSize, 0.9)) {
              if (glyph->base < line->base) {
                glyph->isSuperscript = true;
                continue;
              }

              // Identify subscripts.
              if (glyph->base > line->base) {
                glyph->isSubscript = true;
                continue;
              }
            }

            // Compute the bounding box of the line, with sub- and superscripts ignored.
            line->baseBBoxLeftX = std::min(line->baseBBoxLeftX, glyph->position->leftX);
            line->baseBBoxUpperY = std::min(line->baseBBoxUpperY, glyph->position->upperY);
            line->baseBBoxRightX = std::max(line->baseBBoxRightX, glyph->position->rightX);
            line->baseBBoxLowerY = std::max(line->baseBBoxLowerY, glyph->position->lowerY);
          }
        }
      }
    }
  }
}