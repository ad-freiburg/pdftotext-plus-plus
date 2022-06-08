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

// =================================================================================================

const int FONT_SIZE_PREC = 1;
const int LINE_DIST_PREC = 1;
const int LEFT_MARGIN_PREC = 0;

const double EQUAL_TOLERANCE = 0.0001;
const double FONTSIZE_EQUAL_TOLERANCE = 1;

// The resolution in DPI.
const double H_DPI = 72.0;
const double V_DPI = 72.0;

// =================================================================================================

/**
 * The characters we consider to be alphanumerical. Used by, for example, the
 * string_utils::createRandomString() method.
 */
const char* const ALPHA_NUM_ALPHABET =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

/**
 * The characters we consider to be a sentence delimiter. Used by, for example, the
 * text_element_utils::computeEndsWithSentenceDelimiter() method.
 */
const char* const SENTENCE_DELIMITER_ALPHABET = "?!.);";

/**
 * The characters we use as an identifier for formulas.
 */
const char* const FORMULA_ID_ALPHABET = "=+";

/**
 * The characters which we consider to be a valid part of a superscripted item label. This is used
 * by, for example, the text_lines_utils::computeIsPrefixedByItemLabel() method.
 */
const char* const SUPER_ITEM_LABEL_ALPHABET = "*∗abcdefghijklmnopqrstuvwxyz01234567890()";

/**
 * The characters which we consider, to be a valid footnote label (in addition to alphanumerical
 * symbols). This is used by, for example, the text_lines_utils::computePotentialFootnoteLabels()
 * method.
 */
const char* const SPECIAL_FOOTNOTE_LABELS_ALPHABET = "*∗†‡§‖¶?";

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
const std::unordered_set<std::string> LAST_NAME_PREFIXES = { "van", "von", "de" };

#endif  // CONSTANTS_H_