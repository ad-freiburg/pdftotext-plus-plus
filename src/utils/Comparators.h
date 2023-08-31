/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_COMPARATORS_H_
#define UTILS_COMPARATORS_H_

#include <cassert>

#include "./MathUtils.h"
#include "../PdfDocument.h"

using ppp::types::PdfElement;
using ppp::utils::math::larger;
using ppp::utils::math::smaller;

// =================================================================================================

namespace ppp::utils::comparators {

/**
 * A comparator for sorting PDF elements by their leftX values in ascending order.
 */
class LeftXAscComparator {
 public:
  bool operator() (const PdfElement* e1, const PdfElement* e2) const {
    assert(e1);
    assert(e2);
    assert(e1->pos);
    assert(e2->pos);

    return smaller(e1->pos->leftX, e2->pos->leftX);
  }
};

/**
 * A comparator for sorting PDF elements by their rightX values in descending order.
 */
class RightXDescComparator {
 public:
  bool operator() (const PdfElement* e1, const PdfElement* e2) const {
    assert(e1);
    assert(e2);
    assert(e1->pos);
    assert(e2->pos);

    return larger(e1->pos->rightX, e2->pos->rightX);
  }
};

/**
 * A comparator for sorting PDF elements by their upperY values in ascending order.
 */
class UpperYAscComparator {
 public:
  bool operator() (const PdfElement* e1, const PdfElement* e2) const {
    assert(e1);
    assert(e2);
    assert(e1->pos);
    assert(e2->pos);

    return smaller(e1->pos->upperY, e2->pos->upperY);
  }
};

/**
 * A comparator for sorting PDF elements by their rotLeftX values in ascending order.
 */
class RotLeftXAscComparator {
 public:
  bool operator() (const PdfElement* e1, const PdfElement* e2) const {
    assert(e1);
    assert(e2);
    assert(e1->pos);
    assert(e2->pos);

    return smaller(e1->pos->getRotLeftX(), e2->pos->getRotLeftX());
  }
};

/**
 * A comparator for sorting PDF elements by their rotLeftX values in descending order.
 */
class RotLeftXDescComparator {
 public:
  bool operator() (const PdfElement* e1, const PdfElement* e2) const {
    assert(e1);
    assert(e2);
    assert(e1->pos);
    assert(e2->pos);

    return larger(e1->pos->getRotLeftX(), e2->pos->getRotLeftX());
  }
};

/**
 * A comparator for sorting PDF elements by their rotLowerY values in ascending order.
 */
class RotLowerYAscComparator {
 public:
  bool operator() (const PdfElement* e1, const PdfElement* e2) const {
    assert(e1);
    assert(e2);
    assert(e1->pos);
    assert(e2->pos);

    return smaller(e1->pos->getRotLowerY(), e2->pos->getRotLowerY());
  }
};

/**
 * A comparator for sorting PDF elements by their rotLowerY values in descending order.
 */
class RotLowerYDescComparator {
 public:
  bool operator() (const PdfElement* e1, const PdfElement* e2) const {
    assert(e1);
    assert(e2);
    assert(e1->pos);
    assert(e2->pos);

    return larger(e1->pos->getRotLowerY(), e2->pos->getRotLowerY());
  }
};

}  // namespace ppp::utils::comparators

#endif  // UTILS_COMPARATORS_H_
