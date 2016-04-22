/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */
/**
 * @file variant_tests.cpp
 */
#include <gtest/gtest.h>
#include <string>
#include "helpers/Variant.hpp"

class VariantTests: public ::testing::Test
{
public:
    VariantTests() : ::testing::Test()
    {
    }

    ~VariantTests()
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

TEST_F(VariantTests, testConstructors)
{
  //check if default constructor products invalid variant
  OpenAB::Variant variant;
  ASSERT_TRUE(variant.invalid());

  //check if invalid variant returns default values for getters
  ASSERT_EQ(0, variant.getChar());
  ASSERT_EQ(0, variant.getInt());
  ASSERT_EQ(0.0, variant.getDouble());
  ASSERT_EQ(false, variant.getBool());
  ASSERT_EQ(std::string(), variant.getString());
  ASSERT_EQ(NULL, variant.getPointer());
  ASSERT_TRUE(OpenAB::SecureString() == variant.getSecureString());

  //check constructor for char type
  variant = OpenAB::Variant('a');
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::CHAR, variant.getType());
  ASSERT_EQ('a', variant.getChar());

  //check constructor for int types
  variant = OpenAB::Variant((u_int8_t)123);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::INTEGER, variant.getType());
  ASSERT_EQ(123, variant.getInt());

  variant = OpenAB::Variant((u_int16_t)123);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::INTEGER, variant.getType());
  ASSERT_EQ(123, variant.getInt());

  variant = OpenAB::Variant((u_int32_t)123);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::INTEGER, variant.getType());
  ASSERT_EQ(123, variant.getInt());

  variant = OpenAB::Variant((int8_t)123);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::INTEGER, variant.getType());
  ASSERT_EQ(123, variant.getInt());

  variant = OpenAB::Variant((int16_t)123);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::INTEGER, variant.getType());
  ASSERT_EQ(123, variant.getInt());

  variant = OpenAB::Variant((int32_t)123);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::INTEGER, variant.getType());
  ASSERT_EQ(123, variant.getInt());

  //check constructor for float type
  variant = OpenAB::Variant(123.45f);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::DOUBLE, variant.getType());
  ASSERT_EQ(123.45f, variant.getDouble());

  //check constructor for double type
  variant = OpenAB::Variant(123.45);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::DOUBLE, variant.getType());
  ASSERT_EQ(123.45, variant.getDouble());

  //check constructor for bool type
  variant = OpenAB::Variant(true);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::BOOL, variant.getType());
  ASSERT_EQ(true, variant.getBool());

  //check constructor for pointer type
  variant = OpenAB::Variant((void*)0xBAAD);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::POINTER, variant.getType());
  ASSERT_EQ((void*)0xBAAD, variant.getPointer());

  //check constructor for string type
  variant = OpenAB::Variant("hello");
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::STRING, variant.getType());
  ASSERT_EQ("hello", variant.getString());

  variant = OpenAB::Variant(std::string("hello"));
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::STRING, variant.getType());
  ASSERT_EQ("hello", variant.getString());

  //check constructor for secure string type
  variant = OpenAB::Variant(OpenAB::SecureString("hello"));
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::SECURE_STRING, variant.getType());
  ASSERT_EQ("hello", std::string(variant.getSecureString().str()));

}

TEST_F(VariantTests, testCopyConstructor)
{
  //check if default constructor products invalid variant
  OpenAB::Variant variant;
  ASSERT_TRUE(variant.invalid());

  OpenAB::Variant copy(variant);
  ASSERT_TRUE(copy.invalid());

  variant = OpenAB::Variant(123.0);
  ASSERT_FALSE(variant.invalid());

  copy = OpenAB::Variant(variant);
  ASSERT_FALSE(copy.invalid());
  ASSERT_EQ(OpenAB::Variant::DOUBLE, copy.getType());
  ASSERT_EQ(123.0, copy.getDouble());

  variant = OpenAB::Variant("hello");
  ASSERT_FALSE(variant.invalid());

  copy = OpenAB::Variant(variant);
  ASSERT_FALSE(copy.invalid());
  ASSERT_EQ(OpenAB::Variant::STRING, copy.getType());
  ASSERT_EQ("hello", copy.getString());

  variant = OpenAB::Variant(OpenAB::SecureString("hello"));
  ASSERT_FALSE(variant.invalid());

  copy = OpenAB::Variant(variant);
  ASSERT_FALSE(copy.invalid());
  ASSERT_EQ(OpenAB::Variant::SECURE_STRING, copy.getType());
  ASSERT_EQ("hello", std::string(copy.getSecureString().str()));
}

