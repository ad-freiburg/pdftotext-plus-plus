/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // std::sort
#include <iostream>   // NOLINT(build/include)  (cpplint suggests to use
                      // "#include 'serializers/TextSerializer.h'", ignore this)
#include <filesystem>  // NOLINT(build/include_order)
#include <fstream>
#include <iostream>
#include <string>

#include <poppler/Annot.h>
#include <poppler/PDFDoc.h>
#include <poppler/PDFDocEncoding.h>

#include "../PdfDocument.h"
#include "../TextOutputDev.h"
#include "./TextSerializer.h"

namespace fs = std::filesystem;

// _________________________________________________________________________________________________
TextSerializer::TextSerializer(PdfDocument* doc, bool addControlCharacters,
    bool addSemanticRoles, bool excludePunctuationMarks, bool excludeSubSuperscripts) {
  _doc = doc;
  _addControlCharacters = addControlCharacters;
  _addSemanticRoles = addSemanticRoles;
  _excludePunctuationMarks = excludePunctuationMarks;
  _excludeSubSuperscripts = excludeSubSuperscripts;
}

// _________________________________________________________________________________________________
TextSerializer::~TextSerializer() = default;

// _________________________________________________________________________________________________
void TextSerializer::serialize(const std::string& targetFilePathStr) {
  if (targetFilePathStr.size() == 1 && targetFilePathStr[0] == '-') {
    serializeToStream(std::cout);
  } else {
    fs::path targetFilePath = targetFilePathStr;
    fs::path parentDirPath = targetFilePath.parent_path();

    // Try to create all intermediate directories if the parent directory does not exist.
    if (!parentDirPath.empty() && !fs::exists(parentDirPath)) {
      bool created = fs::create_directories(parentDirPath);
      if (!created) {
        std::cerr << "Could not create directory '" << parentDirPath << "'." << std::endl;
        return;
      }
    }

    std::ofstream outFile(targetFilePath);
    if (!outFile.is_open()) {
      std::cerr << "Could not open file '" << targetFilePathStr << "'." << std::endl;
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

      // Prefix each emphasized block with "^A" (start of heading).
      if (_addControlCharacters && block->isEmphasized) {
        outStream << std::string(1, 0x01);
      }

      // Prefix each emphasized block with "^A" (start of heading).
      if (_addSemanticRoles) {
        std::string role = block->role;
        std::transform(role.begin(), role.end(), role.begin(), ::toupper);
        outStream << "[" << role << "]";
      }

      for (auto* line : block->lines) {
        for (auto* word : line->words) {
          if (prevWord && !prevWord->isSecondPartOfHyphenatedWord) {
            outStream << " ";
          }

          // Check if there are symbols left to the words (punctuation marks, sub- & superscripts).
          std::vector<PdfWord*> leftWords;
          if (!_excludeSubSuperscripts) {
            if (word->leftSuperscript) { leftWords.push_back(word->leftSuperscript); }
            if (word->leftSubscript) { leftWords.push_back(word->leftSubscript); }
          }
          if (!_excludePunctuationMarks) {
            if (word->leftPunctuation) { leftWords.push_back(word->leftPunctuation); }
          }
          sort(leftWords.begin(), leftWords.end(), [](const PdfWord* w1, const PdfWord* w2) {
            return w1->minX < w2->minX;
          });
          for (const auto* leftWord : leftWords) {
            outStream << leftWord->text;
          }

          if (word->isFirstPartOfHyphenatedWord) {
            outStream << word->isFirstPartOfHyphenatedWord->text;
          } else if (word->isSecondPartOfHyphenatedWord) {
            // Do nothing.
          } else {
            outStream << word->text;
          }

          // Check if there are symbols right to the words (punctuation marks, sub- & superscripts).
          std::vector<PdfWord*> rightWords;
          if (!_excludeSubSuperscripts) {
            if (word->rightSuperscript) { rightWords.push_back(word->rightSuperscript); }
            if (word->rightSubscript) { rightWords.push_back(word->rightSubscript); }
          }
          if (!_excludePunctuationMarks) {
            if (word->rightPunctuation) { rightWords.push_back(word->rightPunctuation); }
          }
          sort(rightWords.begin(), rightWords.end(), [](const PdfWord* w1, const PdfWord* w2) {
            return w1->minX < w2->minX;
          });
          for (const auto* rightWord : rightWords) {
            outStream << rightWord->text;
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
}
