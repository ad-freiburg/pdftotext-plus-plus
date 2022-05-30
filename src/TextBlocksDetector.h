/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TEXTBLOCKSDETECTOR_H_
#define TEXTBLOCKSDETECTOR_H_

#include <vector>

#include "./utils/Utils.h"
#include "./utils/LogUtils.h"
#include "./PdfDocument.h"

using namespace std;

// =================================================================================================

/**
 * This class is responsible for detecting text blocks in a PDF document.
 *
 * It expects that the pages of the PDF document to be processed were already segmented and that
 * the text lines of each segment were already detected. The PDF document is processed page-wise.
 * Each page is processed using the following two-step approach.
 *
 * In the first step, the text lines of each segment contained in the page are split into
 * preliminary text blocks, that is: groups of (consecutive) lines that obviously form a text block
 * (e.g., because of a special font size, or a special rotation, or an unusual distance between
 * the text lines) but potentially need to be split further into smaller text blocks.
 *
 * In the second step, the preliminary text blocks are split further into smaller text blocks by
 * analyzing the contained text lines for more advanced layout features, for example: the
 * indentations of a text line (= the difference between the left x-coordinate of the text line
 * and the left x-coordinate of the preliminary text block), the alignment of the text lines (a PDF
 * can contain text blocks in different alignments, for example: body text paragraphs are often
 * justified, while author information or formulas are often centered), or the emphasis of text
 * lines.
 *
 * This two-step approach was chosen, because it particularly enables a more accurate computation
 * of text line indentations. Initially, we computed the text line indentations by computing the
 * difference between the left x-coordinate of the text line and the left x-coordinate of the
 * segment (instead of the preliminary text block). This often resulted in inaccurately computed
 * indentations, since the segments were often broader than expected, mostly because it
 * contained text parts of different alignments and different indentations. For example, page
 * headers/footers and body text paragraphs, which often have different alignments and
 * indentations, were often grouped into the same segment. A frequent consequence was that the text
 * lines of the body text paragraphs were not justified with the segment boundaries, but were
 * positioned somewhere in the middle of the segments instead (which obviously resulted in wrongly
 * computed line indentations).
 */
class TextBlocksDetector {
 public:
  /**
   * This constructor creates and initializes a new instance of the `TextBlocksDetector` class.
   *
   * @param doc
   *    The PDF document to process. It is expected that the pages of the document were already
   *    segmented and that the text lines of each segment were already detected. The segments of
   *    page <i> are expected to be stored in doc.pages[i].segments. The text lines of segment
   *    <j> are expected to be stored in doc.pages[i].segments[j].lines.
   * @param debug
   *    A boolean flag indicating whether or not this class should output debug information to the
   *    console while detecting the text blocks.
   * @param debugPageFilter
   *    A filter that can be used to output only the debug information that is produced while
   *    detecting the text blocks on page <debugPageFilter>. If specified by a value < 0, *all*
   *    debug information produced while detecting the text blocks is outputted, no matter on which
   *    pages the text blocks are located.
   */
  TextBlocksDetector(PdfDocument* doc, bool debug=false, int debugPageFilter=-1);

  /**
   * The deconstructor.
   */
  ~TextBlocksDetector();

  /**
   * This method starts the text block detection. As mentioned in the preliminary comment above,
   * the document is processed page-wise. For each page, the text blocks are detected in two steps:
   * (1) detecting preliminary text blocks, and (2) splitting the preliminary text blocks into
   * smaller (and more accurate) text blocks.
   *
   * For detecting the preliminary text blocks, the text lines are processed segment-wise. For each
   * segment S in page P the text lines stored in S.lines are iterated, while storing the text
   * lines of the "active" text block in a vector V. For each text line L, it is decided whether or
   * not L starts a new text block (this decision is made by the startsPreliminaryBlock() method
   * below). If L doesn't start a new text block, it is added to V. If it starts a new text block,
   * a new `PdfTextBlock` instance consisting of the text lines in V is created and added to
   * S.blocks. Afterwards, V is cleared, L is added to V and the method proceeds with the next
   * line L'.
   *
   * For splitting the preliminary text blocks into smaller text blocks, the approach is
   * principally the same, with three exceptions: (1) instead of segment-wise, the text lines are
   * processed "preliminary text block"-wise; (2) instead of startsPreliminaryBlock(), the
   * startsBlock() method is called for deciding whether or not L starts a new text block; (3)
   * instead to S.blocks, the created `PdfTextBlock` instances are added to P.blocks.
   */
  void detect();

