/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef WORDSDEHYPHENATION_H_
#define WORDSDEHYPHENATION_H_

#include "./Config.h"
#include "./PdfDocument.h"

using ppp::config::WordsDehyphenationConfig;

// =================================================================================================

namespace ppp::modules {

class WordsDehyphenation {
 public:
  WordsDehyphenation(PdfDocument* doc, const WordsDehyphenationConfig& config);

  ~WordsDehyphenation();

  void dehyphenate() const;

 private:
  PdfDocument* _doc;
  WordsDehyphenationConfig _config;
};

}  // namespace ppp::modules

#endif  // WORDSDEHYPHENATION_H_
