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
#include "./utils/WordsUtils.h"

#include "./PdfDocument.h"
#include "./WordsDetector.h"

using global_config::ID_LENGTH;

using std::endl;
using std::max;
using std::min;
using std::string;
using std::vector;

namespace config = words_detector::config;

// _________________________________________________________________________________________________
WordsDetector::WordsDetector(const PdfDocument* doc, LogLevel logLevel, int logPageFilter) {
  _log = new Logger(logLevel, logPageFilter);
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
  _activeWord.pos->pageNum = page->pageNum;
  _activeWord.pos->leftX = numeric_limits<double>::max();
  _activeWord.pos->upperY = numeric_limits<double>::max();
  _activeWord.pos->rightX = numeric_limits<double>::min();
  _activeWord.pos->lowerY = numeric_limits<double>::min();
  _activeWord.pos->rotation = 0;
  _activeWord.pos->wMode = 0;
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
    _log->debug(p) << " └─ char.page:   " << currChar->pos->pageNum << endl;
    _log->debug(p) << " └─ char.leftX:  " << currChar->pos->leftX << endl;
    _log->debug(p) << " └─ char.upperY: " << currChar->pos->upperY << endl;
    _log->debug(p) << " └─ char.rightX: " << currChar->pos->rightX << endl;
    _log->debug(p) << " └─ char.lowerY: " << currChar->pos->lowerY << endl;
    if (currChar->pos->rotation != 0) {
      _log->debug(p) << " └─ char.rotation:  " << currChar->pos->rotation << endl;
      _log->debug(p) << " └─ char.rotLeftX:  " << currChar->pos->getRotLeftX() << endl;
      _log->debug(p) << " └─ char.rotUpperY: " << currChar->pos->getRotUpperY() << endl;
      _log->debug(p) << " └─ char.rotRightX: " << currChar->pos->getRotRightX() << endl;
      _log->debug(p) << " └─ char.rotLowerY: " << currChar->pos->getRotLowerY() << endl;
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
      PdfWord* word = words_utils::createWord(_activeWord.characters, _doc);
      word->rank = page->words.size();
      page->words.push_back(word);

      _log->debug(p) << "---------------------------------------" << endl;
      _log->debug(p) << BOLD << "created word: \"" << word->text << "\"" << OFF << endl;
      _log->debug(p) << " └─ word.page: " << word->pos->pageNum << endl;
      _log->debug(p) << " └─ word.leftX: " << word->pos->leftX << endl;
      _log->debug(p) << " └─ word.upperY: " << word->pos->upperY << endl;
      _log->debug(p) << " └─ word.rightX: " << word->pos->rightX << endl;
      _log->debug(p) << " └─ word.lowerY: " << word->pos->lowerY << endl;

      _activeWord.characters.clear();
      _activeWord.pos->leftX = numeric_limits<double>::max();
      _activeWord.pos->upperY = numeric_limits<double>::max();
      _activeWord.pos->rightX = numeric_limits<double>::min();
      _activeWord.pos->lowerY = numeric_limits<double>::min();
      _activeWord.pos->rotation = 0;
      _activeWord.pos->wMode = 0;
      _activeWord.fontSize = 0;
    }

    // Append the character to the active word and recompute the position.
    _activeWord.characters.push_back(currChar);
    _activeWord.pos->leftX = min(_activeWord.pos->leftX, currChar->pos->leftX);
    _activeWord.pos->upperY = min(_activeWord.pos->upperY, currChar->pos->upperY);
    _activeWord.pos->rightX = max(_activeWord.pos->rightX, currChar->pos->rightX);
    _activeWord.pos->lowerY = max(_activeWord.pos->lowerY, currChar->pos->lowerY);
    _activeWord.pos->rotation = currChar->pos->rotation;
    _activeWord.pos->wMode = currChar->pos->wMode;
    _activeWord.fontSize = max(_activeWord.fontSize, currChar->fontSize);
  }

  // Don't forget to process the last word.
  if (!_activeWord.characters.empty()) {
    PdfWord* word = words_utils::createWord(_activeWord.characters, _doc);
    word->rank = page->words.size();
    page->words.push_back(word);

    _log->debug(p) << "---------------------------------------" << endl;
    _log->debug(p) << BOLD << "created word: \"" << word->text << "\"" << OFF << endl;
    _log->debug(p) << " └─ word.page: " << word->pos->pageNum << endl;
    _log->debug(p) << " └─ word.leftX: " << word->pos->leftX << endl;
    _log->debug(p) << " └─ word.upperY: " << word->pos->upperY << endl;
    _log->debug(p) << " └─ word.rightX: " << word->pos->rightX << endl;
    _log->debug(p) << " └─ word.lowerY: " << word->pos->lowerY << endl;
  }
}

