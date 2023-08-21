/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef PDFTOTEXTPLUSPLUS_H_
#define PDFTOTEXTPLUSPLUS_H_

#include <string>
#include <vector>

#include "./Config.h"
#include "./PdfDocument.h"
#include "./Types.h"
#include "./utils/Log.h"

using std::string;
using std::vector;

using ppp::config::Config;
using ppp::types::PdfDocument;
using ppp::types::Timing;
using ppp::utils::log::LogLevel;

// =================================================================================================

namespace ppp {

/**
 * This class is the core class of pdftotext++. It is responsible for processing a given PDF file
 * by invoking the following modules of the extraction pipeline:
 *
 *  (1) Loading the PDF file;
 *  (2) Parsing the content streams of the PDF file for detecting characters, graphics and shapes;
 *  (3) Computing character statistics;
 *  (4) Merging combining diacritical marks with their base characters;
 *  (5) Detecting words;
 *  (6) Computing word statistics;
 *  (7) Segmenting the pages of the PDF file (= separating the words into columns);
 *  (8) Detecting text lines;
 *  (9) Detecting subscripted and superscripted characters;
 * (10) Computing text line statistics;
 * (11) Detecting text blocks;
 * (12) Detecting the reading order of the text blocks;
 * (13) Dehyphenating words.
 *
 * NOTE: If the parsing mode is activated (that is: if _parsingMode is set to true), the extraction
 * pipeline stops after step (4).
 */
class PdfToTextPlusPlus {
 public:
  /**
   * This constructor creates and initializes a new instance of this class.
   *
   * @param config
   *   The configuration to use.
   * @param parseMode
   *   A boolean flag indicating whether or not to activate the parsing mode, that is: a mode that
   *   parses the content streams of the PDF file for characters, figures, and shapes, and stops
   *   afterwards. Words, text lines, and text blocks are *not* detected. Setting this parameter to
   *   true enables the parsing mode, setting it to false disables the parsing mode.
   *   NOTE: This mode was introduced for the benchmark generator that needs information about the
   *   the contained characters (e.g., the position, text and color) for assembling the true words,
   *   text blocks, etc. based on the color of the characters.
   */
  explicit PdfToTextPlusPlus(const Config* config, bool parseMode = false);

  /** The deconstructor */
  ~PdfToTextPlusPlus();

  /**
   * This method extracts the text from the given PDF file, by processing the pipeline mentioned
   * in the preliminary comment of this class.
   *
   * @param pdfFilePath
   *   The path to the PDF file to process.
   * @param doc
   *   The `PdfDocument` instance to which the extracted text and other elements should be stored.
   * @param timing
   *   A vector to which the running times needed by the different modules of the extraction
   *   pipeline should be appended. If not set, no running times will be appended.
   *
   * @return
   *    0 if the PDF was processed successfully, and a value > 0 otherwise.
   */
  int process(const string* pdfFilePath, PdfDocument* doc, vector<Timing>* timings = nullptr) const;

 private:
  // The configuration to use.
  const Config* _config;
  // Whether or not to parse the embedded font files of a PDF file.
  bool _noEmbeddedFontFilesParsing;
  // Whether or not to activate the parsing mode.
  bool _parseMode;
  // The level for the logging messages produced while parsing the content streams.
  LogLevel _logLevelPdfParsing;
  // The level for the logging messages produced while computing statistics.
  LogLevel _logLevelStatisticsComputation;
  // The level for the logging messages produced while merging diacritical marks.
  LogLevel _logLevelDiacMarksMerging;
  // The level for the logging messages produced while detecting words.
  LogLevel _logLevelWordsDetection;
  // The level for the logging messages produced while segmenting pages.
  LogLevel _logLevelPageSegmentation;
  // The level for the logging messages produced while detecting text lines.
  LogLevel _logLevelTextLinesDetection;
  // The level for the logging messages produced while detecting sub-/superscripts.
  LogLevel _logLevelSubSuperScriptsDetection;
  // The level for the logging messages produced while detecting text blocks.
  LogLevel _logLevelTextBlocksDetection;
  // The page filter for the logging messages.
  int _logPageFilter;
};

}  // namespace ppp

#endif  // PDFTOTEXTPLUSPLUS_H_
