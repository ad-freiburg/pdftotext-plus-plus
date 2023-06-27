/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef DIACRITICALMARKSMERGING_H_
#define DIACRITICALMARKSMERGING_H_

#include "./utils/Log.h"
#include "./Config.h"
#include "./PdfDocument.h"

using ppp::config::DiacriticalMarksMergingConfig;

// =================================================================================================

namespace ppp {

/**
 * This class is responsible for merging diacritical marks with their base characters.
 *
 * This is needed, because characters with diacritical marks can be represented by two characters.
 * For example, the character "à" can be represented by the base character "a" and the
 * combining diacritical mark "`".
 */
class DiacriticalMarksMerging {
 public:
  /**
   * The default constructor.
   *
   * @param doc
   *    The PDF document to process, with the characters extracted from the i-th page stored in
   *    doc->pages[i]->characters.
   * @param config
   *    The configuration to use.
   */
  DiacriticalMarksMerging(PdfDocument* doc, const DiacriticalMarksMergingConfig& config);

  /** The deconstructor */
  ~DiacriticalMarksMerging();

  /**
   * This method merges each diacritic mark with its respective base character.
   *
   * The basic approach is as follows: The characters of each page are iterated. For each
   * character, it is checked whether or not it represents a diacritic mark. If so, the horizontal
   * overlap with the respective previous and next character is computed. The diacritic mark is
   * then merged with the character yielding the larger horizontal overlap (if this overlap exceeds
   * a certain threshold).
   *
   * Let `mark` be a diacritic mark and `base` the base character with which the diacritic mark
   * should be merged. The actual merging process is realized as follows:
   *  - `mark->isDiacriticMarkOfBaseChar` is set to `base`,
   *  - `base->isBaseCharOfDiacriticMark` is set to `mark`,
   *  - `base->textWithDiacriticMark` is set to the string containing the character with the
   *     diacritic mark represented by a single character,
   *  - `base->position` is updated to the bounding box surrounding both characters.
   *
   * NOTE: The character representing the diacritic mark is *not* removed from `page->characters`.
   * If you want to exclude the character from further processing, you need to check whether or not
   * `mark->isDiacriticMarkOfBaseChar` is set.
   */
  void process() const;

 private:
  // The PDF document to process.
  PdfDocument* _doc;
  // The configuration to use.
  DiacriticalMarksMergingConfig _config;
  // The logger.
  Logger* _log;
};

}  // namespace ppp

#endif  // DIACRITICALMARKSMERGING_H_
