/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // transform
#include <fstream>  // ofstream
#include <iostream>
#include <string>

#include "../PdfDocument.h"

#include "./TextSerializer.h"

using std::cerr;
using std::endl;
using std::ofstream;
using std::string;

// _________________________________________________________________________________________________
TextSerializer::TextSerializer() : Serializer() {}

// _________________________________________________________________________________________________
TextSerializer::~TextSerializer() = default;

// _________________________________________________________________________________________________
void TextSerializer::serialize(PdfDocument* doc, const string& targetFilePath) {
  if (targetFilePath.size() == 1 && targetFilePath[0] == '-') {
    serializeToStream(doc, std::cout);
  } else {
    // Compute the path to the parent directory of the target file.
    string parentDirPath = ".";
    size_t posLastSlash = targetFilePath.find_last_of("/");
    if (posLastSlash != string::npos) {
      parentDirPath = targetFilePath.substr(0, posLastSlash);
    }

    // Try to create all intermediate directories if the parent directory does not exist.
    if (system(("mkdir -p " + parentDirPath).c_str())) {
      cerr << "Could not create directory '" << parentDirPath << "'." << endl;
      return;
    }

    ofstream outFile(targetFilePath);
    if (!outFile.is_open()) {
      cerr << "Could not open file '" << targetFilePath << "'." << endl;
      return;
    }

    serializeToStream(doc, outFile);
    outFile.close();
  }
}

// _________________________________________________________________________________________________
void TextSerializer::serializeToStream(PdfDocument* doc, std::ostream& outStream) {
  assert(doc);

  PdfTextBlock* prevBlock = nullptr;
  for (auto* page : doc->pages) {
    for (auto* block : page->blocks) {
      PdfWord* prevWord = nullptr;

      if (prevBlock) {
        outStream << endl;
        outStream << endl;
      }

      // Prefix each block with its semantic role, if requested by the user.
      if (_addSemanticRoles) {
        string role = block->role;
        std::transform(role.begin(), role.end(), role.begin(), ::toupper);
        outStream << "[" << role << "] ";
      }

      // Prefix each emphasized block with "^A" (start of heading), if requested by the user.
      if (_addControlCharacters && block->isEmphasized) {
        outStream << string(1, 0x01);
      }

      for (auto* line : block->lines) {
        for (auto* word : line->words) {
          // Ignore the second part of hyphenated words, their text is included in the text of the
          // first part of the hyphenated word.
          if (word->isSecondPartOfHyphenatedWord) {
            continue;
          }

          // Split the words by a whitespace.
          if (prevWord) {
            outStream << " ";
          }

          // Write the word character-wise. Exclude sub- and superscripts if requested by the user.
          // Ignore diacritic marks that were merged with their base character (their text is part
          // of the base character).
          if (word->isFirstPartOfHyphenatedWord) {
            // TODO(korzen): Hyphenated words should be also processed character-wise.
            outStream << word->isFirstPartOfHyphenatedWord->text;
          } else {
            for (auto* ch : word->characters) {
              if (_excludeSubSuperscripts && (ch->isSubscript || ch->isSuperscript)) {
                continue;
              }

              if (ch->isBaseCharOfDiacriticMark) {
                outStream << ch->textWithDiacriticMark;
              } else if (!ch->isDiacriticMarkOfBaseChar) {
                outStream << ch->text;
              }
            }
          }

          prevWord = word;
        }
      }
      prevBlock = block;
    }

    // Mark each page break with "^L" (form feed), if requested by the user.
    if (_addControlCharacters) {
      outStream << endl << string(1, 0x0C);
    }
  }
  outStream << endl;
}
