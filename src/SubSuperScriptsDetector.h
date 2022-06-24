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

/**
 * This struct provides the configuration (= thresholds and parameters) to be used by the
 * `SubSuperScriptsDetector` class while detecting sub- and superscripted characters.
 */
namespace sub_super_scripts_detector::config {

// TODO
// The maximum allowed difference between the baselines of two characters, so that the characters
// are considered to "sit" on the same baseline. This is needed to identify sub- and superscripts.
// If the baseline of a character is larger than the baseline of the parent text line (under
// consideration of the tolerance), the character is considered to be a superscript. If the
// baseline is smaller, the character is considered to be a subscript.
constexpr double getBaselineEqualTolerance(const PdfDocument* doc) {
  return 0.1;
}

// TODO
// The maximum allowed difference between two font sizes so that they are considered to be equal.
constexpr double getFontsizeEqualTolerance(const PdfDocument* doc) {
  return 0.9;
}

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
   * @param debug
   *   Whether or not this instance should print debug information to the console.
   * @param debugPageFilter
   *   If set to a value > 0, only the debug messages produced while processing the
   *   <debugPageFilter>-th page of the current PDF file will be printed to the console.
   */
  explicit SubSuperScriptsDetector(const PdfDocument* doc, bool debug = false,
      int debugPageFilter = -1);

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
