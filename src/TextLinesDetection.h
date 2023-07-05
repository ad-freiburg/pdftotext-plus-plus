/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TEXTLINESDETECTION_H_
#define TEXTLINESDETECTION_H_

#include <vector>

#include "./utils/Log.h"
#include "./utils/MathUtils.h"
#include "./utils/TextLinesDetectionUtils.h"
#include "./Config.h"
#include "./PdfDocument.h"

using std::vector;

using ppp::config::TextLinesDetectionConfig;
using ppp::utils::TextLinesDetectionUtils;
using ppp::utils::log::Logger;

// =================================================================================================

namespace ppp {

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
class TextLinesDetection {
 public:
  /**
   * This constructor creates and initializes a new instance of this class.
   *
   * @param doc
   *   The PDF document to process.
   * @param config
   *   The configuration to use.
   */
  TextLinesDetection(PdfDocument* doc, const TextLinesDetectionConfig& config);

  /** The deconstructor. */
  ~TextLinesDetection();

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
  PdfDocument* _doc;
  // The configuration to use.
  TextLinesDetectionConfig _config;
  // The text line detection utils.
  TextLinesDetectionUtils* _utils;
  // The logger.
  Logger* _log;
};

}  // namespace ppp

#endif  // TEXTLINESDETECTION_H_
