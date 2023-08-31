/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TEXTLINESSTATISTICSCALCULATION_H_
#define TEXTLINESSTATISTICSCALCULATION_H_

#include "./Config.h"
#include "./Types.h"
#include "./utils/Log.h"

using ppp::config::TextLinesStatisticsCalculationConfig;
using ppp::types::PdfDocument;
using ppp::utils::log::Logger;

// =================================================================================================

namespace ppp::modules {

/**
 * This class calculates some statistics about the text lines in a PDF document, for example: the
 * most frequent line distance.
 */
class TextLinesStatisticsCalculation {
 public:
  /**
   * The default constructor.
   *
   * @param doc
   *    The document for which to calculate the statistics.
   * @param config
   *    The configuration to use.
   */
  TextLinesStatisticsCalculation(
    PdfDocument* doc,
    const TextLinesStatisticsCalculationConfig* config);

  /** The deconstructor. */
  ~TextLinesStatisticsCalculation();

  /**
   * This method calculates statistics about the text lines in a PDF document and stores them in the
   * respective properties of the document. Here is an overview of the calculated properties:
   *
   *  _doc->mostFreqLineDistance: The most frequent line distance between two consecutive lines
   *    NOTE: This line distance is calculated by analyzing the vertical gaps between the *base
   *    *bounding box* of the lines. This usually results in a more accurately calculate most
   *    frequent line distance, because subscripts and superscripts can shrink the vertical gap
   *    between the lines.
   *  _doc->mostFreqLineDistancePerFontSize: The most line distance between two consecutive text
   *    lines with the same font size, broken down by font sizes. The value stored at
   *    _doc->mostFreqLineDistancePerFontSize[x] denotes the most frequent vertical gap between
   *    two consecutive lines with font size x.
   */
  void process() const;

 private:
  // The document for which to calculate the statistics.
  PdfDocument* _doc;
  // The configuration to use.
  const TextLinesStatisticsCalculationConfig* _config;
  // The logger.
  Logger* _log;
};

}  // namespace ppp::modules

#endif  // TEXTLINESSTATISTICSCALCULATION_H_
