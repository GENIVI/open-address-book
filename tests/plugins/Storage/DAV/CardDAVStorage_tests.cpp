/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file CardDAVStorage_tests.cpp
 */
#include <gtest/gtest.h>
#include <string>
#include <OpenAB.hpp>
#include <plugins/carddav/CardDAVStorage.hpp>
#include <helpers/BasicHttpAuthorizer.hpp>


namespace OpenAB_Tests {

	class CardDAVStorageTests: public ::testing::Test
	{
	public:
		CardDAVStorageTests() : ::testing::Test()
		{

		}

		~CardDAVStorageTests()
		{
		}

	protected:
		virtual void SetUp()
		{
			OpenAB::Logger::setDefaultLogger(NULL);
			OpenAB::Logger::OutLevel() = OpenAB::Logger::Error;
		}

		virtual void TearDown()
		{

		}

	};

	TEST_F(CardDAVStorageTests, testEmptyParams)
	{
		OpenAB_Storage::Parameters p;
		OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
		ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("CardDAV"));
		CardDAVStorage* test = (CardDAVStorage*)OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("CardDAV",p);
		ASSERT_FALSE(test);
		OpenAB::PluginManager::getInstance().freePluginInstance(test);
	}


	TEST_F(CardDAVStorageTests, testInitCorrect)
	{
	  OpenAB_Storage::Parameters p;
	  p.setValue("server_url", "https://www.googleapis.com/.well-known/carddav");
	  p.setValue("login", "test.emailaddress@gmail.com");
	  p.setValue("password", "test_email_password");
	  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
	  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("CardDAV"));
	  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("CardDAV", p);
	  ASSERT_TRUE(s);
	  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());
	  OpenAB::PluginManager::getInstance().freePluginInstance(s);
	}
}
