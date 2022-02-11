// Copyright 2021, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Claudius Korzen <korzen@cs.uni-freiburg.de>
//
// Modified under the Poppler project - http://poppler.freedesktop.org

#include <chrono>  // std::chrono::high_resolution_clock, etc.
#include <cstring>
#include <iomanip>  // setw, setprecision
#include <iostream>
#include <locale>  // imbue

#include <poppler/GlobalParams.h>

#include "./utils/parseargs.h"
#include "./utils/Utils.h"
#include "PdfToTextPlusPlus.h"

#include "./serializers/CharactersJsonlSerializer.h"
#include "./serializers/TextBlocksJsonlSerializer.h"
#include "./serializers/TextSerializer.h"
#include "./serializers/WordsJsonlSerializer.h"
#include "./PdfDocumentVisualizer.h"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

// =================================================================================================
// Parameters.

static bool printVersion = false;
static bool printHelp = false;
static bool addControlCharacters = false;
static bool addSemanticRoles = false;
static bool excludeSubSuperscripts = false;
static bool ignoreEmbeddedFontFiles = false;
static char serializeCharactersFilePath[256] = "";
static char serializeWordsFilePath[256] = "";
static char serializeTextBlocksFilePath[256] = "";
static bool visualizeCharacters = false;
static bool visualizeWords = false;
static bool visualizeTextLines = false;
static bool visualizeTextBlocks = false;
static bool visualizePageSegments = false;
static bool visualizeNonTextElements = false;
static bool visualizeReadingOrder = false;
static bool visualizeBlockDetectionCuts = false;
static bool visualizeReadingOrderCuts = false;
static char visualizeFilePath[256] = "";

static const ArgDesc argDesc[] = {
  { "--add-control-characters", argFlag, &addControlCharacters, 0,
      "Prepend each emphasized text block with \"^A\" (\"start of heading\") and mark each "
      "page break with \"^L\" (\"form feed\")." },
  { "--add-semantic-roles", argFlag, &addSemanticRoles, 0,
      "Prepend each text block with its semantic role." },
  { "--exclude-sub-super-scripts", argFlag, &excludeSubSuperscripts, 0,
      "Don't print sub- and superscripts." },
  { "--ignore-embedded-font-files", argFlag, &ignoreEmbeddedFontFiles, 0,
      "Don't parse the embedded font files for more accurate font information. This will usually "
      "result in a faster extraction process but also in less accurate extraction results." },
  { "--serialize-characters", argString, &serializeCharactersFilePath,
      sizeof(serializeCharactersFilePath), "Write the extracted characters together with their "
      "layout information in JSONL format to the specified path." },
  { "--serialize-words", argString, &serializeWordsFilePath, sizeof(serializeWordsFilePath),
      "Write the extracted words with their layout information in JSONL format to the specified "
      "path." },
  { "--serialize-text-blocks", argString, &serializeTextBlocksFilePath,
      sizeof(serializeTextBlocksFilePath), "Write the extracted text blocks together with their "
      "layout information in JSONL format to the specified path." },
  { "--visualization-path", argString, &visualizeFilePath, sizeof(visualizeFilePath),
      "Create a visualization of the extraction result (that is: a copy of the input PDF file, "
      "with different annotations added for debugging purposes) and write it to the specified "
      "path. Which annotations will be added to this visualization can be controlled via the "
      "--visualize-* flags; see the respective help messages of these flags below for more "
      "details." },
  { "--visualize-characters", argFlag, &visualizeCharacters, 0,
      "Draw the bounding boxes of the extracted characters to the visualization." },
  { "--visualize-words", argFlag, &visualizeWords, 0,
      "Draw the bounding boxes of the extracted words to the visualization." },
  { "--visualize-text-lines", argFlag, &visualizeTextLines, 0,
      "Draw the bounding boxes of the extracted text lines to the visualization." },
  { "--visualize-text-blocks", argFlag, &visualizeTextBlocks, 0,
      "Draw the bounding boxes and the semantic roles of the extracted text blocks to the "
      "visualization." },
  { "--visualize-segments", argFlag, &visualizePageSegments, 0,
      "Draw the bounding boxes of the detected segments to the visualization." },
  { "--visualize-non-text-elements", argFlag, &visualizeNonTextElements, 0,
      "Draw the bounding boxes of the extracted non-text elements (i.e., figures and shapes) to "
      "the visualization." },
  { "--visualize-reading-order", argFlag, &visualizeReadingOrder, 0,
      "Add annotations that visualizes the detected reading order of the text blocks to the "
      "visualization." },
  { "--visualize-block-detection-cuts", argFlag, &visualizeBlockDetectionCuts, 0,
      "Add annotations that visualizes the XY-cuts made to detect the text blocks." },
  { "--visualize-reading-order-cuts", argFlag, &visualizeReadingOrderCuts, 0,
      "Add annotations that visualizes the XY-cuts made to detect the reading order." },
  { "-v", argFlag, &printVersion, 0,
      "Print the copyright and version information." },
  { "-h", argFlag, &printHelp, 0,
      "Print usage information." },
  { "-help", argFlag, &printHelp, 0,
      "Print usage information." },
  { "--help", argFlag, &printHelp, 0,
      "Print usage information." },
  { "-?", argFlag, &printHelp, 0,
      "Print usage information." },
  {}
};

