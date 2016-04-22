/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file CalDAVStorage.cpp
 */

#include "CalDAVStorage.hpp"
#include <PIMItem/Calendar/PIMCalendarItem.hpp>
#include <algorithm>
#include <math.h>
#include <locale>
#include <unistd.h>
#include "helpers/OAuth2HttpAuthorizer.hpp"
#include "helpers/BasicHttpAuthorizer.hpp"
#include <OpenAB.hpp>


#define QUERY_SIZE 1000

#define EXP_BACKOFF(numRetries) \
  for (unsigned int _i = 0; _i < (numRetries); ++_i, usleep(pow(2.0, _i) * 10))

CalDAVStorage::CalDAVStorage(const std::string& url,
                             const std::string& login,
                             const OpenAB::SecureString& password,
                             const std::string& calendarURL,
                             const std::string& calName,
                             OpenAB::PIMItemType type)
    : OpenAB_Storage::CalendarStorage(type),
      serverUrl (url),
      calendarUrl(calendarURL),
      calendarName(calName),
      userLogin(login),
      userPassword(password),
      clientId(),
      clientSecret(),
      refreshToken(),
      syncToken(),
      authorizer(NULL),
      calDavHelper(NULL),
      sourceIterator(NULL)
{
  LOG_FUNC();
  construct();
}

CalDAVStorage::CalDAVStorage(const std::string& url,
                             const std::string& clientId,
                             const OpenAB::SecureString& clientSecret,
                             const OpenAB::SecureString& refreshToken,
                             const std::string& calendarURL,
                             const std::string& calName,
                             OpenAB::PIMItemType type)
    : OpenAB_Storage::CalendarStorage(type),
      serverUrl (url),
      calendarUrl(calendarURL),
      calendarName(calName),
      userLogin(),
      userPassword(),
      clientId(clientId),
      clientSecret(clientSecret),
      refreshToken(refreshToken),
      syncToken(),
      authorizer(NULL),
      calDavHelper(NULL),
      sourceIterator(NULL)
{
  LOG_FUNC();
  construct();
}

void CalDAVStorage::construct()
{
  curlSession.init();
}

CalDAVStorage::~CalDAVStorage()
{
  cleanup();

  clientSecret.clear();
  refreshToken.clear();
  userPassword.clear();

  curlSession.cleanup();
}

void CalDAVStorage::cleanup()
{
  if (calDavHelper)
  {
    delete calDavHelper;
    calDavHelper = NULL;
  }

  if (authorizer)
  {
    delete authorizer;
    authorizer = NULL;
  }
}
bool CalDAVStorage::selectFirstCalendar(const CalDAVHelper::Calendars cals,
                                         CalDAVHelper::CalendarInfo& selectedCalendar)
{
  CalDAVHelper::Calendars::const_iterator it;
  for (it = cals.begin(); it != cals.end(); ++it)
  {
    CalDAVHelper::CalendarItemTypes t =
        (OpenAB::eEvent == getItemType()) ? CalDAVHelper::EVENT : CalDAVHelper::TODO;
    if ((*it).supportsType(t))
    {
      selectedCalendar = (*it);
      return true;
    }
  }
  return false;
}

enum OpenAB_Storage::Storage::eInit CalDAVStorage::init()
{
  LOG_DEBUG()<<"Initializing CalDAV"<<std::endl;
  cleanup();

  //Check if user login/password was provided, in that case use Basic Http as authorization method
  if (!userLogin.empty())
  {
    OpenAB::BasicHttpAuthorizer* basicAuthorizer = new OpenAB::BasicHttpAuthorizer();
    authorizer = basicAuthorizer;

    basicAuthorizer->setCredentials(userLogin, userPassword);
  }
  //otherwise use OAuth2
  else
  {
    OpenAB::OAuth2HttpAuthorizer* oauth2Authorizer = new OpenAB::OAuth2HttpAuthorizer();
    authorizer = oauth2Authorizer;

    bool authorizationSuccesful = false;

    authorizationSuccesful = oauth2Authorizer->authorize(clientId, clientSecret, refreshToken);
    if (!authorizationSuccesful)
    {
      LOG_ERROR() << "Cannot authenticate user"<<std::endl;
      return eInitFail;
    }
  }

