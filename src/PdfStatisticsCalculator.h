/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef PDFSTATISTICSCALCULATOR_H_
#define PDFSTATISTICSCALCULATOR_H_

#include "./utils/Log.h"

#include "./PdfDocument.h"

// =================================================================================================
// CONFIG.

namespace pdf_statistics_calculator::config {

// TODO
// The precision (the number of decimal points) to use when rounding coordinates.
const double COORDS_PREC = global_config::COORDS_PREC;

// TODO
// The precision (the number of decimal points) to use when rounding font sizes.
const double FONT_SIZE_PREC = global_config::FONT_SIZE_PREC;

// TODO
// The precision (the number of decimal points) to use when rounding line distances.
const double LINE_DIST_PREC = global_config::LINE_DIST_PREC;

// TODO
// The maximum allowed difference between two font sizes so that they are considered to be equal.
const double FSIZE_EQUAL_TOLERANCE = 0.1;

// TODO
// A threshold that is used while checking if two consecutive words vertically overlap. The
// doc->mostFreqWordDistance is only measured between those two words for which the maximum
// y-overlap ratio is larger or equal to this threshold.
constexpr double getSameLineYOverlapRatioThreshold(const PdfDocument* doc) {
  return 0.5;
}

// TODO
// A threshold that is used while checking if two consecutive words do *not* vertically overlap.
// The doc->mostFreqEstimatedLineDistance is only measured between those two words for which the
// maximum y-overlap ratio is smaller or equal to this threshold.
constexpr double getOtherLineYOverlapRatioThreshold(const PdfDocument* doc) {
  return 0;
}

}  // namespace pdf_statistics_calculator::config

// =================================================================================================

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
   * @param debug
   *    Whether or not this instance should print debug information to the console.
   */
  explicit PdfStatisticsCalculator(PdfDocument* doc, bool debug);

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
   */
  void computeWordStatistics() const;

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
  // The logger.
  const Logger* _log;
};

#endif  // PDFSTATISTICSCALCULATOR_H_
