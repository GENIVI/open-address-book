/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file TimeStamp.hpp
 */

#ifndef TIMESTAMP_HPP_
#define TIMESTAMP_HPP_

#include <sys/time.h>
#include <string>

/*!
 * @brief namespace OpenAB
 */
namespace OpenAB {

/*!
 * @brief Class representing time stamp.
 * Allows for basic operations like adding and subtracting time stamps.
 */
class TimeStamp
{
  public:
    /*!
     *  @brief Constructor.
     */
    TimeStamp();

    TimeStamp(bool current);

    TimeStamp(__time_t s, __suseconds_t us);

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~TimeStamp();

    TimeStamp operator+ (const TimeStamp& other) const;
    TimeStamp operator- (const TimeStamp& other) const;

    TimeStamp& operator+= (const TimeStamp& other);
    TimeStamp& operator-= (const TimeStamp& other);

    bool operator< (const TimeStamp& other) const;
    bool operator<= (const TimeStamp& other) const;
    bool operator> (const TimeStamp& other) const;
    bool operator>= (const TimeStamp& other) const;

    void setNow();

    unsigned int toMs() const;

    std::string toString() const;

  private:

    struct timeval timestamp;
};

} // namespace OpenAB

#endif // TIMESTAMP_HPP_