  //If calendar Url was provided use it and do not query server for all calendars.
  //Query only this calendar information to check if it supports requested type of items.
  if (!calendarUrl.empty())
  {
    calDavHelper = new CalDAVHelper(calendarUrl, true, &curlSession, authorizer);

    CalDAVHelper::CalendarInfo cal;
    if (!calDavHelper->queryCalendarInfo(calendarUrl, cal))
    {
      LOG_ERROR() << "Cannot query calendar details"<<std::endl;
      return eInitFail;
    }

    CalDAVHelper::CalendarItemTypes itemType;
    if (getItemType() == OpenAB::eEvent)
    {
      itemType = CalDAVHelper::EVENT;
    }
    else
    {
      itemType = CalDAVHelper::TODO;
    }

    if (!cal.supportsType(itemType))
    {
      LOG_ERROR() << "Calendar "<<cal.getUrl()<<" ("<<cal.getDisplayName()<<") does not support provided item type"<<std::endl;
      return eInitFail;
    }
  }
  //otherwise query server for all required details
  else
  {
    calDavHelper = new CalDAVHelper(serverUrl, false, &curlSession, authorizer);

    if (!calDavHelper->findPrincipalUrl())
    {
      LOG_ERROR() << "Cannot connect to CardDAV server"<<std::endl;
      return eInitFail;
    }

    if (!calDavHelper->findCalendarHomeSet())
    {
      LOG_ERROR() << "Cannot connect to CardDAV server"<<std::endl;
      return eInitFail;
    }

    if (!calDavHelper->findCalendars())
    {
      LOG_ERROR() << "Cannot connect to CardDAV server"<<std::endl;
      return eInitFail;
    }

    // if user didn't provided calendar name to use,
    // use first one that supports given type of items
    CalDAVHelper::Calendars calendars = calDavHelper->getCalendars();
    if (calendarName.empty())
    {
      if (selectFirstCalendar(calendars, calendarInfo))
      {
        calendarUrl = calendarInfo.getUrl();
      }
    }
    else
    {
      // otherwise look for calendar with given name
      CalDAVHelper::CalendarItemTypes t =
          (OpenAB::eEvent == getItemType()) ? CalDAVHelper::EVENT : CalDAVHelper::TODO;
      CalDAVHelper::Calendars::const_iterator it;
      for (it = calendars.begin(); it != calendars.end(); ++it)
      {
        if ((*it).getDisplayName() == calendarName &&
            (*it).supportsType(t))
        {
          calendarInfo = (*it);
          calendarUrl = calendarInfo.getUrl();
          break;
        }
      }
      if (calendarUrl.empty())
      {
        // if calendar with given name does not exist, use first one
        if (selectFirstCalendar(calendars, calendarInfo))
        {
          calendarUrl = calendarInfo.getUrl();
        }
      }
    }

    //check if we found any calendar that is matching requested params
    if (calendarUrl.empty())
    {
      LOG_ERROR()<<"Couldn't found any calendar that will match provided parameters"<<std::endl;
      return eInitFail;
    }
  }

  return eInitOk;
}

enum OpenAB_Storage::Storage::eAddItem CalDAVStorage::addObject(const std::string& iCal,
                                                             OpenAB::PIMItem::ID& newId,
                                                             OpenAB::PIMItem::Revision& revision)
{
  if (!calDavHelper->addEvent(calendarUrl, iCal, newId, revision))
  {
    return eAddItemFail;
  }
  return eAddItemOk;
}

enum OpenAB_Storage::Storage::eAddItem CalDAVStorage::addObjects(const std::vector<std::string> &iCals,
                                                              OpenAB::PIMItem::IDs& newIds,
                                                              OpenAB::PIMItem::Revisions& revisions)
{
  newIds.clear();
  for (unsigned int i = 0; i < iCals.size(); ++i)
  {
    OpenAB::PIMItem::ID newId;
    OpenAB::PIMItem::Revision etag;
    if (eAddItemFail == addObject(iCals[i], newId, etag))
    {
      newIds.clear();
      revisions.clear();
      return eAddItemFail;
    }
    newIds.push_back(newId);
    revisions.push_back(etag);
  }
  return eAddItemOk;
}

enum OpenAB_Storage::Storage::eModifyItem CalDAVStorage::modifyObject(const std::string& iCal,
                                                                   const OpenAB::PIMItem::ID& id,
                                                                   OpenAB::PIMItem::Revision& revision)
{
  if (!calDavHelper->modifyEvent(id, iCal, revision))
  {
    return eModifyItemFail;
  }
  return eModifyItemOk;
}

