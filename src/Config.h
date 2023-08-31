/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <algorithm>  // std::max
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>  // std::pair
#include <vector>

#include "./PdfDocument.h"
#include "./utils/Log.h"

using std::make_pair;
using std::pair;
using std::regex;
using std::regex_constants::icase;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

using ppp::types::PdfDocument;
using ppp::types::PdfTextLine;
using ppp::types::PdfWord;
using ppp::utils::log::LogLevel;

// =================================================================================================

namespace ppp::config {

// A parameter specifying the maximum allowed difference between two double values so that they
// are considered equal when compared.
static const double DEFAULT_DOUBLE_EQUAL_TOLERANCE = 0.0001;

// All alphanumerical characters.
static const char* const ALPHA_NUM = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";  // NOLINT

// All characters denoting a word delimiter.
static const char* const WORD_DELIMITERS_ALPHABET = " \t\r\n\f\v";

// All characters denoting a dentence delimiter.
static const char* const SENTENCE_DELIMITERS_ALPHABET = "?!.);";


struct BaseConfig {
  // A parameter specifying the verbosity of logging messages.
  LogLevel logLevel = LogLevel::ERROR;

  // The page filter for the logging messages. If set to a value i > 0, only the logging messages
  // produced while processing the <i>-th page of the current PDF file will be printed to the
  // console. Otherwise, all logging messages will be printed to the console.
  unsigned int logPageFilter = 0;

  // The length of the element's ids to be created.
  unsigned int idLength = 8;

  // TODO(korzen): Document these two parameters.
  double hDPI = 72.0;
  double vDPI = 72.0;

  // A parameter specifying the maximum allowed difference between two coordinates so that they are
  // considered equal when compared.
  double coordsEqualTolerance = 0.1;

  // A parameter specifying the maximum allowed difference between two font sizes so that they are
  // considered equal when compared.
  double fsEqualTolerance = 1.0;

  // A parameter specifying the maximum allowed difference between two font weights so that they
  // are considered equal when compared.
  int fontWeightEqualTolerance = 100;

  // A parameter specifying to how many decimal places a coordinate should be rounded before
  // outputting the coordinate.
  int coordinatePrecision = 1;

  // A parameter specifying to how many decimal places a font size should be rounded before
  // computing the most frequent font size or before outputting the font size.
  int fontSizePrecision = 1;

  // A parameter specifying to how many decimal places a computed line distance should be rounded
  // before computing the most frequent line distance.
  int lineDistancePrecision = 1;

  // The characters to use as an identifier for formulas.
  string formulaIdAlphabet = "=+";

  // The characters to consider to be alphanumerical.
  string alphaNumAlphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  // The characters to consider to be a valid footnote label (in addition to alphanumerical
  // symbols). This is used by, for example, the text_lines_utils::computePotentialFootnoteLabels().
  string specialFootnoteLabelsAlphabet = "*∗†‡§‖¶?";
};

// =================================================================================================

/**
 * The config to use for parsing a PDF file for elements like glyphs, figures, shapes, etc.
 */
struct PdfParsingConfig : BaseConfig {
  // A boolean flag indicating whether or not to disable parsing the font files embedded into the
  // current PDF file. Parsing the embedded font files can enable more accurate bounding boxes of
  // the extracted glyphs (in particular, when the chars represent mathematical symbols). It also
  // can enable more correct information about the style of a font (for example, whether or not the
  // font is a bold font), for the following reason: actually, the PDF standard specifies several
  // font flags that describe the style of a font. These flags are however often not set, even if
  // they are supposed to be (for example, there is an isBold flag for a font, but this flag is
  // often not set, even if the font is actually a bold font). Instead, the missing information is
  // often stored in the embedded font file (if the font is actually embedded). The consequence
  // of disabling the parsing of embedded font files is a faster extraction process, but a lower
  // accuracy of the extracted text.
  bool skipEmbeddedFontFilesParsing = false;
};

// =================================================================================================

/**
 * The config to use for calculating glyph statistics.
 */
struct GlyphsStatisticsCalculationConfig : BaseConfig {
  // A parameter specifying whether or not to disable the calculation of glyph statistics.
  bool disabled = false;
};

/**
 * The config to use for calculating word statistics.
 */
struct WordsStatisticsCalculationConfig : BaseConfig {
  // A parameter specifying whether or not to disable the calculation of word statistics.
  bool disabled = false;

