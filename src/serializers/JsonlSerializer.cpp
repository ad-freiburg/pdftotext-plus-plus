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

#include "./JsonlSerializer.h"
#include "../PdfDocument.h"
#include "../utils/Utils.h"

// _________________________________________________________________________________________________
JsonlSerializer::JsonlSerializer(PdfDocument* doc, bool serializePages, bool serializeGlyphs,
      bool serializeFigures, bool serializeShapes, bool serializeWords, bool serializeTextBlocks) {
  _doc = doc;
  _serializePages = serializePages;
  _serializeGlyphs = serializeGlyphs;
  _serializeFigures = serializeFigures;
  _serializeShapes = serializeShapes;
  _serializeWords = serializeWords;
  _serializeTextBlocks = serializeTextBlocks;
}

// _________________________________________________________________________________________________
JsonlSerializer::~JsonlSerializer() = default;

// _________________________________________________________________________________________________
void JsonlSerializer::serialize(const std::string& targetFilePath) {
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
void JsonlSerializer::serializeToStream(std::ostream& stream) {
  for (const auto* page : _doc->pages) {
    if (_serializePages) {
      serializePage(page, stream);
    }
    if (_serializeGlyphs) {
      serializeGlyphs(page->glyphs, stream);
    }
    if (_serializeFigures) {
      serializeFigures(page->figures, stream);
    }
    if (_serializeShapes) {
      serializeShapes(page->shapes, stream);
    }
    if (_serializeWords) {
      serializeWords(page->words, stream);
    }
    if (_serializeTextBlocks) {
      serializeTextBlocks(page->blocks, stream);
    }
  }
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializePage(const PdfPage* page, std::ostream& stream) {
  stream << "{"
    << "\"type\": \"page\", "
    << "\"num\": " << page->pageNum << ", "
    << "\"width\": " << page->width << ", "
    << "\"height\": " << page->height << ", "
    << "\"origin\": \"pdftotext++\""
    << "}"
    << std::endl;
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializeGlyphs(const std::vector<PdfGlyph*>& glyphs, std::ostream& stream) {
  for (const auto* g : glyphs) {
    if (g->isDiacriticMarkOfBaseGlyph) {
      continue;
    }

    const PdfFontInfo* fontInfo = _doc->fontInfos.at(g->fontName);
    std::string text = g->isBaseGlyphOfDiacriticMark ? g->textWithDiacriticMark : g->text;

    // Serialize the glyph information.
    stream << "{"
      << "\"type\": \"glyph\", "
      << "\"id\": \"" << g->id << "\", "
      << "\"rank\": " << g->rank << ", "
      << "\"page\": " << g->pageNum << ", "
      << "\"minX\": " << g->minX << ", "
      << "\"minY\": " << g->minY << ", "
      << "\"maxX\": " << g->maxX << ", "
      << "\"maxY\": " << g->maxY << ", "
      << "\"wMode\": " << g->wMode << ", "
      << "\"rotation\": " << g->rotation << ", "
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

// _________________________________________________________________________________________________
void JsonlSerializer::serializeFigures(const std::vector<PdfFigure*>& figs, std::ostream& stream) {
  for (const auto* f : figs) {
    stream << "{"
      << "\"type\": \"figure\", "
      << "\"page\": " << f->pageNum << ", "
      << "\"minX\": " << f->minX << ", "
      << "\"minY\": " << f->minY << ", "
      << "\"maxX\": " << f->maxX << ", "
      << "\"maxY\": " << f->maxY << ", "
      << "\"origin\": \"pdftotext++\""
      << "}"
      << std::endl;
  }
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializeShapes(const std::vector<PdfShape*>& shapes, std::ostream& stream) {
  for (const auto* s : shapes) {
    stream << "{"
      << "\"type\": \"shape\", "
      << "\"page\": " << s->pageNum << ", "
      << "\"minX\": " << s->minX << ", "
      << "\"minY\": " << s->minY << ", "
      << "\"maxX\": " << s->maxX << ", "
      << "\"maxY\": " << s->maxY << ", "
      << "\"origin\": \"pdftotext++\""
      << "}"
      << std::endl;
  }
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializeWords(const std::vector<PdfWord*>& words, std::ostream& stream) {
  for (const auto* word : words) {
    stream << "{"
      << "\"type\": \"word\", "
      << "\"id\": \"" << word->id << "\", "
      // << "\"rank\": " << rank++ << ", "
      << "\"page\": " << word->pageNum << ", "
      << "\"minX\": " << word->minX << ", "
      << "\"minY\": " << word->minY << ", "
      << "\"maxX\": " << word->maxX << ", "
      << "\"maxY\": " << word->maxY << ", "
      << "\"font\": \"" << word->fontName << "\", "
      << "\"fontSize\": " << word->fontSize << ", "
      << "\"text\": \"" << escapeJson(word->text) << "\", "
      // << "\"block\": \"" << block->id << "\", "
      << "\"origin\": \"pdftotext++\""
      << "}"
      << std::endl;
  }
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializeTextBlocks(const std::vector<PdfTextBlock*>& blocks,
    std::ostream& stream) {
  for (const auto* block : blocks) {
    stream << "{"
      << "\"type\": \"block\", "
      << "\"id\": \"" << block->id << "\", "
      // << "\"rank\": " << rank++ << ", "
      << "\"page\": " << block->pageNum << ", "
      << "\"minX\": " << block->minX << ", "
      << "\"minY\": " << block->minY << ", "
      << "\"maxX\": " << block->maxX << ", "
      << "\"maxY\": " << block->maxY << ", "
      << "\"font\": \"" << block->fontName << "\", "
      << "\"fontSize\": " << block->fontSize << ", "
      << "\"text\": \"" << escapeJson(block->text) << "\", "
      << "\"role\": \"" << block->role << "\", "
      << "\"origin\": \"pdftotext++\""
      << "}"
      << std::endl;
  }
}
