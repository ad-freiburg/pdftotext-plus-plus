/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TEXTBLOCKSDETECTOR_H_
#define TEXTBLOCKSDETECTOR_H_

#include <algorithm>  // std::max
#include <string>
#include <unordered_set>
#include <utility>  // std::make_pair

#include "./utils/Log.h"
#include "./utils/Trool.h"

#include "./PdfDocument.h"

using std::string;
using std::unordered_set;

// =================================================================================================

struct TextBlocksDetectorConfig;

/**
 * This class is responsible for detecting text blocks in a PDF document.
 *
 * It is expected that the pages of the PDF document to be processed were already segmented and
 * that the text lines of each segment were already detected. The PDF document is processed
 * page-wise. Each page is processed using the following two-step approach.
 *
 * In the first step, the text lines of each segment of the page are split into preliminary text
 * blocks. The purpose is to seperate text that exhibits some special layout feature (for
 * example: a special rotation, a special writing mode, a special font size, or a special line
 * distance by which the lines are separated) from the rest of the text (by "special" we mean that
 * the majority of text in the document does not exhibit the same layout feature). Preliminary text
 * blocks can be seen as a division into rough text blocks, which however need to be split further
 * into smaller (and more accurate) text blocks.
 *
 * In the second step, the preliminary text blocks are split further into smaller text blocks by
 * analyzing the contained text lines for more advanced layout features, for example: the
 * indentations of a text line (= the difference between the leftX-coordinate of the text line
 * and the leftX-coordinate of the preliminary text block), the alignment of the text lines (a PDF
 * can contain text blocks in different alignments, for example: body text paragraphs are often
 * justified, while author information or formulas are often centered).
 *
 * This two-step approach was chosen, because it particularly enables a more accurate computation
 * of text line indentations. Initially, we computed the text line indentations by computing the
 * difference between the leftX-coordinate of the text line and the leftX-coordinate of the
 * segment (instead of the preliminary text block). This often resulted in inaccurately computed
 * indentations, since the segments were often broader than expected, mostly because it
 * contained text parts with alignments and indentations different to those of the majority of
 * text in the segments. For example, page headers/footers and body text paragraphs, which often
 * have different alignments and indentations, were often grouped into the same segment. A frequent
 * consequence was that the text lines of the body text paragraphs were not justified with the
 * segment boundaries, but were positioned somewhere in the middle of the segments instead (which
 * obviously resulted in inaccurate line indentations).
 */
class TextBlocksDetector {
 public:
  /**
   * This constructor creates and initializes a new instance of this class.
   *
   * @param doc
   *    The PDF document to process. It is expected that the pages of the document were already
   *    segmented and that the text lines of each segment were already detected. The segments of
   *    the i-th page are expected to be stored in doc.pages[i].segments. The text lines of the
   *    j-th segment of the i-th page are expected to be stored in doc.pages[i].segments[j].lines.
   * @param debug
   *   Whether or not this instance should print debug information to the console.
   * @param debugPageFilter
   *   If set to a value > 0, only the debug messages produced while processing the
   *   <debugPageFilter>-th page of the current PDF file will be printed to the console.
   */
  explicit TextBlocksDetector(PdfDocument* doc, bool debug = false, int debugPageFilter = -1);

  /** The deconstructor. */
  ~TextBlocksDetector();

  /**
   * This method starts the detection of text blocks. As mentioned in the preliminary comment of
   * this class, the document is processed page-wise. For each page, the text blocks are detected
   * in two steps: (1) detecting preliminary text blocks, and (2) splitting the preliminary text
   * blocks into smaller text blocks.
   *
   * For detecting the preliminary text blocks, the text lines are processed segment-wise. For each
   * segment S in page P the text lines stored in S.lines are iterated. There is a vector V which
   * stores the text lines of the "active" text block. For each text line L, it is decided whether
   * or not L starts a text block (this decision is made by the startsPreliminaryBlock() method
   * below). If L doesn't start a text block, it is added to V. If it starts a text block, a new
   * `PdfTextBlock` instance (consisting of the text lines in V) is created and added to S.blocks.
   * Afterwards, V is cleared, L is added to V and the next line L' is processed.
   *
   * For splitting the preliminary text blocks into smaller text blocks, the procedure is
   * principally the same, with three exceptions: (1) instead of segment-wise, the text lines are
   * processed "preliminary text block"-wise; (2) instead of startsPreliminaryBlock(), the
   * startsBlock() method is called for deciding whether or not L starts a text block; (3) instead
   * of to S.blocks, the created `PdfTextBlock` instances are added to P.blocks.
   */
  void process();