  // A parameter in [0, 1] that is used for computing the most frequent distance between two words
  // of the same text line. At time of computing, there is no information about which words
  // are part of the same text line. We therefore use the following heuristic:
  // Given two words v and w, r(v) is the percentage of v's height overlapped by w and r(w) is the
  // percentage of w's height overlapped by v. The words are considered part of the same text line
  // if the maximum of r(v) and r(w) is larger or equal to the specified value.
  double minYOverlapRatioSameLine = 0.5;

  // A parameter in [0, 1] that is used for estimating the most frequent line distance. At time of
  // computing, there is no information about text lines. We therefore compute the most frequent
  // vertical distance between two words v and w for which the maximum of r(v) and r(w) is smaller
  // than the specified value (meaning that they don't overlap vertically, or only slightly).
  double maxYOverlapRatioDifferentLine = 0;
};

/**
 * The config to use for calculating text line statistics.
 */
struct TextLinesStatisticsCalculationConfig : BaseConfig {
  // A parameter specifying whether or not to disable the calculation of text line statistics.
  bool disabled = false;
};

// =================================================================================================

/**
 * The config to use for detecting words.
 */
struct WordsDetectionConfig : BaseConfig {
  // A parameter specifying whether or not to disable the detection of words.
  bool disabled = false;

  // A parameter specifying a threshold for the vertical overlap between the current character
  // and the active word. If the maximum y-overlap ratio between the active word and the current
  // character is larger or equal to this threshold, the character is considered to be a part of
  // the active word; otherwise it is considered to be not a part.
  double minYOverlapRatio = 0.5;

  // A PDF can contain "stacked math symbols", which we want to merge to a single word (see the
  // preliminary comment of the `WordsDetector` class below for more information about how stacked
  // math symbols are defined). The following three sets are used to identify the base word of a
  // stacked math symbol.
  // The first set contains the *text* of characters that are likely to be part of a base word of
  // a stacked math symbol. If a word indeed contains a character that is part of this set, it is
  // considered to be the base word of a stacked math symbol.
  // The second set contains the *names* of characters that are likely to be part of a base word
  // of a stacked math symbol. If a word contains a character with a name that is part of this
  // set, it is considered to be the base word of a stacked math symbol (NOTE: this set was
  // introduced because, in some PDFs, the text of summation symbols does not contain a summation
  // symbol, but some weird symbols (e.g., a "?"), most typically because of a missing encoding.
  // The names of the characters are simply an additional indicator for identifying the base word
  // of a stacked math symbol).
  // The third set contains *words* that are likely to be a base word of a stacked math symbol.
  unordered_set<string> stackedMathCharTexts = { "∑", "∏", "∫", "⊗" };
  unordered_set<string> stackedMathCharNames = {
    "summationdisplay",
    "productdisplay",
    "integraldisplay",
    "circlemultiplydisplay" };
  unordered_set<string> stackedMathWords = { "sup", "lim" };

  // A parameter that is used for detecting words that are part of a stacked math symbol. It
  // denotes the minimum x-overlap ratio between a word w and the base word of the stacked math
  // symbol, so that w is considered to be a part of the stacked math symbol.
  double minStackedMathSymbolXOverlapRatio = 0.5;

