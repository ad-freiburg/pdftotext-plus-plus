/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef DIACRITICALMARKSMERGER_H_
#define DIACRITICALMARKSMERGER_H_

#include <unordered_map>

#include "./utils/Log.h"

#include "./PdfDocument.h"

// =================================================================================================

// Diacritical marks exist in two variants: a "non-combining" variant and a "combining" variant.
// For example, for the grave accent ("`"), the non-combining variant is 0x0060 ("GRAVE ACCENT"),
// and the combining variant is 0x0300 ("COMBINING GRAVE ACCENT"). In PDF, diacritic marks can
// occur in both variants. But for merging, we need the combining variant. This map maps
// non-combining diacritic marks to their combining equivalents.
static const std::unordered_map<unsigned int, unsigned int> combiningMap = {
  { 0x0022, 0x030B },
  { 0x0027, 0x0301 },
  { 0x005E, 0x0302 },
  { 0x005F, 0x0332 },
  { 0x0060, 0x0300 },
  { 0x007E, 0x0303 },
  { 0x00A8, 0x0308 },
  { 0x00AF, 0x0304 },
  { 0x00B0, 0x030A },
  { 0x00B4, 0x0301 },
  { 0x00B8, 0x0327 },
  { 0x02B2, 0x0321 },
  { 0x02B7, 0x032B },
  { 0x02B9, 0x0301 },
  { 0x02CC, 0x0329 },
  { 0x02BA, 0x030B },
  { 0x02BB, 0x0312 },
  { 0x02BC, 0x0313 },
  { 0x02BD, 0x0314 },
  { 0x02C6, 0x0302 },
  { 0x02C7, 0x030C },
  { 0x02C8, 0x030D },
  { 0x02C9, 0x0304 },
  { 0x02CA, 0x0301 },
  { 0x02CB, 0x0300 },
  { 0x02CD, 0x0331 },
  { 0x02D4, 0x031D },
  { 0x02D5, 0x031E },
  { 0x02D6, 0x031F },
  { 0x02D7, 0x0320 },
  { 0x02DA, 0x030A },
  { 0x02DC, 0x0303 },
  { 0x02DD, 0x030B },
  { 0x0384, 0x0301 },
  { 0x0485, 0x0314 },
  { 0x0486, 0x0313 },
  { 0x0559, 0x0314 },
  { 0x055A, 0x0313 },
  { 0x204E, 0x0359 }
};

/**
 * This class merges diacritical marks with their base characters.
 *
 * This is needed, because characters with diacritical marks can be represented by two characters.
 * For example, the character "à" can be represented by the base character "a" and the
 * combining diacritical mark "`".
 */
class DiacriticalMarksMerger {
 public:
  /**
   * This constructor creates and initializes a new instance of this class.
   *
   * @param doc
   *    The PDF document to process, with the characters extracted from the i-th page stored in
   *    doc->pages[i]->characters.
   * @param debug
   *   Whether or not this instance should print debug information to the console.
   * @param debugPageFilter
   *   If set to a value > 0, only the debug messages produced while processing the
   *   <debugPageFilter>-th page of the current PDF file will be printed to the console.
   */
  explicit DiacriticalMarksMerger(PdfDocument* doc, bool debug = false, int debugPageFilter = -1);

  /** The deconstructor */
  ~DiacriticalMarksMerger();

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
  void process();

 private:
  // The PDF document to process.
  PdfDocument* _doc;
  // The logger.
  Logger* _log;
};

#endif  // DIACRITICALMARKSMERGER_H_
