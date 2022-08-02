/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <chrono>  // std::chrono::high_resolution_clock
#include <iomanip>  // std::setw, std::setprecision
#include <iostream>  // std::cout
#include <locale>  // imbue
#include <memory>  // std::make_unique
#include <string>
#include <vector>

#include "./serializers/JsonlSerializer.h"
#include "./serializers/TextSerializer.h"

#include "./utils/MathUtils.h"
#include "./utils/parseargs.h"

#include "./PdfDocumentVisualizer.h"
#include "./PdfToTextPlusPlus.h"

using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

// =================================================================================================
// Parameters.

// The program name.
static const char* programName = "pdftotext++";

// The version number.
static const char* version = "0.1";

// The description of this program to be displayed in the help message.
static const char* description =
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
"      given text unit (e.g., \"characters\", \"words\", or \"blocks\"). It contains one line per\n"
"      instance of the respective unit (e.g., one line per word if the unit is \"words\"), each\n"
"      providing all available layout information about the instance. Here is an example line,\n"
"      showing the general structure of a line and which information are provided for a word:\n"
"      {\"type\": \"word\", \"page\": 9, \"minX\": 448.8, \"minY\": 635.9, \"maxX\": 459.4, ⮨\n"
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

// The usage.
static const char* usage = "pdftotext++ [options] <pdf-file> <output-file>\n"
"\n"
"This processes the PDF file <pdf-file>. The extracted text is written to the file <output-file>.\n"
"If <output-file> is specified as '-', the extracted text is printed to stdout.";

// =================================================================================================
// Options.

