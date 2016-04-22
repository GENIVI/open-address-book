/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file Log.cpp
 */

#include "Log.hpp"
#include <stdio.h>
#include <iomanip>

namespace OpenAB {

class DefaultLogger : public Logger
{
  public:
    DefaultLogger() : Logger()
    {

    }
    ~DefaultLogger()
    {

    }

    Logger& operator<< (std::ostream& (*pf)(std::ostream&))
    {
      typedef std::ostream& (std_endl_type)(std::ostream&);
      std_endl_type *std_endl = std::endl;
      if (pf == std_endl)
      {
        std::cout<<std::setw(7)<<toString(level)<<" : "<<oss.str()<<std::endl;
        oss.clear();
        oss.str("");
      }
      return *this;
    }
  private:

};


static DefaultLogger fallbackLogger;
static Logger* defaultLogger = NULL;

Logger* Logger::getDefaultLogger()
{
  if (NULL == defaultLogger)
    return &fallbackLogger;
  return defaultLogger;
}

void Logger::setDefaultLogger(Logger* l)
{
  defaultLogger = l;
}

Logger::Logger() :
  level (Error)
{
}

Logger::~Logger()
{

}

Logger& Logger::get(Logger::LogLevel l)
{
  level = l;
  return *this;
}

const char* Logger::toString(Logger::LogLevel l)
{
  switch(l){
    case Fatal  : return "Fatal";
    case Error  : return "Error";
    case Warning: return "Warning";
    case Info   : return "Info";
    case Verbose: return "Verbose";
    case DebugF : return "DebugF";
    case Debug  : return "Debug";
  }
  return "";
}

Logger::LogLevel& Logger::OutLevel()
{
  static LogLevel l = Info;
  return l;
}

}
