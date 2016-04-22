/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file CalDAVStorage_tests.cpp
 */
#include <gtest/gtest.h>
#include <string>
#include <OpenAB.hpp>
#include <plugins/carddav/CalDAVStorage.hpp>
#include <helpers/BasicHttpAuthorizer.hpp>
#include <PIMItem/Calendar/PIMCalendarItem.hpp>
#include <OpenAB.hpp>


namespace OpenAB_Tests
{
	 std::string testEvent="BEGIN:VCALENDAR\n"
	"VERSION:2.0\n"
	"BEGIN:VEVENT\n"
	"DTSTART:20141128T150000Z\n"
	"DTEND:20141128T160000Z\n"
	"DESCRIPTION:ModifiedDescription\n"
	"SUMMARY:TestEvent1123\n"
	"UID:1234-h6d7-7h5f-8b86-4ccdca68e55c\n"
	"DTSTAMP:20141217T103142Z\n"
	"CREATED:20141217T112526Z\n"
	"LAST-MODIFIED:20141217T112526Z\n"
	"END:VEVENT\n"
	"END:VCALENDAR\n";

	 std::string testEvent2="BEGIN:VCALENDAR\n"
	"VERSION:2.0\n"
	"BEGIN:VEVENT\n"
	"DTSTART:20141128T150000Z\n"
	"DTEND:20141128T160000Z\n"
	"DESCRIPTION:ModifiedDescription\n"
	"SUMMARY:TestEvent1123\n"
	"UID:1234-s5g7-4bd1-8b86-4ccdca68e55c\n"
	"DTSTAMP:20141217T103142Z\n"
	"CREATED:20141217T112526Z\n"
	"LAST-MODIFIED:20141217T112526Z\n"
	"END:VEVENT\n"
	"END:VCALENDAR\n";

	std::string testEvent3="BEGIN:VCALENDAR\n"
	"VERSION:2.0\n"
	"BEGIN:VEVENT\n"
	"DTSTART:20141128T150000Z\n"
	"DTEND:20141128T160000Z\n"
	"DESCRIPTION:ModifiedDescription\n"
	"SUMMARY:TestEvent1123\n"
	"UID:1234-a4k9-4bd1-8b86-4ccdca68e55c\n"
	"DTSTAMP:20141217T103142Z\n"
	"CREATED:20141217T112526Z\n"
	"LAST-MODIFIED:20141217T112526Z\n"
	"END:VEVENT\n"
	"END:VCALENDAR\n";

	std::string testTask = "BEGIN:VCALENDAR\n"
	"VERSION:2.0\n"
	"BEGIN:VTODO\n"
	"DTSTAMP:19980130T134500Z\n"
    "SEQUENCE:2\n"
    "UID:caldav_loginid@intel.com\n"
    "DUE:19980415T235959\n"
    "STATUS:NEEDS-ACTION\n"
    "SUMMARY:Some summary\n"
    "END:VTODO\n"
    "END:VCALENDAR\n";



	class CalDAVStorageTests: public ::testing::Test
	{
	public:
		CalDAVStorageTests() : ::testing::Test()
		{

		}

		~CalDAVStorageTests()
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


	TEST_F(CalDAVStorageTests, testEmptyParams)
	{
	  OpenAB_Storage::Parameters p;
	  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
	  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("CalDAVCalendar"));
	  CalDAVStorage* test = (CalDAVStorage*)OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("CalDAVCalendar",p);
      ASSERT_FALSE(test);
	  OpenAB::PluginManager::getInstance().freePluginInstance(test);
	}

