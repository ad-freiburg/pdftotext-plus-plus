/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <string>

#include "./PdfDocument.h"
#include "./WordsDehyphenator.h"

using std::string;

// _________________________________________________________________________________________________
WordsDehyphenator::WordsDehyphenator(const PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
WordsDehyphenator::~WordsDehyphenator() = default;

// _________________________________________________________________________________________________
void WordsDehyphenator::dehyphenate() const {
  assert(_doc);

  PdfTextLine* prevLine = nullptr;
  for (const auto* page : _doc->pages) {
    for (const auto* block : page->blocks) {
      for (auto* line : block->lines) {
        // Do nothing if the line do not contain words.
        if (line->words.empty()) {
          continue;
        }

        if (prevLine) {
          PdfWord* prevLineLastWord = prevLine->words[prevLine->words.size() - 1];
          string prevLineLastWordText = prevLineLastWord->text;

          if (prevLineLastWordText.length() > 1) {
            char prevLineLastChar = prevLineLastWordText[prevLineLastWordText.length() - 1];

            // TODO(korzen): Consider also other hyphens.
            bool isHyphenated = prevLineLastChar == '-';

            PdfWord* currLineFirstWord = line->words[0];
            string currLineFirstWordText = currLineFirstWord->text;

            if (isHyphenated) {
              PdfWord* mergedWord = new PdfWord();
              mergedWord->doc = _doc;
              mergedWord->text = prevLineLastWordText.substr(0, prevLineLastWordText.length() - 1);
              mergedWord->text += currLineFirstWordText;

              prevLineLastWord->isFirstPartOfHyphenatedWord = mergedWord;
              currLineFirstWord->isSecondPartOfHyphenatedWord = mergedWord;
            }
          }
        }

        prevLine = line;
      }
    }
  }
}
