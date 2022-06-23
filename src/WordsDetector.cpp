/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // min, max
#include <limits>  // std::numeric_limits
#include <string>  // string
#include <vector>  // vector

#include "./utils/Counter.h"
#include "./utils/Log.h"
#include "./utils/MathUtils.h"
#include "./utils/PdfElementsUtils.h"
#include "./utils/StringUtils.h"

#include "./PdfDocument.h"
#include "./WordsDetector.h"

using global_config::ID_LENGTH;

using std::endl;
using std::max;
using std::min;
using std::string;
using std::vector;

using words_detector::config::FSIZE_EQUAL_TOLERANCE;
using words_detector::config::STACKED_MATH_CHAR_NAMES;
using words_detector::config::STACKED_MATH_CHAR_TEXTS;
using words_detector::config::STACKED_MATH_WORDS;

// _________________________________________________________________________________________________
WordsDetector::WordsDetector(const PdfDocument* doc, bool debug, int debugPageFilter) {
  _log = new Logger(debug ? DEBUG : INFO, debugPageFilter);
  _doc = doc;
}

// _________________________________________________________________________________________________
WordsDetector::~WordsDetector() {
  delete _log;
}

// _________________________________________________________________________________________________
void WordsDetector::process() {
  assert(_doc);

  _log->info() << "Detecting words..." << endl;
  _log->debug() << "=======================================" << endl;
  _log->debug() << BOLD << "DEBUG MODE" << OFF << endl;

  for (auto* page : _doc->pages) {
    detectWords(page);
  }

  _log->debug() << "=======================================" << endl;
  _log->debug() << "Merging stacked math symbols..." << endl;
  for (auto* page : _doc->pages) {
    mergeStackedMathSymbols(page);
  }
}

