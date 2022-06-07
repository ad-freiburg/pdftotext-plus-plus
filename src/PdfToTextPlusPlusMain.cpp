/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <poppler/GlobalParams.h>

#include <cmath>  // round
#include <chrono>  // std::chrono::high_resolution_clock, etc.
#include <iomanip>  // setw, setprecision
#include <iostream>
#include <locale>  // imbue
#include <memory>  // std::make_unique
#include <string>
#include <vector>

#include "./serializers/JsonlSerializer.h"
#include "./serializers/TextSerializer.h"

#include "./utils/Log.h"
#include "./utils/MathUtils.h"
#include "./utils/parseargs.h"

#include "./PdfDocumentVisualizer.h"
#include "./PdfToTextPlusPlus.h"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

// =================================================================================================
// Parameters.

const char* description =
"This tool is an extension of Poppler's pdftotext (https://poppler.freedesktop.org/), with the\n"
"purpose to extract text from PDF files more accurately. The included features are:\n\n"
"• \033[34mWords Detection\033[0m\n"
"  PDF is a format that does not provide the text word-wise, but character-wise. Pdftotext++\n"
"  merges the characters to accurate words by analyzing different layout information of the\n"
"  characters (e.g., the spacings in between, or the font sizes, or the rotations).\n"
"  Special symbols, like ligatures or characters with combining diacritical marks, are translated\n"
"  to the characters they actually represent (e.g., \"ﬁ\" is translated to \"fi\").\n"
"  Words that are hyphenated and appear broken in two parts are merged (\"dehyphenated\") to a\n"
"  single word, with the hyphen between the parts kept if the word is a compound word (like\n"
"  \"well-known\") and removed when the word is not a compound word.\n\n"
"• \033[34mText Blocks Detection\033[0m\n"
"  A PDF typically consists of one or more text blocks (by a text block, we mean a group of text\n"
"  that logically belong together and that is recognizably set off from other text blocks) which\n"
"  can play different semantic roles in the PDF (e.g., \"title\", \"heading\", or \"footnote\").\n"
"  Pdftotext++ detects the beginning and end of text blocks by analyzing different layout\n"
"  information. The semantic roles are detected by using a neural network.\n\n"
"• \033[34mReading Order Detection\033[0m\n"
"  A PDF does not necessarily store the characters in the natural reading order. For example,\n"
"  PDFs with a multi-column layout can store the characters in an order interleaving between the\n"
"  columns. Pdftotext++ detects the natural reading order by ...\n\n"
"• \033[34mSerialization\033[0m\n"
"  Pdftotext++ allows to output the extracted text in the following formats:\n"
"  - Continuous Text: This is the default format. It contains the extracted text in plain text,\n"
"      with the words of a text block separated by whitespaces and the text blocks separated by\n"
"      blank lines.\n"
"  - JSONL: A (structured) format that provides the extracted text broken down by a given text\n"
"      unit (e.g., \"glyphs\", \"words\", or \"blocks\"). The format contains one line per\n"
"      instance of the respective unit (e.g., one line per word if the unit is \"words\"), each\n"
"      providing all available layout information about the instance. Here is an example line\n"
"      showing which information are provided for a word:\n"
"      {\"type\": \"word\", \"page\": 9, \"minX\": 448.8, \"minY\": 635.9, \"maxX\": 459.4,\n"
"        \"maxY\": 647.6, \"font\": \"RSEUZH+CMBX9\", \"fontSize\": 8.9, \"text\": \"panel\"}\n"
"  Note the different --output-* options described below. If one or more --output-* option is\n"
"  used, the extracted text will be outputted in JSONL format. The --output-* options can be\n"
"  combined; for example, if you use the --output-glyphs and --output-words option\n"
"  simultanuously, the produced JSONL will contain one line per glyph and word. If none of the\n"
"  --output-* options is used, the text will be outputted as continuous text instead.\n\n"
"• \033[34mVisualization\033[0m\n"
"  Pdftotext++ allows to create a viusalization of the extracted text, that is: a copy of the PDF\n"
"  file, with different annotations added, for example: the bounding boxes and the semantic roles\n"
"  of the extracted text blocks. This is particularly useful for debugging the extracted text\n"
"  regarding different aspects). Which annotations are added to the visualization can be\n"
"  controlled via the --visualize-* flags described below. Note that the --visualize-* options\n"
"  must be used in conjunction with the --visualization-path option. Otherwise, no visualization\n"
"  will be created.";