 private:
  /**
   * This method returns true, if the given text line starts a new preliminary text block, false
   * otherwise. This decision is made based on analyzing different layout information, for example:
   * the distance between the line and the previous line, the font size, or the rotation.
   *
   * @param line
   *    The text line to process
   *
   * @return True, if the given text lines starts a new preliminary text block, false otherwise.
   */
  bool startsPreliminaryBlock(const PdfTextLine* line) const;

  /**
   * This method returns true, if the given text line starts a new (non-preliminary) text block,
   * false otherwise. This decision is made based on analyzing different advanced layout
   * information, for example: the centering of the preliminary text block of which the line is a
   * part of, or the indentation of the text line (compared to the boundary of the preliminary
   * text block).
   *
   * @param pBlock
   *    The preliminary text block of which the given text line is a part of.
   * @param line
   *    The text line to process
   *
   * @return True, if the given text lines starts a new text block, false otherwise.
   */
  bool startsBlock(const PdfTextBlock* pBlock, const PdfTextLine* line);

  // ===============================================================================================

  /**
   * This method tries to decide whether or not the given line starts a new text block, by
   * analyzing if the given text line has a previous line.
   *
   * @param line
   *    The text line to process.
   *
   * @return
   *    Trool::True if the line starts a new text block;
   *    Trool::False if the line does not start a new text block;
   *    Trool::None if it is unknown whether or not the text line starts a new text block.
   */
  Trool startsBlock_existsPrevLine(const PdfTextLine* line) const;

  /**
   * This method tries to decide whether or not the given line starts a new text block, by
   * analyzing if the given text line and its previous line overlaps the same figure.
   *
   * @param line
   *    The text line to process.
   *
   * @return
   *    Trool::True if the line starts a new text block;
   *    Trool::False if the line does not start a new text block;
   *    Trool::None if it is unknown whether or not the text line starts a new text block.
   */
  Trool startsBlock_sameFigure(const PdfTextLine* line) const;

  /**
   * This method tries to decide whether or not the given line starts a new text block, by
   * analyzing the rotation of the given text line and its previous line.
   *
   * @param line
   *    The text line to process.
   *
   * @return
   *    Trool::True if the line starts a new text block;
   *    Trool::False if the line does not start a new text block;
   *    Trool::None if it is unknown whether or not the text line starts a new text block.
   */
  Trool startsBlock_rotation(const PdfTextLine* line) const;

  /**
   * This method tries to decide whether or not the given line starts a new text block, by
   * analyzing the writing modes of the given text line and its previous line.
   *
   * @param line
   *    The text line to process.
   *
   * @return
   *    Trool::True if the line starts a new text block;
   *    Trool::False if the line does not start a new text block;
   *    Trool::None if it is unknown whether or not the text line starts a new text block.
   */
  Trool startsBlock_wMode(const PdfTextLine* line) const;

  /**
   * This method tries to decide whether or not the given line starts a new text block, by
   * analyzing the font sizes of the given text line and its previous line.
   *
   * @param line
   *    The text line to process.
   * @param maxDelta
   *    The value by which the font sizes of the given text lines and its previous line may differ
   *    in order to be considered equal.
   *
   * @return
   *    Trool::True if the line starts a new text block;
   *    Trool::False if the line does not start a new text block;
   *    Trool::None if it is unknown whether or not the text line starts a new text block.
   */
  Trool startsBlock_fontSize(const PdfTextLine* line, double maxDelta=1.0) const;

