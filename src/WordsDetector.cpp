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
#include <unordered_map>  // unordered_map
#include <utility>  // pair
#include <vector>  // vector

#include "./utils/Log.h"
#include "./utils/MathUtils.h"
#include "./utils/PdfElementsUtils.h"
#include "./utils/StringUtils.h"

#include "./PdfDocument.h"
#include "./WordsDetector.h"

// TODO(korzen): The inter-char space width which will cause the detect() method to start a word.
#define minWordBreakSpace 0.15

using std::endl;
using std::max;
using std::min;
using std::pair;
using std::string;
using std::unordered_map;
using std::vector;

// =================================================================================================

// _________________________________________________________________________________________________
WordsDetector::WordsDetector(PdfDocument* doc, bool debug, int debugPageFilter) {
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

  _log->debug() << BOLD <<"Detecting Words - DEBUG MODE" << OFF << endl;

  // Do nothing if the document does not contain any pages.
  if (_doc->pages.empty()) {
    return;
  }

  // Iterate through the pages. For each, detect words and merge stacked math symbols.
  for (auto* page : _doc->pages) {
    int p = page->pageNum;
    _log->debug(p) << "=======================================" << endl;
    _log->debug(p) << BOLD << "PROCESSING PAGE " << p << OFF << endl;
    _log->debug(p) << " └─ # characters: " << page->characters.size() << endl;

    detectWords(page);
    mergeStackedMathSymbols(page);
  }
}

// _________________________________________________________________________________________________
void WordsDetector::detectWords(PdfPage* page) {
  // Do nothing if no page is given.
  if (!page) {
    return;
  }

  // Do nothing if the page does not contain any characters.
  if (page->characters.empty()) {
    return;
  }

  int p = page->pageNum;

  _log->debug(p) << "=======================================" << endl;
  _log->debug(p) << BOLD << "DETECTING WORDS" << OFF << endl;

  // Iterate through the characters of the page in extraction order. For each character, decide
  // whether or not the character starts a new word by analyzing different layout information.
  PdfCharacter* prevChar = nullptr;
  for (auto* currChar : page->characters) {
    _log->debug(p) << "---------------------------------------" << endl;
    _log->debug(p) << BOLD << "Char: \"" << currChar->text << "\"" << OFF << endl;
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

    // Skip diacritic marks that were already merged with their base characters.
    if (currChar->isDiacriticMarkOfBaseChar) {
      _log->debug(p) << BOLD << "Skipping char (is merged diacritic mark)." << endl;
      continue;
    }

    // Check if the char starts a new word. If so, create a word from the chars collected since
    // the last word delimiter and start a new word.
    if (startsWord(prevChar, currChar) && !_currWordChars.empty()) {
      createWord(_currWordChars, &page->words);

      PdfWord* word = page->words[page->words.size() - 1];
      _log->debug(p) << BOLD << "Created word: \"" << word->text << "\"" << OFF << endl;
      _log->debug(p) << " └─ word.page: " << word->position->pageNum << endl;
      _log->debug(p) << " └─ word.leftX: " << word->position->leftX << endl;
      _log->debug(p) << " └─ word.upperY: " << word->position->upperY << endl;
      _log->debug(p) << " └─ word.rightX: " << word->position->rightX << endl;
      _log->debug(p) << " └─ word.lowerY: " << word->position->lowerY << endl;

      _currWordChars.clear();
      _currWordMinX = numeric_limits<double>::max();
      _currWordMinY = numeric_limits<double>::max();
      _currWordMaxX = numeric_limits<double>::min();
      _currWordMaxY = numeric_limits<double>::min();
      _currWordMaxFontSize = 0;
    }

    // Append the char to the current word and recompute the bounding box.
    _currWordChars.push_back(currChar);
    _currWordMinX = min(_currWordMinX, min(currChar->position->leftX, currChar->position->rightX));
    _currWordMinY = min(_currWordMinY, min(currChar->position->lowerY, currChar->position->upperY));
    _currWordMaxX = max(_currWordMaxX, max(currChar->position->leftX, currChar->position->rightX));
    _currWordMaxY = max(_currWordMaxY, max(currChar->position->lowerY, currChar->position->upperY));
    _currWordMaxFontSize = max(_currWordMaxFontSize, currChar->fontSize);

    prevChar = currChar;
  }

  // Don't forget to process the last word.
  if (!_currWordChars.empty()) {
    createWord(_currWordChars, &page->words);

    PdfWord* word = page->words[page->words.size() - 1];
    _log->debug(p) << BOLD << "Created word: \"" << word->text << "\"" << OFF << endl;
    _log->debug(p) << " └─ word.page: " << word->position->pageNum << endl;
    _log->debug(p) << " └─ word.leftX: " << word->position->leftX << endl;
    _log->debug(p) << " └─ word.upperY: " << word->position->upperY << endl;
    _log->debug(p) << " └─ word.rightX: " << word->position->rightX << endl;
    _log->debug(p) << " └─ word.lowerY: " << word->position->lowerY << endl;

    _currWordChars.clear();
    _currWordMinX = numeric_limits<double>::max();
    _currWordMinY = numeric_limits<double>::max();
    _currWordMaxX = numeric_limits<double>::min();
    _currWordMaxY = numeric_limits<double>::min();
    _currWordMaxFontSize = 0;
  }
}

