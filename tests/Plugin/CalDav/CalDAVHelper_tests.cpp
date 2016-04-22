/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */
/**
 * @file CalDAVHelper_tests.cpp
 */
#include <gtest/gtest.h>
#include <OpenAB.hpp>
#include <plugins/carddav/CalDAVHelper.hpp>
#include <helpers/BasicHttpAuthorizer.hpp>
#include <helpers/OAuth2HttpAuthorizer.hpp>

namespace OpenAB_Tests {

	 std::string testEvent="BEGIN:VCALENDAR\n"
	"VERSION:2.0\n"
	"BEGIN:VEVENT\n"
	"DTSTART:20141128T150000Z\n"
	"DTEND:20141128T160000Z\n"
	"DESCRIPTION:ModifiedDescription\n"
	"SUMMARY:TestEvent1123\n"
	"UID:1234-d7h6-4bd1-8b86-4ccdca68e55c\n"
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
	"UID:1234-a5g7-4bd1-8b86-4ccdca68e55c\n"
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
	"UID:1234-l8g6-4bd1-8b86-4ccdca68e55c\n"
	"DTSTAMP:20141217T103142Z\n"
	"CREATED:20141217T112526Z\n"
	"LAST-MODIFIED:20141217T112526Z\n"
	"END:VEVENT\n"
	"END:VCALENDAR\n";

	std::string failEvent="BEGIN:VCALENDAR\n"
	"VERSION:2.0\n"
	"BEGIN:VEVENT\n"
	"DTSTART:20141128T150000Z\n"
	"DTEND:20141128T160000Z\n"
	"DESCRIPTION:ModifiedDescription\n"
	"SUMMARY:TestEvent1123\n"
	"UID:\n"
	"DTSTAMP:20141217T103142Z\n"
	"CREATED:20141217T112526Z\n"
	"LAST-MODIFIED:20141217T112526Z\n"
	"END:VEVENT\n"
	"END:VCALENDAR\n";

	class CalDAVHelperTests: public ::testing::Test
	{
	public:
		CalDAVHelperTests() : ::testing::Test()
		{

		}

		~CalDAVHelperTests()
		{
		}

	protected:
		virtual void SetUp()
		{
			OpenAB::Logger::setDefaultLogger(NULL);
			OpenAB::Logger::OutLevel() = OpenAB::Logger::Debug;
		}

		virtual void TearDown()
		{

		}

	};

	TEST_F(CalDAVHelperTests, testInit)
	{
	  const std::string url = "https://caldav.icloud.com";
	  OpenAB::HttpSession httpSession;
	  httpSession.init();
	  OpenAB::HttpAuthorizer* httpAuth = NULL;
	  httpAuth = new OpenAB::BasicHttpAuthorizer();
	  ((OpenAB::BasicHttpAuthorizer*)httpAuth)->setCredentials("caldav_loginid@icloud.com", "caldav_icloud_password");
	  CalDAVHelper test(url, false, &httpSession, httpAuth);
	  ASSERT_TRUE(test.findPrincipalUrl());
	  ((OpenAB::BasicHttpAuthorizer*)httpAuth)->setCredentials("","");
	  CalDAVHelper fail(url, false, &httpSession, httpAuth);
	  ASSERT_FALSE(fail.findPrincipalUrl());
	}

	TEST_F(CalDAVHelperTests, testFindCalendarSet)
	{
	  const std::string url = "https://caldav.icloud.com";
	  OpenAB::HttpSession httpSession;
	  httpSession.init();
	  OpenAB::HttpAuthorizer* httpAuth = NULL;
	  httpAuth = new OpenAB::BasicHttpAuthorizer();
	  ((OpenAB::BasicHttpAuthorizer*)httpAuth)->setCredentials("caldav_loginid@icloud.com", "caldav_icloud_password");
	  CalDAVHelper test(url, false, &httpSession, httpAuth);
	  ASSERT_FALSE(test.findCalendarHomeSet());
	  test.findPrincipalUrl();
	  ASSERT_TRUE(test.findCalendarHomeSet());

	}

	TEST_F(CalDAVHelperTests, testFindCalendars)
	{
	  const std::string url = "https://caldav.icloud.com";
	  OpenAB::HttpSession httpSession;
	  httpSession.init();
	  OpenAB::HttpAuthorizer* httpAuth = NULL;
	  httpAuth = new OpenAB::BasicHttpAuthorizer();
	  ((OpenAB::BasicHttpAuthorizer*)httpAuth)->setCredentials("caldav_loginid@icloud.com", "caldav_icloud_password");
	  CalDAVHelper test(url, false, &httpSession, httpAuth);
	  test.findPrincipalUrl();
	  test.findCalendarHomeSet();
	  ASSERT_TRUE(test.findCalendars());
	}

