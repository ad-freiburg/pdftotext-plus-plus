/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <filesystem>  // NOLINT(build/include)  (cpplint suggests to use
                       // "#include 'serializers/TextBlocksJsonlSerializer.h'", ignore this)
#include <fstream>
#include <iostream>
#include <string>

#include "../PdfDocument.h"
#include "./TextBlocksJsonlSerializer.h"
#include "../utils/Utils.h"

namespace fs = std::filesystem;

// _________________________________________________________________________________________________
TextBlocksJsonlSerializer::TextBlocksJsonlSerializer(PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
TextBlocksJsonlSerializer::~TextBlocksJsonlSerializer() = default;

// _________________________________________________________________________________________________
void TextBlocksJsonlSerializer::serialize(const std::string& targetFilePathStr) {
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
void TextBlocksJsonlSerializer::serializeToStream(std::ostream& outStream) {
  // Iterate through the text blocks of the document and add a line to the file for each.
  size_t rank = 0;
  for (const auto* page : _doc->pages) {
    for (const auto* block : page->blocks) {
      outStream << "{"
        << "\"id\": \"" << block->id << "\", "
        << "\"rank\": " << rank++ << ", "
        << "\"page\": " << block->pageNum << ", "
        << "\"minX\": " << block->minX << ", "
        << "\"minY\": " << block->minY << ", "
        << "\"maxX\": " << block->maxX << ", "
        << "\"maxY\": " << block->maxY << ", "
        << "\"font\": \"" << block->fontName << "\", "
        << "\"fontSize\": " << block->fontSize << ", "
        << "\"text\": \"" << escapeJson(block->text) << "\", "
        << "\"role\": \"" << block->role << "\", "
        << "\"origin\": \"pdftotei\""
        << "}"
        << std::endl;
    }
  }
}
