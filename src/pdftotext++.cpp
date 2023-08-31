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
#include "./utils/PdfElementsUtils.h"
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
using ppp::serialization::getSerializationFormatChoicesStr;
using ppp::serialization::Serializer;  // TODO(korzen)
using ppp::types::PdfElementType;
using ppp::types::PdfDocument;
using ppp::types::SemanticRole;
using ppp::types::SerializationFormat;
using ppp::types::Timing;
using ppp::utils::elements::getPdfElementTypes;
using ppp::utils::elements::getPdfElementTypesStr;
using ppp::utils::elements::getSemanticRoles;
using ppp::utils::elements::getSemanticRolesStr;
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
  vector<SemanticRole> roles = getSemanticRoles();
  // The text elements to output, specified by their types.
  vector<PdfElementType> types = getPdfElementTypes();

  bool addControlCharacters = false;  // TODO(korzen): Introduce an extra format.
  bool addSemanticRoles = false;  // TODO(korzen): Introduce an extra format.
  bool noScripts = false;  // TODO(korzen): Allow to disable all optional components of pipeline.
  bool noDehyphenation = false;
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
  bool skipEmbeddedFontFilesParsing = false;
  bool skipGlyphsStatisticsCalculation = false;
  bool skipDiacriticalMarksMerging = false;
  bool skipWordsDetection = false;
  bool skipWordsStatisticsCalculation = false;
  bool skipPageSegmentation = false;
  bool skipTextLinesDetection = false;
  bool skipSubSuperScriptsDetection = false;
  bool skipLineStatisticsCalculation = false;
  bool skipTextBlocksDetection = false;
  bool skipReadingOrderDetection = false;
  bool skipSemanticRolesPrediction = false;
  bool skipWordsDehyphenation = false;
  bool debugPdfParsing = false;  // TODO(korzen): Introduce advanced logging.
  bool debugGlyphsStatisticsCalculation = false;
  bool debugDiacriticalMarksMerging = false;
  bool debugWordsDetection = false;
  bool debugWordsStatisticsCalculation = false;
  bool debugPageSegmentation = false;
  bool debugTextLinesDetection = false;
  bool debugSubSuperScriptsDetection = false;
  bool debugLineStatisticsCalculation = false;
  bool debugTextBlocksDetection = false;
  bool debugReadingOrderDetection = false;
  bool debugSemanticRolesPrediction = false;
  bool debugWordsDehyphenation = false;
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
       "Valid formats: " + getSerializationFormatChoicesStr() + ".\n").c_str()
    )
    (
      "role",
      value<vector<SemanticRole>>(&roles),
      ("Output only the text from text blocks with the specified roles. Use the syntax "
       "'--role <value1> --role <value2> ...' to specify multiple roles.\n"
       "Valid roles: " + getSemanticRolesStr() + ".\n").c_str()
    )
    (
      "type",
      value<vector<PdfElementType>>(&types),
      ("Output semantic and layout information about the specified type(s) of elements. Use "
       "the syntax '--type <value1> --type <value2> ...' to specify multiple types."
       "Valid types: " + getPdfElementTypesStr() + ".\n"
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
    // TODO(korzen): Think about how to handle logging (one log per module?).
    (
      "debug-pdf-parsing",
      bool_switch(&debugPdfParsing),
      "Print the debug messages produced while parsing the content streams of the PDF file."
    )
    (
      "skip-embedded-font-files-parsing",
      bool_switch(&skipEmbeddedFontFilesParsing),
      "Skip the parsing of embedded font files. "
      "NOTE: Using this option results in a faster extraction process, but a less accurate "
      "extraction result."
    )
    (
      "debug-glyphs-statistics-calculation",
      bool_switch(&debugGlyphsStatisticsCalculation),
      "Print the debug messages produced while calculating the glyph statistics."
    )
    (
      "skip-glyphs-statistics-calculation",
      bool_switch(&skipGlyphsStatisticsCalculation),
      "Skip the calculation of glyph statistics."
    )
    (
      "debug-diacritic-marks-merging",
      bool_switch(&debugDiacriticalMarksMerging),
      "Print the debug messages produced while merging diacritical marks with their base "
      "characters."
    )
    (
      "skip-diacritic-marks-merging",
      bool_switch(&skipDiacriticalMarksMerging),
      "Skip the merging of diacritical marks with their base characters."
    )
    (
      "debug-words-detection",
      bool_switch(&debugWordsDetection),
      "Print the debug messages produced while detecting words."
    )
    (
      "skip-words-detection",
      bool_switch(&skipWordsDetection),
      "Skip the detection of words."
    )
    (
      "debug-words-statistics-calculation",
      bool_switch(&debugWordsStatisticsCalculation),
      "Print the debug messages produced while calculating the word statistics."
    )
    (
      "skip-words-statistics-calculation",
      bool_switch(&skipWordsStatisticsCalculation),
      "Skip the calculation of word statistics."
    )
    (
      "debug-page-segmentation",
      bool_switch(&debugPageSegmentation),
      "Print the debug messages produced while segmenting the pages."
    )
    (
      "skip-page-segmentation",
      bool_switch(&skipPageSegmentation),
      "Skip the segmentation of pages."
    )
    (
      "debug-text-lines-detection",
      bool_switch(&debugTextLinesDetection),
      "Print the debug messages produced while detecting text lines."
    )
    (
      "skip-text-lines-detection",
      bool_switch(&skipTextLinesDetection),
      "Skip the detection of text lines."
    )
    (
      "debug-sub-super-scripts-detection",
      bool_switch(&debugSubSuperScriptsDetection),
      "Print the debug messages produced while detecting sub-/superscripts."
    )
    (
      "skip-sub-super-scripts-detection",
      bool_switch(&skipSubSuperScriptsDetection),
      "Skip the detection of sub-/superscripts."
    )
    (
      "debug-text-lines-statistics-calculation",
      bool_switch(&debugLineStatisticsCalculation),
      "Print the debug messages produced while calculating the text line statistics."
    )
    (
      "skip-text-lines-statistics-calculation",
      bool_switch(&skipLineStatisticsCalculation),
      "Skip the calculation of text line statistics."
    )
    (
      "debug-text-blocks-detection",
      bool_switch(&debugTextBlocksDetection),
      "Print the debug messages produced while detecting text blocks."
    )
    (
      "skip-text-blocks-detection",
      bool_switch(&skipTextBlocksDetection),
      "Skip the detection of text blocks."
    )
    (
      "debug-reading-order-detection",
      bool_switch(&debugReadingOrderDetection),
      "Print the debug messages produced while detecting the reading order."
    )
    (
      "skip-reading-order-detection",
      bool_switch(&skipReadingOrderDetection),
      "Skip the detection of the reading order."
    )
    (
      "debug-semantic-roles-prediction",
      bool_switch(&debugSemanticRolesPrediction),
      "Print the debug messages produced while predicting the semantic roles."
    )
    (
      "skip-semantic-roles-prediction",
      bool_switch(&skipSemanticRolesPrediction),
      "Skip the prediction of the semantic roles."
    )
    (
      "debug-words-dehyphenation",
      bool_switch(&debugWordsDehyphenation),
      "Print the debug messages produced while dehyphenating words."
    )
    (
      "skip-words-dehyphenation",
      bool_switch(&skipWordsDehyphenation),
      "Skip the dehyphenation of words."
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

  Config conf;
  // Configure pdf parsing.
  conf.pdfParsing.logLevel = debugPdfParsing ? DEBUG : logLevel;
  conf.pdfParsing.logPageFilter = logPageFilter;
  conf.pdfParsing.skipEmbeddedFontFilesParsing = skipEmbeddedFontFilesParsing;
  // Configure the calculation of glyphs statistics.
  conf.glyphsStatisticsCalculation.disabled = skipGlyphsStatisticsCalculation;
  conf.glyphsStatisticsCalculation.logLevel = debugGlyphsStatisticsCalculation ? DEBUG : logLevel;
  conf.glyphsStatisticsCalculation.logPageFilter = logPageFilter;
  // Configure the merging of combining diacritical marks.
  conf.diacriticalMarksMerging.disabled = skipDiacriticalMarksMerging;
  conf.diacriticalMarksMerging.logLevel = debugDiacriticalMarksMerging ? DEBUG : logLevel;
  conf.diacriticalMarksMerging.logPageFilter = logPageFilter;
  // Configure the detection of words.
  conf.wordsDetection.disabled = skipWordsDetection;
  conf.wordsDetection.logLevel = debugWordsDetection ? DEBUG : logLevel;
  conf.wordsDetection.logPageFilter = logPageFilter;
  // Configure the calculation of words statistics.
  conf.wordsStatisticsCalculation.disabled = skipWordsStatisticsCalculation;
  conf.wordsStatisticsCalculation.logLevel = debugWordsStatisticsCalculation ? DEBUG : logLevel;
  conf.wordsStatisticsCalculation.logPageFilter = logPageFilter;
  // Configure the segmentation of pages.
  conf.pageSegmentation.disabled = skipPageSegmentation;
  conf.pageSegmentation.logLevel = debugPageSegmentation ? DEBUG : logLevel;
  conf.pageSegmentation.logPageFilter = logPageFilter;
  // Configure the detection of text lines.
  conf.textLinesDetection.disabled = skipTextLinesDetection;
  conf.textLinesDetection.logLevel = debugTextLinesDetection ? DEBUG : logLevel;
  conf.textLinesDetection.logPageFilter = logPageFilter;
  // Configure the detection of sub-/superscripts.
  conf.subSuperScriptsDetection.disabled = skipSubSuperScriptsDetection;
  conf.subSuperScriptsDetection.logLevel = debugSubSuperScriptsDetection ? DEBUG : logLevel;
  conf.subSuperScriptsDetection.logPageFilter = logPageFilter;
  // Configure the calculation of text lines statistics.
  conf.textLinesStatisticsCalculation.disabled = skipLineStatisticsCalculation;
  conf.textLinesStatisticsCalculation.logLevel = debugLineStatisticsCalculation ? DEBUG : logLevel;
  conf.textLinesStatisticsCalculation.logPageFilter = logPageFilter;
  // Configure the detection of text blocks.
  conf.textBlocksDetection.disabled = skipTextBlocksDetection;
  conf.textBlocksDetection.logLevel = debugTextBlocksDetection ? DEBUG : logLevel;
  conf.textBlocksDetection.logPageFilter = logPageFilter;
  // Configure the detection of the reading order.
  conf.readingOrderDetection.disabled = skipReadingOrderDetection;
  conf.readingOrderDetection.logLevel = debugReadingOrderDetection ? DEBUG : logLevel;
  conf.readingOrderDetection.logPageFilter = logPageFilter;
  // Configure the prediction of semantic roles.
  conf.semanticRolesPrediction.disabled = skipSemanticRolesPrediction;
  conf.semanticRolesPrediction.logLevel = debugSemanticRolesPrediction ? DEBUG : logLevel;
  conf.semanticRolesPrediction.logPageFilter = logPageFilter;
  conf.semanticRolesPrediction.modelsDir = CONFIG_SEMANTIC_ROLES_DETECTION_MODELS_DIR;
  // Configure words dehyphenation.
  conf.wordsDehyphenation.disabled = skipWordsDehyphenation;
  conf.wordsDehyphenation.logLevel = debugWordsDehyphenation ? DEBUG : logLevel;
  conf.wordsDehyphenation.logPageFilter = logPageFilter;

  PdfDocument doc(pdfFilePath);
  vector<Timing> timings;

  int status = 0;
  // TODO(korzen): Don't use the invalid argument exception; it is currently thrown by
  // SemanticRolesPredictor. Instead, use the exit code to check if something went wrong.
  try {
    PdfToTextPlusPlus engine(&conf);
    status = engine.process(&doc, &timings);
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
    unordered_set<PdfElementType> typesSet(types.begin(), types.end());
    serializer->serialize(&doc, rolesSet, typesSet, outputFilePath);
  }
  auto end = high_resolution_clock::now();
  Timing timingSerializeChars("Serialize", duration_cast<milliseconds>(end - start).count());
  timings.push_back(timingSerializeChars);

  // Visualize the extraction result, if requested by the user.
  const string visualizeFilePathStr(visualizeFilePath);
  if (!visualizeFilePathStr.empty()) {
    auto start = high_resolution_clock::now();
    PdfDocumentVisualization pdv(pdfFilePath, conf.pdfDocumentVisualization);
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
