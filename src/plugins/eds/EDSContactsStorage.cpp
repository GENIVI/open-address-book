/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file EDSContactsStorage.cpp
 */

#include "OpenAB_eds_global.h"
#include "EDSContactsStorage.hpp"


EDSContactsStorage::EDSContactsStorage(const std::string & db)
    : OpenAB_Storage::ContactsStorage(),
      database(db),
      registry(NULL),
      source(NULL),
      client(NULL),
      contactsIterator(NULL)
{
  LOG_FUNC();
}

EDSContactsStorage::~EDSContactsStorage()
{
  LOG_FUNC();
  if (NULL != contactsIterator)
  {
    delete contactsIterator;
  }

  if (NULL != client)
    g_object_unref(client);
  if (NULL != source)
    g_object_unref(source);
  if (NULL != registry)
    g_object_unref(registry);
}

enum OpenAB_Storage::Storage::eInit EDSContactsStorage::init()
{
  LOG_FUNC() << "DB: " << database<<std::endl;

#if GLIB_VERSION_MIN_REQUIRED < GLIB_VERSION_2_36
  /* g_type_init has been deprecated since 2.36 */
  g_type_init();
#endif

  GError *gerror = NULL;

  // 1. Get access to all databases in EDS.
  registry = e_source_registry_new_sync(NULL, &gerror);
  if (!registry)
  {
    LOG_ERROR() << "e_source_registry not found " << GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    return eInitFail;
  }

  // 2. Look up one particular database.
  std::string dbName = database;
  //dbName.append(database);

  source = e_source_registry_ref_source(registry, dbName.c_str());

  if (!source)
  {
    LOG_ERROR() << "e_source not found"<<std::endl;
    GERROR_FREE(gerror);
    return eInitFail;
  }

  GERROR_FREE(gerror);
  client = (EBookClient *) e_book_client_connect_sync(source, NULL, &gerror);
  if (gerror)
  {
    LOG_ERROR() << "Error e_book_client_connect_sync results: " << GERROR_MESSAGE(gerror)<<std::endl;
    return eInitFail;
  }
  LOG_VERBOSE() << "e_book_client_connect_sync\n"<<std::endl;

  return eInitOk;
}

enum OpenAB_Source::Source::eGetItemRet EDSContactsStorage::getItem(OpenAB::SmartPtr<OpenAB::PIMItem> & item)
{
  if (NULL == contactsIterator)
  {
    contactsIterator = new EDSContactsStorageItemIterator();
    if (EDSContactsStorageItemIterator::eCursorInitOK != contactsIterator->cursorInit(client))
      {
        LOG_ERROR() << "Error Cannot Init the EDS IndexElemIterator"<<std::endl;
        return eGetItemRetEnd;
      }
  }

  OpenAB_Storage::StorageItem* sItem = contactsIterator->next();
  if (NULL == sItem)
  {
    return eGetItemRetEnd;
  }

  item = sItem->item;
  return eGetItemRetOk;
}

enum OpenAB_Source::Source::eSuspendRet EDSContactsStorage::suspend()
{
  return eSuspendRetNotSupported;
}

enum OpenAB_Source::Source::eResumeRet EDSContactsStorage::resume()
{
  return eResumeRetNotSupported;
}

enum OpenAB_Source::Source::eCancelRet EDSContactsStorage::cancel()
{
  return eCancelRetNotSupported;
}

int EDSContactsStorage::getTotalCount() const
{
  if (contactsIterator)
    return contactsIterator->getSize();

  return 0;
}

OpenAB::PIMItem::Revision EDSContactsStorage::getRevision(const OpenAB::PIMItem::ID& id)
{
  OpenAB::PIMItem::Revision revision;
  GError *gerror = NULL;
  EContact * contact;
  if (!e_book_client_get_contact_sync(client, id.c_str(), &contact, NULL, &gerror))
  {
    LOG_ERROR() << "Error e_book_client_get_contact_sync results: " << GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    return "";
  }
  const char * rev  = (const char *)e_contact_get_const(contact,E_CONTACT_REV);
  if (rev)
  {
    revision = rev;
  }
  g_object_unref (contact);

  return revision;
}

