/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <chrono>  // std::chrono::high_resolution_clock, etc.
#include <iostream>
#include <string>

#include <goo/GooString.h>
#include <poppler/PDFDoc.h>
#include <poppler/PDFDocFactory.h>

#include "./TextOutputDev.h"
#include "./DiacriticMarksCombiner.h"
#include "./PageSegmentator.h"
#include "./PdfDocumentStatisticsCalculator.h"
#include "./PdfToTextPlusPlus.h"
#include "./ReadingOrderDetector.h"
#include "./SubSuperScriptsDetector.h"
#include "./TextBlockDetector.h"
#include "./TextLineDetector.h"
#include "./WordsDehyphenator.h"
#include "./WordsTokenizer.h"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

// The resolution in DPI.
static double resolution = 72.0;


// _________________________________________________________________________________________________
PdfToTextPlusPlus::PdfToTextPlusPlus(bool parseEmbeddedFontFiles, bool disableWordsDehyphenation,
    TextUnit targetTextUnit) {
  _parseEmbeddedFontFiles = parseEmbeddedFontFiles;
  _disableWordsDehyphenation = disableWordsDehyphenation;
  _targetTextUnit = targetTextUnit;
}

// _________________________________________________________________________________________________
PdfToTextPlusPlus::~PdfToTextPlusPlus() = default;

