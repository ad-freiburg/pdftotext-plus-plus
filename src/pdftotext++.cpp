/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <boost/program_options.hpp>

#include <chrono>    // std::chrono::high_resolution_clock
#include <cstdlib>   // putenv
#include <iomanip>   // std::setw, std::setprecision
#include <iostream>  // std::cout
#include <locale>    // imbue
#include <memory>    // std::make_unique
#include <string>
#include <unordered_set>
#include <vector>

#include "./serialization/Serialization.h"
#include "./utils/Log.h"  // BBLUE, OFF
#include "./utils/MathUtils.h"
#include "./utils/StringUtils.h"
#include "./Config.h"
#include "./PdfDocumentVisualizer.h"
#include "./PdfToTextPlusPlus.h"
#include "./Types.h"
#include "./Validators.h"

using ppp::types::DocumentUnit;
using ppp::types::SemanticRole;
using ppp::types::SerializationFormat;
using ppp::types::Timing;
using ppp::serialization::Serializer;
using ppp::string_utils::wrap;
using ppp::string_utils::strip;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using std::cerr;
using std::cout;
using std::endl;
using std::exception;
using std::string;
using std::unordered_set;
using std::vector;

namespace po = boost::program_options;

// =================================================================================================
// Global parameters.
// NOTE: The variables in uppercase (for example: PROGRAM_NAME) need to be specified at compile
// time, in form of a preprocessor variable which is part of the g++ command. For example, to
// define the variable 'PROGRAM_NAME', type the following: "g++ -DPROGRAM_NAME='pdftotext++' ..."

const char* programName = PROGRAM_NAME;
const char* programDescription = PROGRAM_DESCRIPTION;
const char* programUsage = PROGRAM_USAGE;
const char* version = VERSION;

// =================================================================================================
// Helper methods.

/**
 * This method prints the help message - containing the program name, the program version, a
 * general program description and a description of the command-line options.
 *
 * @param publicOpts
 *    The definition of the public command-line options.
 * @param privateOpts
 *    The definition of the non-public command-line options. NOTE: This can be omitted when the
 *    private options should not appear in the help message.
 * @param maxLineLength
 *    The maximum length of the lines in the help message.
 * @param optionDescIndent
 *    The amount by which to indent the description of a command line option.
 */
void printHelpMessage(
    const po::options_description* publicOpts, const po::options_description* privateOpts = 0,
    int maxLineLength = 100, int optionDescIndent = 4) {
  cout << BBLUE << "NAME" << OFF << endl;
  cout << wrap(strip(programName), maxLineLength) << endl;
  cout << endl;

  cout << BBLUE << "VERSION" << OFF << endl;
  cout << wrap(strip(version), maxLineLength) << endl;
  cout << endl;

  cout << BBLUE << "DESCRIPTION" << OFF << endl;
  cout << wrap(strip(programDescription), maxLineLength) << endl;
  cout << endl;

  cout << BBLUE << "USAGE" << OFF << endl;
  cout << wrap(strip(programUsage), maxLineLength) << endl;
  cout << endl;

  cout << BBLUE << "OPTIONS" << OFF << endl;
  for (const auto& opt : publicOpts->options()) {
    cout << opt->format_name() << endl;
    cout << wrap(strip(opt->description()), maxLineLength, optionDescIndent) << endl;
  }
  cout << endl;

  if (privateOpts) {
    cout << BBLUE << "NON-PUBLIC OPTIONS" << OFF << endl;
    for (const auto& opt : privateOpts->options()) {
      cout << opt->format_name() << endl;
      cout << wrap(strip(opt->description()), maxLineLength, optionDescIndent) << endl;
    }
    cout << endl;
  }
}

/**
 * This method prints the version.
 */
void printVersionMessage() {
  cout << programName << ", version " << version << endl;
}

// =================================================================================================

/**
 * The main method of pdftotext++. It is responsible for parsing the command line options,
 * running the extraction pipeline on a specified PDF, and outputting the extracted text.
 */
