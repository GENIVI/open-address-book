/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file oab_sync_oneway_tests.cpp
 */
#include <gtest/gtest.h>
#include <string>
#include <sys/prctl.h>
#include "OpenAB.hpp"
#include "helpers/PluginManager.hpp"
#include "glib2/OpenAB_glib2_global.h"
#include "PIMItem/Contact/PIMContactItemIndex.hpp"
#include <libebook/libebook.h>
#include <libedata-book/libedata-book.h>

#define   WAIT_FOR_CONDITION(timeout, condition)    \
  do{                                               \
    uint32_t __timeout = (timeout);                 \
    while (!(condition) && --__timeout)             \
    {                                               \
      usleep(1000);                                 \
    }                                               \
  }while(0)

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

class OneWaySyncTest: public ::testing::Test, OpenAB_Sync::Sync::SyncCallback
{
public:
    OneWaySyncTest() : ::testing::Test()
    {
    }

    ~OneWaySyncTest()
    {
    }

    bool& isSyncFinished() {return syncEnded;}
    OpenAB_Sync::Sync::eSync getSyncResult() {return syncResult;}
    bool& isSyncProgressCalled() {return syncProgressCalled;}
    bool& isSyncPhaseStartedCalled() {return syncPhaseStartedCalled;}
    bool& isSyncPhaseFinishedCalled() {return syncPhaseFinishedCalled;}
    std::vector<std::string>& getFinishedSyncPhases() {return finishedSyncPhases;}
protected: 
    bool syncEnded;
    bool syncProgressCalled;
    bool syncPhaseStartedCalled;
    bool syncPhaseFinishedCalled;
    OpenAB_Sync::Sync::eSync syncResult;
    std::vector<std::string> finishedSyncPhases;