// _________________________________________________________________________________________________
void WordsDetector::detectWords(PdfPage* page) {
  assert(page);

  // Reset the active word.
  _activeWord.characters.clear();
  _activeWord.position->pageNum = page->pageNum;
  _activeWord.position->leftX = numeric_limits<double>::max();
  _activeWord.position->upperY = numeric_limits<double>::max();
  _activeWord.position->rightX = numeric_limits<double>::min();
  _activeWord.position->lowerY = numeric_limits<double>::min();
  _activeWord.position->rotation = 0;
  _activeWord.position->wMode = 0;
  _activeWord.fontSize = 0;

  // Do nothing if the page does not contain any characters.
  if (page->characters.empty()) {
    return;
  }

  int p = page->pageNum;

  // Iterate through the characters of the page. For each character, decide whether or not the
  // character starts a new word, by analyzing different layout information.
  for (auto* currChar : page->characters) {
    _log->debug(p) << "=======================================" << endl;
    _log->debug(p) << BOLD << "char: \"" << currChar->text << "\"" << OFF << endl;
    _log->debug(p) << " └─ char.page:   " << currChar->position->pageNum << endl;
    _log->debug(p) << " └─ char.leftX:  " << currChar->position->leftX << endl;
    _log->debug(p) << " └─ char.upperY: " << currChar->position->upperY << endl;
    _log->debug(p) << " └─ char.rightX: " << currChar->position->rightX << endl;
    _log->debug(p) << " └─ char.lowerY: " << currChar->position->lowerY << endl;
    if (currChar->position->rotation != 0) {
      _log->debug(p) << " └─ char.rotation:  " << currChar->position->rotation << endl;
      _log->debug(p) << " └─ char.rotLeftX:  " << currChar->position->getRotLeftX() << endl;
      _log->debug(p) << " └─ char.rotUpperY: " << currChar->position->getRotUpperY() << endl;
      _log->debug(p) << " └─ char.rotRightX: " << currChar->position->getRotRightX() << endl;
      _log->debug(p) << " └─ char.rotLowerY: " << currChar->position->getRotLowerY() << endl;
    }
    _log->debug(p) << "---------------------------------------" << endl;

    // Skip diacritic marks that were already merged with their base characters.
    if (currChar->isDiacriticMarkOfBaseChar) {
      _log->debug(p) << BOLD << "Skipping char (is merged diacritic mark)." << OFF << endl;
      continue;
    }

    // Check if the char starts a new word. If so, create a word from the characters of the
    // "active" word and start a new word.
    if (startsWord(currChar) && !_activeWord.characters.empty()) {
      PdfWord* word = createWord(_activeWord.characters, &page->words);

      _log->debug(p) << "---------------------------------------" << endl;
      _log->debug(p) << BOLD << "created word: \"" << word->text << "\"" << OFF << endl;
      _log->debug(p) << " └─ word.page: " << word->position->pageNum << endl;
      _log->debug(p) << " └─ word.leftX: " << word->position->leftX << endl;
      _log->debug(p) << " └─ word.upperY: " << word->position->upperY << endl;
      _log->debug(p) << " └─ word.rightX: " << word->position->rightX << endl;
      _log->debug(p) << " └─ word.lowerY: " << word->position->lowerY << endl;

      _activeWord.characters.clear();
      _activeWord.position->leftX = numeric_limits<double>::max();
      _activeWord.position->upperY = numeric_limits<double>::max();
      _activeWord.position->rightX = numeric_limits<double>::min();
      _activeWord.position->lowerY = numeric_limits<double>::min();
      _activeWord.position->rotation = 0;
      _activeWord.position->wMode = 0;
      _activeWord.fontSize = 0;
    }

    // Append the character to the active word and recompute the position.
    _activeWord.characters.push_back(currChar);
    _activeWord.position->leftX = min(_activeWord.position->leftX, currChar->position->leftX);
    _activeWord.position->upperY = min(_activeWord.position->upperY, currChar->position->upperY);
    _activeWord.position->rightX = max(_activeWord.position->rightX, currChar->position->rightX);
    _activeWord.position->lowerY = max(_activeWord.position->lowerY, currChar->position->lowerY);
    _activeWord.position->rotation = currChar->position->rotation;
    _activeWord.position->wMode = currChar->position->wMode;
    _activeWord.fontSize = max(_activeWord.fontSize, currChar->fontSize);
  }

  // Don't forget to process the last word.
  if (!_activeWord.characters.empty()) {
    PdfWord* word = createWord(_activeWord.characters, &page->words);

    _log->debug(p) << "---------------------------------------" << endl;
    _log->debug(p) << BOLD << "created word: \"" << word->text << "\"" << OFF << endl;
    _log->debug(p) << " └─ word.page: " << word->position->pageNum << endl;
    _log->debug(p) << " └─ word.leftX: " << word->position->leftX << endl;
    _log->debug(p) << " └─ word.upperY: " << word->position->upperY << endl;
    _log->debug(p) << " └─ word.rightX: " << word->position->rightX << endl;
    _log->debug(p) << " └─ word.lowerY: " << word->position->lowerY << endl;
  }
}