 private:
  /**
   * This method returns true, if the given text line starts a preliminary text block, false
   * otherwise. This decision is made based on analyzing different layout information, for example:
   * the distance between the line and the previous line, the font size, or the rotation.
   *
   * @param line
   *    The text line to process.
   *
   * @return
   *    True, if the given text line starts a preliminary text block, false otherwise.
   */
  bool startsPreliminaryBlock(const PdfTextLine* line) const;

  /**
   * This method returns true, if the given text line starts a (non-preliminary) text block,
   * false otherwise. This decision is made based on analyzing different advanced layout
   * information, for example: the alignment of the preliminary text block of which the line is a
   * part of, or the indentation of the text line (compared to the boundary of the preliminary
   * text block).
   *
   * @param pBlock
   *    The preliminary text block of which the given text line is a part of.
   * @param line
   *    The text line to process.
   *
   * @return
   *    True, if the given text line starts a text block, false otherwise.
   */
  bool startsBlock(const PdfTextBlock* pBlock, const PdfTextLine* line) const;

  // ===============================================================================================

  /**
   * This method checks whether the given line starts a block because it has no previous line.
   *
   * @param line
   *    The line to process.
   *
   * @return
   *    Trool::True if the line starts a block;
   *    Trool::False if the line does *not* start a block;
   *    Trool::None if this method couldn't decide whether the line starts a block.
   */
  Trool startsBlock_existsPrevLine(const PdfTextLine* line) const;

  /**
   * This method checks whether the given line does *not* start a block because it overlaps the
   * same figure as its previous line. The motivation behind is that text in figures usually
   * don't follow the layout of normal text (e.g., the text can be distributed all over the figure
   * in arbitrary order and different alignments). Thus, detecting text blocks in figures doesn't
   * make sense and thus, we want to detect all text contained in a figure as a single block.
   *
   * @param line
   *    The line to process.
   *
   * @return
   *    Trool::True if the line starts a block;
   *    Trool::False if the line does *not* start a block;
   *    Trool::None if this method couldn't decide whether the line starts a block.
   */
  Trool startsBlock_sameFigure(const PdfTextLine* line) const;

  /**
   * This method checks whether the given line starts a block because it has another rotation
   * than its previous line.
   *
   * @param line
   *    The line to process.
   *
   * @return
   *    Trool::True if the line starts a block;
   *    Trool::False if the line does *not* start a block;
   *    Trool::None if this method couldn't decide whether the line starts a block.
   */
  Trool startsBlock_rotation(const PdfTextLine* line) const;

  /**
   * This method checks whether the given line starts a block because it has another writing mode
   * than its previous line.
   *
   * @param line
   *    The line to process.
   *
   * @return
   *    Trool::True if the line starts a block;
   *    Trool::False if the line does *not* start a block;
   *    Trool::None if this method couldn't decide whether the line starts a block.
   */
  Trool startsBlock_wMode(const PdfTextLine* line) const;

  /**
   * This method checks whether the given line starts a block because it has another font size
   * than its previous line. This rule exists to split e.g., headings (which usually have a larger
   * font size) from the body text.
   *
   * @param line
   *    The line to process.
   *
   * @return
   *    Trool::True if the line starts a block;
   *    Trool::False if the line does *not* start a block;
   *    Trool::None if this method couldn't decide whether the line starts a block.
   */
  Trool startsBlock_fontSize(const PdfTextLine* line) const;

  /**
   * This method checks whether the given line starts a block because its distance to the previous
   * line is larger than the most frequent line distance in the document (which was computed in a
   * previous step and is stored in _doc->mostFreqLineDistance).
   *
   * @param line
   *    The line to process.
   *
   * @return
   *    Trool::True if the line starts a block;
   *    Trool::False if the line does *not* start a block;
   *    Trool::None if this method couldn't decide whether the line starts a block.
   */
  Trool startsBlock_lineDistance(const PdfTextLine* line) const;

