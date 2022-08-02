/**
 * Copyright 2022, University of Freiburg,
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

#include "./Constants.h"  // C_DPI, H_DPI
#include "./DiacriticalMarksMerger.h"
#include "./PageSegmentator.h"
#include "./PdfStatisticsCalculator.h"
#include "./PdfToTextPlusPlus.h"
#include "./ReadingOrderDetector.h"
#include "./SubSuperScriptsDetector.h"
#include "./TextBlocksDetector.h"
#include "./TextLinesDetector.h"
#include "./TextOutputDev.h"
#include "./WordsDehyphenator.h"
#include "./WordsDetector.h"

using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using std::string;
using std::unique_ptr;
using std::vector;

// _________________________________________________________________________________________________
PdfToTextPlusPlus::PdfToTextPlusPlus(
      bool noEmbeddedFontFilesParsing,
      bool noWordsDehyphenation,
      bool parseMode,
      bool debugPdfParsing,
      bool debugStatisticsComputation,
      bool debugDiacriticMarksMerging,
      bool debugWordsDetection,
      bool debugPageSegmentation,
      bool debugTextLinesDetection,
      bool debugSubSuperScriptsDetection,
      bool debugTextBlocksDetection,
      int debugPageFilter) {
  _noEmbeddedFontFilesParsing = noEmbeddedFontFilesParsing;
  _noWordsDehyphenation = noWordsDehyphenation;
  _parseMode = parseMode;
  _debugPdfParsing = debugPdfParsing;
  _debugStatisticsComputation = debugStatisticsComputation;
  _debugDiacMarksMerging = debugDiacriticMarksMerging;
  _debugWordsDetection = debugWordsDetection;
  _debugPageSegmentation = debugPageSegmentation;
  _debugTextLinesDetection = debugTextLinesDetection;
  _debugSubSuperScriptsDetection = debugSubSuperScriptsDetection;
  _debugTextBlocksDetection = debugTextBlocksDetection;
  _debugPageFilter = debugPageFilter;
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
  TextOutputDev out(!_noEmbeddedFontFilesParsing, doc, _debugPdfParsing, _debugPageFilter);
  start = high_resolution_clock::now();
  pdfDoc->displayPages(
    &out,
    1,  // firstPage
    pdfDoc->getNumPages(),  // lastPage
    H_DPI,  // hDPI
    V_DPI,  // vDPI
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
  PdfStatisticsCalculator psc(doc, _debugStatisticsComputation);
  start = high_resolution_clock::now();
  psc.computeCharacterStatistics();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Computing character stats", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (4) Merge combining diacritical marks with their base characters.
  start = high_resolution_clock::now();
  DiacriticalMarksMerger dmm(doc, _debugDiacMarksMerging, _debugPageFilter);
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
  WordsDetector wd(doc, _debugWordsDetection, _debugPageFilter);
  wd.process();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Detecting words", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (6) Compute some statistics about the words, for example: the most frequent word height.
  start = high_resolution_clock::now();
  psc.computeWordStatistics();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Computing word stats", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (7) Segment the pages of the document (for identifying columns).
  start = high_resolution_clock::now();
  PageSegmentator ps(doc, _debugPageSegmentation, _debugPageFilter);
  ps.process();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Segmenting pages", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (8) Detect the text lines.
  start = high_resolution_clock::now();
  TextLinesDetector tld(doc, _debugTextLinesDetection, _debugPageFilter);
  tld.process();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Detecting text lines", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (9) Detect subscripted and superscripted characters.
  start = high_resolution_clock::now();
  SubSuperScriptsDetector ssd(doc, _debugSubSuperScriptsDetection, _debugPageFilter);
  ssd.process();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Detecting sub-/superscripts", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (10) Compute some statistics about the text lines, for example: the most frequent indentation.
  start = high_resolution_clock::now();
  psc.computeTextLineStatistics();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Computing line statistics", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (11) Detect the text blocks.
  start = high_resolution_clock::now();
  TextBlocksDetector tbd(doc, _debugTextBlocksDetection, _debugPageFilter);
  tbd.process();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Detecting text blocks", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (12) Detect the reading order of the text blocks.
  start = high_resolution_clock::now();
  ReadingOrderDetector rod(doc);
  rod.detect();
  end = high_resolution_clock::now();
  if (timings) {
    Timing timing("Detecting reading order", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timing);
  }

  // (13) Dehyphenate words, if not deactivated by the user.
  if (!_noWordsDehyphenation) {
    start = high_resolution_clock::now();
    WordsDehyphenator wdh(doc);
    wdh.dehyphenate();
    end = high_resolution_clock::now();
    if (timings) {
      Timing timing("Dehyphenating words", duration_cast<milliseconds>(end - start).count());
      timings->push_back(timing);
    }
  }

  return 0;
}