    virtual void SetUp()
    {
      syncEnded = false;
      syncResult = OpenAB_Sync::Sync::eSyncFail;
      syncProgressCalled = false;
      syncPhaseStartedCalled = false;
      syncPhaseFinishedCalled = false;
      finishedSyncPhases.clear();
      OpenAB::Logger::OutLevel() = OpenAB::Logger::Debug;

      OpenAB::PIMContactItemIndex::addCheck("fn", OpenAB::PIMItemIndex::PIMItemCheck::eKey);
      OpenAB::PIMContactItemIndex::addCheck("tel", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
      OpenAB::PIMContactItemIndex::addCheck("email", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
      OpenAB::PIMContactItemIndex::addCheck("adr", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
      OpenAB::PIMContactItemIndex::addCheck("role", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
      OpenAB::PIMContactItemIndex::addCheck("title", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
      OpenAB::PIMContactItemIndex::addCheck("nickname", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
      OpenAB::PIMContactItemIndex::addCheck("photo", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
      OpenAB::PIMContactItemIndex::addCheck("bday", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
      OpenAB::PIMContactItemIndex::addCheck("geo", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
      OpenAB::PIMContactItemIndex::addCheck("org", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
      OpenAB::PIMContactItemIndex::addCheck("note", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
      OpenAB::PIMContactItemIndex::addCheck("url", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
      OpenAB::PIMContactItemIndex::addCheck("categories", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
    }

   // Tears down the test fixture.
    virtual void TearDown()
    {
    }

    void print(const std::string& msg)
    {
      static_cast<void>(msg);
    }
    
    void syncFinished(const enum OpenAB_Sync::Sync::eSync& result)
    {
      syncResult = result;
      syncEnded = true;
    }
    
    void syncProgress(const std::string& phaseName, double progress, unsigned int numProcessedItems)
    {
      static_cast<void>(phaseName);
      static_cast<void>(progress);
      static_cast<void>(numProcessedItems);
      syncProgressCalled = true;
    }
    
    void syncPhaseStarted(const std::string& name)
    {
      static_cast<void>(name);
      syncPhaseStartedCalled = true;
    }
    
    void syncPhaseFinished(const std::string& name)
    {
      syncPhaseFinishedCalled = true;
      finishedSyncPhases.push_back(name);
    }
};

TEST_F(OneWaySyncTest, testEmptyParams)
{
  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_FALSE(s);
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(OneWaySyncTest, testInitOk)
{
  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  p.setValue("remote_plugin", "File");
  p.setValue("local_plugin", "EDSContacts");
  p.remoteSourcePluginParams.setValue("filename", "vcard_multiple.vcf");
  p.localStoragePluginParams.setValue("db","oab");
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Sync::Sync::eInitOk, s->init());
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(OneWaySyncTest, testInitNoSourcePluginName)
{
  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  p.setValue("local_plugin", "EDSContacts");
  p.remoteSourcePluginParams.setValue("filename", "vcard_multiple.vcf");
  p.localStoragePluginParams.setValue("db","oab");
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_FALSE(s);
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(OneWaySyncTest, testInitNoStoragePluginName)
{
  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  p.setValue("remote_plugin", "File");
  p.remoteSourcePluginParams.setValue("filename", "vcard_multiple.vcf");
  p.localStoragePluginParams.setValue("db","oab");
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_FALSE(s);
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(OneWaySyncTest, testInitWrongTypeOfCallback)
{
  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  p.setValue("remote_plugin", "File");
  p.setValue("local_plugin", "EDSContacts");
  p.setValue("callback", "hello");
  p.remoteSourcePluginParams.setValue("filename", "vcard_multiple.vcf");
  p.localStoragePluginParams.setValue("db","oab");
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_FALSE(s);
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(OneWaySyncTest, testInitNotAvailableSourcePlugin)
{
  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  p.setValue("remote_plugin", "NotAvailableFile");
  p.setValue("local_plugin", "EDSContacts");
  p.remoteSourcePluginParams.setValue("filename", "vcard_multiple.vcf");
  p.localStoragePluginParams.setValue("db","oab");
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Sync::Sync::eInitFail, s->init());
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(OneWaySyncTest, testInitNotAvailableStoragePlugin)
{
  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  p.setValue("remote_plugin", "File");
  p.setValue("local_plugin", "NotAvailableEDS");
  p.remoteSourcePluginParams.setValue("filename", "vcard_multiple.vcf");
  p.localStoragePluginParams.setValue("db","oab");
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Sync::Sync::eInitFail, s->init());
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(OneWaySyncTest,testInitNoParamsForStoragePlugin)
{
  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  p.setValue("remote_plugin", "File");
  p.setValue("local_plugin", "EDSContacts");
  p.remoteSourcePluginParams.setValue("filename", "vcard_multiple.vcf");
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Sync::Sync::eInitFail, s->init());
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}


void debugInstances(const char* file, int line)
{
  LOG_DEBUG()<<"Instances trace "<<file<<": "<<line<<std::endl;
  std::map<void*, std::string> instances = OpenAB::PluginManager::getInstance().getPluginInstancesInfo();
  std::map<void*, std::string>::iterator it;
  for (it = instances.begin(); it != instances.end(); ++it)
  {
    LOG_DEBUG()<<"\t"<<(*it).second<<"  "<<(*it).first<<std::endl;
  }
  LOG_DEBUG()<<"=================="<<std::endl;
}

TEST_F(OneWaySyncTest, testInitNoParamsForSourcePlugin)
{
  debugInstances(__FILE__, __LINE__);
  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  p.setValue("remote_plugin", "File");
  p.setValue("local_plugin", "EDSContacts");
  p.localStoragePluginParams.setValue("db", "oab");
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Sync::Sync::eInitFail, s->init());
  debugInstances(__FILE__, __LINE__);
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
  debugInstances(__FILE__, __LINE__);
}

TEST_F(OneWaySyncTest, testSyncEmptyDBNoPhasesDefined)
{
  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  p.setValue("remote_plugin", "File");
  p.setValue("local_plugin", "EDSContacts");
  p.remoteSourcePluginParams.setValue("filename", "vcard_multiple.vcf");
  p.localStoragePluginParams.setValue("db", "oab");
  p.setValue("callback", (OpenAB_Sync::Sync::SyncCallback*)this);
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Sync::Sync::eInitOk, s->init());
  s->synchronize();
  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());
  ASSERT_EQ(OpenAB_Sync::Sync::eSyncOkWithoutDataChange, this->getSyncResult());
  ASSERT_FALSE(this->isSyncPhaseStartedCalled());
  ASSERT_FALSE(this->isSyncPhaseFinishedCalled());

  unsigned int added,modified,removed,added2,modified2,removed2;
  s->getStats(added, modified, removed, added2,modified2,removed2);
  ASSERT_EQ(0, added);
  ASSERT_EQ(0, modified);
  ASSERT_EQ(0, removed);
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(OneWaySyncTest, testSyncOnePhaseDefined)
{ 
  removeSource("oab3");
  createSource("oab3");

  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  p.setValue("remote_plugin", "File");
  p.setValue("local_plugin", "EDSContacts");
  p.remoteSourcePluginParams.setValue("filename", "vcards_sync_initial.vcf");
  p.localStoragePluginParams.setValue("db", "oab3");
  p.setValue("callback", (OpenAB_Sync::Sync::SyncCallback*)this);
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Sync::Sync::eInitOk, s->init());

  std::vector<std::string> ignoredFields;
  ASSERT_TRUE(s->addPhase("TestPhase", ignoredFields));
  s->synchronize();
  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());
  ASSERT_EQ(OpenAB_Sync::Sync::eSyncOkWithDataChange, this->getSyncResult());

  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());

  ASSERT_TRUE(this->isSyncPhaseStartedCalled());
  ASSERT_TRUE(this->isSyncPhaseFinishedCalled());

  ASSERT_EQ(1, this->getFinishedSyncPhases().size());
  ASSERT_EQ("TestPhase", this->getFinishedSyncPhases().at(0));

  unsigned int added,modified,removed,added2,modified2,removed2;
  s->getStats(added, modified, removed, added2,modified2,removed2);
  ASSERT_EQ(10, added);
  ASSERT_EQ(0, modified);
  ASSERT_EQ(0, removed);

  this->isSyncFinished() = false;
  this->isSyncPhaseStartedCalled() = false;
  this->isSyncPhaseFinishedCalled() = false;
  this->getFinishedSyncPhases().clear();

  //try synchronizing again, no modifications should be done
  s->synchronize();
  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());

  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());
  ASSERT_EQ(OpenAB_Sync::Sync::eSyncOkWithoutDataChange, this->getSyncResult());

  ASSERT_TRUE(this->isSyncPhaseStartedCalled());
  ASSERT_TRUE(this->isSyncPhaseFinishedCalled());

  ASSERT_EQ(1, this->getFinishedSyncPhases().size());
  ASSERT_EQ("TestPhase", this->getFinishedSyncPhases().at(0));

  s->getStats(added, modified, removed, added2,modified2,removed2);
  ASSERT_EQ(0, added);
  ASSERT_EQ(0, modified);
  ASSERT_EQ(0, removed);

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(OneWaySyncTest, testSyncTwoPhaseDefined)
{ 
  removeSource("oab4");
  createSource("oab4");

  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  p.setValue("remote_plugin", "File");
  p.setValue("local_plugin", "EDSContacts");
  p.remoteSourcePluginParams.setValue("filename", "vcards_sync_initial.vcf");
  p.localStoragePluginParams.setValue("db", "oab4");
  p.setValue("callback", (OpenAB_Sync::Sync::SyncCallback*)this);
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Sync::Sync::eInitOk, s->init());

  std::vector<std::string> ignoredFields;
  ASSERT_TRUE(s->addPhase("TestPhase1", ignoredFields));
  ignoredFields.push_back("photo");
  ASSERT_TRUE(s->addPhase("TestPhase2", ignoredFields));
  s->synchronize();
  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());
  ASSERT_EQ(OpenAB_Sync::Sync::eSyncOkWithDataChange, this->getSyncResult());

  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());

  ASSERT_TRUE(this->isSyncPhaseStartedCalled());
  ASSERT_TRUE(this->isSyncPhaseFinishedCalled());

  ASSERT_EQ(2, this->getFinishedSyncPhases().size());
  ASSERT_EQ("TestPhase1", this->getFinishedSyncPhases().at(0));
  ASSERT_EQ("TestPhase2", this->getFinishedSyncPhases().at(1));

  unsigned int added,modified,removed,added2,modified2,removed2;
  s->getStats(added, modified, removed, added2,modified2,removed2);
  ASSERT_EQ(10, added);
  ASSERT_EQ(0, modified);
  ASSERT_EQ(0, removed);

  this->isSyncFinished() = false;
  this->isSyncPhaseStartedCalled() = false;
  this->isSyncPhaseFinishedCalled() = false;
  this->getFinishedSyncPhases().clear();

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}
TEST_F(OneWaySyncTest, testSyncTelModification)
{ 
  removeSource("oab2");
  createSource("oab2");

  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  p.setValue("remote_plugin", "File");
  p.setValue("local_plugin", "EDSContacts");
  p.remoteSourcePluginParams.setValue("filename", "vcards_sync_initial.vcf");
  p.localStoragePluginParams.setValue("db", "oab2");
  p.setValue("callback", (OpenAB_Sync::Sync::SyncCallback*)this);
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Sync::Sync::eInitOk, s->init());

  std::vector<std::string> ignoredFields;
  ASSERT_TRUE(s->addPhase("TestPhase", ignoredFields));
  s->synchronize();
  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());
  ASSERT_EQ(OpenAB_Sync::Sync::eSyncOkWithDataChange, this->getSyncResult());

  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());

  ASSERT_TRUE(this->isSyncPhaseStartedCalled());
  ASSERT_TRUE(this->isSyncPhaseFinishedCalled());

  ASSERT_EQ(1, this->getFinishedSyncPhases().size());
  ASSERT_EQ("TestPhase", this->getFinishedSyncPhases().at(0));

  unsigned int added,modified,removed,added2,modified2,removed2;
  s->getStats(added, modified, removed, added2,modified2,removed2);
  ASSERT_EQ(10, added);
  ASSERT_EQ(0, modified);
  ASSERT_EQ(0, removed);
  
  this->isSyncFinished() = false;
  this->isSyncPhaseStartedCalled() = false;
  this->isSyncPhaseFinishedCalled() = false;
  this->getFinishedSyncPhases().clear();
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
  p.remoteSourcePluginParams.setValue("filename", "vcards_sync_one_modified.vcf");
  s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Sync::Sync::eInitOk, s->init());

  ASSERT_TRUE(s->addPhase("TestPhase", ignoredFields));
 
  //try synchronizing again, now one contact should be modified
  s->synchronize();
  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());

  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());
  ASSERT_EQ(OpenAB_Sync::Sync::eSyncOkWithDataChange, this->getSyncResult());

  ASSERT_TRUE(this->isSyncPhaseStartedCalled());
  ASSERT_TRUE(this->isSyncPhaseFinishedCalled());

  ASSERT_EQ(1, this->getFinishedSyncPhases().size());
  ASSERT_EQ("TestPhase", this->getFinishedSyncPhases().at(0));

  s->getStats(added, modified, removed, added2,modified2,removed2);
  ASSERT_EQ(0, added);
  ASSERT_EQ(1, modified);
  ASSERT_EQ(0, removed);

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}


TEST_F(OneWaySyncTest, testSyncTelModificationDisableTelCheck)
{ 
  removeSource("oab2");
  createSource("oab2");

  OpenAB::PIMContactItemIndex::removeCheck("tel");
 
  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  p.setValue("remote_plugin", "File");
  p.setValue("local_plugin", "EDSContacts");
  p.remoteSourcePluginParams.setValue("filename", "vcards_sync_initial.vcf");
  p.localStoragePluginParams.setValue("db", "oab2");
  p.setValue("callback", (OpenAB_Sync::Sync::SyncCallback*)this);
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Sync::Sync::eInitOk, s->init());