enum OpenAB_Storage::Storage::eModifyItem CalDAVStorage::modifyObjects( const std::vector<std::string> &iCals,
                                                                     const OpenAB::PIMItem::IDs& ids,
                                                                     OpenAB::PIMItem::Revisions& revisions)
{
  for (unsigned int i = 0; i < iCals.size(); ++i)
  {
    std::string newId;
    std::string etag;
    if (eModifyItemFail == modifyObject(iCals[i], ids[i], etag))
    {
      revisions.clear();
      return eModifyItemFail;
    }
    revisions.push_back(etag);
  }
  return eModifyItemOk;
}

enum OpenAB_Storage::Storage::eRemoveItem CalDAVStorage::removeObject( const OpenAB::PIMItem::ID& id)
{
  if (!calDavHelper->removeEvent(id))
  {
    return eRemoveItemFail;
  }
  return eRemoveItemOk;
}

enum OpenAB_Storage::Storage::eRemoveItem CalDAVStorage::removeObjects( const OpenAB::PIMItem::IDs& ids)
{
  for (unsigned int i = 0; i < ids.size(); ++i)
  {
    std::string newId;
    if (eRemoveItemFail == removeObject(ids[i]))
    {
      return eRemoveItemFail;
    }
  }
  return eRemoveItemOk;
}

enum OpenAB_Storage::Storage::eGetItem CalDAVStorage::getEvent (const OpenAB::PIMItem::ID & id,
                                                             OpenAB::SmartPtr<OpenAB::PIMCalendarEventItem> & item)
{
  if (OpenAB::eEvent != getItemType())
  {
    return eGetItemFail;
  }

  std::vector<std::string> uris;
  uris.push_back(id);
  std::vector<std::string> iCals;

  if (!calDavHelper->downloadEvents(calendarUrl, uris, iCals))
  {
    return eGetItemFail;
  }
  if (iCals.size() != 1)
  {
    return eGetItemFail;
  }
  OpenAB::PIMCalendarEventItem* newEvent = new OpenAB::PIMCalendarEventItem();
  if (!newEvent->parse(iCals.at(0)))
  {
    delete newEvent;
    return eGetItemFail;
  }
  newEvent->setId(id);

  //update item with revisions information
  CalDAVHelper::EventsMetadata md = calDavHelper->getEventsMetadata();
  CalDAVHelper::EventsMetadata::iterator it;
  for (it = md.begin(); it != md.end(); ++it)
  {
    if ((*it).uri == id)
    {
      newEvent->setRevision((*it).etag);
      break;
    }
  }
  item = newEvent;
  return eGetItemOk;
}

enum OpenAB_Storage::Storage::eGetItem CalDAVStorage::getEvents (const OpenAB::PIMItem::IDs & ids,
                                                              std::vector<OpenAB::SmartPtr<OpenAB::PIMCalendarEventItem> > & items)
{
  std::vector<std::string> uris;
  for (unsigned int i = 0; i < ids.size(); ++i)
  {
    uris.push_back(ids[i]);
  }
  std::vector<std::string> iCals;

  if (!calDavHelper->downloadEvents(calendarUrl, uris, iCals))
  {
    LOG_ERROR()<<"Download icals failed"<<std::endl;
    return eGetItemFail;
  }

  for (unsigned int i = 0; i < iCals.size(); ++i)
  {
    OpenAB::PIMCalendarEventItem* newEvent = new OpenAB::PIMCalendarEventItem();
    if (!newEvent->parse(iCals.at(i)))
    {
      delete newEvent;
      LOG_ERROR()<<"Cannot parse vcards"<<std::endl;
      return eGetItemFail;
    }

    newEvent->setId(ids[i]);
    CalDAVHelper::EventsMetadata md = calDavHelper->getEventsMetadata();
    CalDAVHelper::EventsMetadata::iterator it;
    for (it = md.begin(); it != md.end(); ++it)
    {
      if ((*it).uri == ids[i])
      {
        newEvent->setRevision((*it).etag);
        break;
      }
    }
    items.push_back(newEvent);
  }
  return eGetItemOk;
}