  /**
   * This method returns a threshold to be used for checking if the horizontal gap between the
   * given active word and a character is large enough in order to be considered as a word
   * delimiter. If the horizontal gap between the word and the character is larger than this
   * threshold, it is considered to be a word delimiter.
   *
   * @param doc
   *    The currently processed document.
   * @param word
   *    The active word.
   *
   * @return
   *    The threshold.
   */
  // TODO(korzen): Parameterize the 0.15.
  constexpr static double getHorizontalGapThreshold(const PdfDocument* doc, const PdfWord* word) {
    return 0.15 * word->fontSize;
  }
};

// =================================================================================================

/**
 * The config to use for merging diacritic marks with their base characters.
 */
struct DiacriticalMarksMergingConfig : BaseConfig {
  // A parameter specifying whether or not to disable the merging of diacritic marks.
  bool disabled = false;

  // Diacritical marks exist in two variants: a "non-combining" variant and a "combining"
  // variant. For example, for the grave accent ("`"), the non-combining variant is 0x0060
  // ("GRAVE ACCENT"), and the combining variant is 0x0300 ("COMBINING GRAVE ACCENT"). In PDF,
  // diacritic marks can occur in both variants. But for merging, we need the combining variant.
  // This map maps non-combining diacritic marks to their combining equivalents.
  unordered_map<unsigned int, unsigned int> combiningMap = {
    { 0x0022, 0x030B },
    { 0x0027, 0x0301 },
    { 0x005E, 0x0302 },
    { 0x005F, 0x0332 },
    { 0x0060, 0x0300 },
    { 0x007E, 0x0303 },
    { 0x00A8, 0x0308 },
    { 0x00AF, 0x0304 },
    { 0x00B0, 0x030A },
    { 0x00B4, 0x0301 },
    { 0x00B8, 0x0327 },
    { 0x02B2, 0x0321 },
    { 0x02B7, 0x032B },
    { 0x02B9, 0x0301 },
    { 0x02CC, 0x0329 },
    { 0x02BA, 0x030B },
    { 0x02BB, 0x0312 },
    { 0x02BC, 0x0313 },
    { 0x02BD, 0x0314 },
    { 0x02C6, 0x0302 },
    { 0x02C7, 0x030C },
    { 0x02C8, 0x030D },
    { 0x02C9, 0x0304 },
    { 0x02CA, 0x0301 },
    { 0x02CB, 0x0300 },
    { 0x02CD, 0x0331 },
    { 0x02D4, 0x031D },
    { 0x02D5, 0x031E },
    { 0x02D6, 0x031F },
    { 0x02D7, 0x0320 },
    { 0x02DA, 0x030A },
    { 0x02DC, 0x0303 },
    { 0x02DD, 0x030B },
    { 0x0384, 0x0301 },
    { 0x0485, 0x0314 },
    { 0x0486, 0x0313 },
    { 0x0559, 0x0314 },
    { 0x055A, 0x0313 },
    { 0x204E, 0x0359 }
  };
};

// =================================================================================================

/**
 * The config to use for segmenting pages.
 */
struct PageSegmentationConfig : BaseConfig {
  // A parameter specifying whether or not to disable the segmentation of pages.
  bool disabled = false;

  // ----------
  // processPage()

  // A parameter that denotes the maximum number of elements an x-cut is allowed to overlap.
  double xCutMaxNumOverlappingElements = 1;

  /**
   * This method returns the minimum width of a horizontal gap between two elements for
   * considering the position between the elements as a valid position for an x-cut candidate.
   * This value is passed as the `minXCutGapWidth` parameter to the xyCut() and xCut() method.
   *
   * @param doc
   *    The PDF document currently processed.
   *
   * @return
   *    The minimum gap width of an x-cut.
   */
  // TODO(korzen): Parameterize the 2.0;
  constexpr static double getXCutMinGapWidth(const PdfDocument* doc) {
    return 2.0 * doc->mostFreqWordDistance;
  }

