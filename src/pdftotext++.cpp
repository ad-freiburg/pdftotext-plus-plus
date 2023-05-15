/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <chrono>    // std::chrono::high_resolution_clock
#include <cstdlib>   // putenv
#include <iomanip>   // std::setw, std::setprecision
#include <iostream>  // std::cout
#include <locale>    // imbue
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "./serializers/JsonlSerializer.h"
#include "./serializers/TextSerializer.h"
#include "./utils/Log.h"  // BBLUE, OFF
#include "./utils/MathUtils.h"
#include "./utils/StringUtils.h"
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
namespace po = boost::program_options;

// =================================================================================================
// Parameters
// NOTE: All variables starting with "CXX_" below need to be specified at compile time, in form of
// a preprocessor variable. For example, to define the CXX_PROGRAM_NAME variable, type the
// following: "g++ -DCXX_PROGRAM_NAME='pdftotext++' ..."

// The program name.
static std::string programName = CXX_PROGRAM_NAME;

// The version number.
static std::string version = CXX_PROGRAM_VERSION;

// The program description.
static std::string description = CXX_PROGRAM_DESCRIPTION;

// The usage of this program.
static std::string usage = CXX_PROGRAM_USAGE;

// The maximum length of lines in the help message.
static int HELP_MAX_LINE_LENGTH = 80;

// The amount by which to indent the description of command line options
static int HELP_OPTION_DESC_LINE_INDENT = 4;