// _________________________________________________________________________________________________
bool WordsDetector::startsWord(const PdfCharacter* prevChar, const PdfCharacter* currChar) const {
  if (!currChar) {
    return false;
  }

  int p = currChar->position->pageNum;

  _log->debug(p) << BLUE << "Does the char has no previous char?" << OFF << endl;
  _log->debug(p) << " └─ char.prevChar: " << (prevChar ? prevChar->text : "-") << endl;

  // The char starts a new word if it has no previous char.
  if (!prevChar) {
    _log->debug(p) << BLUE << BOLD << " yes → starts block" << OFF << endl;
    return true;
  }

  // ----------------
  // The character starts a new word if it has another rotation than the previous character.

  _log->debug(p) << BLUE << "Does the char have another rotation than prev char?" << OFF << endl;
  _log->debug(p) << " └─ prevChar.rotation: " << prevChar->position->rotation << endl;
  _log->debug(p) << " └─ currChar.rotation: " << currChar->position->rotation << endl;
  if (prevChar->position->rotation != currChar->position->rotation) {
    _log->debug(p) << BLUE << BOLD << " yes → starts word" << OFF << endl;
    return true;
  }

  // ----------------
  // The char starts a new word if it has another writing mode than the previous char.

  _log->debug(p) << BLUE << "Does the char have another wMode than prev char?" << OFF << endl;
  _log->debug(p) << " └─ prevChar.wMode: " << prevChar->position->wMode << endl;
  _log->debug(p) << " └─ currChar.wMode: " << currChar->position->wMode << endl;
  if (prevChar->position->wMode != currChar->position->wMode) {
    _log->debug(p) << BLUE << BOLD << " yes → starts word" << OFF << endl;
    return true;
  }

  // ----------------
  // The char starts a new word if the vertical overlap between the char and the current word
  // is less than the half of the char's height, and less than the half of the word's height.

  // The y-coordinates of the char and the current word, for computing the vertical overlap.
  double charMinY, charMaxY, wordMinY, wordMaxY;
  // The boundaries of the left and right horizontal gap.
  double hGapLeftMinX, hGapLeftMaxX, hGapRightMinX, hGapRightMaxX;

  // TODO(korzen): Is there a more elegant way to compute the properties wrt the rotation?
  switch (currChar->position->rotation) {
    case 0:
    default:
      hGapLeftMinX = _currWordMaxX;
      hGapLeftMaxX = currChar->position->leftX;
      hGapRightMinX = currChar->position->rightX;
      hGapRightMaxX = _currWordMinX;
      charMinY = currChar->position->upperY;
      charMaxY = currChar->position->lowerY;
      wordMinY = _currWordMinY;
      wordMaxY = _currWordMaxY;
      break;
    case 1:
      hGapLeftMinX = _currWordMaxY;
      hGapLeftMaxX = currChar->position->upperY;
      hGapRightMinX = currChar->position->lowerY;
      hGapRightMaxX = _currWordMinY;
      charMinY = currChar->position->leftX;
      charMaxY = currChar->position->rightX;
      wordMinY = _currWordMinX;
      wordMaxY = _currWordMaxX;
      break;
    case 2:
      hGapLeftMinX = currChar->position->leftX;
      hGapLeftMaxX = _currWordMinX;
      hGapRightMinX = _currWordMaxX;
      hGapRightMaxX = currChar->position->rightX;
      charMinY = currChar->position->upperY;
      charMaxY = currChar->position->lowerY;
      wordMinY = _currWordMinY;
      wordMaxY = _currWordMaxY;
      break;
    case 3:
      hGapLeftMinX = currChar->position->lowerY;
      hGapLeftMaxX = _currWordMinY;
      hGapRightMinX = _currWordMaxY;
      hGapRightMaxX = currChar->position->upperY;
      charMinY = currChar->position->leftX;
      charMaxY = currChar->position->rightX;
      wordMinY = _currWordMinX;
      wordMaxY = _currWordMaxX;
      break;
  }

  _log->debug(p) << BLUE << "Is the vertical overlap between the char and the current word less "
      "than the half of the char's height and word's height?" << endl;
  _log->debug(p) << " └─ char.minY: " << charMinY << endl;
  _log->debug(p) << " └─ char.maxY: " << charMaxY << endl;
  _log->debug(p) << " └─ word.minY: " << wordMinY << endl;
  _log->debug(p) << " └─ word.maxY: " << wordMaxY << endl;

  // Compute the overlap ratios between the char and the current word. The return value is a pair
  // of doubles. The first double denotes the percentage of the length of the vertical overlap in
  // relation to the char's height. The second double denotes denotes the percentage of the length
  // of the vertical overlap in relation to the word's height.
  pair<double, double> ratios = element_utils::computeOverlapRatios(
      charMinY, charMaxY, wordMinY, wordMaxY);
  _log->debug(p) << " └─ overlapRatios: " << ratios.first << ", " << ratios.second << endl;

  if (ratios.first < 0.5 && ratios.second < 0.5) {
    _log->debug(p) << BLUE << BOLD << " yes → starts block" << OFF << endl;
    return true;
  }

  // ----------------
  // The char starts a new word if the horizontal distance between the char and the current word
  // is "too large".

  _log->debug(p) << BLUE << "Is the horiz. gap between the word and the char too large?" << endl;

  double hDistanceLeft = hGapLeftMaxX - hGapLeftMinX;
  double hDistanceRight = hGapRightMaxX - hGapRightMinX;
  _log->debug(p) << " └─ hDistanceLeftMinX: " << hGapLeftMinX << endl;
  _log->debug(p) << " └─ hDistanceLeftMaxX: " << hGapLeftMaxX << endl;
  _log->debug(p) << " └─ hDistanceLeft: " << hDistanceLeft << endl;
  _log->debug(p) << " └─ hDistanceRightMinX: " << hGapRightMinX << endl;
  _log->debug(p) << " └─ hDistanceRightMaxX: " << hGapRightMaxX << endl;
  _log->debug(p) << " └─ hDistanceRight: " << hDistanceRight << endl;
  _log->debug(p) << " └─ word.fontSize: " << _currWordMaxFontSize << endl;
  _log->debug(p) << " └─ threshold: " << minWordBreakSpace * _currWordMaxFontSize << endl;

  if (hDistanceLeft > minWordBreakSpace * _currWordMaxFontSize) {
    _log->debug(p) << BLUE << BOLD << " yes → starts block" << OFF << endl;
    return true;
  }
  if (hDistanceRight > minWordBreakSpace * _currWordMaxFontSize) {
    _log->debug(p) << BLUE << BOLD << " yes → starts block" << OFF << endl;
    return true;
  }

  _log->debug(p) << BLUE << BOLD << "continues word (no rule applied)." << OFF << endl;
  return false;
}

