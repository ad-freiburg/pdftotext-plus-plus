/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef WORDSDETECTION_H_
#define WORDSDETECTION_H_

#include "./Config.h"
#include "./PdfDocument.h"
#include "./utils/Log.h"
#include "./utils/WordsDetectionUtils.h"

using ppp::config::WordsDetectionConfig;
using ppp::types::PdfCharacter;
using ppp::types::PdfDocument;
using ppp::types::PdfPage;
using ppp::types::PdfWord;
using ppp::utils::WordsDetectionUtils;
using ppp::utils::log::Logger;

// =================================================================================================

namespace ppp::modules {

/**
 * This class is responsible for (a) merging the characters of a PDF document to words, and
 * (b) merging words that are part of the same "stacked math symbol".
 *
 * NOTE: Stacked math symbols are math symbols that are positioned one above the other and
 * logically belong together. A stacked math symbol consists of a base word and one or more
 * additional words. A typical example is the summation symbol, which can contain limits that are
 * positioned below and above the summation symbol: ∑_{i=0}^{n}. The "∑" is the base word, the
 * "i=0" and "n" are the additional words. Without merging, "i=0" and/or "n" would be treated as
 * separate words (e.g., because they do not overlap vertically with the base word). A possible
 * consequence is that the additional words are detected as part of different text lines than the
 * base word (which is of course not the expected output).
 */
class WordsDetection {
 public:
  /**
   * The default constructor.
   *
   * @param doc
   *   The PDF document to process.
   * @param config
   *   The configuration to use.
   */
  WordsDetection(PdfDocument* doc, const WordsDetectionConfig& config);

  /** The deconstructor. */
  ~WordsDetection();

  /**
   * This method detects the words and merges stacked math symbols. It iterates through the pages
   * of the given document and invokes the detectWords() and mergeStackedMathSymbols() methods
   * below for each page.
   */
  void process();

 private:
  /**
   * This method detects the words from the characters of the given page, by using the following
   * iterative process: the characters stored in `page->characters` are iterated. For each
   * character C, it is decided whether or not C starts a new word, by analyzing different layout
   * information. If C doesn't start a new word, it is added to _activeWord.characters. If it
   * starts a word, a new `PdfWord` instance (consisting of the characters in
   * _activeWord.characters) is created and added to page.words. Afterwards, _activeWord is
   * resetted, C is added to _activeWord.characters and the next character C' is processed.
   *
   * @param page
   *    The page to process.
   */
  void detectWords(PdfPage* page);

  /**
   * This method returns true if the given character starts a new word, false otherwise. This
   * decision is made based on analyzing different layout information (e.g., the horizontal
   * gap between the active word and the given character).
   *
   * @param currChar
   *   The character for which to decide whether or not it starts a new word.
   *
   * @return
   *    True if the given character starts a new word, false otherwise.
   */
  bool startsWord(const PdfCharacter* currChar) const;

  /**
   * This method merges stacked math symbols of the given page. The basic procedure is as follows.
   * The words stored in `page->words` are iterated. For each word, it is checked whether it
   * denotes the base word of a stacked math symbol (by looking up the characters of the word, and
   * the word itself in the STACKED_MATH*-sets above). If so, it checks which of the previous and
   * next words in page->words overlap the base word horizontally. All words that horizontally
   * overlap the base word are considered to be additional words of the stacked math symbol.
   * Let `base` the base word of a stacked math symbol and `other` be a word that is considered to
   * be an additional word of the stacked math symbol. The actual merging is realized as follows:
   *  - `other` is added to `base->isBaseOfStackedMathSymbol` (which is a vector),
   *  - `other->isPartOfStackedWord` is set to `base`.
   *
   * NOTE: The words that are merged with the base character are *not* removed from `page->words`.
   * If you want to exclude such words from further processing, you need to check whether or not
   * `word->isPartOfStackedWord` is set.
   *
   * @param page
   *    The page to process.
   */
  void mergeStackedMathSymbols(const PdfPage* page) const;

  // The document to process.
  PdfDocument* _doc;
  // The configuration to use.
  WordsDetectionConfig _config;
  // The words detection utils.
  WordsDetectionUtils* _utils;
  // The logger.
  Logger* _log;

  // The active word.
  PdfWord _activeWord;
};

}  // namespace ppp::modules

#endif  // WORDSDETECTION_H_