// static const ArgDesc options[] {
//   { "--control-characters", argFlag, &addControlCharacters, 0,
//       "Add the following control characters to the continuous text:\n"
//       "\"^A\" (start of heading) in front of each emphasized text block; "
//       "\"^L\" (form feed) between two text blocks when there is a page break in between. "
//       "Has no effect when used together with one or more --output-* options." },
//   { "--semantic-roles", argFlag, &addSemanticRoles, 0,
//       "Prefix each text block by its semantic role in the continuous text. "
//       "Has no effect when used together with one or more --output-* options." },
//   { "--no-scripts", argFlag, &noScripts, 0,
//       "Remove subscripted and superscripted characters from the continuous text. "
//       "Has no effect when used together with one or more --output-* options." },
//   { "--no-embedded-font-files", argFlag, &noEmbeddedFontFiles, 0,
//       "Disable the parsing of embedded font files. The consequence is a faster extraction process, "
//       "but a lower accuracy of the extracted text." },
//   { "--no-dehyphenation", argFlag, &noDehyphenation, 0,
//       "Disable words dehyphenation. The consequence is that each part into which a hyphenated "
//       "word is split will appear as a separate word in the continuous text and the JSONL." },
//   { "--parse-mode", argFlag, &parseMode, 0,
//       "Activate a special mode that parses the content streams of the PDF file for characters, "
//       "figures, and shapes, and stops afterwards. It does not detect words, text lines, and text "
//       "blocks. To output the extracted elements, use --output-characters, --output-figures and/or "
//       "--output-shapes." },
//   { "--output-pages", argFlag, &outputPages, 0, "Instead of continuous text, output JSONL "
//       "with all available information about the pages of the PDF file (e.g., the widths and "
//       "heights)." },
//   { "--output-characters", argFlag, &outputChars, 0, "Instead of continuous text, output "
//       "JSONL with all available information about the characters in the PDF file (e.g., the "
//       "positions and fonts)." },
//   { "--output-figures", argFlag, &outputFigures, 0, "Instead of continuous text, output "
//       "JSONL with all available information about the figures in the PDF file (e.g., the "
//       "positions)." },
//   { "--output-shapes", argFlag, &outputShapes, 0, "Instead of continuous text, output "
//       "JSONL with all available information about the shapes in the PDF file (e.g., the "
//       "positions)." },
//   { "--output-words", argFlag, &outputWords, 0, "Instead of continuous text, output JSONL "
//       "with all available information about the words in the PDF file (e.g., the positions and "
//       "the fonts)." },
//   { "--output-text-blocks", argFlag, &outputBlocks, 0, "Instead of continuous text, output "
//       "JSONL with all available information about the text blocks in the PDF file (e.g., the "
//       "positions and the fonts)." },
//   { "--visualize-characters", argFlag, &visualizeChars, 0,
//       "Add the bounding boxes of the detected characters to the visualization. Must be used "
//       "together with --visualization-path." },
//   { "--visualize-graphics", argFlag, &visualizeGraphics, 0,
//       "Add the bounding boxes of the detected graphics to the visualization. Must be used "
//       "together with --visualization-path." },
//   { "--visualize-figures", argFlag, &visualizeFigures, 0,
//       "Add the bounding boxes of the detected figures to the visualization. Must be used together "
//       "with --visualization-path." },
//   { "--visualize-shapes", argFlag, &visualizeShapes, 0,
//       "Add the bounding boxes of the detected shapes to the visualization. Must be used together "
//       "with --visualization-path." },
//   { "--visualize-words", argFlag, &visualizeWords, 0,
//       "Add the bounding boxes of the detected words to the visualization. Must be used together "
//       "with --visualization-path." },
//   { "--visualize-text-lines", argFlag, &visualizeTextLines, 0,
//       "Add the bounding boxes of the detected text lines to the visualization. Must be used "
//       "together with --visualization-path." },
//   { "--visualize-text-blocks", argFlag, &visualizeTextBlocks, 0,
//       "Add the bounding boxes and the semantic roles of the detected text blocks to the "
//       "visualization. Must be used together with --visualization-path." },
//   { "--visualize-segments", argFlag, &visualizePageSegments, 0,
//       "Add the bounding boxes of the detected page segments to the visualization. Must be used "
//       "together with --visualization-path." },
//   { "--visualize-reading-order", argFlag, &visualizeReadingOrder, 0,
//       "Add the detected reading order of the text blocks to the visualization. Must be used "
//       "together with --visualization-path." },
//   { "--visualize-segment-cuts", argFlag, &visualizeSegmentCuts, 0,
//       "Add the XY-cuts made by the page segmentation. Must be used together with "
//       "--visualization-path." },
//   { "--visualize-reading-order-cuts", argFlag, &visualizeReadingOrderCuts, 0,
//       "Add the XY-cuts made to detect the reading order to the visualization. Must be used "
//       "together with --visualization-path." },
//   { "--visualization-path", argString, &visualizeFilePath, sizeof(visualizeFilePath),
//       "The target path for the visualization. If not specified, no visualization file will "
//       "be created, even if one or more of the --visualize-* options is used." },
//   { "--log", argString, &loggingLevel, sizeof(loggingLevel),
//       "Sets the general verbosity level of the logging messages. Valid levels are: trace, debug, "
//       "info, warn, error. Logging messages with a level lower than the specified level will be "
//       "not printed to the console. Default: info." },
//   { "--debug-pdf-parsing", argFlag, &debugPdfParsing, 0,
//       "Print the debug messages produced while parsing the content streams of the PDF file." },
//   { "--debug-statistics", argFlag, &debugStatisticsComputation, 0,
//       "Print the debug messages produced while computing the statistics." },
//   { "--debug-diacritic-marks-merging", argFlag, &debugDiacriticMarksMerging, 0,
//       "Print the debug messages produced while merging diacritical marks with their base "
//       "characters." },
//   { "--debug-words-detection", argFlag, &debugWordsDetection, 0,
//       "Print the debug messages produced while detecting words." },
//   { "--debug-page-segmentation", argFlag, &debugPageSegmentation, 0,
//       "Print the debug messages produced while segmenting the pages." },
//   { "--debug-text-lines-detection", argFlag, &debugTextLinesDetection, 0,
//       "Print the debug messages produced while detecting text lines." },
//   { "--debug-sub-super-scripts-detection", argFlag, &debugSubSuperScriptsDetection, 0,
//       "Print the debug messages produced while detecting sub-/superscripts." },
//   { "--debug-text-blocks-detection", argFlag, &debugTextBlocksDetection, 0,
//       "Print the debug messages produced while detecting text blocks." },
//   { "--debug-page-filter", argInt, &debugPageFilter, 0,
//       "When one or more of the --debug-* options are used, print only the debug messages "
//       "that are produced while processing the specified page. Note that the page numbers are "
//       "1-based; so to print the debug messages produced while processing the first page, type "
//       "\"--debug-page-filter 1\". " },
//   { "--print-running-times", argFlag, &printRunningTimes, 0,
//       "Print the running times needed by the different extraction steps, for debugging purposes." },
//   { "-v", argFlag, &printVersion, 0,
//       "Print the version info." },
//   { "-version", argFlag, &printVersion, 0,
//       "Print the version info." },
//   { "--version", argFlag, &printVersion, 0,
//       "Print the version info." },
//   { "-h", argFlag, &printHelp, 0,
//       "Print the help info." },
//   { "-help", argFlag, &printHelp, 0,
//       "Print the help info." },
//   { "--help", argFlag, &printHelp, 0,
//       "Print the help info." },
//   { "-?", argFlag, &printHelp, 0,
//       "Print the help info." },
//   {}
// };


