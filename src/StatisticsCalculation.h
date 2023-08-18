/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef STATISTICSCALCULATION_H_
#define STATISTICSCALCULATION_H_

#include "./Config.h"
#include "./PdfDocument.h"
#include "./utils/Log.h"

using ppp::config::StatisticsCalculationConfig;
using ppp::utils::log::Logger;

// =================================================================================================

namespace ppp::modules {

/**
 * This class computes some statistics about the glyphs, words and text lines in a PDF document,
 * for example: the most frequent font size among the glyphs or the most frequent line distance.
 */
class StatisticsCalculation {
 public:
  /**
   * The default constructor.
   *
   * @param doc
   *    The document for which to compute the statistics.
   * @param config
   *   The configuration to use.
   */
  StatisticsCalculation(PdfDocument* doc, const StatisticsCalculationConfig& config);

  /** The deconstructor. */
  ~StatisticsCalculation();

  /**
   * This method computes statistics about the glyphs in a PDF document and stores them in the
   * respective properties of the document. Here is an overview of the computed properties:
   *
   *  _doc->mostFreqFontSize: The most frequent font size among the characters;
   *  _doc->mostFreqFontName: The most frequent font name among the characters;
   *  _doc->avgCharWidth:     The average character width;
   *  _doc->avgCharHeight:    The average character height.
   */
  void computeGlyphStatistics() const;

  /**
   * This method computes statistics about the words in a PDF document and stores them in the
   * respective properties of the document. Here is an overview of the computed properties:
   *
   *  _doc->mostFreqWordHeight:   The most frequent word height
   *  _doc->mostFreqWordDistance: The most frequent horizontal gap between two consecutive words.
   *  _doc->mostFreqEstimatedLineDistance: The most frequent line distance in this PDF document,
   *     estimated by analyzing the vertical gaps between consecutive words that do not vertically
   *     overlap (this is needed for tasks that require the most frequent line distance, but
   *     need to be executed before text lines were detected).
   */
  void computeWordStatistics() const;

  /**
   * This method computes statistics about the text lines in a PDF document and stores them in the
   * respective properties of the document. Here is an overview of the computed properties:
   *
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
  // The configuration to use.
  StatisticsCalculationConfig _config;
  // The logger.
  Logger* _log;
};

}  // namespace ppp::modules

#endif  // STATISTICSCALCULATION_H_