enum OpenAB_Storage::Storage::eGetItem CalDAVStorage::getTask(const OpenAB::PIMItem::ID & id,
                                                           OpenAB::SmartPtr<OpenAB::PIMCalendarTaskItem> & item)
{
  if (OpenAB::eTask != getItemType())
  {
    return eGetItemFail;
  }

  std::vector<std::string> uris;
  uris.push_back(id);
  std::vector<std::string> iCals;

  if (!calDavHelper->downloadEvents(calendarUrl, uris, iCals))
  {
    return eGetItemFail;
  }
  if (iCals.size() != 1)
  {
    return eGetItemFail;
  }
  OpenAB::PIMCalendarTaskItem* newEvent = new OpenAB::PIMCalendarTaskItem();
  if (!newEvent->parse(iCals.at(0)))
  {
    delete newEvent;
    return eGetItemFail;
  }
  newEvent->setId(id);

  //update item with revisions information
  CalDAVHelper::EventsMetadata md = calDavHelper->getEventsMetadata();
  CalDAVHelper::EventsMetadata::iterator it;
  for (it = md.begin(); it != md.end(); ++it)
  {
    if ((*it).uri == id)
    {
      newEvent->setRevision((*it).etag);
      break;
    }
  }
  item = newEvent;
  return eGetItemOk;
}

enum OpenAB_Storage::Storage::eGetItem CalDAVStorage::getTasks (const OpenAB::PIMItem::IDs & ids,
                                                             std::vector<OpenAB::SmartPtr<OpenAB::PIMCalendarTaskItem> > & items)
{
  LOG_FUNC()<<std::endl;
  std::vector<std::string> uris;
  for (unsigned int i = 0; i < ids.size(); ++i)
  {
    uris.push_back(ids[i]);
  }
  std::vector<std::string> iCals;

  if (!calDavHelper->downloadEvents(calendarUrl, uris, iCals))
  {
    LOG_ERROR()<<"Download icals failed"<<std::endl;
    return eGetItemFail;
  }

  for (unsigned int i = 0; i < iCals.size(); ++i)
  {
    OpenAB::PIMCalendarTaskItem* newEvent = new OpenAB::PIMCalendarTaskItem();
    if (!newEvent->parse(iCals.at(i)))
    {
      delete newEvent;
      LOG_ERROR()<<"Cannot parse vcards"<<std::endl;
      return eGetItemFail;
    }

    newEvent->setId(ids[i]);
    CalDAVHelper::EventsMetadata md = calDavHelper->getEventsMetadata();
    CalDAVHelper::EventsMetadata::iterator it;
    for (it = md.begin(); it != md.end(); ++it)
    {
      if ((*it).uri == ids[i])
      {
        newEvent->setRevision((*it).etag);
        break;
      }
    }
    items.push_back(newEvent);
  }
  return eGetItemOk;
}

enum OpenAB_Storage::Storage::eGetRevisions CalDAVStorage::getRevisions(std::map<std::string, std::string>& revisions)
{
  if (!calDavHelper->queryEventsMetadata(calendarUrl))
  {
    LOG_FUNC()<<" Cannot query metadata"<<std::endl;
    return eGetRevisionsFail;
  }
  CalDAVHelper::EventsMetadata eventsMetadata = calDavHelper->getEventsMetadata();
  CalDAVHelper::EventsMetadata::iterator it;
  for (it = eventsMetadata.begin(); it != eventsMetadata.end(); ++it)
  {
    revisions[(*it).uri] = (*it).etag;
  }

  return eGetRevisionsOk;
}

enum OpenAB_Storage::Storage::eGetRevisions CalDAVStorage::getChangedRevisions(const std::string& token,
                                                                            std::map<std::string, std::string>& revisions,
                                                                            std::vector<OpenAB::PIMItem::ID>& removed)
{
  if (token.empty())
  {
    return eGetRevisionsFail;
  }

  if (!calDavHelper->queryChangedEventsMetadata(calendarUrl, token, removed))
  {
    LOG_FUNC()<<" Cannot query metadata"<<std::endl;
    return eGetRevisionsFail;
  }
  syncToken = token;
  CalDAVHelper::EventsMetadata eventsMetadata = calDavHelper->getEventsMetadata();
  CalDAVHelper::EventsMetadata::iterator it;
  for (it = eventsMetadata.begin(); it != eventsMetadata.end(); ++it)
  {
    revisions[(*it).uri] = (*it).etag;
  }

  return eGetRevisionsOk;
}

enum OpenAB_Storage::Storage::eGetSyncToken CalDAVStorage::getLatestSyncToken(std::string& token)
{
  if (!calDavHelper->queryCalendarMetadata(calendarUrl))
  {
    return eGetSyncTokenFail;
  }

