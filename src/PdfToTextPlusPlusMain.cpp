/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <poppler/GlobalParams.h>

#include <cmath>  // round
#include <chrono>  // std::chrono::high_resolution_clock
#include <iomanip>  // std::setw, std::setprecision
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

const char* programName = "pdftotext++";

const char* version = "0.1";

const char* description =
"pdftotext++ extracts text from PDF files. It is an extension of Poppler's pdftotext\n"
"(https://poppler.freedesktop.org/) and provides the following features useful for\n"
"applications like search, information retrieval or document analysis:\n\n"
"• \033[34mWords Detection\033[0m\n"
"  PDF is a format that does not provide the included text word-wise, but character-wise.\n"
"  Pdftotext++ has techniques that assembles the words from the characters accurately.\n"
"  \n"
"• \033[34mWords Dehyphenation\033[0m\n"
"  PDF can contain hyphenated words, that is: words that appear broken in two parts, with a\n"
"  hyphen in between. Pdftotext++ merges the parts of hyphenated words to single words, under\n"
"  consideration whether the hyphen needs to be retained because it is part of a compound word\n"
"  (like in \"well-known\") or removed because it is not part of a compound word.\n"
"  \n"
"• \033[34mSplitting of ligatures\033[0m\n"
"  PDF can contain ligatures, that is: symbols that are one character in the PDF, but actually\n"
"  represent multiple characters (like \"ﬁ\" or \"ﬃ\"). Pdftotext++ splits ligatures into the\n"
"  characters they actually represent (e.g., it splits \"ﬁ\" into \"fi\" and \"ﬃ\" into \"ffi\").\n"
"  \n"
"• \033[34mMerging of diacritical marks\033[0m\n"
"  PDF can contain characters with diacritical marks (like ü or à), which are often represented\n"
"  by two characters in the PDF (the base character and the diacritical mark). Pdftotext++ merges\n"
"  them to single characters (e.g., it merges \"a\" and \"`\" to \"à\"). \n"
"  \n"
"• \033[34mText Blocks Detection\033[0m\n"
"  A PDF typically consists of one or more text blocks. By a text block we mean a group of text\n"
"  that logically belong together and that is recognizably set off from other text blocks. Text\n"
"  blocks play different semantic roles in the PDF (e.g., \"title\", \"heading\", \"paragraph\",\n"
"  \"footnote\"). Pdftotext++ detects the beginning and end of text blocks and is able to\n"
"  identify the semantic roles of the text blocks.\n"
"  \n"
"• \033[34mReading Order Detection\033[0m\n"
"  A PDF does not necessarily store the characters in natural reading order. For example, PDFs\n"
"  with a multi-column layout can store the characters in an order interleaving between the\n"
"  columns. Pdftotext++ has techniques to detect multi-column layouts and to correctly detect\n"
"  the natural reading order in such layouts.\n"
"  \n"
"• \033[34mOutput Formats\033[0m\n"
"  Pdftotext++ allows to output the extracted text in the following formats:\n"
"  - \033[36mContinuous Text:\033[0m Contains the extracted text in plain text format, with the\n"
"      words of a text block separated by whitespaces and the text blocks separated by blank lines."
"      \n"
"  - \033[36mJSONL:\033[0m Contains the extracted text in a structured form, broken down by a\n"
"      given text unit (e.g., \"glyphs\", \"words\", or \"blocks\"). It contains one line per\n"
"      instance of the respective unit (e.g., one line per word if the unit is \"words\"), each\n"
"      providing all available layout information about the instance. Here is an example line,\n"
"      showing the general structure of a line and which information are provided for a word:\n"
"      {\"type\": \"word\", \"page\": 9, \"minX\": 448.8, \"minY\": 635.9, \"maxX\": 459.4,\n"
"        \"maxY\": 647.6, \"font\": \"RSEUZH+CMBX9\", \"fontSize\": 8.9, \"text\": \"panel\"}\n"
"  Continuous text is the default format. To output the text in JSONL instead, you can use the\n"
"  different --output-* options. Note that the --output-* options can be combined; for example,\n"
"  if you use --output-characters in conjunction with --output-words, the outputted JSONL\n"
"  contains one line for each character and each word. If one or more --output-* option is used,\n"
"  the output format is JSONL, otherwise the output format is continuous text.\n"
"  \n"
"• \033[34mVisualization\033[0m\n"
"  Pdftotext++ allows to create a visualization of the extracted text, that is: a copy of the PDF\n"
"  file, with different annotations added to it, for example: the bounding boxes or the semantic\n"
"  roles of the extracted text blocks. This is particularly useful for debugging the extracted\n"
"  text with respect to different aspects. Which annotations are added to the visualization can\n"
"  be controlled via the --visualize-* flags. Multiple --visualize-* options can be combined.\n"
"  Note that the --visualize-* options must be used in conjunction with --visualization-path;\n"
"  otherwise, no visualization will be created.";