// _________________________________________________________________________________________________
int PdfToTextPlusPlus::process(const std::string& pdfFilePath, PdfDocument* doc,
    std::vector<Timing>* timings) {
  // Load the PDF file.
  auto start = high_resolution_clock::now();
  GooString gooPdfFilePath(pdfFilePath);
  std::unique_ptr<PDFDoc> pdfDoc = PDFDocFactory().createPDFDoc(gooPdfFilePath);
  auto end = high_resolution_clock::now();
  if (timings) {
    Timing timingLoading("Loading PDF", duration_cast<milliseconds>(end - start).count());
    timings->push_back(timingLoading);
  }

  if (!pdfDoc->isOk()) {
    return 1;
  }

  TextOutputDev out(_parseEmbeddedFontFiles, doc);
  if (!out.isOk()) {
    return 2;
  }

  PdfDocumentStatisticsCalculator statistician(doc);

  if (_targetTextUnit >= TextUnit::GLYPHS) {
    // Extract the contents of the PDF pages.
    start = high_resolution_clock::now();
    pdfDoc->displayPages(
      &out,
      1,  // firstPage
      pdfDoc->getNumPages(),  // lastPage
      resolution,  // hDPI
      resolution,  // vDPI
      0,  // rotation
      true,  // useMediaBox
      false,  // crop
      false);  // printing

    end = high_resolution_clock::now();
    auto timeParsing = duration_cast<milliseconds>(end - start).count();

    // Compute statistics about the glyphs, for example: the most freq. font size among the glyphs.
    start = high_resolution_clock::now();
    statistician.computeGlyphStatistics();
    end = high_resolution_clock::now();
    auto timeComputeStatistics = duration_cast<milliseconds>(end - start).count();

    // Combine diacritic marks with their base glyphs.
    start = high_resolution_clock::now();
    DiacriticMarksCombiner dmCombiner(doc);
    dmCombiner.combine();
    end = high_resolution_clock::now();
    auto timeCombineDiacriticMarks = duration_cast<milliseconds>(end - start).count();

    if (timings) {
      Timing timingParsing("Parsing PDF", timeParsing);
      timings->push_back(timingParsing);

      Timing timingComputeStatistics("Compute glyph stats", timeComputeStatistics);
      timings->push_back(timingComputeStatistics);

      Timing timingCombineDiacriticMarks("Combine diacritics", timeCombineDiacriticMarks);
      timings->push_back(timingCombineDiacriticMarks);
    }
  }

  if (_targetTextUnit >= TextUnit::WORDS) {
    // Detect the words.
    start = high_resolution_clock::now();
    WordsTokenizer wordsTokenizer(doc);
    wordsTokenizer.tokenize();
    end = high_resolution_clock::now();
    auto timeDetectWords = duration_cast<milliseconds>(end - start).count();

    // Compute statistics about the words, for example: the most frequent word height and -width.
    start = high_resolution_clock::now();
    statistician.computeWordStatistics();
    end = high_resolution_clock::now();
    auto timeComputeStatistics = duration_cast<milliseconds>(end - start).count();

    if (timings) {
      Timing timingDetectWords("Detect words", timeDetectWords);
      timings->push_back(timingDetectWords);

      Timing timingComputeStatistics("Compute word stats", timeComputeStatistics);
      timings->push_back(timingComputeStatistics);
    }
  }

  if (_targetTextUnit >= TextUnit::TEXT_LINES) {
    // Segment each page of the document.
    start = high_resolution_clock::now();
    PageSegmentator pageSegmentator(doc);
    pageSegmentator.segment();
    end = high_resolution_clock::now();
    auto timeSegmentPages = duration_cast<milliseconds>(end - start).count();

    // Detect the text lines.
    start = high_resolution_clock::now();
    TextLineDetector textLineDetector(doc);
    textLineDetector.detect();
    end = high_resolution_clock::now();
    auto timeDetectTextLines = duration_cast<milliseconds>(end - start).count();

    // Compute statistics about the line, for example: the most frequent line indentation.
    start = high_resolution_clock::now();
    statistician.computeLineStatistics();
    end = high_resolution_clock::now();
    auto timeComputeStatistics = duration_cast<milliseconds>(end - start).count();

    // Detect sub- and superscripts.
    start = high_resolution_clock::now();
    SubSuperScriptsDetector scriptsDetector(doc);
    scriptsDetector.detect();
    end = high_resolution_clock::now();
    auto timeDetectSubSuperScripts = duration_cast<milliseconds>(end - start).count();

    if (timings) {
      Timing timingSegmentPages("Segment pages", timeSegmentPages);
      timings->push_back(timingSegmentPages);

      Timing timingDetectTextLines("Detect text lines", timeDetectTextLines);
      timings->push_back(timingDetectTextLines);

      Timing timingComputeStatistics("Compute line stats", timeComputeStatistics);
      timings->push_back(timingComputeStatistics);

      Timing timingDetectSubSuperScripts("Detect scripts", timeDetectSubSuperScripts);
      timings->push_back(timingDetectSubSuperScripts);
    }
  }

  if (_targetTextUnit >= TextUnit::TEXT_BLOCKS) {
    // Detect the text blocks.
    start = high_resolution_clock::now();
    TextBlockDetector textBlockDetector(doc);
    textBlockDetector.detect();
    end = high_resolution_clock::now();
    auto timeDetectTextBlocks = duration_cast<milliseconds>(end - start).count();

    if (timings) {
      Timing timingDetectTextBlocks("Detect text blocks", timeDetectTextBlocks);
      timings->push_back(timingDetectTextBlocks);
    }
  }

  if (_targetTextUnit >= TextUnit::PARAGRAPHS) {
    // Detect the reading order of the text blocks.
    start = high_resolution_clock::now();
    ReadingOrderDetector readingOrderDetector(doc);
    readingOrderDetector.detect();
    end = high_resolution_clock::now();
    auto timeDetectReadingOrder = duration_cast<milliseconds>(end - start).count();

    // Dehyphenate words.
    auto timeDehyphenateWords = 0;
    if (!_disableWordsDehyphenation) {
      start = high_resolution_clock::now();
      WordsDehyphenator wordsDehyphenator(doc);
      wordsDehyphenator.dehyphenate();
      end = high_resolution_clock::now();
      timeDehyphenateWords = duration_cast<milliseconds>(end - start).count();
    }

    if (timings) {
      Timing timingDetectReadingOrder("Detect reading order", timeDetectReadingOrder);
      timings->push_back(timingDetectReadingOrder);

      Timing timingDehyphenateWords("Dehyphenate words", timeDehyphenateWords);
      timings->push_back(timingDehyphenateWords);
    }
  }

  return 0;
}
