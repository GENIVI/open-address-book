/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file caldav_helper_tester.cpp
 */
#include <string.h>
#include <OpenAB.hpp>
#include "helpers/SecureString.hpp"
#include <fstream>
#include "CalDAVHelper.hpp"
#include "helpers/BasicHttpAuthorizer.hpp"
#include "helpers/OAuth2HttpAuthorizer.hpp"

std::string testEvent="BEGIN:VCALENDAR\n"
"VERSION:2.0\n"
"BEGIN:VEVENT\n"
"DTSTART:20141128T150000Z\n"
"DTEND:20141128T160000Z\n"
"DESCRIPTION:TestEvent1123\n"
"SUMMARY:TestEvent1123\n"
"UID:1234-d1a2-4bd1-8b86-4ccdca68e55c\n"
"DTSTAMP:201412\r\n"
" 17T103142Z\n"
"CREATED:20141\n"
" 217T112526Z\n"
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
"UID:1234-d1a2-4bd1-8b86-4ccdca68e55c\n"
"DTSTAMP:20141217T103142Z\n"
"CREATED:20141217T112526Z\n"
"LAST-MODIFIED:20141217T112526Z\n"
"END:VEVENT\n"
"END:VCALENDAR\n";

int main(int argc, char* argv[])
{
  OpenAB::Logger::OutLevel() = OpenAB::Logger::Debug;

  LOG_DEBUG()<<testEvent<<std::endl;
  OpenAB::linearize(testEvent);
  LOG_DEBUG()<<testEvent<<std::endl;;
  return 0;

  if (argc < 2)
  {
    printf ("Use %s <authentication method>\n", argv[0]);
    return 1;
  }

  OpenAB::HttpSession httpSession;
  httpSession.init();

  OpenAB::HttpAuthorizer* httpAuthorizer = NULL;

  OpenAB_Source::Parameters params;
  if (0 == strcmp (argv[1], "password"))
  {
    if (argc < 5)
    {
      printf("Use %s password <server url> <user login> <user password>\n",argv[0]);
      return 1;
    }
    httpAuthorizer = new OpenAB::BasicHttpAuthorizer();
    ((OpenAB::BasicHttpAuthorizer*)httpAuthorizer)->setCredentials(argv[3], argv[4]);
  }
  else
  {
    if (argc < 6)
    {
      printf("Use %s oauth2 <server url> <client id> <client secret> <refresh token>\n",argv[0]);
      return 1;
    }
    httpAuthorizer = new OpenAB::OAuth2HttpAuthorizer();
    ((OpenAB::OAuth2HttpAuthorizer*)httpAuthorizer)->authorize(argv[3], argv[4], argv[5]);
  }

  CalDAVHelper helper(argv[2], false, &httpSession, httpAuthorizer);

  LOG_DEBUG()<<"Finding principal url"<<std::endl;
  if (!helper.findPrincipalUrl())
  {
    LOG_ERROR() << "Cannot connect to CalDAV server"<<std::endl;
    return 1;
  }
  LOG_DEBUG()<<"Finding principal url OK"<<std::endl;

  LOG_DEBUG()<<"Finding calendar home set"<<std::endl;
  if (!helper.findCalendarHomeSet())
  {
    LOG_ERROR() << "Cannot connect to CalDAV server"<<std::endl;
    return 1;
  }
  LOG_DEBUG()<<"Finding calendar home set OK"<<std::endl;

  LOG_DEBUG()<<"Finding calendars"<<std::endl;
  if (!helper.findCalendars())
  {
    LOG_ERROR() << "Cannot connect to CalDAV server"<<std::endl;
    return 1;
  }
  LOG_DEBUG()<<"Finding calendars OK"<<std::endl;

  CalDAVHelper::Calendars cals = helper.getCalendars();
  if (cals.size() == 0)
  {
    LOG_ERROR() << "No calendars found"<<std::endl;
    return 1;
  }

  CalDAVHelper::Calendars::iterator it;
  for (it = cals.begin(); it != cals.end(); ++it)
  {
    if ((*it).getUrl().empty())
    {
      LOG_ERROR()<<"Found calendar with empty URL"<<std::endl;
      return 1;
    }

    LOG_DEBUG()<<"Calendar info:"<<std::endl;
    LOG_DEBUG()<<"URL: "<<(*it).getUrl()<<std::endl;
    LOG_DEBUG()<<"Display name: "<<(*it).getDisplayName()<<std::endl;
  }

  LOG_DEBUG()<<"Querying first calendar metadata"<<std::endl;
  if (!helper.queryCalendarMetadata(cals[0].getUrl()))
  {
    LOG_DEBUG()<<"Querying first calendar metadata FALIED"<<std::endl;
    return 1;
  }
  LOG_DEBUG()<<"Querying first calendar metadata OK"<<std::endl;

  LOG_DEBUG()<<"Querying events metadata"<<std::endl;
  if(!helper.queryEventsMetadata(cals[0].getUrl()))
  {
    LOG_DEBUG()<<"Querying events metadata FAILED"<<std::endl;
  }
  LOG_DEBUG()<<"Querying events metadata OK"<<std::endl;

  if (helper.getEventsMetadata().size() != 0)
  {
    LOG_ERROR()<<"Calendar already contains some items, please remove them before running test again"<<std::endl;
    return 1;
  }


  LOG_DEBUG()<<"Creating event"<<std::endl;
  std::string uri;
  std::string etag;
  if (!helper.addEvent(cals[0].getUrl(), testEvent, uri, etag))
  {
    LOG_DEBUG()<<"Creating event FAILED"<<std::endl;
  }
  LOG_DEBUG()<<"Creating event OK"<<std::endl;
  LOG_DEBUG()<<"New event uri:"<<uri<<" new event etag: "<<etag<<std::endl;

  LOG_DEBUG()<<"Querying events metadata"<<std::endl;
  if(!helper.queryEventsMetadata(cals[0].getUrl()))
  {
    LOG_DEBUG()<<"Querying events metadata FAILED"<<std::endl;
    return 1;
  }
  LOG_DEBUG()<<"Querying events metadata OK"<<std::endl;

  if (helper.getEventsMetadata().size() != 1)
  {
    LOG_ERROR()<<"Calendar should contain only 1 event!!"<<std::endl;
    return 1;
  }

  CalDAVHelper::EventsMetadata metadata = helper.getEventsMetadata();
  if (metadata[0].uri != uri)
  {
    LOG_ERROR()<<"Uri of newly created item does not match with uri in metadata!!!"<<std::endl;
  }

  if (metadata[0].etag != etag)
  {
    LOG_ERROR()<<"etag of newly created item does not match with etag in metadata!!!"<<std::endl;
  }

  LOG_DEBUG()<<"Modifying event"<<std::endl;
  if(!helper.modifyEvent(uri, testEvent2, etag))
  {
    LOG_DEBUG()<<"Modifying event FAILED"<<std::endl;
    return 1;
  }
  LOG_DEBUG()<<"Modifying event OK"<<std::endl;

  LOG_DEBUG()<<"Removing event"<<std::endl;
  if(!helper.removeEvent(uri, etag))
  {
    LOG_DEBUG()<<"Removing event FAILED"<<std::endl;
    return 1;
  }
  LOG_DEBUG()<<"Removing event OK"<<std::endl;
  return 0;
}
