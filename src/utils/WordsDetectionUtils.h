/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_WORDSDETECTIONUTILS_H_
#define UTILS_WORDSDETECTIONUTILS_H_

#include <vector>

#include "../Config.h"
#include "../PdfDocument.h"

using std::vector;

using ppp::config::WordsDetectionConfig;
using ppp::types::PdfCharacter;
using ppp::types::PdfWord;

// =================================================================================================

namespace ppp::utils {

/**
 * A collection of some useful and commonly used functions in context of words.
 */
class WordsDetectionUtils {
 public:
  /**
   * The default constructor.
   *
   * @param config
   *   The configuration to use.
   */
  explicit WordsDetectionUtils(const WordsDetectionConfig* config);

  ~WordsDetectionUtils();

  /**
   * This method (a) creates a new `PdfWord` instance from the given vector of characters,
   * and (b) computes the respective layout information of the word.
   *
   * @param characters
   *   The characters from which to create the word.
   *
   * @return
   *    The created word.
   */
  PdfWord* createWord(const vector<PdfCharacter*>& chars) const;

 private:
  // The configuration to use.
  const WordsDetectionConfig* _config;
};

}  // namespace ppp::utils

#endif  // UTILS_WORDSDETECTIONUTILS_H_

