/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cassert>  // assert
#include <iostream>  // std::endl

#include "./Types.h"
#include "./WordsStatisticsCalculation.h"
#include "./utils/Counter.h"
#include "./utils/Log.h"
#include "./utils/MathUtils.h"
#include "./utils/PdfElementsUtils.h"

using std::endl;

using ppp::config::WordsStatisticsCalculationConfig;
using ppp::types::PdfDocument;
using ppp::types::PdfWord;
using ppp::utils::counter::DoubleCounter;
using ppp::utils::elements::computeHorizontalGap;
using ppp::utils::elements::computeMaxYOverlapRatio;
using ppp::utils::elements::computeVerticalGap;
using ppp::utils::log::BOLD;
using ppp::utils::log::OFF;
using ppp::utils::log::Logger;
using ppp::utils::math::equal;
using ppp::utils::math::equalOrLarger;
using ppp::utils::math::equalOrSmaller;
using ppp::utils::math::smaller;

// =================================================================================================

namespace ppp::modules {

// _________________________________________________________________________________________________
WordsStatisticsCalculation::WordsStatisticsCalculation(
    PdfDocument* doc,
    const WordsStatisticsCalculationConfig* config) {
  _doc = doc;
  _config = config;
  _log = new Logger(config->logLevel, config->logPageFilter);
}

// _________________________________________________________________________________________________
WordsStatisticsCalculation::~WordsStatisticsCalculation() {
  delete _log;
}

// _________________________________________________________________________________________________
void WordsStatisticsCalculation::process() const {
  assert(_doc);

  double minYOverlapRatioSameLine = _config->minYOverlapRatioSameLine;
  double maxYOverlapRatioDiffLine = _config->maxYOverlapRatioDifferentLine;

  _log->info() << "Calculating word statistics..." << endl;
  _log->debug() << "=======================================" << endl;
  _log->debug() << BOLD << "DEBUG MODE" << OFF << endl;
  _log->debug() << " └─ minYOverlapRatioSameLine: " << minYOverlapRatioSameLine << endl;
  _log->debug() << " └─ maxYOverlapRatioDifferentLine: " << maxYOverlapRatioDiffLine << endl;
  _log->debug() << "=======================================" << endl;

  // A counter for the horizontal gaps between two consecutive words that overlap vertically.
  DoubleCounter horizontalGapCounter;
  // A counter for the vertical gaps between two consecutive words that don't overlap vertically.
  DoubleCounter verticalGapCounter;
  // A counter for the word heights.
  DoubleCounter wordHeightCounter;

  for (const auto* page : _doc->pages) {
    for (size_t i = 0; i < page->words.size(); i++) {
      PdfWord* prevWord = i > 0 ? page->words[i - 1] : nullptr;
      PdfWord* word = page->words[i];

      // Skip the word if its font size is smaller than the most frequent font size.
      if (smaller(word->fontSize, _doc->mostFreqFontSize, _config->fsEqualTolerance)) {
        continue;
      }

      // Count the word height.
      wordHeightCounter[word->pos->getHeight()]++;

      // Skip to the next word if there is no previous word.
      if (!prevWord) {
        continue;
      }

      // Skip to the next word if the word does not have the same rotation than the previous word.
      if (prevWord->pos->rotation != word->pos->rotation) {
        continue;
      }

      // Skip to the next word if the word does not have the same writing mode than the prev word.
      if (prevWord->pos->wMode != word->pos->wMode) {
        continue;
      }

      // Skip to the next word if the font size of the previous word is not equal to the most
      // frequent font size.
      if (!equal(prevWord->fontSize, _doc->mostFreqFontSize, _config->fsEqualTolerance)) {
        continue;
      }

      double maxYOverlapRatio = computeMaxYOverlapRatio(prevWord, word);

      // Add the horizontal gap between the previous word and the current word to the counter,
      // when one word vertically overlaps at least the half of the height of the other word.
      if (equalOrLarger(maxYOverlapRatio, minYOverlapRatioSameLine)) {
        double gap = computeHorizontalGap(prevWord, word);
        horizontalGapCounter[gap]++;
      }

      // Add the vertical gap between the previous word and the current word to the counter, when
      // they do *not* vertically overlap.
      if (equalOrSmaller(maxYOverlapRatio, maxYOverlapRatioDiffLine)) {
        double gap = computeVerticalGap(prevWord, word);
        verticalGapCounter[gap]++;
      }
    }
  }

  if (wordHeightCounter.sumCounts() > 0) {
    _doc->mostFreqWordHeight = wordHeightCounter.mostFreq();
  }

  if (horizontalGapCounter.sumCounts() > 0) {
    _doc->mostFreqWordDistance = horizontalGapCounter.mostFreq();
  }

  if (verticalGapCounter.sumCounts() > 0) {
    _doc->mostFreqEstimatedLineDistance = verticalGapCounter.mostFreq();
  }

  _log->debug() << "doc.mostFreqWordHeight: " << _doc->mostFreqWordHeight << endl;
  _log->debug() << "doc.mostFreqWordDistance: " << _doc->mostFreqWordDistance << endl;
  _log->debug() << "doc.mostFreqEstimatedLineDist: " << _doc->mostFreqEstimatedLineDistance << endl;
  _log->debug() << "=======================================" << endl;
}

}  // namespace ppp::modules
