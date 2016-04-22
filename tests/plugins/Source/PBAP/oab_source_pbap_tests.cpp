/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file oab_source_pbap_tests.cpp 
 */
#include <gtest/gtest.h>
#include <string>
#include <sys/prctl.h>
#include "helpers/PluginManager.hpp"


class PBAPSourceTest: public ::testing::Test
{
public:
    PBAPSourceTest() : ::testing::Test(),
      obexMockPid(-1)
    {
    }

    ~PBAPSourceTest()
    {
    }

protected:
    // Sets up the test fixture.
    virtual void SetUp()
    {
      OpenAB::Logger::OutLevel() = OpenAB::Logger::Debug;
      //start mock obex process
      obexMockPid = -1;
      int pid = fork();
      if (pid == 0)
      {
        prctl(PR_SET_PDEATHSIG, SIGTERM);
        execl("obex_mock.py", "./obex_mock.py", (char*) 0);
      }
      obexMockPid = pid;

      //take some time to setup mock
      sleep(1);

      //check if mock process was started
      int wasProcessCreated = kill(obexMockPid, 0);
      ASSERT_EQ(0, wasProcessCreated);
    }

    // Tears down the test fixture.
    virtual void TearDown()
    {
      if(obexMockPid != -1)
      {
        kill(obexMockPid, SIGTERM);
      }
    }

    pid_t obexMockPid;
};

#define GET_SIZE_FAILURE_MAC "22:22:22:22:22:22"
#define GET_FILTERS_FAILURE_MAC "11:11:11:11:11:11"
#define CREATE_SESSION_FAILURE_MAC "DE:AD:DE:AD:DE:AD"
#define PULL_ALL_FAILURE_MAC "33:33:33:33:33:33"
#define MISFORMATTED_VCARD_FAILURE_MAC "44:44:44:44:44:44"
#define CANCEL_FAILURE_MAC "55:55:55:55:55:55"
#define NORMAL_VCARD_MAC "12:34:56:78:90:12"