// =================================================================================================
// Print methods.

/**
 * This method prints the help message, containing the program name, the program version, a
 * general program description and a description of the command-line options.
 *
 * @param options
 *    The command-line options.
 * @param maxLineLength
 *    The maximum width of the help message.
 * @param optionDescIndent
 *    The amount by which to indent the description of the command-line options.
 */
void printHelpMessage(po::options_description& options, int maxLineLength, int optionDescIndent) {
  cout << BBLUE << "NAME" << OFF << endl;
  cout << string_utils::strip(programName) << endl;
  cout << endl;

  cout << BBLUE << "VERSION" << OFF << endl;
  cout << string_utils::strip(version) << endl;
  cout << endl;

  cout << BBLUE << "DESCRIPTION" << OFF << endl;
  cout << string_utils::wrap(string_utils::strip(description), maxLineLength) << endl;
  cout << endl;

  cout << BBLUE << "USAGE" << OFF << endl;
  cout << string_utils::wrap(string_utils::strip(usage), maxLineLength) << endl;
  cout << endl;

  cout << BBLUE << "OPTIONS" << OFF << endl;
  for (auto& x : options.options()) {
    cout << x->format_name() << endl;
    cout << string_utils::wrap(x->description(), maxLineLength, optionDescIndent) << endl;
  }
  cout << endl;
}

// =================================================================================================

/**
 * The main method of pdftotext++. It is responsible for parsing the command line arguments,
 * running pdftotext++ to extract the text from PDF, and serializing and visualizing the extracted
 * text.
 */