  /**
   * This method checks whether the given line starts a block because the line distance between
   * the given line and its previous line (= "curr/prev distance") is larger than the line distance
   * between the previous line and the previous but one line (= "prev/prevPrev distance").
   *
   * @param line
   *    The line to process.
   *
   * @return
   *    Trool::True if the line starts a block;
   *    Trool::False if the line does *not* start a block;
   *    Trool::None if this method couldn't decide whether the line starts a block.
   */
  Trool startsBlock_increasedLineDistance(const PdfTextLine* line) const;

  /**
   * This method checks whether the given line starts a block because the given preliminary
   * block is centered and the given line is the first line of an enumeration item. The motivation
   * behind is that affiliation information are often arranged in centered blocks, with the first
   * lines of blocks belonging to different affiliations prefixed by an enumeration label. We want
   * to detect the blocks belonging to different affiliations as separate blocks.
   *
   * @param pBlock
   *    The preliminary block of which the given line is a part of.
   * @param line
   *    The line to process.
   *
   * @return
   *    Trool::True if the line starts a block;
   *    Trool::False if the line does *not* start a block;
   *    Trool::None if this method couldn't decide whether the line starts a block.
   */
  Trool startsBlock_centered(const PdfTextBlock* pBlock, const PdfTextLine* line) const;

  /**
   * This method checks whether the given line starts a block because the line (or its previous
   * line) is part of an enumeration item.
   *
   * NOTE: Most typically, an enumeration item consists of only a single block and we want to
   * detect each item as a separate text block. But there are cases where an item consists of
   * multiple blocks, e.g., when it contains a formula. Here is an example:
   *
   * - This is the first item. It contains
   *   the following display formula:
   *                x + y = z
   *   We want to detect this formula as a
   *   separate block.
   * - This is another item.
   *
   * The first item consists of three blocks: the block starting with "This...", the formula, and
   * the block starting with "We...". To correctly detect the blocks, this method analyzes the
   * leftX-offsets between the given line and its previous line. If the offset falls into a defined
   * interval (definable via the <leftXOffsetToleranceFactorLow> and
   * <leftXOffsetToleranceFactorHigh> parameters, see below), the lines are considered to be part
   * of the same block. Otherwise, the given line is considered to be the start of a block.
   *
   * @param pBlock
   *    The preliminary block to process.
   * @param line
   *    The line to process.
   *
   * @return
   *    Trool::True if the line starts a block;
   *    Trool::False if the line does *not* start a block;
   *    Trool::None if this method couldn't decide whether the line starts a block.
   */
  Trool startsBlock_item(const PdfTextBlock* pBlock, const PdfTextLine* line) const;

  /**
   * This method checks whether the given line does *not* introduce a block because the line and
   * the previous line are emphasized compared to the body text and both lines exhibit the same
   * font and same font size. The motivation behind is to not split the lines belonging to the same
   * title or heading (which are often emphasized compared to the body text) apart in
   * startsBlock_indent().
   * NOTE: Titles and headings are often centered and thus, exhibit different leftX values. Without
   * this method, titles and heading would be split apart by startsBlock_indent().
   *
   * @param line
   *    The line to process.
   *
   * @return
   *    Trool::True if the line starts a block;
   *    Trool::False if the line does *not* start a block;
   *    Trool::None if this method couldn't decide whether the line starts a block.
   */
  Trool startsBlock_emphasized(const PdfTextLine* line) const;

  /**
   * This method checks whether the given line starts a block because the given preliminary block
   * is in hanging indent format and the given line and/or previous line is indented (or not).
   *
   * @param pBlock
   *    The preliminary block to process.
   * @param line
   *    The line to process.
   *
   * @return
   *    Trool::True if the line starts a block;
   *    Trool::False if the line does *not* start a block;
   *    Trool::None if this method couldn't decide whether the line starts a block.
   */
  Trool startsBlock_hangingIndent(const PdfTextBlock* pBlock, const PdfTextLine* line) const;

  /**
   * This method checks whether the given line starts a block because the given block
   * is not in hanging indent format and the given line and/or previous line is indented (or not).
   *
   * @param pBlock
   *    The preliminary block to process.
   * @param line
   *    The line to process.
   *
   * @return
   *    Trool::True if the line starts a text block;
   *    Trool::False if the line does not start a text block;
   *    Trool::None if it is unknown whether or not the text line starts a text block.
   */
  Trool startsBlock_indent(const PdfTextLine* line) const;

  // The PDF document to process.
  PdfDocument* _doc;

