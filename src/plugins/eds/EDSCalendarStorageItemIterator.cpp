/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file EDSCalendarStorageItemIterator.cpp
 */

#include <PIMItem/Calendar/PIMCalendarItem.hpp>
#include "EDSCalendarStorageItemIterator.hpp"
#include "EDSCalendarStorageCommon.hpp"
#include <helpers/StringHelper.hpp>

EDSCalendarStorageItemIterator::EDSCalendarStorageItemIterator()
: StorageItemIterator(),
  total(0)
{
  LOG_FUNC();
  elem.item = NULL;
}

EDSCalendarStorageItemIterator::~EDSCalendarStorageItemIterator()
{
  databaseFile.close();
}

enum EDSCalendarStorageItemIterator::eCursorInit EDSCalendarStorageItemIterator::cursorInit(const std::string& dbFileName)
{
  LOG_FUNC();
  if (databaseFile.is_open())
  {
    databaseFile.close();
  }

  databaseFile.open(dbFileName.c_str(), std::ios_base::in);
  if (!databaseFile.is_open())
  {
    LOG_ERROR()<<"Cannot open database file: "<<dbFileName<<std::endl;
    return eCursorInitFail;
  }

  /*
   * TODO: count number of events in file
   */
  return eCursorInitOK;
}

OpenAB_Storage::StorageItem* EDSCalendarStorageItemIterator::next()
{
  if (events.empty())
  {
    if (eFetchEventsOK != fetchEvents(100))
    {
      LOG_DEBUG()<<"Fetch events failed"<<std::endl;
      return NULL;
    }
  }

  //get next object
  std::string iCal = events.front();
  events.pop_front();

  if (iCal.empty())
  {
    LOG_DEBUG()<<"empty file"<<std::endl;
    return NULL;
  }

  //get UID of object
  std::string::size_type pos = 0;
  std::string uid = OpenAB::cut(iCal, "UID:", "\n", pos);
  OpenAB::trimSpaces(uid);

  std::vector<std::string> iCals;
  iCals.push_back(iCal);

  /*
   * check if following events contains the same UID, if so
   * process them at the same time.
   * If required fetch next batch.
   */
  bool sameEvent = true;
  while (sameEvent)
  {
    sameEvent = false;
    if (events.empty())
    {
      fetchEvents(100);
    }

    if (!events.empty())
    {
      std::string::size_type pos = 0;
      std::string uid2 = OpenAB::cut(events.front(), "UID:", "\n", pos);
      OpenAB::trimSpaces(uid2);
      if (uid == uid2)
      {
        iCals.push_back(events.front());
        events.pop_front();
        sameEvent = true;
      }
    }
  }

  OpenAB::PIMCalendarItem* newItem = EDSCalendarStorageCommon::processObject(iCals);
  elem.id = newItem->getId();
  elem.item = newItem;

  return &elem;
}

enum EDSCalendarStorageItemIterator::eFetchEvents EDSCalendarStorageItemIterator::fetchEvents(int fetchsize)
{
  LOG_FUNC();
  events.clear();

  while(fetchsize--)
  {
    std::string event = EDSCalendarStorageCommon::cutVObject(databaseFile);
    LOG_DEBUG()<<"Fetched event: "<<event<<std::endl;
    if (event.empty())
    {
      if (events.empty())
        return eFetchEventsEND;
      else
        return eFetchEventsOK;
    }
    events.push_back(event);
  }

  return eFetchEventsOK;
}

OpenAB_Storage::StorageItem EDSCalendarStorageItemIterator::operator*()
{
  return elem;
}

OpenAB_Storage::StorageItem* EDSCalendarStorageItemIterator::operator->()
{
  return &elem;
}

unsigned int EDSCalendarStorageItemIterator::getSize() const
{
  return total;
}
