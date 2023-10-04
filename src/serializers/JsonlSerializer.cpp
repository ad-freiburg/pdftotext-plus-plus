/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cassert>  // assert
#include <iostream>
#include <string>
#include <unordered_set>

#include "./JsonlSerializer.h"
#include "../utils/MathUtils.h"
#include "../utils/PdfElementsUtils.h"
#include "../utils/TextUtils.h"
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
using ppp::utils::elements::getSemanticRoleName;
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
    const unordered_set<SemanticRole>& roles, const unordered_set<PdfElementType>& units,
    ostream& out) const {

  for (PdfElementType unit : units) {
    switch (unit) {
      case PdfElementType::PAGES:
        serializePages(doc, roles, out);
        break;
      case PdfElementType::CHARACTERS:
        serializeCharacters(doc, roles, out);
        break;
      case PdfElementType::WORDS:
        serializeWords(doc, roles, out);
        break;
      case PdfElementType::TEXT_BLOCKS:
        serializeTextBlocks(doc, roles, out);
        break;
      case PdfElementType::FIGURES:
        serializeFigures(doc, roles, out);
        break;
      case PdfElementType::SHAPES:
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
      << "\"width\": " << page->getWidth() << ", "
      << "\"height\": " << page->getHeight() << ", "
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
              << "\"minX\": " << c->pos->leftX << ", "
              << "\"minY\": " << c->pos->upperY << ", "
              << "\"maxX\": " << c->pos->rightX << ", "
              << "\"maxY\": " << c->pos->lowerY << ", "
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
        << "\"minX\": " << f->pos->leftX << ", "
        << "\"minY\": " << f->pos->upperY << ", "
        << "\"maxX\": " << f->pos->rightX << ", "
        << "\"maxY\": " << f->pos->lowerY << ", "
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
        << "\"minX\": " << s->pos->leftX << ", "
        << "\"minY\": " << s->pos->upperY << ", "
        << "\"maxX\": " << s->pos->rightX << ", "
        << "\"maxY\": " << s->pos->lowerY << ", "
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
            << "\"minX\": " << word->pos->leftX << ", "
            << "\"minY\": " << word->pos->upperY << ", "
            << "\"maxX\": " << word->pos->rightX << ", "
            << "\"maxY\": " << word->pos->lowerY << ", "
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
        << "\"minX\": " << block->pos->leftX << ", "
        << "\"minY\": " << block->pos->upperY << ", "
        << "\"maxX\": " << block->pos->rightX << ", "
        << "\"maxY\": " << block->pos->lowerY << ", "
        << "\"font\": \"" << block->fontName << "\", "
        << "\"fontSize\": " << block->fontSize << ", "
        << "\"text\": \"" << escapeJson(block->text) << "\", "
        << "\"role\": \"" << getSemanticRoleName(block->role) << "\", "
        << "\"origin\": \"pdftotext++\""
        << "}"
        << endl;
    }
  }
}

}  // namespace ppp::serialization
