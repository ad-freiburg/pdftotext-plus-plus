/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>
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
TextSerializer::TextSerializer(PdfDocument* doc, bool addControlCharacters,
    bool addSemanticRoles, bool excludeSubSuperscripts) {
  _doc = doc;
  _addControlCharacters = addControlCharacters;
  _addSemanticRoles = addSemanticRoles;
  _excludeSubSuperscripts = excludeSubSuperscripts;
}

// _________________________________________________________________________________________________
TextSerializer::~TextSerializer() = default;

// _________________________________________________________________________________________________
void TextSerializer::serialize(const string& targetFilePath) {
  if (targetFilePath.size() == 1 && targetFilePath[0] == '-') {
    serializeToStream(std::cout);
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

    serializeToStream(outFile);
    outFile.close();
  }
}

// _________________________________________________________________________________________________
void TextSerializer::serializeToStream(std::ostream& outStream) {
  assert(_doc);

  PdfTextBlock* prevBlock = nullptr;
  for (auto* page : _doc->pages) {
    for (auto* block : page->blocks) {
      PdfWord* prevWord = nullptr;

      if (prevBlock) {
        outStream << endl;
        outStream << endl;
      }

      // Prefix each block with its semantic role if the respective option is enabled.
      if (_addSemanticRoles) {
        string role = block->role;
        std::transform(role.begin(), role.end(), role.begin(), ::toupper);
        outStream << "[" << role << "] ";
      }

      // Prefix each emphasized block with "^A" (start of heading), if the resp. option is enabled.
      if (_addControlCharacters && block->isEmphasized) {
        outStream << string(1, 0x01);
      }

      for (auto* line : block->lines) {
        for (auto* word : line->words) {
          if (word->isSecondPartOfHyphenatedWord) {
            continue;
          }

          if (prevWord) {
            outStream << " ";
          }

          if (word->isFirstPartOfHyphenatedWord) {
            // TODO(korzen): Hyphenated words should be also processed character-wise.
            outStream << word->isFirstPartOfHyphenatedWord->text;
          } else {
            for (auto* ch : word->characters) {
              if (_excludeSubSuperscripts && (ch->isSubscript || ch->isSuperscript)) {
                continue;
              }
              outStream << ch->text;
            }
          }

          prevWord = word;
        }
      }
      prevBlock = block;
    }

    // Mark each page break with "^L" (form feed).
    if (_addControlCharacters) {
      outStream << endl << string(1, 0x0C);
    }
  }
  outStream << endl;
}
