/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef PDFDOCUMENTSTATISTICSCALCULATOR_H_
#define PDFDOCUMENTSTATISTICSCALCULATOR_H_

#include "./PdfDocument.h"


/**
 * This class computes document-wide statistics, for example: the most frequent font size among
 * the characters or the most frequent line distance.
 */
class PdfDocumentStatisticsCalculator {
 public:
  /**
   * This constructor creates and initializes a new `PdfDocumentStatisticsCalculator`.
   *
   * @param doc The document to process.
   */
  explicit PdfDocumentStatisticsCalculator(PdfDocument* doc);

  /** The deconstructor. */
  ~PdfDocumentStatisticsCalculator();

  /**
   * This method iterates through the characters of the document in order to compute statistics
   * about the characters, for example: the most common font used among the glpyhs or the average
   * character width and character height. The computed statistics will be written to the
   * respective properties of the `PdfDocument`; for example, the average character width will be
   * written to `_doc->avgCharWidth`.
   */
  void computeCharStatistics() const;

  /**
   * This method iterates through the words of the document in order to compute statistics about
   * the words, for example: the most frequent word height. The computed statistics will be
   * written to the respective properties of the `PdfDocument`; for example, the most frequent
   * word height will be written to `_doc->mostFreqWordHeight`.
   */
  void computeWordStatistics() const;

  void computeLineStatistics() const;

 private:
  /** The document for which to compute the statistics. */
  PdfDocument* _doc;
};

#endif  // PDFDOCUMENTSTATISTICSCALCULATOR_H_