static bool printVersion = false;
static bool printHelp = false;
static bool addControlCharacters = false;
static bool addSemanticRoles = false;
static bool noScripts = false;
static bool noEmbeddedFontFiles = false;
static bool noDehyphenation = false;
static bool parseMode = false;
static bool outputPages = false;
static bool outputChars = false;
static bool outputFigures = false;
static bool outputShapes = false;
static bool outputWords = false;
static bool outputBlocks = false;
static bool visualizeChars = false;
static bool visualizeWords = false;
static bool visualizeTextLines = false;
static bool visualizeTextBlocks = false;
static bool visualizePageSegments = false;
static bool visualizeFigures = false;
static bool visualizeShapes = false;
static bool visualizeGraphics = false;
static bool visualizeReadingOrder = false;
static bool visualizeSegmentCuts = false;
static bool visualizeReadingOrderCuts = false;
static char visualizeFilePath[256] = "";
static bool debugPdfParsing = false;
static bool debugStatisticsComputation = false;
static bool debugDiacriticMarksMerging = false;
static bool debugWordsDetection = false;
static bool debugPageSegmentation = false;
static bool debugTextLinesDetection = false;
static bool debugSubSuperScriptsDetection = false;
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
  { "--no-scripts", argFlag, &noScripts, 0,
      "Remove subscripted and superscripted characters from the continuous text. "
      "Has no effect when used together with one or more --output-* options." },
  { "--no-embedded-font-files", argFlag, &noEmbeddedFontFiles, 0,
      "Disable the parsing of embedded font files. The consequence is a faster extraction process, "
      "but a lower accuracy of the extracted text." },
  { "--no-dehyphenation", argFlag, &noDehyphenation, 0,
      "Disable words dehyphenation. The consequence is that each part into which a hyphenated "
      "word is split will appear as a separate word in the continuous text and the JSONL." },
  { "--parse-mode", argFlag, &parseMode, 0,
      "Activate a special mode that parses the content streams of the PDF file for characters, "
      "figures, and shapes, and stops afterwards. It does not detect words, text lines, and text "
      "blocks. To output the extracted elements, use --output-characters, --output-figures and/or "
      "--output-shapes." },
  { "--output-pages", argFlag, &outputPages, 0, "Instead of continuous text, output JSONL "
      "with all available information about the pages of the PDF file (e.g., the widths and "
      "heights)." },
  { "--output-characters", argFlag, &outputChars, 0, "Instead of continuous text, output "
      "JSONL with all available information about the characters in the PDF file (e.g., the "
      "positions and fonts)." },
  { "--output-figures", argFlag, &outputFigures, 0, "Instead of continuous text, output "
      "JSONL with all available information about the figures in the PDF file (e.g., the "
      "positions)." },
  { "--output-shapes", argFlag, &outputShapes, 0, "Instead of continuous text, output "
      "JSONL with all available information about the shapes in the PDF file (e.g., the "
      "positions)." },
  { "--output-words", argFlag, &outputWords, 0, "Instead of continuous text, output JSONL "
      "with all available information about the words in the PDF file (e.g., the positions and "
      "the fonts)." },
  { "--output-text-blocks", argFlag, &outputBlocks, 0, "Instead of continuous text, output "
      "JSONL with all available information about the text blocks in the PDF file (e.g., the "
      "positions and the fonts)." },
  { "--visualize-characters", argFlag, &visualizeChars, 0,
      "Add the bounding boxes of the detected characters to the visualization. Must be used "
      "together with --visualization-path." },
  { "--visualize-graphics", argFlag, &visualizeGraphics, 0,
      "Add the bounding boxes of the detected graphics to the visualization. Must be used "
      "together with --visualization-path." },
  { "--visualize-figures", argFlag, &visualizeFigures, 0,
      "Add the bounding boxes of the detected figures to the visualization. Must be used together "
      "with --visualization-path." },
  { "--visualize-shapes", argFlag, &visualizeShapes, 0,
      "Add the bounding boxes of the detected shapes to the visualization. Must be used together "
      "with --visualization-path." },
  { "--visualize-words", argFlag, &visualizeWords, 0,
      "Add the bounding boxes of the detected words to the visualization. Must be used together "
      "with --visualization-path." },
  { "--visualize-text-lines", argFlag, &visualizeTextLines, 0,
      "Add the bounding boxes of the detected text lines to the visualization. Must be used "
      "together with --visualization-path." },
  { "--visualize-text-blocks", argFlag, &visualizeTextBlocks, 0,
      "Add the bounding boxes and the semantic roles of the detected text blocks to the "
      "visualization. Must be used together with --visualization-path." },
  { "--visualize-segments", argFlag, &visualizePageSegments, 0,
      "Add the bounding boxes of the detected page segments to the visualization. Must be used "
      "together with --visualization-path." },
  { "--visualize-reading-order", argFlag, &visualizeReadingOrder, 0,
      "Add the detected reading order of the text blocks to the visualization. Must be used "
      "together with --visualization-path." },
  { "--visualize-segment-cuts", argFlag, &visualizeSegmentCuts, 0,
      "Add the XY-cuts made by the page segmentation. Must be used together with "
      "--visualization-path." },
  { "--visualize-reading-order-cuts", argFlag, &visualizeReadingOrderCuts, 0,
      "Add the XY-cuts made to detect the reading order to the visualization. Must be used "
      "together with --visualization-path." },
  { "--visualization-path", argString, &visualizeFilePath, sizeof(visualizeFilePath),
      "The target path for the visualization. If not specified, no visualization file will "
      "be created, even if one or more of the --visualize-* options is used." },
  { "--debug-pdf-parsing", argFlag, &debugPdfParsing, 0,
      "Print the debug messages produced while parsing the content streams of the PDF file." },
  { "--debug-statistics", argFlag, &debugStatisticsComputation, 0,
      "Print the debug messages produced while computing the statistics." },
  { "--debug-diacritic-marks-merging", argFlag, &debugDiacriticMarksMerging, 0,
      "Print the debug messages produced while merging diacritical marks with their base "
      "characters." },
  { "--debug-words-detection", argFlag, &debugWordsDetection, 0,
      "Print the debug messages produced while detecting words." },
  { "--debug-page-segmentation", argFlag, &debugPageSegmentation, 0,
      "Print the debug messages produced while segmenting the pages." },
  { "--debug-text-lines-detection", argFlag, &debugTextLinesDetection, 0,
      "Print the debug messages produced while detecting text lines." },
  { "--debug-sub-super-scripts-detection", argFlag, &debugSubSuperScriptsDetection, 0,
      "Print the debug messages produced while detecting sub-/superscripts." },
  { "--debug-text-blocks-detection", argFlag, &debugTextBlocksDetection, 0,
      "Print the debug messages produced while detecting text blocks." },
  { "--debug-page-filter", argInt, &debugPageFilter, 0,
      "When one or more of the --debug-* options are used, print only the debug messages "
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

/**
 * This method prints the usage info (that is: the help info without the description) to stdout.
 */
void printUsageInfo() {
  printHelpInfo(programName, version, nullptr, usage, options);
}

/**
 * This method prints the help info (that is: a detailed description of the program, the usage
 * info, and a description of the options) to stdout.
 */
void printHelpInfo() {
  printHelpInfo(programName, version, description, usage, options);
}

/**
 * This method prints the version info to stdout.
 */
void printVersionInfo() {
  cout << "Version: " << version << endl;
}

// =================================================================================================

/**
 * This method is the main method of pdftotext++. It is responsible for parsing the command line
 * arguments, running pdftotext++ to extract the text from PDF, and serializing and visualizing the
 * extracted text.
 */
int main(int argc, char* argv[]) {
  // Seed the random generator (needed to, for example, create the random ids of the text elements).
  srand((unsigned) time(NULL) * getpid());

  // Parse the command line arguments.
  bool ok = parseArgs(options, &argc, argv);

  // Print the usage info if the command line arguments couldn't be parsed successfully.
  if (!ok) {
    printUsageInfo();
    return 1;
  }

  // Print the help info if explicitly requested by the user.
  if (printHelp) {
    printHelpInfo();
    return 0;
  }

  // Print the version info if explicitly requested by the user.
  if (printVersion) {
    printVersionInfo();
    return 0;
  }

  // Print the usage info if not all required arguments were specified by the user.
  if (argc < 3) {
    printUsageInfo();
    return 2;
  }

  // Obtain the path to the input PDF file.
  string pdfFilePathStr(argv[1]);

  // Obtain the path to the output file (could be "-", if the output should be printed to stdout).
  string outputFilePathStr(argv[2]);

  // ------------
  // Start the extraction process.

  PdfToTextPlusPlus engine(
    noEmbeddedFontFiles,
    noDehyphenation,
    parseMode,
    debugPdfParsing,
    debugStatisticsComputation,
    debugDiacriticMarksMerging,
    debugWordsDetection,
    debugPageSegmentation,
    debugTextLinesDetection,
    debugSubSuperScriptsDetection,
    debugTextBlocksDetection,
    debugPageFilter);

  PdfDocument doc;
  vector<Timing> timings;

  int status = 0;
  // TODO(korzen): Don't use the invalid argument exception; it is currently thrown by
  // SemanticRolesPredictor. Instead, use the exit code to check if something went wrong.
  try {
    status = engine.process(pdfFilePathStr, &doc, &timings);
  } catch (const std::invalid_argument& ia) {
    cerr << "An error occurred: " << ia.what() << '\n';
    return 3;
  }

  // Abort if the exit code of pdftotext++ is > 0 (meaning that some error occurred).
  if (status > 0) {
    return status;
  }

  // Serialize the extraction result. If one of the --output-* options is used, output the text in
  // JSONL format. Otherwise, output the text as continuous text.
  auto start = high_resolution_clock::now();
  if (outputPages || outputChars || outputFigures || outputShapes || outputWords|| outputBlocks) {
    JsonlSerializer serializer(&doc,
      outputPages,
      outputChars,
      outputFigures,
      outputShapes,
      outputWords,
      outputBlocks);
    serializer.serialize(outputFilePathStr);
  } else {
    TextSerializer serializer(&doc,
      addControlCharacters,
      addSemanticRoles,
      noScripts);
    serializer.serialize(outputFilePathStr);
  }
  auto end = high_resolution_clock::now();
  Timing timingSerializeChars("Serialize", duration_cast<milliseconds>(end - start).count());
  timings.push_back(timingSerializeChars);

  // Visualize the extraction result, if requested by the user.
  const string visualizeFilePathStr(visualizeFilePath);
  if (!visualizeFilePathStr.empty()) {
    auto start = high_resolution_clock::now();
    PdfDocumentVisualizer visualizer(pdfFilePathStr);
    if (visualizeChars) {
      visualizer.visualizeCharacters(doc, visualizer::color_schemes::blue);
    }
    if (visualizeFigures) {
      visualizer.visualizeFigures(doc, visualizer::color_schemes::blue);
    }
    if (visualizeShapes) {
      visualizer.visualizeShapes(doc, visualizer::color_schemes::blue);
    }
    if (visualizeGraphics) {
      visualizer.visualizeGraphics(doc, visualizer::color_schemes::blue);
    }
    if (visualizeWords) {
      visualizer.visualizeWords(doc, visualizer::color_schemes::blue);
    }
    if (visualizeTextLines) {
      visualizer.visualizeTextLines(doc, visualizer::color_schemes::blue);
    }
    if (visualizeTextBlocks) {
      visualizer.visualizeTextBlocks(doc, visualizer::color_schemes::red);
    }
    if (visualizePageSegments) {
      visualizer.visualizePageSegments(doc, visualizer::color_schemes::blue);
    }
    if (visualizeReadingOrder) {
      visualizer.visualizeReadingOrder(doc, visualizer::color_schemes::blue);
    }
    if (visualizeSegmentCuts) {
      visualizer.visualizeSegmentCuts(doc, visualizer::color_schemes::blue);
    }
    if (visualizeReadingOrderCuts) {
      visualizer.visualizeReadingOrderCuts(doc, visualizer::color_schemes::blue);
    }
    visualizer.save(visualizeFilePathStr);
    auto end = high_resolution_clock::now();
    Timing timingVisualizing("Visualize", duration_cast<milliseconds>(end - start).count());
    timings.push_back(timingVisualizing);
  }

  // Print the running times needed by the different processing steps, if requested by the user.
  if (printRunningTimes) {
    // Print the values with thousands separator.
    cout.imbue(std::locale(""));
    // Print the values with a precision of one decimal point.
    cout << std::fixed;
    cout << std::setprecision(1);

    int64_t timeTotal = 0;
    for (const auto& timing : timings) { timeTotal += timing.time; }
    cout << BOLD << "Finished in " << timeTotal << " ms." << OFF << endl;

    for (const auto& timing : timings) {
      string prefix = " * " + timing.name + ":";
      cout << std::left << std::setw(25) << prefix;
      cout << std::right << std::setw(4) << timing.time << " ms ";
      double time = math_utils::round(timing.time / static_cast<double>(timeTotal) * 100, 1);
      cout << "(" << time << "%)";
      cout << endl;
    }
  }

  return 0;
}