OpenAB::PIMItem::Revisions EDSContactsStorage::getRevisions(const OpenAB::PIMItem::IDs& ids)
{
  OpenAB::PIMItem::Revisions revisions;
  OpenAB::PIMItem::IDs::const_iterator it;
  for (it = ids.begin(); it != ids.end(); ++it)
  {
    revisions.push_back(getRevision((*it)));
  }

  return revisions;
}

enum OpenAB_Storage::Storage::eAddItem EDSContactsStorage::addContact(const std::string & vCard,
                                                                   OpenAB::PIMItem::ID & newId,
                                                                   OpenAB::PIMItem::Revision & revision)
{
  GError *gerror = NULL;
  char * uid_new;

  std::string vcard = vCard;

  //remove UID and let EDS generate new one
  std::string::size_type uidStart = vCard.find("UID:");
  if (uidStart != std::string::npos)
  {
    std::string::size_type uidEnd = vCard.find("\n", uidStart);
    vcard = vcard.erase(uidStart, uidEnd-uidStart);
  }

  EContact * contact_new = e_contact_new_from_vcard(vcard.c_str());
  if (NULL == contact_new)
  {
    LOG_ERROR() << "Error e_contact_new_from_vcard"<<std::endl;
    return eAddItemFail;
  }
  if (!e_book_client_add_contact_sync(client, contact_new, &uid_new, NULL, &gerror))
  {
    LOG_ERROR() << "Error e_book_client_add_contact_sync results: " << GERROR_MESSAGE(gerror)<<std::endl;
    g_object_unref(contact_new);
    GERROR_FREE(gerror);
    return eAddItemFail;
  }
  newId = uid_new;
  g_free(uid_new);
  g_object_unref(contact_new);
  revision = getRevision(newId);
  GERROR_FREE(gerror);
  return eAddItemOk;
}

enum OpenAB_Storage::Storage::eAddItem EDSContactsStorage::addContacts(const std::vector<std::string> & vCards,
                                                                    OpenAB::PIMItem::IDs & newIds,
                                                                    OpenAB::PIMItem::Revisions & revisions)
{
  GError *gerror = NULL;
  GSList * contacts = NULL;
  GSList * new_uids;

  for(unsigned int i = 0; i < vCards.size(); ++i)
  {
    std::string vcard = vCards[i];

    //remove UID and let EDS generate new one
    std::string::size_type uidStart = vcard.find("UID:");
    if (uidStart != std::string::npos)
    {
      std::string::size_type uidEnd = vcard.find("\n", uidStart);
      vcard = vcard.erase(uidStart, uidEnd-uidStart);
    }


    EContact * contact_new = e_contact_new_from_vcard(vcard.c_str());
    if (NULL == contact_new)
    {
      LOG_ERROR() << "Error e_contact_new_from_vcard"<<std::endl;
      g_slist_free_full(contacts, (GDestroyNotify) g_object_unref);
      return eAddItemFail;
    }
    contacts = g_slist_append(contacts, contact_new);
  }


  if (!e_book_client_add_contacts_sync(client, contacts, &new_uids, NULL, &gerror))
  {
    LOG_ERROR() << "Error e_book_client_add_contact_sync results: " << GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    g_slist_free_full(contacts, (GDestroyNotify) g_object_unref);
    return eAddItemFail;
  }

  GSList *l = new_uids;
  newIds.clear();
  while(l)
  {
    std::string id = (const gchar*)l->data;
    newIds.push_back(id);
    l = g_slist_next(l);
  }

  g_slist_free_full(contacts, (GDestroyNotify) g_object_unref);
  e_client_util_free_string_slist(new_uids);

  revisions = getRevisions(newIds);
  return eAddItemOk;
}

enum OpenAB_Storage::Storage::eModifyItem EDSContactsStorage::modifyContact(const std::string & vCard,
                                                                         const OpenAB::PIMItem::ID & id,
                                                                         OpenAB::PIMItem::Revision & revision)
{
  GError *gerror = NULL;
  EContact * contact_new = e_contact_new_from_vcard_with_uid(vCard.c_str(), id.c_str());
  if (NULL == contact_new)
  {
    LOG_ERROR() << "Error e_contact_new_from_vcard_with_uid uid: " << id<<std::endl;
    return eModifyItemFail;
  }
  if (!e_book_client_modify_contact_sync(client, contact_new, NULL, &gerror))
  {
    LOG_ERROR() << "Error e_book_client_modify_contact_sync results: " << GERROR_MESSAGE(gerror)<<std::endl;
    g_object_unref(contact_new);
    GERROR_FREE(gerror);
    return eModifyItemFail;
  }

  g_object_unref(contact_new);
  revision = getRevision(id);
  GERROR_FREE(gerror);
  return eModifyItemOk;
}

