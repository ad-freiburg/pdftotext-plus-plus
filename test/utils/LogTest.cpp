/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <iostream>  // std::cout
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

// TODO(korzen): Why is the namespace needed here?
namespace ppp::utils::log {

static const regex tsRegex("\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}.\\d{3}");
static const regex prefixRegexInfo("\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}.\\d{3}\t-.*INFO.*");
static const regex prefixRegexWarn("\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}.\\d{3}\t-.*WARN.*");
static const regex prefixRegexError("\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}.\\d{3}\t-.*ERROR.*");

// _________________________________________________________________________________________________
TEST(LogTest, constructor) {
  Logger log(LogLevel::DEBUG);
  ASSERT_EQ(log._logLevel, LogLevel::DEBUG);
  ASSERT_EQ(log._pageFilter, -1);

  Logger log2(LogLevel::WARN, 3);
  ASSERT_EQ(log2._logLevel, LogLevel::WARN);
  ASSERT_EQ(log2._pageFilter, 3);
}

// _________________________________________________________________________________________________
TEST(LogTest, setLogLevel) {
  Logger log(LogLevel::DEBUG);
  ASSERT_EQ(log._logLevel, LogLevel::DEBUG);

  log.setLogLevel(LogLevel::ERROR);
  ASSERT_EQ(log._logLevel, LogLevel::ERROR);

  log.setLogLevel(LogLevel::TRACE);
  ASSERT_EQ(log._logLevel, LogLevel::TRACE);
}

// _________________________________________________________________________________________________
TEST(LogTest, setPageFilter) {
  Logger log(LogLevel::DEBUG);
  ASSERT_EQ(log._pageFilter, -1);

  log.setPageFilter(2);
  ASSERT_EQ(log._pageFilter, 2);

  log.setPageFilter(5);
  ASSERT_EQ(log._pageFilter, 5);
}

// _________________________________________________________________________________________________
TEST(LogTest, getostream) {
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
TEST(LogTest, createLogMessagePrefix) {
  Logger logger(INFO);

  // Check if the returned string has format "2023-06-29 15:04:53.856 - <LEVEL>".
  smatch matchInfo;
  string prefixInfo = logger.createLogMessagePrefix(INFO);
  ASSERT_TRUE(regex_match(prefixInfo, matchInfo, prefixRegexInfo));

  smatch matchWarn;
  string prefixWarn = logger.createLogMessagePrefix(WARN);
  ASSERT_TRUE(regex_match(prefixWarn, matchWarn, prefixRegexWarn));

  smatch matchError;
  string prefixError = logger.createLogMessagePrefix(ERROR);
  ASSERT_TRUE(regex_match(prefixError, matchError, prefixRegexError));
}

// _________________________________________________________________________________________________
TEST(LogTest, getTimeStamp) {
  // Check if the returned string has format "2023-06-29 15:04:53.856".
  smatch match;
  string ts = Logger::getTimeStamp();
  ASSERT_TRUE(regex_match(ts, match, tsRegex));
}

}  // namespace ppp::utils::log
