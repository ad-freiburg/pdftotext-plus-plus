/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TEXTBLOCKUTILS_H_
#define TEXTBLOCKUTILS_H_

#include "../PdfDocument.h"

using namespace std;

// =================================================================================================

const unordered_set<string> LAST_NAME_PREFIXES = { "van", "von", "de" };
const std::string FORMULA_ID_ALPHABET = "=+";

namespace text_block_utils {

bool computeIsCentered(const PdfTextBlock* block);
double computeHangingIndent(const PdfTextBlock* block);
void computeTextLineMargins(const PdfTextBlock* block);

}

#endif  // TEXTBLOCKUTILS_H_