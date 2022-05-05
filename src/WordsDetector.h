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
#include <vector>  // std::vector

#include "./utils/LogUtils.h"
#include "./PdfDocument.h"

// =================================================================================================

// A PDF can contain "stacked math symbols", which we want to merge to a single word (see the
// preliminary comment of the `WordsDetector` class below for more information). The following
// three sets are used to identify the base word of a stacked math symbol.
// The first set contains the text of glyphs that are likely to be part of a base word of a stacked
// math symbol. If a word contains a glyph that is part of this set, it is considered as the base
// word of a stacked math symbol.
// The second set contains the names of glyphs that are likely to be part of a base word of a
// stacked math symbol. If a word contains a glyph with a name that is part of this set, it is
// considered as the base word of a stacked math symbol (NOTE: this set was introduced because, in
// some PDFs, the text of summation symbols do not contain a summation symbol, but some
// weird symbols (e.g., a "?"), because of a missing encoding. The names of the glyphs are simply
// an additional indicator for identifying the base word of a stacked math symbol).
// The third set contains *words* that are likely to be the base part of stacked math symbols.
static const std::unordered_set<std::string> stackedMathGlyphTexts = { "∑", "∏", "∫", "⊗" };
static const std::unordered_set<std::string> stackedMathGlyphNames = { "summationdisplay",
    "productdisplay", "integraldisplay", "circlemultiplydisplay" };
static const std::unordered_set<std::string> stackedMathWords = { "sup", "lim" };

// =================================================================================================

/**
 * This class has two purposes: (a) given the glyphs extracted from a PDF document, detect the
 * words and (b) given the detected words, merge "stacked math symbols". Stacked math symbols are
 * math symbols that are positioned one above the other and logically belong together. A stacked
 * math symbol consist of a base word and one or more additional words. A typical example is the
 * summation symbol, which can contain indices that are positioned below and above the summation
 * symbol: ∑_{i=0}^{n}. The "∑" is the base word, the "i=0" and "n" are the additional words.
 * Without merging, "i=0" and/or "n" would be treated as separate words (because they do not
 * overlap vertically with the base word). A possible consequence is that the additional words are
 * detected as a part of a different text line than the base word (which is not the expected
 * output).
 */
class WordsDetector {
 public:
  /**
   * This constructor creates and initializes a new instance of this `WordsDetector` class.
   *
   * @param doc
   *   The PDF document to process.
   * @param debug
   *   Whether or not this instance should print debug information to the console.
   * @param debugPageFilter
   *   The number of the page to which the debug information should be reduced. If specified as a
   *   value > 0, only those messages that relate to the given page will be printed to the console.
   */
  WordsDetector(PdfDocument* doc, bool debug=false, int debugPageFilter=-1);

  /** The deconstructor. */
  ~WordsDetector();

  /**
   * This method detects words and merges stacked math symbols. It iterates through the pages of
   * the given document and calls the detectWords() and mergeStackedMathSymbols() methods
   * below for each page. More information about each method is given in the comment of the
   * respective method below.
   */
  void detect();

 private:
  /**
   * This method detects words in the given page. The basic procedure is as follows. It iterates
   * the glyphs stored in `page->glyphs` in extraction order. For each glyph, it decides whether
   * or not the glyph starts a new word, by analyzing different layout information of the glyphs.
   * The detected words from the i-th page are appended to `_doc->pages[i]->words`.
   *
   * @param page
   *    The page to process.
   */
  void detectWords(PdfPage* page);

  /**
   * This method returns true if the given current glyph starts a new word, false otherwise.
   * This decision is made based on analyzing different layout information of the glyphs.
   *
   * @param prevGlyph
   *   The previously processed glyph, if such exists; null otherwise.
   * @param currGlyph
   *   The glyph for which to decide whether or not it starts a new word.
   *
   * @return
   *    True if the given current glyph starts a new word, false otherwise.
   */
  bool startsWord(const PdfGlyph* prevGlyph, const PdfGlyph* currGlyph) const;

  /**
   * This method merges stacked math symbols in the given page. It iterates through the words stored
   * in `page->words`. For each word, it checks whether it denotes the base word of a stacked math
   * symbol (by looking up the glyphs of the word, and the word itself in the stackedMath*-sets
   * above). If so, it checks whether the adjacents words horizontally overlap the base word.
   * If so, they are considered to be additional words of the stacked math symbol.
   * Let `base` the base word of a stacked math symbol and `other` be a word that is considered to
   * be an additional word of the stacked math word. The actual merging is realized as follows:
   *  - `other` is added to `base->isBaseOfStackedMathSymbol` (which is a vector),
   *  - `other->isPartOfStackedWord` is set to `base`.
   *
   * NOTE: The words that are merged with the base glyph are *not* removed from `page->words`. If
   *  you want to exclude the word from further processing, you need to check whether or not
   *  `word->isPartOfStackedWord` is set.
   *
   * @param page
   *    The page to process.
   */
  void mergeStackedMathSymbols(PdfPage* page) const;

  /**
   * This method (a) creates a new `PdfWord` object from the given list of glyphs, (b) computes the
   * respective layout information of the word and (c) appends the word to the given result list.
   *
   * @param glyphs
   *   The glyphs from which to create the word.
   * @param words
   *   The vector to which the created word should be appended.
   */
  void createWord(const std::vector<PdfGlyph*>& glyphs, std::vector<PdfWord*>* words) const;

  /** The document to process. */
  PdfDocument* _doc;

  /** The glyphs of the current word, together with some word properties (e.g., bounding box). */
  std::vector<PdfGlyph*> _currWordGlyphs;
  double _currWordMinX = std::numeric_limits<double>::max();
  double _currWordMinY = std::numeric_limits<double>::max();
  double _currWordMaxX = std::numeric_limits<double>::min();
  double _currWordMaxY = std::numeric_limits<double>::min();
  double _currWordMaxFontSize = 0;

  /** The logger. */
  Logger* _log;
};

#endif  // WORDSDETECTOR_H_
