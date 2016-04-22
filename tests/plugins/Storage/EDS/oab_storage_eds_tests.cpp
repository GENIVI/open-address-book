/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file oab_storage_eds_tests.cpp
 */
#include <gtest/gtest.h>
#include <string>
#include "helpers/PluginManager.hpp"
#include "PIMItem/Contact/PIMContactItem.hpp"
#include "PIMItem/Calendar/PIMCalendarItem.hpp"
#include "glib2/OpenAB_glib2_global.h"
#include <libebook/libebook.h>
#include <libedata-book/libedata-book.h>

bool createSource(const std::string& name)
{
  ESourceRegistry* sourceRegistry;
  ESource* newSource;
  GError* gerror = NULL;

  sourceRegistry = e_source_registry_new_sync(NULL, &gerror);
  if (!sourceRegistry)
  {
    std::cout<<"Cannot create source registry "<<name<<": "<<GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    return false;
  }

  newSource = e_source_new_with_uid(name.c_str(), NULL, &gerror);
  if(!newSource)
  {
    std::cout<<"Cannot create source registry "<<name<<": "<<GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    g_object_unref (sourceRegistry);
    return false;
  }

  e_source_set_display_name (newSource, name.c_str());

  g_type_ensure (E_TYPE_SOURCE_ADDRESS_BOOK);
  ESourceBackend *backend_setup = E_SOURCE_BACKEND(e_source_get_extension (newSource,
                                                                           E_SOURCE_EXTENSION_ADDRESS_BOOK));
  if(backend_setup)
  {
    e_source_backend_set_backend_name(backend_setup, "local");
  }

  if(!e_source_registry_commit_source_sync(sourceRegistry,
                                           newSource,
                                           NULL,
                                           &gerror))
  {
    std::cout<<"Cannot commit source "<<name<<": "<<GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    g_object_unref (newSource);
    g_object_unref (sourceRegistry);
    return false;
  }
  std::cout<<"Source created "<<name<<std::endl;
  g_object_unref (newSource);
  g_object_unref (sourceRegistry);
  return true;
}

bool removePeerData(const std::string& path)
{
  char command[255];
  sprintf(command, "rm -rf %s", path.c_str());
  system(command);

  return true;
}

bool removeSource(const std::string& name)
{
  ESourceRegistry* sourceRegistry;
  ESource* source;
  GError* gerror = NULL;

  sourceRegistry = e_source_registry_new_sync(NULL, &gerror);

  if (!sourceRegistry)
  {
    LOG_ERROR()<<"Cannot crate source registry "<<name<<": "<<GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    return false;
  }

  source = e_source_registry_ref_source (sourceRegistry, name.c_str());
  if(NULL == source)
  {
    LOG_ERROR()<<"Cannot ref source "<<name<<": "<<GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    g_object_unref (sourceRegistry);
    return false;
  }

  if(!e_source_remove_sync (source, NULL, &gerror))
  {
    LOG_ERROR()<<"Cannot remove source "<<name<<": "<<GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    g_object_unref (source);
    g_object_unref (sourceRegistry);
    return false;
  }
  g_object_unref (source);
  g_object_unref (sourceRegistry);

  const char* dataDir = g_get_user_data_dir();
  std::string peerDataDir = std::string(dataDir) + std::string("/evolution/addressbook/") + name;

  if(!removePeerData(peerDataDir))
  {
    LOG_ERROR()<<"Cannot remove source data "<<peerDataDir<<": "<<GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    return false;
  }

  return true;
}

class EDSStorageTest: public ::testing::Test
{
public:
    EDSStorageTest() : ::testing::Test()
    {
    }

    ~EDSStorageTest()
    {
    }

protected:
    // Sets up the test fixture.
    virtual void SetUp()
    {
      OpenAB::Logger::setDefaultLogger(NULL);
      OpenAB::Logger::OutLevel() = OpenAB::Logger::Error;
      removeSource("oab");
      createSource("oab");
    }

    // Tears down the test fixture.
    virtual void TearDown()
    {
       //removeSource("oab");
    }
};

static const char* vcard0 = \
"BEGIN:VCARD\n"
"VERSION:3.0\n"
"N:Surname1;Name1;Middle1;Perfix1;Suffix1\n"
"END:VCARD\n";

static const char* vcard1 = \
"BEGIN:VCARD\n"
"VERSION:3.0\n"
"N:Surname2;Name2;Middle2;Perfix2;Suffix2\n"
"END:VCARD\n";


