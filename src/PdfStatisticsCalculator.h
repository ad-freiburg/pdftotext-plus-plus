/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef PDFSTATISTICSCALCULATOR_H_
#define PDFSTATISTICSCALCULATOR_H_

#include "./PdfDocument.h"

// =================================================================================================
// CONFIG.

namespace pdf_statistics_calculator::config {

// Do not use the default FS_EQUAL_TOLERANCE from global_config here, but an alternative value.
const double FS_EQUAL_TOLERANCE = 0.1;

// A threshold that is used while checking if two consecutive words vertically overlap. The
// doc->mostFreqWordDistance is only measured between those two words for which the maximum
// y-overlap ratio is larger or equal to this threshold.
const double Y_OVERLAP_RATIO_OVERLAPPING_THRESH = 0.5;

//  A threshold that is used while checking if two consecutive words do *not* vertically overlap.
// The doc->mostFreqEstimatedLineDistance is only measured between those two words for which the
// maximum y-overlap ratio is smaller or equal to this threshold.
const double Y_OVERLAP_RATIO_NOT_OVERLAPPING_THRESH = 0;

}  // namespace pdf_statistics_calculator::config

// =================================================================================================

using pdf_statistics_calculator::config::Y_OVERLAP_RATIO_OVERLAPPING_THRESH;
using pdf_statistics_calculator::config::Y_OVERLAP_RATIO_NOT_OVERLAPPING_THRESH;

/**
 * This class computes some statistics about the characters, words and text lines in a PDF document,
 * for example: the most frequent font size among the characters or the most frequent line distance.
 */
class PdfStatisticsCalculator {
 public:
  /**
   * This constructor creates and initializes a new instance of this class.
   *
   * @param doc
   *    The document for which to compute the statistics.
   */
  explicit PdfStatisticsCalculator(PdfDocument* doc);

  /** The deconstructor. */
  ~PdfStatisticsCalculator();

  /**
   * This method computes statistics about the characters in a PDF document and stores them in the
   * respective properties of the document. Here is an overview of the computed properties:
   *  _doc->mostFreqFontSize: The most frequent font size among the characters;
   *  _doc->mostFreqFontName: The most frequent font name among the characters;
   *  _doc->avgCharWidth: The average character width;
   *  _doc->avgCharHeight: The average character height.
   */
  void computeCharacterStatistics() const;

  /**
   * This method computes statistics about the words in a PDF document and stores them in the
   * respective properties of the document. Here is an overview of the computed properties:
   *  _doc->mostFreqWordHeight: The most frequent word height
   *  _doc->mostFreqWordDistance: The most frequent horizontal gap between two consecutive words
   *  _doc->mostFreqEstimatedLineDistance: The most frequent line distance in this PDF document,
   *     estimated by analyzing the vertical gaps between consecutive words that do not vertically
   *     overlap (this is needed for tasks that require the most frequent line distance, but
   *     need to be executed before text lines were detected).
   *
   * @param yOverlapRatioOverlappingThreshold
   *    A threshold that is used while checking if two consecutive words vertically overlap. The
   *    mostFreqWordDistance is only measured between those two words for which the maximum
   *    y-overlap ratio is larger or equal to this threshold.
   * @param yOverlapRatioNotOverlappingThreshold
   *    A threshold that is used while checking if two consecutive words do *not* vertically
   *    overlap. The mostFreqEstimatedLineDistance is only measured between those two words for
   *    which the maximum y-overlap ratio is smaller or equal to this threshold.
   */
  void computeWordStatistics(
    double yOverlapRatioOverlappingThreshold = Y_OVERLAP_RATIO_OVERLAPPING_THRESH,
    double yOverlapRatioNotOverlappingThreshold = Y_OVERLAP_RATIO_NOT_OVERLAPPING_THRESH) const;

  /**
   * This method computes statistics about the text lines in a PDF document and stores them in the
   * respective properties of the document. Here is an overview of the computed properties:
   *  _doc->mostFreqLineDistance: The most frequent line distance between two consecutive lines
   *    NOTE: This line distance is computed by analyzing the vertical gaps between the *base
   *    *bounding box* of the lines. This usually results in a more accurately computed most
   *    frequent line distance, because subscripts and superscripts can shrink the vertical gap
   *    between the lines.
   *  _doc->mostFreqLineDistancePerFontSize: The most line distance between two consecutive text
   *    lines with the same font size, broken down by font sizes. The value stored at
   *    _doc->mostFreqLineDistancePerFontSize[x] denotes the most frequent vertical gap between
   *    two consecutive lines with font size x.
   */
  void computeTextLineStatistics() const;

 private:
  // The document for which to compute the statistics.
  PdfDocument* _doc;
};

#endif  // PDFSTATISTICSCALCULATOR_H_
