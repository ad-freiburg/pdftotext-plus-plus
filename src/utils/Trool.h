/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_TROOL_H_
#define UTILS_TROOL_H_

/**
 * A tri-state "boolean", with values: true, false and "not set". This is required by, for example,
 * the text block detector to express the following three possible states:
 *   (a) the current text line starts a new text block;
 *   (b) the current text line does not start a new text block;
 *   (c) it can't be decided whether the text line starts a new text block.
 */
enum Trool { None = -1, False, True };

#endif  // UTILS_TROOL_H_