TEST_F(EDSStorageTest, testEmptyParams)
{
  OpenAB_Storage::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_FALSE(s);
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testInitWrongDB)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "nonexistingdb");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitFail, s->init());
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testInitCorrectDB)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testEmptyStorageIterator)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());

  OpenAB_Storage::StorageItemIterator* iter = s->newStorageItemIterator();
  OpenAB_Storage::StorageItem* item = NULL;
  item = iter->next();
  ASSERT_FALSE(item);
  delete iter;

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testAddContact)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());
  OpenAB::PIMItem* newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard0));
  OpenAB::SmartPtr<OpenAB::PIMItem> newItem = newContact;
  OpenAB::PIMItem::ID itemId = "";
  OpenAB::PIMItem::Revision itemRev = "";
  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemOk, s->addItem(newItem, itemId, itemRev));
  ASSERT_NE("", itemId);
  ASSERT_NE("", itemRev);

  //now we should be able to retrieve contact using iterator
  OpenAB_Storage::StorageItemIterator* iter = s->newStorageItemIterator();
  ASSERT_TRUE(iter->next());
  ASSERT_EQ(itemId, (*iter)->id);
  ASSERT_EQ(OpenAB::eContact, (*iter)->item->getType());
  ASSERT_NE("", (*iter)->item->getRawData());
  //ensure that there is only one contact in db
  ASSERT_FALSE(iter->next());
  delete iter;

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testGetContact)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());

  //first add contact to db
  OpenAB::PIMItem* newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard0));
  OpenAB::SmartPtr<OpenAB::PIMItem> newItem = newContact;
  OpenAB::PIMItem::ID itemId = "";
  OpenAB::PIMItem::Revision itemRev = "";
  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemOk, s->addItem(newItem, itemId, itemRev));
  ASSERT_NE("", itemId);
  ASSERT_NE("", itemRev);

  //now we should be able to retrieve contact using getItem
  OpenAB::SmartPtr<OpenAB::PIMItem> retrievedItem;
  ASSERT_EQ(OpenAB_Storage::Storage::eGetItemOk, s->getItem(itemId, retrievedItem));
  ASSERT_TRUE(retrievedItem.getPointer());
  ASSERT_EQ(OpenAB::eContact, retrievedItem->getType());
  ASSERT_NE("", retrievedItem->getRawData());

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testGetContactEmptyId)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());

  //first add contact to db
  OpenAB::PIMItem* newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard0));
  OpenAB::SmartPtr<OpenAB::PIMItem> newItem = newContact;
  OpenAB::PIMItem::ID itemId = "";
  OpenAB::PIMItem::Revision itemRev = "";
  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemOk, s->addItem(newItem, itemId, itemRev));
  ASSERT_NE("", itemId);
  ASSERT_NE("", itemRev);

  //now try getting contact using empty id
  OpenAB::SmartPtr<OpenAB::PIMItem> retrievedItem;
  ASSERT_EQ(OpenAB_Storage::Storage::eGetItemFail, s->getItem("", retrievedItem));
  ASSERT_FALSE(retrievedItem.getPointer());

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testGetContactNonExistingId)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());

  //first add contact to db
  OpenAB::PIMItem* newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard0));
  OpenAB::SmartPtr<OpenAB::PIMItem> newItem = newContact;
  OpenAB::PIMItem::ID itemId = "";
  OpenAB::PIMItem::Revision itemRev = "";
  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemOk, s->addItem(newItem, itemId, itemRev));
  ASSERT_NE("", itemId);
  ASSERT_NE("", itemRev);

  //now try getting contact with nonexisting id
  OpenAB::SmartPtr<OpenAB::PIMItem> retrievedItem;
  ASSERT_EQ(OpenAB_Storage::Storage::eGetItemFail, s->getItem("someNonExistingContactId", retrievedItem));
  ASSERT_FALSE(retrievedItem.getPointer());

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testAddContacts)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());

  //prepare contacts to be inserted
  std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > newItems;
  OpenAB::PIMItem* newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard0));
  newItems.push_back(newContact);

  newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard1));
  newItems.push_back(newContact);

  OpenAB::PIMItem::IDs itemIds;
  OpenAB::PIMItem::Revisions itemRevs;

  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemOk, s->addItems(newItems, itemIds, itemRevs));
  ASSERT_EQ(2, itemIds.size());
  ASSERT_EQ(2, itemRevs.size());


  //now we should be able to retrieve contact using iterator
  OpenAB_Storage::StorageItemIterator* iter = s->newStorageItemIterator();
  ASSERT_TRUE(iter->next());
  ASSERT_EQ(itemIds[0], (*iter)->id);
  ASSERT_EQ(OpenAB::eContact, (*iter)->item->getType());
  ASSERT_NE("", (*iter)->item->getRawData());

  ASSERT_TRUE(iter->next());
  ASSERT_EQ(itemIds[1], (*iter)->id);
  ASSERT_EQ(OpenAB::eContact, (*iter)->item->getType());
  ASSERT_NE("", (*iter)->item->getRawData());

  //ensure that there are only two contacts in db
  ASSERT_FALSE(iter->next());
  delete iter;

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testRemoveContact)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());

  // add contact to db
  OpenAB::PIMItem* newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard0));
  OpenAB::SmartPtr<OpenAB::PIMItem> newItem = newContact;
  OpenAB::PIMItem::ID itemId = "";
  OpenAB::PIMItem::Revision itemRev = "";
  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemOk, s->addItem(newItem, itemId, itemRev));
  ASSERT_NE("", itemId);

  //ensure that there is only one contact in db
  OpenAB_Storage::StorageItemIterator* iter = s->newStorageItemIterator();
  ASSERT_TRUE(iter->next());
  ASSERT_EQ(itemId, (*iter)->id);
  ASSERT_EQ(OpenAB::eContact, (*iter)->item->getType());
  ASSERT_NE("", (*iter)->item->getRawData());
  ASSERT_FALSE(iter->next());
  delete iter;

  ASSERT_EQ(OpenAB_Storage::Storage::eRemoveItemOk, s->removeItem(itemId));
  //ensure that there are no contacts in db
  iter = s->newStorageItemIterator();
  ASSERT_FALSE(iter->next());
  delete iter;
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testRemoveContactEmptyId)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());

  ASSERT_EQ(OpenAB_Storage::Storage::eRemoveItemFail, s->removeItem(""));
 
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testRemoveContactNonExistingId)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());

  ASSERT_EQ(OpenAB_Storage::Storage::eRemoveItemFail, s->removeItem("NonExistingId"));
 
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testRemoveContacts)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());

  // add two contacts to db
  OpenAB::PIMItem* newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard0));
  OpenAB::SmartPtr<OpenAB::PIMItem> newItem = newContact;
  OpenAB::PIMItem::ID item0Id = "";
  OpenAB::PIMItem::Revision item0Rev = "";
  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemOk, s->addItem(newItem, item0Id, item0Rev));
  ASSERT_NE("", item0Id);
  ASSERT_NE("", item0Rev);

  newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard1));
  newItem = newContact;
  OpenAB::PIMItem::ID item1Id = "";
  OpenAB::PIMItem::Revision item1Rev = "";
  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemOk, s->addItem(newItem, item1Id, item1Rev));
  ASSERT_NE("", item1Id);
  ASSERT_NE("", item1Rev);

  //ensure that there are two contacts in db
  OpenAB_Storage::StorageItemIterator* iter = s->newStorageItemIterator();
  ASSERT_TRUE(iter->next());
  ASSERT_EQ(item0Id, (*iter)->id);
  ASSERT_TRUE(iter->next());
  ASSERT_EQ(item1Id, (*iter)->id);
  ASSERT_FALSE(iter->next());
  delete iter;

  std::vector<std::string> idsToRemove;
  idsToRemove.push_back(item0Id);
  idsToRemove.push_back(item1Id);

  ASSERT_EQ(OpenAB_Storage::Storage::eRemoveItemOk, s->removeItems(idsToRemove));
  //ensure that there are no contacts in db
  iter = s->newStorageItemIterator();
  ASSERT_FALSE(iter->next());
  delete iter;
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testRemoveContactsSomeNonExisting)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());

  // add two contacts to db
  OpenAB::PIMItem* newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard0));
  OpenAB::SmartPtr<OpenAB::PIMItem> newItem = newContact;
  OpenAB::PIMItem::ID item0Id = "";
  OpenAB::PIMItem::Revision item0Rev = "";
  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemOk, s->addItem(newItem, item0Id, item0Rev));
  ASSERT_NE("", item0Id);
  ASSERT_NE("", item0Rev);

  newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard1));
  newItem = newContact;
  OpenAB::PIMItem::ID item1Id = "";
  OpenAB::PIMItem::Revision item1Rev = "";
  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemOk, s->addItem(newItem, item1Id, item1Rev));
  ASSERT_NE("", item1Id);
  ASSERT_NE("", item1Rev);

  //ensure that there are two contacts in db
  OpenAB_Storage::StorageItemIterator* iter = s->newStorageItemIterator();
  ASSERT_TRUE(iter->next());
  ASSERT_EQ(item0Id, (*iter)->id);
  ASSERT_TRUE(iter->next());
  ASSERT_EQ(item1Id, (*iter)->id);
  ASSERT_FALSE(iter->next());
  delete iter;

  std::vector<std::string> idsToRemove;
  idsToRemove.push_back(item0Id);
  idsToRemove.push_back(item1Id);
  //try to remove nonexisting contact
  idsToRemove.push_back("SomeNonExistingId");

  ASSERT_EQ(OpenAB_Storage::Storage::eRemoveItemFail, s->removeItems(idsToRemove));
  //ensure that no contacts were removed, because of error
  iter = s->newStorageItemIterator();
  ASSERT_TRUE(iter->next());
  ASSERT_EQ(item0Id, (*iter)->id);
  ASSERT_TRUE(iter->next());
  ASSERT_EQ(item1Id, (*iter)->id);
  ASSERT_FALSE(iter->next());
  delete iter;
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testModifyContact)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());

  // add contact to db
  OpenAB::PIMItem* newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard0));
  OpenAB::SmartPtr<OpenAB::PIMItem> newItem = newContact;
  OpenAB::PIMItem::ID itemId = "";
  OpenAB::PIMItem::Revision itemRev;
  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemOk, s->addItem(newItem, itemId, itemRev));
  ASSERT_NE("", itemId);
  ASSERT_NE("", itemRev);

  //ensure that there is only one contact in db
  OpenAB_Storage::StorageItemIterator* iter = s->newStorageItemIterator();
  ASSERT_TRUE(iter->next());
  ASSERT_EQ(itemId, (*iter)->id);
  ASSERT_EQ(OpenAB::eContact, (*iter)->item->getType());
  ASSERT_NE(std::string::npos, (*iter)->item->getRawData().find("Surname1"));
  ASSERT_FALSE(iter->next());
  delete iter;

  //modify contact
  newItem = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newItem->parse(vcard1));
  OpenAB::PIMItem::Revision oldRev = itemRev;
  ASSERT_EQ(OpenAB_Storage::Storage::eModifyItemOk, s->modifyItem(newItem, itemId, itemRev));

  //make sure that revision has been changed
  ASSERT_NE(oldRev, itemRev);
  
  //ensure that contact was modified
  iter = s->newStorageItemIterator();
  ASSERT_TRUE(iter->next());
  ASSERT_EQ(itemId, (*iter)->id);
  ASSERT_EQ(OpenAB::eContact, (*iter)->item->getType());
  ASSERT_NE(std::string::npos, (*iter)->item->getRawData().find("Surname2"));
  ASSERT_FALSE(iter->next());
  delete iter;
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testModifyContactEmptyId)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());

  OpenAB::SmartPtr<OpenAB::PIMItem> newItem = new OpenAB::PIMContactItem();
  newItem->parse(vcard0);
  OpenAB::PIMItem::Revision revision;
  ASSERT_EQ(OpenAB_Storage::Storage::eModifyItemFail, s->modifyItem(newItem, "", revision));

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testModifyContactNonExistingId)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());

  OpenAB::SmartPtr<OpenAB::PIMItem> newItem = new OpenAB::PIMContactItem();
  newItem->parse(vcard0);
  OpenAB::PIMItem::Revision revision;
  ASSERT_EQ(OpenAB_Storage::Storage::eModifyItemFail, s->modifyItem(newItem, "NonExistingId", revision));

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testModifyContacts)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());

  // add two contacts to db
  OpenAB::PIMItem* newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard0));
  OpenAB::SmartPtr<OpenAB::PIMItem> newItem = newContact;
  OpenAB::PIMItem::ID item0Id = "";
  OpenAB::PIMItem::Revision item0Rev;
  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemOk, s->addItem(newItem, item0Id, item0Rev));
  ASSERT_NE("", item0Id);

  newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard1));
  newItem = newContact;
  OpenAB::PIMItem::ID item1Id = "";
  OpenAB::PIMItem::Revision item1Rev;
  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemOk, s->addItem(newItem, item1Id, item1Rev));
  ASSERT_NE("", item1Id);

  //ensure that there are two contacts in db
  OpenAB_Storage::StorageItemIterator* iter = s->newStorageItemIterator();
  ASSERT_TRUE(iter->next());
  ASSERT_EQ(item0Id, (*iter)->id);
  ASSERT_NE(std::string::npos, (*iter)->item->getRawData().find("Surname1"));
  ASSERT_TRUE(iter->next());
  ASSERT_EQ(item1Id, (*iter)->id);
  ASSERT_NE(std::string::npos, (*iter)->item->getRawData().find("Surname2"));
  ASSERT_FALSE(iter->next());
  delete iter;

  //swap contacts
  std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > modifiedItems;
  OpenAB::PIMItem::IDs idsToModify;
  OpenAB::PIMItem::Revisions revsToModify;

  newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard1));
  modifiedItems.push_back(newContact);

  newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard0));
  modifiedItems.push_back(newContact);

  idsToModify.push_back(item0Id);
  idsToModify.push_back(item1Id);


  ASSERT_EQ(OpenAB_Storage::Storage::eModifyItemOk, s->modifyItems(modifiedItems, idsToModify, revsToModify));
 
  //ensure that contacts were modified
  OpenAB::SmartPtr<OpenAB::PIMItem> retrievedItem;
  ASSERT_EQ(OpenAB_Storage::Storage::eGetItemOk, s->getItem(item0Id, retrievedItem));
  ASSERT_TRUE(retrievedItem.getPointer());
  ASSERT_EQ(OpenAB::eContact, retrievedItem->getType());
  ASSERT_NE(std::string::npos, retrievedItem->getRawData().find("Surname2"));

  ASSERT_EQ(OpenAB_Storage::Storage::eGetItemOk, s->getItem(item1Id, retrievedItem));
  ASSERT_TRUE(retrievedItem.getPointer());
  ASSERT_EQ(OpenAB::eContact, retrievedItem->getType());
  ASSERT_NE(std::string::npos, retrievedItem->getRawData().find("Surname1"));
  
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testModifyContactsNotMatchingNumberOfItemsAndIds)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());

  // add two contacts to db
  OpenAB::PIMItem* newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard0));
  OpenAB::SmartPtr<OpenAB::PIMItem> newItem = newContact;
  OpenAB::PIMItem::ID item0Id = "";
  OpenAB::PIMItem::Revision item0Rev = "";
  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemOk, s->addItem(newItem, item0Id, item0Rev));
  ASSERT_NE("", item0Id);
  ASSERT_NE("", item0Rev);

  newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard1));
  newItem = newContact;
  OpenAB::PIMItem::ID item1Id = "";
  OpenAB::PIMItem::Revision item1Rev = "";
  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemOk, s->addItem(newItem, item1Id, item1Rev));
  ASSERT_NE("", item1Id);
  ASSERT_NE("", item1Rev);

  //ensure that there are two contacts in db
  OpenAB_Storage::StorageItemIterator* iter = s->newStorageItemIterator();
  ASSERT_TRUE(iter->next());
  ASSERT_EQ(item0Id, (*iter)->id);
  ASSERT_NE(std::string::npos, (*iter)->item->getRawData().find("Surname1"));
  ASSERT_TRUE(iter->next());
  ASSERT_EQ(item1Id, (*iter)->id);
  ASSERT_NE(std::string::npos, (*iter)->item->getRawData().find("Surname2"));
  ASSERT_FALSE(iter->next());
  delete iter;

  //swap contacts
  std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > modifiedItems;
  OpenAB::PIMItem::IDs idsToModify;
  OpenAB::PIMItem::Revisions revsToModify;

  newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard1));
  modifiedItems.push_back(newContact);

  newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard0));
  modifiedItems.push_back(newContact);

  idsToModify.push_back(item0Id);
  idsToModify.push_back(item1Id);
  //there will be only two items and three ids
  idsToModify.push_back("NonExistingId");

  ASSERT_EQ(OpenAB_Storage::Storage::eModifyItemFail, s->modifyItems(modifiedItems, idsToModify, revsToModify));
 
  //ensure that contacts were not modified
  OpenAB::SmartPtr<OpenAB::PIMItem> retrievedItem;
  ASSERT_EQ(OpenAB_Storage::Storage::eGetItemOk, s->getItem(item0Id, retrievedItem));
  ASSERT_TRUE(retrievedItem.getPointer());
  ASSERT_EQ(OpenAB::eContact, retrievedItem->getType());
  ASSERT_NE(std::string::npos, retrievedItem->getRawData().find("Surname1"));

  ASSERT_EQ(OpenAB_Storage::Storage::eGetItemOk, s->getItem(item1Id, retrievedItem));
  ASSERT_TRUE(retrievedItem.getPointer());
  ASSERT_EQ(OpenAB::eContact, retrievedItem->getType());
  ASSERT_NE(std::string::npos, retrievedItem->getRawData().find("Surname2"));
  
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testModifyContactsNotExistingId)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db", "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());

  // add two contacts to db
  OpenAB::PIMItem* newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard0));
  OpenAB::SmartPtr<OpenAB::PIMItem> newItem = newContact;
  OpenAB::PIMItem::ID item0Id = "";
  OpenAB::PIMItem::Revision item0Rev = "";
  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemOk, s->addItem(newItem, item0Id, item0Rev));
  ASSERT_NE("", item0Id);
  ASSERT_NE("", item0Rev);

  newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard1));
  newItem = newContact;
  OpenAB::PIMItem::ID item1Id = "";
  OpenAB::PIMItem::Revision item1Rev = "";
  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemOk, s->addItem(newItem, item1Id, item1Rev));
  ASSERT_NE("", item1Id);
  ASSERT_NE("", item1Rev);

  //ensure that there are two contacts in db
  OpenAB_Storage::StorageItemIterator* iter = s->newStorageItemIterator();
  ASSERT_TRUE(iter->next());
  ASSERT_EQ(item0Id, (*iter)->id);
  ASSERT_NE(std::string::npos, (*iter)->item->getRawData().find("Surname1"));
  ASSERT_TRUE(iter->next());
  ASSERT_EQ(item1Id, (*iter)->id);
  ASSERT_NE(std::string::npos, (*iter)->item->getRawData().find("Surname2"));
  ASSERT_FALSE(iter->next());
  delete iter;

  //swap contacts
  std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > modifiedItems;
  OpenAB::PIMItem::IDs idsToModify;
  OpenAB::PIMItem::Revisions revsToModify;

  newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard1));
  modifiedItems.push_back(newContact);

  newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard0));
  modifiedItems.push_back(newContact);

  newContact = new OpenAB::PIMContactItem();
  ASSERT_TRUE(newContact->parse(vcard0));
  modifiedItems.push_back(newContact);

  idsToModify.push_back(item0Id);
  idsToModify.push_back(item1Id);
  //try to modify non existing contact
  idsToModify.push_back("NonExistingId");

  ASSERT_EQ(OpenAB_Storage::Storage::eModifyItemFail, s->modifyItems(modifiedItems, idsToModify, revsToModify));
 
  //ensure that contacts were not modified
  OpenAB::SmartPtr<OpenAB::PIMItem> retrievedItem;
  ASSERT_EQ(OpenAB_Storage::Storage::eGetItemOk, s->getItem(item0Id, retrievedItem));
  ASSERT_TRUE(retrievedItem.getPointer());
  ASSERT_EQ(OpenAB::eContact, retrievedItem->getType());
  ASSERT_NE(std::string::npos, retrievedItem->getRawData().find("Surname1"));

  ASSERT_EQ(OpenAB_Storage::Storage::eGetItemOk, s->getItem(item1Id, retrievedItem));
  ASSERT_TRUE(retrievedItem.getPointer());
  ASSERT_EQ(OpenAB::eContact, retrievedItem->getType());
  ASSERT_NE(std::string::npos, retrievedItem->getRawData().find("Surname2"));
  
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(EDSStorageTest, testAddNullItem)
{
  OpenAB_Storage::Parameters p;
  p.setValue("db",  "oab");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  ASSERT_TRUE(OpenAB::PluginManager::getInstance().isPluginAvailable("EDSContacts"));
  OpenAB_Storage::Storage* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("EDSContacts", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Storage::Storage::eInitOk, s->init());

  OpenAB::PIMItem::ID id;
  OpenAB::PIMItem::Revision rev;
  OpenAB::SmartPtr<OpenAB::PIMItem> item;
  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemFail, s->addItem(item, id, rev));
  ASSERT_EQ(OpenAB_Storage::Storage::eModifyItemFail, s->modifyItem(item, id, rev));

  std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > items;
  items.push_back(NULL);
  items.push_back(NULL);

  OpenAB::PIMItem::IDs ids;
  OpenAB::PIMItem::Revisions revs;
  ids.push_back("");
  ids.push_back("");

  ASSERT_EQ(OpenAB_Storage::Storage::eAddItemFail, s->addItems(items, ids, revs));
  ASSERT_EQ(OpenAB_Storage::Storage::eModifyItemFail, s->modifyItems(items, ids, revs));

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

