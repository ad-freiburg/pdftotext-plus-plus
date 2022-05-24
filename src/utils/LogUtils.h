/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef LOGUTILS_H_
#define LOGUTILS_H_

#include <iostream>
#include <memory>
#include <string>

// =================================================================================================

enum LogLevel { ERROR, WARN, INFO, DEBUG, TRACE };

class NullStream : public std::ostream {
 public:
  NullStream() : std::ostream(nullptr) {}
  NullStream(const NullStream &) : std::ostream(nullptr) {}
};

class Logger {
 public:
  Logger(LogLevel logLevel, int pageNum);

  void setLogLevel(LogLevel logLevel);
  void setPageFilter(int pageNum);

  std::ostream& trace(int page=-1);
  std::ostream& debug(int page=-1);
  std::ostream& info(int page=-1);
  std::ostream& warn(int page=-1);
  std::ostream& error(int page=-1);

  private:
    std::ostream& log(LogLevel logLevel, int page=-1);
    static std::string getTimeStamp();

    LogLevel _logLevel = ERROR;
    int _pageFilter = -1;
};

#endif  // LOGUTILS_H_