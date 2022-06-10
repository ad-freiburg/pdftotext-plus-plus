/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <goo/GooString.h>
#include <poppler/PDFDoc.h>
#include <poppler/PDFDocFactory.h>

#include <chrono>  // std::chrono::high_resolution_clock, etc.
#include <memory>  // std::unique_ptr
#include <string>
#include <vector>

#include "./Constants.h"
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

// _________________________________________________________________________________________________
PdfToTextPlusPlus::PdfToTextPlusPlus(
      bool noEmbeddedFontFilesParsing,
      bool noWordsDehyphenation,
      bool parseMode,
      bool debugPdfParsing,
      bool debugDiacriticMarksMerging,
      bool debugWordsDetection,
      bool debugPageSegmentation,
      bool debugTextLinesDetection,
      bool debugTextBlocksDetection,
      int debugPageFilter) {
  _noEmbeddedFontFilesParsing = noEmbeddedFontFilesParsing;
  _noWordsDehyphenation = noWordsDehyphenation;
  _parseMode = parseMode;
  _debugPdfParsing = debugPdfParsing;
  _debugDiacMarksMerging = debugDiacriticMarksMerging;
  _debugWordsDetection = debugWordsDetection;
  _debugPageSegmentation = debugPageSegmentation;
  _debugTextLinesDetection = debugTextLinesDetection;
  _debugTextBlocksDetection = debugTextBlocksDetection;
  _debugPageFilter = debugPageFilter;
}

// _________________________________________________________________________________________________
PdfToTextPlusPlus::~PdfToTextPlusPlus() = default;

