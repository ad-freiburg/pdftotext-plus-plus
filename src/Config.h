/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <unordered_set>

#include "./utils/Log.h"

using std::string;
using std::unordered_set;

namespace ppp {

static int DEFAULT_COORDS_PREC = 1;
static int DEFAULT_FONT_SIZE_PREC = 1;
static int DEFAULT_LINE_DIST_PREC = 1;

static double DEFAULT_COORDS_EQUAL_TOLERANCE = 0.1;
static double DEFAULT_FS_EQUAL_TOLERANCE = 1.0;

class Config {
 public:
  Config() = default;
  ~Config() = default;

  int idLength = 8;

  // The config for the TextOutputDev class.
  struct {
    // The log level.
    LogLevel logLevel = ERROR;

    // A parameter specifying a filter for the logging messages. If set to a value > 0, only the
    // logging messages produced while processing the <logPageFilter>-th page of the current PDF
    // file will be printed to the console. Otherwise, all logging messages will be printed.
    int logPageFilter = -1;

    // A boolean flag indicating whether or not to parse the font files embedded into the current
    // PDF file. Setting this parameter to true disables the parsing; setting it to false enables
    // the parsing. Parsing the font files can enable more accurate bounding boxes of the chars
    // (in particular, when the chars represent mathematical symbols). It also can enable more
    // correct information about the style of a font (for example, whether or not the font is a
    // bold font), for the following reason: actually, the PDF standard specifies several font
    // flags that describe the style of a font. These flags are however often not set, even if they
    // are supposed to be (for example, there is an isBold flag for a font, but this flag is often
    // not set, even if the font is actually a bold font). Instead, the missing information is
    // often stored in the embedded font file (if the font is actually embedded). The consequence
    // of disabling the parsing of the font files is a faster extraction process, but a lower
    // accuracy of the extracted text.
    bool noEmbeddedFontFilesParsing = false;

    // A parameter that denotes the precision to use when rounding the font sizes of text elements
    // before computing the most frequent font size.
    const double fsPrec = DEFAULT_FONT_SIZE_PREC;

    // A parameter used for computing if two given coordinates are equal. It denotes the maximum
    // allowed difference between two coordinates so that they are considered to be equal.
    const double coordsEqualTolerance = DEFAULT_COORDS_EQUAL_TOLERANCE;

  } textOutputDev;

  // The config for the PdfStatisticsCalculator class.
  struct {
    // The log level.
    LogLevel logLevel = ERROR;

    // A parameter that denotes the precision to use when rounding the coordinates of elements
    // before computing the most frequent coordinates.
    double coordsPrec = DEFAULT_COORDS_PREC;

    // A parameter that denotes the precision to use when rounding the font sizes of text elements
    // before computing the most frequent font size.
    const double fsPrec = DEFAULT_FONT_SIZE_PREC;

    // A parameter used for computing if two given font sizes are equal. It denotes the maximum
    // allowed difference between two font sizes so that the font sizes are considered to be equal.
    const double fsEqualTolerance = DEFAULT_FS_EQUAL_TOLERANCE;

    // ----------
    // computeWordStatistics()

    // A parameter that is used for computing whether or not two words are part of the same text
    // line. If the maximum y-overlap ratio between two words is larger or equal to the returned
    // threshold, the words are considered to be part of the same text line.
    const double sameLineYOverlapRatioThreshold = 0.5;

    // A parameter that is used for computing whether or not two words are part of different text
    // lines. If the maximum y-overlap ratio between two words is smaller or equal to the returned
    // threshold, the words are considered to be part of different text lines.
    const double otherLineYOverlapRatioThreshold = 0;

    // ----------
    // computeTextLineStatistics()

    // A parameter that denotes the precision to use when rounding the distances between text lines
    // before computing the most frequent line distance.
    const double lineDistPrec = DEFAULT_LINE_DIST_PREC;

  } pdfStatisticsCalculator;







  string semanticRolesDetectionModelsDir;

  int COORDS_PREC = 1;
  int FONT_SIZE_PREC = 1;
  int LINE_DIST_PREC = 1;

  double DOUBLE_EQUAL_TOLERANCE = 0.0001;
  double FS_EQUAL_TOLERANCE = 1;
  double COORDS_EQUAL_TOLERANCE = 0.1;

  int ID_LENGTH = 8;

  /** The characters to use as an identifier for formulas. */
  const char* FORMULA_ID_ALPHABET = "=+";

  /**
   * The characters to consider to be alphanumerical. Used by, for example, the
   * ppp::string_utils::createRandomString() method.
   */
  const char* ALPHA_NUM_ALPHABET = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  /** The characters to consider to be word delimiters.*/
  const char* WORD_DELIMITERS_ALPHABET = " \t\r\n\f\v";

  /**
   * The characters to consider to be a sentence delimiter. Used by, for example, the
   * text_element_utils::computeEndsWithSentenceDelimiter() method.
   */
  const char* SENTENCE_DELIMITER_ALPHABET = "?!.);";

  /**
   * A set of common last name prefixes. This is used, for example, by
   * text_blocks_utils::computeHangingIndent(), for checking if a given block is in hanging indent
   * format. Normally, a block is not considered as a block in hanging indent when it contains one
   * or more unindented lines that start with a lowercase character. However, there are references
   * in hanging indent format that start with a (lowercased) last name prefix. To not accidentally
   * consider a block to be not in hanging indent format when it contains a line starting with such
   * prefix, we do not count a line as a lowercased line if it starts with a word contained in
   * this set.
   */
  unordered_set<string> LAST_NAME_PREFIXES = { "van", "von", "de" };

  const int LEFT_MARGIN_PREC = 0;

  // The resolution in DPI.
  const double H_DPI = 72.0;
  const double V_DPI = 72.0;

  // ===============================================================================================

  /**
   * The characters which we consider, to be a valid footnote label (in addition to alphanumerical
   * symbols). This is used by, for example, the text_lines_utils::computePotentialFootnoteLabels()
   * method.
   */
  const char* const SPECIAL_FOOTNOTE_LABELS_ALPHABET = "*∗†‡§‖¶?";
};

}  // namespace ppp

#endif  // CONFIG_H_
