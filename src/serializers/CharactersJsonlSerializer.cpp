/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cstdlib>  // system()
#include <fstream>
#include <iostream>
#include <string>

#include "./CharactersJsonlSerializer.h"
#include "../PdfDocument.h"
#include "../utils/Utils.h"

// _________________________________________________________________________________________________
CharactersJsonlSerializer::CharactersJsonlSerializer(PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
CharactersJsonlSerializer::~CharactersJsonlSerializer() = default;

// _________________________________________________________________________________________________
void CharactersJsonlSerializer::serialize(const std::string& targetFilePath) {
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
void CharactersJsonlSerializer::serializeToStream(std::ostream& outStream) {
  // Iterate through the glyphs of the document and add a line to the file for each.
  size_t rank = 0;
  for (const auto* page : _doc->pages) {
    for (const auto* block : page->blocks) {
      for (const auto* line : block->lines) {
        for (const auto* word : line->words) {
          for (const auto* g : word->glyphs) {
            if (g->isDiacriticMarkOfBaseGlyph) {
              continue;
            }

            const PdfFontInfo* fontInfo = _doc->fontInfos.at(g->fontName);
            std::string text = g->isBaseGlyphOfDiacriticMark ? g->textWithDiacriticMark : g->text;

            outStream << "{"
              << "\"id\": \"" << g->id << "\", "
              << "\"rank\": " << rank++ << ", "
              << "\"page\": " << g->pageNum << ", "
              << "\"minX\": " << g->minX << ", "
              << "\"minY\": " << g->minY << ", "
              << "\"maxX\": " << g->maxX << ", "
              << "\"maxY\": " << g->maxY << ", "
              << "\"font\": \"" << g->fontName << "\", "
              << "\"fontSize\": " << g->fontSize << ", "
              << "\"weight\": " << fontInfo->weight << ", "
              << "\"italic\": " << (fontInfo->isItalic ? "true" : "false") << ", "
              << "\"color\": [" << g->color[0] << "," << g->color[1] << "," << g->color[2] << "],"
              << "\"opacity\": " << g->opacity << ", "
              << "\"text\": \"" << escapeJson(text) << "\", "
              << "\"word\": \"" << word->id << "\", "
              << "\"origin\": \"pdftotext++\""
              << "}"
              << std::endl;
          }
        }
      }
    }
  }
}