// _________________________________________________________________________________________________
bool WordsDetector::startsWord(const PdfCharacter* currChar, double minYOverlapRatioThreshold,
      double hGapThresholdFactor) const {
  assert(currChar);

  int p = currChar->position->pageNum;

  PdfCharacter* prevChar = nullptr;
  if (!_activeWord.characters.empty()) {
    prevChar = _activeWord.characters.back();
  }

  // ----------------
  // The character starts a new word if the active word is empty.

  _log->debug(p) << BLUE << "Is the active word empty?" << OFF << endl;
  _log->debug(p) << " └─ prevChar: " << (prevChar ? prevChar->text : "-") << endl;
  if (!prevChar) {
    _log->debug(p) << BLUE << BOLD << " yes → starts word" << OFF << endl;
    return true;
  }

  // ----------------
  // The character starts a new word if it has another rotation than the active word.

  _log->debug(p) << BLUE << "Does the char have another rotation than activeWord?" << OFF << endl;
  _log->debug(p) << " └─ activeWord.rotation: " << _activeWord.position->rotation << endl;
  _log->debug(p) << " └─ char.rotation: " << currChar->position->rotation << endl;
  if (_activeWord.position->rotation != currChar->position->rotation) {
    _log->debug(p) << BLUE << BOLD << " yes → starts word" << OFF << endl;
    return true;
  }

  // ----------------
  // The character starts a new word if it has another writing mode than the active word.

  _log->debug(p) << BLUE << "Does the char have another wMode than activeWord?" << OFF << endl;
  _log->debug(p) << " └─ activeWord.wMode: " << _activeWord.position->wMode << endl;
  _log->debug(p) << " └─ char.wMode: " << currChar->position->wMode << endl;
  if (_activeWord.position->wMode != currChar->position->wMode) {
    _log->debug(p) << BLUE << BOLD << " yes → starts word" << OFF << endl;
    return true;
  }

  // ----------------
  // The character starts a new word if the maximum y-overlap between the character and the active
  // word is smaller than the given threshold.

  double maxYOverlapRatio = element_utils::computeMaxYOverlapRatio(currChar, &_activeWord);

  _log->debug(p) << BLUE << "Is the maximum y-overlap ratio between the character and the active "
      << "word smaller than a threshold?" << endl;
  _log->debug(p) << " └─ maxYRatio: " << maxYOverlapRatio << endl;
  _log->debug(p) << " └─ threshold: " << minYOverlapRatioThreshold << endl;
  if (maxYOverlapRatio < minYOverlapRatioThreshold) {
    _log->debug(p) << BLUE << BOLD << " yes → starts word" << OFF << endl;
    return true;
  }

  // ----------------
  // The character starts a new word if the horizontal gap between the character and the previous
  // character is larger than a threshold.

  double hGapLeft = element_utils::computeHorizontalGap(currChar, &_activeWord);
  double hGapRight = element_utils::computeHorizontalGap(&_activeWord, currChar);
  double hGapThreshold = hGapThresholdFactor * _activeWord.fontSize;

  _log->debug(p) << BLUE << "Are the horizontal gaps between the character and the active word "
      << "larger than a threshold?" << OFF << endl;
  _log->debug(p) << " └─ hGapLeft:  " << hGapLeft << endl;
  _log->debug(p) << " └─ hGapRight: " << hGapRight << endl;
  _log->debug(p) << " └─ threshold: " << hGapThreshold << endl;

  if (hGapLeft > hGapThreshold) {
    _log->debug(p) << BLUE << BOLD << " yes (hGapLeft) → starts word" << OFF << endl;
    return true;
  }
  if (hGapRight > hGapThreshold) {
    _log->debug(p) << BLUE << BOLD << " yes (hGapRight) → starts word" << OFF << endl;
    return true;
  }

  _log->debug(p) << BLUE << BOLD << "no rule applied → continues word" << OFF << endl;
  return false;
}

