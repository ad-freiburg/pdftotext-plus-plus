/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef PDFTOTEXTPLUSPLUS_H_
#define PDFTOTEXTPLUSPLUS_H_

#include <string>
#include <vector>

#include "./PdfDocument.h"

using std::string;
using std::vector;

// =================================================================================================

/**
 * This class is the core class of pdftotext++. It is responsible for processing a given PDF file
 * and invoking the following modules of the extraction pipeline, for extracting the text from the
 * PDF file:
 *
 *  (1) Loading the PDF file;
 *  (2) Parsing the content streams of the PDF file for characters, figures and shapes;
 *  (3) Computing character statistics;
 *  (4) Merging combining diacritical marks with their base characters;
 *  (5) Detecting words;
 *  (6) Computing word statistics;
 *  (7) Segmenting the pages of the PDF file (= separating the words into columns);
 *  (8) Detecting text lines;
 *  (9) Detecting subscripted and superscripted characters;
 * (10) Computing text line statistics;
 * (11) Detecting text blocks;
 * (12) Detecting the reading order of the text blocks;
 * (13) Dehyphenating words.
 *
 * NOTE: If the parsing mode is activated (that is: if _parsingMode is set to true), the extraction
 * pipeline stops after step (4).
 */
class PdfToTextPlusPlus {
 public:
  /**
   * This constructor creates and initializes a new instance of this `PdfToTextPlusPlus` class.
   *
   * @param noEmbeddedFontFilesParsing
   *   A boolean flag indicating whether or not to parse the font files, embedded into the current
   *   PDF file, while parsing the content streams. Setting this parameter to true disables the
   *   parsing; setting it to false enables the parsing. Parsing the font files can enable more
   *   accurate bounding boxes of the characters (in particular, when the characters represent
   *   mathematical symbols). It also can enable more correct information about the style of a font
   *   (for example, whether or not the font is a bold font), for the following reason: actually,
   *   the PDF standard specifies several font flags that describe the style of a font. These flags
   *   are however often not set, even if they are supposed to be (for example, there is an isBold
   *   flag for a font, but this flag is often not set, even if the font is actually a bold font).
   *   Instead, the missing information is often stored in the embedded font file (if the font is
   *   actually embedded). The consequence of disabling the parsing of the font files is a faster
   *   extraction process, but a lower accuracy of the extracted text.
   * @param noWordsDehyphenation
   *   A boolean flag indicating whether or not to disable words dehyphenation. Setting this
   *   parameter to true disables words dehpyhenation; setting it to false enables it. Disabling
   *   words dehyphenation has the consequence that each part, into which a hyphenated word is
   *   split, will appear as a separate word in the extracted text.
   * @param parseMode
   *   A boolean flag indicating whether or not to activate the parsing mode, that is: a mode that
   *   parses the content streams of the PDF file for characters, figures, and shapes, and stops
   *   afterwards. Words, text lines, and text blocks are *not* detected. Setting this parameter to
   *   true enables the parsing mode, setting it to false disables the parsing mode.
   *   NOTE: This mode was introduced for the benchmark generator that needs information about the
   *   the contained characters (e.g., the position, text and color) for assembling the true words,
   *   text blocks, etc. based on the color of the characters.
   * @param debugPdfParsing
   *  A boolean flag indicating whether or not to print the debug messages, produced while parsing
   *  the content streams of the current PDF file, to the console.
   * @param debugDiacriticMarksMerging
   *  A boolean flag indicating whether or not to print the debug messages, produced while merging
   *  diacritical marks with their base characters, to the console.
   * @param debugWordsDetection
   *  A boolean flag indicating whether or not to print the debug messages, produced while
   *  detecting words, to the console.
   * @param debugPageSegmentation
   *  A boolean flag indicating whether or not to print the debug messages, produced while
   *  segmenting the pages, to the console.
   * @param debugTextLinesDetection
   *  A boolean flag indicating whether or not to print the debug messages, produced while
   *  detecting text lines, to the console.
   * @param debugTextBlocksDetection
   *  A boolean flag indicating whether or not to print the debug messages, produced while
   *  detecting text blocks, to the console.
   * @param debugPageFilter
   *  If set to a value > 0, only the debug messages, produced while processing the
   *  <debugPageFilter>-th page of the current PDF file, will be printed to the console. If set to
   *  a value <= 0, all debug messages will be printed to the console, no matter when (= for which
   *  page) they were produced. Note that the page numbers are 1-based; so to print only the
   *  messages produced while processing the first page, set this parameter to 1.
   */
  PdfToTextPlusPlus(
    bool noEmbeddedFontFilesParsing = false,
    bool noWordsDehyphenation = false,
    bool parseMode = false,
    bool debugPdfParsing = false,
    bool debugDiacriticMarksMerging = false,
    bool debugWordsDetection = false,
    bool debugPageSegmentation = false,
    bool debugTextLinesDetection = false,
    bool debugTextBlocksDetection = false,
    int debugPageFilter = -1);

  /** The deconstructor */
  ~PdfToTextPlusPlus();

  /**
   * This method extracts the text from the given PDF file, by processing the pipeline mentioned
   * in the preliminary comment of this class.
   *
   * @param pdfFilePath
   *   The path to the PDF file to process.
   * @param doc
   *   The `PdfDocument` instance to which the extracted text should be stored.
   * @param timing
   *   A vector to which the running times needed by the different modules of the extraction
   *   pipeline should be appended. If not set, no running times will be appended.
   *
   * @return
   *    0 if the PDF was processed successfully, and a value > 0 otherwise.
   */
  int process(const string& pdfFilePath, PdfDocument* doc, vector<Timing>* timings = nullptr) const;

 private:
  // Whether or not to parse the embedded font files of a PDF file.
  bool _noEmbeddedFontFilesParsing;
  // Whether or not to disable words dehyphenation.
  bool _noWordsDehyphenation;
  // Whether or not to activate the parsing mode.
  bool _parseMode;
  // Whether or not to print the debug messages produced while parsing the content streams.
  bool _debugPdfParsing;
  // Whether or not to print the debug messages produced while merging diacritical marks.
  bool _debugDiacMarksMerging;
  // Whether or not to print the debug messages produced while detecting words.
  bool _debugWordsDetection;
  // Whether or not to print the debug messages produced while segmenting pages.
  bool _debugPageSegmentation;
  // Whether or not to print the debug messages produced while detecting text lines.
  bool _debugTextLinesDetection;
  // Whether or not to print the debug messages produced while detecting text blocks.
  bool _debugTextBlocksDetection;
  // The page filter for the debug messages.
  int _debugPageFilter;
};

#endif  // PDFTOTEXTPLUSPLUS_H_