// _________________________________________________________________________________________________
void WordsDetector::mergeStackedMathSymbols(PdfPage* page) const {
  int p = page->pageNum;
  _log->debug(p) << "=======================================" << endl;
  _log->debug(p) << BOLD << "MERGING STACKED WORDS" << OFF << endl;
  _log->debug(p) << " └─ # words: " << page->words.size() << endl;

  for (size_t i = 0; i < page->words.size(); i++) {
    PdfWord* word = page->words.at(i);

    _log->debug(p) << "---------------------------------------" << endl;
    _log->debug(p) << BOLD << "Word: \"" << word->text << "\"" << OFF << endl;
    _log->debug(p) << " └─ word.page: " << word->position->pageNum << endl;
    _log->debug(p) << " └─ word.leftX: " << word->position->leftX << endl;
    _log->debug(p) << " └─ word.upperY: " << word->position->upperY << endl;
    _log->debug(p) << " └─ word.rightX: " << word->position->rightX << endl;
    _log->debug(p) << " └─ word.lowerY: " << word->position->lowerY << endl;

    // Check if the word is the base word of a stacked math symbol.
    bool isBaseOfStackedMathSymbol = false;
    for (auto* ch : word->characters) {
      if (stackedMathCharTexts.count(ch->text) > 0) {
        isBaseOfStackedMathSymbol = true;
        break;
      }
      if (stackedMathCharNames.count(ch->name) > 0) {
        isBaseOfStackedMathSymbol = true;
        break;
      }
    }
    if (stackedMathWords.count(word->text) > 0) {
      isBaseOfStackedMathSymbol = true;
    }
    _log->debug(p) << " └─ word.isBaseOfStackedSymbol: " << isBaseOfStackedMathSymbol << endl;

    // Skip the word if it is not the base word of a stacked math word.
    if (!isBaseOfStackedMathSymbol) {
      _log->debug(p) << BOLD << "Skipping word (no stacked math symbol)." << OFF << endl;
      continue;
    }

    // Iterate the previous words in reversed order (starting at the current word) for checking
    // if they are also part of the stacked math symbol. Consider a word to be part of the
    // stacked math symbol, if the horizontal overlap between the word word and the base word is
    // large enough, and if the font size of the word is smaller.
    _log->debug(p) << "---------" << endl;
    _log->debug(p) << "Searching for prev words that are part of the stacked symbol." << endl;
    for (size_t j = i; j --> 0 ;) {
      auto* otherWord = page->words.at(j);

      _log->debug(p) << BOLD << "prevWord: \"" << otherWord->text << "\"" << OFF << endl;
      _log->debug(p) << " └─ prevWord.page: " << otherWord->position->pageNum << endl;
      _log->debug(p) << " └─ prevWord.leftX: " << otherWord->position->leftX << endl;
      _log->debug(p) << " └─ prevWord.upperY: " << otherWord->position->upperY << endl;
      _log->debug(p) << " └─ prevWord.rightX: " << otherWord->position->rightX << endl;
      _log->debug(p) << " └─ prevWord.lowerY: " << otherWord->position->lowerY << endl;

      pair<double, double> xOverlapRatios =
          element_utils::computeXOverlapRatios(word, otherWord);
      double maxOverlapRatio = max(xOverlapRatios.first, xOverlapRatios.second);
      bool isSmallerFontSize = math_utils::smaller(otherWord->fontSize, word->fontSize, 1);

      _log->debug(p) << " └─ word.maxOverlapRatio: " << maxOverlapRatio << endl;
      _log->debug(p) << " └─ word.hasSmallerFontSize: " << isSmallerFontSize << endl;

      if (maxOverlapRatio < 0.5 || !isSmallerFontSize) {
        _log->debug(p) << BOLD << "Not part of the stacked symbol." << OFF << endl;
        break;
      }
      word->isBaseOfStackedMathSymbol.push_back(otherWord);
      otherWord->isPartOfStackedMathSymbol = word;

      _log->debug(p) << BOLD << "Part of the stacked symbol." << OFF << endl;
    }

    // Iterate the next words for checking if they are also part of the stacked math symbol.
    // Consider a word to be part of the stacked math symbol, if the horizontal overlap between
    // the word word and the base word is large enough, and if the font size of the word is
    // smaller.
    _log->debug(p) << "---------" << endl;
    _log->debug(p) << "Searching for next words that are part of the stacked symbol." << endl;
    for (size_t j = i + 1; j < page->words.size(); j++) {
      auto* otherWord = page->words.at(j);

      _log->debug(p) << BOLD << "nextWord: \"" << otherWord->text << "\"" << OFF << endl;
      _log->debug(p) << " └─ nextWord.page: " << otherWord->position->pageNum << endl;
      _log->debug(p) << " └─ nextWord.leftX: " << otherWord->position->leftX << endl;
      _log->debug(p) << " └─ nextWord.upperY: " << otherWord->position->upperY << endl;
      _log->debug(p) << " └─ nextWord.rightX: " << otherWord->position->rightX << endl;
      _log->debug(p) << " └─ nextWord.lowerY: " << otherWord->position->lowerY << endl;

      pair<double, double> xOverlapRatios =
          element_utils::computeXOverlapRatios(word, otherWord);
      double maxOverlapRatio = max(xOverlapRatios.first, xOverlapRatios.second);
      bool isSmallerFontSize = math_utils::smaller(otherWord->fontSize, word->fontSize, 1);

      _log->debug(p) << " └─ word.maxOverlapRatio: " << maxOverlapRatio << endl;
      _log->debug(p) << " └─ word.hasSmallerFontSize: " << isSmallerFontSize << endl;

      if (maxOverlapRatio < 0.5 || !isSmallerFontSize) {
        _log->debug(p) << BOLD << "Not part of the stacked symbol." << OFF << endl;
        break;
      }
      word->isBaseOfStackedMathSymbol.push_back(otherWord);
      otherWord->isPartOfStackedMathSymbol = word;

      _log->debug(p) << BOLD << "Part of the stacked symbol." << OFF << endl;
    }
  }
}

