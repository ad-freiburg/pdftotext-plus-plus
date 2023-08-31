/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <iostream>  // std::endl

#include "./PdfDocument.h"
#include "./GlyphsStatisticsCalculation.h"
#include "./utils/Counter.h"
#include "./utils/Log.h"

using std::endl;

using ppp::config::GlyphsStatisticsCalculationConfig;
using ppp::types::PdfDocument;
using ppp::utils::counter::DoubleCounter;
using ppp::utils::counter::StringCounter;
using ppp::utils::log::BOLD;
using ppp::utils::log::OFF;
using ppp::utils::log::Logger;

// =================================================================================================

namespace ppp::modules {

// _________________________________________________________________________________________________
GlyphsStatisticsCalculation::GlyphsStatisticsCalculation(
    PdfDocument* doc,
    const GlyphsStatisticsCalculationConfig* config) {
  _doc = doc;
  _config = config;
  _log = new Logger(config->logLevel, config->logPageFilter);
}

// _________________________________________________________________________________________________
GlyphsStatisticsCalculation::~GlyphsStatisticsCalculation() {
  delete _log;
}

// _________________________________________________________________________________________________
void GlyphsStatisticsCalculation::process() const {
  assert(_doc);

  _log->info() << "Calculating glyph statistics..." << endl;
  _log->debug() << "=======================================" << endl;
  _log->debug() << BOLD << "DEBUG MODE" << OFF << endl;
  _log->debug() << "=======================================" << endl;

  // A counter for the font sizes of the glyphs.
  DoubleCounter fontSizeCounter;
  // A counter for the font names of the glyphs.
  StringCounter fontNameCounter;

  // The sum of the char widths and -heights, for calculating the average char width/-height.
  double sumWidths = 0;
  double sumHeights = 0;

  // The number of glyphs in the document.
  int numGlyphs = 0;

  for (const auto* page : _doc->pages) {
    for (const auto* character : page->characters) {
      fontSizeCounter[character->fontSize]++;
      fontNameCounter[character->fontName]++;
      sumWidths += character->pos->getWidth();
      sumHeights += character->pos->getHeight();
      numGlyphs++;
    }
  }

  // Abort if the document contains no glyphs.
  if (numGlyphs == 0) {
    return;
  }

  // Calculate the most frequent font size and font name.
  _doc->mostFreqFontSize = fontSizeCounter.sumCounts() > 0 ? fontSizeCounter.mostFreq() : 0.0;
  _doc->mostFreqFontName = fontNameCounter.sumCounts() > 0 ? fontNameCounter.mostFreq() : "";

  _log->debug() << "doc.mostFreqFontSize: " << _doc->mostFreqFontSize << endl;
  _log->debug() << "doc.mostFreqFontName: " << _doc->mostFreqFontName << endl;

  // Calculate the average glyph width and -height.
  _doc->avgCharWidth = sumWidths / static_cast<double>(numGlyphs);
  _doc->avgCharHeight = sumHeights / static_cast<double>(numGlyphs);

  _log->debug() << "doc.avgCharWidth:  " << _doc->avgCharWidth << endl;
  _log->debug() << "doc.avgCharHeight: " << _doc->avgCharHeight << endl;
  _log->debug() << "=======================================" << endl;
}

}  // namespace ppp::modules
