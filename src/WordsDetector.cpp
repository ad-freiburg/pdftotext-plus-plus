/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // std::min, std::max
#include <limits> // std::numeric_limits

#include "./utils/MathUtils.h"
#include "./utils/Log.h"
#include "./utils/PdfElementsUtils.h"
#include "./utils/StringUtils.h"
#include "./WordsDetector.h"
#include "./PdfDocument.h"

// The inter-glyph space width which will cause the detect() method to start a new word.
#define minWordBreakSpace 0.15

using namespace std;

// =================================================================================================

// _________________________________________________________________________________________________
WordsDetector::WordsDetector(PdfDocument* doc, bool debug, int debugPageFilter) {
  _doc = doc;
  _log = new Logger(debug ? DEBUG : INFO, debugPageFilter);

  _log->debug() << "=======================================" << std::endl;
  _log->debug() << "\033[1mDEBUG MODE | Detecting Words\033[0m" << std::endl;
  _log->debug() << " └─ debug page filter: " << debugPageFilter << std::endl;
}

// _________________________________________________________________________________________________
WordsDetector::~WordsDetector() {
  delete _log;
};

// _________________________________________________________________________________________________
void WordsDetector::detect() {
  // Do nothing if no document is given.
  if (!_doc) {
    return;
  }

  // Do nothing if the document does not contain any pages.
  if (_doc->pages.empty()) {
    return;
  }

  // Iterate through the pages. For each, detect words and merge stacked math symbols.
  for (auto* page : _doc->pages) {
    int p = page->pageNum;
    _log->debug(p) << "=======================================" << std::endl;
    _log->debug(p) << "\033[1mPROCESSING PAGE " << p << "\033[0m" << std::endl;
    _log->debug(p) << " └─ # glyphs: " << page->glyphs.size() << std::endl;

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

  // Do nothing if the page does not contain any glyphs.
  if (page->glyphs.empty()) {
    return;
  }

  int p = page->pageNum;

  _log->debug(p) << "=======================================" << std::endl;
  _log->debug(p) << "\033[1mDETECTING WORDS\033[0m" << std::endl;

  // Iterate through the glyphs of the page in extraction order. For each glyph, decide whether or
  // not the glyph starts a new word by analyzing different layout information.
  PdfGlyph* prevGlyph = nullptr;
  for (auto* glyph : page->glyphs) {
    _log->debug(p) << "---------------------------------------" << std::endl;
    _log->debug(p) << "\033[1mglyph: text:\033[0m \"" << glyph->text << "\""
        << "; \033[1mpage:\033[0m " << glyph->position->pageNum
        << "; \033[1mleftX:\033[0m " << glyph->position->leftX
        << "; \033[1mupperY:\033[0m " << glyph->position->upperY
        << "; \033[1mrightX:\033[0m " << glyph->position->rightX
        << "; \033[1mlowerY:\033[0m " << glyph->position->lowerY << std::endl;
    if (glyph->position->rotation != 0) {
      _log->debug(p) << "\033[1mrot:\033[0m " << glyph->position->rotation
          << "; \033[1mrotLeftX:\033[0m " << glyph->position->getRotLeftX()
          << "; \033[1mrotUpperY:\033[0m " << glyph->position->getRotUpperY()
          << "; \033[1mrotRightX:\033[0m " << glyph->position->getRotRightX()
          << "; \033[1mrotLowerY:\033[0m " << glyph->position->getRotLowerY() << std::endl;
    }

    // Skip diacritic marks that were already merged with their base characters.
    if (glyph->isDiacriticMarkOfBaseGlyph) {
      _log->debug(p) << "\033[1mSkipping glyph (is diacritic mark).\033[0m" << std::endl;
      continue;
    }

    // Check if the glyph starts a new word. If so, create a word from the glyphs collected since
    // the last word delimiter and start a new word.
    if (startsWord(prevGlyph, glyph) && !_currWordGlyphs.empty()) {
      createWord(_currWordGlyphs, &page->words);

      PdfWord* word = page->words[page->words.size() - 1];
      _log->debug(p) << "\033[1mCreated word.\033[0m" << std::endl;
      _log->debug(p) << " └─ word.text: \"" << word->text << "\"" << std::endl;
      _log->debug(p) << " └─ word.pageNum: " << word->position->pageNum << std::endl;
      _log->debug(p) << " └─ word.leftX: " << word->position->leftX << std::endl;
      _log->debug(p) << " └─ word.upperY: " << word->position->upperY << std::endl;
      _log->debug(p) << " └─ word.rightX: " << word->position->rightX << std::endl;
      _log->debug(p) << " └─ word.lowerY: " << word->position->lowerY << std::endl;

      _currWordGlyphs.clear();
      _currWordMinX = numeric_limits<double>::max();
      _currWordMinY = numeric_limits<double>::max();
      _currWordMaxX = numeric_limits<double>::min();
      _currWordMaxY = numeric_limits<double>::min();
      _currWordMaxFontSize = 0;
    }

    // Append the glyph to the current word.
    _currWordGlyphs.push_back(glyph);
    _currWordMinX = min(_currWordMinX, min(glyph->position->leftX, glyph->position->rightX));
    _currWordMinY = min(_currWordMinY, min(glyph->position->lowerY, glyph->position->upperY));
    _currWordMaxX = max(_currWordMaxX, max(glyph->position->leftX, glyph->position->rightX));
    _currWordMaxY = max(_currWordMaxY, max(glyph->position->lowerY, glyph->position->upperY));
    _currWordMaxFontSize = max(_currWordMaxFontSize, glyph->fontSize);

    prevGlyph = glyph;
  }

  if (!_currWordGlyphs.empty()) {
    createWord(_currWordGlyphs, &page->words);

    PdfWord* word = page->words[page->words.size() - 1];
    _log->debug(p) << "\033[1mCreated word.\033[0m" << std::endl;
    _log->debug(p) << " └─ word.text: \"" << word->text << "\"" << std::endl;
    _log->debug(p) << " └─ word.pageNum: " << word->position->pageNum << std::endl;
    _log->debug(p) << " └─ word.leftX: " << word->position->leftX << std::endl;
    _log->debug(p) << " └─ word.upperY: " << word->position->upperY << std::endl;
    _log->debug(p) << " └─ word.rightX: " << word->position->rightX << std::endl;
    _log->debug(p) << " └─ word.lowerY: " << word->position->lowerY << std::endl;

    _currWordGlyphs.clear();
    _currWordMinX = numeric_limits<double>::max();
    _currWordMinY = numeric_limits<double>::max();
    _currWordMaxX = numeric_limits<double>::min();
    _currWordMaxY = numeric_limits<double>::min();
    _currWordMaxFontSize = 0;
  }
}

// _________________________________________________________________________________________________
bool WordsDetector::startsWord(const PdfGlyph* prevGlyph, const PdfGlyph* currGlyph) const {
  if (!currGlyph) {
    return false;
  }

  int p = currGlyph->position->pageNum;

  if (!prevGlyph) {
    _log->debug(p) << "\033[1mstarts new word (no previous glyph).\033[0m" << std::endl;
    return true;
  }

  // ----------------
  // The glyph starts a new word if it has another rotation than the previous glyph.

  _log->debug(p) << "Checking rotations..." << std::endl;
  _log->debug(p) << " └─ prevGlyph.rotation: " << prevGlyph->position->rotation << std::endl;
  _log->debug(p) << " └─ currGlyph.rotation: " << currGlyph->position->rotation << std::endl;
  if (prevGlyph->position->rotation != currGlyph->position->rotation) {
    _log->debug(p) << "\033[1mstarts new word (rotations differ).\033[0m" << std::endl;
    return true;
  }

  // ----------------
  // The glyph starts a new word if it has another writing mode than the previous glyph.

  _log->debug(p) << "Checking writing modes..." << std::endl;
  _log->debug(p) << " └─ prevGlyph.wMode: " << prevGlyph->position->wMode << std::endl;
  _log->debug(p) << " └─ currGlyph.wMode: " << currGlyph->position->wMode << std::endl;
  if (prevGlyph->position->wMode != currGlyph->position->wMode) {
    _log->debug(p) << "\033[1mstarts new word (writing modes differ).\033[0m" << std::endl;
    return true;
  }

  // ----------------
  // The glyph starts a new word if the vertical overlap between the glyph and the current word
  // is less than the half of the glyph's height, and less than the half of the word's height.

  // The y-coordinates of the glyph and the current word, for computing the vertical overlap.
  double glyphMinY, glyphMaxY, wordMinY, wordMaxY;
  // The boundaries of the left and right horizontal gap.
  double hGapLeftMinX, hGapLeftMaxX, hGapRightMinX, hGapRightMaxX;

  // TODO: Is there a more elegant way to compute the properties with respect to the rotation?
  switch(currGlyph->position->rotation) {
    case 0:
    default:
      hGapLeftMinX = _currWordMaxX;
      hGapLeftMaxX = currGlyph->position->leftX;
      hGapRightMinX = currGlyph->position->rightX;
      hGapRightMaxX = _currWordMinX;
      glyphMinY = currGlyph->position->upperY;
      glyphMaxY = currGlyph->position->lowerY;
      wordMinY = _currWordMinY;
      wordMaxY = _currWordMaxY;
      break;
    case 1:
      hGapLeftMinX = _currWordMaxY;
      hGapLeftMaxX = currGlyph->position->upperY;
      hGapRightMinX = currGlyph->position->lowerY;
      hGapRightMaxX = _currWordMinY;
      glyphMinY = currGlyph->position->leftX;
      glyphMaxY = currGlyph->position->rightX;
      wordMinY = _currWordMinX;
      wordMaxY = _currWordMaxX;
      break;
    case 2:
      hGapLeftMinX = currGlyph->position->leftX;
      hGapLeftMaxX = _currWordMinX;
      hGapRightMinX = _currWordMaxX;
      hGapRightMaxX = currGlyph->position->rightX;
      glyphMinY = currGlyph->position->upperY;
      glyphMaxY = currGlyph->position->lowerY;
      wordMinY = _currWordMinY;
      wordMaxY = _currWordMaxY;
      break;
    case 3:
      hGapLeftMinX = currGlyph->position->lowerY;
      hGapLeftMaxX = _currWordMinY;
      hGapRightMinX = _currWordMaxY;
      hGapRightMaxX = currGlyph->position->upperY;
      glyphMinY = currGlyph->position->leftX;
      glyphMaxY = currGlyph->position->rightX;
      wordMinY = _currWordMinX;
      wordMaxY = _currWordMaxX;
      break;
  }

  _log->debug(p) << "Checking vertical overlap with the current word ..." << std::endl;
  _log->debug(p) << " └─ glyph.minY: " << glyphMinY << std::endl;
  _log->debug(p) << " └─ glyph.maxY: " << glyphMaxY << std::endl;
  _log->debug(p) << " └─ word.minY: " << wordMinY << std::endl;
  _log->debug(p) << " └─ word.maxY: " << wordMaxY << std::endl;

  // Compute the overlap ratios. The return value is a pair of doubles. The first double denotes
  // the percentage of the length of the vertical overlap in relation to the glyph's height.
  // The second double denotes denotes the percentage of the length of the vertical overlap in
  // relation to the word's height.
  std::pair<double, double> ratios = element_utils::computeOverlapRatios(glyphMinY, glyphMaxY, wordMinY, wordMaxY);
  _log->debug(p) << " └─ overlapRatios: " << ratios.first << ", " << ratios.second << std::endl;

  if (ratios.first < 0.5 && ratios.second < 0.5) {
    _log->debug(p) << "\033[1mstarts new word (both overlaps are < 0.5).\033[0m" << std::endl;
    return true;
  }

  // ----------------
  // The glyph starts a new word if the horizontal distance between the glyph and the current word
  // is "too large".

  _log->debug(p) << "Checking the horizontal distance to the current word ..." << std::endl;

  double hDistanceLeft = hGapLeftMaxX - hGapLeftMinX;
  double hDistanceRight = hGapRightMaxX - hGapRightMinX;
  _log->debug(p) << " └─ hDistanceLeftMinX: " << hGapLeftMinX << std::endl;
  _log->debug(p) << " └─ hDistanceLeftMaxX: " << hGapLeftMaxX << std::endl;
  _log->debug(p) << " └─ hDistanceLeft: " << hDistanceLeft << std::endl;
  _log->debug(p) << " └─ hDistanceRightMinX: " << hGapRightMinX << std::endl;
  _log->debug(p) << " └─ hDistanceRightMaxX: " << hGapRightMaxX << std::endl;
  _log->debug(p) << " └─ hDistanceRight: " << hDistanceRight << std::endl;
  _log->debug(p) << " └─ word.fontSize: " << _currWordMaxFontSize << std::endl;
  _log->debug(p) << " └─ threshold: " << minWordBreakSpace * _currWordMaxFontSize << std::endl;

  if (hDistanceLeft > minWordBreakSpace * _currWordMaxFontSize) {
    _log->debug(p) << "\033[1mstarts new word (hDistanceLeft too large).\033[0m" << std::endl;
    return true;
  }
  if (hDistanceRight > minWordBreakSpace * _currWordMaxFontSize) {
    _log->debug(p) << "\033[1mstarts new word (hDistanceRight too large).\033[0m" << std::endl;
    return true;
  }

  _log->debug(p) << "\033[1mcontinues word (no rule applied).\033[0m" << std::endl;
  return false;
}

// _________________________________________________________________________________________________
void WordsDetector::mergeStackedMathSymbols(PdfPage* page) const {
  int p = page->pageNum;
  _log->debug(p) << "=======================================" << std::endl;
  _log->debug(p) << "\033[1mMERGING STACKED WORDS\033[0m" << std::endl;
  _log->debug(p) << "# words: " << page->words.size() << std::endl;

  for (size_t i = 0; i < page->words.size(); i++) {
    PdfWord* word = page->words.at(i);

    _log->debug(p) << "---------------------------------------" << std::endl;
    _log->debug(p) << "\033[1mword: text:\033[0m \"" << word->text << "\""
        << "; \033[1mpage:\033[0m " << word->position->pageNum
        << "; \033[1mleftX:\033[0m " << word->position->leftX
        << "; \033[1mupperY:\033[0m " << word->position->upperY
        << "; \033[1mrightX:\033[0m " << word->position->rightX
        << "; \033[1mlowerY:\033[0m " << word->position->lowerY << std::endl;

    // Check if the word is the base word of a stacked math symbol.
    bool isBaseOfStackedMathSymbol = false;
    for (auto* glyph : word->glyphs) {
      if (stackedMathGlyphTexts.count(glyph->text) > 0) {
        isBaseOfStackedMathSymbol = true;
        break;
      }
      if (stackedMathGlyphNames.count(glyph->charName) > 0) {
        isBaseOfStackedMathSymbol = true;
        break;
      }
    }
    if (stackedMathWords.count(word->text) > 0) {
      isBaseOfStackedMathSymbol = true;
    }
    _log->debug(p) << " └─ word.isBaseOfStackedSymbol: " << isBaseOfStackedMathSymbol << std::endl;

    // Skip the word if it is not the base word of a stacked math word.
    if (!isBaseOfStackedMathSymbol) {
      _log->debug(p) << "\033[1mSkipping word (no stacked math symbol).\033[0m" << std::endl;
      continue;
    }

    // Iterate the previous words in reversed order (starting at the current word) for checking
    // if they are also part of the stacked math symbol. Consider a word to be part of the
    // stacked math symbol, if the horizontal overlap between the word word and the base word is
    // large enough, and if the font size of the word is smaller.
    _log->debug(p) << "---------" << std::endl;
    _log->debug(p) << "Searching for prev words that are part of the stacked symbol." << std::endl;
    for (size_t j = i; j --> 0 ;) {
      auto* otherWord = page->words.at(j);

      _log->debug(p) << "\033[1mprev word: text:\033[0m \"" << otherWord->text << "\""
        << "; \033[1mpage:\033[0m " << otherWord->position->pageNum
        << "; \033[1mleftX:\033[0m " << otherWord->position->leftX
        << "; \033[1mupperY:\033[0m " << otherWord->position->upperY
        << "; \033[1mrightX:\033[0m " << otherWord->position->rightX
        << "; \033[1mlowerY:\033[0m " << otherWord->position->lowerY << std::endl;

      std::pair<double, double> xOverlapRatios = element_utils::computeXOverlapRatios(word, otherWord);
      double maxOverlapRatio = std::max(xOverlapRatios.first, xOverlapRatios.second);
      bool isSmallerFontSize = math_utils::smaller(otherWord->fontSize, word->fontSize, 1);

      _log->debug(p) << " └─ word.maxOverlapRatio: " << maxOverlapRatio << std::endl;
      _log->debug(p) << " └─ word.hasSmallerFontSize: " << isSmallerFontSize << std::endl;

      if (maxOverlapRatio < 0.5 || !isSmallerFontSize) {
        _log->debug(p) << "\033[1mNot part of the stacked symbol.\033[0m" << std::endl;
        break;
      }
      word->isBaseOfStackedMathSymbol.push_back(otherWord);
      otherWord->isPartOfStackedMathSymbol = word;

      _log->debug(p) << "\033[1mPart of the stacked symbol.\033[0m" << std::endl;
    }

    // Iterate the next words for checking if they are also part of the stacked math symbol.
    // Consider a word to be part of the stacked math symbol, if the horizontal overlap between
    // the word word and the base word is large enough, and if the font size of the word is
    // smaller.
    _log->debug(p) << "---------" << std::endl;
    _log->debug(p) << "Searching for next words that are part of the stacked symbol." << std::endl;
    for (size_t j = i + 1; j < page->words.size(); j++) {
      auto* otherWord = page->words.at(j);

      _log->debug(p) << "\033[1mnext word: text:\033[0m \"" << otherWord->text << "\""
        << "; \033[1mpage:\033[0m " << otherWord->position->pageNum
        << "; \033[1mleftX:\033[0m " << otherWord->position->leftX
        << "; \033[1mupperY:\033[0m " << otherWord->position->upperY
        << "; \033[1mrightX:\033[0m " << otherWord->position->rightX
        << "; \033[1mlowerY:\033[0m " << otherWord->position->lowerY << std::endl;

      std::pair<double, double> xOverlapRatios = element_utils::computeXOverlapRatios(word, otherWord);
      double maxOverlapRatio = std::max(xOverlapRatios.first, xOverlapRatios.second);
      bool isSmallerFontSize = math_utils::smaller(otherWord->fontSize, word->fontSize, 1);

      _log->debug(p) << " └─ word.maxOverlapRatio: " << maxOverlapRatio << std::endl;
      _log->debug(p) << " └─ word.hasSmallerFontSize: " << isSmallerFontSize << std::endl;

      if (maxOverlapRatio < 0.5 || !isSmallerFontSize) {
        _log->debug(p) << "\033[1mNot part of the stacked symbol.\033[0m" << std::endl;
        break;
      }
      word->isBaseOfStackedMathSymbol.push_back(otherWord);
      otherWord->isPartOfStackedMathSymbol = word;

      _log->debug(p) << "\033[1mPart of the stacked symbol.\033[0m" << std::endl;
    }
  }
}

// _________________________________________________________________________________________________
void WordsDetector::createWord(const std::vector<PdfGlyph*>& glyphs, std::vector<PdfWord*>* words)
    const {
  PdfWord* word = new PdfWord();
  word->id = string_utils::createRandomString(8, "w-");
  word->doc = _doc;

  // Iteratively compute the text, the x,y-coordinates of the bounding box, and the font info.
  std::string text;
  std::unordered_map<std::string, int> fontNameFreqs;
  std::unordered_map<double, int> fontSizeFreqs;
  for (auto* glyph : glyphs) {
    // double glyphMinX = std::min(glyph->position->leftX, glyph->position->rightX);
    // double glyphMinY = std::min(glyph->position->upperY, glyph->position->lowerY);
    // double glyphMaxX = std::max(glyph->position->leftX, glyph->position->rightX);
    // double glyphMaxY = std::max(glyph->position->upperY, glyph->position->lowerY);

    // Update the x,y-coordinates.
    word->position->leftX = std::min(word->position->leftX, glyph->position->leftX);
    word->position->upperY = std::min(word->position->upperY, glyph->position->upperY);
    word->position->rightX = std::max(word->position->rightX, glyph->position->rightX);
    word->position->lowerY = std::max(word->position->lowerY, glyph->position->lowerY);

    // Compose the text. If the glyph was merged with a diacritic mark, append the text with the
    // diacitic mark. If the glyph is a diacritic mark which was merged with a base glyph, ignore
    // its text. Otherwise, append the normal text.
    if (glyph->isBaseGlyphOfDiacriticMark) {
      text += glyph->textWithDiacriticMark;
    } else if (!glyph->isDiacriticMarkOfBaseGlyph) {
      text += glyph->text;
    }

    // Count the font names and font sizes, for computing the most frequent font name / font size.
    fontNameFreqs[glyph->fontName]++;
    fontSizeFreqs[glyph->fontSize]++;

    glyph->word = word;
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
  word->position->pageNum = glyphs[0]->position->pageNum;

  // Set the writing mode.
  word->position->wMode = glyphs[0]->position->wMode;

  // Set the rotation value.
  word->position->rotation = glyphs[0]->position->rotation;

  // Set the rank.
  word->rank = words->size();

  // Set the glyphs.
  word->glyphs = glyphs;

  words->push_back(word);
}
