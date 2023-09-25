/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_TEXTBLOCKSDETECTIONUTILS_H_
#define UTILS_TEXTBLOCKSDETECTIONUTILS_H_

#include <string>
#include <unordered_set>
#include <vector>

#include "../Config.h"
#include "../Types.h"

using std::string;
using std::unordered_set;
using std::vector;

using ppp::config::TextBlocksDetectionConfig;
using ppp::types::PdfElement;
using ppp::types::PdfFigure;
using ppp::types::PdfTextBlock;
using ppp::types::PdfTextElement;
using ppp::types::PdfTextLine;

// =================================================================================================

namespace ppp::utils {

/**
 * A collection of some useful and commonly used functions in context of text blocks.
 */
class TextBlocksDetectionUtils {
 public:
  /**
   * The default constructor.
   *
   * @param config
   *   The configuration to use.
   */
  explicit TextBlocksDetectionUtils(const TextBlocksDetectionConfig* config);

  /** The deconstructor. */
  ~TextBlocksDetectionUtils();

  /**
   * This method returns true if the two given lines are centered compared to each other.
   *
   * For the returned value to be true, all of the following requirements must be fulfilled:
   * (1) One of the lines must completely overlap the respective other line horizontally, that is:
   *     one of the values returned by computeXOverlapRatios(line.prevLine, line) must be == 1.
   * (2) The leftX offset (= line1.leftX - line2.leftX) and the rightX offset (= line1.rightX -
   *     line2.rightX) must be equal, under consideration of a (small) tolerance.
   *
   * @param line1
   *   The first line to process.
   * @param line2
   *   The second line to process.
   *
   * @return
   *    True, if the two given lines are centered with respect to the requirements mentioned above,
   *    false otherwise.
   */
  bool computeIsCentered(const PdfTextLine* line1, const PdfTextLine* line2) const;

  /**
   * This method returns true if the lines in the given text block are centered compared to each
   * other; false otherwise.
   *
   * For the returned value to be true, all of the following requirements must be fulfilled:
   * (1) Each line in the block is centered compared to the respective previous line.
   *     NOTE: We consider a text line L as centered compared to another text line M, if:
   *      (a) the leftX offset (= L.leftX - M.leftX) is equal to the rightX offset
   *          (= L.rightX - M.rightX)
   *      (b) one line completely overlaps the other line horizontally, that is: one of the values
   *          returned by computeXOverlapRatios(line.prevLine, line) is equal to 1.
   * (2) There is at least one line (which does not represent a display formula) for which the
   *     leftX offset (resp. rightX offset) is larger than a given threshold;
   * (3) The number of justified text lines (i.e.: lines with leftX offset == rightX offset == 0)
   *     is smaller than a given threshold.
   *
   * @param block
   *    The text block to process.
   *
   * @return
   *    True if the lines in the given text block are centered with respect to the requirements
   *    described above; false otherwise.
   */
  bool computeIsTextLinesCentered(const PdfTextBlock* block) const;

  /**
   * This method returns true if the text of the given element is emphasized compared to the
   * majority of the rest of the text in the document.
   *
   * An element is considered to be emphasized when one of the following requirements is fulfilled:
   *  (1) The font size of the element is larger than the most frequent font size in the document;
   *  (2) The font weight of the element is larger than the most frequent font weight in the
   *      document, and the font size of the element is not smaller than the most freq. font size;
   *  (3) The text of the element is printed in italics, and the font size of the element is not
   *      smaller than the most frequent font size;
   *  (4) The text of the element contains at least one alphabetic character and all alphabetic
   *      characters are in uppercase.
   *
   * @param element
   *    The text element to process.
   *
   * @return
   *    True if the text of the given text block is emphasized, false otherwise.
   */
  bool computeIsEmphasized(const PdfTextElement* element) const;

  /**
   * This method returns true if the first of the two given lines has capacity, that is: if the
   * first word of the second line would have enough space to be placed at the end of the first
   * line (or: if the right margin of the first line is larger than the width of the first word of
   * the second line + some extra space for an additional whitespace).
   *
   * This method is primarily used to detect text block boundaries and forced line breaks. If this
   * method returns true, it is assumed that the two given lines do not belong to the same text
   * block, because otherwise the first word of the second line could have been placed at the end
   * of the first line.
   *
   * @param prevLine
   *    The first line.
   * @param line
   *    The second line.
   *
   * @return
   *    True if the first line has capacity, false otherwise.
   */
  bool computeHasPrevLineCapacity(const PdfTextLine* prevLine, const PdfTextLine* line) const;