  std::vector<std::string> ignoredFields;
  ASSERT_TRUE(s->addPhase("TestPhase", ignoredFields));
  s->synchronize();
  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());
  ASSERT_EQ(OpenAB_Sync::Sync::eSyncOkWithDataChange, this->getSyncResult());

  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());

  ASSERT_TRUE(this->isSyncPhaseStartedCalled());
  ASSERT_TRUE(this->isSyncPhaseFinishedCalled());

  ASSERT_EQ(1, this->getFinishedSyncPhases().size());
  ASSERT_EQ("TestPhase", this->getFinishedSyncPhases().at(0));

  unsigned int added,modified,removed,added2,modified2,removed2;
  s->getStats(added, modified, removed, added2,modified2,removed2);
  ASSERT_EQ(10, added);
  ASSERT_EQ(0, modified);
  ASSERT_EQ(0, removed);
  
  this->isSyncFinished() = false;
  this->isSyncPhaseStartedCalled() = false;
  this->isSyncPhaseFinishedCalled() = false;
  this->getFinishedSyncPhases().clear();
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
  p.remoteSourcePluginParams.setValue("filename", "vcards_sync_one_modified.vcf");
  s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Sync::Sync::eInitOk, s->init());

  ASSERT_TRUE(s->addPhase("TestPhase", ignoredFields));
 
  //try synchronizing again, now tel modification shouldn't be visible
  s->synchronize();
  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());

  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());
  ASSERT_EQ(OpenAB_Sync::Sync::eSyncOkWithoutDataChange, this->getSyncResult());

  ASSERT_TRUE(this->isSyncPhaseStartedCalled());
  ASSERT_TRUE(this->isSyncPhaseFinishedCalled());

  ASSERT_EQ(1, this->getFinishedSyncPhases().size());
  ASSERT_EQ("TestPhase", this->getFinishedSyncPhases().at(0));

  s->getStats(added, modified, removed, added2,modified2,removed2);
  ASSERT_EQ(0, added);
  ASSERT_EQ(0, modified);
  ASSERT_EQ(0, removed);

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}


