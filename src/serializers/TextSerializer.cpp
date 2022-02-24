/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // std::sort
#include <cstdlib>  // system()
#include <iostream>   // NOLINT(build/include)  (cpplint suggests to use
                      // "#include 'serializers/TextSerializer.h'", ignore this)
#include <fstream>
#include <iostream>
#include <string>

#include <poppler/Annot.h>
#include <poppler/PDFDoc.h>
#include <poppler/PDFDocEncoding.h>

#include "../PdfDocument.h"
#include "../TextOutputDev.h"
#include "./TextSerializer.h"

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
void TextSerializer::serialize(const std::string& targetFilePath) {
  if (targetFilePath.size() == 1 && targetFilePath[0] == '-') {
    serializeToStream(std::cout);
  } else {
    // Compute the path to the parent directory of the target file.
    std::string parentDirPath = ".";
    size_t posLastSlash = targetFilePath.find_last_of("/");
    if (posLastSlash != std::string::npos) {
      parentDirPath = targetFilePath.substr(0, posLastSlash);
    }

    // Try to create all intermediate directories if the parent directory does not exist.
    if (system(("mkdir -p " + parentDirPath).c_str())) {
      std::cerr << "Could not create directory '" << parentDirPath << "'." << std::endl;
      return;
    }

    std::ofstream outFile(targetFilePath);
    if (!outFile.is_open()) {
      std::cerr << "Could not open file '" << targetFilePath << "'." << std::endl;
      return;
    }

    serializeToStream(outFile);
    outFile.close();
  }
}

// _________________________________________________________________________________________________
void TextSerializer::serializeToStream(std::ostream& outStream) {
  PdfTextBlock* prevBlock = nullptr;
  for (auto* page : _doc->pages) {
    for (auto* block : page->blocks) {
      PdfWord* prevWord = nullptr;

      if (prevBlock) {
        outStream << std::endl;
        outStream << std::endl;
      }

      // Prefix each block with its semantic role if the respective option is enabled.
      if (_addSemanticRoles) {
        std::string role = block->role;
        std::transform(role.begin(), role.end(), role.begin(), ::toupper);
        outStream << "[" << role << "] ";
      }

      // Prefix each emphasized block with "^A" (start of heading), if the resp. option is enabled.
      if (_addControlCharacters && block->isEmphasized) {
        outStream << std::string(1, 0x01);
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
            // TODO: Hyphenated words should be also processed glyph-wise.
            outStream << word->isFirstPartOfHyphenatedWord->text;
          } else {
            for (auto* glyph : word->glyphs) {
              if (_excludeSubSuperscripts && (glyph->isSubscript || glyph->isSuperscript)) {
                continue;
              }
              outStream << glyph->text;
            }
          }

          prevWord = word;
        }
      }
      prevBlock = block;
    }

    // Mark each page break with "^L" (form feed).
    if (_addControlCharacters) {
      outStream << std::endl << std::string(1, 0x0C);
    }
  }
  outStream << std::endl;
}
