/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */
/**
 * @file generic_params_tests.cpp
 */
#include <gtest/gtest.h>
#include <string>
#include <plugin/GenericParameters.hpp>


class GenericParametersTests: public ::testing::Test
{
public:
    GenericParametersTests() : ::testing::Test()
    {
    }

    ~GenericParametersTests()
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

TEST_F(GenericParametersTests, testEmptyParameters)
{
  OpenAB_Plugin::GenericParameters params;
  ASSERT_EQ("{ }", params.toJSON());

  OpenAB_Plugin::GenericParameters params2("");
  ASSERT_EQ("{ }", params2.toJSON());

  OpenAB_Plugin::GenericParameters params3(" ");
  ASSERT_EQ("{ }", params3.toJSON());

  ASSERT_TRUE(params.fromJSON("{ }"));
  ASSERT_EQ("{ }", params.toJSON());
}

TEST_F(GenericParametersTests, testParsingEmptyJSONString)
{
  OpenAB_Plugin::GenericParameters params;
  ASSERT_FALSE(params.fromJSON(""));
}

TEST_F(GenericParametersTests, testGettingNotExistingKey)
{
  OpenAB_Plugin::GenericParameters params;
  OpenAB::Variant value = params.getValue("value1");
  ASSERT_TRUE(value.invalid());
}

TEST_F(GenericParametersTests, testSetValue)
{
  OpenAB::Variant value1("hello");
  OpenAB::Variant value2(false);
  OpenAB::Variant value3(64.0f);

  OpenAB_Plugin::GenericParameters params;
  params.setValue("value1", value1);
  params.setValue("value2", value2);
  params.setValue("value3", value3);

  ASSERT_TRUE(value1 == params.getValue("value1"));
  ASSERT_TRUE(value2 == params.getValue("value2"));
  ASSERT_TRUE(value3 == params.getValue("value3"));

  //override value1
  params.setValue("value1", value3);
  ASSERT_TRUE(value3 == params.getValue("value1"));
}

TEST_F(GenericParametersTests, testRemoveValue)
{
  OpenAB::Variant value1("hello");
  OpenAB::Variant value2(false);
  OpenAB::Variant value3(64.0f);

  OpenAB_Plugin::GenericParameters params;
  params.setValue("value1", value1);
  params.setValue("value2", value2);
  params.setValue("value3", value3);

  ASSERT_FALSE(params.getValue("value1").invalid());
  ASSERT_FALSE(params.getValue("value2").invalid());
  ASSERT_FALSE(params.getValue("value3").invalid());

  params.removeKey("value1");
  ASSERT_TRUE(params.getValue("value1").invalid());
  ASSERT_FALSE(params.getValue("value2").invalid());
  ASSERT_FALSE(params.getValue("value3").invalid());

  params.removeKey("value2");
  ASSERT_TRUE(params.getValue("value1").invalid());
  ASSERT_TRUE(params.getValue("value2").invalid());
  ASSERT_FALSE(params.getValue("value3").invalid());

  params.removeKey("value3");
  ASSERT_TRUE(params.getValue("value1").invalid());
  ASSERT_TRUE(params.getValue("value2").invalid());
  ASSERT_TRUE(params.getValue("value3").invalid());

  params.removeKey("value3");
  ASSERT_TRUE(params.getValue("value1").invalid());
  ASSERT_TRUE(params.getValue("value2").invalid());
  ASSERT_TRUE(params.getValue("value3").invalid());
}

TEST_F(GenericParametersTests, testJSONDeserialization)
{
  OpenAB_Plugin::GenericParameters params;
  params.setValue("string_value", OpenAB::Variant("hello"));
  params.setValue("bool_value", OpenAB::Variant(true));
  params.setValue("float_value", OpenAB::Variant(12.4f));
  params.setValue("double_value", OpenAB::Variant(12.4));
  params.setValue("uint8_value", OpenAB::Variant((u_int8_t)123));
  params.setValue("uint16_value", OpenAB::Variant((u_int16_t)123));
  params.setValue("uint32_value", OpenAB::Variant((u_int32_t)123));
  params.setValue("int8_value", OpenAB::Variant((int8_t)123));
  params.setValue("int16_value", OpenAB::Variant((int16_t)123));
  params.setValue("int32_value", OpenAB::Variant((int32_t)123));
  params.setValue("invalid_value", OpenAB::Variant());

  std::string expectedJSON = "{ \"bool_value\": true, \"double_value\": 12.400000, \"float_value\": 12.400000, \"int16_value\": 123, \"int32_value\": 123, \"int8_value\": 123, \"string_value\": \"hello\", \"uint16_value\": 123, \"uint32_value\": 123, \"uint8_value\": 123 }";
  std::string json = params.toJSON();
  ASSERT_EQ(expectedJSON, json);
}

TEST_F(GenericParametersTests, testJSONSerialization)
{
  std::string json = "{ \"bool_value\": true, \"double_value\": 12.400000, \"float_value\": 12.400000, \"int16_value\": 123, \"int32_value\": 123, \"int8_value\": 123, \"string_value\": \"hello\", \"uint16_value\": 123, \"uint32_value\": 123, \"uint8_value\": 123 }";
  OpenAB_Plugin::GenericParameters params;
  ASSERT_TRUE(params.fromJSON(json));
  ASSERT_TRUE(OpenAB::Variant("hello") == params.getValue("string_value"));
  ASSERT_TRUE(OpenAB::Variant(true) == params.getValue("bool_value"));
  ASSERT_TRUE(OpenAB::Variant(12.4) == params.getValue("float_value"));
  ASSERT_TRUE(OpenAB::Variant(12.4) == params.getValue("double_value"));
  ASSERT_TRUE(OpenAB::Variant((int32_t)123) == params.getValue("uint8_value"));
  ASSERT_TRUE(OpenAB::Variant((int32_t)123) == params.getValue("uint16_value"));
  ASSERT_TRUE(OpenAB::Variant((int32_t)123) == params.getValue("uint32_value"));
  ASSERT_TRUE(OpenAB::Variant((int32_t)123) == params.getValue("int8_value"));
  ASSERT_TRUE(OpenAB::Variant((int32_t)123) == params.getValue("int16_value"));
  ASSERT_TRUE(OpenAB::Variant((int32_t)123) == params.getValue("int32_value"));
}