  // The config.
  TextBlocksDetectorConfig* _config;

  // The potential footnote labels, that is: superscripted strings consisting of alphanumerical
  // characters.
  unordered_set<string> _potentFnLabels;

  // The logger.
  Logger* _log;
};

// =================================================================================================

/**
 * This struct provides the configuration (= thresholds and parameters) to be used by the
 * `TextBlocksDetector` class while detecting text blocks.
 */
struct TextBlocksDetectorConfig {
  /**
   * This method returns the value by which two font sizes may differ in order to be considered
   * equal.
   *
   * NOTE: This method is used by the TextBlocksDetector::startsBlock_fontSize() method.
   *
   * @param doc
   *    The PDF document currently processed.
   *
   * @return
   *    The value by which two font sizes may differ in order to be considered equal.
   */
  double getFontSizeEqualTolerance(const PdfDocument* doc) const {
    return 1.0;
  }

  /**
   * This method returns a tolerance to be used on checking if the distance between two text lines
   * is larger than the given expected line distance. The line distance is only then considered to
   * be larger than the given expected line distance when the difference between the two distances
   * is larger than the returned tolerance.
   *
   * NOTE: This method is used by TextBlocksDetector::startsBlock_lineDistance().
   *
   * @param doc
   *    The PDF document currently processed.
   * @param expectedLineDistance
   *    The expected line distance.
   *
   * @return
   *    The tolerance to be used on checking if the distance between two text lines is larger than
   *    the given expected line distance.
   */
  double getExpectedLineDistanceTolerance(const PdfDocument* doc, double expectedLineDist) const {
    return std::max(1.0, 0.1 * expectedLineDist);
  }

  /**
   * This method returns a tolerance to be used on checking if the distance between the current
   * line and the next line (= "curr/next distance") is larger than the distance between the
   * current line and previous line (= "curr/prev distance").
   * The curr/next distance is only then considered to be larger than the "curr/prev distance" if
   * the difference between the two distances is larger than the returned tolerance.
   *
   * NOTE: This method is used by TextBlocksDetector::startsBlock_increasedLineDistance().
   *
   * @param doc
   *    The PDF document currently processed.
   * @param expectedLineDistance
   *    The expected line distance.
   *
   * @return
   *    The tolerance to be used on checking if the "curr/next distance" is larger than the
   *    "curr/prev distance".
   */
  double getPrevCurrNextLineDistanceTolerance(const PdfDocument* doc) const {
    return 0.5 * doc->mostFreqWordHeight;
  }

  /**
   * This method returns an interval into which the "leftX offset" of a line and its previous line
   * must fall so that both lines are considered to be part of the same block. The "leftX offset"
   * of a line and its previous line is computed as:
   *    line.prevLine.leftX - line.leftX
   * If the offset falls into the returned interval, the line and the previous line are considered
   * to be part of the same block. Otherwise, the line is considered to be the start of a new block.
   *
   * NOTE: This method is used by TextBlocksDetector::startsBlock_item() and
   * TextBlocksDetector::startsBlock_hangingIndent().
   *
   * @param doc
   *    The PDF document currently processed.
   *
   * @return
   *    A pair of doubles, with the first value denoting the start point of the interval and the
   *    second point denoting the end point of the interval.
   */
  std::pair<double, double> getLeftXOffsetToleranceInterval(const PdfDocument* doc) const {
    return std::make_pair(-1 * doc->avgCharWidth, 6 * doc->avgCharWidth);
  }

  /**
   * This method returns an interval into which the left margin of a line must fall so that the
   * line is considered to be the start of a new text block because it denotes the indented first
   * line of a text block.
   * If the left margin of a line falls into the returned interval (and if the parent segment is
   * not in hanging indent format), the line is considered to be the indented first line of a text
   * block. Otherwise it is not considered to be the first line of a text block.
   *
   * NOTE: This method is used by TextBlocksDetector::startsBlock_indent().
   *
   * @param doc
   *    The PDF document currently processed.
   *
   * @return
   *    A pair of doubles, with the first value denoting the start point of the interval and the
   *    second point denoting the end point of the interval.
   */
  std::pair<double, double> getIndentToleranceInterval(const PdfDocument* doc) const {
    return std::make_pair(1 * doc->avgCharWidth, 6 * doc->avgCharWidth);
  }
};

#endif  // TEXTBLOCKSDETECTOR_H_
