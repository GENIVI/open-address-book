/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */
/**
 * @file secure_string_tests.cpp 
 */
#include <gtest/gtest.h>
#include <string>
#include "helpers/SecureString.hpp"

class SecureStringTests: public ::testing::Test
{
public:
    SecureStringTests() : ::testing::Test()
    {
    }

    ~SecureStringTests()
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

TEST_F(SecureStringTests, DISABLED_testConstructors)
{
  OpenAB::SecureString string("test");
  OpenAB::SecureString string1(string);
  OpenAB::SecureString string2 = string;
  OpenAB::SecureString string3(std::string("test"));

  ASSERT_TRUE(strcmp("test", string.str()) == 0);
  ASSERT_TRUE(strcmp("test", string1.str()) == 0);
  ASSERT_TRUE(strcmp("test", string2.str()) == 0);
  ASSERT_TRUE(strcmp("test", string3.str()) == 0);

  string.clearStr();
  string.clear();
  ASSERT_TRUE(strcmp("test", string.str()) == 1);
}

