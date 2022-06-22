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
#include <vector>

#include "./JsonlSerializer.h"

#include "../utils/MathUtils.h"
#include "../utils/StringUtils.h"

#include "../Constants.h"
#include "../PdfDocument.h"

using global_config::COORDS_PREC;
using std::cout;
using std::cerr;
using std::endl;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// _________________________________________________________________________________________________
JsonlSerializer::JsonlSerializer(const PdfDocument* doc, bool serializePages, bool serializeChars,
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
void JsonlSerializer::serialize(const string& targetFilePath) const {
  // If the target file path is specified as "-", write the elements to the console. Otherwise,
  // write the elements to the referenced file.
  if (targetFilePath.size() == 1 && targetFilePath[0] == '-') {
    serializeToStream(cout);
  } else {
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
void JsonlSerializer::serializeToStream(ostream& stream) const {
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
void JsonlSerializer::serializePage(const PdfPage* page, ostream& stream) const {
  assert(page);

  stream << "{"
    << "\"type\": \"page\", "
    << "\"num\": " << page->pageNum << ", "
    << "\"width\": " << math_utils::round(page->getWidth(), COORDS_PREC) << ", "
    << "\"height\": " << math_utils::round(page->getHeight(), COORDS_PREC) << ", "
    << "\"origin\": \"pdftotext++\""
    << "}"
    << endl;
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializeChars(const vector<PdfCharacter*>& chars, ostream& stream) const {
  for (const auto* ch : chars) {
    // Ignore diacritics marks (since they were merged with their base characters).
    if (ch->isDiacriticMarkOfBaseChar) {
      continue;
    }

    // Get the font info about the character.
    const PdfFontInfo* fontInfo = _doc->fontInfos.at(ch->fontName);

    // Get the text.
    string text = ch->isBaseCharOfDiacriticMark ? ch->textWithDiacriticMark : ch->text;

    // Get the id of the parent word.
    string wordId = ch->word ? ch->word->id : "";

    // Get the id of the parent text block.
    string blockId = ch->word && ch->word->line && ch->word->line->block
        ? ch->word->line->block->id : "";

    // Serialize the character.
    stream << "{"
      << "\"type\": \"char\", "
      << "\"id\": \"" << ch->id << "\", "
      << "\"rank\": " << ch->rank << ", "
      << "\"page\": " << ch->position->pageNum << ", "
      << "\"minX\": " << math_utils::round(ch->position->leftX, COORDS_PREC) << ", "
      << "\"minY\": " << math_utils::round(ch->position->upperY, COORDS_PREC) << ", "
      << "\"maxX\": " << math_utils::round(ch->position->rightX, COORDS_PREC) << ", "
      << "\"maxY\": " << math_utils::round(ch->position->lowerY, COORDS_PREC) << ", "
      << "\"wMode\": " << ch->position->wMode << ", "
      << "\"rotation\": " << ch->position->rotation << ", "
      << "\"font\": \"" << ch->fontName << "\", "
      << "\"fontSize\": " << ch->fontSize << ", "
      << "\"weight\": " << fontInfo->weight << ", "
      << "\"italic\": " << (fontInfo->isItalic ? "true" : "false") << ", "
      << "\"type-3\": " << (fontInfo->isType3 ? "true" : "false") << ", "
      << "\"color\": [" << ch->color[0] << "," << ch->color[1] << "," << ch->color[2] << "],"
      << "\"opacity\": " << ch->opacity << ", "
      << "\"text\": \"" << string_utils::escapeJson(text) << "\", "
      << "\"word\": \"" << wordId << "\", "
      << "\"block\": \"" << blockId << "\", "
      << "\"origin\": \"pdftotext++\""
      << "}"
      << endl;
  }
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializeFigures(const vector<PdfFigure*>& figs, ostream& stream) const {
  for (const auto* f : figs) {
    stream << "{"
      << "\"type\": \"figure\", "
      << "\"rank\": " << f->rank << ", "
      << "\"id\": \"" << f->id << "\", "
      << "\"page\": " << f->position->pageNum << ", "
      << "\"minX\": " << math_utils::round(f->position->leftX, COORDS_PREC) << ", "
      << "\"minY\": " << math_utils::round(f->position->upperY, COORDS_PREC) << ", "
      << "\"maxX\": " << math_utils::round(f->position->rightX, COORDS_PREC) << ", "
      << "\"maxY\": " << math_utils::round(f->position->lowerY, COORDS_PREC) << ", "
      << "\"origin\": \"pdftotext++\""
      << "}"
      << endl;
  }
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializeShapes(const vector<PdfShape*>& shapes, ostream& stream) const {
  for (const auto* s : shapes) {
    stream << "{"
      << "\"type\": \"shape\", "
      << "\"rank\": " << s->rank << ", "
      << "\"id\": \"" << s->id << "\", "
      << "\"page\": " << s->position->pageNum << ", "
      << "\"minX\": " << math_utils::round(s->position->leftX, COORDS_PREC) << ", "
      << "\"minY\": " << math_utils::round(s->position->upperY, COORDS_PREC) << ", "
      << "\"maxX\": " << math_utils::round(s->position->rightX, COORDS_PREC) << ", "
      << "\"maxY\": " << math_utils::round(s->position->lowerY, COORDS_PREC) << ", "
      << "\"origin\": \"pdftotext++\""
      << "}"
      << endl;
  }
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializeWords(const vector<PdfWord*>& words, ostream& stream) const {
  for (const auto* word : words) {
    string blockId = word->line && word->line->block ? word->line->block->id : "";

    stream << "{"
      << "\"type\": \"word\", "
      << "\"id\": \"" << word->id << "\", "
      << "\"rank\": " << word->rank << ", "
      << "\"page\": " << word->position->pageNum << ", "
      << "\"minX\": " << math_utils::round(word->position->leftX, COORDS_PREC) << ", "
      << "\"minY\": " << math_utils::round(word->position->upperY, COORDS_PREC) << ", "
      << "\"maxX\": " << math_utils::round(word->position->rightX, COORDS_PREC) << ", "
      << "\"maxY\": " << math_utils::round(word->position->lowerY, COORDS_PREC) << ", "
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
      ostream& stream) const {
  for (const auto* block : blocks) {
    stream << "{"
      << "\"type\": \"block\", "
      << "\"id\": \"" << block->id << "\", "
      << "\"rank\": " << block->rank << ", "
      << "\"page\": " << block->position->pageNum << ", "
      << "\"minX\": " << math_utils::round(block->position->leftX, COORDS_PREC) << ", "
      << "\"minY\": " << math_utils::round(block->position->upperY, COORDS_PREC) << ", "
      << "\"maxX\": " << math_utils::round(block->position->rightX, COORDS_PREC) << ", "
      << "\"maxY\": " << math_utils::round(block->position->lowerY, COORDS_PREC) << ", "
      << "\"font\": \"" << block->fontName << "\", "
      << "\"fontSize\": " << block->fontSize << ", "
      << "\"text\": \"" << string_utils::escapeJson(block->text) << "\", "
      << "\"role\": \"" << block->role << "\", "
      << "\"origin\": \"pdftotext++\""
      << "}"
      << endl;
  }
}
