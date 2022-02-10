/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef SUBSUPERSCRIPTSDETECTOR_H_
#define SUBSUPERSCRIPTSDETECTOR_H_

#include <vector>

#include "./PdfDocument.h"


class SubSuperScriptsDetector {
 public:
  explicit SubSuperScriptsDetector(PdfDocument* doc);

  ~SubSuperScriptsDetector();

  void detect() const;

 private:
  PdfDocument* _doc;
};

#endif  // SUBSUPERSCRIPTSDETECTOR_H_