  /**
   * This method returns the minimum height of a vertical gap between two elements for considering
   * the position between the elements as a valid position for an y-cut candidate. This value is
   * passed as the `minYCutGapWidth` parameter to the xyCut() method.
   *
   * @param doc
   *    The PDF document currently processed.
   *
   * @return
   *    The minimum gap height of an y-cut.
   */
  // TODO(korzen): Parameterize the 2.0.
  constexpr static double getYCutMinGapHeight(const PdfDocument* doc) {
    return 2.0;
  }

  // ----------
  // chooseXCut_overlappingElements()

  // A parameter that is used for choosing x-cut candidates. It denotes the minimum number of
  // elements an x-cut must at least divide, so that the cut is allowed to overlap one or more
  // elements. The purpose of this threshold is to allow for overlapping elements only when the
  // group divided by a cut is large enough (small groups are divided accidentally too often).
  size_t overlappingMinNumElements = 500;

  /**
   * This method returns a threshold that is used for deciding if an element overlapped by an
   * x-cut is positioned at the top or at the bottom of an x-cut. If the top margin of an
   * overlapped element (= the vertical distance between the upperY of the element and the upperY
   * of the cut) is smaller than this threshold, the element is considered to be positioned at
   * the top of the cut. If the bottom margin of an overlapped element (= the vertical distance
   * between the lowerY of the cut and the lowerY of an element) is smaller than this threshold,
   * it is considered to be positioned at the bottom of the cut.
   *
   * @param doc
   *    The currently processed document.
   *
   * @return
   *    The threshold.
   */
  constexpr static double getOverlappingElementsMarginThreshold(const PdfDocument* doc) {
    return 5.0 * doc->avgCharHeight;
  }

  // ----------
  // chooseXCut_smallGapWidthHeight()

  /**
   * This method returns two thresholds that are used for deciding if the gap width *and* gap
   * height of a given x-cut is too small in order to be a valid x-cut. The first value denotes
   * the threshold for the gap width, the second value denotes the threshold for the gap height.
   * If the gap width of an x-cut is smaller than the first value *and* the gap height of the
   * same x-cut is smaller than the second value, the cut will *not* be chosen.
   *
   * @param doc
   *    The currently processed document.
   *
   * @return
   *    The two thresholds.
   */
  constexpr static pair<double, double> getSmallGapWidthHeightThresholds(const PdfDocument* doc) {
    return make_pair(2.0 * doc->avgCharWidth, 6.0 * doc->avgCharHeight);
  }

  // ----------
  // chooseXCut_contiguousWords()

  // A parameter that is used for choosing x-cut candidates. It denotes the minimum y-overlap
  // ratio between two words so that the words are considered to be contiguous.
  double contiguousWordsYOverlapRatioThreshold = 0.1;

  // ----------
  // chooseXCut_slimGroups()

  /**
   * This method returns a threshold that is used for checking if the width of one of the groups
   * resulting from an x-cut is too small. If the width of one of the groups resulting from an
   * x-cut is smaller than this threshold, the cut will not be chosen.
   *
   * @param doc
   *    The currently processed document.
   *
   * @return
   *    The threshold.
   */
  constexpr static double getSlimGroupWidthThreshold(const PdfDocument* doc) {
    return 10 * doc->avgCharWidth;
  }
};

// =================================================================================================

struct TextLinesDetectionConfig : BaseConfig {
  // A parameter specifying whether or not to disable the detection of text lines.
  bool disabled = false;

  // -------
  // Config for computeTextLineHierarchy().

  // A parameter that is used for computing the text line hierarchy. It denotes the maximum line
  // distance between two text lines so that the one text line is considered to be a candidate for
  // the parent text line (or a sibling text line) of the other text line.
  double lineHierarchyMaxLineDist = 10.0;

