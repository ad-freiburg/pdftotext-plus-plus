/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_WORDSUTILS_H_
#define UTILS_WORDSUTILS_H_

#include <vector>

#include "../PdfDocument.h"

using std::vector;

/**
 * A collection of some useful and commonly used functions in context of words.
 */
namespace words_utils {

/**
 * This method (a) creates a new `PdfWord` instance from the given vector of characters,
 * and (b) computes the respective layout information of the word.
 *
 * @param characters
 *   The characters from which to create the word.
 * @param idLength
 *   The length of the word's id to be created.
 * @param doc
 *   The PDF document of which the word is a part of.
 *
 * @return
 *    The created word.
 */
PdfWord* createWord(const vector<PdfCharacter*>& characters, int idLength, const PdfDocument* doc);

}  // namespace words_utils

#endif  // UTILS_WORDSUTILS_H_

