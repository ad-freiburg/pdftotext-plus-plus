/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <goo/GooString.h>
#include <poppler/GlobalParams.h>
#include <poppler/PDFDoc.h>
#include <poppler/PDFDocFactory.h>

#include <chrono>  // std::chrono::high_resolution_clock, etc.
#include <memory>  // std::unique_ptr
#include <string>
#include <vector>

#include "./DiacriticalMarksMerging.h"
#include "./PageSegmentation.h"
#include "./PdfParsing.h"
#include "./StatisticsCalculation.h"
#include "./PdfToTextPlusPlus.h"
#include "./ReadingOrderDetection.h"
#include "./SubSuperScriptsDetection.h"
#include "./TextBlocksDetection.h"
#include "./TextLinesDetection.h"
#include "./Types.h"
#include "./WordsDehyphenation.h"
#include "./WordsDetection.h"

using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using std::string;
using std::unique_ptr;
using std::vector;

using ppp::DiacriticalMarksMerging;
using ppp::PdfParsing;
using ppp::ReadingOrderDetection;
using ppp::StatisticsCalculation;
using ppp::SubSuperScriptsDetection;
using ppp::TextLinesDetection;
using ppp::WordsDehyphenation;
using ppp::WordsDetection;

using ppp::config::Config;
using ppp::types::Timing;

// =================================================================================================

namespace ppp {

// _________________________________________________________________________________________________
PdfToTextPlusPlus::PdfToTextPlusPlus(
      const Config& config,
      bool noWordsDehyphenation,
      bool parseMode) {
  _config = config;
  _noWordsDehyphenation = noWordsDehyphenation;
  _parseMode = parseMode;
}

// _________________________________________________________________________________________________
PdfToTextPlusPlus::~PdfToTextPlusPlus() = default;

// _________________________________________________________________________________________________
int PdfToTextPlusPlus::process(const string& pdfFilePath, PdfDocument* doc,
    vector<Timing>* timings) const {
  assert(doc);

  doc->pdfFilePath = pdfFilePath;

  // Initialize the global parameters, needed by Poppler.
  globalParams = std::make_unique<GlobalParams>();

  // (1) Load the PDF file. Abort if it couldn't be loaded successfully.
  auto start = high_resolution_clock::now();
  GooString gooPdfFilePath(pdfFilePath);
  unique_ptr<PDFDoc> pdfDoc = PDFDocFactory().createPDFDoc(gooPdfFilePath);
  auto end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Loading PDF", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }
  if (!pdfDoc->isOk()) {
    return pdfDoc->getErrorCode();
  }

  // (2) Parse the content streams of the PDF file for characters, graphics and shapes.
  PdfParsing pp(doc, _config.pdfParsing);
  start = high_resolution_clock::now();
  pdfDoc->displayPages(
    &pp,
    1,  // firstPage
    pdfDoc->getNumPages(),  // lastPage
    _config.pdfParsing.hDPI,  // hDPI
    _config.pdfParsing.vDPI,  // vDPI
    0,  // rotation
    true,  // useMediaBox
    false,  // crop
    false);  // printing
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Parsing PDF", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (3) Compute some statistics about the characters, for example: the most frequent font size.
  StatisticsCalculation sc(doc, _config.statisticsCalculation);
  start = high_resolution_clock::now();
  sc.computeGlyphStatistics();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Computing character stats", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (4) Merge combining diacritical marks with their base characters.
  start = high_resolution_clock::now();
  DiacriticalMarksMerging dmm(doc, _config.diacriticalMarksMerging);
  dmm.process();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Merging diacritics", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // Stop here when the parsing mode is activated (since it is supposed to extract only the
  // characters, graphics and shapes from the PDF file).
  if (_parseMode) {
    return 0;
  }

  // (5) Detect the words.
  start = high_resolution_clock::now();
  WordsDetection wd(doc, _config.wordsDetection);
  wd.process();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Detecting words", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (6) Compute some statistics about the words, for example: the most frequent word height.
  start = high_resolution_clock::now();
  sc.computeWordStatistics();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Computing word stats", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (7) Segment the pages of the document (for identifying columns).
  start = high_resolution_clock::now();
  PageSegmentation ps(doc, _config.pageSegmentation);
  ps.process();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Segmenting pages", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (8) Detect the text lines.
  start = high_resolution_clock::now();
  TextLinesDetection tld(doc, _config.textLinesDetection);
  tld.process();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Detecting text lines", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (9) Detect subscripted and superscripted characters.
  start = high_resolution_clock::now();
  SubSuperScriptsDetection ssd(doc, _config.subSuperScriptsDetection);
  ssd.process();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Detecting sub-/superscripts", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (10) Compute some statistics about the text lines, for example: the most frequent indentation.
  start = high_resolution_clock::now();
  sc.computeTextLineStatistics();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Computing line statistics", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (11) Detect the text blocks.
  start = high_resolution_clock::now();
  TextBlocksDetection tbd(doc, _config.textBlocksDetection);
  tbd.process();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Detecting text blocks", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (12) Detect the reading order of the text blocks.
  start = high_resolution_clock::now();
  ReadingOrderDetection rod(doc, _config.readingOrderDetection, _config.semanticRolesPrediction);
  rod.detect();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Detecting reading order", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (13) Dehyphenate words, if not deactivated by the user.
  if (!_noWordsDehyphenation) {
    start = high_resolution_clock::now();
    WordsDehyphenation wdh(doc, _config.wordsDehyphenation);
    wdh.dehyphenate();
    end = high_resolution_clock::now();
    if (timings) {
      Timing timing("Dehyphenating words", duration_cast<milliseconds>(end - start).count());
      timings->push_back(timing);
    }
  }

  return 0;
}

}  // namespace ppp