	TEST_F(CalDAVStorageTests, testInit)
	{
	  OpenAB_Storage::Parameters p;
	  p.setValue("server_url", "https://apidata.googleusercontent.com/caldav/v2");
	  p.setValue("client_id", "caldav_client_id.apps.googleusercontent.com");
	  p.setValue("client_secret", "caldav_client_secret");
	  p.setValue("refresh_token", "caldav_refresh_token");
	  p.setValue("item_type", OpenAB::eEvent);
	  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
	  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("CalDAVCalendar"));
	  CalDAVStorage* test = (CalDAVStorage*)OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("CalDAVCalendar", p);
	  ASSERT_TRUE(test);
	  ASSERT_EQ(test->init(), OpenAB_Storage::Storage::eInitOk);
	  OpenAB::PluginManager::getInstance().freePluginInstance(test);
	}

	TEST_F(CalDAVStorageTests, testAddObject)
	{
	  OpenAB_Storage::Parameters p;
	  p.setValue("server_url", "https://apidata.googleusercontent.com/caldav/v2");
	  p.setValue("client_id", "caldav_client_id.apps.googleusercontent.com");
	  p.setValue("client_secret", "caldav_client_secret");
	  p.setValue("refresh_token", "caldav_refresh_token");
	  p.setValue("item_type", OpenAB::eEvent);
	  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
	  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("CalDAVCalendar"));
	  CalDAVStorage* test = (CalDAVStorage*)OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("CalDAVCalendar", p);
	  ASSERT_TRUE(test);
	  ASSERT_EQ(test->init(), OpenAB_Storage::Storage::eInitOk);
	  OpenAB::PIMItem::ID newId;
	  OpenAB::PIMItem::Revision revision;
	  ASSERT_EQ(test->addObject(testEvent, newId, revision), OpenAB_Storage::Storage::eAddItemOk);
	  ASSERT_EQ(test->removeObject(newId), OpenAB_Storage::Storage::eRemoveItemOk);
	  OpenAB::PluginManager::getInstance().freePluginInstance(test);
	}

	TEST_F(CalDAVStorageTests, testAddObjects)
	{
	  OpenAB_Storage::Parameters p;
	  p.setValue("server_url", "https://apidata.googleusercontent.com/caldav/v2");
	  p.setValue("client_id", "caldav_client_id.apps.googleusercontent.com");
	  p.setValue("client_secret", "caldav_client_secret");
	  p.setValue("refresh_token", "caldav_refresh_token");
	  p.setValue("item_type", OpenAB::eEvent);
	  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
	  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("CalDAVCalendar"));
	  CalDAVStorage* test = (CalDAVStorage*)OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("CalDAVCalendar", p);
	  ASSERT_TRUE(test);
	  ASSERT_EQ(test->init(), OpenAB_Storage::Storage::eInitOk);
	  std::vector<std::string> iCals;
	  iCals.push_back(testEvent2);
	  iCals.push_back(testEvent3);
	  OpenAB::PIMItem::IDs ids;
	  OpenAB::PIMItem::Revisions revisions;
	  ASSERT_EQ(test->addObjects(iCals, ids, revisions), OpenAB_Storage::Storage::eAddItemOk);
	  ASSERT_EQ(test->removeObjects(ids), OpenAB_Storage::Storage::eRemoveItemOk);
	  OpenAB::PluginManager::getInstance().freePluginInstance(test);
	}

	TEST_F(CalDAVStorageTests, tesModifyObject)
	{
	  OpenAB_Storage::Parameters p;
	  p.setValue("server_url", "https://caldav.icloud.com");
	  p.setValue("login", "caldav_loginid@icloud.com");
	  p.setValue("password", "caldav_icloud_password");
	  p.setValue("item_type", OpenAB::eEvent);
	  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
	  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("CalDAVCalendar"));
	  CalDAVStorage* test = (CalDAVStorage*)OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("CalDAVCalendar", p);
	  ASSERT_TRUE(test);
	  ASSERT_EQ(test->init(), OpenAB_Storage::Storage::eInitOk);
	  OpenAB::PIMItem::ID newId;
	  OpenAB::PIMItem::Revision revision;
	  ASSERT_EQ(test->addObject(testEvent, newId, revision), OpenAB_Storage::Storage::eAddItemOk);
	  ASSERT_EQ(test->modifyObject(testEvent, newId, revision), OpenAB_Storage::Storage::eModifyItemOk);
	  ASSERT_EQ(test->removeObject(newId), OpenAB_Storage::Storage::eRemoveItemOk);
	  OpenAB::PluginManager::getInstance().freePluginInstance(test);
	}

	TEST_F(CalDAVStorageTests, testModifyObjects)
	{
	  OpenAB_Storage::Parameters p;
	  p.setValue("server_url", "https://caldav.icloud.com");
	  p.setValue("login", "caldav_loginid@icloud.com");
	  p.setValue("password", "caldav_icloud_password");
	  p.setValue("item_type", OpenAB::eEvent);
	  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
	  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("CalDAVCalendar"));
	  CalDAVStorage* test = (CalDAVStorage*)OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("CalDAVCalendar", p);
	  ASSERT_TRUE(test);
	  ASSERT_EQ(test->init(), OpenAB_Storage::Storage::eInitOk);
	  std::vector<std::string> iCals;
	  iCals.push_back(testEvent2);
	  iCals.push_back(testEvent3);
	  OpenAB::PIMItem::IDs ids;
	  OpenAB::PIMItem::Revisions revisions;
	  ASSERT_EQ(test->addObjects(iCals, ids, revisions), OpenAB_Storage::Storage::eAddItemOk);
	  ASSERT_EQ(test->modifyObjects(iCals, ids, revisions), OpenAB_Storage::Storage::eModifyItemOk);
	  ASSERT_EQ(test->removeObjects(ids), OpenAB_Storage::Storage::eRemoveItemOk);
	  OpenAB::PluginManager::getInstance().freePluginInstance(test);
	}

	TEST_F(CalDAVStorageTests, testRemoveObject)
	{
	  OpenAB_Storage::Parameters p;
	  p.setValue("server_url", "https://apidata.googleusercontent.com/caldav/v2");
	  p.setValue("client_id", "caldav_client_id.apps.googleusercontent.com");
	  p.setValue("client_secret", "caldav_client_secret");
	  p.setValue("refresh_token", "caldav_refresh_token");
 	  p.setValue("item_type", OpenAB::eEvent);
	  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
	  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("CalDAVCalendar"));
	  CalDAVStorage* test = (CalDAVStorage*)OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("CalDAVCalendar", p);
	  ASSERT_TRUE(test);
	  ASSERT_EQ(test->init(), OpenAB_Storage::Storage::eInitOk);
	  OpenAB::PIMItem::ID newId;
	  OpenAB::PIMItem::Revision revision;
	  ASSERT_EQ(test->addObject(testEvent, newId, revision), OpenAB_Storage::Storage::eAddItemOk);
	  ASSERT_EQ(test->removeObject(newId), OpenAB_Storage::Storage::eRemoveItemOk);
	  OpenAB::PluginManager::getInstance().freePluginInstance(test);
	}

	TEST_F(CalDAVStorageTests, testRemoveObjects)
	{
	  OpenAB_Storage::Parameters p;
	  p.setValue("server_url", "https://apidata.googleusercontent.com/caldav/v2");
	  p.setValue("client_id", "caldav_client_id.apps.googleusercontent.com");
	  p.setValue("client_secret", "caldav_client_secret");
	  p.setValue("refresh_token", "caldav_refresh_token");
	  p.setValue("item_type", OpenAB::eEvent);
	  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
	  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("CalDAVCalendar"));
	  CalDAVStorage* test = (CalDAVStorage*)OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("CalDAVCalendar", p);
	  ASSERT_TRUE(test);
	  ASSERT_EQ(test->init(), OpenAB_Storage::Storage::eInitOk);
	  std::vector<std::string> iCals;
	  iCals.push_back(testEvent2);
	  iCals.push_back(testEvent3);
	  OpenAB::PIMItem::IDs ids;
	  OpenAB::PIMItem::Revisions revisions;
	  ASSERT_EQ(test->addObjects(iCals, ids, revisions), OpenAB_Storage::Storage::eAddItemOk);
	  ASSERT_EQ(test->removeObjects(ids), OpenAB_Storage::Storage::eRemoveItemOk);
	  OpenAB::PluginManager::getInstance().freePluginInstance(test);
	}

	TEST_F(CalDAVStorageTests, testGetEvent)
	{
	  OpenAB_Storage::Parameters p;
	  p.setValue("server_url", "https://apidata.googleusercontent.com/caldav/v2");
	  p.setValue("client_id", "caldav_client_id.apps.googleusercontent.com");
	  p.setValue("client_secret", "caldav_client_secret");
	  p.setValue("refresh_token", "caldav_refresh_token");
	  p.setValue("item_type", OpenAB::eEvent);
	  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
	  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("CalDAVCalendar"));
	  CalDAVStorage* test = (CalDAVStorage*)OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("CalDAVCalendar", p);
	  ASSERT_TRUE(test);
	  ASSERT_EQ(test->init(), OpenAB_Storage::Storage::eInitOk);
	  OpenAB::PIMItem::ID newId;
	  OpenAB::PIMItem::Revision revision;
	  ASSERT_EQ(test->addObject(testEvent, newId, revision), OpenAB_Storage::Storage::eAddItemOk);
	  OpenAB::SmartPtr<OpenAB::PIMCalendarEventItem> event;
	  ASSERT_EQ(test->getEvent(newId, event), OpenAB_Storage::Storage::eGetItemOk);
	  ASSERT_EQ(test->removeObject(newId), OpenAB_Storage::Storage::eRemoveItemOk);
	  OpenAB::PluginManager::getInstance().freePluginInstance(test);
	}

	TEST_F(CalDAVStorageTests, testGetEvents)
	{
	  OpenAB_Storage::Parameters p;
	  p.setValue("server_url", "https://apidata.googleusercontent.com/caldav/v2");
	  p.setValue("client_id", "caldav_client_id.apps.googleusercontent.com");
	  p.setValue("client_secret", "caldav_client_secret");
	  p.setValue("refresh_token", "caldav_refresh_token");
	  p.setValue("item_type", OpenAB::eEvent);
	  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
	  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("CalDAVCalendar"));
	  CalDAVStorage* test = (CalDAVStorage*)OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("CalDAVCalendar", p);
	  ASSERT_TRUE(test);
	  ASSERT_EQ(test->init(), OpenAB_Storage::Storage::eInitOk);
	  std::vector<std::string> iCals;
	  iCals.push_back(testEvent2);
	  iCals.push_back(testEvent3);
	  OpenAB::PIMItem::IDs ids;
	  OpenAB::PIMItem::Revisions revisions;
	  ASSERT_EQ(test->addObjects(iCals, ids, revisions), OpenAB_Storage::Storage::eAddItemOk);
	  std::vector<OpenAB::SmartPtr<OpenAB::PIMCalendarEventItem> > events;
	  ASSERT_EQ(test->getEvents(ids, events), OpenAB_Storage::Storage::eGetItemOk);
	  ASSERT_EQ(test->removeObjects(ids), OpenAB_Storage::Storage::eRemoveItemOk);
	  OpenAB::PluginManager::getInstance().freePluginInstance(test);
	}

	//not working must get correct task format
	/*TEST_F(CalDAVStorageTests, testGetTask)
	{
	  OpenAB_Storage::Parameters p;
	  p.setValue("server_url", "https://apidata.googleusercontent.com/caldav/v2");
	  p.setValue("client_id", "caldav_client_id.apps.googleusercontent.com");
	  p.setValue("client_secret", "caldav_client_secret");
	  p.setValue("refresh_token", "caldav_refresh_token");
	  p.setValue("item_type", OpenAB::eEvent);
	  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
	  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("CalDAVCalendar"));
	  CalDAVStorage* test = (CalDAVStorage*)OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("CalDAVCalendar", p);
	  ASSERT_TRUE(test);
	  ASSERT_EQ(test->init(), OpenAB_Storage::Storage::eInitOk);
	  OpenAB::PIMItem::ID newId;
	  OpenAB::PIMItem::Revision revision;
	  ASSERT_EQ(test->addObject(testTask, newId, revision), OpenAB_Storage::Storage::eAddItemOk);
	  OpenAB::SmartPtr<OpenAB::PIMCalendarTaskItem> task;
	  ASSERT_EQ(test->getTask(newId, task), OpenAB_Storage::Storage::eGetItemOk);
	  ASSERT_EQ(test->removeObject(newId), OpenAB_Storage::Storage::eRemoveItemOk);
	  OpenAB::PluginManager::getInstance().freePluginInstance(test);
	}*/


	TEST_F(CalDAVStorageTests, testGetRevisions)
	{
	  OpenAB_Storage::Parameters p;
	  p.setValue("server_url", "https://apidata.googleusercontent.com/caldav/v2");
	  p.setValue("client_id", "caldav_client_id.apps.googleusercontent.com");
	  p.setValue("client_secret", "caldav_client_secret");
	  p.setValue("refresh_token", "caldav_refresh_token");
	  p.setValue("item_type", OpenAB::eEvent);
	  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
	  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("CalDAVCalendar"));
	  CalDAVStorage* test = (CalDAVStorage*)OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("CalDAVCalendar", p);
	  ASSERT_TRUE(test);
	  ASSERT_EQ(test->init(), OpenAB_Storage::Storage::eInitOk);
	  OpenAB::PIMItem::ID newId;
	  OpenAB::PIMItem::Revision revision;
	  ASSERT_EQ(test->addObject(testEvent, newId, revision), OpenAB_Storage::Storage::eAddItemOk);
	  std::map<std::string, std::string>revisions;
	  ASSERT_EQ(test->getRevisions(revisions), OpenAB_Storage::Storage::eGetRevisionsOk);
	  ASSERT_EQ(test->removeObject(newId), OpenAB_Storage::Storage::eRemoveItemOk);
	  OpenAB::PluginManager::getInstance().freePluginInstance(test);
	}


	TEST_F(CalDAVStorageTests, testGeLatestSyncToken)
	{
	  OpenAB_Storage::Parameters p;
	  p.setValue("server_url", "https://apidata.googleusercontent.com/caldav/v2");
	  p.setValue("client_id", "caldav_client_id.apps.googleusercontent.com");
	  p.setValue("client_secret", "caldav_client_secret");
	  p.setValue("refresh_token", "caldav_refresh_token");
	  p.setValue("item_type", OpenAB::eEvent);
	  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
	  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("CalDAVCalendar"));
	  CalDAVStorage* test = (CalDAVStorage*)OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("CalDAVCalendar", p);
	  ASSERT_TRUE(test);
	  ASSERT_EQ(test->init(), OpenAB_Storage::Storage::eInitOk);
	  OpenAB::PIMItem::ID newId;
	  OpenAB::PIMItem::Revision revision;
	  ASSERT_EQ(test->addObject(testEvent, newId, revision), OpenAB_Storage::Storage::eAddItemOk);
	  std::string token;
	  ASSERT_EQ(test->getLatestSyncToken(token), OpenAB_Storage::Storage::eGetSyncTokenOk);
	  ASSERT_NE(token, "");
	  ASSERT_EQ(test->removeObject(newId), OpenAB_Storage::Storage::eRemoveItemOk);
	  OpenAB::PluginManager::getInstance().freePluginInstance(test);
	}

	TEST_F(CalDAVStorageTests, testGeChangedRevisions)
	{
	  OpenAB_Storage::Parameters p;
	  p.setValue("server_url", "https://apidata.googleusercontent.com/caldav/v2");
	  p.setValue("client_id", "caldav_client_id.apps.googleusercontent.com");
	  p.setValue("client_secret", "caldav_client_secret");
	  p.setValue("refresh_token", "caldav_refresh_token");
	  p.setValue("item_type", OpenAB::eEvent);
	  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
	  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("CalDAVCalendar"));
	  CalDAVStorage* test = (CalDAVStorage*)OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("CalDAVCalendar", p);
	  ASSERT_TRUE(test);
	  ASSERT_EQ(test->init(), OpenAB_Storage::Storage::eInitOk);
	  OpenAB::PIMItem::ID newId;
	  OpenAB::PIMItem::Revision revision;
	  ASSERT_EQ(test->addObject(testEvent, newId, revision), OpenAB_Storage::Storage::eAddItemOk);
	  std::string token;
	  std::map<std::string, std::string> revisions;
	  std::vector<OpenAB::PIMItem::ID> removed;
	  ASSERT_EQ(test->getLatestSyncToken(token), OpenAB_Storage::Storage::eGetSyncTokenOk);
	  ASSERT_EQ(test->getChangedRevisions(token, revisions, removed), OpenAB_Storage::Storage::eGetRevisionsOk);
	  ASSERT_EQ(test->removeObject(newId), OpenAB_Storage::Storage::eRemoveItemOk);
	  OpenAB::PluginManager::getInstance().freePluginInstance(test);
	}

	/*TEST_F(CalDAVStorageTests, testNewStorageItemIterator)
	{
	  OpenAB_Storage::Parameters p;
	  p.setValue("server_url", "https://apidata.googleusercontent.com/caldav/v2");
	  p.setValue("client_id", "caldav_client_id.apps.googleusercontent.com");
	  p.setValue("client_secret", "caldav_client_secret");
	  p.setValue("refresh_token", "caldav_refresh_token");
	  p.setValue("item_type", OpenAB::eEvent);
	  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
	  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("CalDAVCalendar"));
	  CalDAVStorage* test = (CalDAVStorage*)OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("CalDAVCalendar", p);
	  ASSERT_TRUE(test);
	  ASSERT_EQ(test->init(), OpenAB_Storage::Storage::eInitOk);
	  OpenAB_Storage::StorageItemIterator* it = test->newStorageItemIterator();
	  ASSERT_TRUE(it);
	  std::cout << "************** test true ***************" << std::endl;
	 // delete it;
	  OpenAB::PluginManager::getInstance().freePluginInstance(test);
	  std::cout << "*********** end of tests **********" << std::endl;
	}*/


}
