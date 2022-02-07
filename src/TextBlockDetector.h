/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TEXTBLOCKDETECTOR_H_
#define TEXTBLOCKDETECTOR_H_

#include <vector>

#include "./PdfDocument.h"


class TextBlockDetector {
 public:
  explicit TextBlockDetector(PdfDocument* doc);

  ~TextBlockDetector();

  void detect();

 private:
  void createTextBlock(const std::vector<PdfTextLine*>& lines, std::vector<PdfTextBlock*>* blocks);

  bool computeIsTextBlockEmphasized(const std::vector<PdfTextLine*>& lines);

  PdfDocument* _doc;
};

#endif  // TEXTBLOCKDETECTOR_H_
