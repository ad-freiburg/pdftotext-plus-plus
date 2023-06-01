/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <memory>  // std::unique_ptr

#include "./Globals.h"

std::unique_ptr<Globals> globals;

// _________________________________________________________________________________________________
Globals::Globals() = default;

// _________________________________________________________________________________________________
Globals::~Globals() = default;
