/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef DIACRITICMARKSCOMBINER_H_
#define DIACRITICMARKSCOMBINER_H_

#include <unordered_map>

#include "./PdfDocument.h"

// A mapping of diacritic marks to their "combining" equivalents.
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

class DiacriticMarksCombiner {
 public:
  explicit DiacriticMarksCombiner(PdfDocument* doc);

  ~DiacriticMarksCombiner();

  void combine();

 private:
  PdfDocument* _doc;
};

#endif  // DIACRITICMARKSCOMBINER_H_
