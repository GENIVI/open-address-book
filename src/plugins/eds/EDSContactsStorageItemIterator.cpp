/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file EDSContactsStorageItemIterator.cpp
 */

#include <PIMItem/Contact/PIMContactItem.hpp>
#include "EDSContactsStorageItemIterator.hpp"

EDSContactsStorageItemIterator::EDSContactsStorageItemIterator()
:StorageItemIterator()
,cursor(NULL)
,contacts(NULL)
,total(0)
{
  LOG_FUNC();
  elem.item = NULL;
}

EDSContactsStorageItemIterator::~EDSContactsStorageItemIterator()
{
  LOG_FUNC();
  FREE_CONTACTS(contacts);
  if (NULL != cursor)
    g_object_unref(cursor);
}

enum EDSContactsStorageItemIterator::eCursorInit EDSContactsStorageItemIterator::cursorInit(EBookClient* c)
{
  LOG_FUNC();

  GError *gerror = NULL;

  EContactField sort_fields[]      = { E_CONTACT_FULL_NAME };
  EBookCursorSortType sort_types[] = { E_BOOK_CURSOR_SORT_ASCENDING };

  e_book_client_get_cursor_sync(c, NULL, sort_fields, sort_types, 1, &cursor, NULL, &gerror);

  if (NULL != gerror)
  {
    LOG_ERROR() << "Error e_book_client_get_cursor_sync results: " << GERROR_MESSAGE(gerror)<<std::endl ;
    GERROR_FREE(gerror);
    return eCursorInitFail;
  }

  total = e_book_client_cursor_get_total(cursor);
  LOG_VERBOSE() << "Number of contacts: " << total <<std::endl;

  GERROR_FREE(gerror);

  return eCursorInitOK;
}

OpenAB_Storage::StorageItem* EDSContactsStorageItemIterator::next()
{
  GError * gerror   = NULL;

  static GSList * cont = NULL;
  if (NULL == cont)
  {
    if (eFetchContactsOK != fetchContacts(1000))
    {
      return NULL;
    }
    cont = contacts;
  }

  EContact *data = static_cast<EContact *>(cont->data);
  char * vcard = e_vcard_to_string (E_VCARD (data), EVC_FORMAT_VCARD_30);
  const char * id   = (const char *)e_contact_get_const(data,E_CONTACT_UID);
  const char * rev  = (const char *)e_contact_get_const(data,E_CONTACT_REV);

  elem.id = id;
  OpenAB::PIMContactItem* newItem = new OpenAB::PIMContactItem();
  newItem->parse(vcard);
  newItem->setId(id);
  newItem->setRevision(rev);
  elem.item = newItem;

  g_free(vcard);

  cont = cont->next;

  GERROR_FREE(gerror);
  return &elem;
}

enum EDSContactsStorageItemIterator::eFetchContacts EDSContactsStorageItemIterator::fetchContacts(int fetchsize)
{
  GError * gerror   = NULL;

  FREE_CONTACTS(contacts);

  LOG_VERBOSE() << "Cursor Position: " << e_book_client_cursor_get_position(cursor)<<std::endl;

  if (e_book_client_cursor_get_position(cursor) > total){
    FREE_CONTACTS(contacts);
    return eFetchContactsEND;
  }

  int num = e_book_client_cursor_step_sync(cursor,
                                           E_BOOK_CURSOR_STEP_FETCH ,
                                           E_BOOK_CURSOR_ORIGIN_CURRENT,
                                           fetchsize, &contacts, NULL, &gerror);
  if (-1 == num){
    LOG_ERROR() << "Error e_book_client_cursor_step_sync results: " << GERROR_MESSAGE(gerror)<<std::endl ;
    GERROR_FREE(gerror);
    FREE_CONTACTS(contacts);
    return eFetchContactsFail;
  }
  if (0 == num){
    GERROR_FREE(gerror);
    FREE_CONTACTS(contacts);
    return eFetchContactsEND;
  }

  GERROR_FREE(gerror);
  e_book_client_cursor_step_sync(cursor,
                                 E_BOOK_CURSOR_STEP_MOVE,
                                 E_BOOK_CURSOR_ORIGIN_CURRENT,
                                 fetchsize, NULL, NULL,
                                 &gerror);
  if (NULL != gerror){
    LOG_ERROR() << "Error e_book_client_cursor_step_sync results: " << GERROR_MESSAGE(gerror) <<std::endl;
    GERROR_FREE(gerror);
    FREE_CONTACTS(contacts);
    return eFetchContactsFail;
  }
  GERROR_FREE(gerror);
  return eFetchContactsOK;
}


OpenAB_Storage::StorageItem EDSContactsStorageItemIterator::operator*()
{
  return elem;
}

OpenAB_Storage::StorageItem* EDSContactsStorageItemIterator::operator->()
{
  return &elem;
}

unsigned int EDSContactsStorageItemIterator::getSize() const
{
  return total;
}