  // A factor that is used for computing a threshold for determining whether or not a text line
  // is a parent text line or a sibling text line of another text line. The threshold is computed
  // as <factor> * 'average character width in the PDF document'. If the leftX value of a text
  // line L is larger than the leftX value of another text line M, M is considered to be the
  // parent text line of L. If the difference between the leftX values is smaller than this
  // threshold, the text lines are considered to be sibling text lines.
  double textLineHierarchyLeftXOffsetThresholdFactor = 1.0;

  /**
   * This method returns a threshold to be used for detecting text lines. It denotes the maximum
   * vertical overlap ratio that two consecutive text lines must achieve so that the text lines
   * are merged. If the maximum vertical overlap ratio between two consecutive lines is larger or
   * equal to the returned threshold, the text lines are merged; otherwise the text lines are not
   * merged.
   *
   * @param doc
   *    The currently processed PDF document.
   * @param xGap
   *    The horizontal gap between the two text lines for which it is to be decided whether or not
   *    they should be merged.
   *
   * @return
   *    The threshold.
   */
  constexpr static double getYOverlapRatioThreshold(const PdfDocument* doc, double xGap) {
    return xGap < 3 * doc->avgCharWidth ? 0.4 : 0.8;
  }

  // A parameter in [0, 1] used for computing the trim box of a segment. It denotes the minimum
  // percentage of text lines in a given segment that must exhibit the most frequent rightX so
  // that this rightX is considered to be the rightX of the trim box of the segment.
  double minPrecLinesSameRightX = 0.5;

  // A parameter used for computing the trim box of a segment. It denotes the precision to use
  // when rounding the rightX values of the text lines of the segment before computing the most
  // frequent rightX value.
  double trimBoxCoordsPrec = 0;
};

// =================================================================================================

struct SubSuperScriptsDetectionConfig : BaseConfig {
  // A parameter specifying whether or not to disable the detection of sub/superscripts.
  bool disabled = false;

  // A parameter that denotes the maximum allowed difference between the baseline of a character
  // and the baseline of a text line, so that the character "sit" on the same baseline. If the
  // baseline of a character is larger than the baseline of the text line (under consideration of
  // the threshold), the character is considered to be a superscript. If the baseline is smaller,
  // the character is considered to be a subscript.
  double baseEqualTolerance = 0.1;

  // A parameter that denotes the maximum allowed difference between the font size of a character
  // and the most frequent font size in the document, so that the font sizes are considered to be
  // equal. If the font size of a character is smaller than the most frequent font size (under
  // consideration of the threshold), the character is considered to be a sub- or superscript.
  // Otherwise, it is not considered to be a sub-/superscript.
  double fsEqualTolerance = 0.9;
};

// =================================================================================================

struct TextBlocksDetectionConfig : BaseConfig {
  // A parameter specifying whether or not to disable the detection of text blocks.
  bool disabled = false;

  // ----------
  // startsBlock_lineDistance()

  /**
   * This method returns a threshold to be used for checking if the distance between two text
   * lines is larger than the given expected line distance. The line distance is only then
   * considered to be larger than the given expected line distance, when the difference between
   * the two distances is larger than the returned threshold.
   *
   * @param doc
   *    The PDF document currently processed.
   * @param expectedLineDistance
   *    The expected line distance.
   *
   * @return
   *    The threshold.
   */
  constexpr static double getExpectedLineDistanceThreshold(const PdfDocument* doc,
      double expectedLineDist) {
    return std::max(1.0, 0.1 * expectedLineDist);
  }

  // ----------
  // startsBlock_increasedLineDistance()

  /**
   * This method returns a threshold to be used for checking if the distance between the current
   * line and the next line (= "curr/next distance") is larger than the distance between the
   * current line and the previous line (= "curr/prev distance").
   * The curr/next distance is only then considered to be larger than the "curr/prev distance" if
   * the difference between the two distances is larger than the returned tolerance.
   *
   * @param doc
   *    The PDF document currently processed.
   *
   * @return
   *    The threshold.
   */
  constexpr static double getPrevCurrNextLineDistanceTolerance(const PdfDocument* doc) {
    return 0.5 * doc->mostFreqWordHeight;
  }

