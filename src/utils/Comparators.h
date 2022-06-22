/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_COMPARATORS_H_
#define UTILS_COMPARATORS_H_

#include "../PdfDocument.h"

// =================================================================================================

namespace comparators {

/**
 * A comparator that can be used to sort given PDF elements by their leftX values in ascending
 * order.
 */
class LeftXAscComparator {
 public:
  bool operator() (const PdfElement* e1, const PdfElement* e2) const {
    assert(e1);
    assert(e2);
    return e1->position->leftX < e2->position->leftX;
  }
};

/**
 * A comparator that can be used to sort given PDF elements by their getRotLeftX() values in
 * ascending order.
 */
class RotLeftXAscComparator {
 public:
  bool operator() (const PdfElement* e1, const PdfElement* e2) const {
    assert(e1);
    assert(e2);
    return e1->position->getRotLeftX() < e2->position->getRotLeftX();
  }
};

/**
 * A comparator that can be used to sort given PDF elements by their getRotLeftX() values in
 * descending order.
 */
class RotLeftXDescComparator {
 public:
  bool operator() (const PdfElement* e1, const PdfElement* e2) const {
    assert(e1);
    assert(e2);
    return e1->position->getRotLeftX() > e2->position->getRotLeftX();
  }
};

/**
 * A comparator that can be used to sort given PDF elements by their rightX values in ascending
 * order.
 */
class RightXAscComparator {
 public:
  bool operator() (const PdfElement* e1, const PdfElement* e2) const {
    assert(e1);
    assert(e2);
    return e1->position->rightX < e2->position->rightX;
  }
};

/**
 * A comparator that can be used to sort given PDF elements by their rightX values in descending
 * order.
 */
class RightXDescComparator {
 public:
  bool operator() (const PdfElement* e1, const PdfElement* e2) const {
    assert(e1);
    assert(e2);
    return e1->position->rightX > e2->position->rightX;
  }
};

/**
 * A comparator that can be used to sort given PDF elements by their upperY values in ascending
 * order.
 */
class UpperYAscComparator {
 public:
  bool operator() (const PdfElement* e1, const PdfElement* e2) const {
    assert(e1);
    assert(e2);
    return e1->position->upperY < e2->position->upperY;
  }
};

/**
 * A comparator that can be used to sort given PDF elements by their getRotLowerY() values in
 * ascending order.
 */
class RotLowerYAscComparator {
 public:
  bool operator() (const PdfElement* e1, const PdfElement* e2) const {
    assert(e1);
    assert(e2);
    return e1->position->getRotLowerY() < e2->position->getRotLowerY();
  }
};

/**
 * A comparator that can be used to sort given PDF elements by their getRotLowerY() values in
 * descending order.
 */
class RotLowerYDescComparator {
 public:
  bool operator() (const PdfElement* e1, const PdfElement* e2) const {
    assert(e1);
    assert(e2);
    return e1->position->getRotLowerY() > e2->position->getRotLowerY();
  }
};

}  // namespace comparators

#endif  // UTILS_COMPARATORS_H_
