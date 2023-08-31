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
#include "../utils/TextUtils.h"
#include "../PdfDocument.h"
#include "../PdfFontInfo.h"
#include "../Types.h"

using std::endl;
using std::find;
using std::ostream;
using std::string;
using std::unordered_set;

using ppp::types::PdfCharacter;
using ppp::types::PdfDocument;
using ppp::types::PdfFigure;
using ppp::types::PdfFontInfo;
using ppp::types::PdfPage;
using ppp::types::PdfShape;
using ppp::types::PdfTextBlock;
using ppp::types::PdfTextLine;
using ppp::types::PdfWord;
using ppp::types::SemanticRole;
using ppp::utils::math::round;
using ppp::utils::text::escapeJson;

// =================================================================================================

namespace ppp::serialization {

// _________________________________________________________________________________________________
JsonlSerializer::JsonlSerializer(size_t coordsPrecision) : Serializer() {
  _coordsPrecision = coordsPrecision;
}

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
      << "\"width\": " << round(page->getWidth(), _coordsPrecision) << ", "
      << "\"height\": " << round(page->getHeight(), _coordsPrecision) << ", "
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
      if (find(roles.begin(), roles.end(), block->role) == roles.end()) {
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
              << "\"minX\": " << round(c->pos->leftX, _coordsPrecision) << ", "
              << "\"minY\": " << round(c->pos->upperY, _coordsPrecision) << ", "
              << "\"maxX\": " << round(c->pos->rightX, _coordsPrecision) << ", "
              << "\"maxY\": " << round(c->pos->lowerY, _coordsPrecision) << ", "
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
        << "\"minX\": " << round(f->pos->leftX, _coordsPrecision) << ", "
        << "\"minY\": " << round(f->pos->upperY, _coordsPrecision) << ", "
        << "\"maxX\": " << round(f->pos->rightX, _coordsPrecision) << ", "
        << "\"maxY\": " << round(f->pos->lowerY, _coordsPrecision) << ", "
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
        << "\"minX\": " << round(s->pos->leftX, _coordsPrecision) << ", "
        << "\"minY\": " << round(s->pos->upperY, _coordsPrecision) << ", "
        << "\"maxX\": " << round(s->pos->rightX, _coordsPrecision) << ", "
        << "\"maxY\": " << round(s->pos->lowerY, _coordsPrecision) << ", "
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
      if (find(roles.begin(), roles.end(), block->role) == roles.end()) {
        continue;
      }

      for (const PdfTextLine* line : block->lines) {
        for (const PdfWord* word : line->words) {
          out << "{"
            << "\"type\": \"word\", "
            << "\"id\": \"" << word->id << "\", "
            << "\"rank\": " << word->rank << ", "
            << "\"page\": " << word->pos->pageNum << ", "
            << "\"minX\": " << round(word->pos->leftX, _coordsPrecision) << ", "
            << "\"minY\": " << round(word->pos->upperY, _coordsPrecision) << ", "
            << "\"maxX\": " << round(word->pos->rightX, _coordsPrecision) << ", "
            << "\"maxY\": " << round(word->pos->lowerY, _coordsPrecision) << ", "
            << "\"font\": \"" << word->fontName << "\", "
            << "\"fontSize\": " << word->fontSize << ", "
            << "\"text\": \"" << escapeJson(word->text) << "\", "
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
      if (find(roles.begin(), roles.end(), block->role) == roles.end()) {
        continue;
      }

      out << "{"
        << "\"type\": \"block\", "
        << "\"id\": \"" << block->id << "\", "
        << "\"rank\": " << block->rank << ", "
        << "\"page\": " << block->pos->pageNum << ", "
        << "\"minX\": " << round(block->pos->leftX, _coordsPrecision) << ", "
        << "\"minY\": " << round(block->pos->upperY, _coordsPrecision) << ", "
        << "\"maxX\": " << round(block->pos->rightX, _coordsPrecision) << ", "
        << "\"maxY\": " << round(block->pos->lowerY, _coordsPrecision) << ", "
        << "\"font\": \"" << block->fontName << "\", "
        << "\"fontSize\": " << block->fontSize << ", "
        << "\"text\": \"" << escapeJson(block->text) << "\", "
        << "\"role\": \"" << ppp::types::getName(block->role) << "\", "
        << "\"origin\": \"pdftotext++\""
        << "}"
        << endl;
    }
  }
}

}  // namespace ppp::serialization
