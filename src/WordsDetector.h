/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef WORDSDETECTOR_H_
#define WORDSDETECTOR_H_

#include <string>
#include <unordered_set>
#include <vector>

#include "./utils/Log.h"

#include "./PdfDocument.h"

using std::string;
using std::unordered_set;
using std::vector;

// =================================================================================================

/**
 * A factor that is used to compute a tolerance for checking if the horizontal gap between two
 * characters is large enough in order to be considered as a word delimiter. The tolerance is
 * computed as: <HGAP_TOLERANCE_FACTOR> * _activeWord.fontSize. If the horizontal gap between the
 * characters is larger than this tolerance, it is considered to be a word delimiter.
 */
static const double HGAP_TOLERANCE_FACTOR = 0.15;

/**
 * A PDF can contain "stacked math symbols", which we want to merge to a single word (see the
 * preliminary comment of the `WordsDetector` class below for more information about how stacked
 * math symbols are defined). The following three sets are used to identify the base word of a
 * stacked math symbol.
 * The first set contains the *text* of characters that are likely to be part of a base word of a
 * stacked math symbol. If a word indeed contains a character that is part of this set, it is
 * considered to be the base word of a stacked math symbol.
 * The second set contains the *names* of characters that are likely to be part of a base word of a
 * stacked math symbol. If a word contains a character with a name that is part of this set, it is
 * considered to be the base word of a stacked math symbol (NOTE: this set was introduced because,
 * in some PDFs, the text of summation symbols does not contain a summation symbol, but some
 * weird symbols (e.g., a "?"), most typically because of a missing encoding. The names of the
 * characters are simply an additional indicator for identifying the base word of a stacked math
 * symbol).
 * The third set contains *words* that are likely to be a base word of a stacked math symbol.
 */
static const unordered_set<string> STACKED_MATH_CHAR_TEXTS = { "∑", "∏", "∫", "⊗" };
static const unordered_set<string> STACKED_MATH_CHAR_NAMES = { "summationdisplay",
    "productdisplay", "integraldisplay", "circlemultiplydisplay" };
static const unordered_set<string> STACKED_MATH_WORDS = { "sup", "lim" };

// =================================================================================================

/**
 * This class (a) merges the characters of a PDF document to words, and (b) merges words that are
 * part of the same "stacked math symbol".
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
   * @param hGapToleranceFactor
   *  A factor that is used to compute a tolerance for checking if the horizontal gap between
   *  the active word and the given character is "large enough", so that the given
   *  character is considered to be the start of a new word. The tolerance is computed as:
   *    <hGapToleranceFactor> * _activeWord.fontSize.
   * If the horizontal gap between the active word and the current character is larger than this
   * tolerance, the character is considered to be the start of a new word.
   *
   * @return
   *    True if the given character starts a new word, false otherwise.
   */
  bool startsWord(const PdfCharacter* currChar,
      double hGapToleranceFactor = HGAP_TOLERANCE_FACTOR) const;

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
  void mergeStackedMathSymbols(PdfPage* page) const;

  /**
   * This method (a) creates a new `PdfWord` instance from the given vector of characters,
   * (b) computes the respective layout information of the word and (c) appends the word to the
   * given result vector.
   *
   * @param characters
   *   The characters from which to create the word.
   * @param words
   *   The vector to which the created word should be appended.
   *
   * @return
   *    The created word.
   */
  PdfWord* createWord(const vector<PdfCharacter*>& characters, vector<PdfWord*>* words) const;

  // The document to process.
  PdfDocument* _doc;

  // The active word.
  PdfWord _activeWord;

  // The logger.
  Logger* _log;
};

#endif  // WORDSDETECTOR_H_
