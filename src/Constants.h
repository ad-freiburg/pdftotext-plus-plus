/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

const int FONT_SIZE_PREC = 1;
const int LINE_DIST_PREC = 1;
const int LEFT_MARGIN_PREC = 0;

const double EQUAL_TOLERANCE = 0.0001;
const double FONTSIZE_EQUAL_TOLERANCE = 1;

// =================================================================================================

/**
 * The characters we consider to be a sentence delimiter. Used by, for example, the
 * text_element_utils::computeEndsWithSentenceDelimiter() method.
 */
const char* const SENTENCE_DELIMITER_ALPHABET = "?!.);";

#endif  // CONSTANTS_H_