int main(int argc, char* argv[]) {
  // Specify the command-line options.
  bool printVersion = false;
  bool printHelp = false;
  string verbosity = "error";

  string pdfFilePath;
  string outputFilePath = "-";
// static bool addControlCharacters = false;
// static bool addSemanticRoles = false;
// static bool noScripts = false;
// static bool noEmbeddedFontFiles = false;
// static bool noDehyphenation = false;
// static bool parseMode = false;
// static bool outputPages = false;
// static bool outputChars = false;
// static bool outputFigures = false;
// static bool outputShapes = false;
// static bool outputWords = false;
// static bool outputBlocks = false;
// static bool visualizeChars = false;
// static bool visualizeWords = false;
// static bool visualizeTextLines = false;
// static bool visualizeTextBlocks = false;
// static bool visualizePageSegments = false;
// static bool visualizeFigures = false;
// static bool visualizeShapes = false;
// static bool visualizeGraphics = false;
// static bool visualizeReadingOrder = false;
// static bool visualizeSegmentCuts = false;
// static bool visualizeReadingOrderCuts = false;
// static char visualizeFilePath[256] = "";
// static char loggingLevel[16] = "";
// static bool debugPdfParsing = false;
// static bool debugStatisticsComputation = false;
// static bool debugDiacriticMarksMerging = false;
// static bool debugWordsDetection = false;
// static bool debugPageSegmentation = false;
// static bool debugTextLinesDetection = false;
// static bool debugSubSuperScriptsDetection = false;
// static bool debugTextBlocksDetection = false;
// static int debugPageFilter = -1;
// static bool printRunningTimes = false;

  // Specify public options (= options that will be shown to the user when printing the help).
  po::options_description publicOptions;
  publicOptions.add_options()
    (
      "verbosity",
      po::value<string>(&verbosity),
      "Specify the verbosity. Valid values are: trace, debug, info, warn, error. "
      "Logging messages with a level lower than the specified value will be not printed to the "
      "console. Default: error." // TODO: Specify the default value.
    )
    (
      "version,v",
      po::bool_switch(&printVersion),
      "Print version info."
    )
    (
      "help,h",
      po::bool_switch(&printHelp),
      "Print the help."
    );

  // Specify private options (= options that will not be shown to the user when printing the help).
  // NOTE: these options *must* include an entry for each positional options (defined below), this
  // is a restriction of the boost library.
  po::options_description privateOptions("");
  privateOptions.add_options()
    (
      "pdf-file",
      po::value<string>(&pdfFilePath)->required(),
      "The path to the PDF file to process."
    )
    (
      "output-file",
      po::value<string>(&outputFilePath),
      "The path to the file into which the extracted text should be written."
    );

  // Specify positional options.
  po::positional_options_description positional;
  positional.add("pdf-file", 1);
  positional.add("output-file", 1);

  // Parse the command-line options.
  po::options_description opts;
  opts.add(publicOptions).add(privateOptions);
  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).options(opts).positional(positional).run(), vm);
    po::notify(vm);
  } catch (const std::exception& e) {
    if (vm.count("help")) {
      printHelpMessage(publicOptions, HELP_MAX_LINE_LENGTH, HELP_OPTION_DESC_LINE_INDENT);
      return EXIT_SUCCESS;
    } else {
      cerr << "An error occurred on parsing the command-line options: " << e.what() << endl;
      cerr << "Type \"" << programName << " --help\" for printing the help." << endl;
      return EXIT_FAILURE;
    }
  }

  // Print the help info if explicitly requested by the user.
  if (printHelp) {
    printHelpMessage(publicOptions, HELP_MAX_LINE_LENGTH, HELP_OPTION_DESC_LINE_INDENT);
    return EXIT_SUCCESS;
  }

  // Print the version info if explicitly requested by the user.
  if (printVersion) {
    cout << programName << ", version " << version << endl;
    return EXIT_SUCCESS;
  }

  // Disable the log output of Tensorflow.
  char env[] = "TF_CPP_MIN_LOG_LEVEL=3";
  putenv(env);

  // Seed the random generator (needed for creating the random ids of the text elements).
  srand((unsigned) time(NULL) * getpid());

  // ------------
  // Start the extraction process.

  LogLevel logLevel = ERROR;
  if (verbosity == "trace" || verbosity == "TRACE") { logLevel = LogLevel::TRACE; }
  if (verbosity == "debug" || verbosity == "DEBUG") { logLevel = LogLevel::DEBUG; }
  if (verbosity == "info" || verbosity == "INFO") { logLevel = LogLevel::INFO; }
  if (verbosity == "warn" || verbosity == "WARN") { logLevel = LogLevel::WARN; }

//   PdfToTextPlusPlus engine(
//     noEmbeddedFontFiles,
//     noDehyphenation,
//     parseMode,
//     debugPdfParsing ? DEBUG : logLevel,
//     debugStatisticsComputation ? DEBUG : logLevel,
//     debugDiacriticMarksMerging ? DEBUG : logLevel,
//     debugWordsDetection ? DEBUG : logLevel,
//     debugPageSegmentation ? DEBUG : logLevel,
//     debugTextLinesDetection ? DEBUG : logLevel,
//     debugSubSuperScriptsDetection ? DEBUG : logLevel,
//     debugTextBlocksDetection ? DEBUG : logLevel,
//     debugPageFilter
//   );

//   PdfDocument doc;
//   vector<Timing> timings;

//   int status = 0;
//   // TODO(korzen): Don't use the invalid argument exception; it is currently thrown by
//   // SemanticRolesPredictor. Instead, use the exit code to check if something went wrong.
//   try {
//     status = engine.process(pdfFilePathStr, &doc, &timings);
//   } catch (const std::invalid_argument& ia) {
//     cerr << "An error occurred: " << ia.what() << '\n';
//     return 3;
//   }

