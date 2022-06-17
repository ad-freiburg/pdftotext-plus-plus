/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef WORDSDEHYPHENATOR_H_
#define WORDSDEHYPHENATOR_H_

#include "./PdfDocument.h"

class WordsDehyphenator {
 public:
  explicit WordsDehyphenator(PdfDocument* doc);

  ~WordsDehyphenator();

  void dehyphenate();

 private:
  PdfDocument* _doc;
};

#endif  // WORDSDEHYPHENATOR_H_