// =================================================================================================
// Print methods.

void printHelpInfo();
void printVersionInfo();

// This method prints the help information to stdout.
void printHelpInfo() {
  printUsage("pdftotext++", "<pdf-file> [<text-file>]", argDesc);
}

// This method prints the copyright and version information to stdout.
void printVersionInfo() {
  std::cout << "Version: 0.1" << std::endl;
}

// =================================================================================================

int main(int argc, char *argv[]) {
  // Seed the random generator (needed to, for example, create the unique ids of text elements).
  srand((unsigned) time(NULL) * getpid());

  // Parse the command line arguments.
  bool ok = parseArgs(argDesc, &argc, argv);
  if (!ok || argc < 2) {
    printHelpInfo();
    return 99;
  }

  // Print the help message if requested.
  if (printHelp) {
    printHelpInfo();
    return 0;
  }

  // Print the version information if requested.
  if (printVersion) {
    printVersionInfo();
    return 0;
  }

  // Initialize the global parameters, needed by Poppler.
  globalParams = std::make_unique<GlobalParams>();

  // Get the specified path to the input PDF file.
  std::string pdfFilePathStr(argv[1]);

  // ------------
  // Compute the extraction result.

  PdfToTextPlusPlus pdfToTextPlusPlus(!ignoreEmbeddedFontFiles);
  PdfDocument doc;
  std::vector<Timing> timings;

  int status = pdfToTextPlusPlus.process(pdfFilePathStr, &doc, &timings);

  // ------------
  // Process the extraction result.

  // Get the specified path to the output file.
  std::string outputFilePathStr;
  if (argc > 2) {
    outputFilePathStr = std::string(argv[2]);
  } else {
    // The path is *not* explicitly specified by the user. Compose the path from the PDF file path.
    const char* p = pdfFilePathStr.c_str() + pdfFilePathStr.length() - 4;

    // If the PDF path is "file.pdf" or "file.PDF", define the output file path as "file.txt".
    // If the PDF path is "file.xxx", define the output file path as "file.xxx.txt".
    if (!strcmp(p, ".pdf") || !strcmp(p, ".PDF")) {
      outputFilePathStr = std::string(pdfFilePathStr.c_str(), pdfFilePathStr.length() - 4);
    } else {
      outputFilePathStr = pdfFilePathStr;
    }
    outputFilePathStr.append(".txt");
  }

  // Write the extracted text to the output file.
  if (!outputFilePathStr.empty()) {
    auto start = high_resolution_clock::now();
    TextSerializer serializer(&doc, addControlCharacters, addSemanticRoles, excludeSubSuperscripts);
    serializer.serialize(outputFilePathStr);
    auto end = high_resolution_clock::now();
    Timing timingLoading("Serialize text", duration_cast<milliseconds>(end - start).count());
    timings.push_back(timingLoading);
  }

  // Write the extracted characters to the specified file if required.
  const std::string serializeCharactersFilePathStr(serializeCharactersFilePath);
  if (!serializeCharactersFilePathStr.empty()) {
    auto start = high_resolution_clock::now();
    CharactersJsonlSerializer serializer(&doc);
    serializer.serialize(serializeCharactersFilePathStr);
    auto end = high_resolution_clock::now();
    Timing timingSerializeCharacters("Serialize characters",
        duration_cast<milliseconds>(end - start).count());
    timings.push_back(timingSerializeCharacters);
  }

  // Write the extracted words to the specified file if required.
  const std::string serializeWordsFilePathStr(serializeWordsFilePath);
  if (!serializeWordsFilePathStr.empty()) {
    auto start = high_resolution_clock::now();
    WordsJsonlSerializer serializer(&doc);
    serializer.serialize(serializeWordsFilePathStr);
    auto end = high_resolution_clock::now();
    Timing timingSerializeWords("Serialize words",
        duration_cast<milliseconds>(end - start).count());
    timings.push_back(timingSerializeWords);
  }

  // Write the extracted text blocks to the specified file if required.
  const std::string serializeTextBlocksFilePathStr(serializeTextBlocksFilePath);
  if (!serializeTextBlocksFilePathStr.empty()) {
    auto start = high_resolution_clock::now();
    TextBlocksJsonlSerializer serializer(&doc);
    serializer.serialize(serializeTextBlocksFilePathStr);
    auto end = high_resolution_clock::now();
    Timing timingSerializeWords("Serialize text blocks",
        duration_cast<milliseconds>(end - start).count());
    timings.push_back(timingSerializeWords);
  }

  // Create the visualization.
  const std::string visualizeFilePathStr(visualizeFilePath);
  if (!visualizeFilePathStr.empty()) {
    auto start = high_resolution_clock::now();
    PdfDocumentVisualizer visualizer(pdfFilePathStr);
    if (visualizeCharacters) {
      visualizer.visualizeGlyphs(doc, blue);
    }
    if (visualizeNonTextElements) {
      visualizer.visualizeNonTextElements(doc, blue);
    }
    if (visualizeWords) {
      visualizer.visualizeWords(doc, blue);
    }
    if (visualizeTextLines) {
      visualizer.visualizeTextLines(doc, blue);
    }
    if (visualizeTextBlocks) {
      visualizer.visualizeTextBlocks(doc, blue);
    }
    if (visualizePageSegments) {
      visualizer.visualizePageSegments(doc, blue);
    }
    if (visualizeReadingOrder) {
      visualizer.visualizeReadingOrder(doc, blue);
    }
    if (visualizeBlockDetectionCuts) {
      visualizer.visualizeTextBlockDetectionCuts(doc, blue);
    }
    if (visualizeReadingOrderCuts) {
      visualizer.visualizeReadingOrderCuts(doc, blue);
    }
    visualizer.save(visualizeFilePathStr);
    auto end = high_resolution_clock::now();
    Timing timingVisualizing("Visualize", duration_cast<milliseconds>(end - start).count());
    timings.push_back(timingVisualizing);
  }

  // Print the timings.
  std::cout.imbue(std::locale(""));  // Print the values with thousands separator.
  std::cout << std::fixed;           // Print the values with a precision of one decimal point.
  std::cout << std::setprecision(1);

  int64_t timeTotal = 0;
  for (const auto& timing : timings) { timeTotal += timing.time; }

  std::cerr << "\033[1m" << "Finished in " << timeTotal << " ms." << "\033[22m" << std::endl;

  for (const auto& timing : timings) {
    std::string prefix = " * " + timing.description + ":";
    std::cerr << std::left << std::setw(25) << prefix;
    std::cerr << std::right << std::setw(4) << timing.time << " ms ";
    std::cerr << "(" << round(timing.time / static_cast<double>(timeTotal) * 100, 1) << "%)";
    std::cerr << std::endl;
  }

  return status;
}
