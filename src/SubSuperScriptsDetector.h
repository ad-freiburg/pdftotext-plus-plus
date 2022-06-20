/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef SUBSUPERSCRIPTSDETECTOR_H_
#define SUBSUPERSCRIPTSDETECTOR_H_

#include "./PdfDocument.h"

// =================================================================================================

struct SubSuperScriptsDetectorConfig;

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
   */
  explicit SubSuperScriptsDetector(const PdfDocument* doc);

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

  // The config.
  SubSuperScriptsDetectorConfig* _config;
};

// =================================================================================================

/**
 * This struct provides the configuration (= thresholds and parameters) to be used by the
 * `SubSuperScriptsDetector` class while detecting sub- and superscripted characters.
 */
struct SubSuperScriptsDetectorConfig {
  /**
   * This method returns a tolerance used while checking if the font size of a character is smaller
   * than the most frequent font size in the PDF document. Only if the font size of a character is
   * smaller than the most freq. font size by the returned value, the character is actually
   * considered to be smaller. Otherwise, it is considered to be not smaller.
   *
   * @param doc
   *    A reference to the PDF document currently processed, which can be used to get general
   *    statistics about the document, for example: the average character width and -height.
   *
   * @return
   *    The tolerance to use while checking whether or not the font size of a character is smaller
   *    than the most frequent font size.
   */
  double getFontSizeTolerance(const PdfDocument* doc) const {
    return 0.9;
  }
};

#endif  // SUBSUPERSCRIPTSDETECTOR_H_
