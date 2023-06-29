/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <ostream>
#include <unordered_set>

#include "../PdfDocument.h"
#include "../Types.h"
#include "./PlainTextSerializer.h"

using ppp::types::DocumentUnit;
using ppp::types::SemanticRole;
using std::endl;
using std::unordered_set;

// =================================================================================================

namespace ppp::serialization {

// _________________________________________________________________________________________________
PlainTextSerializer::PlainTextSerializer() : Serializer() {}

// _________________________________________________________________________________________________
PlainTextSerializer::~PlainTextSerializer() = default;

// _________________________________________________________________________________________________
void PlainTextSerializer::serializeToStream(const PdfDocument* doc,
    const unordered_set<SemanticRole>& roles, const unordered_set<DocumentUnit>& units,
    ostream& out) const {
  assert(doc);

  PdfTextBlock* prevBlock = nullptr;
  for (auto* page : doc->pages) {
    for (auto* block : page->blocks) {
      // Skip the block if its role is not included in 'roles'.
      if (std::find(roles.begin(), roles.end(), block->role) == roles.end()) {
        continue;
      }

      if (prevBlock) {
        out << endl;
        out << endl;
      }

      PdfWord* prevWord = nullptr;
      for (auto* line : block->lines) {
        for (auto* word : line->words) {
          // Ignore the second part of hyphenated words, since their text is included in the text
          // of the first part of the hyphenated word.
          if (word->isSecondPartOfHyphenatedWord) {
            continue;
          }

          // Split the words by a whitespace.
          if (prevWord) {
            out << " ";
          }

          // Write the word character-wise. Ignore diacritic marks that were merged with their base
          // character (their text is part of the base character).
          if (word->isFirstPartOfHyphenatedWord) {
            // TODO(korzen): Hyphenated words should be also processed character-wise.
            out << word->isFirstPartOfHyphenatedWord->text;
          } else {
            for (const auto* ch : word->characters) {
              if (ch->isBaseCharOfDiacriticMark) {
                out << ch->textWithDiacriticMark;
              } else if (!ch->isDiacriticMarkOfBaseChar) {
                out << ch->text;
              }
            }
          }

          prevWord = word;
        }
      }
      prevBlock = block;
    }
  }
  out << endl;
}

}  // namespace ppp::serialization
