/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */
/**
 * @file  http_tests.cpp
 */
#include <gtest/gtest.h>
#include <string>
#include "helpers/Http.hpp"
#include "helpers/BasicHttpAuthorizer.hpp"

class HttpTests: public ::testing::Test
{
public:
    HttpTests() : ::testing::Test()
    {
    }

    ~HttpTests()
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

TEST_F(HttpTests, testInit)
{
  OpenAB::HttpSession session;
  ASSERT_TRUE(session.init());

  session.cleanup();
}

TEST_F(HttpTests, testGetters)
{
  OpenAB::HttpMessage msg;
  msg.setURL("http://httpbin.org/get");
  ASSERT_EQ("http://httpbin.org/get", msg.getURL());

  //custom request type
  msg.setRequestType("PROPFIND");
  ASSERT_EQ(OpenAB::HttpMessage::CUSTOM, msg.getRequestType());
  ASSERT_EQ("PROPFIND", msg.getCustomRequestType());

  msg.setRequestType(OpenAB::HttpMessage::GET);
  ASSERT_EQ(OpenAB::HttpMessage::GET, msg.getRequestType());

  msg.setFollowRedirection(true);
  ASSERT_EQ(true, msg.followRedirection());

  msg.setData("hello");
  ASSERT_EQ("hello", msg.getData());

  ASSERT_EQ("", msg.getErrorString());
  ASSERT_EQ(0, msg.getResponseCode());
  ASSERT_EQ("", msg.getResponse());

  ASSERT_TRUE(msg.getHeaders().empty());

  msg.appendHeader("Authorizer", "Basic 123123123");
  ASSERT_EQ(1, msg.getHeaders().size());
  ASSERT_EQ("Authorizer", msg.getHeaders().at(0).first);
  ASSERT_EQ("Basic 123123123", msg.getHeaders().at(0).second);
}

TEST_F(HttpTests, DISABLED_testGetMessage)
{
  OpenAB::HttpSession session;
  ASSERT_TRUE(session.init());

  OpenAB::HttpMessage msg;
  msg.setURL("http://httpbin.org/get");
  msg.setRequestType(OpenAB::HttpMessage::GET);
  msg.setFollowRedirection(true);
  msg.setData("hello");

  ASSERT_TRUE(session.execute(&msg));
  ASSERT_EQ(200, msg.getResponseCode());
  ASSERT_TRUE(msg.getResponse().size() > 0);

  session.cleanup();
}

TEST_F(HttpTests, DISABLED_testBasicAuth)
{
  OpenAB::HttpSession session;
  ASSERT_TRUE(session.init());

  OpenAB::HttpMessage msg;
  msg.setURL("http://httpbin.org//basic-auth/test/test");
  msg.setRequestType(OpenAB::HttpMessage::GET);
  msg.setFollowRedirection(true);

  OpenAB::BasicHttpAuthorizer authorizer;
  authorizer.setCredentials("test", "test");

  authorizer.authorizeMessage(&msg);

  ASSERT_TRUE(session.execute(&msg));
  ASSERT_EQ(200, msg.getResponseCode());
  ASSERT_TRUE(msg.getResponse().size() > 0);

  authorizer.setCredentials("test", "test2");
  authorizer.authorizeMessage(&msg);
  ASSERT_TRUE(session.execute(&msg));
  ASSERT_NE(200, msg.getResponseCode());

  session.cleanup();
}

TEST_F(HttpTests, testBasicHttpAuthenticationEnabled)
{
	OpenAB::HttpMessage msg;
	ASSERT_FALSE(msg.basicHttpAuthenticationEnabled());
	msg.enableBasicHttpAuthentication(true);
	ASSERT_TRUE(msg.basicHttpAuthenticationEnabled());
	ASSERT_FALSE(msg.digestHttpAuthenticationEnabled());
}

TEST_F(HttpTests, testEnableDigestHttpAuthentication)
{
	OpenAB::HttpMessage msg;
	msg.enableDigestHttpAuthentication(true);
	ASSERT_TRUE(msg.digestHttpAuthenticationEnabled());
}

TEST_F(HttpTests, testResponseCodeDescription)
{
	OpenAB::HttpMessage msg;
	ASSERT_EQ("The request has succeeded", msg.responseCodeDescription(200));
	ASSERT_EQ("The request has succeeded and a new resource was created", msg.responseCodeDescription(201));
	ASSERT_EQ("The request was accepted for processing", msg.responseCodeDescription(202));
	ASSERT_EQ("The request was fulfilled, but no content was returned", msg.responseCodeDescription(204));
	ASSERT_EQ("The request has succeeded and WebDAV multistatus XML was returned", msg.responseCodeDescription(207));
	ASSERT_EQ("The requested resource has been moved to new URI", msg.responseCodeDescription(301));
	ASSERT_EQ("The server does not understood request", msg.responseCodeDescription(400));
	ASSERT_EQ("The request requires user to be authenticated", msg.responseCodeDescription(401));
	ASSERT_EQ("The server rejects request", msg.responseCodeDescription(403));
	ASSERT_EQ("The requested URI wasn't found", msg.responseCodeDescription(404));
	ASSERT_EQ("The precondition given in request was not met", msg.responseCodeDescription(412));
	ASSERT_EQ("Unknown code", msg.responseCodeDescription(111));
}

TEST_F(HttpTests, testSetErrorString)
{
	OpenAB::HttpMessage msg;
	const std::string errorCode = "Some error code";
	msg.setErrorString(errorCode);
	ASSERT_EQ(errorCode, msg.getErrorString());
}

TEST_F(HttpTests, testCredentials)
{
	OpenAB::HttpMessage msg;
	std::string login = "uName";
	std::string passw = "password";
	msg.setCredentials(login, passw);
	//ASSERT_EQ((login,passw), msg.getCredentials(login,passw));
}
















