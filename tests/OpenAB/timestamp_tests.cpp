/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */
/**
 * @file timestamp_tests.cpp 
 */
#include <gtest/gtest.h>
#include <string>
#include "helpers/TimeStamp.hpp"

class TimeStampTests: public ::testing::Test
{
public:
    TimeStampTests() : ::testing::Test()
    {
    }

    ~TimeStampTests()
    {
    }

protected:
    // Sets up the test fixture.
    virtual void SetUp()
    {
    }

    // Tears down the test fixture.
    virtual void TearDown()
    {

    }

};

TEST_F(TimeStampTests, testConstructors)
{
  OpenAB::TimeStamp ts1;
  ASSERT_EQ(0, ts1.toMs());

  OpenAB::TimeStamp ts2(true);
  ASSERT_NE(0, ts2.toMs());

  OpenAB::TimeStamp ts3(false);
  ASSERT_EQ(0, ts3.toMs());

  OpenAB::TimeStamp ts4(5, 0);
  ASSERT_EQ(5000, ts4.toMs());

  ts3.setNow();

  sleep(1);

  ts4.setNow();

  ASSERT_TRUE(ts4 > ts3);
}

TEST_F(TimeStampTests, testArithmetics)
{
  OpenAB::TimeStamp ts1(5, 900000);
  OpenAB::TimeStamp ts2(1, 200000);

  OpenAB::TimeStamp result;

  result = ts1 + ts2;
  ASSERT_EQ(7100, result.toMs());

  result = ts1 - ts2;
  ASSERT_EQ(4700, result.toMs());

  ts1 += ts2;
  ASSERT_EQ(7100, ts1.toMs());

  ts1 = OpenAB::TimeStamp(5, 900000);
  ts1 -= ts2;
  ASSERT_EQ(4700, ts1.toMs());

  ASSERT_TRUE(ts1 > ts2);
  ASSERT_TRUE(ts1 >= ts2);

  ASSERT_FALSE(ts2 > ts1);
  ASSERT_FALSE(ts2 >= ts1);


  ASSERT_FALSE(ts1 < ts2);
  ASSERT_FALSE(ts1 <= ts2);

  ASSERT_TRUE(ts2 < ts1);
  ASSERT_TRUE(ts2 <= ts1);
}

TEST_F(TimeStampTests, testToStringCoversion)
{
  OpenAB::TimeStamp ts(5, 140);
  ASSERT_EQ("5 s 140 us", ts.toString());
}
