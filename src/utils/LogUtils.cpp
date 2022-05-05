/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "./LogUtils.h"

static NullStream nullStr;

// _________________________________________________________________________________________________
Logger::Logger(LogLevel logLevel, int pageFilter) {
  _logLevel = logLevel;
  _pageFilter = pageFilter;
}

// _________________________________________________________________________________________________
void Logger::setLogLevel(LogLevel logLevel) {
  _logLevel = logLevel;
}

// _________________________________________________________________________________________________
void Logger::setPageFilter(int pageFilter) {
  _pageFilter = pageFilter;
}

// _________________________________________________________________________________________________
std::ostream& Logger::debug(int page) {
  return log(LogLevel::DEBUG, page);
}

// _________________________________________________________________________________________________
std::ostream& Logger::info(int page) {
  return log(LogLevel::INFO, page);
}

// _________________________________________________________________________________________________
std::ostream& Logger::warn(int page) {
  return log(LogLevel::WARN, page);
}

// _________________________________________________________________________________________________
std::ostream& Logger::error(int page) {
  return log(LogLevel::ERROR, page);
}

// _________________________________________________________________________________________________
std::ostream& Logger::log(LogLevel logLevel, int page) {
  if (_logLevel < logLevel) {
    return nullStr;
  }

  if (_pageFilter > 0 && page > 0 && _pageFilter != page) {
    return nullStr;
  }

  std::string logLevelStr;
  switch (logLevel) {
    case DEBUG:
      logLevelStr = "\033[32;1mDEBUG:\033[0m"; // green + bold
      break;
    case INFO:
      logLevelStr = "\033[34;1mINFO: \033[0m";  // blue + bold
      break;
    case WARN:
      logLevelStr = "\033[33;1mWARN: \033[0m";  // yellow + bold
      break;
    case ERROR:
      logLevelStr = "\033[31;1mERROR:\033[0m";  // red + bold
      break;
    default:
      break;
  }

  return std::cout << Logger::getTimeStamp() << "\t- " << logLevelStr << " ";
}

// _________________________________________________________________________________________________
std::string Logger::getTimeStamp() {
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

  std::stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X") << '.'
     << std::setw(3) << std::setfill('0') << ms.count();
  return std::move(ss).str();
}