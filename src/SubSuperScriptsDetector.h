/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef SUBSUPERSCRIPTSDETECTOR_H_
#define SUBSUPERSCRIPTSDETECTOR_H_

#include "./utils/Log.h"

#include "./PdfDocument.h"

// =================================================================================================
// CONFIG

namespace sub_super_scripts_detector::config {

// A parameter that is used to detect sub- and superscripts. It denotes the maximum allowed
// difference between the baseline of a character and the baseline of a text line, so that the
// character "sit" on the same baseline. If the baseline of a character is larger than the baseline
// of the text line (under consideration of the threshold), the character is considered to
// be a superscript. If the baseline is smaller, the character is considered to be a subscript.
const double BASE_EQUAL_TOLERANCE = 0.1;

// A parameter that is used to detect sub- and superscripts. It denotes the maximum allowed
// difference between the font size of a character and the most frequent font size in the document,
// so that the font sizes are considered to be equal. If the font size of a character is smaller
// than the most frequent font size (under consideration of the threshold), the character is
// considered to be a sub- or superscript. Otherwise, it is not considered to be a sub-/superscript.
const double FSIZE_EQUAL_TOLERANCE = 0.9;

}  // namespace sub_super_scripts_detector::config

// =================================================================================================

/**
 * This class is responsible for detecting sub- and superscripted characters in a PDF document.
 *
 * The basic approach is as follows: A given PDF document is processed text line-wise. For each
 * text line, the contained characters are iterated from left to right. A character is considered
 * to be subscripted, when its font size is smaller than the most frequent font size among all
 * characters in the PDF document (under consideration of a threshold), and its base line is lower
 * than the base line of the text line.
 * Similarily, A character is considered to be superscripted, when its font size is smaller than
 * the most frequent font size and its base line is *higher* than the base line of the text line.
*/
class SubSuperScriptsDetector {
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
  explicit SubSuperScriptsDetector(const PdfDocument* doc, LogLevel logLevel = ERROR,
      int logPageFilter = -1);

  /** The deconstructor. */
  ~SubSuperScriptsDetector();

  /**
   * This method starts the process of detecting sub- and superscripted characters in the given
   * PDF document.
   *
   * In addition to the detection of sub- and superscripted characters, this method also computes
   * the base bounding box of each text line, that is: the bounding box around all characters of
   * a text line which are not sub- or superscripted.
   */
  void process() const;

 private:
  // The PDF document to process.
  const PdfDocument* _doc;

  // The logger.
  const Logger* _log;
};

// =================================================================================================

#endif  // SUBSUPERSCRIPTSDETECTOR_H_
