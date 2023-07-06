/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_LOG_H_
#define UTILS_LOG_H_

#include <iostream>  // std::ostream
#include <string>

using std::ostream;
using std::string;

// =================================================================================================

namespace ppp::utils::log {

// Some ANSI codes to print text in colors or in bold. For example, to print text in blue, you can
// type: 'cout << BLUE << "Hello World" << OFF << endl;'. To print text in bold
// *and* blue you can type: 'cout << BOLD << BLUE << "Hello World" << OFF << endl;' or
// 'cout << BBOLD << "Hello World" << OFF << endl;'.
const char* const BOLD = "\033[1m";
const char* const RED = "\033[31m";
const char* const GREEN = "\033[32m";
const char* const YELLOW = "\033[33m";
const char* const BLUE = "\033[34m";
const char* const BBLUE = "\033[1;34m";
const char* const MAGENTA = "\033[35m";
const char* const GRAY = "\033[90m";
const char* const OFF = "\033[0m";

// =================================================================================================

/**
 * The available log levels, ordered by their severity (in ascending order).
 */
enum LogLevel { TRACE, DEBUG, INFO, WARN, ERROR };

// =================================================================================================

/**
 * An output stream that acts like /dev/null, meaning that any text forwarded to this stream will
 * be ignored (it does not appear on the console). This stream is used by the 'Logger' class for
 * filtering out messages that do not match the current log level filter or page filter.
 *
 * Disclaimer: The code of this class is stolen from: https://stackoverflow.com/questions/8243743.
 */
class NullStream : public ostream {
 public:
  /** The default constructor. */
  NullStream() : ostream(nullptr) {}
  /** The copy constructor. */
  NullStream(const NullStream&) : ostream(nullptr) {}
};

// =================================================================================================

/**
 * A custom logger for printing logging messages with different severity levels to the console.
 */
class Logger {
 public:
  /**
   * The default constructor.
   *
   * @param logLevel
   *    The lowest level of log messages this logger should print to the console. The order of log
   *    levels is as follows (ordered by their severity, from low to high): TRACE, DEBUG, INFO,
   *    WARN, ERROR. For example, if the log level is specified as INFO, the logger will only
   *    print messages of level INFO, WARN, and ERROR to the console, and will ignore messages of
   *    level TRACE and DEBUG.
   * @param pageFilter
   *    The page filter. Log message can be associated with a page number, with the purpose to
   *    specify that the message was produced while processing the respective page. If specified
   *    with a value > 0, only those message that are associated with the <pageFilter>-th page will
   *    be printed to the console. All messages that are associated with other pages will be
   *    ignored. If specified by a value <= 0, all messages will be printed to the console, no
   *    matter with which pages the messages are associated.
   */
  explicit Logger(const LogLevel& logLevel, int pageFilter = -1);

  /**
   * This method sets the log level of this logger, specifying the lowest level of log messages
   * this logger should print to the console. See the comment of the constructor for more
   * information.
   *
   * @param logLevel
   *    The logging level.
   */
  void setLogLevel(const LogLevel& logLevel);

  /**
   * This method sets the page filter of this logger. See the comment of the constructor for more
   * information about page filters.
   *
   * @param pageNum
   *    The page filter.
   */
  void setPageFilter(int pageNum);

  /**
   * This method returns the stream to use for outputting logging messages of log level TRACE.
   * The usage is as follows: 'log->trace() << "This is a message" << endl;'. The output is:
   * '2022-06-02 10:49:00.990 - TRACE:  This is a message'.
   *
   * NOTE: In general, this method returns std::cout. It may return also the null stream however,
   * depending on the current values of _logLevel and _pageFilter (see the comment of the
   * constructor for more information). The null stream is equivalent to /dev/null, meaning that
   * logging messages forwarded to this stream will be ignored (they do not appear anywhere).
   *
   * @param pageNum
   *    A page number, indicating that the message sent to the stream was produced while processing
   *    the <pageNum>-th page of the current document. This is needed by a feature of this logger
   *    that allows to print only those messages to the console that were produced while processing
   *    a specific page, see the comment of the constructor for more information.
   *
   * @return
   *    The stream to use for outputting logging messages of log level TRACE.
   */
  ostream& trace(int pageNum = -1) const;

  /**
   * This method returns the stream to use for outputting logging messages of log level DEBUG.
   * The usage is as follows: 'log->debug() << "This is a message" << endl;'. The output is:
   * '2022-06-02 10:49:00.990 - DEBUG:  This is a message'.
   *
   * NOTE: In general, this method returns std::cout. It may return also the null stream however,
   * depending on the current values of _logLevel and _pageFilter (see the comment of the
   * constructor for more information). The null stream is equivalent to /dev/null, meaning that
   * logging messages forwarded to this stream will be ignored (they do not appear anywhere).
   *
   * @param pageNum
   *    A page number, indicating that the message sent to the stream was produced while processing
   *    the <pageNum>-th page of the current document. This is needed by a feature of this logger
   *    that allows to print only those messages to the console that were produced while processing
   *    a specific page, see the comment of the constructor for more information.
   *
   * @return
   *    The stream to use for outputting logging messages of log level DEBUG.
   */
  ostream& debug(int pageNum = -1) const;