	TEST_F(CalDAVHelperTests, tesQueryMetadata)
	{
	  const std::string url = "https://caldav.icloud.com";
	  OpenAB::HttpSession httpSession;
	  httpSession.init();
	  OpenAB::HttpAuthorizer* httpAuth = NULL;
	  httpAuth = new OpenAB::BasicHttpAuthorizer();
	  ((OpenAB::BasicHttpAuthorizer*)httpAuth)->setCredentials("caldav_loginid@icloud.com", "caldav_icloud_password");
	  CalDAVHelper test(url, false, &httpSession, httpAuth);
	  test.findPrincipalUrl();
	  test.findCalendarHomeSet();
	  test.findCalendars();
	  CalDAVHelper::Calendars cals = test.getCalendars();
	  ASSERT_TRUE(test.queryCalendarMetadata(cals[0].getUrl()));
	  ASSERT_TRUE(test.queryEventsMetadata(cals[0].getUrl()));
	  ASSERT_FALSE(test.queryCalendarMetadata(""));
	  ASSERT_FALSE(test.queryEventsMetadata(""));
	  ASSERT_FALSE(test.queryCalendarMetadata("www.google.ie"));
	  ASSERT_FALSE(test.queryEventsMetadata("www.google.ie"));
	}

	TEST_F(CalDAVHelperTests, tesQueryChangedMetadata)
	{
	  const std::string url = "https://caldav.icloud.com";
	  OpenAB::HttpSession httpSession;
	  httpSession.init();
	  OpenAB::HttpAuthorizer* httpAuth = NULL;
	  httpAuth = new OpenAB::BasicHttpAuthorizer();
	  ((OpenAB::BasicHttpAuthorizer*)httpAuth)->setCredentials("caldav_loginid@icloud.com", "caldav_icloud_password");
	  CalDAVHelper test(url, false, &httpSession, httpAuth);
	  test.findPrincipalUrl();
	  test.findCalendarHomeSet();
	  test.findCalendars();
	  CalDAVHelper::Calendars cals = test.getCalendars();
	  std::string token = test.getSyncToken();
	  std::vector<OpenAB::PIMItem::ID> removed;
	  ASSERT_TRUE(test.queryChangedEventsMetadata(cals[0].getUrl(), token, removed));
	  ASSERT_FALSE(test.queryChangedEventsMetadata("", token, removed));
	  ASSERT_FALSE(test.queryChangedEventsMetadata("www.google.ie", token, removed));
	}

	TEST_F(CalDAVHelperTests, tesQueryChangedCalendarInfo)
	{
	  const std::string url = "https://caldav.icloud.com";
	  OpenAB::HttpSession httpSession;
	  httpSession.init();
	  OpenAB::HttpAuthorizer* httpAuth = NULL;
	  httpAuth = new OpenAB::BasicHttpAuthorizer();
	  ((OpenAB::BasicHttpAuthorizer*)httpAuth)->setCredentials("caldav_loginid@icloud.com", "caldav_icloud_password");
	  CalDAVHelper test(url, false, &httpSession, httpAuth);
	  test.findPrincipalUrl();
	  test.findCalendarHomeSet();
	  test.findCalendars();
	  CalDAVHelper::CalendarInfo info;
	  CalDAVHelper::CalendarInfo failInfo;
	  CalDAVHelper::Calendars cals = test.getCalendars();
	  ASSERT_TRUE(test.queryCalendarInfo(cals[0].getUrl(), info));
	  ASSERT_FALSE(test.queryCalendarInfo("", failInfo));
	  ASSERT_FALSE(test.queryCalendarInfo("www.google.ie", failInfo));
	}

	TEST_F(CalDAVHelperTests, testAddEvent)
	{
	  const std::string url = "https://caldav.icloud.com";
	  OpenAB::HttpSession httpSession;
	  httpSession.init();
	  OpenAB::HttpAuthorizer* httpAuth = NULL;
	  httpAuth = new OpenAB::BasicHttpAuthorizer();
	  ((OpenAB::BasicHttpAuthorizer*)httpAuth)->setCredentials("caldav_loginid@icloud.com", "caldav_icloud_password");
	  CalDAVHelper test(url, false, &httpSession, httpAuth);
	  ASSERT_TRUE(test.findPrincipalUrl());
	  ASSERT_TRUE(test.findCalendarHomeSet());
	  ASSERT_TRUE(test.findCalendars());
	  CalDAVHelper::Calendars cals = test.getCalendars();
	  ASSERT_NE(cals.size(), 0);
	  std::string uri;
	  std::string etag;
	  ASSERT_TRUE(test.addEvent(cals[0].getUrl(), testEvent, uri, etag));
	  ASSERT_TRUE(test.removeEvent(uri, etag));
	  ASSERT_FALSE(test.addEvent(cals[0].getUrl(), failEvent, uri, etag));
	}