// _________________________________________________________________________________________________
bool WordsDetector::startsWord(const PdfCharacter* currChar) const {
  assert(currChar);

  int p = currChar->pos->pageNum;

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
  _log->debug(p) << " └─ activeWord.rotation: " << _activeWord.pos->rotation << endl;
  _log->debug(p) << " └─ char.rotation: " << currChar->pos->rotation << endl;
  if (_activeWord.pos->rotation != currChar->pos->rotation) {
    _log->debug(p) << BLUE << BOLD << " yes → starts word" << OFF << endl;
    return true;
  }

  // ----------------
  // The character starts a new word if it has another writing mode than the active word.

  _log->debug(p) << BLUE << "Does the char have another wMode than activeWord?" << OFF << endl;
  _log->debug(p) << " └─ activeWord.wMode: " << _activeWord.pos->wMode << endl;
  _log->debug(p) << " └─ char.wMode: " << currChar->pos->wMode << endl;
  if (_activeWord.pos->wMode != currChar->pos->wMode) {
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
  _log->debug(p) << " └─ threshold: " << config::Y_OVERLAP_RATIO_THRESHOLD << endl;
  if (ppp::math_utils::smaller(maxYOverlapRatio, config::Y_OVERLAP_RATIO_THRESHOLD)) {
    _log->debug(p) << BLUE << BOLD << " yes → starts word" << OFF << endl;
    return true;
  }

  // ----------------
  // The character starts a new word if the horizontal gap between the character and the previous
  // character is larger than a threshold.

  double hGapLeft = element_utils::computeHorizontalGap(currChar, &_activeWord);
  double hGapRight = element_utils::computeHorizontalGap(&_activeWord, currChar);
  double hGapThreshold = config::getHorizontalGapThreshold(_doc, &_activeWord);

  _log->debug(p) << BLUE << "Are the horizontal gaps between the character and the active word "
      << "larger than a threshold?" << OFF << endl;
  _log->debug(p) << " └─ hGapLeft:  " << hGapLeft << endl;
  _log->debug(p) << " └─ hGapRight: " << hGapRight << endl;
  _log->debug(p) << " └─ threshold: " << hGapThreshold << endl;

  if (ppp::math_utils::larger(hGapLeft, hGapThreshold)) {
    _log->debug(p) << BLUE << BOLD << " yes (hGapLeft) → starts word" << OFF << endl;
    return true;
  }
  if (ppp::math_utils::larger(hGapRight, hGapThreshold)) {
    _log->debug(p) << BLUE << BOLD << " yes (hGapRight) → starts word" << OFF << endl;
    return true;
  }

  _log->debug(p) << BLUE << BOLD << "no rule applied → continues word" << OFF << endl;
  return false;
}

// _________________________________________________________________________________________________
void WordsDetector::mergeStackedMathSymbols(const PdfPage* page) const {
  assert(page);

  int p = page->pageNum;
  double xOverlapRatioThreshold = config::STACKED_MATH_SYMBOL_X_OVERLAP_RATIO_THRESHOLD;

  for (size_t i = 0; i < page->words.size(); i++) {
    PdfWord* word = page->words.at(i);

    _log->debug(p) << "=======================================" << endl;
    _log->debug(p) << BOLD << "word: \"" << word->text << "\"" << OFF << endl;
    _log->debug(p) << " └─ word.page: " << word->pos->pageNum << endl;
    _log->debug(p) << " └─ word.leftX: " << word->pos->leftX << endl;
    _log->debug(p) << " └─ word.upperY: " << word->pos->upperY << endl;
    _log->debug(p) << " └─ word.rightX: " << word->pos->rightX << endl;
    _log->debug(p) << " └─ word.lowerY: " << word->pos->lowerY << endl;
    _log->debug(p) << " └─ word.fontSize: " << word->fontSize << endl;
    if (word->pos->rotation != 0) {
      _log->debug(p) << " └─ word.rotation:  " << word->pos->rotation << endl;
      _log->debug(p) << " └─ word.rotLeftX:  " << word->pos->getRotLeftX() << endl;
      _log->debug(p) << " └─ word.rotUpperY: " << word->pos->getRotUpperY() << endl;
      _log->debug(p) << " └─ word.rotRightX: " << word->pos->getRotRightX() << endl;
      _log->debug(p) << " └─ word.rotLowerY: " << word->pos->getRotLowerY() << endl;
    }

    // Check if the word is the base word of a stacked math symbol.
    bool isBaseOfStackedMathSymbol = false;
    for (auto* ch : word->characters) {
      if (config::STACKED_MATH_CHAR_TEXTS.count(ch->text) > 0) {
        isBaseOfStackedMathSymbol = true;
        break;
      }
      if (config::STACKED_MATH_CHAR_NAMES.count(ch->name) > 0) {
        isBaseOfStackedMathSymbol = true;
        break;
      }
    }
    if (config::STACKED_MATH_WORDS.count(word->text) > 0) {
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
      _log->debug(p) << " └─ prevWord.page: " << prevWord->pos->pageNum << endl;
      _log->debug(p) << " └─ prevWord.leftX: " << prevWord->pos->leftX << endl;
      _log->debug(p) << " └─ prevWord.upperY: " << prevWord->pos->upperY << endl;
      _log->debug(p) << " └─ prevWord.rightX: " << prevWord->pos->rightX << endl;
      _log->debug(p) << " └─ prevWord.lowerY: " << prevWord->pos->lowerY << endl;
      _log->debug(p) << " └─ prevWord.fontSize: " << prevWord->fontSize << endl;

      // The previous word is not a part of the stacked math symbol when the maximum x-overlap
      // ratio is smaller than a threshold.
      double maxXOverlapRatio = element_utils::computeMaxXOverlapRatio(word, prevWord);
      _log->debug(p) << " └─ maxXOverlapRatio: " << maxXOverlapRatio << endl;
      _log->debug(p) << " └─ xOverlapRatioThreshold: " << xOverlapRatioThreshold << endl;
      if (ppp::math_utils::smaller(maxXOverlapRatio, xOverlapRatioThreshold)) {
        _log->debug(p) << BOLD << "is *not* part of the stacked math symbol "
            << "(maxXOverlapRatio < threshold)." << OFF << endl;
        break;
      }

      // The previous word is not a part of the stacked math symbol when its font size is not
      // smaller than the font size of the base word.
      _log->debug(p) << " └─ prevWord.fontSize: " << prevWord->fontSize << endl;
      _log->debug(p) << " └─ word.fontSize:     " << word->fontSize << endl;
      if (!ppp::math_utils::smaller(prevWord->fontSize, word->fontSize, config::FSIZE_EQUAL_TOLERANCE)) {
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
      _log->debug(p) << " └─ nextWord.page: " << nextWord->pos->pageNum << endl;
      _log->debug(p) << " └─ nextWord.leftX: " << nextWord->pos->leftX << endl;
      _log->debug(p) << " └─ nextWord.upperY: " << nextWord->pos->upperY << endl;
      _log->debug(p) << " └─ nextWord.rightX: " << nextWord->pos->rightX << endl;
      _log->debug(p) << " └─ nextWord.lowerY: " << nextWord->pos->lowerY << endl;
      _log->debug(p) << " └─ nextWord.fontSize: " << nextWord->fontSize << endl;

      // The next word is not a part of the stacked math symbol when the maximum x-overlap ratio is
      // smaller than a threshold.
      double maxXOverlapRatio = element_utils::computeMaxXOverlapRatio(word, nextWord);
      _log->debug(p) << " └─ maxXOverlapRatio:          " << maxXOverlapRatio << endl;
      _log->debug(p) << " └─ minXOverlapRatioThreshold: " << xOverlapRatioThreshold << endl;
      if (ppp::math_utils::smaller(maxXOverlapRatio, xOverlapRatioThreshold)) {
        _log->debug(p) << BOLD << "is *not* part of the stacked math symbol "
            << "(maxXOverlapRatio < threshold)." << OFF << endl;
        break;
      }

      // The next word is not a part of the stacked math symbol when its font size is not
      // smaller than the font size of the base word.
      _log->debug(p) << " └─ nextWord.fontSize: " << nextWord->fontSize << endl;
      _log->debug(p) << " └─ word.fontSize:     " << word->fontSize << endl;
      if (!ppp::math_utils::smaller(nextWord->fontSize, word->fontSize, config::FSIZE_EQUAL_TOLERANCE)) {
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
