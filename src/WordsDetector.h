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

#include "./Constants.h"
#include "./PdfDocument.h"

using std::string;
using std::unordered_set;
using std::vector;

// =================================================================================================
// CONFIG.

namespace words_detector::config {

// The maximum allowed difference between two font sizes so that they are considered to be equal.
const double FSIZE_EQUAL_TOLERANCE = global_config::FS_EQUAL_TOLERANCE;

// A threshold used on checking if the active word and the current character vertically overlap.
// If the maximum y-overlap ratio between the active word and the current character is smaller than
// this threshold, the word and the character are considered to not overlap; otherwise they are
// considered to overlap.
const double MIN_Y_OVERLAP_RATIO_THRESHOLD = 0.5;

// A factor that is used to compute a threshold for checking if the horizontal gap between two
// characters is large enough in order to be considered as a word delimiter. The threshold is
// computed as: <FACTOR> * _activeWord.fontSize. If the horizontal gap between the
// characters is larger than this threshold, it is considered to be a word delimiter.
const double HGAP_THRESHOLD_FACTOR = 0.15;

// A threshold that is used while merging staked math symbolds, to check if two words horizontally
// overlap. If the maximum x-overlap ratio between two words is smaller than this threshold, the
// words are considered to not horizontally overlap; otherwise they are considered to horizontally
// overlap.
const double STACKED_MATH_MIN_X_OVERLAP_RATIO_THRESHOLD = 0.5;

// A PDF can contain "stacked math symbols", which we want to merge to a single word (see the
// preliminary comment of the `WordsDetector` class below for more information about how stacked
// math symbols are defined). The following three sets are used to identify the base word of a
// stacked math symbol.
// The first set contains the *text* of characters that are likely to be part of a base word of a
// stacked math symbol. If a word indeed contains a character that is part of this set, it is
// considered to be the base word of a stacked math symbol.
// The second set contains the *names* of characters that are likely to be part of a base word of a
// stacked math symbol. If a word contains a character with a name that is part of this set, it is
// considered to be the base word of a stacked math symbol (NOTE: this set was introduced because,
// in some PDFs, the text of summation symbols does not contain a summation symbol, but some
// weird symbols (e.g., a "?"), most typically because of a missing encoding. The names of the
// characters are simply an additional indicator for identifying the base word of a stacked math
// symbol).
// The third set contains *words* that are likely to be a base word of a stacked math symbol.
const unordered_set<string> STACKED_MATH_CHAR_TEXTS = { "∑", "∏", "∫", "⊗" };
const unordered_set<string> STACKED_MATH_CHAR_NAMES = { "summationdisplay",
    "productdisplay", "integraldisplay", "circlemultiplydisplay" };
const unordered_set<string> STACKED_MATH_WORDS = { "sup", "lim" };

}  // namespace words_detector::config

// =================================================================================================

using words_detector::config::HGAP_THRESHOLD_FACTOR;
using words_detector::config::MIN_Y_OVERLAP_RATIO_THRESHOLD;
using words_detector::config::STACKED_MATH_MIN_X_OVERLAP_RATIO_THRESHOLD;

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
  explicit WordsDetector(const PdfDocument* doc, bool debug = false, int debugPageFilter = -1);

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
   * @param minYOverlapRatioThreshold
   *   A threshold used on checking if the active word and the current character vertically overlap.
   *   (which is a requirement for the character to be part of the active word). If the maximum
   *   y-overlap ratio between the active word and the current character is smaller than this
   *   threshold, the word and the character are considered to not vertically overlap; otherwise
   *   they are considered to vertically overlap.
   * @param hGapThresholdFactor
   *  A factor that is used to compute a threshold for checking if the horizontal gap between
   *  the active word and the given character is "large enough", so that the given
   *  character is considered to be the start of a new word. The threshold is computed as:
   *    <hGapThresholdFactor> * _activeWord.fontSize.
   * If the horizontal gap between the active word and the current character is larger than this
   * threshold, the character is considered to be the start of a new word.
   *
   * @return
   *    True if the given character starts a new word, false otherwise.
   */
  bool startsWord(const PdfCharacter* currChar,
      double minYOverlapRatioThreshold = MIN_Y_OVERLAP_RATIO_THRESHOLD,
      double hGapThresholdFactor = HGAP_THRESHOLD_FACTOR) const;

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
   * @param minXOverlapRatioThreshold
   *    A threshold that is used to check if two words horizontally overlap (to check if they
   *    are part of the same stacked math symbol). If the maximum x-overlap ratio between two words
   *    is smaller than this threshold, the words are considered to not horizontally overlap;
   *    otherwise they are considered to horizontally overlap.
   */
  void mergeStackedMathSymbols(const PdfPage* page,
      double minXOverlapRatioThreshold = STACKED_MATH_MIN_X_OVERLAP_RATIO_THRESHOLD) const;

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
  const PdfDocument* _doc;

  // The active word.
  PdfWord _activeWord;

  // The logger.
  const Logger* _log;
};

#endif  // WORDSDETECTOR_H_
