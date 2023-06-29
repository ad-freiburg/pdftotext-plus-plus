/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <chrono>    // std::chrono::system_clock
#include <iomanip>   // std::setw, std::setfill
#include <iostream>  // std::cout, std::ostream
#include <sstream>   // std::stringstream
#include <string>
#include <utility>   // std::move

#include "./Log.h"

using std::cout;
using std::move;
using std::ostream;
using std::string;
using std::stringstream;

// =================================================================================================

namespace ppp::utils::log {

static NullStream _nullStream;

// _________________________________________________________________________________________________
Logger::Logger(const LogLevel& logLevel, int pageFilter) {
  _logLevel = logLevel;
  _pageFilter = pageFilter;
}

// _________________________________________________________________________________________________
void Logger::setLogLevel(const LogLevel& logLevel) {
  _logLevel = logLevel;
}

// _________________________________________________________________________________________________
void Logger::setPageFilter(int pageFilter) {
  _pageFilter = pageFilter;
}

// _________________________________________________________________________________________________
ostream& Logger::trace(int pageNum) const {
  return getostream(LogLevel::TRACE, pageNum) << createLogMessagePrefix(LogLevel::TRACE);
}

// _________________________________________________________________________________________________
ostream& Logger::debug(int pageNum) const {
  return getostream(LogLevel::DEBUG, pageNum) << createLogMessagePrefix(LogLevel::DEBUG);
}

// _________________________________________________________________________________________________
ostream& Logger::info(int pageNum) const {
  return getostream(LogLevel::INFO, pageNum) << createLogMessagePrefix(LogLevel::INFO);
}

// _________________________________________________________________________________________________
ostream& Logger::warn(int pageNum) const {
  return getostream(LogLevel::WARN, pageNum) << createLogMessagePrefix(LogLevel::WARN);
}

// _________________________________________________________________________________________________
ostream& Logger::error(int pageNum) const {
  return getostream(LogLevel::ERROR, pageNum) << createLogMessagePrefix(LogLevel::ERROR);
}

// _________________________________________________________________________________________________
ostream& Logger::getostream(const LogLevel& logLevel, int pageNum) const {
  // If the given log level is smaller than _logLevel, forward the messages to /dev/null (= do not
  // print the messages to the console),
  if (_logLevel > logLevel) {
    return _nullStream;
  }

  // If _pageFilter is set and the given page number is set, but the page number is not
  // equal to _pageFilter, forward the messages sent to the stream to /dev/null (= do not print
  // the messages to the console) .
  if (_pageFilter > 0 && pageNum > 0 && _pageFilter != pageNum) {
    return _nullStream;
  }

  return cout;
}

// _________________________________________________________________________________________________
string Logger::createLogMessagePrefix(const LogLevel& logLevel) const {
  stringstream logLevelStr;

  switch (logLevel) {
    case TRACE:
      logLevelStr << BOLD << MAGENTA << "TRACE:" << OFF;
      break;
    case DEBUG:
      logLevelStr << BOLD << GREEN << "DEBUG:" << OFF;
      break;
    case INFO:
      logLevelStr << BOLD << BLUE << "INFO: " << OFF;
      break;
    case WARN:
      logLevelStr << BOLD << YELLOW << "WARN: " << OFF;
      break;
    case ERROR:
      logLevelStr << BOLD << RED << "ERROR:" << OFF;
      break;
    default:
      break;
  }

  return getTimeStamp() + "\t- " + logLevelStr.str() + " ";
}

// _________________________________________________________________________________________________
string Logger::getTimeStamp() {
  // Disclaimer: The following code is stolen from https://stackoverflow.com/questions/17223096.
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

  stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X") << '.'
     << std::setw(3) << std::setfill('0') << ms.count();

  return move(ss).str();
}

}  // namespace ppp::utils::log
