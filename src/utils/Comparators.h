/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef COMPARATORS_H_
#define COMPARATORS_H_

#include "../PdfDocument.h"

// =================================================================================================

namespace comparators {

/**
 * A comparator that can be used to sort given `PdfElement` instances by their leftX values in
 * ascending order.
 */
class LeftXAscComparator {
 public:
  bool operator() (PdfElement* e1, PdfElement* e2) {
    assert(e1);
    assert(e2);
    return e1->position->leftX < e2->position->leftX;
  }
};

/**
 * A comparator that can be used to sort given `PdfElement` instances by their rightX values in
 * ascending order.
 */
class RightXAscComparator {
 public:
  bool operator() (PdfElement* e1, PdfElement* e2) {
    assert(e1);
    assert(e2);
    return e1->position->rightX < e2->position->rightX;
  }
};

/**
 * A comparator that can be used to sort given `PdfElement` instances by their rightX values in
 * descending order.
 */
class RightXDescComparator {
 public:
  bool operator() (PdfElement* e1, PdfElement* e2) {
    assert(e1);
    assert(e2);
    return e1->position->rightX > e2->position->rightX;
  }
};

/**
 * A comparator that can be used to sort given `PdfElement` instances by their upperY values in
 * ascending order.
 */
class UpperYAscComparator {
 public:
  bool operator() (PdfElement* e1, PdfElement* e2) {
    assert(e1);
    assert(e2);
    return e1->position->upperY < e2->position->upperY;
  }
};

}  // namespace comparators


#endif  // COMPARATORS_H_