enum OpenAB_Storage::Storage::eModifyItem EDSContactsStorage::modifyContacts(const std::vector<std::string> & vCards,
                                                                          const OpenAB::PIMItem::IDs & ids,
                                                                          OpenAB::PIMItem::Revisions & revisions)
{
  GError *gerror = NULL;
  GSList * contacts = NULL;

  if (vCards.size() != ids.size())
  {
    return eModifyItemFail;
  }

  for(unsigned int i = 0; i < vCards.size(); ++i)
  {
    EContact * contact_new = e_contact_new_from_vcard_with_uid(vCards[i].c_str(), ids[i].c_str());
    if (NULL == contact_new)
    {
      LOG_ERROR() << "Error e_contact_new_from_vcard_with_uid"<<std::endl;
      g_slist_free_full(contacts, (GDestroyNotify) g_object_unref);
      return eModifyItemFail;
    }
    contacts = g_slist_append(contacts, contact_new);
  }

  if (!e_book_client_modify_contacts_sync(client, contacts, NULL, &gerror))
  {
    LOG_ERROR() << "Error e_book_client_modify_contacts_sync results: " << GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    g_slist_free_full(contacts, (GDestroyNotify) g_object_unref);
    return eModifyItemFail;
  }
  revisions = getRevisions(ids);
  g_slist_free_full(contacts, (GDestroyNotify) g_object_unref);

  return eModifyItemOk;
}

enum OpenAB_Storage::Storage::eRemoveItem EDSContactsStorage::removeContact(const OpenAB::PIMItem::ID & id)
{
  GError *gerror = NULL;
  if (!e_book_client_remove_contact_by_uid_sync(client, id.c_str(), NULL, &gerror))
  {
    LOG_ERROR() << "Error e_book_client_remove_contact_by_uid_sync results: " << GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    return eRemoveItemFail;
  }
  GERROR_FREE(gerror);
  return eRemoveItemOk;
}

enum OpenAB_Storage::Storage::eRemoveItem EDSContactsStorage::removeContacts(const OpenAB::PIMItem::IDs & ids)
{
  GError *gerror = NULL;
  GSList *contacts_ids = NULL;

  for(unsigned int i = 0; i < ids.size(); ++i)
  {
    contacts_ids = g_slist_append(contacts_ids, (void*)ids[i].c_str());
  }

  if (!e_book_client_remove_contacts_sync(client, contacts_ids, NULL, &gerror))
  {
    LOG_ERROR() << "Error e_book_client_remove_contacts_sync results: " << GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    g_slist_free(contacts_ids);
    return eRemoveItemFail;
  }

  g_slist_free(contacts_ids);

  return eRemoveItemOk;
}

enum OpenAB_Storage::Storage::eGetItem EDSContactsStorage::getContact(const OpenAB::PIMItem::ID & id, OpenAB::SmartPtr<OpenAB::PIMContactItem> & item)
{
  GError *gerror = NULL;
  EContact * contact;
  std::string vCard;
  if (!e_book_client_get_contact_sync(client, id.c_str(), &contact, NULL, &gerror))
  {
    LOG_ERROR() << "Error e_book_client_get_contact_sync results: " << GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    return eGetItemFail;
  }
  //e_contact_inline_local_photos(contact, &gerror);
  gchar * gvc =  e_vcard_to_string(E_VCARD(contact), EVC_FORMAT_VCARD_30);
  vCard = gvc;
  const char * rev  = (const char *)e_contact_get_const(contact, E_CONTACT_REV);

  OpenAB::PIMContactItem* newItem = new OpenAB::PIMContactItem();
  newItem->parse(vCard);
  newItem->setId(id);
  newItem->setRevision(rev);
  item = newItem;

  g_free (gvc);
  g_object_unref(contact);
  GERROR_FREE(gerror);
  return eGetItemOk;
}