const char* usage =
"pdftotext++ [options] <pdf-file> <output-file>\n"
"\n"
"This processes the PDF file <pdf-file>. The extracted text is written to the file <output-file>.\n"
"If <output-file> is specified as '-', the extracted text is printed to stdout.";

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
      "\"^L\" (form feed) between two text blocks when there is a page break in between. "
      "Has no effect when used together with one or more --output-* options." },
  { "--semantic-roles", argFlag, &addSemanticRoles, 0,
      "Prefix each text block by its semantic role in the continuous text. "
      "Has no effect when used together with one or more --output-* options." },
  { "--no-scripts", argFlag, &noSubSuperscripts, 0,
      "Remove subscripted and superscripted characters from the continuous text. "
      "Has no effect when used together with one or more --output-* options." },
  { "--no-embedded-font-files", argFlag, &noEmbeddedFontFiles, 0,
      "Disable the parsing of embedded font files. The consequence is a faster extraction process, "
      "but a less accurate extraction result." },
  { "--no-dehyphenation", argFlag, &noDehyphenation, 0,
      "Disable words dehyphenation. The consequence is that each part into which a hyphenated "
      "word is split will appear as a separate word in the continuous text and the JSONL." },
  { "--parse-mode", argFlag, &parseMode, 0,
      "Activate a special mode that only parses the content streams of the PDF file for the "
      "contained characters, figures, and shapes. It does not detect words, text lines, and text "
      "blocks. To output the extracted elements, use the --output-characters, --output-figures\n"
      "and --output-shapes options." },
  { "--output-pages", argFlag, &outputPages, 0, "Instead of continuous text, output JSONL "
      "with all available information about the pages of the PDF file (e.g., the widths and "
      "heights)." },
  { "--output-glyphs", argFlag, &outputGlyphs, 0, "Instead of continuous text, output "
      "JSONL with all available information about the glyphs in the PDF file (e.g., the positions "
      "and fonts)." },
  { "--output-figures", argFlag, &outputFigures, 0, "Instead of continuous text, output "
      "JSONL with all available information about the figures in the PDF file (e.g., the "
      "positions)." },
  { "--output-shapes", argFlag, &outputShapes, 0, "Instead of continuous text, output "
      "JSONL with all available information about the shapes in the PDF file (e.g., the "
      "positions)." },
  { "--output-words", argFlag, &outputWords, 0, "Instead of continuous text, output JSONL "
      "with all available information about the words in the PDF file (e.g., the positions and "
      "the fonts)." },
  { "--output-text-blocks", argFlag, &outputTextBlocks, 0, "Instead of continuous text, output "
      "JSONL with all available information about the text blocks in the PDF file (e.g., the "
      "positions and the fonts)." },
  { "--visualize-glyphs", argFlag, &visualizeGlyphs, 0,
      "Add the bounding boxes of the glyphs to the visualization. Must be used together with "
      "--visualization-path." },
  { "--visualize-graphics", argFlag, &visualizeGraphics, 0,
      "Add the bounding boxes of the graphics to the visualization. Must be used together with "
      "--visualization-path." },
  { "--visualize-figures", argFlag, &visualizeFigures, 0,
      "Add the bounding boxes of the figures to the visualization. Must be used together with "
      "--visualization-path." },
  { "--visualize-shapes", argFlag, &visualizeShapes, 0,
      "Add the bounding boxes of the shapes to the visualization. Must be used together with "
      "--visualization-path." },
  { "--visualize-words", argFlag, &visualizeWords, 0,
      "Add the bounding boxes of the words to the visualization. Must be used together with "
      "--visualization-path." },
  { "--visualize-text-lines", argFlag, &visualizeTextLines, 0,
      "Add the bounding boxes of the text lines to the visualization. Must be used together with "
      "--visualization-path." },
  { "--visualize-text-blocks", argFlag, &visualizeTextBlocks, 0,
      "Add the bounding boxes and the semantic roles of the text blocks to the visualization. "
      "Must be used together with --visualization-path." },
  { "--visualize-segments", argFlag, &visualizePageSegments, 0,
      "Add the bounding boxes of the page segments to the visualization. Must be used together "
      "with --visualization-path." },
  { "--visualize-reading-order", argFlag, &visualizeReadingOrder, 0,
      "Add the reading order of the text blocks to the visualization. Must be used together with "
      "--visualization-path." },
  { "--visualize-text-block-cuts", argFlag, &visualizeBlockDetectionCuts, 0,
      "Add the XY-cuts made to detect the text blocks to the visualization. Must be used together "
      "with --visualization-path." },
  { "--visualize-reading-order-cuts", argFlag, &visualizeReadingOrderCuts, 0,
      "Add the XY-cuts made to detect the reading order to the visualization. Must be used "
      "together with --visualization-path." },
  { "--visualization-path", argString, &visualizeFilePath, sizeof(visualizeFilePath),
      "The target path for the visualization. If not specified, no visualization file will "
      "be created, even if one or more of the --visualize-* options is used." },
  { "--debug-pdf-parsing", argFlag, &debugPdfParsing, 0,
      "Print the debug messages produced while parsing the content streams of the PDF file." },
  { "--debug-diacritic-marks-merging", argFlag, &debugDiacriticMarksMerging, 0,
      "Print the debug messages produced while merging diacritical marks with their base "
      "characters." },
  { "--debug-words-detection", argFlag, &debugWordsDetection, 0,
      "Print the debug messages produced while detecting words." },
  { "--debug-page-segmentation", argFlag, &debugPageSegmentation, 0,
      "Print the debug messages produced while segmenting the pages." },
  { "--debug-text-lines-detection", argFlag, &debugTextLinesDetection, 0,
      "Print the debug messages produced while detecting text lines." },
  { "--debug-text-blocks-detection", argFlag, &debugTextBlocksDetection, 0,
      "Print the debug messages produced while detecting text blocks." },
  { "--debug-page-filter", argInt, &debugPageFilter, 0,
      "When one or more of the --debug-* options is used, print only the debug messages "
      "that are produced while processing the specified page. Note that the page numbers are "
      "1-based; so to print the debug messages produced while processing the first page, type "
      "\"--debug-page-filter 1\". " },
  { "--print-running-times", argFlag, &printRunningTimes, 0,
      "Print the running times needed by the different extraction steps, for debugging purposes." },
  { "-v", argFlag, &printVersion, 0,
      "Print the version info." },
  { "-version", argFlag, &printVersion, 0,
      "Print the version info." },
  { "--version", argFlag, &printVersion, 0,
      "Print the version info." },
  { "-h", argFlag, &printHelp, 0,
      "Print the help info." },
  { "-help", argFlag, &printHelp, 0,
      "Print the help info." },
  { "--help", argFlag, &printHelp, 0,
      "Print the help info." },
  { "-?", argFlag, &printHelp, 0,
      "Print the help info." },
  {}
};

// =================================================================================================
// Print methods.

// This method prints the usage information to stdout.
void printUsageInfo() {
  printHelpInfo(programName, version, nullptr, usage, options);
}

// This method prints the help information (= description + usage information) to stdout.
void printHelpInfo() {
  printHelpInfo(programName, version, description, usage, options);
}

// This method prints the copyright and version information to stdout.
void printVersionInfo() {
  std::cout << "Version: " << version << std::endl;
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