  token = calDavHelper->getSyncToken();

  return eGetSyncTokenOk;
}

OpenAB_Storage::StorageItemIterator* CalDAVStorage::newStorageItemIterator()
{
  CalDAVStorageItemIterator * ie = new CalDAVStorageItemIterator();
  if (NULL == ie)
  {
    LOG_ERROR() << "Error Cannot create the CalDAV IndexElemIterator"<<std::endl;
    return NULL;
  }

  if (ie->eCursorInitOK != ie->cursorInit(calDavHelper, calendarUrl, getItemType()))
  {
    LOG_ERROR() << "Error Cannot Init the CalDAV IndexElemIterator"<<std::endl;
    delete ie;
    return NULL;
  }
  return ie;
}

enum OpenAB_Source::Source::eGetItemRet CalDAVStorage::getItem(OpenAB::SmartPtr<OpenAB::PIMItem> & item)
{
  if (NULL == sourceIterator)
  {
    sourceIterator = static_cast<CalDAVStorageItemIterator*>(newStorageItemIterator());
  }

  OpenAB_Storage::StorageItem* stItem = sourceIterator->next();

  //does stream of items has ended
  if (stItem == NULL)
  {
    delete sourceIterator;
    sourceIterator = NULL;
    return OpenAB_Source::Source::eGetItemRetEnd;
  }

  item = stItem->item;

  return OpenAB_Source::Source::eGetItemRetOk;
}

enum OpenAB_Source::Source::eSuspendRet CalDAVStorage::suspend()
{
  return OpenAB_Source::Source::eSuspendRetFail;
}

enum OpenAB_Source::Source::eResumeRet CalDAVStorage::resume()
{
  return OpenAB_Source::Source::eResumeRetFail;
}

enum OpenAB_Source::Source::eCancelRet CalDAVStorage::cancel()
{
  return OpenAB_Source::Source::eCancelRetFail;
}

int CalDAVStorage::getTotalCount() const
{
  if (sourceIterator)
  {
    return sourceIterator->getSize();
  }
  return 0;
}

CalDAVStorageItemIterator::CalDAVStorageItemIterator() :
    calDavHelper(NULL),
    total(0),
    offset(0),
    offsetOfCachedICals(0),
    paused(false),
    cancelled(false),
    threadCreated(false),
    transferStatus(OpenAB_Source::Source::eGetItemRetOk),
    itemsType(OpenAB::eEvent)
{
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&bufferReadyCond, NULL);
}

CalDAVStorageItemIterator::~CalDAVStorageItemIterator()
{
  cancelled = true;
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&bufferReadyCond);
  if (threadCreated)
  {
    pthread_join(downloadThread, NULL);
  }
}

enum CalDAVStorageItemIterator::eCursorInit CalDAVStorageItemIterator::cursorInit(CalDAVHelper* helper,
                                                                                  const std::string& calURL,
                                                                                  OpenAB::PIMItemType type)
{
  calDavHelper = helper;
  calendarURL = calURL;

  itemsType = type;

  if (!calDavHelper->queryEventsMetadata(calendarURL))
  {
    return eCursorInitFail;
  }
  eventsMetadata = calDavHelper->getEventsMetadata();
  total = eventsMetadata.size();

  offsetOfCachedICals = 0;
  offset = 0;
  cachedEvents.clear();

  cancelled = false;
  transferStatus = OpenAB_Source::Source::eGetItemRetOk;
  pthread_create(&downloadThread, NULL, downloadThreadFunc, this);
  threadCreated = true;

  return eCursorInitOK;
}

void* CalDAVStorageItemIterator::downloadThreadFunc(void* ptr)
{
  CalDAVStorageItemIterator& iterator = *static_cast<CalDAVStorageItemIterator*>(ptr);
  unsigned int offset = 0;

  while (offset < iterator.total)
  {
    while (iterator.paused && !iterator.cancelled)
    {
      usleep(1000);
    }

    if (iterator.cancelled)
    {
      pthread_mutex_lock(&iterator.mutex);
      pthread_cond_signal(&iterator.bufferReadyCond);
      pthread_mutex_unlock(&iterator.mutex);
      return NULL;
    }

    if (!iterator.downloadICals(offset, QUERY_SIZE))
    {
      LOG_DEBUG()<<"DownloadThread download error"<<std::endl;
      iterator.transferStatus = OpenAB_Source::Source::eGetItemRetError;
      pthread_mutex_lock(&iterator.mutex);
      pthread_cond_signal(&iterator.bufferReadyCond);
      pthread_mutex_unlock(&iterator.mutex);
      return NULL;
    }
    offset += QUERY_SIZE;
  }

  pthread_mutex_lock(&iterator.mutex);
  iterator.transferStatus = OpenAB_Source::Source::eGetItemRetEnd;
  pthread_cond_signal(&iterator.bufferReadyCond);
  pthread_mutex_unlock(&iterator.mutex);
  return NULL;
}

