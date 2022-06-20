/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cstdlib>  // system()
#include <fstream>  // std::ofstream
#include <iostream>
#include <string>

#include "./JsonlSerializer.h"

#include "../utils/StringUtils.h"

#include "../PdfDocument.h"

using std::cerr;
using std::endl;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// _________________________________________________________________________________________________
JsonlSerializer::JsonlSerializer(PdfDocument* doc, bool serializePages, bool serializeChars,
      bool serializeFigures, bool serializeShapes, bool serializeWords, bool serializeTextBlocks) {
  _doc = doc;
  _serializePages = serializePages;
  _serializeChars = serializeChars;
  _serializeFigures = serializeFigures;
  _serializeShapes = serializeShapes;
  _serializeWords = serializeWords;
  _serializeTextBlocks = serializeTextBlocks;
}

// _________________________________________________________________________________________________
JsonlSerializer::~JsonlSerializer() = default;

// _________________________________________________________________________________________________
void JsonlSerializer::serialize(const string& targetFilePath) {
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
void JsonlSerializer::serializeToStream(ostream& stream) {
  assert(_doc);

  for (const auto* page : _doc->pages) {
    if (_serializePages) {
      serializePage(page, stream);
    }
    if (_serializeChars) {
      serializeChars(page->characters, stream);
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
void JsonlSerializer::serializePage(const PdfPage* page, ostream& stream) {
  assert(page);

  stream << "{"
    << "\"type\": \"page\", "
    << "\"num\": " << page->pageNum << ", "
    << "\"width\": " << page->getWidth() << ", "
    << "\"height\": " << page->getHeight() << ", "
    << "\"origin\": \"pdftotext++\""
    << "}"
    << endl;
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializeChars(const vector<PdfCharacter*>& chars, ostream& stream) {
  for (const auto* g : chars) {
    if (g->isDiacriticMarkOfBaseChar) {
      continue;
    }

    const PdfFontInfo* fontInfo = _doc->fontInfos.at(g->fontName);
    string text = g->isBaseCharOfDiacriticMark ? g->textWithDiacriticMark : g->text;

    string wordId = g->word ? g->word->id : "";
    string blockId = g->word && g->word->line && g->word->line->block
        ? g->word->line->block->id : "";

    // Serialize the character information.
    stream << "{"
      << "\"type\": \"char\", "
      << "\"id\": \"" << g->id << "\", "
      << "\"rank\": " << g->rank << ", "
      << "\"page\": " << g->position->pageNum << ", "
      << "\"minX\": " << g->position->leftX << ", "
      << "\"minY\": " << g->position->upperY << ", "
      << "\"maxX\": " << g->position->rightX << ", "
      << "\"maxY\": " << g->position->lowerY << ", "
      << "\"wMode\": " << g->position->wMode << ", "
      << "\"rotation\": " << g->position->rotation << ", "
      << "\"font\": \"" << g->fontName << "\", "
      << "\"fontSize\": " << g->fontSize << ", "
      << "\"weight\": " << fontInfo->weight << ", "
      << "\"italic\": " << (fontInfo->isItalic ? "true" : "false") << ", "
      << "\"type-3\": " << (fontInfo->isType3 ? "true" : "false") << ", "
      << "\"color\": [" << g->color[0] << "," << g->color[1] << "," << g->color[2] << "],"
      << "\"opacity\": " << g->opacity << ", "
      << "\"text\": \"" << string_utils::escapeJson(text) << "\", "
      << "\"word\": \"" << wordId << "\", "
      << "\"block\": \"" << blockId << "\", "
      << "\"origin\": \"pdftotext++\""
      << "}"
      << endl;
  }
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializeFigures(const vector<PdfFigure*>& figs, ostream& stream) {
  for (const auto* f : figs) {
    stream << "{"
      << "\"type\": \"figure\", "
      << "\"rank\": " << f->rank << ", "
      << "\"id\": \"" << f->id << "\", "
      << "\"page\": " << f->position->pageNum << ", "
      << "\"minX\": " << f->position->leftX << ", "
      << "\"minY\": " << f->position->upperY << ", "
      << "\"maxX\": " << f->position->rightX << ", "
      << "\"maxY\": " << f->position->lowerY << ", "
      << "\"origin\": \"pdftotext++\""
      << "}"
      << endl;
  }
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializeShapes(const vector<PdfShape*>& shapes, ostream& stream) {
  for (const auto* s : shapes) {
    stream << "{"
      << "\"type\": \"shape\", "
      << "\"rank\": " << s->rank << ", "
      << "\"id\": \"" << s->id << "\", "
      << "\"page\": " << s->position->pageNum << ", "
      << "\"minX\": " << s->position->leftX << ", "
      << "\"minY\": " << s->position->upperY << ", "
      << "\"maxX\": " << s->position->rightX << ", "
      << "\"maxY\": " << s->position->lowerY << ", "
      << "\"origin\": \"pdftotext++\""
      << "}"
      << endl;
  }
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializeWords(const vector<PdfWord*>& words, ostream& stream) {
  for (const auto* word : words) {
    string blockId = word->line && word->line->block ? word->line->block->id : "";

    stream << "{"
      << "\"type\": \"word\", "
      << "\"id\": \"" << word->id << "\", "
      << "\"rank\": " << word->rank << ", "
      << "\"page\": " << word->position->pageNum << ", "
      << "\"minX\": " << word->position->leftX << ", "
      << "\"minY\": " << word->position->upperY << ", "
      << "\"maxX\": " << word->position->rightX << ", "
      << "\"maxY\": " << word->position->lowerY << ", "
      << "\"font\": \"" << word->fontName << "\", "
      << "\"fontSize\": " << word->fontSize << ", "
      << "\"text\": \"" << string_utils::escapeJson(word->text) << "\", "
      << "\"block\": \"" << blockId << "\", "
      << "\"origin\": \"pdftotext++\""
      << "}"
      << endl;
  }
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializeTextBlocks(const vector<PdfTextBlock*>& blocks,
    ostream& stream) {
  for (const auto* block : blocks) {
    stream << "{"
      << "\"type\": \"block\", "
      << "\"id\": \"" << block->id << "\", "
      << "\"rank\": " << block->rank << ", "
      << "\"page\": " << block->position->pageNum << ", "
      << "\"minX\": " << block->position->leftX << ", "
      << "\"minY\": " << block->position->upperY << ", "
      << "\"maxX\": " << block->position->rightX << ", "
      << "\"maxY\": " << block->position->lowerY << ", "
      << "\"font\": \"" << block->fontName << "\", "
      << "\"fontSize\": " << block->fontSize << ", "
      << "\"text\": \"" << string_utils::escapeJson(block->text) << "\", "
      << "\"role\": \"" << block->role << "\", "
      << "\"origin\": \"pdftotext++\""
      << "}"
      << endl;
  }
}
