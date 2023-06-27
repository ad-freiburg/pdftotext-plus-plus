/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_TEXTLINESDETECTIONUTILS_H_
#define UTILS_TEXTLINESDETECTIONUTILS_H_

#include <regex>
#include <string>
#include <unordered_set>

#include "../Config.h"
#include "../PdfDocument.h"

using std::regex;
using std::string;
using std::unordered_set;

using ppp::config::TextLinesDetectionConfig;

// =================================================================================================

namespace ppp::utils {

/**
 * A collection of some useful and commonly used functions in context of text lines detection.
 */
class TextLinesDetectionUtils {
 public:
  /**
   * The default constructor.
   *
   * @param config
   *   The configuration to use.
   */
  explicit TextLinesDetectionUtils(const TextLinesDetectionConfig& config);

  /** The deconstructor. */
  ~TextLinesDetectionUtils();

  // ===============================================================================================

  /**
   * This method computes the parent text line, the previous sibling text line and the next sibling
   * text line for each text line of the given page. Here is an explanation of the different types
   * of lines:
   *
   * - Parent Text Line: a text line L is the parent text line of text line M if
   *   (a) L is the nearest previous text line of M with L.leftX < M.leftX (meaning that M is
   *       indented compared to L).
   *   (b) the line distance between L and M is smaller than a given threshold.
   *   (c) L.lowerY < M.lowerY (meaning that M must be positioned below L).
   *
   * - Previous Sibling Text Line: a text line L is the previous sibling text line of text line M
   *   if
   *   (a) L is the nearest previous text line of M with L.leftX == M.leftX (under consideration
   *       of a small tolerance)
   *   (b) there is no other text line K between L and M with K.leftX < M.leftX.
   *   (c) the line distance between L and M is smaller than a given threshold.
   *   (d) L.lowerY < M.lowerY (meaning that M must be positioned below L).
   *
   * - Next Sibling Text Line: a text line L is the next sibling text line of text line M if:
   *   (a) L is the nearest next text line of M with L.leftX == M.leftX (under consideration
   *       of a small tolerance)
   *   (b) there is no other text line K between M and L with K.leftX < M.leftX.
   *   (c) the line distance between L and M is smaller than a given threshold.
   *   (d) L.lowerY < M.lowerY (meaning that M must be positioned below L).
   *
   * Here is an example which helps to understand the different line types:
   *
   * Aarseth S J 1999 PASP 111 1333            (1)
   * Amaro-Seoane P, Gair J R, Freitag M,      (2)
   *   Miller M C, Mandel I, Cutler C J        (3)
   *   and Babak S 2007 Classical and          (4)
   *   Quantum Gravity 24 113                  (5)
   * Brown D A, Brink J, Fang H, Gair J R,     (6)
   *   Li C, Lovelace G, Mandel I and Thorne   (7)
   *     K S 2007 PRL 99 201102                (8)
   *
   * Line (1):  parent: -        ; prev sibling: -        ; next sibling: line (2)
   * Line (2):  parent: -        ; prev sibling: line (1) ; next sibling: line (6)
   * Line (3):  parent: line (2) ; prev sibling: -        ; next sibling: line (4)
   * Line (4):  parent: line (2) ; prev sibling: line (3) ; next sibling: line (5)
   * Line (5):  parent: line (2) ; prev sibling: line (4) ; next sibling: -
   * Line (6):  parent: -        ; prev sibling: line (2) ; next sibling: -
   * Line (7):  parent: line (6) ; prev sibling: -        ; next sibling: -
   * Line (8):  parent: line (7) ; prev sibling: -        ; next sibling: -
   *
   * The entry for line (3) in the above listing is to be read as follows:
   *  - "The parent line of line (3) is line (2)";
   *  - "Line (3) has no previous sibling line."
   *  - "The next sibling line of line (3) is line (4)."
   *
   * The reason why line (5) is not a previous sibling of line (7) is that there is line (6)
   * in between, which have a smaller leftX than line (5) and line (7).
   *
   * @param page
   *    The page to process.
   */
  void computeTextLineHierarchy(const PdfPage* page);

  /**
   * This method returns true if the text of the given text line ends with a sentence delimiter.
   *
   * @param line
   *    The text line to process.
   *
   * @return
   *    True if the text of the given text line ends with a sentence delimiter, false otherwise.
   */
  bool computeEndsWithSentenceDelimiter(const PdfTextLine* line);

 private:
  // The configuration to use.
  TextLinesDetectionConfig _config;
};

}  // namespace ppp::utils

#endif  // UTILS_TEXTLINESDETECTIONUTILS_H_
