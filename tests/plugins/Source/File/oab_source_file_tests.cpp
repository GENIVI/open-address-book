/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */
/**
 * @file oab_source_file_tests.cpp
 */
#include <gtest/gtest.h>
#include <string>
#include "helpers/PluginManager.hpp"


class FileSourceTest: public ::testing::Test
{
public:
    FileSourceTest() : ::testing::Test()
    {
    }

    ~FileSourceTest()
    {
    }

protected:
    // Sets up the test fixture.
    virtual void SetUp()
    {
      OpenAB::Logger::OutLevel() = OpenAB::Logger::Debug;
    }

    // Tears down the test fixture.
    virtual void TearDown()
    {

    }
};

TEST_F(FileSourceTest, testEmptyParams)
{
  OpenAB_Source::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("File", p);
  ASSERT_FALSE(s);
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(FileSourceTest, testNonExistingFile)
{
  OpenAB_Source::Parameters p;
  p.setValue("filename", "someNonexistingFile");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("File", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitFail, s->init());
  ASSERT_EQ(0, s->getTotalCount());
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(FileSourceTest, testSingleVCard)
{
  OpenAB_Source::Parameters p;
  p.setValue("filename", "./vcard_single.vcf");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("File", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitOk, s->init());
  ASSERT_EQ(1, s->getTotalCount());

  OpenAB::SmartPtr<OpenAB::PIMItem> item;
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetOk, s->getItem(item));
  ASSERT_TRUE(item.getPointer());
  std::string vCard = item->getRawData();
  ASSERT_EQ("BEGIN:VCARD\nVERSION:3.0\nN:Surname;Name;Middle;Prefix;Suffix\nEND:VCARD\n", vCard);
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetEnd, s->getItem(item));

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(FileSourceTest, testIncorrectVCardFile)
{
  OpenAB_Source::Parameters p;
  p.setValue("filename", "./vcard_incorrect.vcf");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("File", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitFail, s->init());
  ASSERT_EQ(0, s->getTotalCount());
  OpenAB::SmartPtr<OpenAB::PIMItem> item;
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetEnd, s->getItem(item));
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(FileSourceTest, testMisformattedVCardFile)
{
  OpenAB_Source::Parameters p;
  p.setValue("filename", "./vcard_misformatted.vcf");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("File", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitOk, s->init());
  ASSERT_EQ(3, s->getTotalCount());
  OpenAB::SmartPtr<OpenAB::PIMItem> item;
  //second vcard is missformatted and won't parse - file source will return error
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetOk, s->getItem(item));
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetError, s->getItem(item));
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetOk, s->getItem(item));
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetEnd, s->getItem(item));
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetEnd, s->getItem(item));
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(FileSourceTest, testDirectoryOfVCards)
{
  OpenAB_Source::Parameters p;
  p.setValue("filename", "./vcards");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("File", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitOk, s->init());
  ASSERT_EQ(2, s->getTotalCount());
  OpenAB::SmartPtr<OpenAB::PIMItem> item;
  //there are two vcard files in directory
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetOk, s->getItem(item));
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetOk, s->getItem(item));
  ASSERT_EQ(OpenAB_Source::Source::eGetItemRetEnd, s->getItem(item));
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(FileSourceTest, testSuspend)
{
  OpenAB_Source::Parameters p;
  p.setValue("filename", "./vcard_single.vcf");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("File", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitOk, s->init());
  ASSERT_EQ(OpenAB_Source::Source::eSuspendRetNotSupported, s->suspend());
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(FileSourceTest, testResume)
{
  OpenAB_Source::Parameters p;
  p.setValue("filename", "./vcard_single.vcf");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("File", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitOk, s->init());
  ASSERT_EQ(OpenAB_Source::Source::eResumeRetNotSupported, s->resume());
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(FileSourceTest, testCancel)
{
  OpenAB_Source::Parameters p;
  p.setValue("filename", "./vcard_single.vcf");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("File", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Source::Source::eInitOk, s->init());
  ASSERT_EQ(OpenAB_Source::Source::eCancelRetNotSupported, s->cancel());
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

