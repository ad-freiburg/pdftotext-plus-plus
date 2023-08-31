/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structurestrings.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cassert>  // assert
#include <string>
#include <vector>

#include "./Counter.h"
#include "./MathUtils.h"
#include "./TextUtils.h"
#include "./WordsDetectionUtils.h"
#include "../PdfDocument.h"

using std::string;
using std::vector;

using ppp::types::PdfCharacter;
using ppp::types::PdfWord;
using ppp::utils::counter::DoubleCounter;
using ppp::utils::counter::StringCounter;
using ppp::utils::math::maximum;
using ppp::utils::math::minimum;
using ppp::utils::text::createRandomString;

// =================================================================================================

namespace ppp::utils {

// _________________________________________________________________________________________________
WordsDetectionUtils::WordsDetectionUtils(const WordsDetectionConfig* config) {
  _config = config;
}

// _________________________________________________________________________________________________
WordsDetectionUtils::~WordsDetectionUtils() = default;

// _________________________________________________________________________________________________
PdfWord* WordsDetectionUtils::createWord(const vector<PdfCharacter*>& characters) const {
  assert(!characters.empty());

  PdfWord* word = new PdfWord();

  // Create a (unique) id.
  word->id = createRandomString(_config->idLength, "word-");

  // Set the page number.
  word->pos->pageNum = characters[0]->pos->pageNum;

  // Iterative through the characters and compute the text, the x,y-coordinates of the
  // bounding box, and the font info.
  StringCounter fontNameCounter;
  DoubleCounter fontSizeCounter;
  string text;
  for (auto* ch : characters) {
    // Update the x,y-coordinates of the bounding box.
    word->pos->leftX = minimum(word->pos->leftX, ch->pos->leftX);
    word->pos->upperY = minimum(word->pos->upperY, ch->pos->upperY);
    word->pos->rightX = maximum(word->pos->rightX, ch->pos->rightX);
    word->pos->lowerY = maximum(word->pos->lowerY, ch->pos->lowerY);

    // Compose the text. If the char was merged with a diacritic mark, append the text with the
    // diacritic mark. If the char is a diacritic mark which was merged with a base char, ignore
    // its text. Otherwise, append the normal text.
    if (ch->isBaseCharOfDiacriticMark) {
      text += ch->textWithDiacriticMark;
    } else if (!ch->isDiacriticMarkOfBaseChar) {
      text += ch->text;
    }

    // Count the font names and font sizes, for computing the most frequent font name / font size.
    fontNameCounter[ch->fontName]++;
    fontSizeCounter[ch->fontSize]++;

    // Set the reference to the created word.
    ch->word = word;
  }

  // Set the text.
  word->text = text;

  // Set the most frequent font name and font size.
  if (fontNameCounter.sumCounts() > 0) {
    word->fontName = fontNameCounter.mostFreq();
  }
  if (fontSizeCounter.sumCounts() > 0) {
    word->fontSize = fontSizeCounter.mostFreq();
  }

  // Set the writing mode.
  word->pos->wMode = characters[0]->pos->wMode;

  // Set the rotation value.
  word->pos->rotation = characters[0]->pos->rotation;

  // Set the chars.
  word->characters = characters;

  word->doc = characters[0]->doc;

  return word;
}

}  // namespace ppp::utils