bool CalDAVStorageItemIterator::downloadICals(unsigned int offset, unsigned int size)
{
  LOG_FUNC();
  std::vector<std::string> ids;
  std::vector<std::string> icals;

  unsigned int len = offset + size;
  if (len > eventsMetadata.size())
  {
    len = eventsMetadata.size();
  }

  for (unsigned int i = offset; i < len; ++i)
  {
    LOG_DEBUG()<<"Adding items to be downloaded "<<eventsMetadata[i].uri<<std::endl;
    ids.push_back(eventsMetadata[i].uri);
  }
  if (!calDavHelper->downloadEvents(calendarURL, ids, icals))
  {
    return false;
  }

  pthread_mutex_lock(&mutex);
  for (unsigned int i = 0; i < icals.size(); ++i)
  {
    OpenAB::PIMCalendarItem* newItem;
    if (OpenAB::eEvent == itemsType)
      newItem = new OpenAB::PIMCalendarEventItem();
    else
      newItem = new OpenAB::PIMCalendarTaskItem();

    LOG_DEBUG()<<"========== DOWNLOADED ICAL "<<icals.at(i)<<std::endl;

    newItem->parse(icals.at(i));
    newItem->setId(eventsMetadata[offset + i].uri);
    newItem->setRevision(eventsMetadata[offset + i].etag);
    cachedEvents.push_back(newItem);
  }
  pthread_cond_signal(&bufferReadyCond);
  pthread_mutex_unlock(&mutex);

  return true;
}

OpenAB_Storage::StorageItem* CalDAVStorageItemIterator::next()
{
  pthread_mutex_lock(&mutex);

  if (cachedEvents.empty())
  {
    if (OpenAB_Source::Source::eGetItemRetEnd == transferStatus)
    {
      LOG_DEBUG()<<"download finished"<<std::endl;
      //Cache empty, download finished
      pthread_mutex_unlock(&mutex);
      return NULL;
    }
    //Cache empty, waiting for new batch
    pthread_cond_wait(&bufferReadyCond, &mutex);
  }

  if (OpenAB_Source::Source::eGetItemRetEnd == transferStatus && cachedEvents.empty())
  {
    //Download end
    pthread_mutex_unlock(&mutex);
    return NULL;
  }

  if (OpenAB_Source::Source::eGetItemRetError == transferStatus)
  {
    //Download error
    pthread_mutex_unlock(&mutex);
    return NULL;
  }
  OpenAB::SmartPtr<OpenAB::PIMCalendarItem> nextItem = cachedEvents.front();
  cachedEvents.pop_front();
  LOG_DEBUG()<<"Getting next element from CalDAVStorageItemIterator "<<nextItem->getRawData()<<std::endl;
  pthread_mutex_unlock(&mutex);

  if (OpenAB::eEvent == itemsType)
  {
    OpenAB::PIMCalendarEventItem* it = (OpenAB::PIMCalendarEventItem*)nextItem.getPointer();
    elem.item = new OpenAB::PIMCalendarEventItem(*it);
  }
  else
  {
    OpenAB::PIMCalendarTaskItem* it = (OpenAB::PIMCalendarTaskItem*)nextItem.getPointer();
    elem.item = new OpenAB::PIMCalendarTaskItem(*it);
  }
  elem.id = nextItem->getId();

  return &elem;
}

OpenAB_Storage::StorageItem CalDAVStorageItemIterator::operator*()
{
  return elem;
}

OpenAB_Storage::StorageItem* CalDAVStorageItemIterator::operator->()
{
  return &elem;
}

unsigned int CalDAVStorageItemIterator::getSize() const
{
  return total;
}

