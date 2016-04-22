/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file TimeStamp.cpp
 */

#include "TimeStamp.hpp"
#include <sstream>

namespace OpenAB {

TimeStamp::TimeStamp()
{
  timestamp.tv_sec = 0;
  timestamp.tv_usec = 0;
}

TimeStamp::TimeStamp(bool current)
{
  if (current)
  {
    gettimeofday(&timestamp, NULL);
  }
  else
  {
    timestamp.tv_sec = 0;
    timestamp.tv_usec = 0;
  }
}

TimeStamp::TimeStamp(__time_t s, __suseconds_t us)
{
  timestamp.tv_sec = s;
  timestamp.tv_usec = us;
}

TimeStamp::~TimeStamp()
{
}

void TimeStamp::setNow()
{
  gettimeofday(&timestamp, NULL);
}

TimeStamp TimeStamp::operator +(const TimeStamp& other) const
{
  TimeStamp result;

  timeradd(&timestamp, &other.timestamp, &result.timestamp);

  return result;
}

TimeStamp TimeStamp::operator -(const TimeStamp& other) const
{
  TimeStamp result;

  timersub(&timestamp, &other.timestamp, &result.timestamp);

  return result;
}

TimeStamp& TimeStamp::operator +=(const TimeStamp& other)
{
  timeval temp;
  timeradd(&timestamp, &other.timestamp, &temp);
  timestamp = temp;

  return *this;
}

TimeStamp& TimeStamp::operator -=(const TimeStamp& other)
{
  timeval temp;
  timersub(&timestamp, &other.timestamp, &temp);
  timestamp = temp;

  return *this;
}

bool TimeStamp::operator <(const TimeStamp& other) const
{
  return timercmp(&timestamp, &other.timestamp, <);
}


bool TimeStamp::operator <=(const TimeStamp& other) const
{
  return timercmp(&timestamp, &other.timestamp, <=);
}

bool TimeStamp::operator >(const TimeStamp& other) const
{
  return timercmp(&timestamp, &other.timestamp, >);
}


bool TimeStamp::operator >=(const TimeStamp& other) const
{
  return timercmp(&timestamp, &other.timestamp, >=);
}


std::string TimeStamp::toString() const
{
  std::ostringstream oss;
  oss << timestamp.tv_sec <<" s "<<timestamp.tv_usec <<" us";
  return oss.str();
}

unsigned int TimeStamp::toMs() const
{
  return (timestamp.tv_sec * 1000 + timestamp.tv_usec/1000);
}

} // namespace OpenAB
