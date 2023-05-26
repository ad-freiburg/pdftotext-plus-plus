/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <iostream>
#include <string>
#include <unordered_set>

#include "./JsonlSerializer.h"

#include "../utils/MathUtils.h"
#include "../utils/StringUtils.h"

#include "../Constants.h"
#include "../PdfDocument.h"
#include "../Types.h"

using global_config::COORDS_PREC;

using ppp::types::SemanticRole;
using ppp::math_utils::round;
using ppp::string_utils::escapeJson;
using std::cout;
using std::cerr;
using std::endl;
using std::ostream;
using std::string;
using std::unordered_set;

// =================================================================================================

namespace ppp::serialization {

// _________________________________________________________________________________________________
JsonlSerializer::JsonlSerializer() : Serializer() {}

// _________________________________________________________________________________________________
JsonlSerializer::~JsonlSerializer() = default;

// _________________________________________________________________________________________________
void JsonlSerializer::serializeToStream(const PdfDocument* doc,
    const unordered_set<SemanticRole>& roles, const unordered_set<DocumentUnit>& units,
    ostream& out) const {

  for (DocumentUnit unit : units) {
    switch (unit) {
      case DocumentUnit::PAGES:
        serializePages(doc, roles, out);
        break;
      case DocumentUnit::CHARACTERS:
        serializeCharacters(doc, roles, out);
        break;
      case DocumentUnit::WORDS:
        serializeWords(doc, roles, out);
        break;
      case DocumentUnit::TEXT_BLOCKS:
        serializeTextBlocks(doc, roles, out);
        break;
      case DocumentUnit::FIGURES:
        serializeFigures(doc, roles, out);
        break;
      case DocumentUnit::SHAPES:
        serializeShapes(doc, roles, out);
        break;
      default:
        break;
    }
  }
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializePages(const PdfDocument* doc,
    const unordered_set<SemanticRole>& roles, ostream& out) const {
  assert(doc);

  for (const PdfPage* page : doc->pages) {
    out << "{"
      << "\"type\": \"page\", "
      << "\"num\": " << page->pageNum << ", "
      << "\"width\": " << round(page->getWidth(), COORDS_PREC) << ", "
      << "\"height\": " << round(page->getHeight(), COORDS_PREC) << ", "
      << "\"origin\": \"pdftotext++\""
      << "}"
      << endl;
  }
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializeCharacters(const PdfDocument* doc,
    const unordered_set<SemanticRole>& roles, ostream& out) const {
  assert(doc);

  for (const PdfPage* page : doc->pages) {
    for (const PdfTextBlock* block : page->blocks) {
      // Skip the block if its role is not included in 'roles'.
      if (std::find(roles.begin(), roles.end(), block->role) == roles.end()) {
        continue;
      }

      for (const PdfTextLine* line : block->lines) {
        for (const PdfWord* word : line->words) {
          for (const PdfCharacter* c : word->characters) {
            // Ignore diacritics marks (since they were merged with their base characters).
            if (c->isDiacriticMarkOfBaseChar) {
              continue;
            }

            // Get the font info about the character.
            const PdfFontInfo* fontInfo = doc->fontInfos.at(c->fontName);

            // Get the text.
            string text = c->isBaseCharOfDiacriticMark ? c->textWithDiacriticMark : c->text;

            // Serialize the character.
            out << "{"
              << "\"type\": \"char\", "
              << "\"id\": \"" << c->id << "\", "
              << "\"rank\": " << c->rank << ", "
              << "\"page\": " << c->pos->pageNum << ", "
              << "\"minX\": " << round(c->pos->leftX, COORDS_PREC) << ", "
              << "\"minY\": " << round(c->pos->upperY, COORDS_PREC) << ", "
              << "\"maxX\": " << round(c->pos->rightX, COORDS_PREC) << ", "
              << "\"maxY\": " << round(c->pos->lowerY, COORDS_PREC) << ", "
              << "\"wMode\": " << c->pos->wMode << ", "
              << "\"rotation\": " << c->pos->rotation << ", "
              << "\"font\": \"" << c->fontName << "\", "
              << "\"fontSize\": " << c->fontSize << ", "
              << "\"weight\": " << fontInfo->weight << ", "
              << "\"italic\": " << (fontInfo->isItalic ? "true" : "false") << ", "
              << "\"type-3\": " << (fontInfo->isType3 ? "true" : "false") << ", "
              << "\"color\": [" << c->color[0] << "," << c->color[1] << "," << c->color[2] << "],"
              << "\"opacity\": " << c->opacity << ", "
              << "\"text\": \"" << escapeJson(text) << "\", "
              << "\"word\": \"" << word->id << "\", "
              << "\"block\": \"" << block->id << "\", "
              << "\"origin\": \"pdftotext++\""
              << "}"
              << endl;
          }
        }
      }
    }
  }
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializeFigures(const PdfDocument* doc,
    const unordered_set<SemanticRole>& roles, ostream& out) const {
  assert(doc);

  for (const PdfPage* page : doc->pages) {
    for (const PdfFigure* f : page->figures) {
      out << "{"
        << "\"type\": \"figure\", "
        << "\"rank\": " << f->rank << ", "
        << "\"id\": \"" << f->id << "\", "
        << "\"page\": " << f->pos->pageNum << ", "
        << "\"minX\": " << round(f->pos->leftX, COORDS_PREC) << ", "
        << "\"minY\": " << round(f->pos->upperY, COORDS_PREC) << ", "
        << "\"maxX\": " << round(f->pos->rightX, COORDS_PREC) << ", "
        << "\"maxY\": " << round(f->pos->lowerY, COORDS_PREC) << ", "
        << "\"origin\": \"pdftotext++\""
        << "}"
        << endl;
    }
  }
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializeShapes(const PdfDocument* doc,
    const unordered_set<SemanticRole>& roles, ostream& out) const {
  assert(doc);

  for (const PdfPage* page : doc->pages) {
    for (const PdfShape* s : page->shapes) {
      out << "{"
        << "\"type\": \"shape\", "
        << "\"rank\": " << s->rank << ", "
        << "\"id\": \"" << s->id << "\", "
        << "\"page\": " << s->pos->pageNum << ", "
        << "\"minX\": " << round(s->pos->leftX, COORDS_PREC) << ", "
        << "\"minY\": " << round(s->pos->upperY, COORDS_PREC) << ", "
        << "\"maxX\": " << round(s->pos->rightX, COORDS_PREC) << ", "
        << "\"maxY\": " << round(s->pos->lowerY, COORDS_PREC) << ", "
        << "\"origin\": \"pdftotext++\""
        << "}"
        << endl;
    }
  }
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializeWords(const PdfDocument* doc,
    const unordered_set<SemanticRole>& roles, ostream& out) const {
  assert(doc);

  for (const PdfPage* page : doc->pages) {
    for (const PdfTextBlock* block : page->blocks) {
      // Skip the block if its role is not included in 'roles'.
      if (std::find(roles.begin(), roles.end(), block->role) == roles.end()) {
        continue;
      }

      for (const PdfTextLine* line : block->lines) {
        for (const PdfWord* word : line->words) {
          out << "{"
            << "\"type\": \"word\", "
            << "\"id\": \"" << word->id << "\", "
            << "\"rank\": " << word->rank << ", "
            << "\"page\": " << word->pos->pageNum << ", "
            << "\"minX\": " << round(word->pos->leftX, COORDS_PREC) << ", "
            << "\"minY\": " << round(word->pos->upperY, COORDS_PREC) << ", "
            << "\"maxX\": " << round(word->pos->rightX, COORDS_PREC) << ", "
            << "\"maxY\": " << round(word->pos->lowerY, COORDS_PREC) << ", "
            << "\"font\": \"" << word->fontName << "\", "
            << "\"fontSize\": " << word->fontSize << ", "
            << "\"text\": \"" << ppp::string_utils::escapeJson(word->text) << "\", "
            << "\"block\": \"" << block->id << "\", "
            << "\"origin\": \"pdftotext++\""
            << "}"
            << endl;
        }
      }
    }
  }
}

// _________________________________________________________________________________________________
void JsonlSerializer::serializeTextBlocks(const PdfDocument* doc,
    const unordered_set<SemanticRole>& roles, ostream& out) const {
  assert(doc);

  for (const PdfPage* page : doc->pages) {
    for (const PdfTextBlock* block : page->blocks) {
      // Skip the block if its role is not included in 'roles'.
      if (std::find(roles.begin(), roles.end(), block->role) == roles.end()) {
        continue;
      }

      out << "{"
        << "\"type\": \"block\", "
        << "\"id\": \"" << block->id << "\", "
        << "\"rank\": " << block->rank << ", "
        << "\"page\": " << block->pos->pageNum << ", "
        << "\"minX\": " << round(block->pos->leftX, COORDS_PREC) << ", "
        << "\"minY\": " << round(block->pos->upperY, COORDS_PREC) << ", "
        << "\"maxX\": " << round(block->pos->rightX, COORDS_PREC) << ", "
        << "\"maxY\": " << round(block->pos->lowerY, COORDS_PREC) << ", "
        << "\"font\": \"" << block->fontName << "\", "
        << "\"fontSize\": " << block->fontSize << ", "
        << "\"text\": \"" << string_utils::escapeJson(block->text) << "\", "
        << "\"role\": \"" << ppp::types::getName(block->role) << "\", "
        << "\"origin\": \"pdftotext++\""
        << "}"
        << endl;
    }
  }
}

}  // namespace ppp::serialization
