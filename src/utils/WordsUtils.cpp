/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // min, max
#include <string>
#include <vector>

#include "../Constants.h"
#include "../PdfDocument.h"

#include "./Counter.h"
#include "./StringUtils.h"
#include "./WordsUtils.h"

using global_config::ID_LENGTH;

using std::max;
using std::min;

// _________________________________________________________________________________________________
PdfWord* words_utils::createWord(const vector<PdfCharacter*>& characters, const PdfDocument* doc) {
  PdfWord* word = new PdfWord();
  word->doc = doc;

  // Create a (unique) id.
  word->id = string_utils::createRandomString(ID_LENGTH, "word-");

  // Iterative through the characters and compute the text, the x,y-coordinates of the
  // bounding box, and the font info.
  StringCounter fontNameCounter;
  DoubleCounter fontSizeCounter;
  string text;
  for (auto* ch : characters) {
    // Update the x,y-coordinates of the bounding box.
    word->pos->leftX = min(word->pos->leftX, ch->pos->leftX);
    word->pos->upperY = min(word->pos->upperY, ch->pos->upperY);
    word->pos->rightX = max(word->pos->rightX, ch->pos->rightX);
    word->pos->lowerY = max(word->pos->lowerY, ch->pos->lowerY);

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
  word->fontName = fontNameCounter.mostFreq();
  word->fontSize = fontSizeCounter.mostFreq();

  // Set the page number.
  word->pos->pageNum = characters[0]->pos->pageNum;

  // Set the writing mode.
  word->pos->wMode = characters[0]->pos->wMode;

  // Set the rotation value.
  word->pos->rotation = characters[0]->pos->rotation;

  // Set the chars.
  word->characters = characters;

  return word;
}
