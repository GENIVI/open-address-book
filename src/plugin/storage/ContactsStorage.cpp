/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file ContactsStorage.cpp
 */

#include "ContactsStorage.hpp"

namespace OpenAB_Storage {

ContactsStorage::ContactsStorage() :
    Storage(OpenAB::eContact)
{
}

ContactsStorage::~ContactsStorage()
{
}

enum Storage::eAddItem ContactsStorage::addItem(const OpenAB::SmartPtr<OpenAB::PIMItem>& item,
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

  return addContact(item->getRawData(), newId, revision);
}

enum Storage::eAddItem ContactsStorage::addItems(const std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > & items,
                                                 OpenAB::PIMItem::IDs & newIds,
                                                 OpenAB::PIMItem::Revisions & revisions)
{
  LOG_FUNC();
  std::vector<std::string> vCards;
  std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> >::const_iterator it;
  for(it = items.begin(); it != items.end(); ++it)
  {
    if(!((*it).getPointer() && (*it)->getType() == getItemType()))
    {
      if ((*it).getPointer())
      {
        LOG_ERROR()<<"Mismatched item types"<<std::endl;
      }
      else
      {
        LOG_ERROR()<<"Null item"<<std::endl;
      }

      return Storage::eAddItemFail;
    }
    vCards.push_back((*it)->getRawData());
  }

  return addContacts(vCards, newIds, revisions);
}

enum Storage::eModifyItem ContactsStorage::modifyItem(const OpenAB::SmartPtr<OpenAB::PIMItem>& item,
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


  return modifyContact(item->getRawData(), id, revision);
}

enum Storage::eModifyItem ContactsStorage::modifyItems(const std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > & items,
                                                       const OpenAB::PIMItem::IDs & ids,
                                                       OpenAB::PIMItem::Revisions & revisions)
{
  LOG_FUNC();
  std::vector<std::string> vCards;
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
    vCards.push_back((*it)->getRawData());
  }

  return modifyContacts(vCards, ids, revisions);
}

enum Storage::eRemoveItem ContactsStorage::removeItem(const OpenAB::PIMItem::ID & id)
{
  return removeContact(id);
}

enum Storage::eRemoveItem ContactsStorage::removeItems(const OpenAB::PIMItem::IDs & ids)
{
  return removeContacts(ids);
}

enum Storage::eGetItem ContactsStorage::getItem(const OpenAB::PIMItem::ID & id,
                                                OpenAB::SmartPtr<OpenAB::PIMItem> & item)
{
  OpenAB::SmartPtr<OpenAB::PIMContactItem> contactItem;
  Storage::eGetItem result = getContact(id, contactItem);
  /**
   * @todo find more elegant way to take ownership of pointer.
   */
  if (result == Storage::eGetItemOk)
  {
	  item = new OpenAB::PIMContactItem(*(contactItem.getPointer()));
  }
  return result;
}

enum Storage::eGetItem ContactsStorage::getItems(const OpenAB::PIMItem::IDs & id,
                                                 std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > & items)
{
  LOG_FUNC()<<std::endl;
  std::vector<OpenAB::SmartPtr<OpenAB::PIMContactItem> > contactItems;
  Storage::eGetItem result = getContacts(id, contactItems);
  /**
   * find more elegant way to take ownership of pointer.
   */
  if (result == Storage::eGetItemOk)
  {
    for (unsigned int i = 0; i < contactItems.size(); ++i)
    {
      items.push_back (new OpenAB::PIMContactItem(*(contactItems[i].getPointer())));
    }
  }

  return result;
}


} // namespace Ias