  // ----------
  // startsBlock_item()

  /**
   * This method returns an interval to be used for checking if the leftX-offset between a line
   * and its previous line falls into. If the offset falls into the returned interval, the line
   * and the previous line are considered to be part of the same block. Otherwise, the line is
   * considered to be the start of a new text block.
   *
   * @param doc
   *    The PDF document currently processed.
   *
   * @return
   *    A pair of doubles, with the first value denoting the start point of the interval and the
   *    second point denoting the end point of the interval.
   */
  constexpr static pair<double, double> getLeftXOffsetToleranceInterval(const PdfDocument* doc) {
    return make_pair(-1 * doc->avgCharWidth, 6 * doc->avgCharWidth);
  }

  // ----------
  // startsBlock_indent()

  /**
   * This method returns an interval to be used for checking if the left margin of a line falls
   * into. If the left margin of a line falls into the interval (and if the parent segment is not
   * in hanging indent format), the line is considered to be the indented first line of a text
   * block. Otherwise it is not considered to be the first line of a text block.
   *
   * @param doc
   *    The PDF document currently processed.
   *
   * @return
   *    A pair of doubles, with the first value denoting the start point of the interval and the
   *    second point denoting the end point of the interval.
   */
  constexpr static pair<double, double> getIndentToleranceInterval(const PdfDocument* doc) {
    return make_pair(1 * doc->avgCharWidth, 6 * doc->avgCharWidth);
  }

  // A set of common last name prefixes, e.g.: "van", "de", etc. It is used while computing
  // whether or not a text block is in hanging indent format. The motivation is the following:
  // Normally, all non-indented text lines of a text block must start with an uppercase character,
  // so that the text block is considered to be in hanging indent format. But there are
  // references that start with a last name prefix like "van" or "de", in which case the
  // respective text block contains non-indented text lines starting with a lowercase character.
  // This alphabet is for allowing such text lines in a hanging indent text block.
  unordered_set<string> lastNamePrefixes = { "van", "von", "de" };

  // -------
  // Config for computeIsCentered().

  // A parameter used for computing whether or not a text line is centered compared to another
  // text line. It denotes the minimum ratio by which one of the text line must horizontally
  // overlap the other text line, so that the text lines are considered to be centered to each
  // other. If the maximum x-overlap ratio between both text lines is smaller than this value,
  // the text lines are considered to be *not* centered.
  double centeringXOverlapRatioThreshold = 0.99;

  /**
   * This method returns a double value denoting the maximum allowed difference between the left
   * x-offset and right x-offset of a text line (computed relatively to the previous text line), so
   * that both offsets are considered to be equal and that the text line is considered to be
   * centered compared to the previous text line.
   *
   * @param line
   *    The text line to process.
   *
   * @return
   *    The threshold.
   */
  constexpr double getCenteringXOffsetEqualTolerance(const PdfTextLine* line) const {
    return line->doc ? 2.0 * line->doc->avgCharWidth : 0.0;
  }

  // ----------
  // Config for computeIsTextLinesCentered().

  // A parameter that is used for computing whether or not the text lines of a text block are
  // centered among each other. It denotes the maximum number of justified lines (= lines with a
  // left margin and right margin == 0) a text block is allowed to contain so that the text lines
  // are considered to be centered.
  int centeringMaxNumJustifiedLines = 5;

  /**
   * This method returns a threshold used for computing whether or not the text lines of a text
   * block are centered among each other. The text lines are not considered to be centered, when none
   * of the text lines has a leftX-offset and rightX-offset larger than this threshold.
   *
   * @param doc
   *    The currently processed PDF document.
   *
   * @return
   *    The threshold.
   */
  constexpr double getCenteringXOffsetThreshold(const PdfDocument* doc) {
    return 2.0 * doc->avgCharWidth;
  }

