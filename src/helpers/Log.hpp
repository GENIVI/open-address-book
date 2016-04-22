/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file Log.hpp
 */

#ifndef _SYNC_LOG_HPP
#define _SYNC_LOG_HPP

#include <sstream>
#include <iomanip>
#include <iostream>

namespace OpenAB {

/**
 * @brief Logger class, allows to override way in which messages from OpenAB will be logged.
 */
class Logger
{
  public:
    /**
     * @brief Default constructor.
     */
    Logger();

    /**
     * @brief Destructor, virtual by default.
     */
    virtual ~Logger();

    /** @enum
     * Levels of log messages
     */
    enum LogLevel
    {
      Fatal = 0 , /**< Fatal messages */
      Error,      /**< Error messages */
      Warning,    /**< Warning messages */
      Info,       /**< Info messages */
      Verbose,    /**< Verbose messages */
      DebugF,     /**< Debug messages with current function name*/
      Debug       /**< Debug messages */
    };

    template <typename T>
    Logger& operator<< (T data)
    {
      oss<<data;
      return *this;
    }

    virtual Logger& operator<< (std::ostream& (*pf)(std::ostream&)) = 0;

    /**
     * Returns Logger reference set to print messages in given LogLevel.
     * @param [in] l log level
     * @returns reference to Logger
     */
    Logger& get(LogLevel l);

    /**
     * @brief Returns currently set LogLevel and allows to override it.
     * @return reference to currently set LogLevel
     */
    static LogLevel& OutLevel();

    /**
     * @brief Returns pointer to currently set default logger.
     * There can be only one default logger set at the time
     * @return Pointer to currently set logger, or NULL if no logger was set.
     */
    static Logger* getDefaultLogger();

    /**
     * @brief Sets new default logger.
     * @param [in] logger new logger to be used as default one.
     * @note OpenAB provides default logger that is used by default,
     * until explicitly override by setDefaultLogger call.
     */
    static void setDefaultLogger(Logger* logger);

  protected:
    const char* toString(LogLevel l);
    LogLevel level;
    std::ostringstream oss;
};



/**
 * @param[in]  __level logging level
 * @return  An Logger object
 */
#define LOG_L(__level)  \
  if (OpenAB::Logger::OutLevel() >= __level) \
    OpenAB::Logger::getDefaultLogger()->get(__level)

/**
 * @return  An Logger for a Verbose Log
 */
#define LOG_VERBOSE()  \
    LOG_L(OpenAB::Logger::Verbose)

/**
 * @return  An Logger for a Debug Log
 */
#define LOG_DEBUG()  \
    LOG_L(OpenAB::Logger::Debug)

/**
 * @return  An Logger for a Error Log
 */
#define LOG_ERROR()  \
    LOG_L(OpenAB::Logger::Error)

/**
 * @return  An Logger for a Fatal Error Log
 */
#define LOG_FATAL()  \
    LOG_L(OpenAB::Logger::Fatal)

/**
 * @return  An Logger for a Warning Log
 */
#define LOG_WARNING()  \
    LOG_L(OpenAB::Logger::Warning)

/**
 * @return  An Logger for a Info Log
 */
#define LOG_INFO()  \
    LOG_L(OpenAB::Logger::Info)

/**
 * @return  An Logger for a Log (by default Info Level)
 */
#define LOG()  \
  LOG_INFO()

/**
 * @return  An Logger with the line number and the function prefix
 */
#define LOG_FUNC() \
  LOG_L(OpenAB::Logger::DebugF) << __FILE__ <<": " << (long long int)(__LINE__) << ": "<< __FUNCTION__ << std::endl

/** @} */

}
#endif /* end  _SYNC_LOG_HPP */
