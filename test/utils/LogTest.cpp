/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <regex>
#include <string>

#include "../../src/utils/Log.h"

using std::cout;
using std::regex;
using std::regex_match;
using std::smatch;
using std::string;

using ppp::utils::log::Logger;
using ppp::utils::log::LogLevel;

// =================================================================================================

namespace ppp::utils::log {

// _________________________________________________________________________________________________
TEST(Log, constructor) {
  Logger log(LogLevel::DEBUG);
  ASSERT_EQ(log._logLevel, LogLevel::DEBUG);
  ASSERT_EQ(log._pageFilter, -1);

  Logger log2(LogLevel::WARN, 3);
  ASSERT_EQ(log2._logLevel, LogLevel::WARN);
  ASSERT_EQ(log2._pageFilter, 3);
}

// _________________________________________________________________________________________________
TEST(Log, setLogLevel) {
  Logger log(LogLevel::DEBUG);
  ASSERT_EQ(log._logLevel, LogLevel::DEBUG);

  log.setLogLevel(LogLevel::ERROR);
  ASSERT_EQ(log._logLevel, LogLevel::ERROR);

  log.setLogLevel(LogLevel::TRACE);
  ASSERT_EQ(log._logLevel, LogLevel::TRACE);
}

// _________________________________________________________________________________________________
TEST(Log, setPageFilter) {
  Logger log(LogLevel::DEBUG);
  ASSERT_EQ(log._pageFilter, -1);

  log.setPageFilter(2);
  ASSERT_EQ(log._pageFilter, 2);

  log.setPageFilter(5);
  ASSERT_EQ(log._pageFilter, 5);
}

// _________________________________________________________________________________________________
TEST(Log, getostream) {
  // The rdbuf() method is used to compare the returned output stream with std::cout, see
  // https://stackoverflow.com/questions/3318714/.
  Logger logger(TRACE);
  ASSERT_EQ(logger.getostream(TRACE).rdbuf(), cout.rdbuf());
  ASSERT_EQ(logger.getostream(DEBUG).rdbuf(), cout.rdbuf());
  ASSERT_EQ(logger.getostream(INFO).rdbuf(), cout.rdbuf());
  ASSERT_EQ(logger.getostream(WARN).rdbuf(), cout.rdbuf());
  ASSERT_EQ(logger.getostream(ERROR).rdbuf(), cout.rdbuf());

  logger.setLogLevel(DEBUG);
  ASSERT_EQ(logger.getostream(TRACE).rdbuf(), nullptr);
  ASSERT_EQ(logger.getostream(DEBUG).rdbuf(), cout.rdbuf());
  ASSERT_EQ(logger.getostream(INFO).rdbuf(), cout.rdbuf());
  ASSERT_EQ(logger.getostream(WARN).rdbuf(), cout.rdbuf());
  ASSERT_EQ(logger.getostream(ERROR).rdbuf(), cout.rdbuf());

  logger.setLogLevel(INFO);
  ASSERT_EQ(logger.getostream(TRACE).rdbuf(), nullptr);
  ASSERT_EQ(logger.getostream(DEBUG).rdbuf(), nullptr);
  ASSERT_EQ(logger.getostream(INFO).rdbuf(), cout.rdbuf());
  ASSERT_EQ(logger.getostream(WARN).rdbuf(), cout.rdbuf());
  ASSERT_EQ(logger.getostream(ERROR).rdbuf(), cout.rdbuf());

  logger.setLogLevel(WARN);
  ASSERT_EQ(logger.getostream(TRACE).rdbuf(), nullptr);
  ASSERT_EQ(logger.getostream(DEBUG).rdbuf(), nullptr);
  ASSERT_EQ(logger.getostream(INFO).rdbuf(), nullptr);
  ASSERT_EQ(logger.getostream(WARN).rdbuf(), cout.rdbuf());
  ASSERT_EQ(logger.getostream(ERROR).rdbuf(), cout.rdbuf());

  logger.setLogLevel(ERROR);
  ASSERT_EQ(logger.getostream(TRACE).rdbuf(), nullptr);
  ASSERT_EQ(logger.getostream(DEBUG).rdbuf(), nullptr);
  ASSERT_EQ(logger.getostream(INFO).rdbuf(), nullptr);
  ASSERT_EQ(logger.getostream(WARN).rdbuf(), nullptr);
  ASSERT_EQ(logger.getostream(ERROR).rdbuf(), cout.rdbuf());

  logger.setLogLevel(INFO);
  logger.setPageFilter(3);
  ASSERT_EQ(logger.getostream(INFO, 1).rdbuf(), nullptr);
  ASSERT_EQ(logger.getostream(INFO, 2).rdbuf(), nullptr);
  ASSERT_EQ(logger.getostream(INFO, 3).rdbuf(), cout.rdbuf());
  ASSERT_EQ(logger.getostream(INFO, 4).rdbuf(), nullptr);
  ASSERT_EQ(logger.getostream(INFO, 5).rdbuf(), nullptr);
}

// _________________________________________________________________________________________________
TEST(Log, createLogMessagePrefix) {
  Logger logger(INFO);

  // Check if the string returned by formatLogPrefix has format "2023-06-29 15:04:53.856 - <LEVEL>".
  smatch match1;
  string prefix1 = logger.createLogMessagePrefix(INFO);
  const regex logPrefixRegex1("\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}.\\d{3}\t-.*INFO.*");
  ASSERT_TRUE(regex_match(prefix1, match1, logPrefixRegex1));

  smatch match2;
  string prefix2 = logger.createLogMessagePrefix(WARN);
  const regex logPrefixRegex2("\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}.\\d{3}\t-.*WARN.*");
  ASSERT_TRUE(regex_match(prefix2, match2, logPrefixRegex2));

  smatch match3;
  string prefix3 = logger.createLogMessagePrefix(ERROR);
  const regex logPrefixRegex3("\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}.\\d{3}\t-.*ERROR.*");
  ASSERT_TRUE(regex_match(prefix3, match3, logPrefixRegex3));
}

// _________________________________________________________________________________________________
TEST(Log, getTimeStamp) {
  // Check if the string returned by getTimeStamp has format "2023-06-29 15:04:53.856".
  smatch match1;
  string ts = Logger::getTimeStamp();
  const regex logPrefixRegex1("\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}.\\d{3}");
  ASSERT_TRUE(regex_match(ts, match1, logPrefixRegex1));
}

}  // namespace ppp::utils::log
