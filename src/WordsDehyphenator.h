/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef WORDSDEHYPHENATOR_H_
#define WORDSDEHYPHENATOR_H_

#include "./Config.h"
#include "./PdfDocument.h"

using ppp::Config;

// =================================================================================================

class WordsDehyphenator {
 public:
  explicit WordsDehyphenator(PdfDocument* doc, const Config& config);

  ~WordsDehyphenator();

  void dehyphenate() const;

 private:
  PdfDocument* _doc;
  Config _config;
};

#endif  // WORDSDEHYPHENATOR_H_