  /**
   * This method checks if the given block is in hanging indent format (meaning that the first line
   * of a text block is not indented and the continuation lines are indented by a certain value).
   * If the block is in hanging indent format, this method returns a value > 0, denoting the value
   * (in pt) by which the continuation lines are indented. If the block is not in hanging indent
   * format, this method returns 0.0.
   *
   * For the returned value to be > 0.0, all of the following requirements must be fulfilled:
   * (1) The block must contain at least two lines with a length larger than a threshold.
   * (2) The block must contain at least one indented line, that is: a line with a left margin
   *     equal to the most frequent left margin (which is computed among the lines in the block
   *     that exhibit a left margin larger than a threshold).
   * (3) The block does not contain any non-indented line that starts with a lowercase character.
   *
   * Additionally, one of the following requirements must be fulfilled:
   * (a) The first line is not indented, but all other lines, and the first line has no capacity.
   *     This should identify single enumeration items in the format:
   *       Dynamics: The low energy behavior of
   *         a physical system depends on its
   *         dynamics.
   * (b) The number of non-indented lines exceeds a given threshold, and all non-indented lines
   *     start with an uppercase character.
   * (c) There is at least one indented line that start with an uppercase character, and the number
   *     of lines exceeds a given threshold.
   *
   * NOTE: The given text block may be a preliminary text block (computed while detecting text
   * blocks), meaning that it could contain multiple text blocks, which need to be split further
   * in a subsequent step.
   *
   * @param block
   *    The text block to process.
   *
   * @return
   *    A value > 0 if the block is in hanging indent format, and 0.0 otherwise.
   */
  double computeHangingIndent(const PdfTextBlock* block) const;

  /**
   * This method iterates through the text lines of the given block (the elements stored in
   * block.lines) and computes the left and right margins of each. Writes the computed left margin
   * of text line L to L.leftMargin and the computed right margin to L.rightMargin.
   *
   * The left margin of the text line L in block B is the distance between the left boundary of B
   * and the left boundary of L, that is: abs(L.leftX - B.trimLeftX).
   * The right margin of L is the distance between the right boundary of L and the right boundary
   * of B, that is: abs(B.trimRightX - L.rightX).
   *
   * TODO: The right margin of a text line is primarily used to check if the text line has capacity
   * (see the comment of computeHasPrevLineCapacity() for information about how
   * the capacity of a line is defined). There are text blocks that consists of only short lines
   * (meaning that they are shorter than the lines in the same column). See the second block on
   * page 2 in hep-ex0205091 for an example. For such blocks, the computed right margins of the
   * text lines are smaller than they actually are. A frequent consequence is that it is assumed
   * that the text lines have no capacity, although they actually do (because they are short, and
   * there is enough space for some further text). So the correct way of computing the right margin
   * would be to compute the distance between the right boundary of the text line and the "correct"
   * right boundary of the column. However, finding the "correct" right boundary of the column is
   * surprisingly difficult. One option is to use the largest rightX value of a line in the column.
   * But there could be outlier lines, which extend beyond the actual column boundaries. Another
   * option is to use the most frequent rightX among the lines in the column (this sounds promising
   * and should be tried out).
   *
   * @param block
   *    The text block to process.
   */
  void computeTextLineMargins(const PdfTextBlock* block) const;

  /**
   * This method returns true if the given text line is the first line of an enumeration item or of
   * a footnote.
   *
   * For the returned value to be true, the line must be prefixed by an item label (that is:
   * computeIsPrefixedByItemLabel(line) must return true) and one of the following further
   * requirements must be fulfilled:
   * (1) If the given line has a previous sibling line (stored in line.prevSibling), it is also
   *     prefixed by an item label, and it exhibits the same font and font size as the given line;
   * (2) If the given line has a next sibling line (stored in line.nextSibling), it is also
   *     prefixed by an item label, and it exhibits the same font and font size as the given line;
   * (3) The line is prefixed by a footnote label, that is: computeIsPrefixedByFootnoteLabel(line,
   *     potentialFootnoteLabels) returns true.
   *
   * @param line
   *    The text line to process.
   * @param potentialFootnoteLabels
   *    A set of strings that is used to check if the line is the first line of a footnote. It
   *    contains strings that occur somewhere in the document as a superscript, meaning that each
   *    string represents a potential footnote label. If a line starts with a string that occur in
   *    this set, we consider the line as a potential first line of a footnote. Further heuristics
   *    are used to distinguish lines which are indeed the first line of a footnote from lines that
   *    occasionally start with a footnote label (but are actually not part of a footnote).
   *
   * @return
   *    True if the given line is the first line of an enumeration item or of a footnote, false
   *    otherwise.
   */
  bool computeIsFirstLineOfItem(const PdfTextLine* line,
      const unordered_set<string>* potentialFootnoteLabels = nullptr) const;

