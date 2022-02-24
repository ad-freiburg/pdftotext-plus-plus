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

#include "../PdfDocument.h"
#include "./TextBlocksJsonlSerializer.h"
#include "../utils/Utils.h"

// _________________________________________________________________________________________________
TextBlocksJsonlSerializer::TextBlocksJsonlSerializer(PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
TextBlocksJsonlSerializer::~TextBlocksJsonlSerializer() = default;

// _________________________________________________________________________________________________
void TextBlocksJsonlSerializer::serialize(const std::string& targetFilePath) {
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