// _________________________________________________________________________________________________
void WordsDetector::mergeStackedMathSymbols(const PdfPage* page,
      double minXOverlapRatioThreshold) const {
  assert(page);

  int p = page->pageNum;

  for (size_t i = 0; i < page->words.size(); i++) {
    PdfWord* word = page->words.at(i);

    _log->debug(p) << "=======================================" << endl;
    _log->debug(p) << BOLD << "word: \"" << word->text << "\"" << OFF << endl;
    _log->debug(p) << " └─ word.page: " << word->position->pageNum << endl;
    _log->debug(p) << " └─ word.leftX: " << word->position->leftX << endl;
    _log->debug(p) << " └─ word.upperY: " << word->position->upperY << endl;
    _log->debug(p) << " └─ word.rightX: " << word->position->rightX << endl;
    _log->debug(p) << " └─ word.lowerY: " << word->position->lowerY << endl;
    _log->debug(p) << " └─ word.fontSize: " << word->fontSize << endl;
    if (word->position->rotation != 0) {
      _log->debug(p) << " └─ word.rotation:  " << word->position->rotation << endl;
      _log->debug(p) << " └─ word.rotLeftX:  " << word->position->getRotLeftX() << endl;
      _log->debug(p) << " └─ word.rotUpperY: " << word->position->getRotUpperY() << endl;
      _log->debug(p) << " └─ word.rotRightX: " << word->position->getRotRightX() << endl;
      _log->debug(p) << " └─ word.rotLowerY: " << word->position->getRotLowerY() << endl;
    }

    // Check if the word is the base word of a stacked math symbol.
    bool isBaseOfStackedMathSymbol = false;
    for (auto* ch : word->characters) {
      if (STACKED_MATH_CHAR_TEXTS.count(ch->text) > 0) {
        isBaseOfStackedMathSymbol = true;
        break;
      }
      if (STACKED_MATH_CHAR_NAMES.count(ch->name) > 0) {
        isBaseOfStackedMathSymbol = true;
        break;
      }
    }
    if (STACKED_MATH_WORDS.count(word->text) > 0) {
      isBaseOfStackedMathSymbol = true;
    }
    _log->debug(p) << " └─ word.isBaseOfStackedSymbol: " << isBaseOfStackedMathSymbol << endl;

    // Skip the word if it is not the base word of a stacked math symbol.
    if (!isBaseOfStackedMathSymbol) {
      _log->debug(p) << BOLD << "Skipping word (not base of stacked math symbol)." << OFF << endl;
      continue;
    }

    // Iterate through the previous words in reversed order (starting at the current word) for
    // checking if they are also part of the stacked math symbol. Consider a word to be part of the
    // stacked math symbol, if the horizontal overlap between the word and the base word is
    // larger than a threshold, and if the font size of the word is smaller.
    _log->debug(p) << "---------" << endl;
    _log->debug(p) << "Searching for prev words that are part of the stacked symbol..." << endl;
    for (size_t j = i; j --> 0 ;) {  // equivalent to: for (int j = i; j > 0; j--) {
      auto* prevWord = page->words.at(j);

      _log->debug(p) << BOLD << "prevWord: \"" << prevWord->text << "\"" << OFF << endl;
      _log->debug(p) << " └─ prevWord.page: " << prevWord->position->pageNum << endl;
      _log->debug(p) << " └─ prevWord.leftX: " << prevWord->position->leftX << endl;
      _log->debug(p) << " └─ prevWord.upperY: " << prevWord->position->upperY << endl;
      _log->debug(p) << " └─ prevWord.rightX: " << prevWord->position->rightX << endl;
      _log->debug(p) << " └─ prevWord.lowerY: " << prevWord->position->lowerY << endl;
      _log->debug(p) << " └─ prevWord.fontSize: " << prevWord->fontSize << endl;

      // The previous word is not a part of the stacked math symbol when the maximum x-overlap
      // ratio is smaller than a threshold.
      double maxXOverlapRatio = element_utils::computeMaxXOverlapRatio(word, prevWord);
      _log->debug(p) << " └─ maxXOverlapRatio:          " << maxXOverlapRatio << endl;
      _log->debug(p) << " └─ minXOverlapRatioThreshold: " << minXOverlapRatioThreshold << endl;
      if (maxXOverlapRatio < minXOverlapRatioThreshold) {
        _log->debug(p) << BOLD << "is *not* part of the stacked math symbol "
            << "(maxXOverlapRatio < threshold)." << OFF << endl;
        break;
      }

      // The previous word is not a part of the stacked math symbol when its font size is not
      // smaller than the font size of the base word.
      _log->debug(p) << " └─ prevWord.fontSize: " << prevWord->fontSize << endl;
      _log->debug(p) << " └─ word.fontSize:     " << word->fontSize << endl;
      if (!math_utils::smaller(prevWord->fontSize, word->fontSize, FSIZE_EQUAL_TOLERANCE)) {
        _log->debug(p) << BOLD << "is *not* part of the stacked math symbol "
            << "(prevWord.fontSize >= word.fontSize)." << OFF << endl;
        break;
      }

      _log->debug(p) << BOLD << "is part of the stacked math symbol." << OFF << endl;
      word->isBaseOfStackedMathSymbol.push_back(prevWord);
      prevWord->isPartOfStackedMathSymbol = word;
    }

    // Iterate through the next words for checking if they are also part of the stacked math symbol.
    // Consider a word to be part of the stacked math symbol, if the horizontal overlap between
    // the word and the base word is large enough, and if the font size of the word is smaller.
    _log->debug(p) << "---------" << endl;
    _log->debug(p) << "Searching for next words that are part of the stacked symbol..." << endl;
    for (size_t j = i + 1; j < page->words.size(); j++) {
      auto* nextWord = page->words.at(j);

      _log->debug(p) << BOLD << "nextWord: \"" << nextWord->text << "\"" << OFF << endl;
      _log->debug(p) << " └─ nextWord.page: " << nextWord->position->pageNum << endl;
      _log->debug(p) << " └─ nextWord.leftX: " << nextWord->position->leftX << endl;
      _log->debug(p) << " └─ nextWord.upperY: " << nextWord->position->upperY << endl;
      _log->debug(p) << " └─ nextWord.rightX: " << nextWord->position->rightX << endl;
      _log->debug(p) << " └─ nextWord.lowerY: " << nextWord->position->lowerY << endl;
      _log->debug(p) << " └─ nextWord.fontSize: " << nextWord->fontSize << endl;

      // The next word is not a part of the stacked math symbol when the maximum x-overlap ratio is
      // smaller than a threshold.
      double maxXOverlapRatio = element_utils::computeMaxXOverlapRatio(word, nextWord);
      _log->debug(p) << " └─ maxXOverlapRatio:          " << maxXOverlapRatio << endl;
      _log->debug(p) << " └─ minXOverlapRatioThreshold: " << minXOverlapRatioThreshold << endl;
      if (maxXOverlapRatio < minXOverlapRatioThreshold) {
        _log->debug(p) << BOLD << "is *not* part of the stacked math symbol "
            << "(maxXOverlapRatio < threshold)." << OFF << endl;
        break;
      }

      // The next word is not a part of the stacked math symbol when its font size is not
      // smaller than the font size of the base word.
      _log->debug(p) << " └─ nextWord.fontSize: " << nextWord->fontSize << endl;
      _log->debug(p) << " └─ word.fontSize:     " << word->fontSize << endl;
      if (!math_utils::smaller(nextWord->fontSize, word->fontSize, FSIZE_EQUAL_TOLERANCE)) {
        _log->debug(p) << BOLD << "is *not* part of the stacked math symbol "
            << "(nextWord.fontSize >= word.fontSize)." << OFF << endl;
        break;
      }

      word->isBaseOfStackedMathSymbol.push_back(nextWord);
      nextWord->isPartOfStackedMathSymbol = word;
      _log->debug(p) << BOLD << "is part of the stacked math symbol." << OFF << endl;
    }
  }

  _log->debug(p) << "=======================================" << endl;
}