TEST_F(OneWaySyncTest, testSyncRemovedItem)
{ 
  removeSource("oab2");
  createSource("oab2");

  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  p.setValue("remote_plugin", "File");
  p.setValue("local_plugin", "EDSContacts");
  p.remoteSourcePluginParams.setValue("filename", "vcards_sync_initial.vcf");
  p.localStoragePluginParams.setValue("db", "oab2");
  p.setValue("callback", (OpenAB_Sync::Sync::SyncCallback*)this);
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Sync::Sync::eInitOk, s->init());

  std::vector<std::string> ignoredFields;
  ASSERT_TRUE(s->addPhase("TestPhase", ignoredFields));
  s->synchronize();
  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());
  ASSERT_EQ(OpenAB_Sync::Sync::eSyncOkWithDataChange, this->getSyncResult());

  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());

  ASSERT_TRUE(this->isSyncPhaseStartedCalled());
  ASSERT_TRUE(this->isSyncPhaseFinishedCalled());

  ASSERT_EQ(1, this->getFinishedSyncPhases().size());
  ASSERT_EQ("TestPhase", this->getFinishedSyncPhases().at(0));

  unsigned int added,modified,removed,added2,modified2,removed2;
  s->getStats(added, modified, removed, added2,modified2,removed2);
  ASSERT_EQ(10, added);
  ASSERT_EQ(0, modified);
  ASSERT_EQ(0, removed);
  
  this->isSyncFinished() = false;
  this->isSyncPhaseStartedCalled() = false;
  this->isSyncPhaseFinishedCalled() = false;
  this->getFinishedSyncPhases().clear();
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
  p.remoteSourcePluginParams.setValue("filename", "vcards_sync_one_removed.vcf");
  s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Sync::Sync::eInitOk, s->init());

  ASSERT_TRUE(s->addPhase("TestPhase", ignoredFields));
 
  s->synchronize();
  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());

  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());
  ASSERT_EQ(OpenAB_Sync::Sync::eSyncOkWithDataChange, this->getSyncResult());

  ASSERT_TRUE(this->isSyncPhaseStartedCalled());
  ASSERT_TRUE(this->isSyncPhaseFinishedCalled());

  ASSERT_EQ(1, this->getFinishedSyncPhases().size());
  ASSERT_EQ("TestPhase", this->getFinishedSyncPhases().at(0));

  s->getStats(added, modified, removed, added2,modified2,removed2);
  ASSERT_EQ(0, added);
  ASSERT_EQ(0, modified);
  ASSERT_EQ(1, removed);

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(OneWaySyncTest, testSyncSuspend)
{ 
  removeSource("oab2");
  createSource("oab2");

  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  p.setValue("remote_plugin", "File");
  p.setValue("local_plugin", "EDSContacts");
  p.remoteSourcePluginParams.setValue("filename", "vcards_sync_initial.vcf");
  p.localStoragePluginParams.setValue("db", "oab2");
  p.setValue("callback", (OpenAB_Sync::Sync::SyncCallback*)this);
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Sync::Sync::eInitOk, s->init());

  std::vector<std::string> ignoredFields;
  ASSERT_TRUE(s->addPhase("TestPhase", ignoredFields));
  ASSERT_EQ(OpenAB_Sync::Sync::eSuspendNotInProgress, s->suspend());
  ASSERT_EQ(OpenAB_Sync::Sync::eResumeNotSuspended, s->resume());
  
  s->synchronize();

  ASSERT_EQ(OpenAB_Sync::Sync::eSuspendFail, s->suspend());
  ASSERT_EQ(OpenAB_Sync::Sync::eResumeFail, s->resume());
  
  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());
  ASSERT_EQ(OpenAB_Sync::Sync::eSyncOkWithDataChange, this->getSyncResult());

  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());

  ASSERT_TRUE(this->isSyncPhaseStartedCalled());
  ASSERT_TRUE(this->isSyncPhaseFinishedCalled());

  ASSERT_EQ(1, this->getFinishedSyncPhases().size());
  ASSERT_EQ("TestPhase", this->getFinishedSyncPhases().at(0));

  unsigned int added,modified,removed,added2,modified2,removed2;
  s->getStats(added, modified, removed, added2,modified2,removed2);
  ASSERT_EQ(10, added);
  ASSERT_EQ(0, modified);
  ASSERT_EQ(0, removed);
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(OneWaySyncTest, testSyncCancel)
{ 
  removeSource("oab2");
  createSource("oab2");

  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  p.setValue("remote_plugin", "File");
  p.setValue("local_plugin", "EDSContacts");
  p.remoteSourcePluginParams.setValue("filename", "vcards_sync_initial.vcf");
  p.localStoragePluginParams.setValue("db", "oab2");
  p.setValue("callback", (OpenAB_Sync::Sync::SyncCallback*)this);
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Sync::Sync::eInitOk, s->init());

  std::vector<std::string> ignoredFields;
  ASSERT_TRUE(s->addPhase("TestPhase", ignoredFields));
  ASSERT_EQ(OpenAB_Sync::Sync::eCancelNotInProgress, s->cancel());
  
  s->synchronize();

  ASSERT_EQ(OpenAB_Sync::Sync::eCancelOk, s->cancel());
  
  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());
  ASSERT_EQ(OpenAB_Sync::Sync::eSyncCancelled, this->getSyncResult());

  unsigned int added,modified,removed,added2,modified2,removed2;
  s->getStats(added, modified, removed, added2,modified2,removed2);
  ASSERT_EQ(0, added);
  ASSERT_EQ(0, modified);
  ASSERT_EQ(0, removed);
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}

