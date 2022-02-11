/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <iostream>

#include "./WordsDehyphenator.h"

// _________________________________________________________________________________________________
WordsDehyphenator::WordsDehyphenator(PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
WordsDehyphenator::~WordsDehyphenator() = default;

// _________________________________________________________________________________________________
void WordsDehyphenator::dehyphenate() {
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
          std::string prevLineLastWordText = prevLineLastWord->text;

          if (prevLineLastWordText.length() > 1) {
            char prevLineLastChar = prevLineLastWordText[prevLineLastWordText.length() - 1];

            bool isHyphenated = prevLineLastChar == '-';  // TODO: Consider also other hyphens.

            PdfWord* currLineFirstWord = line->words[0];
            std::string currLineFirstWordText = currLineFirstWord->text;

            if (isHyphenated) {
              PdfWord* dehyphenatedWord = new PdfWord();
              dehyphenatedWord->text = prevLineLastWordText.substr(0, prevLineLastWordText.length() - 1);
              dehyphenatedWord->text += currLineFirstWordText;

              prevLineLastWord->isFirstPartOfHyphenatedWord = dehyphenatedWord;
              currLineFirstWord->isSecondPartOfHyphenatedWord = dehyphenatedWord;
            }
          }
        }

        prevLine = line;
      }
    }
  }
}


// _________________________________________________________________________________________________
PdfWord::PdfWord(const PdfWord& word) {
  text = word.text;
  fontSize = word.fontSize;
  fontName = word.fontName;
  wMode = word.wMode;
  rotation = word.rotation;
  glyphs = word.glyphs;
}
