/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file CalendarStorage.cpp
 */

#include "CalendarStorage.hpp"

namespace OpenAB_Storage {

CalendarStorage::CalendarStorage(OpenAB::PIMItemType type) :
    Storage(type)
{
}

CalendarStorage::~CalendarStorage()
{
}

enum Storage::eAddItem CalendarStorage::addItem(const OpenAB::SmartPtr<OpenAB::PIMItem>& item,
                                                OpenAB::PIMItem::ID & newId,
                                                OpenAB::PIMItem::Revision & revision)
{
  LOG_FUNC();
  if (!(item.getPointer() && item->getType() == getItemType()))
  {
    if (item.getPointer())
    {
      LOG_ERROR()<<"Mismatched item types"<<std::endl;
    }
    else
    {
      LOG_ERROR()<<"Null item"<<std::endl;
    }

    return Storage::eAddItemFail;
  }

  return addObject(item->getRawData(), newId, revision);
}

enum Storage::eAddItem CalendarStorage::addItems(const std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > & items,
                                                 OpenAB::PIMItem::IDs & newIds,
                                                 OpenAB::PIMItem::Revisions & revisions)
{
  LOG_FUNC();
  std::vector<std::string> iCals;
  std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> >::const_iterator it;
  for(it = items.begin(); it != items.end(); ++it)
  {
    if(!((*it).getPointer() && (*it)->getType() == getItemType()))
    {
      if ((*it).getPointer())
      {
        LOG_ERROR()<<"Mismatched item types: item type - "<<(*it)->getType()<<" storage type "<<getItemType()<<std::endl;
      }
      else
      {
        LOG_ERROR()<<"Null item"<<std::endl;
      }

      return Storage::eAddItemFail;
    }
    iCals.push_back((*it)->getRawData());
  }

  return addObjects(iCals, newIds, revisions);
}

enum Storage::eModifyItem CalendarStorage::modifyItem(const OpenAB::SmartPtr<OpenAB::PIMItem>& item,
                                                      const OpenAB::PIMItem::ID & id,
                                                      OpenAB::PIMItem::Revision & revision)
{
  LOG_FUNC();
  if (!(item.getPointer() && item->getType() == getItemType()))
  {
    if (item.getPointer())
    {
      LOG_ERROR()<<"Mismatched item types"<<std::endl;
    }
    else
    {
      LOG_ERROR()<<"Null item"<<std::endl;
    }
    return Storage::eModifyItemFail;
  }


  return modifyObject(item->getRawData(), id, revision);
}

enum Storage::eModifyItem CalendarStorage::modifyItems(const std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > & items,
                                                       const OpenAB::PIMItem::IDs & ids,
                                                       OpenAB::PIMItem::Revisions & revisions)
{
  LOG_FUNC();
  std::vector<std::string> iCals;
  std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> >::const_iterator it;
  for(it = items.begin(); it != items.end(); ++it)
  {
    if (!((*it).getPointer() && (*it)->getType() == getItemType()))
    {
      if ((*it).getPointer())
      {
        LOG_ERROR()<<"Mismatched item types"<<std::endl;
      }
      else
      {
        LOG_ERROR()<<"Null item"<<std::endl;
      }
      return Storage::eModifyItemFail;
    }
    iCals.push_back((*it)->getRawData());
  }

  return modifyObjects(iCals, ids, revisions);
}

enum Storage::eRemoveItem CalendarStorage::removeItem(const OpenAB::PIMItem::ID & id)
{
  return removeObject(id);
}

enum Storage::eRemoveItem CalendarStorage::removeItems(const OpenAB::PIMItem::IDs & ids)
{
  return removeObjects(ids);
}

enum Storage::eGetItem CalendarStorage::getItem(const OpenAB::PIMItem::ID & id,
                                                OpenAB::SmartPtr<OpenAB::PIMItem> & item)
{
  Storage::eGetItem result = eGetItemFail;
  if (OpenAB::eEvent == getItemType())
  {
    OpenAB::SmartPtr<OpenAB::PIMCalendarEventItem> calendarItem;
    result = getEvent(id, calendarItem);

    /**
     * find more elegant way to take ownership of pointer.
     */
    if (result == Storage::eGetItemOk)
    {
      item = new OpenAB::PIMCalendarEventItem(*(calendarItem.getPointer()));
    }
  }
  else
  {
    OpenAB::SmartPtr<OpenAB::PIMCalendarTaskItem> calendarItem;
    result = getTask(id, calendarItem);

    /**
     * find more elegant way to take ownership of pointer.
     */
    if (result == Storage::eGetItemOk)
    {
      item = new OpenAB::PIMCalendarTaskItem(*(calendarItem.getPointer()));
    }
  }

  return result;
}

enum Storage::eGetItem CalendarStorage::getItems(const OpenAB::PIMItem::IDs & id,
                                                 std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > & items)
{
  Storage::eGetItem result = eGetItemFail;
  if (OpenAB::eEvent == getItemType())
  {
    std::vector<OpenAB::SmartPtr<OpenAB::PIMCalendarEventItem> > calendarItems;
    result = getEvents(id, calendarItems);
    /**
     * find more elegant way to take ownership of pointer.
     */
    if (result == Storage::eGetItemOk)
    {
      for (unsigned int i = 0; i < calendarItems.size(); ++i)
      {
        items.push_back(new OpenAB::PIMCalendarEventItem(*(calendarItems[i].getPointer())));
      }
    }
  }
  else
  {
    std::vector<OpenAB::SmartPtr<OpenAB::PIMCalendarTaskItem> > calendarItems;
    result = getTasks(id, calendarItems);
    /**
     * find more elegant way to take ownership of pointer.
     */
    if (result == Storage::eGetItemOk)
    {
      for (unsigned int i = 0; i < calendarItems.size(); ++i)
      {
        items.push_back(new OpenAB::PIMCalendarTaskItem(*(calendarItems[i].getPointer())));
      }
    }
  }
  return result;
}


} // namespace Ias