static bool printVersion = false;
static bool printHelp = false;
static bool addControlCharacters = false;
static bool addSemanticRoles = false;
static bool noSubSuperscripts = false;
static bool noEmbeddedFontFiles = false;
static bool noDehyphenation = false;
static bool parseMode = false;
static bool outputPages = false;
static bool outputGlyphs = false;
static bool outputFigures = false;
static bool outputShapes = false;
static bool outputWords = false;
static bool outputTextBlocks = false;
static bool visualizeGlyphs = false;
static bool visualizeWords = false;
static bool visualizeTextLines = false;
static bool visualizeTextBlocks = false;
static bool visualizePageSegments = false;
static bool visualizeFigures = false;
static bool visualizeShapes = false;
static bool visualizeGraphics = false;
static bool visualizeReadingOrder = false;
static bool visualizeBlockDetectionCuts = false;
static bool visualizeReadingOrderCuts = false;
static char visualizeFilePath[256] = "";
static bool debugPdfParsing = false;
static bool debugDiacriticMarksMerging = false;
static bool debugWordsDetection = false;
static bool debugPageSegmentation = false;
static bool debugTextLinesDetection = false;
static bool debugTextBlocksDetection = false;
static int debugPageFilter = -1;
static bool printRunningTimes = false;

static const ArgDesc options[] = {
  { "--control-characters", argFlag, &addControlCharacters, 0,
      "Add the following control characters to the continuous text: "
      "\"^A\" (start of heading) in front of each emphasized text block; "
      "\"^L\" (form feed) between two text blocks when there is a page break in between." },
  { "--semantic-roles", argFlag, &addSemanticRoles, 0,
      "Prefix each text block by its semantic role in the continuous text." },
  { "--no-scripts", argFlag, &noSubSuperscripts, 0,
      "Remove subscripts and superscripts in the continuous text." },
  { "--no-embedded-font-files", argFlag, &noEmbeddedFontFiles, 0,
      "Disable embedded font file parsing. The consequence is a faster extraction process, but "
      "a less accurate extraction result." },
  { "--no-dehyphenation", argFlag, &noDehyphenation, 0,
      "Disable words dehyphenation. The consequence is that each part into which a hyphenated "
      "word is split will appear as a separate word in the continuous text and the JSONL." },
  { "--parse-mode", argFlag, &parseMode, 0,
      "Activate the parsing mode. This mode only parses the content streams of the PDF file for "
      "information about the glyphs, figures, and shapes. It does not detect words, text lines, "
      "and text blocks. To output the extracted information about the glyphs, figures and shapes, "
      "you can use the --output-glyphs, --output-figures and --output-shapes options "
      "below." },
  { "--output-pages", argFlag, &outputPages, 0, "Instead of continuous text, output JSONL "
      "with all extracted information about the pages of the PDF file (e.g., the widths and "
      "heights)." },
  { "--output-glyphs", argFlag, &outputGlyphs, 0, "Instead of continuous text, output "
      "JSONL with all extracted information about the glyphs in the PDF file (e.g., the positions "
      "and fonts)." },
  { "--output-figures", argFlag, &outputFigures, 0, "Instead of continuous text, output "
      "JSONL with all extracted information about the figures in the PDF file (e.g., the "
      "positions)." },
  { "--output-shapes", argFlag, &outputShapes, 0, "Instead of continuous text, output "
      "JSONL with all extracted information about the shapes in the PDF file (e.g., the "
      "positions)." },
  { "--output-words", argFlag, &outputWords, 0, "Instead of continuous text, output JSONL "
      "with all extracted information about the words in the PDF file (e.g., the positions and "
      "the fonts)." },
  { "--output-text-blocks", argFlag, &outputTextBlocks, 0, "Instead of continuous text, output "
      "JSONL with all extracted information about the text blocks in the PDF file (e.g., the "
      "positions and the fonts)." },
  { "--visualize-glyphs", argFlag, &visualizeGlyphs, 0,
      "Draw the bounding boxes of the glyphs into the visualization file." },
  { "--visualize-graphics", argFlag, &visualizeGraphics, 0,
      "Draw the bounding boxes of the graphics into the visualization file." },
  { "--visualize-figures", argFlag, &visualizeFigures, 0,
      "Draw the bounding boxes of the figures into the visualization file." },
  { "--visualize-shapes", argFlag, &visualizeShapes, 0,
      "Draw the bounding boxes of the shapes into the visualization file." },
  { "--visualize-words", argFlag, &visualizeWords, 0,
      "Draw the bounding boxes of the words into the visualization file." },
  { "--visualize-text-lines", argFlag, &visualizeTextLines, 0,
      "Draw the bounding boxes of the text lines into the visualization file." },
  { "--visualize-text-blocks", argFlag, &visualizeTextBlocks, 0,
      "Draw the bounding boxes and the semantic roles of the text blocks into the "
      "visualization file." },
  { "--visualize-segments", argFlag, &visualizePageSegments, 0,
      "Draw the bounding boxes of the page segments into the visualization file." },
  { "--visualize-reading-order", argFlag, &visualizeReadingOrder, 0,
      "Draw the reading order of the text blocks into the visualization file." },
  { "--visualize-text-block-cuts", argFlag, &visualizeBlockDetectionCuts, 0,
      "Draw the XY-cuts made to detect the text blocks into the visualization file." },
  { "--visualize-reading-order-cuts", argFlag, &visualizeReadingOrderCuts, 0,
      "Draw the XY-cuts made to detect the reading order into the visualization file." },
  { "--visualization-path", argString, &visualizeFilePath, sizeof(visualizeFilePath),
      "The target path for the visualization file. If not specified, no visualization file will "
      "be created, even if one or more of the --visualize-* options above is used." },
  { "--debug-pdf-parsing", argFlag, &debugPdfParsing, 0,
      "Print the debug information produced while parsing the content streams of the PDF file." },
  { "--debug-diacritic-marks-merging", argFlag, &debugDiacriticMarksMerging, 0,
      "Print the debug information produced while merging diacritic marks with their base "
      "characters." },
  { "--debug-words-detection", argFlag, &debugWordsDetection, 0,
      "Print the debug information produced while detecting words." },
  { "--debug-page-segmentation", argFlag, &debugPageSegmentation, 0,
      "Print the debug information produced while segmenting the pages." },
  { "--debug-text-lines-detection", argFlag, &debugTextLinesDetection, 0,
      "Print the debug information produced while detecting text lines." },
  { "--debug-text-blocks-detection", argFlag, &debugTextBlocksDetection, 0,
      "Print the debug information produced while detecting text blocks." },
  { "--debug-page-filter", argInt, &debugPageFilter, 0,
      "When one or more of the --debug-* options above is used, print only the debug information "
      "that is produced while processing the specified page. Note that the page numbers are "
      "1-based; so to print the debug information produced while processing the first page, type "
      "\"--debug-page-filter 1\". " },
  { "--print-running-times", argFlag, &printRunningTimes, 0,
      "Print the running times needed by the different extraction steps, for debugging purposes." },
  { "-v", argFlag, &printVersion, 0,
      "Print the version information." },
  { "-version", argFlag, &printVersion, 0,
      "Print the version information." },
  { "--version", argFlag, &printVersion, 0,
      "Print the version information." },
  { "-h", argFlag, &printHelp, 0,
      "Print the help information." },
  { "-help", argFlag, &printHelp, 0,
      "Print the help information." },
  { "--help", argFlag, &printHelp, 0,
      "Print the help information." },
  { "-?", argFlag, &printHelp, 0,
      "Print the help information." },
  {}
};

