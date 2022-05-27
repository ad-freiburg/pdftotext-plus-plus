/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef PAGESEGMENTUTILS_H_
#define PAGESEGMENTUTILS_H_

#include "../PdfDocument.h"

using namespace std;

// =================================================================================================

namespace page_segment_utils {

tuple<double, double, double, double> computeTrimBox(const PdfPageSegment* segment);

}

#endif  // TEXTBLOCKUTILS_H_