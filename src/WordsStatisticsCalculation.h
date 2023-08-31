/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef WORDSSTATISTICSCALCULATION_H_
#define WORDSSTATISTICSCALCULATION_H_

#include "./Config.h"
#include "./Types.h"
#include "./utils/Log.h"

using ppp::config::WordsStatisticsCalculationConfig;
using ppp::types::PdfDocument;
using ppp::utils::log::Logger;

// =================================================================================================

namespace ppp::modules {

/**
 * This class calculates some statistics about the words in a PDF document, for example: the most
 * frequent word distance.
 */
class WordsStatisticsCalculation {
 public:
  /**
   * The default constructor.
   *
   * @param doc
   *    The document for which to calculate the statistics.
   * @param config
   *   The configuration to use.
   */
  WordsStatisticsCalculation(PdfDocument* doc, const WordsStatisticsCalculationConfig* config);

  /** The deconstructor. */
  ~WordsStatisticsCalculation();

  /**
   * This method calculates statistics about the words in a PDF document and stores them in the
   * respective properties of the document. Here is an overview of the calculates properties:
   *
   *  _doc->mostFreqWordHeight:   The most frequent word height
   *  _doc->mostFreqWordDistance: The most frequent horizontal gap between two consecutive words.
   *  _doc->mostFreqEstimatedLineDistance: The most frequent line distance in this PDF document,
   *     estimated by analyzing the vertical gaps between consecutive words that do not vertically
   *     overlap (this is needed for tasks that require the most frequent line distance, but
   *     need to be executed before text lines were detected).
   */
  void process() const;

 private:
  // The document for which to calculate the statistics.
  PdfDocument* _doc;
  // The configuration to use.
  const WordsStatisticsCalculationConfig* _config;
  // The logger.
  Logger* _log;
};

}  // namespace ppp::modules

#endif  // WORDSSTATISTICSCALCULATION_H_