TEST_F(VariantTests, testSetters)
{
  OpenAB::Variant variant;
  ASSERT_TRUE(variant.invalid());

  //set bool variable
  variant.set(true);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::BOOL, variant.getType());
  ASSERT_EQ(true, variant.getBool());

  //set char variable
  variant.set('a');
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::CHAR, variant.getType());
  ASSERT_EQ('a', variant.getChar());

  //set string variable
  variant.set("hello");
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::STRING, variant.getType());
  ASSERT_EQ("hello", variant.getString());

  variant.set(std::string("hello"));
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::STRING, variant.getType());
  ASSERT_EQ("hello", variant.getString());

  //set secure string variable
  variant.set(OpenAB::SecureString("hello"));
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::SECURE_STRING, variant.getType());
  ASSERT_EQ("hello", std::string(variant.getSecureString().str()));

  //set pointer variable
  variant.set((void*)0xBAAD);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::POINTER, variant.getType());
  ASSERT_EQ((void*)0xBAAD, variant.getPointer());

  //set float variable
  variant.set(123.45f);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::DOUBLE, variant.getType());
  ASSERT_EQ(123.45f, variant.getDouble());

  //set double variable
  variant.set(123.45);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::DOUBLE, variant.getType());
  ASSERT_EQ(123.45, variant.getDouble());

  //set int variables
  variant.set((u_int8_t)123);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::INTEGER, variant.getType());
  ASSERT_EQ(123, variant.getInt());

  variant.set((u_int16_t)123);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::INTEGER, variant.getType());
  ASSERT_EQ(123, variant.getInt());

  variant.set((u_int32_t)123);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::INTEGER, variant.getType());
  ASSERT_EQ(123, variant.getInt());

  variant.set((int8_t)123);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::INTEGER, variant.getType());
  ASSERT_EQ(123, variant.getInt());

  variant.set((int16_t)123);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::INTEGER, variant.getType());
  ASSERT_EQ(123, variant.getInt());

  variant.set((int32_t)123);
  ASSERT_FALSE(variant.invalid());
  ASSERT_EQ(OpenAB::Variant::INTEGER, variant.getType());
  ASSERT_EQ(123, variant.getInt());
}

TEST_F(VariantTests, testComparison)
{
  //test comparison between two different types of variant
  OpenAB::Variant variant1("hello");
  OpenAB::Variant variant2(123);
  ASSERT_FALSE(variant1 == variant2);

  //test comparison between char types
  variant1.set('a');
  variant2.set('a');
  ASSERT_TRUE(variant1 == variant2);

  variant2.set('b');
  ASSERT_FALSE(variant1 == variant2);

  //test comparison between bool types
  variant1.set(true);
  variant2.set(true);
  ASSERT_TRUE(variant1 == variant2);

  variant2.set(false);
  ASSERT_FALSE(variant1 == variant2);

  //test comparison between pointer types
  variant1.set((void*)0xBAAD);
  variant2.set((void*)0xBAAD);
  ASSERT_TRUE(variant1 == variant2);

  variant2.set((void*)0xF00D);
  ASSERT_FALSE(variant1 == variant2);

  //test comparison between string types
  variant1.set("hello");
  variant2.set(std::string("hello"));
  ASSERT_TRUE(variant1 == variant2);

  variant2.set(std::string("goodbye"));
  ASSERT_FALSE(variant1 == variant2);


  //test comparison between secure string types
  variant1.set(OpenAB::SecureString("hello"));
  variant2 = variant1;
  ASSERT_TRUE(variant1 == variant2);

  //each secure string will be different even if containing the same string
  variant2.set(OpenAB::SecureString("hello"));
  ASSERT_FALSE(variant1 == variant2);

  //test comparison between float types
  variant1.set(123.45f);
  variant2.set(123.45f);
  ASSERT_TRUE(variant1 == variant2);

  variant2.set(543.21f);
  ASSERT_FALSE(variant1 == variant2);

  //test comparison between double types
  variant1.set(123.45);
  variant2.set(123.45);
  ASSERT_TRUE(variant1 == variant2);

  variant2.set(543.21);
  ASSERT_FALSE(variant1 == variant2);

  //test comparison between int types
  variant1.set((u_int8_t)123);
  variant2.set((u_int8_t)123);
  ASSERT_TRUE(variant1 == variant2);

  variant2.set((u_int8_t)321);
  ASSERT_FALSE(variant1 == variant2);


  variant1.set((u_int16_t)123);
  variant2.set((u_int16_t)123);
  ASSERT_TRUE(variant1 == variant2);

  variant2.set((u_int16_t)321);
  ASSERT_FALSE(variant1 == variant2);


  variant1.set((u_int32_t)123);
  variant2.set((u_int32_t)123);
  ASSERT_TRUE(variant1 == variant2);

  variant2.set((u_int32_t)321);
  ASSERT_FALSE(variant1 == variant2);


  variant1.set((int8_t)123);
  variant2.set((int8_t)123);
  ASSERT_TRUE(variant1 == variant2);

  variant2.set((int8_t)321);
  ASSERT_FALSE(variant1 == variant2);


  variant1.set((int16_t)123);
  variant2.set((int16_t)123);
  ASSERT_TRUE(variant1 == variant2);

  variant2.set((int16_t)321);
  ASSERT_FALSE(variant1 == variant2);


  variant1.set((int32_t)123);
  variant2.set((int32_t)123);
  ASSERT_TRUE(variant1 == variant2);

  variant2.set((int32_t)321);
  ASSERT_FALSE(variant1 == variant2);
}
