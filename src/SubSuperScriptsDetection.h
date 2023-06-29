/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef SUBSUPERSCRIPTSDETECTION_H_
#define SUBSUPERSCRIPTSDETECTION_H_

#include "./utils/Log.h"
#include "./Config.h"
#include "./PdfDocument.h"

using ppp::config::SubSuperScriptsDetectionConfig;
using ppp::utils::log::Logger;

// =================================================================================================

namespace ppp {

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
class SubSuperScriptsDetection {
 public:
  /**
   * This constructor creates and initializes a new instance of this class.
   *
   * @param doc
   *   The PDF document to process.
   * @param config
   *   The configuration to use.
   */
  SubSuperScriptsDetection(PdfDocument* doc, const SubSuperScriptsDetectionConfig& config);

  /** The deconstructor. */
  ~SubSuperScriptsDetection();

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
  PdfDocument* _doc;
  // The configuration to use.
  SubSuperScriptsDetectionConfig _config;
  // The logger.
  Logger* _log;
};

}  // namespace ppp

#endif  // SUBSUPERSCRIPTSDETECTION_H_