// =================================================================================================
// Print methods.

// This method prints the usage information to stdout.
void printUsageInfo() {
  printHelpInfo("pdftotext++", "<pdf-file> [<output-file>]", options, nullptr);
}

// This method prints the help information (= description + usage information) to stdout.
void printHelpInfo() {
  printHelpInfo("pdftotext++", "<pdf-file> [<output-file>]", options, description);
}

// This method prints the copyright and version information to stdout.
void printVersionInfo() {
  std::cout << "Version: 0.1" << std::endl;
}

// =================================================================================================

int main(int argc, char *argv[]) {
  // Seed the random generator (needed to, for example, create random ids for the text elements).
  srand((unsigned) time(NULL) * getpid());

  // Parse the command line arguments.
  bool ok = parseArgs(options, &argc, argv);
  if (!ok) {
    printUsageInfo();
    return 99;
  }

  // Print the help information if requested.
  if (printHelp) {
    printHelpInfo();
    return 0;
  }

  // Print the version information if requested.
  if (printVersion) {
    printVersionInfo();
    return 0;
  }

  if (argc < 2) {
    printUsageInfo();
    return 98;
  }

  // Initialize the global parameters, needed by Poppler.
  globalParams = std::make_unique<GlobalParams>();

  // Get the specified path to the input PDF file.
  std::string pdfFilePathStr(argv[1]);

  // Get the specified path to the output file.
  std::string outputFilePathStr;
  if (argc > 2) {
    outputFilePathStr = std::string(argv[2]);
  }

  // Print the usage information if no output file path is given.
  if (outputFilePathStr.empty()) {
    printUsageInfo();
    return 99;
  }

  // ------------
  // Run pdftotext++

  PdfToTextPlusPlus pdfToTextPlusPlus(
    !noEmbeddedFontFiles,
    noDehyphenation,
    parseMode,
    debugPdfParsing,
    debugDiacriticMarksMerging,
    debugWordsDetection,
    debugPageSegmentation,
    debugTextLinesDetection,
    debugTextBlocksDetection,
    debugPageFilter);
  PdfDocument doc;
  std::vector<Timing> timings;

  try {
    pdfToTextPlusPlus.process(pdfFilePathStr, &doc, &timings);
  } catch (const std::invalid_argument& ia) {
    std::cerr << "An error occurred: " << ia.what() << '\n';
    return 1;
  }

  // ------------
  // Serialize the extraction result.

  auto start = high_resolution_clock::now();
  if (outputPages || outputGlyphs || outputFigures || outputShapes || outputWords
      || outputTextBlocks) {
    JsonlSerializer serializer(&doc,
      outputPages,
      outputGlyphs,
      outputFigures,
      outputShapes,
      outputWords,
      outputTextBlocks);
    serializer.serialize(outputFilePathStr);
  } else {
    TextSerializer serializer(&doc,
      addControlCharacters,
      addSemanticRoles,
      noSubSuperscripts);
    serializer.serialize(outputFilePathStr);
  }
  auto end = high_resolution_clock::now();
  Timing timingSerializeGlyphs("Serialize", duration_cast<milliseconds>(end - start).count());
  timings.push_back(timingSerializeGlyphs);

  // ------------
  // Visualize the extraction result.

  const std::string visualizeFilePathStr(visualizeFilePath);
  if (!visualizeFilePathStr.empty()) {
    auto start = high_resolution_clock::now();
    PdfDocumentVisualizer visualizer(pdfFilePathStr);
    if (visualizeGlyphs) {
      visualizer.visualizeGlyphs(doc, blue);
    }
    if (visualizeFigures) {
      visualizer.visualizeFigures(doc, blue);
    }
    if (visualizeShapes) {
      visualizer.visualizeShapes(doc, blue);
    }
    if (visualizeGraphics) {
      visualizer.visualizeGraphics(doc, blue);
    }
    if (visualizeWords) {
      visualizer.visualizeWords(doc, blue);
    }
    if (visualizeTextLines) {
      visualizer.visualizeTextLines(doc, blue);
    }
    if (visualizeTextBlocks) {
      visualizer.visualizeTextBlocks(doc, red);
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

  if (printRunningTimes) {
    // Print the running timings.
    std::cout.imbue(std::locale(""));  // Print the values with thousands separator.
    std::cout << std::fixed;           // Print the values with a precision of one decimal point.
    std::cout << std::setprecision(1);

    int64_t timeTotal = 0;
    for (const auto& timing : timings) { timeTotal += timing.time; }

    std::cout << "\033[1m" << "Finished in " << timeTotal << " ms." << "\033[22m" << std::endl;

    for (const auto& timing : timings) {
      std::string prefix = " * " + timing.description + ":";
      std::cout << std::left << std::setw(25) << prefix;
      std::cout << std::right << std::setw(4) << timing.time << " ms ";
      double time = math_utils::round(timing.time / static_cast<double>(timeTotal) * 100, 1);
      std::cout << "(" << time << "%)";
      std::cout << std::endl;
    }
  }

  return 0;
}