  /**
   * This method returns the stream to use for outputting logging messages of log level INFO.
   * The usage is as follows: 'log->info() << "This is a message" << endl;'. The output is:
   * '2022-06-02 10:49:00.990 - INFO:  This is a message'.
   *
   * NOTE: In general, this method returns std::cout. It may return also the null stream however,
   * depending on the current values of _logLevel and _pageFilter (see the comment of the
   * constructor for more information). The null stream is equivalent to /dev/null, meaning that
   * logging messages forwarded to this stream will be ignored (they do not appear anywhere).
   *
   * @param pageNum
   *    A page number, indicating that the message sent to the stream was produced while processing
   *    the <pageNum>-th page of the current document. This is needed by a feature of this logger
   *    that allows to print only those messages to the console that were produced while processing
   *    a specific page, see the comment of the constructor for more information.
   *
   * @return
   *    The stream to use for outputting logging messages of log level INFO.
   */
  ostream& info(int pageNum = -1) const;

  /**
   * This method returns the stream to use for outputting logging messages of log level WARN.
   * The usage is as follows: 'log->warn() << "This is a message" << endl;'. The output is:
   * '2022-06-02 10:49:00.990 - WARN:  This is a message'.
   *
   * NOTE: In general, this method returns std::cout. It may return also the null stream however,
   * depending on the current values of _logLevel and _pageFilter (see the comment of the
   * constructor for more information). The null stream is equivalent to /dev/null, meaning that
   * logging messages forwarded to this stream will be ignored (they do not appear anywhere).
   *
   * @param pageNum
   *    A page number, indicating that the message sent to the stream was produced while processing
   *    the <pageNum>-th page of the current document. This is needed by a feature of this logger
   *    that allows to print only those messages to the console that were produced while processing
   *    a specific page, see the comment of the constructor for more information.
   *
   * @return
   *    The stream to use for outputting logging messages of log level WARN.
   */
  ostream& warn(int pageNum = -1) const;

  /**
   * This method returns the stream to use for outputting logging messages of log level ERROR.
   * The usage is as follows: 'log->error() << "This is a message" << endl;'. The output is:
   * '2022-06-02 10:49:00.990 - ERROR:  This is a message'.
   *
   * NOTE: In general, this method returns std::cout. It may return also the null stream however,
   * depending on the current values of _logLevel and _pageFilter (see the comment of the
   * constructor for more information). The null stream is equivalent to /dev/null, meaning that
   * logging messages forwarded to this stream will be ignored (they do not appear anywhere).
   *
   * @param pageNum
   *    A page number, indicating that the message sent to the stream was produced while processing
   *    the <pageNum>-th page of the current document. This is needed by a feature of this logger
   *    that allows to print only those messages to the console that were produced while processing
   *    a specific page, see the comment of the constructor for more information.
   *
   * @return
   *    The stream to use for outputting logging messages of log level ERROR.
   */
  ostream& error(int pageNum = -1) const;

 private:
  /**
   * This method returns the output stream to which a logging message related to the given log
   * level and page number should be forwarded.
   *
   * If (1) the given log level is smaller than _logLevel or (2) if _pageFilter is set and the
   * given page number is set, but the page number is not equal to _pageFilter, this method returns
   * the null stream (logging messages forwarded to this stream will be ignored, meaning that they
   * don't appear on stdout). Otherwise, the method returns std::cout (logging messages forwarded
   * to this stream appear on stdout).
   *
   * @param logLevel
   *    The logging level.
   * @param pageNum
   *    A page number, indicating that the logging message was created while processing the
   *    <pageNum>-th page of the current PDF document.
   *
   * @return
   *    An output stream to which a log message associated with the given log level and page number
   *    should be forwarded.
   */
  ostream& getostream(const LogLevel& logLevel, int pageNum = -1) const;

  /**
   * This method returns the string to prepend to each log message associated with the given
   * logging level. The returned string contains the current timestamp and the given logging level.
   * Here is an example: "2022-06-02 10:49:00.990 - ERROR:"
   *
   * @return
   *    The string to prepend to each log message associated with the given logging level.
   */
  string createLogMessagePrefix(const LogLevel& logLevel) const;

  /**
   * This method returns the current timestamp as a human-readable string, for example:
   * "2022-06-02 10:49:00.990".
   *
   * @return
   *    The current time stamp as a human-readable string.
   */
  static string getTimeStamp();

  // The logging level.
  LogLevel _logLevel = ERROR;
  // The page filter.
  int _pageFilter = -1;

  friend class LogTest_constructor_Test;
  friend class LogTest_setLogLevel_Test;
  friend class LogTest_setPageFilter_Test;
  friend class LogTest_getostream_Test;
  friend class LogTest_createLogMessagePrefix_Test;
  friend class LogTest_getTimeStamp_Test;
};

}  // namespace ppp::utils::log

#endif  // UTILS_LOG_H_
