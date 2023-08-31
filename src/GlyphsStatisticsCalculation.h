/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef GLYPHSSTATISTICSCALCULATION_H_
#define GLYPHSSTATISTICSCALCULATION_H_

#include "./Config.h"
#include "./PdfDocument.h"
#include "./utils/Log.h"

using ppp::config::GlyphsStatisticsCalculationConfig;
using ppp::types::PdfDocument;
using ppp::utils::log::Logger;

// =================================================================================================

namespace ppp::modules {

/**
 * This class calculates some statistics about the glyphs in a PDF document, for example: the most
 * frequent font size among the glyphs.
 */
class GlyphsStatisticsCalculation {
 public:
  /**
   * The default constructor.
   *
   * @param doc
   *    The document for which to calculate the statistics.
   * @param config
   *    The configuration to use.
   */
  GlyphsStatisticsCalculation(
    PdfDocument* doc,
    const GlyphsStatisticsCalculationConfig* config);

  /** The deconstructor. */
  ~GlyphsStatisticsCalculation();

  /**
   * This method calculates statistics about the glyphs in a PDF document and stores them in the
   * respective properties of the document. Here is an overview of the calculated properties:
   *
   *  _doc->mostFreqFontSize: The most frequent font size among the characters;
   *  _doc->mostFreqFontName: The most frequent font name among the characters;
   *  _doc->avgCharWidth:     The average character width;
   *  _doc->avgCharHeight:    The average character height.
   */
  void process() const;

 private:
  // The document for which to calculate the statistics.
  PdfDocument* _doc;
  // The configuration to use.
  const GlyphsStatisticsCalculationConfig* _config;
  // The logger.
  Logger* _log;
};

}  // namespace ppp::modules

#endif  // GLYPHSSTATISTICSCALCULATION_H_