//   // Abort if the exit code of pdftotext++ is > 0 (meaning that some error occurred).
//   if (status > 0) {
//     return status;
//   }

//   // Serialize the extraction result. If one of the --output-* options is used, output the text in
//   // JSONL format. Otherwise, output the text as continuous text.
//   auto start = high_resolution_clock::now();
//   if (outputPages || outputChars || outputFigures || outputShapes || outputWords|| outputBlocks) {
//     JsonlSerializer serializer(&doc,
//       outputPages,
//       outputChars,
//       outputFigures,
//       outputShapes,
//       outputWords,
//       outputBlocks);
//     serializer.serialize(outputFilePathStr);
//   } else {
//     TextSerializer serializer(&doc,
//       addControlCharacters,
//       addSemanticRoles,
//       noScripts);
//     serializer.serialize(outputFilePathStr);
//   }
//   auto end = high_resolution_clock::now();
//   Timing timingSerializeChars("Serialize", duration_cast<milliseconds>(end - start).count());
//   timings.push_back(timingSerializeChars);

//   // Visualize the extraction result, if requested by the user.
//   const string visualizeFilePathStr(visualizeFilePath);
//   if (!visualizeFilePathStr.empty()) {
//     auto start = high_resolution_clock::now();
//     PdfDocumentVisualizer visualizer(pdfFilePathStr);
//     if (visualizeChars) {
//       visualizer.visualizeCharacters(doc, visualizer::color_schemes::blue);
//     }
//     if (visualizeFigures) {
//       visualizer.visualizeFigures(doc, visualizer::color_schemes::blue);
//     }
//     if (visualizeShapes) {
//       visualizer.visualizeShapes(doc, visualizer::color_schemes::blue);
//     }
//     if (visualizeGraphics) {
//       visualizer.visualizeGraphics(doc, visualizer::color_schemes::blue);
//     }
//     if (visualizeWords) {
//       visualizer.visualizeWords(doc, visualizer::color_schemes::blue);
//     }
//     if (visualizeTextLines) {
//       visualizer.visualizeTextLines(doc, visualizer::color_schemes::blue);
//     }
//     if (visualizeTextBlocks) {
//       visualizer.visualizeTextBlocks(doc, visualizer::color_schemes::red);
//     }
//     if (visualizePageSegments) {
//       visualizer.visualizePageSegments(doc, visualizer::color_schemes::blue);
//     }
//     if (visualizeReadingOrder) {
//       visualizer.visualizeReadingOrder(doc, visualizer::color_schemes::blue);
//     }
//     if (visualizeSegmentCuts) {
//       visualizer.visualizeSegmentCuts(doc, visualizer::color_schemes::blue);
//     }
//     if (visualizeReadingOrderCuts) {
//       visualizer.visualizeReadingOrderCuts(doc, visualizer::color_schemes::blue);
//     }
//     visualizer.save(visualizeFilePathStr);
//     auto end = high_resolution_clock::now();
//     Timing timingVisualizing("Visualize", duration_cast<milliseconds>(end - start).count());
//     timings.push_back(timingVisualizing);
//   }

//   // Print the running times needed by the different processing steps, if requested by the user.
//   if (printRunningTimes) {
//     // Print the values with thousands separator.
//     cout.imbue(std::locale(""));
//     // Print the values with a precision of one decimal point.
//     cout << std::fixed;
//     cout << std::setprecision(1);

//     int64_t timeTotal = 0;
//     for (const auto& timing : timings) { timeTotal += timing.time; }
//     cout << BOLD << "Finished in " << timeTotal << " ms." << OFF << endl;

//     for (const auto& timing : timings) {
//       string prefix = " * " + timing.name + ":";
//       cout << std::left << std::setw(25) << prefix;
//       cout << std::right << std::setw(4) << timing.time << " ms ";
//       double time = math_utils::round(timing.time / static_cast<double>(timeTotal) * 100, 1);
//       cout << "(" << time << "%)";
//       cout << endl;
//     }
//   }

  return 0;
}
