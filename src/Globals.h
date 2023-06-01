/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <memory>  // std::unique_ptr
#include <string>

class Globals;

// The global parameters object.
extern std::unique_ptr<Globals> globals;

class Globals {
 public:
  Globals();
  ~Globals();

  std::string programName;
  std::string programDescription;
  std::string programUsage;
  std::string programVersion;
  std::string semanticRolesDetectionModelsDir;
};

#endif  // GLOBALS_H_
