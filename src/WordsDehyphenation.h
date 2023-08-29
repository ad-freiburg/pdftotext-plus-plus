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
using ppp::types::PdfDocument;

// =================================================================================================

namespace ppp::modules {

class WordsDehyphenation {
 public:
  WordsDehyphenation(PdfDocument* doc, const WordsDehyphenationConfig* config);

  ~WordsDehyphenation();

  void process() const;

 private:
  PdfDocument* _doc;
  const WordsDehyphenationConfig* _config;
};

}  // namespace ppp::modules

#endif  // WORDSDEHYPHENATION_H_