int main(int argc, char* argv[]) {
  // The path to the PDF file to process.
  string pdfFilePath;
  // The path to the file into which to output the extracted text.
  // NOTE: If specified as "-", the text is output to stdout.
  string outputFilePath = "-";

  // The format in which to output the extracted text.
  SerializationFormat format = SerializationFormat::TXT;
  // The semantic roles of the text blocks to output.
  vector<SemanticRole> roles = ppp::types::getSemanticRoles();
  // The units of the text to output.
  vector<DocumentUnit> units = ppp::types::getDocumentUnits();

  bool addControlCharacters = false;  // TODO(korzen): Introduce an extra format.
  bool addSemanticRoles = false;  // TODO(korzen): Introduce an extra format.
  bool noScripts = false;  // TODO(korzen): Allow to disable all optional components of pipeline.
  bool noEmbeddedFontFiles = false;
  bool noDehyphenation = false;
  bool parseMode = false;
  bool visualizeChars = false;
  bool visualizeWords = false;
  bool visualizeTextLines = false;
  bool visualizeTextBlocks = false;
  bool visualizePageSegments = false;
  bool visualizeFigures = false;
  bool visualizeShapes = false;
  bool visualizeGraphics = false;
  bool visualizeReadingOrder = false;
  bool visualizeSegmentCuts = false;
  bool visualizeReadingOrderCuts = false;
  string visualizeFilePath = "";
  string verbosity = "error";
  bool debugPdfParsing = false;  // TODO(korzen): Introduce advanced logging.
  bool debugStatisticsComputation = false;
  bool debugDiacriticMarksMerging = false;
  bool debugWordsDetection = false;
  bool debugPageSegmentation = false;
  bool debugTextLinesDetection = false;
  bool debugSubSuperScriptsDetection = false;
  bool debugTextBlocksDetection = false;
  int debugPageFilter = -1;
  bool printRunningTimes = false;
  bool printVersion = false;
  bool printHelp = false;
  bool printFullHelp = false;

  // Specify the public options (= options that will be shown to the user when printing the help).
  po::options_description publicOpts;
  publicOpts.add_options()
    (
      "format",
      po::value<SerializationFormat>(&format),
      (string("Output the extracted text in the specified format.\nValid formats: ")
        + ppp::serialization::getSerializationFormatChoicesStr() + string(".\n")).c_str()
    )
    (
      "role",
      po::value<vector<SemanticRole>>(&roles),
      (string("Only output text from text blocks with the specified roles. Use the syntax\n")
        + string("'--role <value1> --role <value2> ...' to specify multiple roles.\nValid roles: ")
        + ppp::types::getSemanticRolesStr() + string(".\n")).c_str()
    )
    (
      "unit",
      po::value<vector<DocumentUnit>>(&units),
      (string("Output semantic and layout information about the specified document unit(s). Use\n")
        + string("the syntax '--unit <value1> --unit <value2> ...' to specify multiple units.\n")
        + string("NOTE: This option does not have an effect when used together with the ")
        + string("'--format txt' or '--format txt-extended' option.\n")
        + string("Valid units: ") + ppp::types::getDocumentUnitsStr() + string(".\n")).c_str()
    )
    (
      "control-characters",
      po::bool_switch(&addControlCharacters),
      "Output the extracted text together with the following control characters:\n"
      "• \"^A\" (start of heading) in front of each emphasized text block\n"
      "• \"^L\" (form feed) between two text blocks when there is a page break in between.\n"
      "NOTE: This option only has an effect when used together with the '--format txt' option."
    )
    // TODO(korzen): This option should be removed, when there are more structured formats.
    (
      "semantic-roles",
      po::bool_switch(&addSemanticRoles),
      "Output each extracted text block together with its semantic role. "
      "NOTE: This option only has an effect when used together with the '--format txt' option."
    )
    (
      "no-scripts",
      po::bool_switch(&noScripts),
      "Output the extracted text without characters that appear as a subscript or superscript in "
      "the PDF."
    )
    (
      "no-dehyphenation",
      po::bool_switch(&noDehyphenation),
      "Do not merge hyphenated words. "
      "NOTE: Using this option has the consequence that each part into which a hyphenated word is "
      "divided appears as a separate word in the extracted text."
    )
    (
      "no-embedded-font-files",
      po::bool_switch(&noEmbeddedFontFiles),
      "Do not parse the font files embedded into the PDF file. "
      "NOTE: Using this option results in a faster extraction process, but a less accurate "
      "extraction result."
    )
    (
      "visualization-path",
      po::value<string>(&visualizeFilePath),
      "Create a visualization PDF file, that is: a copy of the PDF file, with annotations added "
      "by the different '--visualize-*' options below, and write it to the specified file. "
      "NOTE: If not specified, no such visualization will be created, even if one or more of the "
      "'--visualize-*' options is used."
    )
    (
      "visualize-characters",
      po::bool_switch(&visualizeChars),
      "Draw bounding boxes around the detected characters into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    // TODO(korzen): explain the difference between graphics and figure.
    (
      "visualize-graphics",
      po::bool_switch(&visualizeGraphics),
      "Draw bounding boxes around the detected graphics into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "visualize-figures",
      po::bool_switch(&visualizeFigures),
      "Draw bounding boxes around the detected figures into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "visualize-shapes",
      po::bool_switch(&visualizeShapes),
      "Draw bounding boxes around the detected shapes into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "visualize-words",
      po::bool_switch(&visualizeWords),
      "Draw bounding boxes around the detected words into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "visualize-text-lines",
      po::bool_switch(&visualizeTextLines),
      "Draw bounding boxes around the detected text lines into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "visualize-text-blocks",
      po::bool_switch(&visualizeTextBlocks),
      "Draw bounding boxes around the detected text blocks into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "visualize-segments",
      po::bool_switch(&visualizePageSegments),
      "Draw bounding boxes around the detected page segments into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "visualize-segment-cuts",
      po::bool_switch(&visualizeSegmentCuts),
      "Draw the XY-cuts made to segment the pages into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "visualize-reading-order",
      po::bool_switch(&visualizeReadingOrder),
      "Draw (directed) edges between the detected text blocks into the visualization PDF file, for "
      "visualizing the detected reading order. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "visualize-reading-order-cuts",
      po::bool_switch(&visualizeReadingOrderCuts),
      "Draw the XY-cuts made to detect the reading order into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "verbosity",
      po::value<string>(&verbosity),
      "Specify the verbosity. Valid values are: trace, debug, info, warn, error. "
      "Logging messages with a level lower than the specified value will be not printed to the "
      "console. Default: error."  // TODO(korzen): Specify the default value.
    )
    (
      "version,v",
      po::bool_switch(&printVersion),
      "Print the version info."
    )
    (
      "help,h",
      po::bool_switch(&printHelp),
      "Print the help.");

  // Specify the private options (= options that will not be shown in the help message).
  // NOTE: these options *must* include an entry for each positional option (defined below), this
  // is a restriction of the boost library.
  po::options_description privateOpts("");
  privateOpts.add_options()
    (
      "pdf-file",
      po::value<string>(&pdfFilePath)->required(),
      "The path to the PDF file to process."
    )
    (
      "output-file",
      po::value<string>(&outputFilePath),
      "The path to the file into which the extracted text should be written."
    )
    (
      "parse-mode",
      po::bool_switch(&parseMode),
      "Only parse the content streams of the PDF file for characters, figures, and shapes. "
      "Do not detect words, text lines, and text blocks. "
      "NOTE: To output the extracted elements, use the '--output-characters', '--output-figures' "
      "and/or '--output-shapes' options."
    )
    // TODO(korzen): Think about how to handle logging (one log per module?).
    (
      "debug-pdf-parsing",
      po::bool_switch(&debugPdfParsing),
      "Print the debug messages produced while parsing the content streams of the PDF file."
    )
    (
      "debug-statistics",
      po::bool_switch(&debugStatisticsComputation),
      "Print the debug messages produced while computing the statistics."
    )
    (
      "debug-diacritic-marks-merging",
      po::bool_switch(&debugDiacriticMarksMerging),
      "Print the debug messages produced while merging diacritical marks with their base "
      "characters."
    )
    (
      "debug-words-detection",
      po::bool_switch(&debugWordsDetection),
      "Print the debug messages produced while detecting words."
    )
    (
      "debug-page-segmentation",
      po::bool_switch(&debugPageSegmentation),
      "Print the debug messages produced while segmenting the pages."
    )
    (
      "debug-text-lines-detection",
      po::bool_switch(&debugTextLinesDetection),
      "Print the debug messages produced while detecting text lines."
    )
    (
      "debug-sub-super-scripts-detection",
      po::bool_switch(&debugSubSuperScriptsDetection),
      "Print the debug messages produced while detecting sub-/superscripts."
    )
    (
      "debug-text-blocks-detection",
      po::bool_switch(&debugTextBlocksDetection),
      "Print the debug messages produced while detecting text blocks."
    )
    (
      "debug-page-filter",
      po::value<int>(&debugPageFilter),
      "When one or more of the '--debug-*' options are used, print only the debug messages that "
      "are produced while processing the specified page. "
      "NOTE: the page numbers are 1-based; so to print the debug messages produced while "
      "processing the first page, type \"--debug-page-filter 1\". "
    )
    (
      "print-running-times",
      po::bool_switch(&printRunningTimes),
      "Print the running times needed by the different steps in the extraction pipeline."
    )
    (
      "full-help",
      po::bool_switch(&printFullHelp),
      "Print the full help (containing also the descriptions of all non-public commands).");

  // Specify the positional options.
  po::positional_options_description positional;
  positional.add("pdf-file", 1);
  positional.add("output-file", 1);

  // Parse the command-line options.
  po::options_description opts;
  opts.add(publicOpts).add(privateOpts);
  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).options(opts).positional(positional).run(), vm);
    po::notify(vm);
  } catch (const exception& e) {
    bool showFullHelp = vm.count("full-help") ? vm["full-help"].as<bool>() : false;
    if (showFullHelp) {
      printHelpMessage(&publicOpts, &privateOpts);
      return EXIT_SUCCESS;
    }
    bool showHelp = vm.count("help") ? vm["help"].as<bool>() : false;
    if (showHelp) {
      printHelpMessage(&publicOpts);
      return EXIT_SUCCESS;
    }
    bool showVersion = vm.count("version") ? vm["version"].as<bool>() : false;
    if (showVersion) {
      printVersionMessage();
      return EXIT_SUCCESS;
    }
    cerr << "An error occurred on parsing the command-line options: " << e.what() << endl;
    cerr << "Type \"" << programName << " --help\" for printing the help." << endl;
    return EXIT_FAILURE;
  }

  // Print the full help info if explicitly requested by the user.
  if (printFullHelp) {
    printHelpMessage(&publicOpts, &privateOpts);
    return EXIT_SUCCESS;
  }

  // Print the help info if explicitly requested by the user.
  if (printHelp) {
    printHelpMessage(&publicOpts);
    return EXIT_SUCCESS;
  }

  // Print the version info if explicitly requested by the user.
  if (printVersion) {
    printVersionMessage();
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

  ppp::Config config;
  config.textOutputDev.logLevel = debugPdfParsing ? DEBUG : logLevel;
  config.textOutputDev.logPageFilter = debugPageFilter;
  config.textOutputDev.noEmbeddedFontFilesParsing = noEmbeddedFontFiles;

  config.pdfStatisticsCalculator.logLevel = debugStatisticsComputation ? DEBUG : logLevel;
  config.semanticRolesDetectionModelsDir = CONFIG_SEMANTIC_ROLES_DETECTION_MODELS_DIR;

  PdfToTextPlusPlus engine(
    &config,
    noEmbeddedFontFiles,
    noDehyphenation,
    parseMode,
    debugPdfParsing ? DEBUG : logLevel,
    debugStatisticsComputation ? DEBUG : logLevel,
    debugDiacriticMarksMerging ? DEBUG : logLevel,
    debugWordsDetection ? DEBUG : logLevel,
    debugPageSegmentation ? DEBUG : logLevel,
    debugTextLinesDetection ? DEBUG : logLevel,
    debugSubSuperScriptsDetection ? DEBUG : logLevel,
    debugTextBlocksDetection ? DEBUG : logLevel,
    debugPageFilter);

  PdfDocument doc;
  vector<Timing> timings;

  int status = 0;
  // TODO(korzen): Don't use the invalid argument exception; it is currently thrown by
  // SemanticRolesPredictor. Instead, use the exit code to check if something went wrong.
  try {
    status = engine.process(pdfFilePath, &doc, &timings);
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
  // if (outputPages || outputChars || outputFigures || outputShapes || outputWords|| outputBlock) {
  //   JsonlSerializer serializer(&doc,
  //     outputPages,
  //     outputChars,
  //     outputFigures,
  //     outputShapes,
  //     outputWords,
  //     outputBlocks);
  //   serializer.serialize(outputFilePath);
  // } else {
  //   TextSerializer serializer(&doc,
  //     addControlCharacters,
  //     addSemanticRoles,
  //     noScripts);
  //   serializer.serialize(outputFilePath);
  // }
  Serializer* serializer = ppp::serialization::getSerializer(format);
  if (serializer) {
    unordered_set<SemanticRole> rolesSet(roles.begin(), roles.end());
    unordered_set<DocumentUnit> unitsSet(units.begin(), units.end());
    serializer->serialize(&doc, rolesSet, unitsSet, outputFilePath);
  }
  auto end = high_resolution_clock::now();
  Timing timingSerializeChars("Serialize", duration_cast<milliseconds>(end - start).count());
  timings.push_back(timingSerializeChars);

  // Visualize the extraction result, if requested by the user.
  const string visualizeFilePathStr(visualizeFilePath);
  if (!visualizeFilePathStr.empty()) {
    auto start = high_resolution_clock::now();
    PdfDocumentVisualizer visualizer(pdfFilePath);
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
      double time = ppp::math_utils::round(timing.time / static_cast<double>(timeTotal) * 100, 1);
      cout << "(" << time << "%)";
      cout << endl;
    }
  }

  return EXIT_SUCCESS;
}