namespace CalDavFactory {
OpenAB_Storage::Storage * createInstance(const OpenAB_Storage::Parameters & params)
{
  LOG_FUNC();
  std::string login;
  OpenAB::SecureString password;

  std::string clientId;
  OpenAB::SecureString clientSecret;
  OpenAB::SecureString refreshToken;

  std::string serverUrl = "";
  std::string calendarUrl = "";
  std::string calendarName = "";

  OpenAB::PIMItemType type;

  CalDAVStorage* src = NULL;
  OpenAB::Variant param;

  //Server URL is not mandatory if calendar_url will be provided
  param = params.getValue("server_url");
  if (!param.invalid() && OpenAB::Variant::STRING == param.getType())
  {
    serverUrl = param.getString();
  }

  param = params.getValue("calendar_url");
  if (!param.invalid() && OpenAB::Variant::STRING == param.getType())
  {
    calendarUrl = param.getString();
  }


  bool useOAuth2 = true;
  if (params.getValue("refresh_token").invalid() || params.getValue("client_id").invalid()
      || params.getValue("client_secret").invalid())
  {
    useOAuth2 = false;
  }

  if (useOAuth2)
  {
    if (OpenAB::Variant::STRING != params.getValue("client_id").getType()
        || OpenAB::Variant::STRING != params.getValue("client_secret").getType()
        || OpenAB::Variant::STRING != params.getValue("refresh_token").getType())
    {
      LOG_ERROR() << "Wrong type of parameters" << std::endl;
      return NULL;
    }

    clientId = params.getValue("client_id").getString();
    clientSecret = params.getValue("client_secret").getString();
    refreshToken = params.getValue("refresh_token").getString();
  }
  else
  {
    param = params.getValue("login");
    if (param.invalid())
    {
      LOG_ERROR() << "Parameter 'login' not found" << std::endl;
      return NULL;
    }
    login = param.getString();

    param = params.getValue("password");
    if (param.invalid() || OpenAB::Variant::STRING != param.getType())
    {
      LOG_ERROR() << "Parameter 'password' not found" << std::endl;
      return NULL;
    }
    password = param.getString();
  }

  param = params.getValue("calendar_name");
  if (!param.invalid() && OpenAB::Variant::STRING == param.getType())
  {
    calendarName = param.getString();
  }

  param = params.getValue("item_type");
  if (param.invalid() || param.getType() != OpenAB::Variant::INTEGER)
  {
    LOG_ERROR()<< "Parameter 'item_type' not found "<<param.getType()<<std::endl;
    return NULL;
  }
  switch (param.getInt())
  {
    case OpenAB::eEvent:
        type = OpenAB::eEvent;
        break;
    case OpenAB::eTask:
        type = OpenAB::eTask;
        break;
    default:
      LOG_ERROR()<< "Provided item_type not supported"<<std::endl;
      return NULL;
  }

  if (useOAuth2)
  {
    src = new CalDAVStorage(serverUrl,
                            clientId,
                            clientSecret,
                            refreshToken,
                            calendarUrl,
                            calendarName,
                            type);
  }
  else
  {
    src = new CalDAVStorage(serverUrl,
                            login,
                            password,
                            calendarUrl,
                            calendarName,
                            type);
  }
  if (NULL == src)
  {
    LOG_ERROR() << "Cannot Initialize CardDAV" << std::endl;
    return NULL;
  }

  return src;
}
class CalDAVSourceFactory : OpenAB_Source::Factory
{
  public:
    /*!
     *  @brief Constructor.
     */
  CalDAVSourceFactory()
        : Factory::Factory("CalDAVCalendar")
    {
      LOG_FUNC();
    }
    ;

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~CalDAVSourceFactory()
    {
      LOG_FUNC();
    }
    ;

    OpenAB_Source::Source * newIstance(const OpenAB_Source::Parameters & params)
    {
      return createInstance(params);
    }
};

class CalDAVStorageFactory : OpenAB_Storage::Factory
{
  public:
    /*!
     *  @brief Constructor.
     */
  CalDAVStorageFactory()
        : Factory::Factory("CalDAVCalendar")
    {
      LOG_FUNC();
    }
    ;

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~CalDAVStorageFactory()
    {
      LOG_FUNC();
    }
    ;

    OpenAB_Storage::Storage * newIstance(const OpenAB_Storage::Parameters & params)
    {
      return createInstance(params);
    }
};

}

namespace CalDavSourceFactory
{
REGISTER_PLUGIN_FACTORY(CalDavFactory::CalDAVSourceFactory);
}
namespace CalDavStorageFactory
{
REGISTER_PLUGIN_FACTORY(CalDavFactory::CalDAVStorageFactory);
}
