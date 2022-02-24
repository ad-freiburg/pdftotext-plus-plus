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

// _________________________________________________________________________________________________
void CharactersJsonlSerializer::serializeToStream(std::ostream& outStream) {
  // Iterate through the glyphs of the document and add a line to the file for each.
  size_t rank = 0;
  for (const auto* page : _doc->pages) {
    for (const auto* block : page->blocks) {
      for (const auto* line : block->lines) {
        for (const auto* word : line->words) {
          for (const auto* glyph : word->glyphs) {
            outStream << "{"
              << "\"id\": \"" << glyph->id << "\", "
              << "\"rank\": " << rank++ << ", "
              << "\"page\": " << glyph->pageNum << ", "
              << "\"minX\": " << glyph->minX << ", "
              << "\"minY\": " << glyph->minY << ", "
              << "\"maxX\": " << glyph->maxX << ", "
              << "\"maxY\": " << glyph->maxY << ", "
              << "\"font\": \"" << glyph->fontName << "\", "
              << "\"fontSize\": " << glyph->fontSize << ", "
              << "\"text\": \"" << escapeJson(glyph->text) << "\", "
              << "\"word\": \"" << word->id << "\", "
              << "\"origin\": \"pdftotei\""
              << "}"
              << std::endl;
          }
        }
      }
    }
  }
}
