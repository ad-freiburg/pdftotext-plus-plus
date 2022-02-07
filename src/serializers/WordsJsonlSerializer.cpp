/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <filesystem>   // NOLINT(build/include)  (cpplint suggests to use
                        // "#include 'serializers/WordsJsonlSerializer.h'", ignore this)
#include <fstream>
#include <iostream>
#include <string>

#include "../PdfDocument.h"
#include "../utils/Utils.h"
#include "./WordsJsonlSerializer.h"

namespace fs = std::filesystem;

// _________________________________________________________________________________________________
WordsJsonlSerializer::WordsJsonlSerializer(PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
WordsJsonlSerializer::~WordsJsonlSerializer() = default;

// _________________________________________________________________________________________________
void WordsJsonlSerializer::serialize(const std::string& targetFilePathStr) {
  fs::path targetFilePath = targetFilePathStr;
  fs::path parentDirPath = targetFilePath.parent_path();

  // Try to create all intermediate directories if the parent directory does not exist.
  if (!fs::exists(parentDirPath)) {
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

// _________________________________________________________________________________________________
void WordsJsonlSerializer::serializeToStream(std::ostream& outStream) {
  // Iterate through the glyphs of the document and add a line to the file for each.
  size_t rank = 0;
  for (const auto* page : _doc->pages) {
    for (const auto* block : page->blocks) {
      for (const auto* line : block->lines) {
        for (const auto* word : line->words) {
          outStream << "{"
            << "\"id\": \"" << word->id << "\", "
            << "\"rank\": " << rank++ << ", "
            << "\"page\": " << word->pageNum << ", "
            << "\"minX\": " << word->minX << ", "
            << "\"minY\": " << word->minY << ", "
            << "\"maxX\": " << word->maxX << ", "
            << "\"maxY\": " << word->maxY << ", "
            << "\"font\": \"" << word->fontName << "\", "
            << "\"fontSize\": " << word->fontSize << ", "
            << "\"text\": \"" << escapeJson(word->text) << "\", "
            << "\"block\": \"" << block->id << "\", "
            << "\"origin\": \"pdftotei\""
            << "}"
            << std::endl;
        }
      }
    }
  }
}