  /**
   * This method tries to decide whether or not the given line starts a new text block, by
   * comparing the actual line distance between the given text line and its previous line with the
   * expected line distance (computed in a previous step and stored in _doc).
   *
   * @param line
   *    The text line to process.
   * @param minTolerance
   *    The minimum tolerance used on comparing the actual line distance with the expected line
   *    distance. See the comment given for the <toleranceFactor> parameter for more details.
   * @param toleranceFactor
   *    A factor used to compute a tolerance for comparing the actual line distance with the
   *    expected line distance. This tolerance is computed as follows:
   *    tolerance = max(minTolerance, toleranceFactor * expectedLineDistance).
   *    For the given text line to be considered as the start of a new text block, the difference
   *    between the actual line distance and the expected line distance must be larger than this
   *    threshold.
   *
   * @return
   *    Trool::True if the line starts a new text block;
   *    Trool::False if the line does not start a new text block;
   *    Trool::None if it is unknown whether or not the text line starts a new text block.
   */
  Trool startsBlock_lineDistance(const PdfTextLine* line, double minTolerance=1.0,
      double toleranceFactor=0.1) const;

  /**
   * This method tries to decide whether or not the given line starts a new text block, by
   * comparing the line distance between the given text line and its previous line (= curr+prev
   * distance) with the line distance between the previous line and the previous but one line
   * (prev+prevPrev distance).
   *
   * @param line
   *    The text line to process.
   * @param toleranceFactor
   *    A factor used to compute a tolerance for comparing the curr+prev distance and the
   *    prev+prevPrev distance. This tolerance is computed as follows:
   *    tolerance = toleranceFactor * _doc.mostFreqWordHeight.
   *    For the given text line to be considered as the start of a new text block, the difference
   *    between the curr+prev distance and the prev+prevPrev distance must be larger than this
   *    threshold.
   *
   * @return
   *    Trool::True if the line starts a new text block;
   *    Trool::False if the line does not start a new text block;
   *    Trool::None if it is unknown whether or not the text line starts a new text block.
   */
  Trool startsBlock_increasedLineDistance(const PdfTextLine* line, double toleranceFactor=0.5) const;

  /**
   * This method tries to decide whether or not the given line starts a new text block, by
   * analyzing if the given preliminary block and line is centered.
   *
   * @param pBlock
   *    The preliminary text block to process.
   * @param line
   *    The text line to process.
   *
   * @return
   *    Trool::True if the line starts a new text block;
   *    Trool::False if the line does not start a new text block;
   *    Trool::None if it is unknown whether or not the text line starts a new text block.
   */
  Trool startsBlock_centered(const PdfTextBlock* pBlock, const PdfTextLine* line) const;

  /**
   * This method tries to decide whether or not the given line starts a new text block, by
   * analyzing if the given line (or its previous line) is part of an enumeration item.
   *
   * @param pBlock
   *    The preliminary text block to process.
   * @param line
   *    The text line to process.
   * @param lowerXOffsetToleranceFactor
   *    A factor used to compute a minimum value by which the left x-coordinates of the previous
   *    line and the given text line may differ so that both lines are considered to be part of the
   *    same text block. This value is computed as follows: lowerXOffsetToleranceFactor *
   *    _doc.avgGlyphWidth. If the difference is smaller than this value, the given text line is
   *    considered to be the start of a new text block.
   * @param upperXOffsetToleranceFactor
   *    A factor used to compute a maximum value by which the left x-coordinates of the previous
   *    line and the given text line may differ so that both lines are considered to be part of the
   *    same text block. This value is computed as follows: upperXOffsetToleranceFactor *
   *    _doc.avgGlyphWidth. If the difference is larger than this value, the given text line is
   *    considered to be the start of a new text block.
   * @param rightMarginToleranceFactor
   *    TODO
   *
   * @return
   *    Trool::True if the line starts a new text block;
   *    Trool::False if the line does not start a new text block;
   *    Trool::None if it is unknown whether or not the text line starts a new text block.
   */
  Trool startsBlock_item(const PdfTextBlock* pBlock, const PdfTextLine* line,
      double lowerXOffsetToleranceFactor=-1, double upperXOffsetToleranceFactor=6,
      double rightMarginToleranceFactor=3) const;

