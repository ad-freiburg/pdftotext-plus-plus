/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <chrono>    // timestamp
#include <iomanip>   // setw, setfill
#include <iostream>  // cout, ostream
#include <sstream>
#include <string>
#include <utility>   // move

#include "./Log.h"

using std::move;
using std::ostream;
using std::string;
using std::stringstream;

static NullStream nullStr;

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
ostream& Logger::trace(int page) const {
  return log(LogLevel::TRACE, page);
}

// _________________________________________________________________________________________________
ostream& Logger::debug(int page) const {
  return log(LogLevel::DEBUG, page);
}

// _________________________________________________________________________________________________
ostream& Logger::info(int page) const {
  return log(LogLevel::INFO, page);
}

// _________________________________________________________________________________________________
ostream& Logger::warn(int page) const {
  return log(LogLevel::WARN, page);
}

// _________________________________________________________________________________________________
ostream& Logger::error(int page) const {
  return log(LogLevel::ERROR, page);
}

// _________________________________________________________________________________________________
ostream& Logger::log(const LogLevel& logLevel, int pageNum) const {
  // Forward the messages sent to the stream to /dev/null (= do not print the messages to the
  // console), if the given log level is smaller than _logLevel.
  if (_logLevel > logLevel) {
    return nullStr;
  }

  // Forward the messages sent to the stream to /dev/null (= do not print the messages to the
  // console) if _pageFilter is set and the given page number is set, but the page number is not
  // equal to _pageFilter.
  if (_pageFilter > 0 && pageNum > 0 && _pageFilter != pageNum) {
    return nullStr;
  }

  // Prepend the log message with the current timestamp and the name of the logging level (each
  // logging level in another color).
  stringstream logLevelStr;
  switch (logLevel) {
    case TRACE:
      logLevelStr << BOLD << MAGENTA << "TRACE:" << OFF;
      break;
    case DEBUG:
      logLevelStr << BOLD << GREEN << "DEBUG:" << OFF;
      break;
    case INFO:
      logLevelStr << BOLD << BLUE << "INFO:" << OFF;
      break;
    case WARN:
      logLevelStr << BOLD << YELLOW << "WARN:" << OFF;
      break;
    case ERROR:
      logLevelStr << BOLD << RED << "ERROR:" << OFF;
      break;
    default:
      break;
  }

  return std::cout << Logger::getTimeStamp() << "\t- " << logLevelStr.str() << " ";
}

// _________________________________________________________________________________________________
string Logger::getTimeStamp() {
  // Disclaimer: this code is stolen from https://stackoverflow.com/questions/17223096.
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

  stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X") << '.'
     << std::setw(3) << std::setfill('0') << ms.count();
  return move(ss).str();
}
