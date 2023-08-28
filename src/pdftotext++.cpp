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
#include <string>
#include <unordered_set>
#include <vector>

#include "./Config.h"
#include "./PdfDocumentSerialization.h"
#include "./PdfDocumentVisualization.h"
#include "./PdfToTextPlusPlus.h"
#include "./Types.h"
#include "./Validators.h"
#include "./utils/Log.h"
#include "./utils/MathUtils.h"
#include "./utils/TextUtils.h"

using std::cerr;
using std::cout;
using std::endl;
using std::exception;
using std::fixed;
using std::invalid_argument;
using std::locale;
using std::setprecision;
using std::setw;
using std::string;
using std::unordered_set;
using std::vector;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

using boost::program_options::bool_switch;
using boost::program_options::command_line_parser;
using boost::program_options::notify;
using boost::program_options::options_description;
using boost::program_options::positional_options_description;
using boost::program_options::store;
using boost::program_options::value;
using boost::program_options::variables_map;

using ppp::PdfToTextPlusPlus;
using ppp::config::Config;
using ppp::serialization::Serializer;  // TODO(korzen)
using ppp::types::DocumentUnit;
using ppp::types::PdfDocument;
using ppp::types::SemanticRole;
using ppp::types::SerializationFormat;
using ppp::types::Timing;
using ppp::utils::log::BBLUE;
using ppp::utils::log::BOLD;
using ppp::utils::log::DEBUG;
using ppp::utils::log::ERROR;
using ppp::utils::log::INFO;
using ppp::utils::log::OFF;
using ppp::utils::log::TRACE;
using ppp::utils::log::WARN;
using ppp::utils::log::LogLevel;
using ppp::utils::math::round;
using ppp::utils::text::wrap;
using ppp::utils::text::strip;
using ppp::visualization::PdfDocumentVisualization;

// =================================================================================================
// Global parameters.
// NOTE: The variables in uppercase (for example: PROGRAM_NAME or VERSION) need to be specified at
// compile time, in form of preprocessor variables which are part of the g++ command. For example,
// to define the variable 'PROGRAM_NAME', type the following: "g++ -DPROGRAM_NAME='pdftotext++'..."

static const char* programName = PROGRAM_NAME;
static const char* programDescription = PROGRAM_DESCRIPTION;
static const char* programUsage = PROGRAM_USAGE;
static const char* version = VERSION;

// =================================================================================================
// Helper methods.

/**
 * This method prints the help message to stdout - including the program name, the program version,
 * a general program description and a description of the command-line options.
 *
 * @param publicOpts
 *    The definition of the public command-line options.
 * @param privateOpts
 *    The definition of the non-public command-line options.
 *    NOTE: When omitted, the non-public options do not appear in the help message.
 * @param maxLineLength
 *    The maximum length of lines in the help message.
 * @param optionDescIndent
 *    The amount by which the description of a single command line option should be indented.
 */
