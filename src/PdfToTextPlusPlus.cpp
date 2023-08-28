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
#include "./PdfDocument.h"
#include "./PdfParsing.h"
#include "./PdfToTextPlusPlus.h"
#include "./ReadingOrderDetection.h"
#include "./StatisticsCalculation.h"
#include "./SubSuperScriptsDetection.h"
#include "./TextBlocksDetection.h"
#include "./TextLinesDetection.h"
#include "./Types.h"
#include "./WordsDehyphenation.h"
#include "./WordsDetection.h"

using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using std::make_unique;
using std::string;
using std::unique_ptr;
using std::vector;

using ppp::config::Config;
using ppp::modules::DiacriticalMarksMerging;
using ppp::modules::PageSegmentation;
using ppp::modules::PdfParsing;
using ppp::modules::ReadingOrderDetection;
using ppp::modules::StatisticsCalculation;
using ppp::modules::SubSuperScriptsDetection;
using ppp::modules::TextBlocksDetection;
using ppp::modules::TextLinesDetection;
using ppp::modules::WordsDehyphenation;
using ppp::modules::WordsDetection;
using ppp::types::PdfDocument;
using ppp::types::Timing;

// =================================================================================================

namespace ppp {

// _________________________________________________________________________________________________
PdfToTextPlusPlus::PdfToTextPlusPlus(const Config* config, bool parseMode) {
  _config = config;
  _parseMode = parseMode;
}

// _________________________________________________________________________________________________
PdfToTextPlusPlus::~PdfToTextPlusPlus() = default;

// _________________________________________________________________________________________________
int PdfToTextPlusPlus::process(
    const string* pdfFilePath,
    PdfDocument* doc,
    vector<Timing>* timings) const {
  assert(doc);

  doc->pdfFilePath = *pdfFilePath;

  // Initialize the global parameters, needed by Poppler.
  globalParams = make_unique<GlobalParams>();

  // (1) Load the PDF file. Abort if it couldn't be loaded successfully.
  auto start = high_resolution_clock::now();
  GooString gooPdfFilePath(*pdfFilePath);
  unique_ptr<PDFDoc> pdfDoc = PDFDocFactory().createPDFDoc(gooPdfFilePath);
  auto end = high_resolution_clock::now();
  auto duration = duration_cast<milliseconds>(end - start).count();
  if (timings) {
    timings->push_back(Timing("Load PDF", duration));
  }
  if (!pdfDoc->isOk()) {
    return pdfDoc->getErrorCode();
  }

  // (2) Parse the content streams of the PDF file for the characters, graphics and shapes.
  PdfParsing pp(doc, _config->pdfParsing);
  start = high_resolution_clock::now();
  pdfDoc->displayPages(
    &pp,
    1,  // firstPage
    pdfDoc->getNumPages(),  // lastPage
    _config->pdfParsing.hDPI,  // hDPI
    _config->pdfParsing.vDPI,  // vDPI
    0,  // rotation
    true,  // useMediaBox
    false,  // crop
    false);  // printing
  end = high_resolution_clock::now();
  duration = duration_cast<milliseconds>(end - start).count();
  if (timings) {
    timings->push_back(Timing("Parse PDF", duration));
  }

  // (3) Compute some statistics about the characters, for example: the most frequent font size.
  StatisticsCalculation sc(doc, _config->statisticsCalculation);
  if (!_config->statisticsCalculation.disable) {
    start = high_resolution_clock::now();
    sc.computeGlyphStatistics();
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    if (timings) {
      timings->push_back(Timing("Compute glyph stats", duration));
    }
  }

  // (4) Merge combining diacritical marks with their base characters.
  if (!_config->diacriticalMarksMerging.disable) {
    start = high_resolution_clock::now();
    DiacriticalMarksMerging dmm(doc, _config->diacriticalMarksMerging);
    dmm.process();
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    if (timings) {
      timings->push_back(Timing("Merge diacritics", duration));
    }
  }

  // Stop here when the parsing mode is activated (since it is supposed to extract only the
  // characters, graphics and shapes from the PDF file).
  // TODO(korzen): Replace the parse mode with the new disable flags in the different configs.
  if (_parseMode) {
    return 0;
  }

  // (5) Detect the words.
  if (!_config->wordsDetection.disable) {
    start = high_resolution_clock::now();
    WordsDetection wd(doc, _config->wordsDetection);
    wd.process();
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    if (timings) {
      timings->push_back(Timing("Detect words", duration));
    }
  }

  // (6) Compute some statistics about the words, for example: the most frequent word height.
  if (!_config->statisticsCalculation.disable) {
    start = high_resolution_clock::now();
    sc.computeWordStatistics();
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    if (timings) {
      timings->push_back(Timing("Compute word stats", duration));
    }
  }

  // (7) Segment the pages of the document (for identifying columns).
  if (!_config->pageSegmentation.disable) {
    start = high_resolution_clock::now();
    PageSegmentation ps(doc, _config->pageSegmentation);
    ps.process();
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    if (timings) {
      timings->push_back(Timing("Segment pages", duration));
    }
  }

  // (8) Detect the text lines.
  if (!_config->textLinesDetection.disable) {
    start = high_resolution_clock::now();
    TextLinesDetection tld(doc, _config->textLinesDetection);
    tld.process();
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    if (timings) {
      timings->push_back(Timing("Detect lines", duration));
    }

    // FIXME(korzen): Find another solution. It is currently needed only for testing.
    for (auto* page : doc->pages) {
      page->textLines.clear();
      for (auto* segment : page->segments) {
        for (auto* line : segment->lines) {
          page->textLines.push_back(line);
        }
      }
    }
  }

  // (9) Detect subscripted and superscripted characters.
  if (!_config->subSuperScriptsDetection.disable) {
    start = high_resolution_clock::now();
    SubSuperScriptsDetection ssd(doc, _config->subSuperScriptsDetection);
    ssd.process();
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    if (timings) {
      timings->push_back(Timing("Detect sub-/superscripts", duration));
    }
  }

  // (10) Compute some statistics about the text lines, for example: the most frequent indentation.
  if (!_config->statisticsCalculation.disable) {
    start = high_resolution_clock::now();
    sc.computeTextLineStatistics();
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    if (timings) {
      timings->push_back(Timing("Compute line stats", duration));
    }
  }

  // (11) Detect the text blocks.
  if (!_config->textBlocksDetection.disable) {
    start = high_resolution_clock::now();
    TextBlocksDetection tbd(doc, _config->textBlocksDetection);
    tbd.process();
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    if (timings) {
      timings->push_back(Timing("Detect blocks", duration));
    }
  }

  // (12) Detect the reading order of the text blocks.
  if (!_config->readingOrderDetection.disable) {
    start = high_resolution_clock::now();
    ReadingOrderDetection rod(
      doc,
      _config->readingOrderDetection,
      _config->semanticRolesPrediction);
    rod.detect();
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    if (timings) {
      timings->push_back(Timing("Detect reading order", duration));
    }
  }

  // (13) Dehyphenate words, if not deactivated by the user.
  if (!_config->wordsDehyphenation.disable) {
    start = high_resolution_clock::now();
    WordsDehyphenation wdh(doc, _config->wordsDehyphenation);
    wdh.dehyphenate();
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    if (timings) {
      timings->push_back(Timing("Dehyphenate words", duration));
    }
  }

  // FIXME(korzen): Find another solution. It is currently needed only for testing.
  for (auto* page : doc->pages) {
    page->textLines.clear();
    for (auto* block : page->blocks) {
      for (auto* line : block->lines) {
        page->textLines.push_back(line);
      }
    }
  }

  return 0;
}

}  // namespace ppp
