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

#include "./utils/Log.h"
#include "./utils/MathUtils.h"

#include "./PdfDocument.h"

using std::vector;

// =================================================================================================
// CONFIG

namespace text_lines_detector::config {

// A parameter used for detecting text lines. It denotes the precision to use when rounding the
// lowerY of a word for computing the cluster to which the word should be assigned.
const double COORDS_PREC = global_config::COORDS_PREC;

/**
 * This method returns a threshold to be used for detecting text lines. It denotes the maximum
 * vertical overlap ratio that two consecutive text lines must achieve so that the text lines are
 * merged. If the maximum vertical overlap ratio between two consecutive lines is larger or equal
 * to the returned threshold, the text lines are merged; otherwise the text lines are not merged.
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
constexpr double getYOverlapRatioThreshold(const PdfDocument* doc, double xGap) {
  return xGap < 3 * doc->avgCharWidth ? 0.4 : 0.8;
}

}  // namespace text_lines_detector::config

// =================================================================================================

/**
 * This class is responsible for detecting text lines from the words of a PDF document.
 *
 * The basic approach is as follows: A given PDF-document is processed segment-wise. The words of a
 * segment are clustered twice, first by their rotations, then by their leftX values. For each
 * cluster, a text line consisting of the words of the cluster is created. The created lines are
 * sorted by their lowerY values in ascending order (from top to bottom).
 *
 * Consecutive lines that vertically overlap each other are merged in rounds, until there are no
 * consecutive lines that vertically overlap anymore. This should merge words that were assigned to
 * different clusters but actually belong to the same text line, because they are sub- or
 * superscripted, or they are part of fractions in formulas.
 */
class TextLinesDetector {
 public:
  /**
   * This constructor creates and initializes a new instance of this class.
   *
   * @param doc
   *   The PDF document to process.
   * @param logLevel
   *   The logging level.
   * @param logPageFilter
   *   If set to a value > 0, only the logging messages produced while processing the
   *   <logPageFilter>-th page of the current PDF file will be printed to the console.
   */
  explicit TextLinesDetector(const PdfDocument* doc, LogLevel logLevel = ERROR,
      int logPageFilter = -1);

  /** The deconstructor. */
  ~TextLinesDetector();

  /**
   * This method starts the detection of text lines in the given PDF document.
   */
  void process();

 private:
  /**
   * This method creates a new `PdfTextLine` from the given vector of words, computes and sets the
   * respective layout properties of the text line, and appends the text line to the given result
   * vector.
   *
   * @param words
   *   The words that are part of the text line.
   * @param segment
   *   The segment of which the text line is a part.
   * @param lines
   *   The vector to which the created text line should be appended.
   *
   * @return
   *   The created text line.
   */
  PdfTextLine* createTextLine(const vector<PdfWord*>& words, const PdfPageSegment* segment,
      vector<PdfTextLine*>* lines) const;

  /**
   * This method merges the given first text line with the given second text line. This is
   * accomplished by adding all words of the first text line to the words of the second text line
   * and (re-)computing the layout properties of the second text line, by invoking
   * computeTextLineProperties(line2).
   *
   * @param line1
   *    The text line to merge with the second text line.
   * @param line2
   *    The text line to be merged with the first text line.
   */
  void mergeTextLines(const PdfTextLine* line1, PdfTextLine* line2) const;

  /**
   * This method iterates through the words stored in `line->words` and computes all layout
   * properties of the text line (for example: the bounding box, or the font). The computed
   * properties are written to the respective member variables of the text line.
   *
   * @param line
   *    The text line for which to compute the layout properties.
   */
  void computeTextLineProperties(PdfTextLine* line) const;

  // The PDF document to process.
  const PdfDocument* _doc;

  // The logger.
  const Logger* _log;
};

#endif  // TEXTLINESDETECTOR_H_