void printHelpMessage(
    const options_description* publicOpts,
    const options_description* privateOpts = 0,
    int maxLineLength = 100,
    int optionDescIndent = 4) {
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
 * This method prints the version message to stdout.
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

  // The format in which the extracted text should be output.
  SerializationFormat format = SerializationFormat::TXT;
  // The roles filter (blocks with a role that do not appear in this vector will not be output).
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
  bool debugStatisticsCalculation = false;
  bool debugDiacriticMarksMerging = false;
  bool debugWordsDetection = false;
  bool debugPageSegmentation = false;
  bool debugTextLinesDetection = false;
  bool debugSubSuperScriptsDetection = false;
  bool debugTextBlocksDetection = false;
  int logPageFilter = -1;
  bool printRunningTimes = false;
  bool printVersion = false;
  bool printHelp = false;
  bool printFullHelp = false;

  // Specify the public options (= options that will be shown to the user when printing the help).
  options_description publicOpts;
  publicOpts.add_options()
    (
      "format",
      value<SerializationFormat>(&format),
      // TODO(korzen): Add an description for each serializer.
      ("Output the extracted text in the specified format.\n"
       "Valid formats: " + ppp::serialization::getSerializationFormatChoicesStr() + ".\n").c_str()
    )
    (
      "role",
      value<vector<SemanticRole>>(&roles),
      ("Output only the text from text blocks with the specified roles. Use the syntax "
       "'--role <value1> --role <value2> ...' to specify multiple roles.\n"
       "Valid roles: " + ppp::types::getSemanticRolesStr() + ".\n").c_str()
    )
    (
      "unit",
      value<vector<DocumentUnit>>(&units),
      ("Output semantic and layout information about the specified document unit(s). Use "
       "the syntax '--unit <value1> --unit <value2> ...' to specify multiple units."
       "Valid units: " + ppp::types::getDocumentUnitsStr() + ".\n"
       "NOTE: This option does not have an effect when used together with the "
       "'--format txt' or '--format txt-extended' option.").c_str()
    )
    (
      "control-characters",
      bool_switch(&addControlCharacters),
      "Output the extracted text together with the following control characters:\n"
      "• \"^A\" (start of heading) in front of each emphasized text block\n"
      "• \"^L\" (form feed) between two text blocks when there is a page break in between.\n"
      "NOTE: This option only has an effect when used together with the '--format txt' option."
    )
    // TODO(korzen): This option should be removed, when there are more structured formats.
    (
      "semantic-roles",
      bool_switch(&addSemanticRoles),
      "Output each extracted text block together with its semantic role. "
      "NOTE: This option only has an effect when used together with the '--format txt' option."
    )
    (
      "no-scripts",
      bool_switch(&noScripts),
      "Output the extracted text without characters that appear as a subscript or superscript in "
      "the PDF."
    )
    (
      "no-dehyphenation",
      bool_switch(&noDehyphenation),
      "Do not merge hyphenated words. "
      "NOTE: Using this option has the consequence that each part into which a hyphenated word is "
      "divided appears as a separate word in the extracted text."
    )
    (
      "no-embedded-font-files",
      bool_switch(&noEmbeddedFontFiles),
      "Do not parse the font files embedded into the PDF file. "
      "NOTE: Using this option results in a faster extraction process, but a less accurate "
      "extraction result."
    )
    (
      "visualization-path",
      value<string>(&visualizeFilePath),
      "Create a visualization PDF file, that is: a copy of the PDF file, with annotations added "
      "by the different '--visualize-*' options below, and write it to the specified file. "
      "NOTE: If not specified, no such visualization will be created, even if one or more of the "
      "'--visualize-*' options is used."
    )
    (
      "visualize-characters",
      bool_switch(&visualizeChars),
      "Draw bounding boxes around the detected characters into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    // TODO(korzen): explain the difference between graphics and figure.
    (
      "visualize-graphics",
      bool_switch(&visualizeGraphics),
      "Draw bounding boxes around the detected graphics into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "visualize-figures",
      bool_switch(&visualizeFigures),
      "Draw bounding boxes around the detected figures into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "visualize-shapes",
      bool_switch(&visualizeShapes),
      "Draw bounding boxes around the detected shapes into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "visualize-words",
      bool_switch(&visualizeWords),
      "Draw bounding boxes around the detected words into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "visualize-text-lines",
      bool_switch(&visualizeTextLines),
      "Draw bounding boxes around the detected text lines into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "visualize-text-blocks",
      bool_switch(&visualizeTextBlocks),
      "Draw bounding boxes around the detected text blocks into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "visualize-segments",
      bool_switch(&visualizePageSegments),
      "Draw bounding boxes around the detected page segments into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "visualize-segment-cuts",
      bool_switch(&visualizeSegmentCuts),
      "Draw the XY-cuts made to segment the pages into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "visualize-reading-order",
      bool_switch(&visualizeReadingOrder),
      "Draw (directed) edges between the detected text blocks into the visualization PDF file, for "
      "visualizing the detected reading order. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "visualize-reading-order-cuts",
      bool_switch(&visualizeReadingOrderCuts),
      "Draw the XY-cuts made to detect the reading order into the visualization PDF file. "
      "NOTE: This option only has an effect when used together with the "
      "'--visualization-path <string>' option."
    )
    (
      "verbosity",
      value<string>(&verbosity),
      "Specify the verbosity. Valid values are: trace, debug, info, warn, error. "
      "Logging messages with a level lower than the specified value will be not printed to the "
      "console. Default: error."  // TODO(korzen): Specify the default value.
    )
    (
      "version,v",
      bool_switch(&printVersion),
      "Print the version info."
    )
    (
      "help,h",
      bool_switch(&printHelp),
      "Print the help.");

  // Specify the private options (= options that will not be shown in the help message).
  // NOTE: these options *must* include an entry for each positional option (defined below) - this
  // is a restriction of the boost library.
  options_description privateOpts("");
  privateOpts.add_options()
    (
      "pdf-file",
      value<string>(&pdfFilePath)->required(),
      "The path to the PDF file to process."
    )
    (
      "output-file",
      value<string>(&outputFilePath),
      "The path to the file into which the extracted text should be written."
    )
    (
      "parse-mode",
      bool_switch(&parseMode),
      "Only parse the content streams of the PDF file for characters, figures, and shapes. "
      "Do not detect words, text lines, and text blocks. "
      "NOTE: To output the extracted elements, use the '--output-characters', '--output-figures' "
      "and/or '--output-shapes' options."
    )
    // TODO(korzen): Think about how to handle logging (one log per module?).
    (
      "debug-pdf-parsing",
      bool_switch(&debugPdfParsing),
      "Print the debug messages produced while parsing the content streams of the PDF file."
    )
    (
      "debug-statistics",
      bool_switch(&debugStatisticsCalculation),
      "Print the debug messages produced while calculating the statistics."
    )
    (
      "debug-diacritic-marks-merging",
      bool_switch(&debugDiacriticMarksMerging),
      "Print the debug messages produced while merging diacritical marks with their base "
      "characters."
    )
    (
      "debug-words-detection",
      bool_switch(&debugWordsDetection),
      "Print the debug messages produced while detecting words."
    )
    (
      "debug-page-segmentation",
      bool_switch(&debugPageSegmentation),
      "Print the debug messages produced while segmenting the pages."
    )
    (
      "debug-text-lines-detection",
      bool_switch(&debugTextLinesDetection),
      "Print the debug messages produced while detecting text lines."
    )
    (
      "debug-sub-super-scripts-detection",
      bool_switch(&debugSubSuperScriptsDetection),
      "Print the debug messages produced while detecting sub-/superscripts."
    )
    (
      "debug-text-blocks-detection",
      bool_switch(&debugTextBlocksDetection),
      "Print the debug messages produced while detecting text blocks."
    )
    (
      "log-page-filter",
      value<int>(&logPageFilter),
      "When specified, only the logging messages that are produced while processing the specified "
      "page is printed to the console. "
      "NOTE: the page numbers are 1-based; so to print the logging messages produced while "
      "processing the first page, type \"--log-page-filter 1\". "
    )
    (
      "print-running-times",
      bool_switch(&printRunningTimes),
      "Print the running times needed by the different steps in the extraction pipeline."
    )
    (
      "full-help",
      bool_switch(&printFullHelp),
      "Print the full help (containing also the descriptions of all non-public commands).");

  // Specify the positional options.
  positional_options_description positional;
  positional.add("pdf-file", 1);
  positional.add("output-file", 1);

  // Parse the command-line options.
  options_description opts;
  opts.add(publicOpts).add(privateOpts);
  variables_map vm;
  try {
    store(command_line_parser(argc, argv).options(opts).positional(positional).run(), vm);
    notify(vm);
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
  if (verbosity == "trace" || verbosity == "TRACE") { logLevel = TRACE; }
  if (verbosity == "debug" || verbosity == "DEBUG") { logLevel = DEBUG; }
  if (verbosity == "info" || verbosity == "INFO") { logLevel = INFO; }
  if (verbosity == "warn" || verbosity == "WARN") { logLevel = WARN; }

  Config config;
  // Configure pdf parsing.
  config.pdfParsing.logLevel = debugPdfParsing ? DEBUG : logLevel;
  config.pdfParsing.logPageFilter = logPageFilter;
  config.pdfParsing.disableParsingEmbeddedFontFiles = noEmbeddedFontFiles;
  // Configure statistics calculation.
  config.statisticsCalculation.logLevel = debugStatisticsCalculation ? DEBUG : logLevel;
  config.statisticsCalculation.logPageFilter = logPageFilter;
  // Configure diacritics merging.
  config.diacriticalMarksMerging.logLevel = debugDiacriticMarksMerging ? DEBUG : logLevel;
  config.diacriticalMarksMerging.logPageFilter = logPageFilter;
  // Configure words detection.
  config.wordsDetection.logLevel = debugWordsDetection ? DEBUG : logLevel;
  config.wordsDetection.logPageFilter = logPageFilter;
  // Configure text lines detection.
  config.textLinesDetection.logLevel = debugTextLinesDetection ? DEBUG : logLevel;
  config.textLinesDetection.logPageFilter = logPageFilter;
  // Configure sub-/super scripts detection.
  config.subSuperScriptsDetection.logLevel = debugSubSuperScriptsDetection ? DEBUG : logLevel;
  config.subSuperScriptsDetection.logPageFilter = logPageFilter;
  // Configure text blocks detection.
  config.textBlocksDetection.logLevel = debugSubSuperScriptsDetection ? DEBUG : logLevel;
  config.textBlocksDetection.logPageFilter = logPageFilter;
  // Configure reading order detection.
  config.readingOrderDetection.logLevel = logLevel;
  config.readingOrderDetection.logPageFilter = logPageFilter;
  config.semanticRolesPrediction.logLevel = logLevel;
  config.semanticRolesPrediction.logPageFilter = logPageFilter;
  config.semanticRolesPrediction.modelsDir = CONFIG_SEMANTIC_ROLES_DETECTION_MODELS_DIR;
  // Configure words dehyphenation.
  config.wordsDehyphenation.logLevel = logLevel;
  config.wordsDehyphenation.logPageFilter = logPageFilter;
  config.wordsDehyphenation.disable = noDehyphenation;

  PdfToTextPlusPlus engine(&config, parseMode);

  PdfDocument doc;
  vector<Timing> timings;

  int status = 0;
  // TODO(korzen): Don't use the invalid argument exception; it is currently thrown by
  // SemanticRolesPredictor. Instead, use the exit code to check if something went wrong.
  try {
    status = engine.process(&pdfFilePath, &doc, &timings);
  } catch (const invalid_argument& ia) {
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
    PdfDocumentVisualization pdv(pdfFilePath, config.pdfDocumentVisualization);
    if (visualizeChars) {
      pdv.visualizeCharacters(doc, ppp::visualization::color_schemes::blue);
    }
    if (visualizeFigures) {
      pdv.visualizeFigures(doc, ppp::visualization::color_schemes::blue);
    }
    if (visualizeShapes) {
      pdv.visualizeShapes(doc, ppp::visualization::color_schemes::blue);
    }
    if (visualizeGraphics) {
      pdv.visualizeGraphics(doc, ppp::visualization::color_schemes::blue);
    }
    if (visualizeWords) {
      pdv.visualizeWords(doc, ppp::visualization::color_schemes::blue);
    }
    if (visualizeTextLines) {
      pdv.visualizeTextLines(doc, ppp::visualization::color_schemes::blue);
    }
    if (visualizeTextBlocks) {
      pdv.visualizeTextBlocks(doc, ppp::visualization::color_schemes::red);
    }
    if (visualizePageSegments) {
      pdv.visualizePageSegments(doc, ppp::visualization::color_schemes::blue);
    }
    if (visualizeReadingOrder) {
      pdv.visualizeReadingOrder(doc, ppp::visualization::color_schemes::blue);
    }
    if (visualizeSegmentCuts) {
      pdv.visualizeSegmentCuts(doc, ppp::visualization::color_schemes::blue);
    }
    if (visualizeReadingOrderCuts) {
      pdv.visualizeReadingOrderCuts(doc, ppp::visualization::color_schemes::blue);
    }
    pdv.save(visualizeFilePathStr);
    auto end = high_resolution_clock::now();
    Timing timingVisualizing("Visualize", duration_cast<milliseconds>(end - start).count());
    timings.push_back(timingVisualizing);
  }

  // Print the running times needed by the different pipeline steps, if requested by the user.
  if (printRunningTimes) {
    // Print the values with thousands separator.
    cout.imbue(locale(""));
    // Print the values with a precision of one decimal point.
    cout << fixed;
    cout << setprecision(1);

    int64_t timeTotal = 0;
    for (const auto& timing : timings) { timeTotal += timing.time; }
    cout << BOLD << "Finished in " << timeTotal << " ms." << OFF << endl;

    for (const auto& timing : timings) {
      string prefix = " * " + timing.name + ":";
      cout << std::left << setw(25) << prefix;
      cout << std::right << setw(4) << timing.time << " ms ";
      double time = round(timing.time / static_cast<double>(timeTotal) * 100, 1);
      cout << "(" << time << "%)";
      cout << endl;
    }
  }

  return EXIT_SUCCESS;
}
