/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cassert>  // assert
#include <ostream>
#include <unordered_set>

#include "./PlainTextSerializer.h"
#include "../Types.h"

using std::endl;
using std::find;
using std::ostream;
using std::unordered_set;

using ppp::types::PdfDocument;
using ppp::types::PdfElementType;
using ppp::types::PdfTextBlock;
using ppp::types::PdfWord;
using ppp::types::SemanticRole;

// =================================================================================================

namespace ppp::serialization {

// _________________________________________________________________________________________________
PlainTextSerializer::PlainTextSerializer() : Serializer() {}

// _________________________________________________________________________________________________
PlainTextSerializer::~PlainTextSerializer() = default;

// _________________________________________________________________________________________________
void PlainTextSerializer::serializeToStream(const PdfDocument* doc,
    const unordered_set<SemanticRole>& roles, const unordered_set<PdfElementType>& types,
    ostream& out) const {
  assert(doc);

  PdfTextBlock* prevBlock = nullptr;
  for (auto* page : doc->pages) {
    for (auto* block : page->blocks) {
      // Skip the block if its role is not included in 'roles'.
      if (find(roles.begin(), roles.end(), block->role) == roles.end()) {
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