  /**
   * This method returns true if the given line is a continuation line of an enumeration item or of
   * a footnote, that is: if the line belongs to an enumeration item (resp. a footnote) but it is
   * not the first line of the item (resp. the footnote).
   *
   * For the returned value to be true, the given line must have a parent line (stored in
   * line.parentLine), which is either the first line of an item (resp. footnote), or also the
   * continuation of an item (resp. footnote).
   *
   * TODO: The assumption here is that the continuation line of an item or footnote is indented
   * compared to the first line of the item (otherwise, the continuation does not have a parent
   * line). This is however not always the case (there are items where the continuation lines are
   * not intended).
   *
   * @param line
   *    The line to process.
   * @param potentialFootnoteLabels
   *    The set of potential footnote labels, passed to the computeIsFirstLineOfItem() method. See
   *    the comment given for this method for more information about this parameter.
   *
   * @return
   *    True if the given line is a continuation line of an enumeration item or a footnote, false
   *    otherwise.
   */
  bool computeIsContinuationOfItem(const PdfTextLine* line,
      const unordered_set<string>* potentialFootnoteLabels = nullptr) const;

  /**
   * This method computes potential footnote labels contained in the given line and appends it to
   * the given set.
   *
   * This method is primarily used by the text block detector, for detecting the first text lines
   * of footnotes. The motivation is the following: the first line of a footnote is usually
   * prefixed by a label that consists of a superscripted character or number, or a special symbol
   * like: *, †, ‡, §, ‖, ¶. However, a PDF can contain text lines which occasionally starts with
   * such a label, although they are not an actual part of a footnote. A possible consequence is
   * that lines which are not an actual part of a footnote are mistakenly detected as footnotes.
   *
   * One observation is that the label of a footnote usually occurs a second time in the body text
   * of the document (this is for referencing the footnote at a certain position in the body text).
   * We use this fact and scan the given line for labels (that is: superscripted numbers and the
   * special symbols mentioned above) that potentially reference a footnote. On detecting
   * footnotes, we consider a line to be the start of a footnote only when it is prefixed by text
   * that occurs in the computed set of potential footnote labels.
   *
   * @param line
   *    The line to process.
   * @param result
   *    The set to which the detected potential footnote labels should be appended.
   */
  void computePotentialFootnoteLabels(const PdfTextLine* line, unordered_set<string>* result) const;

  /**
   * This method returns true if the given line is prefixed by an enumeration item label, that is:
   * if it starts with a *superscripted* character that occurs in _config.superItemLabelAlphabet
   * or if it matches one of the regular expressions in _config.itemLabelRegexes (note that the
   * matching parts must *not* be superscripted).
   *
   * @param line
   *    The line to process.
   *
   * @return
   *    True if the line is prefixed by an enumeration label, false otherwise.
   */
  bool computeIsPrefixedByItemLabel(const PdfTextLine* line) const;

  /**
   * This method returns true if the given line is prefixed by a footnote label.
   *
   * For the returned value to be true, all of the following requirements must be fulfilled:
   * (1) The given line starts with one or more superscripted characters.
   * (2) If 'potentialFootnoteLabels' is specified, it must contain the superscripted prefix (= the
   *     concatenation of all superscripted characters in front of the line).
   *
   * @param line
   *    The line to process.
   * @param potentialFootnoteLabels
   *    The set of potential footnote labels.
   *
   * @return
   *    True if the line is prefixed by an enumeration label, false otherwise.
   */
  bool computeIsPrefixedByFootnoteLabel(const PdfTextLine* line,
      const unordered_set<string>* potentialFootnoteLabels = nullptr) const;

  /**
   * This method iterates through the given figures and returns the first figure which horizontally
   * overlaps the given element by a ratio larger than minXOverlapRatio and that vertically overlaps
   * the given element by a ratio larger than minYOverlapRatio.
   *
   * This method is primarily used by the text block detector, for determining whether or not two
   * text lines are part of the same figure (because they are overlapped by the same figure) and for
   * deciding whether or not both text lines belond to different text blocks.
   *
   * @param element
   *    The element which should be checked whether or not it is overlapped by a figure.
   * @param figures
   *    The vector of figures.
   *
   * @return
   *    The first figure in the given vector which fulfills the given minimum overlap ratios, or
   *    nullptr if there is no such figure.
   */
  PdfFigure* computeOverlapsFigure(const PdfElement* element,
      const vector<PdfFigure*>& figures) const;

  /**
   * This method creates a new `PdfTextBlock` instance consisting of the given text lines, computes
   * and sets all parameters of the instance and appends the instance to the given vector.
   *
   * @param lines
   *    The text lines belonging to the text block to create.
   * @param blocks
   *    The vector to which the created text block should be appended.
   */
  void createTextBlock(const vector<PdfTextLine*>& lines, vector<PdfTextBlock*>* blocks) const;

 private:
  // The configuration to use.
  const TextBlocksDetectionConfig* _config;
};

}  // namespace ppp::utils

#endif  // UTILS_TEXTBLOCKSDETECTIONUTILS_H_
