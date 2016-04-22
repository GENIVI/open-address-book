/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */
/**
 * @file logger_tests.cpp
 */
#include <gtest/gtest.h>
#include <string>
#include "TestLogger.hpp"
#include "helpers/Log.hpp"


class LoggeTests: public ::testing::Test
{
public:
    LoggeTests() : ::testing::Test()
    {
    }

    ~LoggeTests()
    {
      OpenAB::Logger::setDefaultLogger(NULL);
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

TEST_F(LoggeTests, testDefaultLogger)
{
  OpenAB::Logger::OutLevel() = OpenAB::Logger::Debug;
  LOG_DEBUG()<<"String"<<123<<(void*)0xBAAD<<std::string("abcd")<<std::endl;
  LOG_FUNC();
  LOG_INFO()<<"String"<<123<<(void*)0xBAAD<<std::string("abcd")<<std::endl;
  LOG_VERBOSE()<<"String"<<123<<(void*)0xBAAD<<std::string("abcd")<<std::endl;
  LOG_FATAL()<<"String"<<123<<(void*)0xBAAD<<std::string("abcd")<<std::endl;
  LOG_WARNING()<<"String"<<123<<(void*)0xBAAD<<std::string("abcd")<<std::endl;
  LOG_ERROR()<<"String"<<123<<(void*)0xBAAD<<std::string("abcd")<<std::endl;
}

TEST_F(LoggeTests, testCustomLogger)
{
  OpenAB_TESTS::TestLogger testLogger;
  OpenAB::Logger::setDefaultLogger(&testLogger);

  OpenAB::Logger::OutLevel() = OpenAB::Logger::Debug;
  LOG_DEBUG()<<"String "<<123<<" "<<(void*)0xBAAD<<" "<<std::string("abcd")<<std::endl;
  ASSERT_EQ("  Debug : String 123 0xbaad abcd", testLogger.lastMessage);

  LOG_FUNC();
 // ASSERT_EQ(" DebugF : ../../tests/logger_tests.cpp: 110: TestBody", testLogger.lastMessage);

  LOG_INFO()<<"String "<<123<<" "<<(void*)0xBAAD<<" "<<std::string("abcd")<<std::endl;
  ASSERT_EQ("   Info : String 123 0xbaad abcd", testLogger.lastMessage);

  LOG_VERBOSE()<<"String "<<123<<" "<<(void*)0xBAAD<<" "<<std::string("abcd")<<std::endl;
  ASSERT_EQ("Verbose : String 123 0xbaad abcd", testLogger.lastMessage);

  LOG_FATAL()<<"String "<<123<<" "<<(void*)0xBAAD<<" "<<std::string("abcd")<<std::endl;
  ASSERT_EQ("  Fatal : String 123 0xbaad abcd", testLogger.lastMessage);

  LOG_WARNING()<<"String "<<123<<" "<<(void*)0xBAAD<<" "<<std::string("abcd")<<std::endl;
  ASSERT_EQ("Warning : String 123 0xbaad abcd", testLogger.lastMessage);

  LOG_ERROR()<<"String "<<123<<" "<<(void*)0xBAAD<<" "<<std::string("abcd")<<std::endl;
  ASSERT_EQ("  Error : String 123 0xbaad abcd", testLogger.lastMessage);
}
