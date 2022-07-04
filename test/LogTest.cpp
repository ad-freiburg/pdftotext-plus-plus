/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include "../src/utils/Log.h"

// _________________________________________________________________________________________________
TEST(Log, constructor) {
  Logger log(LogLevel::DEBUG);
  ASSERT_EQ(log._logLevel, LogLevel::DEBUG);
  ASSERT_EQ(log._pageFilter, -1);

  Logger log2(LogLevel::WARN, 1);
  ASSERT_EQ(log2._logLevel, LogLevel::WARN);
  ASSERT_EQ(log2._pageFilter, 1);

  log2.setLogLevel(LogLevel::ERROR);
  log2.setPageFilter(3);
  ASSERT_EQ(log2._logLevel, LogLevel::ERROR);
  ASSERT_EQ(log2._pageFilter, 3);
}
