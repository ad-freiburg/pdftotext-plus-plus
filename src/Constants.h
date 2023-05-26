/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <string>
#include <unordered_set>

using std::string;
using std::unordered_set;

// =================================================================================================

namespace global_config {

const int COORDS_PREC = 1;
const int FONT_SIZE_PREC = 1;
const int LINE_DIST_PREC = 1;

const double DOUBLE_EQUAL_TOLERANCE = 0.0001;
const double FS_EQUAL_TOLERANCE = 1;
const double COORDS_EQUAL_TOLERANCE = 0.1;

const int ID_LENGTH = 8;

/**
 * The characters we use as an identifier for formulas.
 */
const char* const FORMULA_ID_ALPHABET = "=+";

/**
 * The characters we consider to be alphanumerical. Used by, for example, the
 * ppp::ppp::string_utils::createRandomString() method.
 */
const char* const ALPHA_NUM_ALPHABET =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

/**
 * The characters we consider to be word delimiters.
 */
const char* const WORD_DELIMITERS_ALPHABET = " \t\r\n\f\v";

/**
 * The characters we consider to be a sentence delimiter. Used by, for example, the
 * text_element_utils::computeEndsWithSentenceDelimiter() method.
 */
const char* const SENTENCE_DELIMITER_ALPHABET = "?!.);";

/**
 * A set of common last name prefixes. This is used, for example, by
 * text_blocks_utils::computeHangingIndent(), for checking if a given block is in hanging indent
 * format. Normally, a block is not considered as a block in hanging indent when it contains one or
 * more unindented lines that start with a lowercase character. However, there are references
 * in hanging indent format that start with a (lowercased) last name prefix. To not accidentally
 * consider a block to be not in hanging indent format when it contains a line starting with
 * such prefix, we do not count a line as a lowercased line if it starts with a word contained in
 * this set.
 */
const unordered_set<string> LAST_NAME_PREFIXES = { "van", "von", "de" };


}  // namespace global_config

const int LEFT_MARGIN_PREC = 0;

// The resolution in DPI.
const double H_DPI = 72.0;
const double V_DPI = 72.0;

// =================================================================================================

/**
 * The characters which we consider, to be a valid footnote label (in addition to alphanumerical
 * symbols). This is used by, for example, the text_lines_utils::computePotentialFootnoteLabels()
 * method.
 */
const char* const SPECIAL_FOOTNOTE_LABELS_ALPHABET = "*∗†‡§‖¶?";

#endif  // CONSTANTS_H_
