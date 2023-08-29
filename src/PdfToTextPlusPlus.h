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
 *  (3) Calculating glyphs statistics;
 *  (4) Merging combining diacritical marks with their base characters;
 *  (5) Detecting words;
 *  (6) Calculating words statistics;
 *  (7) Segmenting the pages of the PDF file (= separating the words into columns);
 *  (8) Detecting text lines;
 *  (9) Detecting subscripted and superscripted characters;
 * (10) Calculating text lines statistics;
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
   */
  explicit PdfToTextPlusPlus(const Config* config);

  /** The deconstructor */
  ~PdfToTextPlusPlus();

  /**
   * This method extracts the text from the given PDF document, by processing the pipeline mentioned
   * in the preliminary comment of this class.
   *
   * @param doc
   *   The `PdfDocument` to process and to which the extracted text and other elements should be
   *   stored.
   * @param timing
   *   A vector to which the running times needed by the different modules of the extraction
   *   pipeline should be appended. If not set, no running times will be appended.
   *
   * @return
   *    0 if the PDF was processed successfully, and a value > 0 otherwise.
   */
  int process(PdfDocument* doc, vector<Timing>* timings = nullptr) const;

 private:
  // The configuration to use.
  const Config* _config;
};

}  // namespace ppp

#endif  // PDFTOTEXTPLUSPLUS_H_