TEST_F(OneWaySyncTest, testSyncPhases)
{ 
  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  p.setValue("remote_plugin", "File");
  p.setValue("local_plugin", "EDSContacts");
  p.remoteSourcePluginParams.setValue("filename", "vcards_sync_initial.vcf");
  p.localStoragePluginParams.setValue("db", "oab2");
  p.setValue("callback", (OpenAB_Sync::Sync::SyncCallback*)this);
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Sync::Sync::eInitOk, s->init());

  std::vector<std::string> ignoreFields;

  ASSERT_TRUE(s->addPhase("Phase1", ignoreFields));
  ASSERT_TRUE(s->addPhase("Phase2", ignoreFields));
  ASSERT_FALSE(s->addPhase("Phase1", ignoreFields));
  s->clearPhases();
  ASSERT_TRUE(s->addPhase("Phase1", ignoreFields));

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}
TEST_F(OneWaySyncTest, testSyncInProgress)
{ 
  removeSource("oab2");
  createSource("oab2");

  OpenAB_Sync::Parameters p;
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  p.setValue("remote_plugin", "File");
  p.setValue("local_plugin", "EDSContacts");
  p.remoteSourcePluginParams.setValue("filename", "vcards_sync_initial.vcf");
  p.localStoragePluginParams.setValue("db", "oab2");
  p.setValue("callback", (OpenAB_Sync::Sync::SyncCallback*)this);
  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", p);
  ASSERT_TRUE(s);
  ASSERT_EQ(OpenAB_Sync::Sync::eInitOk, s->init());

  std::vector<std::string> ignoredFields;
  ASSERT_TRUE(s->addPhase("TestPhase", ignoredFields));
  
  s->synchronize();
  s->synchronize();
  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());
  ASSERT_EQ(OpenAB_Sync::Sync::eSyncAlreadyInProgress, this->getSyncResult());
  this->isSyncFinished() = false;
  WAIT_FOR_CONDITION(10000, this->isSyncFinished());
  ASSERT_TRUE(this->isSyncFinished());
  ASSERT_EQ(OpenAB_Sync::Sync::eSyncOkWithDataChange, this->getSyncResult());

  unsigned int added,modified,removed,added2,modified2,removed2;
  s->getStats(added, modified, removed, added2,modified2,removed2);
  ASSERT_EQ(10, added);
  ASSERT_EQ(0, modified);
  ASSERT_EQ(0, removed);
  OpenAB::PluginManager::getInstance().freePluginInstance(s);
}