// _________________________________________________________________________________________________
PdfWord* WordsDetector::createWord(const vector<PdfCharacter*>& characters,
      vector<PdfWord*>* words) const {
  assert(words);

  PdfWord* word = new PdfWord();
  word->doc = _doc;

  // Create a (unique) id.
  word->id = string_utils::createRandomString(ID_LENGTH, "word-");

  // Iterative through the characters and compute the text, the x,y-coordinates of the
  // bounding box, and the font info.
  StringCounter fontNameCounter;
  DoubleCounter fontSizeCounter;
  string text;
  for (auto* ch : characters) {
    // Update the x,y-coordinates of the bounding box.
    word->position->leftX = min(word->position->leftX, ch->position->leftX);
    word->position->upperY = min(word->position->upperY, ch->position->upperY);
    word->position->rightX = max(word->position->rightX, ch->position->rightX);
    word->position->lowerY = max(word->position->lowerY, ch->position->lowerY);

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
  word->position->pageNum = characters[0]->position->pageNum;

  // Set the writing mode.
  word->position->wMode = characters[0]->position->wMode;

  // Set the rotation value.
  word->position->rotation = characters[0]->position->rotation;

  // Set the rank.
  word->rank = words->size();

  // Set the chars.
  word->characters = characters;

  words->push_back(word);

  return word;
}