  // -------
  // Config for computeHasPrevLineCapacity().

  // A factor used to compute a threshold that is used for computing whether or not the previous
  // text line has capacity (the threshold is computed as <factor> * 'avg. character width of the
  // PDF document'). If the difference between the right margin of the previous line and the
  // width of the first word of the current text line is larger than this threshold, the previous
  // line is considered to have capacity. Otherwise, the previous line is considered to have *no*
  // capacity.
  double prevTextLineCapacityThresholdFactor = 2.0;

  // ----------
  // Config for computeHangingIndent().

  // A parameter that is used for computing whether or not a text block is in hanging indent
  // format. It denotes the min length of a text line so that the line is considered to be a
  // "long" text line.
  double hangIndentMinLengthLongLines = 3;

  // A parameter that is used for computing whether or not a text block is in hanging indent
  // format. If all non-indented lines of a text block start with an uppercase character and if
  // the number of non-indented lines is larger than this threshold, the block is considered to
  // be in hanging indent format.
  int hangIndentNumNonIndentedLinesThreshold = 10;

  // A parameter that is used for computing whether or not a text block is in hanging indent
  // format. If there is at least one indented line that starts with a lowercase character, and
  // the number of long lines is larger than this threshold, the text block is considered to be
  // in hanging indent format.
  int hangIndentNumLongLinesThreshold = 4;

  // A parameter in [0, 1] that is used for computing whether or not a text block is in hanging
  // indent format. It denotes the minimum percentage of *indented* lines in a given text block
  // that must exhibit the most frequent left margin > 0. If the percentage of such lines is
  // smaller than this threshold, the text block is considered to be *not* in hanging indent
  // format.
  double hangIndentMinPercLinesSameLeftMargin = 0.5;

  // A parameter that is used for computing whether or not a text block is in hanging indent
  // format. It denotes the maximum number of lowercased non-indented text lines a text block is
  // allowed to contain so that the text block is considered to be in hanging indent format.
  int hangIndentNumLowerNonIndentedLinesThreshold = 0;

  // A parameter that is used for computing whether or not a text block is in hanging indent
  // format. It denotes the minimum number of lowercased indented lines a text block is allowed
  // to contain so that the text block is considered to be in hanging indent format.
  int hangIndentNumLowerIndentedLinesThreshold = 1;

  // A factor used to compute a threshold for checking if the left margin of a text line is
  // "large enough" so that the text line is considered to be indented. If the left margin is
  // larger than this threshold, the text line is considered to be indented; otherwise it is
  // considered to be not indented.
  double hangIndentMarginThresholdFactor = 1.0;

  // A parameter used for computing whether or not an element is part of a figure. It denotes the
  // minimum percentage of the element's width which must be overlapped by a figure so that the
  // element is considered to be part of the figure.
  double figureXOverlapThreshold = 0.5;

  // A parameter used for computing whether or not an element is part of a figure. It denotes the
  // minimum percentage of the element's height which must be overlapped by a figure so that the
  // element is considered to be part of the figure.
  double figureYOverlapThreshold = 0.5;

  // -------
  // Config for computeIsPrefixedByItemLabel().

  // An alphabet of characters which we consider to be a valid part of a superscripted item label.
  const char* superItemLabelAlphabet = "*∗abcdefghijklmnopqrstuvwxyz01234567890()";

