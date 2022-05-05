/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TEXTLINESDETECTOR_H_
#define TEXTLINESDETECTOR_H_

#include <vector>

#include "./utils/LogUtils.h"
#include "./PdfDocument.h"

// =================================================================================================

/**
 * This class groups the words of a PDF document into text lines.
 */
class TextLinesDetector {
 public:
  /**
   * This constructor creates and initializes a new instance of this `TextLinesDetector` class.
   *
   * @param doc
   *   The PDF document to process.
   * @param debug
   *   Whether or not this instance should print debug information to the console.
   * @param debugPageFilter
   *   The number of the page to which the debug information should be reduced. If specified as a
   *   value > 0, only those messages that relate to the given page will be printed to the console.
   */
  TextLinesDetector(PdfDocument* doc, bool debug=false, int debugPageFilter=-1);

  /** The deconstructor. */
  ~TextLinesDetector();

  /**
   * This method groups the words of the given PDF document (detected by the `WordsDetector` class)
   * into text lines. The basic procedure is as follows: The given PDF document is processed
   * page-wise. For each page, the words stored in `page->words` are clustered first by their
   * rotation and then by their lowerY coordinates. From each cluster, a (preliminary)
   * `PdfTextLine` is created. The preliminary text lines are then merged in rounds. In each round,
   * the text lines that vertically overlap are merged. This is repeated until there are no text
   * lines anymore that vertically overlap. This should merge words that were assigned to different
   * clusters but actually belong to the same text line, which particulary often happens in
   * case of sub- and superscripts, or fractions in formulas. The final text lines of a page are
   * appended to `page->lines`.
   */
  void detect();

 private:
  /**
   * This method (a) creates a new `PdfTextLine` object from the given list of words, (b) computes
   * the respective layout information of the text line and (c) appends the text line to the given
   * result list.
   *
   * @param words
   *   The words from which to create the text line.
   * @param segment
   *   The segment of which the text line is a part.
   * @param words
   *   The vector to which the created text line should be appended.
   */
  void createTextLine(const std::vector<PdfWord*>& words, const PdfPageSegment* segment,
      std::vector<PdfTextLine*>* lines) const;

  /**
   * This method merges the given second text line with the given first text line. This is done
   * by adding all words of the second text line to the first text line and (re-)computing the
   * text line properties of the first text line by calling computeTextLineProperties(line1).
   *
   * @param line1
   *    The first text line to merge.
   * @param line2
   *    The second text line to merge.
   */
  void mergeTextLines(PdfTextLine* line1, PdfTextLine* line2) const;

  /**
   * This method iterates the words stored in `line->words` and computes all layout information
   * about the text line (for example: the bounding box or the font). The computed information
   * is written to the respective member variables of the text line.
   *
   * @param line
   *    The text line for which to compute the layout information.
   */
  void computeTextLineProperties(PdfTextLine* line) const;

  /** The PDF document to process. */
  PdfDocument* _doc;

  /** The logger. */
  Logger* _log;
};

#endif  // TEXTLINESDETECTOR_H_
