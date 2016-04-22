/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file CalDAVHelper.cpp
 */

#include "CalDAVHelper.hpp"
#include <helpers/Log.hpp>
#include <helpers/StringHelper.hpp>
#include "config.h" //for DAV_USER_AGENT definition

CalDAVHelper::CalDAVHelper(const std::string& serverUrl,
                           bool isCalendarUrl,
                           OpenAB::HttpSession* httpSession,
                           OpenAB::HttpAuthorizer* httpAuthorizer) :
  serverUrl(serverUrl),
  httpSession(httpSession),
  httpAuthorizer(httpAuthorizer),
  userAgent(DAV_USER_AGENT)
{
  //if we have calendar url, parse princpialCalendarSetHostUrl from it
  if (isCalendarUrl)
  {
    principalCalendarSetHostUrl = OpenAB::parseURLHostPart(serverUrl);
  }
  else
  {
    serverHostUrl = OpenAB::parseURLHostPart(serverUrl);
  }

  httpSession->enableTrace(true);
}

CalDAVHelper::~CalDAVHelper()
{
}

bool CalDAVHelper::findPrincipalUrl()
{
  OpenAB::HttpMessage msg;
  msg.setRequestType("PROPFIND");
  msg.appendHeader("Content-Type", "text/xml");
  msg.appendHeader("User-Agent", userAgent);
  msg.appendHeader("Depth", "0");
  msg.setData("<D:propfind xmlns:D='DAV:'><D:prop><D:current-user-principal/></D:prop></D:propfind>");
  msg.setURL(serverUrl);
  msg.setFollowRedirection(true);

  httpAuthorizer->authorizeMessage(&msg);

  if (httpSession->execute(&msg))
  {
    std::string resp = msg.getResponse();
    if (OpenAB::HttpMessage::MULTISTATUS == msg.getResponseCode())
    {
      std::vector<DAVHelper::DAVResponse> responses;
      if (!davHelper.parseDAVMultistatus(resp, responses))
      {
        LOG_ERROR()<<"Cannot parse server response"<<std::endl;
        return false;
      }

      std::vector<DAVHelper::DAVResponse>::iterator it = responses.begin();
      bool principalUrlFound = false;

      for (; it != responses.end(); ++it)
      {
        if ((*it).hasProperty(davHelper.PROPERTY_CURRENT_USER_PRINCIPAL_HREF))
        {
          principalUrlFound = true;
          principalUrl = (*it).getProperty(davHelper.PROPERTY_CURRENT_USER_PRINCIPAL_HREF);
          break;
        }
      }

      if (!principalUrlFound)
      {
        LOG_ERROR()<<"CardDAV request error: " << "misformatted response" <<std::endl;
        return false;
      }
      if (OpenAB::beginsWith(principalUrl, "/"))
      {
        principalUrl = serverHostUrl + principalUrl;
      }

      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
    LOG_ERROR()<<"CardDAV request error: " << msg.getErrorString() <<std::endl;
    return false;
  }
}

bool CalDAVHelper::findCalendarHomeSet()
{
  OpenAB::HttpMessage msg;
  msg.setRequestType("PROPFIND");
  msg.appendHeader("Content-Type", "text/xml");
  msg.appendHeader("Depth", "0");
  msg.appendHeader("User-Agent", userAgent);
  msg.setData("<D:propfind xmlns:D='DAV:' xmlns:C=\"urn:ietf:params:xml:ns:caldav\"><D:prop><C:calendar-home-set/></D:prop></D:propfind>");
  msg.setURL(principalUrl);
  msg.setFollowRedirection(true);

  httpAuthorizer->authorizeMessage(&msg);

  if (httpSession->execute(&msg))
  {
    if (OpenAB::HttpMessage::MULTISTATUS == msg.getResponseCode())
    {
      std::string resp = msg.getResponse();
      std::vector<DAVHelper::DAVResponse> responses;
      if (!davHelper.parseDAVMultistatus(resp, responses))
      {
        LOG_ERROR()<<"Cannot parse server response"<<std::endl;
        return false;
      }

      std::vector<DAVHelper::DAVResponse>::iterator it = responses.begin();
      bool calendarSetFound = false;

      for (; it != responses.end(); ++it)
      {
        if ((*it).hasProperty(davHelper.PROPERTY_CALENDAR_HOME_SET_HREF))
        {
          calendarSetFound = true;
          principalCalendarHomeSetUrl = (*it).getProperty(davHelper.PROPERTY_CALENDAR_HOME_SET_HREF);
          break;
        }
      }

      if (!calendarSetFound)
      {
        LOG_ERROR()<<"CardDAV request error: " << "misformatted response" <<std::endl;
        return false;
      }
      if (OpenAB::beginsWith(principalCalendarHomeSetUrl, "/"))
      {
        principalCalendarHomeSetUrl = serverHostUrl + principalCalendarHomeSetUrl;
      }

      principalCalendarSetHostUrl = OpenAB::parseURLHostPart(principalCalendarHomeSetUrl);
      LOG_DEBUG()<<"Response "<<principalCalendarHomeSetUrl<<std::endl;
      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
    LOG_ERROR()<<"CardDAV request error: " << msg.getErrorString() <<std::endl;
    return false;
  }
}

bool CalDAVHelper::processCalendarsInfo(const std::string& davResponse, Calendars& cals)
{
  std::vector<DAVHelper::DAVResponse> responses;
  if (!davHelper.parseDAVMultistatus(davResponse, responses))
  {
    LOG_ERROR()<<"Cannot parse server response"<<std::endl;
    return false;
  }


  std::vector<DAVHelper::DAVResponse>::iterator it = responses.begin();
  cals.clear();

  for (; it != responses.end(); ++it)
  {
    if ((*it).hasProperty(davHelper.PROPERTY_RESOURCE_TYPE_CALENDAR))
    {
      std::string url;
      std::string displayName;
      std::vector<CalendarItemTypes> types;


      url = principalCalendarSetHostUrl + (*it).href;
      if ((*it).hasProperty(davHelper.PROPERTY_DISPLAY_NAME))
      {
        displayName = (*it).getProperty(davHelper.PROPERTY_DISPLAY_NAME);
      }

      if ((*it).hasProperty(davHelper.PROPERTY_SUPPORTED_CALENDAR_COMPONENT_SET_EVENT))
        types.push_back(EVENT);
      if ((*it).hasProperty(davHelper.PROPERTY_SUPPORTED_CALENDAR_COMPONENT_SET_JOURNAL))
        types.push_back(JOURNAL);
      if ((*it).hasProperty(davHelper.PROPERTY_SUPPORTED_CALENDAR_COMPONENT_SET_TODO))
        types.push_back(TODO);

      cals.push_back(CalendarInfo(url, displayName, types));
    }
  }
  return true;
}

bool CalDAVHelper::findCalendars()
{
  OpenAB::HttpMessage msg;
  msg.setRequestType("PROPFIND");
  msg.appendHeader("Content-Type", "text/xml");
  msg.appendHeader("Depth", "1");
  msg.appendHeader("User-Agent", userAgent);
  msg.setData("<d:propfind xmlns:d='DAV:' xmlns:C=\"urn:ietf:params:xml:ns:caldav\"><d:prop><d:resourcetype /><d:displayname /><C:supported-calendar-component-set/></d:prop></d:propfind>");

  msg.setURL(principalCalendarHomeSetUrl);
  msg.setFollowRedirection(true);

  httpAuthorizer->authorizeMessage(&msg);

  if (httpSession->execute(&msg))
  {
    if (OpenAB::HttpMessage::MULTISTATUS == msg.getResponseCode())
    {
      std::string resp = msg.getResponse();
      bool anyCalendarFound = false;
      if (!processCalendarsInfo(resp, calendars))
      {
        return false;
      }
      if (calendars.size() != 0)
        anyCalendarFound = true;

      LOG_DEBUG()<<"==================="<<std::endl;
      for (unsigned int i = 0; i < calendars.size(); ++i)
      {
        LOG_DEBUG()<<"Calendar: "<<calendars[i].getDisplayName()<<std::endl;
        LOG_DEBUG()<<"\tURL: "<<calendars[i].getUrl()<<std::endl;
        LOG_DEBUG()<<"\tSupported types:";
        for (unsigned int j = 0; j < calendars[i].getSupportedCalendarTypes().size(); ++j)
        {
          switch (calendars[i].getSupportedCalendarTypes()[j])
          {
            case EVENT:
              LOG_DEBUG()<<"events,";
              break;
            case JOURNAL:
              LOG_DEBUG()<<"journals,";
              break;
            case TODO:
              LOG_DEBUG()<<"tasks,";
              break;
          }
        }
        LOG_DEBUG()<<std::endl;
      }
      if (!anyCalendarFound)
      {
        LOG_ERROR()<<"CardDAV request error: " << "misformatted response" <<std::endl;
        return false;
      }

      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
    LOG_ERROR()<<"CardDAV request error: " << msg.getErrorString() <<std::endl;
    return false;
  }
}

bool CalDAVHelper::queryCalendarInfo(const std::string& calendarURL, CalendarInfo& info)
{
  OpenAB::HttpMessage msg;
  msg.setRequestType("PROPFIND");
  msg.appendHeader("Content-Type", "text/xml");
  msg.appendHeader("Depth", "1");
  msg.appendHeader("User-Agent", userAgent);
  msg.setData("<d:propfind xmlns:d='DAV:' xmlns:C=\"urn:ietf:params:xml:ns:caldav\"><d:prop><d:resourcetype /><d:displayname /><C:supported-calendar-component-set/></d:prop></d:propfind>");

  msg.setURL(calendarURL);
  msg.setFollowRedirection(true);

  httpAuthorizer->authorizeMessage(&msg);

  if (httpSession->execute(&msg))
  {
    if (OpenAB::HttpMessage::MULTISTATUS == msg.getResponseCode())
    {
      std::string resp = msg.getResponse();
      Calendars cals;
      if (!processCalendarsInfo(resp, cals))
      {
        return false;
      }

      if (cals.empty())
      {
        LOG_ERROR()<<"CardDAV request error: " << "misformatted response" <<std::endl;
        return false;
      }
      info = cals.at(0);
      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
    LOG_ERROR()<<"CardDAV request error: " << msg.getErrorString() <<std::endl;
    return false;
  }
}

bool CalDAVHelper::queryCalendarMetadata(const std::string& calendarURL)
{
  OpenAB::HttpMessage msg;
  msg.setRequestType("PROPFIND");
  msg.appendHeader("Content-Type", "text/xml");
  msg.appendHeader("Depth", "0");
  msg.appendHeader("User-Agent", userAgent);
  msg.setData("<D:propfind xmlns:D='DAV:'> <D:prop><D:displayname /><D:getctag/><D:sync-token/></D:prop></D:propfind>");
  msg.setURL(calendarURL);

  httpAuthorizer->authorizeMessage(&msg);

  if (httpSession->execute(&msg))
  {
    if (OpenAB::HttpMessage::MULTISTATUS == msg.getResponseCode())
    {
      std::string resp = msg.getResponse();
      std::vector<DAVHelper::DAVResponse> responses;
      if (!davHelper.parseDAVMultistatus(resp, responses))
      {
        LOG_ERROR()<<"Cannot parse server response"<<std::endl;
        return false;
      }
      std::vector<DAVHelper::DAVResponse>::iterator it = responses.begin();

      for (; it != responses.end(); ++it)
      {
        if ((*it).hasProperty(davHelper.PROPERTY_CTAG))
        {
          calendarCTag = (*it).getProperty(davHelper.PROPERTY_CTAG);
        }
        if ((*it).hasProperty(davHelper.PROPERTY_SYNC_TOKEN))
        {
          calendarSyncToken = (*it).getProperty(davHelper.PROPERTY_SYNC_TOKEN);
        }
      }
      LOG_DEBUG()<<"CTAG: "<<calendarCTag<<" SyncToken "<<calendarSyncToken<<std::endl;
      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
   LOG_ERROR()<<"CardDAV request error: " << msg.getErrorString() <<std::endl;
   return false;
  }
}

bool CalDAVHelper::queryEventsMetadata(const std::string& calendarURL)
{
  eventsMetadata.clear();
  OpenAB::HttpMessage msg;
  msg.setRequestType("PROPFIND");
  msg.appendHeader("Content-Type", "text/xml");
  msg.appendHeader("Depth", "1");
  msg.appendHeader("User-Agent", userAgent);
  msg.setData("<d:propfind xmlns:d='DAV:'><d:prop><d:getetag/><d:resourcetype/></d:prop></d:propfind>");
  msg.setURL(calendarURL);

  httpAuthorizer->authorizeMessage(&msg);

  if (httpSession->execute(&msg))
  {
    LOG_DEBUG()<<msg.getResponse()<<std::endl;
    if (OpenAB::HttpMessage::MULTISTATUS == msg.getResponseCode())
    {
      std::string resp = msg.getResponse();
      std::vector<DAVHelper::DAVResponse> responses;
      if (!davHelper.parseDAVMultistatus(resp, responses))
      {
        LOG_ERROR()<<"Cannot parse server response"<<std::endl;
        return false;
      }
      std::vector<DAVHelper::DAVResponse>::iterator it = responses.begin();

      for (; it != responses.end(); ++it)
      {
        if ((*it).hasProperty(davHelper.PROPERTY_RESOURCE_TYPE) &&
            (*it).getProperty(davHelper.PROPERTY_RESOURCE_TYPE).empty())
        {
          EventMetadata metadata;
          metadata.uri = (*it).href;
          metadata.etag = (*it).getProperty(davHelper.PROPERTY_ETAG);
          eventsMetadata.push_back(metadata);
        }
      }
      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
   LOG_ERROR()<<"CardDAV request error: " << msg.getErrorString() <<std::endl;
   return false;
  }
}

bool CalDAVHelper::queryChangedEventsMetadata(const std::string& calendarURL,
                                              const std::string& syncToken,
                                              std::vector<OpenAB::PIMItem::ID>& removed)
{
  eventsMetadata.clear();
  OpenAB::HttpMessage msg;
  msg.setRequestType("REPORT");
  msg.appendHeader("Content-Type", "text/xml");
  msg.appendHeader("User-Agent", userAgent);
  msg.appendHeader("Depth", "1");
  msg.setURL(calendarURL);

  httpAuthorizer->authorizeMessage(&msg);

  std::stringstream oss;
  oss<<"<D:sync-collection xmlns:D='DAV:'><D:sync-token>";
  oss<<syncToken;
  oss<<"</D:sync-token><D:sync-level>1</D:sync-level>";
  oss<<"<D:prop><D:getetag/><D:resourcetype/></D:prop></D:sync-collection>";

  msg.setData(oss.str());

  if (httpSession->execute(&msg))
  {
    LOG_DEBUG()<<msg.getResponse()<<std::endl;
    if (OpenAB::HttpMessage::MULTISTATUS == msg.getResponseCode())
    {
      std::string resp = msg.getResponse();
      LOG_DEBUG()<<resp<<std::endl;
      std::vector<DAVHelper::DAVResponse> responses;
      if (!davHelper.parseDAVMultistatus(resp, responses, calendarSyncToken))
      {
        LOG_ERROR()<<"Cannot parse server response"<<std::endl;
        return false;
      }
      std::vector<DAVHelper::DAVResponse>::iterator it = responses.begin();

      for (; it != responses.end(); ++it)
      {
        //check if href ends with .ics - iCloud is also returning entry with whole calendar
        if (OpenAB::endsWith((*it).href, ".ics"))
        {
          if ((*it).hasProperty(davHelper.PROPERTY_ETAG))
          {
            EventMetadata metadata;
            metadata.uri = (*it).href;
            metadata.etag = (*it).getProperty(davHelper.PROPERTY_ETAG);
            eventsMetadata.push_back(metadata);
          }
          else
          {
            removed.push_back((*it).href);
          }
        }
      }

      LOG_DEBUG() << "Got  " << eventsMetadata.size() << " events SyncToken "<<calendarSyncToken<<std::endl;
      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
   LOG_ERROR()<<"CardDAV request error: " << msg.getErrorString() <<std::endl;
   return false;
  }
}

bool CalDAVHelper::downloadEvents(const std::string& calendarURL,
                                  std::vector<std::string>& uris,
                                  std::vector<std::string>& iCals)
{
  OpenAB::HttpMessage msg;
  msg.setRequestType("REPORT");
  msg.setURL(calendarURL);
  msg.appendHeader("Content-Type", "text/xml");
  msg.appendHeader("User-Agent", userAgent);
  msg.appendHeader("Depth", "1");

  httpAuthorizer->authorizeMessage(&msg);


  std::stringstream oss;
  oss<<"<C:calendar-multiget xmlns:D='DAV:' xmlns:C='urn:ietf:params:xml:ns:caldav'>";
  oss<<"<D:prop><D:getetag/><C:calendar-data/>";
  oss<<"</D:prop>";

  for (unsigned int i = 0; i < uris.size(); ++i)
  {
    oss<<"<D:href>"<<uris[i]<<"</D:href>";
  }

  oss<<"</C:calendar-multiget>";

  msg.setData(oss.str());

  //Google does not returns iCals in the same order like provided uris,
  //resize output buffer and place iCals in right place
  iCals.clear();
  iCals.resize(uris.size());

  if (httpSession->execute(&msg))
  {
    if (msg.MULTISTATUS == msg.getResponseCode())
    {
      std::string resp = msg.getResponse();
      std::vector<DAVHelper::DAVResponse> responses;
      if (!davHelper.parseDAVMultistatus(resp, responses))
      {
        LOG_ERROR()<<"Cannot parse server response"<<std::endl;
        return false;
      }
      std::vector<DAVHelper::DAVResponse>::iterator it = responses.begin();
      for (; it != responses.end(); ++it)
      {
        if ((*it).hasProperty(davHelper.PROPERTY_CALENDAR_DATA))
        {
          std::string iCal = (*it).getProperty(davHelper.PROPERTY_CALENDAR_DATA);
          if (!iCal.empty())
          {
            int idx = getIndexFromUris(uris, (*it).href);
            iCals[idx] = iCal;
          }
        }
      }
      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
    LOG_ERROR()<<"CardDAV request error: " << msg.getErrorString() <<std::endl;
    return false;
  }
}

int CalDAVHelper::getIndexFromUris(const std::vector<std::string>& uris, const std::string& uri)
{
  for (unsigned int i = 0; i < uris.size(); ++i)
  {
    if (uris.at(i) == uri)
    {
      return i;
    }
  }

  return -1;
}

bool CalDAVHelper::downloadEvents(const std::string& calendarURL,
                                  unsigned int offset, unsigned int size,
                                  std::vector<std::string>& iCals)
{
  OpenAB::HttpMessage msg;
  msg.setRequestType("REPORT");
  msg.setURL(calendarURL);
  msg.appendHeader("Content-Type", "text/xml");
  msg.appendHeader("User-Agent", userAgent);
  msg.appendHeader("Depth", "1");

  httpAuthorizer->authorizeMessage(&msg);


  std::stringstream oss;
  oss<<"<C:calendar-multiget xmlns:D='DAV:' xmlns:C='urn:ietf:params:xml:ns:caldav'>";
  oss<<"<D:prop><D:getetag/><C:calendar-data/></D:prop>";

  std::vector<std::string> uris;
  unsigned int count = (eventsMetadata.size() > (offset + size)) ? (offset + size) : eventsMetadata.size();
  for (unsigned int i = offset; i < count; ++i)
  {
    uris.push_back(eventsMetadata[i].uri);
    oss<<"<D:href>"<<eventsMetadata[i].uri<<"</D:href>";
  }

  oss<<"</C:calendar-multiget>";

  msg.setData(oss.str());

  iCals.clear();
  iCals.resize(uris.size());
  if (httpSession->execute(&msg))
  {
    LOG_DEBUG()<<msg.getResponse()<<std::endl;
    if (msg.MULTISTATUS == msg.getResponseCode())
    {
      std::string resp = msg.getResponse();
      std::vector<DAVHelper::DAVResponse> responses;
      if (!davHelper.parseDAVMultistatus(resp, responses))
      {
        LOG_ERROR()<<"Cannot parse server response"<<std::endl;
        return false;
      }
      std::vector<DAVHelper::DAVResponse>::iterator it = responses.begin();
      for (; it != responses.end(); ++it)
      {
        if ((*it).hasProperty(davHelper.PROPERTY_CALENDAR_DATA))
        {
          std::string iCal = (*it).getProperty(davHelper.PROPERTY_CALENDAR_DATA);
          if (!iCal.empty())
          {
            int idx = getIndexFromUris(uris, (*it).href);
            iCals[idx] = iCal;
          }
        }
      }
      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
    LOG_ERROR()<<"CardDAV request error: " << msg.getErrorString() <<std::endl;
    return false;
  }
}

bool CalDAVHelper::addEvent(const std::string& calendarURL,
                            const std::string& ical,
                            std::string& uri,
                            std::string& etag)
{
  LOG_DEBUG()<<ical<<std::endl;
  //check if provided ical contains valid UID
  std::string::size_type pos = 0;
  std::string uid = OpenAB::cut(ical, "UID:", "\n", pos);
  if (uid.empty())
  {
    LOG_DEBUG()<<"Provided ical does not contain valid UID"<<std::endl;
    return false;
  }

  OpenAB::trimSpaces(uid);
  OpenAB::HttpMessage msg;
  msg.setRequestType(msg.PUT);
  msg.setData(ical);
  msg.setURL(calendarURL + uid + ".ics");
  msg.appendHeader("User-Agent", userAgent);
  msg.appendHeader("Content-Type", "text/calendar; charset=utf-8");

  httpAuthorizer->authorizeMessage(&msg);

  if (httpSession->execute(&msg))
  {
    LOG_DEBUG()<<msg.getResponse()<<std::endl;
    if (msg.CREATED == msg.getResponseCode())
    {
      OpenAB::HttpMessage::Headers headers = msg.getResponseHeaders();
      for (unsigned int i = 0; i < headers.size(); ++i)
      {
        if (headers[i].first == "Location")
        {
          uri = headers[i].second;
          OpenAB::trimSpaces(uri);
        }
        else if (headers[i].first == "ETag")
        {
          etag = headers[i].second;
          OpenAB::trimSpaces(etag);
        }
      }
      //Google server responds with CREATED code, but does not include etag and location
      //in headers. Needs to manually ask for etag of newly created item, save eventsMetadata,
      //so from user perspective there will be no internal state change.
      if (uri.empty() || etag.empty())
      {
        EventsMetadata oldMetadata = eventsMetadata;
        queryEventsMetadata(calendarURL + uid + ".ics");
        if (!eventsMetadata.empty())
        {
          uri = (*eventsMetadata.begin()).uri;
          etag = (*eventsMetadata.begin()).etag;
          OpenAB::trimSpaces(uri);
          OpenAB::trimSpaces(etag);
        }
        eventsMetadata = oldMetadata;
      }
      return true;
    }

    if (msg.MULTISTATUS == msg.getResponseCode())
    {
      LOG_DEBUG()<<"MULTISTATUS CODE"<<std::endl;
      std::string resp = msg.getResponse();
      std::vector<DAVHelper::DAVResponse> responses;
      if (!davHelper.parseDAVMultistatus(resp, responses))
      {
        LOG_ERROR()<<"Cannot parse server response"<<std::endl;
        return false;
      }
      std::vector<DAVHelper::DAVResponse>::iterator it = responses.begin();

      for (; it != responses.end(); ++it)
      {
        LOG_DEBUG()<<"Response status "<<(*it).status<<std::endl;
        if ((*it).hasProperty(davHelper.PROPERTY_ETAG))
        {
          uri = (*it).href;
          etag = (*it).getProperty(davHelper.PROPERTY_ETAG);
          OpenAB::trimSpaces(uri);
          OpenAB::trimSpaces(etag);
          LOG_ERROR()<<"Event created with uid: " << uri<<" etag: "<<etag<<std::endl;
          return true;
        }
        else if ((*it).hasError(davHelper.ERROR_UID_CONFLICT))
        {
          LOG_ERROR()<<"Event with the same UID already exists on server"<<std::endl;
          return false;
        }
        else
        {
          LOG_ERROR()<<"CalDAV request error: " <<std::endl;
          return false;
        }
      }
      return true;
    }
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {

    LOG_ERROR()<<"CalDAV request error: " << msg.getResponseCode()<<" "<<msg.getErrorString() <<std::endl;
    return false;
  }
  return true;
}

bool CalDAVHelper::removeEvent(const std::string& uri,
                               const std::string& etag)
{
  OpenAB::HttpMessage msg;
  msg.setRequestType("DELETE");
  msg.appendHeader("User-Agent", userAgent);
  msg.setURL(principalCalendarSetHostUrl + uri);
  if (!etag.empty())
  {
    msg.appendHeader("If-Match", etag);
  }

  httpAuthorizer->authorizeMessage(&msg);

  if (httpSession->execute(&msg))
  {
    if (msg.NO_CONTENT == msg.getResponseCode())
    {
      OpenAB::HttpMessage::Headers headers = msg.getResponseHeaders();
      for (unsigned int i = 0; i < headers.size(); ++i)
      {
        LOG_DEBUG()<<headers[i].first<<" "<<headers[i].second<<std::endl;
      }
      LOG_ERROR()<<"Contact removed: " << msg.getResponseCode()<<" " <<msg.getResponse() <<std::endl;
      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
    LOG_ERROR()<<"CardDAV request error: " << msg.getResponseCode()<<" "<<msg.getErrorString() <<std::endl;
    return false;
  }
}

bool CalDAVHelper::modifyEvent(const std::string& uri,
                               const std::string& ical,
                               std::string& etag)
{
  LOG_DEBUG()<<ical<<std::endl;
  OpenAB::HttpMessage msg;
  msg.setRequestType(OpenAB::HttpMessage::PUT);
  msg.setData(ical);
  msg.setURL(principalCalendarSetHostUrl + uri);
  msg.appendHeader("User-Agent", userAgent);
  msg.appendHeader("Content-Type", "text/calendar; charset=utf-8");

  if (!etag.empty())
  {
    msg.appendHeader("If-Match", etag);
  }

  //clear old etag, so will will know if server returned new one directly in response or we have to query it
  etag.clear();

  LOG_DEBUG()<<"Updating "<<principalCalendarSetHostUrl + uri<<std::endl;

  httpAuthorizer->authorizeMessage(&msg);

  if (httpSession->execute(&msg))
  {
    OpenAB::HttpMessage::Headers headers = msg.getResponseHeaders();
    for (unsigned int i = 0; i < headers.size(); ++i)
    {
      LOG_DEBUG()<<headers[i].first<<" "<<headers[i].second<<std::endl;
    }
    LOG_ERROR()<<"Event updated: " << msg.getResponseCode()<<" " <<msg.getResponse() <<std::endl;

    if (msg.NO_CONTENT == msg.getResponseCode())
    {
      OpenAB::HttpMessage::Headers headers = msg.getResponseHeaders();
      for (unsigned int i = 0; i < headers.size(); ++i)
      {
        if (headers[i].first == "ETag")
        {
          etag = headers[i].second;
          OpenAB::trimSpaces(etag);
        }
        LOG_DEBUG()<<headers[i].first<<" "<<headers[i].second<<std::endl;
      }
      //Google server responds with CREATED code, but does not include etag and location
      //in headers. Needs to manually ask for etag of newly created item, save eventsMetadata,
      //so from user perspective there will be no internal state change.
      if (etag.empty())
      {
        EventsMetadata oldMetadata = eventsMetadata;
        queryEventsMetadata(principalCalendarSetHostUrl + uri);
        if (!eventsMetadata.empty())
        {
          etag = (*eventsMetadata.begin()).etag;
          OpenAB::trimSpaces(etag);
        }
        eventsMetadata = oldMetadata;
      }
      LOG_ERROR()<<"Event updated with uid: " << uri<<" etag: "<<etag<<std::endl;
      return true;
    }
    else if (msg.PRECONDITION_FAILED == msg.getResponseCode())
    {
      LOG_ERROR()<<"Cannot update event - provided ETag does not match one on server - probably event was modified before"<<std::endl;
      return false;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
    LOG_ERROR()<<"CalDAV request error: " << msg.getResponseCode()<<" "<<msg.getErrorString() <<std::endl;
    return false;
  }
}

CalDAVHelper::CalendarInfo::CalendarInfo()
{

}

CalDAVHelper::CalendarInfo::~CalendarInfo()
{

}

CalDAVHelper::CalendarInfo::CalendarInfo(const std::string& url,
                                         const std::string& name,
                                         const std::vector<CalendarItemTypes>& types) :
     url (url),
     displayName (name),
     supportedTypes (types)
{}

std::string CalDAVHelper::CalendarInfo::getUrl() const
{
  return url;
}

std::string CalDAVHelper::CalendarInfo::getDisplayName() const
{
  return displayName;
}

std::vector<CalDAVHelper::CalendarItemTypes> CalDAVHelper::CalendarInfo::getSupportedCalendarTypes() const
{
  return supportedTypes;
}

bool CalDAVHelper::CalendarInfo::supportsType(CalendarItemTypes type) const
{
  std::vector<CalendarItemTypes>::const_iterator it;
  for (it = supportedTypes.begin(); it != supportedTypes.end(); ++it)
  {
    if ((*it) == type)
    {
      return true;
    }
  }

  return false;
}
