/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef WORDSDETECTOR_H_
#define WORDSDETECTOR_H_

#include <limits>  // std::numeric_limits
#include <string>
#include <unordered_set>
#include <vector>

#include "./utils/Log.h"

#include "./PdfDocument.h"

using std::string;
using std::unordered_set;

// =================================================================================================

// A PDF can contain "stacked math symbols", which we want to merge to a single word (see the
// preliminary comment of the `WordsDetector` class below for more information). The following
// three sets are used to identify the base word of a stacked math symbol.
// The first set contains the text of characters that are likely to be part of a base word of a
// stacked math symbol. If a word contains a character that is part of this set, it is considered
// as the base word of a stacked math symbol.
// The second set contains the names of characters that are likely to be part of a base word of a
// stacked math symbol. If a word contains a character with a name that is part of this set, it is
// considered as the base word of a stacked math symbol (NOTE: this set was introduced because, in
// some PDFs, the text of summation symbols do not contain a summation symbol, but some
// weird symbols (e.g., a "?"), because of a missing encoding. The names of the characters are
// simply an additional indicator for identifying the base word of a stacked math symbol).
// The third set contains *words* that are likely to be the base part of stacked math symbols.
static const unordered_set<string> stackedMathCharTexts = { "∑", "∏", "∫", "⊗" };
static const unordered_set<string> stackedMathCharNames = { "summationdisplay",
    "productdisplay", "integraldisplay", "circlemultiplydisplay" };
static const unordered_set<string> stackedMathWords = { "sup", "lim" };

// =================================================================================================

/**
 * This class has the following two purposes:
 *  (a) merging the characters extracted from a PDF document to words,
 *  (b) merging words that are part of the same "stacked math symbols".
 *
 * NOTE: Stacked math symbols are math symbols that are positioned one above the other and logically
 * belong together. A stacked math symbol consist of a base word and one or more additional words.
 * A typical example is the summation symbol, which can contain indices that are positioned below
 * and above the summation symbol: ∑_{i=0}^{n}. The "∑" is the base word, the "i=0" and "n" are the
 * additional words. Without merging, "i=0" and/or "n" would be treated as separate words (because
 * they do not overlap vertically with the base word). A possible consequence is that the
 * additional words are detected as a part of a different text line than the base word (which is
 * not the expected output).
 */
class WordsDetector {
 public:
  /**
   * This constructor creates and initializes a new instance of this class.
   *
   * @param doc
   *   The PDF document to process.
   * @param debug
   *   Whether or not this instance should print debug information to the console.
   * @param debugPageFilter
   *   If set to a value > 0, only the debug messages produced while processing the
   *   <debugPageFilter>-th page of the current PDF file will be printed to the console.
   */
  explicit WordsDetector(PdfDocument* doc, bool debug = false, int debugPageFilter = -1);

  /** The deconstructor. */
  ~WordsDetector();

  /**
   * This method detects the words and merges stacked math symbols. It iterates through the pages
   * of the given document and invokes the detectWords() and mergeStackedMathSymbols() methods
   * below for each page. More information about each method is given in the comment of the
   * respective method below.
   */
  void process();

 private:
  /**
   * This method detects the words in the given page. The basic procedure is as follows. It
   * iterates through the characters stored in `page->characters` in extraction order. For each
   * character, it decides whether or not the character starts a new word, by analyzing different
   * layout information of the characters. The detected words from the i-th page are appended to
   * `_doc->pages[i]->words`.
   *
   * @param page
   *    The page to process.
   */
  void detectWords(PdfPage* page);

  /**
   * This method returns true if the given current character starts a new word, false otherwise.
   * This decision is made based on analyzing different layout information of the characters.
   *
   * @param prevChar
   *   The previously processed character, if such exists; nullptr otherwise.
   * @param currChar
   *   The character for which to decide whether or not it starts a new word.
   *
   * @return
   *    True if the given current character starts a new word, false otherwise.
   */
  bool startsWord(const PdfCharacter* prevChar, const PdfCharacter* currChar) const;

  /**
   * This method merges stacked math symbols in the given page. It iterates through the words
   * stored in `page->words`. For each word, it checks whether it denotes the base word of a
   * stacked math symbol (by looking up the characters of the word, and the word itself in the
   * stackedMath*-sets above). If so, it checks whether the adjacents words horizontally overlap
   * the base word. If so, they are considered to be additional words of the stacked math symbol.
   * Let `base` the base word of a stacked math symbol and `other` be a word that is considered to
   * be an additional word of the stacked math word. The actual merging is realized as follows:
   *  - `other` is added to `base->isBaseOfStackedMathSymbol` (which is a vector),
   *  - `other->isPartOfStackedWord` is set to `base`.
   *
   * NOTE: The words that are merged with the base character are *not* removed from `page->words`.
   * If you want to exclude the word from further processing, you need to check whether or not
   * `word->isPartOfStackedWord` is set.
   *
   * @param page
   *    The page to process.
   */
  void mergeStackedMathSymbols(PdfPage* page) const;

  /**
   * This method (a) creates a new `PdfWord` object from the given list of characters,
   * (b) computes the respective layout information of the word and (c) appends the word to the
   * given result list.
   *
   * @param chars
   *   The characters from which to create the word.
   * @param words
   *   The vector to which the created word should be appended.
   */
  void createWord(const std::vector<PdfCharacter*>& chars, std::vector<PdfWord*>* words) const;

  // The document to process.
  PdfDocument* _doc;

  // The characters of the current word.
  std::vector<PdfCharacter*> _currWordChars;

  // The coordinates of the bounding box around the characters in _currWordChars.
  double _currWordMinX = std::numeric_limits<double>::max();
  double _currWordMinY = std::numeric_limits<double>::max();
  double _currWordMaxX = std::numeric_limits<double>::min();
  double _currWordMaxY = std::numeric_limits<double>::min();

  // The maximum font size among the characters in _currWordChars.
  double _currWordMaxFontSize = 0;

  /** The logger. */
  Logger* _log;
};

#endif  // WORDSDETECTOR_H_