	TEST_F(CalDAVHelperTests, testRemoveEvent)
	{
	  const std::string url = "https://caldav.icloud.com";
	  OpenAB::HttpSession httpSession;
	  httpSession.init();
	  OpenAB::HttpAuthorizer* httpAuth = NULL;
	  httpAuth = new OpenAB::BasicHttpAuthorizer();
	  ((OpenAB::BasicHttpAuthorizer*)httpAuth)->setCredentials("caldav_loginid@icloud.com", "caldav_icloud_password");
	  CalDAVHelper test(url, false, &httpSession, httpAuth);
	  ASSERT_TRUE(test.findPrincipalUrl());
	  ASSERT_TRUE(test.findCalendarHomeSet());
	  ASSERT_TRUE(test.findCalendars());
	  CalDAVHelper::Calendars cals = test.getCalendars();
	  ASSERT_NE(cals.size(), 0);
	  std::string uri;
	  std::string etag;
	  test.addEvent(cals[0].getUrl(), testEvent, uri, etag);
	  ASSERT_TRUE(test.removeEvent(uri, etag));
	  std::string emptyUri = "";
	  ASSERT_FALSE(test.removeEvent(emptyUri, etag));
	}

	TEST_F(CalDAVHelperTests, testDownloadEvents)
	{
	  const std::string url = "https://caldav.icloud.com";
	  OpenAB::HttpSession httpSession;
	  httpSession.init();
	  OpenAB::HttpAuthorizer* httpAuth = NULL;
	  httpAuth = new OpenAB::BasicHttpAuthorizer();
	  ((OpenAB::BasicHttpAuthorizer*)httpAuth)->setCredentials("caldav_loginid@icloud.com", "caldav_icloud_password");
	  CalDAVHelper test(url, false, &httpSession, httpAuth);
	  test.findPrincipalUrl();
	  test.findCalendarHomeSet();
	  test.findCalendars();
	  CalDAVHelper::Calendars cals = test.getCalendars();
	  std::vector<std::string> uris;
	  std::vector<std::string> iCals;
	  ASSERT_TRUE(test.downloadEvents(cals[0].getUrl(), uris, iCals));
	  ASSERT_FALSE(test.downloadEvents("", uris, iCals));
	  ASSERT_FALSE(test.downloadEvents("www,google", uris, iCals));
	  unsigned int offset = 2;
	  unsigned int size = 4;
	  ASSERT_TRUE(test.downloadEvents(cals[0].getUrl(), offset, size, iCals));
	  ASSERT_FALSE(test.downloadEvents("", offset, size, iCals));
	  ASSERT_FALSE(test.downloadEvents("www.google.ie", offset, size, iCals));
	  offset = 0;
	  size = 0;
	  ASSERT_TRUE(test.downloadEvents(cals[0].getUrl(), offset, size, iCals));
	}

	//modify event does not work for either icloud or gmail on my machine
	TEST_F(CalDAVHelperTests, testModifyEvent)
	{
	   const std::string url = "https://caldav.icloud.com";
	   OpenAB::HttpSession httpSession;
	   httpSession.init();
	   OpenAB::HttpAuthorizer* httpAuth = NULL;
	   httpAuth = new OpenAB::BasicHttpAuthorizer();
	   ((OpenAB::BasicHttpAuthorizer*)httpAuth)->setCredentials("caldav_loginid@icloud.com", "caldav_icloud_password");
	   CalDAVHelper test(url, false, &httpSession, httpAuth);
	   test.findPrincipalUrl();
	   test.findCalendarHomeSet();
	   test.findCalendars();
	   CalDAVHelper::Calendars cals = test.getCalendars();
	   std::string uri;
	   std::string etag;
	   ASSERT_TRUE(test.addEvent(cals[0].getUrl(), testEvent2, uri, etag));
	   ASSERT_TRUE(test.modifyEvent(uri, testEvent2, etag));
	   ASSERT_TRUE(test.removeEvent(uri, etag));
	}

	/*TEST_F(CalDAVHelperTests, testGoogleEvents)
	{
	  const std::string url = "https://apidata.googleusercontent.com/caldav/v2";
	  const std::string clientId = "caldav_clientId.apps.googleusercontent.com";
	  const std::string clientSecret = "caldav_clientsecret";
	  const std::string refreshToken = "caldav_refreshtoken";
	  OpenAB::HttpSession httpSession;
	  httpSession.init();
	  OpenAB::HttpAuthorizer* httpAuth = NULL;
	  httpAuth = new OpenAB::OAuth2HttpAuthorizer();
	  ((OpenAB::OAuth2HttpAuthorizer*)httpAuth)->authorize(clientId, clientSecret, refreshToken);
	  CalDAVHelper test(url, false, &httpSession, httpAuth);
	  ASSERT_TRUE(test.findPrincipalUrl());
	  ASSERT_TRUE(test.findCalendarHomeSet());
	  ASSERT_TRUE(test.findCalendars());
	  CalDAVHelper::Calendars cals = test.getCalendars();
	  std::string uri;
	  std::string etag;
	  ASSERT_TRUE(test.addEvent(cals[0].getUrl(), testEvent2, uri, etag));
	  ASSERT_TRUE(test.modifyEvent(uri, testEvent2, etag));
	  ASSERT_TRUE(test.removeEvent(uri, etag));
	}*/

}
