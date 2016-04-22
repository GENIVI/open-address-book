/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file TestLogger.hpp
 */

#ifndef TESTLOGGER_HPP_
#define TESTLOGGER_HPP_
#include <string>
#include "helpers/Log.hpp"
/*!
 * @brief namespace OpenAB_TESTS
 */
namespace OpenAB_TESTS {

class TestLogger : public OpenAB::Logger
{
  public:
    TestLogger() : Logger()
  {

  }
  ~TestLogger()
  {

  }

  Logger& operator<< (std::ostream& (*pf)(std::ostream&))
  {
    typedef std::ostream& (std_endl_type)(std::ostream&);
    std_endl_type *std_endl = std::endl;
    if (pf == std_endl)
    {
      std::stringstream sss;
      sss<<std::setw(7)<<toString(level)<<" : "<<oss.str();
      lastMessage = sss.str();
      oss.clear();
      oss.str("");
    }
    return *this;
  }
  std::string lastMessage;
};
} // namespace OpenAB_TESTS

#endif // TESTLOGGER_HPP_
