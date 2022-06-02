/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TROOL_H_
#define TROOL_H_

/**
 * A tri-state "boolean", with values "true", "false" and "not set".
 */
enum Trool { None = -1, False, True };

#endif  // TROOL_H_