// _________________________________________________________________________________________________
int PdfToTextPlusPlus::process(const std::string& pdfFilePath, PdfDocument* doc,
    std::vector<Timing>* timings) const {
  assert(doc);

  doc->pdfFilePath = pdfFilePath;

  // Load the PDF file. Abort if it couldn't be loaded successfully.
  auto start = high_resolution_clock::now();
  GooString gooPdfFilePath(pdfFilePath);
  std::unique_ptr<PDFDoc> pdfDoc = PDFDocFactory().createPDFDoc(gooPdfFilePath);
  auto end = high_resolution_clock::now();
  if (timings) {
    Timing timingLoading("Loading PDF", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timingLoading);
  }
  if (!pdfDoc->isOk()) {
    return pdfDoc->getErrorCode();
  }

  // Parse the content streams of the PDF file for characters, figures and shapes.
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
  auto timeParsing = duration_cast<milliseconds>(end - start).count();
  if (timings) {
    Timing timingParsing("Parsing PDF", timeParsing);
    timings->push_back(timingParsing);
  }

  // Compute some statistics about the characters, for example: the most frequent font size.
  PdfStatisticsCalculator statistician(doc);
  start = high_resolution_clock::now();
  statistician.computeCharacterStatistics();
  end = high_resolution_clock::now();
  auto timeComputeStatistics = duration_cast<milliseconds>(end - start).count();
  if (timings) {
    Timing timingComputeStatistics("Compute character stats", timeComputeStatistics);
    timings->push_back(timingComputeStatistics);
  }

  // Merge combining diacritical marks with their base characters.
  start = high_resolution_clock::now();
  DiacriticalMarksMerger dmm(doc, _debugDiacMarksMerging, _debugPageFilter);
  dmm.process();
  end = high_resolution_clock::now();
  auto timeMergeDiacriticMarks = duration_cast<milliseconds>(end - start).count();
  if (timings) {
    Timing timingMergeDiacriticMarks("Merge diacritics", timeMergeDiacriticMarks);
    timings->push_back(timingMergeDiacriticMarks);
  }

  // Stop here when the parsing mode is activated (since it is supposed to extract only the
  // characters, figures and shapes from the PDF file).
  if (_parseMode) {
    return 0;
  }

  // Detect the words.
  start = high_resolution_clock::now();
  WordsDetector wd(doc, _debugWordsDetection, _debugPageFilter);
  wd.detect();
  end = high_resolution_clock::now();
  auto timeDetectWords = duration_cast<milliseconds>(end - start).count();
  if (timings) {
    Timing timingDetectWords("Detect words", timeDetectWords);
    timings->push_back(timingDetectWords);
  }

  // Compute some statistics about the words, for example: the most frequent word height and -width.
  start = high_resolution_clock::now();
  statistician.computeWordStatistics();
  end = high_resolution_clock::now();
  timeComputeStatistics = duration_cast<milliseconds>(end - start).count();
  if (timings) {
    Timing timingComputeStatistics("Compute word stats", timeComputeStatistics);
    timings->push_back(timingComputeStatistics);
  }

  // Segment the pages of the document (for identifying columns).
  start = high_resolution_clock::now();
  PageSegmentator ps(doc, _debugPageSegmentation, _debugPageFilter);
  ps.segment();
  end = high_resolution_clock::now();
  auto timeSegmentPages = duration_cast<milliseconds>(end - start).count();
  if (timings) {
    Timing timingSegmentPages("Segment pages", timeSegmentPages);
    timings->push_back(timingSegmentPages);
  }

  // Detect the text lines.
  start = high_resolution_clock::now();
  TextLinesDetector tld(doc, _debugTextLinesDetection, _debugPageFilter);
  tld.detect();
  end = high_resolution_clock::now();
  auto timeDetectTextLines = duration_cast<milliseconds>(end - start).count();
  if (timings) {
    Timing timingDetectTextLines("Detect text lines", timeDetectTextLines);
    timings->push_back(timingDetectTextLines);
  }

  // Detect subscripted and superscripted characters.
  start = high_resolution_clock::now();
  SubSuperScriptsDetector ssd(doc);
  ssd.detect();
  end = high_resolution_clock::now();
  auto timeDetectSubSuperScripts = duration_cast<milliseconds>(end - start).count();
  if (timings) {
    Timing timingDetectSubSuperScripts("Detect scripts", timeDetectSubSuperScripts);
    timings->push_back(timingDetectSubSuperScripts);
  }

  // Compute some statistics about the text lines, for example: the most frequent line indentation.
  start = high_resolution_clock::now();
  statistician.computeTextLineStatistics();
  end = high_resolution_clock::now();
  timeComputeStatistics = duration_cast<milliseconds>(end - start).count();
  if (timings) {
    Timing timingComputeStatistics("Compute line stats", timeComputeStatistics);
    timings->push_back(timingComputeStatistics);
  }

  // Detect the text blocks.
  start = high_resolution_clock::now();
  TextBlocksDetector tbd(doc, _debugTextBlocksDetection, _debugPageFilter);
  tbd.detect();
  end = high_resolution_clock::now();
  auto timeDetectTextBlocks = duration_cast<milliseconds>(end - start).count();
  if (timings) {
    Timing timingDetectTextBlocks("Detect text blocks", timeDetectTextBlocks);
    timings->push_back(timingDetectTextBlocks);
  }

  // Detect the reading order of the text blocks.
  start = high_resolution_clock::now();
  ReadingOrderDetector rod(doc);
  rod.detect();
  end = high_resolution_clock::now();
  auto timeDetectReadingOrder = duration_cast<milliseconds>(end - start).count();
  if (timings) {
    Timing timingDetectReadingOrder("Detect reading order", timeDetectReadingOrder);
    timings->push_back(timingDetectReadingOrder);
  }

  // Dehyphenate words, if not deactivated by the user.
  if (!_noWordsDehyphenation) {
    start = high_resolution_clock::now();
    WordsDehyphenator wdh(doc);
    wdh.dehyphenate();
    end = high_resolution_clock::now();
    auto timeDehyphenateWords = duration_cast<milliseconds>(end - start).count();
    if (timings) {
      Timing timingDehyphenateWords("Dehyphenate words", timeDehyphenateWords);
      timings->push_back(timingDehyphenateWords);
    }
  }

  return 0;
}