enum OpenAB_Storage::Storage::eGetItem EDSContactsStorage::getContacts(const OpenAB::PIMItem::IDs & ids,
                                                                    std::vector<OpenAB::SmartPtr<OpenAB::PIMContactItem> > & items)
{
  for (unsigned int i = 0; i < ids.size(); ++i)
  {
    OpenAB::SmartPtr<OpenAB::PIMContactItem> contact;
    if (eGetItemFail == getContact(ids[i], contact))
    {
      return eGetItemFail;
    }
    items.push_back(contact);
  }

  return eGetItemOk;
}

enum OpenAB_Storage::Storage::eGetRevisions EDSContactsStorage::getRevisions(std::map<std::string, std::string>& revisions)
{
  GError *gerror = NULL;
  GSList* uids;
  if (!e_book_client_get_contacts_uids_sync (client, "", &uids, NULL, &gerror))
  {
    LOG_ERROR() << "Error e_book_client_get_contacts_uids_sync results: " << GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    return eGetRevisionsFail;
  }

  OpenAB::PIMItem::IDs ids;
  GSList *l = uids;
  while(l)
  {
    ids.push_back(std::string((const gchar*)l->data));
    l = g_slist_next(l);
  }
  e_client_util_free_string_slist(uids);
  uids = NULL;

  OpenAB::PIMItem::Revisions revs = getRevisions(ids);
  for (unsigned int i = 0; i < ids.size(); ++i)
  {
    revisions[ids[i]] = revs[i];
  }

  return eGetRevisionsOk;
}

enum OpenAB_Storage::Storage::eGetRevisions EDSContactsStorage::getChangedRevisions(const std::string&,
                                                                                 std::map<std::string, std::string>& ,
                                                                                 std::vector<OpenAB::PIMItem::ID>&)
{
  return eGetRevisionsFail;
}

enum OpenAB_Storage::Storage::eGetSyncToken EDSContactsStorage::getLatestSyncToken(std::string&)
{
  return eGetSyncTokenFail;
}

OpenAB_Storage::StorageItemIterator* EDSContactsStorage::newStorageItemIterator()
{
  EDSContactsStorageItemIterator * ie = new EDSContactsStorageItemIterator();
  if (NULL == ie)
  {
    LOG_ERROR() << "Error Cannot create the EDS IndexElemIterator"<<std::endl;
    return NULL;
  }
  if (ie->eCursorInitOK != ie->cursorInit(client))
  {
    LOG_ERROR() << "Error Cannot Init the EDS IndexElemIterator"<<std::endl;
    delete ie;
    return NULL;
  }
  return ie;
}

namespace EDSContactsFactory
{
  OpenAB_Storage::Storage* createInstance (const OpenAB_Storage::Parameters& params)
  {
    LOG_FUNC();
    OpenAB::Variant param = params.getValue("db");
    if (param.invalid() || param.getType() != OpenAB::Variant::STRING)
    {
      LOG_ERROR() << "Parameter 'db' not found"<<std::endl;
      return NULL;
    }
    EDSContactsStorage * ab = new EDSContactsStorage(param.getString());
    if (NULL == ab)
    {
      LOG_ERROR() << "Cannot Initialize EDSAddressbook"<<std::endl;
      return NULL;
    }
    return ab;
  }

  class EDSContactsStorageFactory : OpenAB_Storage::Factory
  {
    public:
      /*!
       *  @brief Constructor.
       */
      EDSContactsStorageFactory():
        Factory::Factory("EDSContacts"){};

      /*!
       *  @brief Destructor, virtual by default.
       */
      virtual ~EDSContactsStorageFactory(){};

      OpenAB_Storage::Storage * newIstance(const OpenAB_Storage::Parameters & params)
      {
        return createInstance(params);
      };
  };

  class EDSContactsSourceFactory : OpenAB_Source::Factory
  {
    public:
      /*!
       *  @brief Constructor.
       */
      EDSContactsSourceFactory()
          : Factory::Factory("EDSContacts")
      {
      }
      ;

      /*!
       *  @brief Destructor, virtual by default.
       */
      virtual ~EDSContactsSourceFactory()
      {
      }
      ;

      OpenAB_Source::Source * newIstance(const OpenAB_Source::Parameters & params)
      {
        return createInstance(params);
      }
  };
}

namespace EDSContactsStorageFactory
{
  REGISTER_PLUGIN_FACTORY(EDSContactsFactory::EDSContactsStorageFactory);
}

namespace EDSContactsSourceFactory
{

  REGISTER_PLUGIN_FACTORY(EDSContactsFactory::EDSContactsSourceFactory);
}
