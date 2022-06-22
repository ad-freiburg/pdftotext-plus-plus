/**
 * Copyright 2022, University of Freiburg,
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

// Some ANSI codes to print text in colors or in bold to the console. For example, to print text in
// bold, you can type: 'cout << BLUE << "Hello World" << OFF << endl;'. To print text in bold
// *and* blue you can type: 'cout << BOLD << BLUE << "Hello World" << OFF << endl;'.
const char* const BOLD = "\033[1m";
const char* const RED = "\033[31m";
const char* const GREEN = "\033[32m";
const char* const YELLOW = "\033[33m";
const char* const BLUE = "\033[34m";
const char* const MAGENTA = "\033[35m";
const char* const OFF = "\033[0m";

// =================================================================================================

/**
 * The available log levels, ordered by their severity.
 */
enum LogLevel { TRACE, DEBUG, INFO, WARN, ERROR };

// =================================================================================================

/**
 * An output stream that acts like /dev/null, meaning that any text that is streamed to it will be
 * ignored (that is: will be not printed to the console). This stream is used by the 'Logger' class
 * for filtering out messages that do not match the current log level filter or page filter.
 *
 * The code of this class is stolen from: https://stackoverflow.com/questions/8243743.
 */
class NullStream : public ostream {
 public:
  /** The default constructor. */
  NullStream() : ostream(nullptr) {}
  /** The copy constructor. */
  NullStream(const NullStream &) : ostream(nullptr) {}
};

// =================================================================================================

/**
 * A custom logger for printing logging messages with different severity levels to the console.
 */
class Logger {
 public:
  /**
   * This constructor creates and initializes a new instance of the 'Logger' class.
   *
   * @param logLevel
   *    The lowest level of log messages this logger should print to the console. The order of log
   *    levels is as follows (ordered by their severity, from low to high): TRACE, DEBUG, INFO,
   *    WARN, ERROR. For example, if the log level is specified as INFO, the logger will only
   *    print messages of level INFO, WARN, and ERROR to the console, and will ignore messages of
   *    level TRACE and DEBUG.
   * @param pageFilter
   *    The page filter. This logger allows to associate a log message with a page number of the
   *    current document (see the logging methods below), meaning that the message was produced
   *    while processing the respective page. If specified by a value > 0, this logger prints only
   *    those message to the console that are associated with the <pageFilter>-th page and will
   *    ignore messages that are associated with other pages. If specified by a value <= 0, this
   *    logger prints all messages to the console, no matter with which pages the messages are
   *    associated.
   */
  Logger(const LogLevel& logLevel, int pageFilter);

  /**
   * This method sets the log level of this logger.
   *
   * @param logLevel
   *    The lowest level of log messages this logger should print to the console. This parameter
   *    is already described in the comment of the constructor, so see this comment for more
   *    details about this parameter.
   */
  void setLogLevel(const LogLevel& logLevel);

  /**
   * This method sets the page filter of this logger.
   *
   * @param pageNum
   *    The page filter. This parameter is already described in the comment of the constructor, so
   *    see this comment for more details about this parameter.
   */
  void setPageFilter(int pageNum);

  /**
   * This method returns a stream which prints the received messages with log level TRACE to the
   * console. The usage is as follows: 'log->trace() << "This is a message" << endl;', which
   * outputs: '2022-06-02 10:49:00.990 - TRACE:  This is a message'.
   *
   * @param pageNum
   *    A page number, indicating that the message sent to the stream was produced while processing
   *    the <pageNum>-th page of the current document. This is needed by the feature of this logger
   *    that allows to print only those messages to the console that were produced while processing
   *    a specific page, see the comment given for the constructor for more information.
   *
   * @return
   *    A stream which prints the received messages with log level TRACE to the console.
   */
  ostream& trace(int pageNum = -1) const;

  /**
   * This method returns a stream which prints the received messages with log level DEBUG to the
   * console. The usage is as follows: 'log->debug() << "This is a message" << endl;', which
   * outputs: '2022-06-02 10:49:00.990 - DEBUG: This is a message'.
   *
   * @param pageNum
   *    A page number, indicating that the message sent to the stream was produced while processing
   *    the <pageNum>-th page of the current document. This is needed by the feature of this logger
   *    that allows to print only those messages to the console that were produced while processing
   *    a specific page, see the comment given for the constructor for more information.
   *
   * @return
   *    A stream which prints the received messages with log level DEBUG to the console.
   */
  ostream& debug(int pageNum = -1) const;

  /**
   * This method returns a stream which prints the received messages with log level INFO to the
   * console. The usage is as follows: 'log->info() << "This is a message" << endl;', which
   * outputs: '2022-06-02 10:49:00.990 - INFO: This is a message'.
   *
   * @param pageNum
   *    A page number, indicating that the message sent to the stream was produced while processing
   *    the <pageNum>-th page of the current document. This is needed by the feature of this logger
   *    that allows to print only those messages to the console that were produced while processing
   *    a specific page, see the comment given for the constructor for more information.
   *
   * @return
   *    A stream which prints the received messages with log level INFO to the console.
   */
  ostream& info(int pageNum = -1) const;

  /**
   * This method returns a stream which prints the received messages with log level WARN to the
   * console. The usage is as follows: 'log->warn() << "This is a message" << endl;', which
   * outputs: '2022-06-02 10:49:00.990 - WARN:  This is a message'.
   *
   * @param pageNum
   *    A page number, indicating that the message sent to the stream was produced while processing
   *    the <pageNum>-th page of the current document. This is needed by the feature of this logger
   *    that allows to print only those messages to the console that were produced while processing
   *    a specific page, see the comment given for the constructor for more information.
   *
   * @return
   *    A stream which prints the received messages with log level WARN to the console.
   */
  ostream& warn(int pageNum = -1) const;

  /**
   * This method returns a stream which prints the received messages with log level ERROR to the
   * console. Thes usage is as follows: 'log->error() << "This is a message" << endl;', which
   * outputs: '2022-06-02 10:49:00.990 - ERROR:  This is a message'.
   *
   * @param pageNum
   *    A page number, indicating that the message sent to the stream was produced while processing
   *    the <pageNum>-th page of the current document. This is needed by the feature of this logger
   *    that allows to print only those messages to the console that were produced while processing
   *    a specific page, see the comment given for the constructor for more information.
   *
   * @return
   *    A stream which prints the received messages with log level ERROR to the console.
   */
  ostream& error(int pageNum = -1) const;

 private:
  /**
   * This method returns a stream which prints the received messages with the given log level to
   * the console.
   *
   * @param logLevel
   *    The log level of the messages that will be sent to the stream.
   * @param pageNum
   *    A page number, indicating that the message sent to the stream was produced while processing
   *    the <pageNum>-th page of the current document.
   *
   * @return
   *    A stream which prints the received messages with the given log level to the console.
   */
  ostream& log(const LogLevel& logLevel, int pageNum = -1) const;

  /**
   * This method returns the current timestamp as a human-readable string, for example:
   * "2022-06-02 10:49:00.990". This string is prepended to each log message printed by this
   * logger, for reproducibility purposes.
   *
   * @return
   *    The current time stamp as a human-readable string.
   */
  static string getTimeStamp();

  // The logging level.
  LogLevel _logLevel = ERROR;

  // The page filter.
  int _pageFilter = -1;
};

#endif  // UTILS_LOG_H_
