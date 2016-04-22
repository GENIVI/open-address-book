/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */
/**
 * @file smart_ptr_tests.cpp
 */
#include <gtest/gtest.h>
#include <string>
#include "helpers/SmartPtr.hpp"

class SmartPtrTests: public ::testing::Test
{
public:
    SmartPtrTests() : ::testing::Test()
    {
    }

    ~SmartPtrTests()
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

TEST_F(SmartPtrTests, testConstructors)
{
  //check construction of empty smart ptr
  OpenAB::SmartPtr<int> pointer;
  ASSERT_EQ(NULL, pointer.getPointer());
  ASSERT_EQ(NULL, pointer.operator ->());

  int* p = new int;
  OpenAB::SmartPtr<int> pointer2(p);
  ASSERT_TRUE(pointer2.getPointer() == p);

  OpenAB::SmartPtr<int> pointer3(pointer2);
  ASSERT_TRUE(pointer2 == pointer3);
  ASSERT_TRUE(pointer2.getPointer() == pointer3.getPointer());
}

TEST_F(SmartPtrTests, testAssignmentOperator)
{
  OpenAB::SmartPtr<int> pointer;
  ASSERT_EQ(NULL, pointer.getPointer());
  ASSERT_EQ(NULL, pointer.operator ->());

  int* p = new int;
  OpenAB::SmartPtr<int> pointer2(p);
  ASSERT_TRUE(pointer2.getPointer() == p);

  //check assignment to empty smart ptr
  pointer = pointer2;
  ASSERT_TRUE(pointer == pointer2);
  ASSERT_TRUE(pointer.getPointer() == pointer2.getPointer());

  //check direct assignment
  pointer = NULL;
  ASSERT_EQ(NULL, pointer.getPointer());
  pointer2 = NULL;
  ASSERT_EQ(NULL, pointer2.getPointer());

  p = new int;
  pointer = p;
  ASSERT_EQ(p, pointer.getPointer());
}

TEST_F(SmartPtrTests, testComparison)
{
  int *p = new int;
  *p = 123;

  int *p2 = new int;
  *p2 = 123;

  OpenAB::SmartPtr<int> pointer1(p);
  OpenAB::SmartPtr<int> pointer2(p2);
  OpenAB::SmartPtr<int> empty;

  ASSERT_EQ(p, pointer1.getPointer());
  ASSERT_EQ(p2, pointer2.getPointer());

  ASSERT_TRUE(pointer1 == pointer2);
  ASSERT_TRUE(pointer2 == pointer1);
  ASSERT_FALSE(pointer1 != pointer2);
  ASSERT_FALSE(pointer2 != pointer1);

  ASSERT_TRUE(*pointer1 == *pointer2);
  ASSERT_TRUE(*pointer2 == *pointer1);
  ASSERT_FALSE(*pointer1 != *pointer2);
  ASSERT_FALSE(*pointer2 != *pointer1);

  ASSERT_FALSE(pointer1.getPointer() == pointer2.getPointer());
  ASSERT_FALSE((int*)pointer1 == (int*)pointer2);
  ASSERT_FALSE((const int*)pointer1 == (const int*)pointer2);

  *p2 = 312;
  ASSERT_FALSE(pointer1 == pointer2);
  ASSERT_FALSE(pointer2 == pointer1);
  ASSERT_TRUE(pointer1 != pointer2);
  ASSERT_TRUE(pointer2 != pointer1);

  ASSERT_FALSE(pointer1.getPointer() == pointer2.getPointer());
  ASSERT_FALSE(*pointer1 == *pointer2);
  ASSERT_FALSE((int*)pointer1 == (int*)pointer2);
  ASSERT_FALSE((const int*)pointer1 == (const int*)pointer2);

  //compare with empty smart ptr, should always return false, except when comparing two empty pointers
  ASSERT_FALSE(pointer1 == empty);
  ASSERT_FALSE(empty == pointer1);

  ASSERT_TRUE(pointer1 != empty);
  ASSERT_TRUE(empty != pointer1);

  ASSERT_TRUE(empty == empty);
  ASSERT_FALSE(empty != empty);

  //check < operator
  *p = *p2;
  ASSERT_FALSE(pointer1 < pointer2);

  *p = 0;
  ASSERT_TRUE(pointer1 < pointer2);
  ASSERT_FALSE(pointer2 < pointer1);

  ASSERT_TRUE(empty < pointer2);
  ASSERT_FALSE(pointer2 < empty);
  ASSERT_FALSE(empty < empty);

}
