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
// CONFIG

namespace pdf_statistics_calculator::config {

// A parameter that denotes the precision to use when rounding the coordinates of elements before
// computing the most frequent coordinates.
const double COORDS_PREC = global_config::COORDS_PREC;

// A parameter that denotes the precision to use when rounding the font sizes of text elements
// before computing the most frequent font size.
const double FONT_SIZE_PREC = global_config::FONT_SIZE_PREC;

// A parameter used for computing if two given font sizes are equal. It denotes the maximum allowed
// difference between two font sizes so that the font sizes are considered to be equal.
const double FSIZE_EQUAL_TOLERANCE = 0.1;

// ----------
// computeWordStatistics()

// A parameter that is used for computing whether or not two words are part of the same text line.
// If the maximum y-overlap ratio between two words is larger or eqaul to the returned threshold,
// the words are considered to be part of the same text line.
const double SAME_LINE_Y_OVERLAP_RATIO_THRESHOLD = 0.5;

// A parameter that is used for computing whether or not two words are part of different text lines.
// If the maximum y-overlap ratio between two words is smaller or eqaul to the returned threshold,
// the words are considered to be part of different text lines.
const double OTHER_LINE_Y_OVERLAP_RATIO_THRESHOLD = 0;

// ----------
// computeTextLineStatistics()

// A parameter that denotes the precision to use when rounding the distances between text lines
// before computing the most frequent line distance.
const double LINE_DIST_PREC = global_config::LINE_DIST_PREC;

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
   * @param logLevel
   *   The logging level.
   */
  explicit PdfStatisticsCalculator(PdfDocument* doc, LogLevel logLevel = ERROR);

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
