/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef WORDSTOKENIZER_H_
#define WORDSTOKENIZER_H_

#include <vector>  // std::vector

#include "./PdfDocument.h"


/**
 * This class iterates through the glyphs of a given document in extraction order and merges them
 * to words, for example by analyzing the spacings, font sizes and writing modes of the glyphs.
 */
class WordsDetector {
 public:
  /**
   * This constructor creates and initializes a new `WordsDetector`.
   *
   * @param doc The document to process.
   */
  explicit WordsDetector(PdfDocument* doc);

  /** The deconstructor. */
  ~WordsDetector();

  /**
   * This method processes the document page-wise. For each page, it iterates through the glyphs
   * of the page in extraction order and merges them to words, for example by analyzing the
   * spacings, font sizes and writing modes of the glyphs. The detected words are appended to
   * `page->words`, where `page` is the `PdfPage` currently processed.
   */
  void detect() const;

 private:
  void tokenize() const;

  void mergeStackedWords() const;

  /**
   * This method (1) creates a new `PdfWord` from the given list of glyphs, (2) computes the
   * layout information of the word and (3) appends the word to the given result list.
   *
   * @param glyphs
   *   The glyphs from which to create the word.
   * @param words
   *   The vector to which the created word should be appended.
   */
  void createWord(const std::vector<PdfGlyph*>& glyphs, std::vector<PdfWord*>* words) const;

  /** The document to process. */
  PdfDocument* _doc;
};

#endif  // WORDSTOKENIZER_H_
