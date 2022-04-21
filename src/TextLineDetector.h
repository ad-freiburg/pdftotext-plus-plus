/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TEXTLINEDETECTOR_H_
#define TEXTLINEDETECTOR_H_

#include <vector>

#include "./PdfDocument.h"


class TextLineDetector {
 public:
  explicit TextLineDetector(PdfDocument* doc);

  ~TextLineDetector();

  void detect();

 private:
  void tokenize();
  void computeTextLineProperties(PdfTextLine* line);

  PdfDocument* _doc;
  int _numTextLines = 0;
};

#endif  // TEXTLINEDETECTOR_H_
