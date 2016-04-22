/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */
/**
 * @file strings_helper_tests.cpp
 */
#include <gtest/gtest.h>
#include <string>
#include "helpers/StringHelper.hpp"

class StringHelperTests: public ::testing::Test
{
public:
    StringHelperTests() : ::testing::Test()
    {
    }

    ~StringHelperTests()
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

TEST_F(StringHelperTests, testContains)
{
  std::vector<std::string> stringsVector;
  stringsVector.push_back("testString1");
  stringsVector.push_back("testString2");
  stringsVector.push_back("testString3");

  ASSERT_TRUE(OpenAB::contains(stringsVector, "testString1"));
  ASSERT_TRUE(OpenAB::contains(stringsVector, "testString2"));
  ASSERT_TRUE(OpenAB::contains(stringsVector, "testString3"));
  ASSERT_FALSE(OpenAB::contains(stringsVector, "testString4"));
}

TEST_F(StringHelperTests, testTrimWhitespaces)
{
  std::string strWithSpacesAndWhiteSpaces = " \t\n\v\f\rTest\t\n\v\f\r ";
  std::string strWithWhiteSpaces = "\t\n\v\f\rTest\t\n\v\f\r";

  OpenAB::trimWhitespaces(strWithWhiteSpaces);
  ASSERT_EQ("Test", strWithWhiteSpaces);
  OpenAB::trimSpaces(strWithSpacesAndWhiteSpaces);
  ASSERT_EQ("Test", strWithSpacesAndWhiteSpaces);
}

TEST_F(StringHelperTests, testTokenize)
{
  std::string str1 = "token1 token2 token3";
  std::string str2 = "token1;;token2;token3";
  std::string str3 = "token1 token2 token1";


  std::vector<std::string> tokens;

  tokens = OpenAB::tokenize(str1, ' ', false, false);
  ASSERT_EQ(3, tokens.size());
  ASSERT_EQ("token1", tokens.at(0));
  ASSERT_EQ("token2", tokens.at(1));
  ASSERT_EQ("token3", tokens.at(2));

  //leave empty tokens
  tokens = OpenAB::tokenize(str2, ';', false, true);
  ASSERT_EQ(4, tokens.size());
  ASSERT_EQ("token1", tokens.at(0));
  ASSERT_EQ("", tokens.at(1));
  ASSERT_EQ("token2", tokens.at(2));
  ASSERT_EQ("token3", tokens.at(3));

  //remove empty tokens
  tokens = OpenAB::tokenize(str2, ';', false, false);
  ASSERT_EQ(3, tokens.size());
  ASSERT_EQ("token1", tokens.at(0));
  ASSERT_EQ("token2", tokens.at(1));
  ASSERT_EQ("token3", tokens.at(2));

  //leave all tokens
  tokens = OpenAB::tokenize(str3, ' ', false, false);
  ASSERT_EQ(3, tokens.size());
  ASSERT_EQ("token1", tokens.at(0));
  ASSERT_EQ("token2", tokens.at(1));
  ASSERT_EQ("token1", tokens.at(2));

  //leave only unique tokens
  tokens = OpenAB::tokenize(str3, ' ', true, false);
  ASSERT_EQ(2, tokens.size());
  ASSERT_EQ("token1", tokens.at(0));
  ASSERT_EQ("token2", tokens.at(1));
}

TEST_F(StringHelperTests, testRemoveAllOccurences)
{
  std::string test = "<<!Test!<";
  OpenAB::eraseAllOccurences(test, '<');
  ASSERT_EQ("!Test!", test);
  OpenAB::eraseAllOccurences(test, '!');
  ASSERT_EQ("Test", test);
}

TEST_F(StringHelperTests, testBeginWith)
{
  std::string test1 = "testString1";
  std::string test2 = "someString";

  ASSERT_TRUE(OpenAB::beginsWith(test1, "test"));
  ASSERT_TRUE(OpenAB::beginsWith(test2, "some"));
  ASSERT_FALSE(OpenAB::beginsWith(test2, "test"));
  ASSERT_FALSE(OpenAB::beginsWith(test1, "some"));
}

TEST_F(StringHelperTests, testSubstituteAll)
{
  std::string test = "testtoReplaceStringtoReplace";

  OpenAB::substituteAll(test, "toReplace", "Replaced");
  ASSERT_EQ("testReplacedStringReplaced", test);
}

TEST_F(StringHelperTests, testCut)
{
  std::string test = "<a>some text</a><a>some text2</a>";

  std::string::size_type pos = 0;
  std::string cut = OpenAB::cut(test, "<a>", "</a>", pos);
  ASSERT_FALSE(pos == std::string::npos);
  ASSERT_EQ("some text", cut);

  cut = OpenAB::cut(test, "<a>", "</a>", pos);
  ASSERT_FALSE(pos == std::string::npos);
  ASSERT_EQ("some text2", cut);

  cut = OpenAB::cut(test, "<a>", "</a>", pos);
  ASSERT_TRUE(pos == std::string::npos);

  pos = 0;
  cut = OpenAB::cut(test, "<a>", "</b>", pos);
  ASSERT_TRUE(pos == std::string::npos);
  ASSERT_TRUE(cut.empty());

  pos = 0;
  cut = OpenAB::cut(test, "<b>", "</a>", pos);
  ASSERT_TRUE(pos == std::string::npos);
  ASSERT_TRUE(cut.empty());
}

TEST_F(StringHelperTests, testParseUrl)
{
  std::string url1 = "http://google.com";
  std::string url2 = "http://google.com/search";
  std::string url3 = "http://google.com/search?query=test";
  std::string url4 = "google.com";
  std::string url5 = "google.com/search";
  std::string url6 = "google.com/search?query=test";

  std::vector<std::string> parsed = OpenAB::parserURL(url1);
  ASSERT_EQ(4, parsed.size());
  ASSERT_EQ("http", parsed.at(0));
  ASSERT_EQ("google.com", parsed.at(1));
  ASSERT_EQ("", parsed.at(2));
  ASSERT_EQ("", parsed.at(3));

  parsed = OpenAB::parserURL(url2);
  ASSERT_EQ(4, parsed.size());
  ASSERT_EQ("http", parsed.at(0));
  ASSERT_EQ("google.com", parsed.at(1));
  ASSERT_EQ("search", parsed.at(2));
  ASSERT_EQ("", parsed.at(3));

  parsed = OpenAB::parserURL(url3);
  ASSERT_EQ(4, parsed.size());
  ASSERT_EQ("http", parsed.at(0));
  ASSERT_EQ("google.com", parsed.at(1));
  ASSERT_EQ("search", parsed.at(2));
  ASSERT_EQ("query=test", parsed.at(3));

  parsed = OpenAB::parserURL(url4);
  ASSERT_EQ(4, parsed.size());
  ASSERT_EQ("", parsed.at(0));
  ASSERT_EQ("google.com", parsed.at(1));
  ASSERT_EQ("", parsed.at(2));
  ASSERT_EQ("", parsed.at(3));

  parsed = OpenAB::parserURL(url5);
  ASSERT_EQ(4, parsed.size());
  ASSERT_EQ("", parsed.at(0));
  ASSERT_EQ("google.com", parsed.at(1));
  ASSERT_EQ("search", parsed.at(2));
  ASSERT_EQ("", parsed.at(3));

  parsed = OpenAB::parserURL(url6);
  ASSERT_EQ(4, parsed.size());
  ASSERT_EQ("", parsed.at(0));
  ASSERT_EQ("google.com", parsed.at(1));
  ASSERT_EQ("search", parsed.at(2));
  ASSERT_EQ("query=test", parsed.at(3));
}

TEST_F(StringHelperTests, testEndsWith)
{
	std::string t = "test";
	ASSERT_FALSE(OpenAB::endsWith(t,"hel"));
	ASSERT_TRUE(OpenAB::endsWith(t,"est"));
}

TEST_F(StringHelperTests, testUnfoldedLine)
{
	std::string text = "line 1\n line 2";
	std::string result;
	std::stringstream str(text);
	ASSERT_TRUE(OpenAB::getUnfoldedLine(str,result));
	ASSERT_EQ("line 1line 2",result);
	ASSERT_FALSE(OpenAB::getUnfoldedLine(str,result));
	text = "line 1\nline 2";
	str.str(text);
	str.clear();
	ASSERT_TRUE(OpenAB::getUnfoldedLine(str,result));
	ASSERT_EQ("line 1",result);
	ASSERT_TRUE(OpenAB::getUnfoldedLine(str,result));
	ASSERT_EQ("line 2",result);
	ASSERT_FALSE(OpenAB::getUnfoldedLine(str,result));
}

TEST_F(StringHelperTests, testlinearize)
{
  std::string textWithNewLineAndReturnAndSpace = "line 1\r\n line 2";
  std::string textWithNewLineAndSpace = "line 1\n line 2";
  OpenAB::linearize(textWithNewLineAndReturnAndSpace);
  ASSERT_EQ("line 1line 2", textWithNewLineAndReturnAndSpace);
  OpenAB::linearize(textWithNewLineAndSpace);
  ASSERT_EQ("line 1line 2", textWithNewLineAndSpace);
}

TEST_F(StringHelperTests, testUnquoteSpecialCharacters)
{
	std::string textWithSpecialChars = ":\\ hello\\,";
	OpenAB::unquoteSpecialCharacters(textWithSpecialChars);
	ASSERT_EQ(": hello,", textWithSpecialChars);
}

TEST_F(StringHelperTests, testParseUrlHostPart)
{
	std::string urlWithHost = "http://www.google.com/search?q=test";
	std::string urlWithoutHost = "www.google.com/search?q=test";
	ASSERT_EQ("http://www.google.com", OpenAB::parseURLHostPart(urlWithHost));
	ASSERT_EQ("www.google.com", OpenAB::parseURLHostPart(urlWithoutHost));
}