// _________________________________________________________________________________________________
void WordsDetector::createWord(const vector<PdfCharacter*>& chars,
    vector<PdfWord*>* words) const {
  PdfWord* word = new PdfWord();
  word->id = string_utils::createRandomString(8, "w-");
  word->doc = _doc;

  // Iteratively compute the text, the x,y-coordinates of the bounding box, and the font info.
  string text;
  unordered_map<string, int> fontNameFreqs;
  unordered_map<double, int> fontSizeFreqs;
  for (auto* ch : chars) {
    // double charMinX = min(char->position->leftX, char->position->rightX);
    // double charMinY = min(char->position->upperY, char->position->lowerY);
    // double charMaxX = max(char->position->leftX, char->position->rightX);
    // double charMaxY = max(char->position->upperY, char->position->lowerY);

    // Update the x,y-coordinates.
    word->position->leftX = min(word->position->leftX, ch->position->leftX);
    word->position->upperY = min(word->position->upperY, ch->position->upperY);
    word->position->rightX = max(word->position->rightX, ch->position->rightX);
    word->position->lowerY = max(word->position->lowerY, ch->position->lowerY);

    // Compose the text. If the char was merged with a diacritic mark, append the text with the
    // diacitic mark. If the char is a diacritic mark which was merged with a base char, ignore
    // its text. Otherwise, append the normal text.
    if (ch->isBaseCharOfDiacriticMark) {
      text += ch->textWithDiacriticMark;
    } else if (!ch->isDiacriticMarkOfBaseChar) {
      text += ch->text;
    }

    // Count the font names and font sizes, for computing the most frequent font name / font size.
    fontNameFreqs[ch->fontName]++;
    fontSizeFreqs[ch->fontSize]++;

    ch->word = word;
  }

  // Set the text.
  word->text = text;

  // Compute and set the most frequent font name.
  int mostFreqFontNameCount = 0;
  for (const auto& pair : fontNameFreqs) {
    if (pair.second > mostFreqFontNameCount) {
      word->fontName = pair.first;
      mostFreqFontNameCount = pair.second;
    }
  }

  // Compute and set the most frequent font size.
  int mostFreqFontSizeCount = 0;
  for (const auto& pair : fontSizeFreqs) {
    if (pair.second > mostFreqFontSizeCount) {
      word->fontSize = pair.first;
      mostFreqFontSizeCount = pair.second;
    }
  }

  // Set the page number.
  word->position->pageNum = chars[0]->position->pageNum;

  // Set the writing mode.
  word->position->wMode = chars[0]->position->wMode;

  // Set the rotation value.
  word->position->rotation = chars[0]->position->rotation;

  // Set the rank.
  word->rank = words->size();

  // Set the chars.
  word->characters = chars;

  words->push_back(word);
}
