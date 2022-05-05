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

/**
 * Given the words of a PDF document, this class groups the words into text lines.
 */
class TextLinesDetector {
 public:
  /**
   * This constructor creates and initializes a new instance of this `TextLinesDetector` class.
   *
   * @param doc
   *   The document to process.
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
   * This method groups the words of the given document (detected by the `WordsDetector` class)
   * into text lines. The basic procedure is as follows: The given PDF document is processed
   * page-wise. For each page, the words stored in `page->words` are clustered first by their
   * rotation and then by their lowerY coordinates. From each cluster, a `PdfTextLine` is created.  iterates through the pages of the given document.
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

  // TODO
  void mergeTextLines(PdfTextLine* line1, PdfTextLine* line2) const;

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
  void computeTextLineProperties(PdfTextLine* line) const;

  /** The PDF document to process. */
  PdfDocument* _doc;

  /** The logger. */
  Logger* _log;
};

#endif  // TEXTLINESDETECTOR_H_