TEST_F(PBAPSourceTest, testEmptyParams)
{
  OpenAB_Source::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("PBAP", p);
  ASSERT_FALSE(s);
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(PBAPSourceTest, testInit)
{
  OpenAB_Source::Parameters p;
  p.setValue("MAC", NORMAL_VCARD_MAC);
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("PBAP", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitOk, s->init());
  ASSERT_EQ(1, s->getTotalCount());
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(PBAPSourceTest, testInitFailure)
{
  OpenAB_Source::Parameters p;
  p.setValue("MAC", CREATE_SESSION_FAILURE_MAC); //special use case for MAC = DE:AD:DE:AD:DE:AD obex mock will trigger exception
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("PBAP", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitFail, s->init());
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}


TEST_F(PBAPSourceTest, testSelectPhonebookFailure)
{
  OpenAB_Source::Parameters p;
  p.setValue("MAC", "12:34:56:78:90:12");
  p.setValue("loc", "sim3"); //special use case for loc = sim3 obex mock will trigger exception
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("PBAP", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitFail, s->init());
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(PBAPSourceTest, testListFiltersFailure)
{
  OpenAB_Source::Parameters p;
  p.setValue("MAC", GET_FILTERS_FAILURE_MAC); //special use case for MAC = 11:11:11:11:11:11 obex mock will trigger exception
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("PBAP", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitFail, s->init());
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(PBAPSourceTest, testGetSizeFailure)
{
  OpenAB_Source::Parameters p;
  p.setValue("MAC", GET_SIZE_FAILURE_MAC); //special use case for MAC = 22:22:22:22:22:22 obex mock will trigger exception
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("PBAP", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitFail, s->init());
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(PBAPSourceTest, testPullAllFailure)
{
  OpenAB_Source::Parameters p;
  p.setValue("MAC", PULL_ALL_FAILURE_MAC); //special use case for MAC = 33:33:33:33:33:33 obex mock will trigger exception
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("PBAP", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitOk, s->init());
  OpenAB::SmartPtr<OpenAB::PIMItem> item;
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetError, s->getItem(item));
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(PBAPSourceTest, testGetItem)
{
  OpenAB_Source::Parameters p;
  p.setValue("MAC", NORMAL_VCARD_MAC);
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("PBAP", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitOk, s->init());
  OpenAB::SmartPtr<OpenAB::PIMItem> item;
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetOk, s->getItem(item));
  ASSERT_TRUE(item.getPointer());
  ASSERT_NE("", item->getRawData());
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetEnd, s->getItem(item));

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(PBAPSourceTest, testGetItemWithIgnoredPhotoField)
{
  OpenAB_Source::Parameters p;
  p.setValue("MAC", NORMAL_VCARD_MAC);
  p.setValue("ignore_fields", "PHOTO");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("PBAP", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitOk, s->init());
  OpenAB::SmartPtr<OpenAB::PIMItem> item;
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetOk, s->getItem(item));
  ASSERT_TRUE(item.getPointer());
  ASSERT_NE("", item->getRawData());
  ASSERT_EQ(std::string::npos, item->getRawData().find("PHOTO"));
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetEnd, s->getItem(item));

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(PBAPSourceTest, testGetItemWithMultipleIgnoredFields)
{
  OpenAB_Source::Parameters p;
  p.setValue("MAC", NORMAL_VCARD_MAC);
  //check also invalid fields
  p.setValue("ignore_fields", "PHOTO,ADR, TEL, SOME_UNKNOW_FIELD");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("PBAP", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitOk, s->init());
  OpenAB::SmartPtr<OpenAB::PIMItem> item;
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetOk, s->getItem(item));
  ASSERT_TRUE(item.getPointer());
  ASSERT_NE("", item->getRawData());
  ASSERT_EQ(std::string::npos, item->getRawData().find("PHOTO"));
  ASSERT_EQ(std::string::npos, item->getRawData().find("ADR"));
  ASSERT_EQ(std::string::npos, item->getRawData().find("TEL"));
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetEnd, s->getItem(item));

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(PBAPSourceTest, testMisformattedVCard)
{
  OpenAB_Source::Parameters p;
  p.setValue("MAC", MISFORMATTED_VCARD_FAILURE_MAC);
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("PBAP", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitOk, s->init());
  OpenAB::SmartPtr<OpenAB::PIMItem> item;
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetOk, s->getItem(item));
  ASSERT_TRUE(item.getPointer());
  ASSERT_NE("", item->getRawData());
  item = NULL;
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetError, s->getItem(item));
  ASSERT_FALSE(item.getPointer());
  item = NULL;
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetOk, s->getItem(item));
  ASSERT_TRUE(item.getPointer());
  ASSERT_NE("", item->getRawData());
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetEnd, s->getItem(item));

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}


TEST_F(PBAPSourceTest, testSuspendResume)
{
  OpenAB_Source::Parameters p;
  p.setValue("MAC", NORMAL_VCARD_MAC);
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("PBAP", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitOk, s->init());
  usleep(100000);
  ASSERT_EQ(OpenAB_Source::Source::eSuspendRetOk, s->suspend());
  ASSERT_EQ(OpenAB_Source::Source::eResumeRetOk, s->resume());

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(PBAPSourceTest, testSuspendResumeFailure)
{
  OpenAB_Source::Parameters p;
  p.setValue("MAC", CANCEL_FAILURE_MAC);
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("PBAP", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitOk, s->init());
  ASSERT_EQ(OpenAB_Source::Source::eSuspendRetFail, s->suspend());
  ASSERT_EQ(OpenAB_Source::Source::eResumeRetFail, s->resume());

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(PBAPSourceTest, testCancel)
{
  OpenAB_Source::Parameters p;
  p.setValue("MAC", NORMAL_VCARD_MAC);
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("PBAP", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitOk, s->init());

  usleep(100000);
  ASSERT_EQ(OpenAB_Source::Source::eCancelRetOk, s->cancel());
  OpenAB::SmartPtr<OpenAB::PIMItem> item;
  //after cancelling next call to getItem should return error
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetError, s->getItem(item));
  ASSERT_FALSE(item.getPointer());

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(PBAPSourceTest, testCancelFailure)
{
  OpenAB_Source::Parameters p;
  p.setValue("MAC", CANCEL_FAILURE_MAC);
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("PBAP", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitOk, s->init());

  sleep(1);
  ASSERT_EQ(OpenAB_Source::Source::eCancelRetFail, s->cancel());
  OpenAB::SmartPtr<OpenAB::PIMItem> item;
  //after cancelling next call to getItem should return error
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetOk, s->getItem(item));
  ASSERT_TRUE(item.getPointer());

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

