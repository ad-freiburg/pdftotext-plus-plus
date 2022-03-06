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

#include "./GlyphsJsonlSerializer.h"
#include "../PdfDocument.h"
#include "../utils/Utils.h"

// _________________________________________________________________________________________________
GlyphsJsonlSerializer::GlyphsJsonlSerializer(PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
GlyphsJsonlSerializer::~GlyphsJsonlSerializer() = default;

// _________________________________________________________________________________________________
void GlyphsJsonlSerializer::serialize(const std::string& targetFilePath) {
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
void GlyphsJsonlSerializer::serializeToStream(std::ostream& outStream) {
  // for (const auto* page : _doc->pages) {
  //   for (const auto* block : page->blocks) {
  //     for (const auto* line : block->lines) {
  //       for (const auto* word : line->words) {
  //         for (const auto* g : word->glyphs) {
  //           std::cout << "g" << std::endl;
  //           if (g->isDiacriticMarkOfBaseGlyph) {
  //             continue;
  //           }

  //           const PdfFontInfo* fontInfo = _doc->fontInfos.at(g->fontName);
  //           std::string text = g->isBaseGlyphOfDiacriticMark ? g->textWithDiacriticMark : g->text;

  //           outStream << "{"
  //             << "\"id\": \"" << g->id << "\", "
  //             << "\"rank\": " << rank++ << ", "
  //             << "\"page\": " << g->pageNum << ", "
  //             << "\"minX\": " << g->minX << ", "
  //             << "\"minY\": " << g->minY << ", "
  //             << "\"maxX\": " << g->maxX << ", "
  //             << "\"maxY\": " << g->maxY << ", "
  //             << "\"font\": \"" << g->fontName << "\", "
  //             << "\"fontSize\": " << g->fontSize << ", "
  //             << "\"weight\": " << fontInfo->weight << ", "
  //             << "\"italic\": " << (fontInfo->isItalic ? "true" : "false") << ", "
  //             << "\"color\": [" << g->color[0] << "," << g->color[1] << "," << g->color[2] << "],"
  //             << "\"opacity\": " << g->opacity << ", "
  //             << "\"text\": \"" << escapeJson(text) << "\", "
  //             << "\"word\": \"" << word->id << "\", "
  //             << "\"origin\": \"pdftotext++\""
  //             << "}"
  //             << std::endl;
  //         }
  //       }
  //     }
  //   }
  // }

  for (const auto* page : _doc->pages) {
    // Serialize the page information.
    outStream << "{"
        << "\"type\": \"page\", "
        << "\"num\": " << page->pageNum << ", "
        << "\"width\": " << page->width << ", "
        << "\"height\": " << page->height
        << "}" << std::endl;

    for (const auto* g : page->glyphs) {
      if (g->isDiacriticMarkOfBaseGlyph) {
        continue;
      }

      const PdfFontInfo* fontInfo = _doc->fontInfos.at(g->fontName);
      std::string text = g->isBaseGlyphOfDiacriticMark ? g->textWithDiacriticMark : g->text;

      // Serialize the glyph information.
      outStream << "{"
        << "\"type\": \"glyph\", "
        << "\"id\": \"" << g->id << "\", "
        << "\"rank\": " << g->rank << ", "
        << "\"page\": " << g->pageNum << ", "
        << "\"minX\": " << g->minX << ", "
        << "\"minY\": " << g->minY << ", "
        << "\"maxX\": " << g->maxX << ", "
        << "\"maxY\": " << g->maxY << ", "
        << "\"font\": \"" << g->fontName << "\", "
        << "\"fontSize\": " << g->fontSize << ", "
        << "\"weight\": " << fontInfo->weight << ", "
        << "\"italic\": " << (fontInfo->isItalic ? "true" : "false") << ", "
        << "\"type-3\": " << (fontInfo->isType3 ? "true" : "false") << ", "
        << "\"color\": [" << g->color[0] << "," << g->color[1] << "," << g->color[2] << "],"
        << "\"opacity\": " << g->opacity << ", "
        << "\"text\": \"" << escapeJson(text) << "\", "
        << "\"origin\": \"pdftotext++\""
        << "}"
        << std::endl;
    }
  }
}
