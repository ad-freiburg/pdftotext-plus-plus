/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>

namespace ppp {

class Config {
 public:
  Config() = default;
  ~Config() = default;

  std::string semanticRolesDetectionModelsDir;
};

}  // namespace ppp

#endif  // CONFIG_H_