  // The regular expressions we use to detect enumeration item labels.
  vector<regex> itemLabelRegexes = {
    // A regex to find item labels of form "• ", or "- ", or "+ ", etc.
    regex("^(•|-|–|\\+)\\s+"),
    // A regex to find item labels of form "I. ", "II. ", "III. ", "IV. ", etc.
    regex("^(X{0,1}(IX|IV|V?I{0,3}))\\.\\s+", icase),
    // A regex to find item labels of form "(I)", "(II)", "(III)", "(IV) ", etc.
    regex("^\\((X{0,1}(IX|IV|V?I{0,3}))\\)\\s+", icase),
    // A regex to find item labels of form "a. ", "b. ", "c. ", etc.
    regex("^([a-z])\\.\\s+"),
    // A regex to find item labels of form "1. ", "2. ", "3. ", etc.
    regex("^([0-9]+)\\.\\s+"),
    // A regex to find item labels of form "(A) ", "(1) ", "(C1) ", "[1] ", "[2] ", etc.
    regex("^(\\(|\\[)([a-z0-9][0-9]{0,2})(\\)|\\])\\s+", icase),
    // A regex to find item labels of form "[Bu2] ", "[Ch] ", "[Enn2020] ", etc.
    regex("^(\\[)([A-Z][a-zA-Z0-9]{0,5})(\\])\\s+"),
    // A regex to find item labels of form "A) " or "1) " or "a1) ".
    regex("^([a-z0-9][0-9]{0,1})\\)\\s+", icase),
    // A regex to find item labels of form "PACS" (1011.5073).
    regex("^PACS\\s+", icase)
  };
};

// =================================================================================================

struct ReadingOrderDetectionConfig : BaseConfig {
  // A parameter specifying whether or not to disable the detection of the reading order.
  bool disabled = false;
};

// =================================================================================================

struct SemanticRolesPredictionConfig : BaseConfig {
  // A parameter specifying whether or not to disable the prediction of semantic roles.
  bool disabled = false;

  // The path to the directory containing the (serialized) learning model.
  string modelsDir = "/path/not/specified";
};

// =================================================================================================

struct WordsDehyphenationConfig : BaseConfig {
  // A parameter specifying whether or not to disable the dehyphenation of words.
  bool disabled = false;
};

// =================================================================================================

struct PdfDocumentVisualizationConfig : BaseConfig {
  // The appearance of a semantic role in a visualization.
  // "/Helv" is the font name (= Helvetica), "7" is the font size, "0 0 1" is the color (= blue).
  string semanticRoleAppearance = "/Helv 7 Tf 0 0 1 rg";

  // The width of a line that connects consecutive text blocks (wrt. the reading order).
  double readingOrderLineWidth = 4.0;

  // The radius of a circle containing a reading order index.
  double readingOrderCircleRadius = 5;

  // The appearance of a reading order index (= the number in a reading order circle).
  string readingOrderIndexAppearance = "/Helv 7 Tf 1 1 1 rg";

  // The width of a line that represents an XY-cut.
  double cutWidth = 2.0;

  // The font appearance of a cut index.
  string cutIndexAppearance = "/Helv 7 Tf 1 1 1 rg";

  // The radius of a square containing a cut index.
  double cutSquareRadius = 5;

  // The font appearance of a cut id.
  string cutIdAppearance = "/Helv 6 Tf .7 .7 .7 rg";
};

// =================================================================================================

struct Config {
  PdfParsingConfig pdfParsing;
  GlyphsStatisticsCalculationConfig glyphsStatisticsCalculation;
  DiacriticalMarksMergingConfig diacriticalMarksMerging;
  WordsDetectionConfig wordsDetection;
  WordsStatisticsCalculationConfig wordsStatisticsCalculation;
  PageSegmentationConfig pageSegmentation;
  TextLinesDetectionConfig textLinesDetection;
  TextLinesStatisticsCalculationConfig textLinesStatisticsCalculation;
  SubSuperScriptsDetectionConfig subSuperScriptsDetection;
  TextBlocksDetectionConfig textBlocksDetection;
  ReadingOrderDetectionConfig readingOrderDetection;
  SemanticRolesPredictionConfig semanticRolesPrediction;
  WordsDehyphenationConfig wordsDehyphenation;
  PdfDocumentVisualizationConfig pdfDocumentVisualization;
};

}  // namespace ppp::config

#endif  // CONFIG_H_