  /**
   * This method tries to decide whether or not the given line starts a new text block, by
   * analyzing if the given line (and its previous line) is emphasized compared to the most
   * frequent font and font style in the document.
   *
   * @param line
   *    The text line to process.
   *
   * @return
   *    Trool::True if the line starts a new text block;
   *    Trool::False if the line does not start a new text block;
   *    Trool::None if it is unknown whether or not the text line starts a new text block.
   */
  Trool startsBlock_emphasized(const PdfTextLine* line) const;

  /**
   * This method tries to decide whether or not the given line starts a new text block, by
   * analyzing if the given preliminary text block is in hanging indent format and if the given
   * text line is indented or not.
   *
   * @param pBlock
   *    The preliminary text block to process.
   * @param line
   *    The text line to process.
   * @param lowerXOffsetToleranceFactor
   *    A factor used to compute a minimum value by which the left x-coordinates of the previous
   *    line and the given text line may differ so that both lines are considered to be part of the
   *    same text block. This value is computed as follows: lowerXOffsetToleranceFactor *
   *    _doc.avgGlyphWidth. If the difference is smaller than this value, the given text line is
   *    considered to be the start of a new text block.
   * @param upperXOffsetToleranceFactor
   *    A factor used to compute a maximum value by which the left x-coordinates of the previous
   *    line and the given text line may differ so that both lines are considered to be part of the
   *    same text block. This value is computed as follows: upperXOffsetToleranceFactor *
   *    _doc.avgGlyphWidth. If the difference is larger than this value, the given text line is
   *    considered to be the start of a new text block.
   *
   * @return
   *    Trool::True if the line starts a new text block;
   *    Trool::False if the line does not start a new text block;
   *    Trool::None if it is unknown whether or not the text line starts a new text block.
   */
  Trool startsBlock_hangingIndent(const PdfTextBlock* pBlock, const PdfTextLine* line,
      double lowerXOffsetToleranceFactor=-1, double upperXOffsetToleranceFactor=3) const;


  /**
   * This method tries to decide whether or not the given line starts a new text block, by
   * analyzing if the given preliminary text block is not in hanging indent format and if the given
   * text line is indented or not.
   *
   * @param pBlock
   *    The preliminary text block to process.
   * @param line
   *    The text line to process.
   * @param lowerLeftMarginToleranceFactor
   *    A factor used to compute a minimum value by which the left margin of the previous
   *    line and of the given text line may differ so that one line is considered to be indented
   *    compared to the other text line. This value is computed as follows:
   *    lowerLeftMarginToleranceFactor * _doc.avgGlyphWidth. If the difference is smaller than this
   *    value, the lines are not considered to be indented to each other.
   * @param upperLeftMarginToleranceFactor
   *    A factor used to compute a maximum value by which the left margin of the previous
   *    line and of the given text line may differ so that one line is considered to be indented
   *    compared to the other text line. This value is computed as follows:
   *    upperLeftMarginToleranceFactor * _doc.avgGlyphWidth. If the difference is larger than this
   *    value, the lines are not considered to be more indented to each other.
   * @param rightXToleranceFactor
   *    TODO
   *
   * @return
   *    Trool::True if the line starts a new text block;
   *    Trool::False if the line does not start a new text block;
   *    Trool::None if it is unknown whether or not the text line starts a new text block.
   */
  Trool startsBlock_indent(const PdfTextLine* line, double lowerLeftMarginToleranceFactor=1,
      double upperLeftMarginToleranceFactor=6, double rightXToleranceFactor=5) const;

  // ===============================================================================================

  /**
   * The PDF documen to process.
   */
  PdfDocument* _doc;

  /**
   * The potential footnote labels, that is: superscripted strings consisting of alphanumerical
   * characters.
   */
  unordered_set<string> _potentialFnLabels;

  /**
   * The logger for outputting the debug information.
   */
  Logger* _log;
};

#endif  // TEXTBLOCKDETECTOR_H_
