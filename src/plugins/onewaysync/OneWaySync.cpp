/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file OneWaySync.cpp
 */

#include <string>
#include <algorithm>
#include <sys/time.h>
#include <unistd.h>

#include <helpers/PluginManager.hpp>
#include <plugin/source/Source.hpp>
#include <plugin/storage/Storage.hpp>

#include <PIMItem/Contact/PIMContactItemIndex.hpp>

#include <OpenAB.hpp>
#include "OneWaySync.hpp"

#define CHECK_DB_ERROR() \
    if (dbError){    \
      return OpenAB_Sync::Sync::eSyncFail; \
    }

#define CHECK_CANCEL() \
    if (cancelSync && syncInProgress){ \
      return OpenAB_Sync::Sync::eSyncCancelled; \
    }

#define CHECK_INPUT_ERROR() \
    if (inputError){ \
      return OpenAB_Sync::Sync::eSyncFail; \
    }


OneWaySync::OneWaySync(OpenAB_Sync_params & p)
    : phaseStats(),
      globalStats(),
      params(p),
      source(NULL),
      storage(NULL),
      dbError(false),
      inputError(false),
      threadCreated(false),
      syncInProgress(false),
      lastSyncResult(eSyncFail),
      cancelSync(false)
{
  pthread_mutex_init(&phaseStats.mutex, NULL);
  pthread_mutex_init(&globalStats.mutex, NULL);
  pthread_mutex_init(&syncMutex, NULL);
  
  pthread_mutex_lock(&globalStats.mutex);
  globalStats.clean();
  pthread_mutex_unlock(&globalStats.mutex);
  
  pthread_mutex_lock(&phaseStats.mutex);
  phaseStats.clean();
  pthread_mutex_unlock(&phaseStats.mutex);
  
  LOG_FUNC();
}

OneWaySync::~OneWaySync()
{
  LOG_FUNC();
  if (NULL != source)
  {
    LOG_FUNC() << "Delete Source"<<std::endl;
    OpenAB::PluginManager::getInstance().freePluginInstance(source);
    source = NULL;
  }
  if (NULL != storage)
  {
    LOG_FUNC() << "Delete Storage"<<std::endl;;
    OpenAB::PluginManager::getInstance().freePluginInstance(storage);
    storage = NULL;
  }
  indexDB.clear();
  if (threadCreated)
  {
    pthread_join(syncThread, NULL);
  }
  pthread_mutex_destroy(&globalStats.mutex);
  pthread_mutex_destroy(&phaseStats.mutex);
  pthread_mutex_destroy(&syncMutex);
}

enum OpenAB_Sync::Sync::eInit OneWaySync::init()
{
  LOG_FUNC();

  /* Check and Load OpenAB_Input plugin */
  LOG_VERBOSE() << "Starting Input Plugin: " << params.input_plugin<<std::endl;
  if(!OpenAB::PluginManager::getInstance().isPluginAvailable(params.input_plugin))
  {
    LOG_ERROR()<<params.input_plugin<<" not available"<<std::endl;
    return OpenAB_Sync::Sync::eInitFail;
  }

  source = NULL;

  /* Check and Load OpenAB_AB plugin */
  LOG_VERBOSE() << "Starting AB Plugin: " << params.ab_plugin<<std::endl;
  if(!OpenAB::PluginManager::getInstance().isPluginAvailable(params.ab_plugin))
  {
    LOG_ERROR()<<params.ab_plugin<<" not available"<<std::endl;
    return OpenAB_Sync::Sync::eInitFail;
  }
  
  source = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>(params.input_plugin, params.input_params);
  if (NULL == source)
  {
    LOG_ERROR() << "Cannot create Source object"<<std::endl;;
    return OpenAB_Sync::Sync::eInitFail;
  }

  storage = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>(params.ab_plugin, params.ab_params);
  if (NULL == storage)
  {
    LOG_ERROR() << "Cannot create Storage object"<<std::endl;;
    return OpenAB_Sync::Sync::eInitFail;
  }

  return OpenAB_Sync::Sync::eInitOk;
}

void OneWaySync::synchronize()
{
  pthread_mutex_lock(&syncMutex);
  if (syncInProgress)
  {
    pthread_mutex_unlock(&syncMutex);
    if(params.cb)
      params.cb->syncFinished(eSyncAlreadyInProgress);
    return;
  }
  syncInProgress = true;
  if (threadCreated)
  {
    pthread_join(syncThread, NULL);
  }
  pthread_create(&syncThread, NULL, threadSync, this);
  pthread_mutex_unlock(&syncMutex);
  threadCreated = true;
}

void* OneWaySync::threadSync(void *ptr)
{
  OneWaySync* sync = static_cast<OneWaySync*>(ptr);
  OpenAB_Sync::Sync::eSync res = sync->doSynchronize();
  pthread_mutex_lock(&sync->syncMutex);
  sync->syncInProgress = false;
  pthread_mutex_unlock(&sync->syncMutex);
  if (sync->params.cb)
    sync->params.cb->syncFinished(res);

  return NULL;
}

enum OpenAB_Sync::Sync::eCancel OneWaySync::cancel()
{
  eCancel res;
  pthread_mutex_lock(&syncMutex);
  if (syncInProgress)
  {
    cancelSync = true;
    source->cancel();
    res = eCancelOk;
  }
  else
  {
    res = eCancelNotInProgress;
  }
  pthread_mutex_unlock(&syncMutex);

  LOG_FUNC();
  return res;
}

enum OpenAB_Sync::Sync::eSuspend OneWaySync::suspend()
{
  LOG_FUNC();

  eSuspend res = eSuspendOk;
  pthread_mutex_lock(&syncMutex);
  if (syncInProgress)
  {
    OpenAB_Source::Source::eSuspendRet ret = source->suspend();
    if (ret != OpenAB_Source::Source::eSuspendRetOk)
    {
      res = eSuspendFail;
    }
  }
  else
  {
    res = eSuspendNotInProgress;
  }
  pthread_mutex_unlock(&syncMutex);

  return res;
}

enum OpenAB_Sync::Sync::eResume OneWaySync::resume()
{
  LOG_FUNC();

  eResume res = eResumeOk;
  pthread_mutex_lock(&syncMutex);
  if (syncInProgress)
  {
    OpenAB_Source::Source::eResumeRet ret = source->resume();
    if (ret != OpenAB_Source::Source::eResumeRetOk)
    {
      res = eResumeFail;
    }
  }
  else
  {
    res = eResumeNotSuspended;
  }
  pthread_mutex_unlock(&syncMutex);

  return res;
}

void OneWaySync::getStats(unsigned int& locallyAdded,
                          unsigned int& locallyModified,
                          unsigned int& locallyRemoved,
                          unsigned int& remotelyAdded,
                          unsigned int& remotelyModified,
                          unsigned int& remotelyRemoved)
{
  locallyAdded = globalStats.added;
  locallyModified = globalStats.modified;
  locallyRemoved = globalStats.removed;
  remotelyAdded = remotelyModified = remotelyRemoved = 0;
}

enum OpenAB_Sync::Sync::eSync OneWaySync::doSynchronize()
{
  LOG_FUNC();
  if (NULL == storage)
  {
    LOG_ERROR() << "Addressbook Plugin has not been initialized/defined"<<std::endl;
    return OpenAB_Sync::Sync::eSyncFail;
  }

  if (storage->eInitOk != storage->init())
  {
    return OpenAB_Sync::Sync::eSyncFail;
  }

  //Clean global stats
  pthread_mutex_lock(&globalStats.mutex);
  globalStats.clean();
  pthread_mutex_unlock(&globalStats.mutex);

  unsigned int phaseNum = 0;
  std::vector<OpenAB_Sync::Sync::Phase>::iterator it;
  for(it = phases.begin(); it != phases.end(); ++it)
  {
    switch (storage->getItemType())
    {
      case OpenAB::eContact:
        OpenAB::PIMContactItemIndex::enableAllChecks();
        break;
      case OpenAB::eEvent:
        //OpenAB::PIMCalendarItemIndex::enableAllChecks();
        break;
      case OpenAB::eTask:
        //OpenAB::PIMTaskItemIndex::enableAllChecks();
        break;
    }

    params.input_params.removeKey("ignore_fields");
    std::stringstream ss;
    for(unsigned int i = 0; i < (*it).ignoredFields.size(); ++i)
    {
      switch (storage->getItemType())
      {
        case OpenAB::eContact:
          OpenAB::PIMContactItemIndex::disableCheck((*it).ignoredFields[i]);
          break;
        case OpenAB::eEvent:
          //OpenAB::PIMCalendartemIndex::disableCheck((*it).ignoredFields[i]);
          break;
        case OpenAB::eTask:
          //OpenAB::PIMTasktItemIndex::disableCheck((*it).ignoredFields[i]);
          break;
      }

      ss<<(*it).ignoredFields[i]<<",";
    }
    if (!ss.str().empty())
      params.input_params.setValue("ignore_fields", ss.str());

    OpenAB_Source::Source* oldSource = source;
    source = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>(params.input_plugin, params.input_params);
    OpenAB::PluginManager::getInstance().freePluginInstance(oldSource);

    if (NULL == source)
    {
      LOG_ERROR() << "Cannot initialize input object"<<std::endl;
      return OpenAB_Sync::Sync::eSyncFail;
    }

    int numRetries = 5;
    OpenAB_Source::Source::eInit initRes;
    while((source->eInitOk != ( initRes = source->init())) && numRetries--)
    {
      usleep(100000);
    }
    if(source->eInitOk != initRes)
    {
      LOG_ERROR() << "Cannot initialize input object"<<std::endl;
      return OpenAB_Sync::Sync::eSyncFail;
    }

    /* Clean Statistics values */
    pthread_mutex_lock(&phaseStats.mutex);
    phaseStats.clean();
    pthread_mutex_unlock(&phaseStats.mutex);
    dbError = false;

    //========================================================

    if(params.cb)
      params.cb->syncPhaseStarted((*it).name);
    /* Phase 1 populate the indexDB*/
    LOG_VERBOSE() << "updateIndexDB() ..."<<std::endl;
    updateIndexDB();
    LOG_VERBOSE() << "updateIndexDB() DONE"<<std::endl;

    //========================================================

    /* Phase 2 retrieve vCards and process them
     * If this is Text phase, only fields containing text info will be processed
     */
    LOG_VERBOSE() << "processVCards() ..."<<std::endl;
    processItems(phaseNum);
    LOG_VERBOSE() << "processVCards() DONE"<<std::endl;
    CHECK_INPUT_ERROR();
    CHECK_DB_ERROR();
    CHECK_CANCEL();


    //========================================================
    /* Phase 3 remove vCards marked as NOT_FOUND */
    LOG_VERBOSE() << "cleanAddressbook() ..."<<std::endl;
    cleanStorage();
    LOG_VERBOSE() << "cleanAddressbook() DONE"<<std::endl;
    CHECK_DB_ERROR();
    CHECK_CANCEL();

    if(params.cb)
      params.cb->syncPhaseFinished((*it).name);


    phaseNum++;
    LOG_ERROR() << "Added    : " << phaseStats.added<<std::endl;
    LOG_ERROR() << "Modified : " << phaseStats.modified<<std::endl;
    LOG_ERROR() << "Removed  : " << phaseStats.removed<<std::endl;
  }
  OpenAB::PIMContactItemIndex::enableAllChecks();
  indexDB.clear();

  if(globalStats.added != 0 ||
     globalStats.modified != 0 ||
     globalStats.removed != 0)
    return OpenAB_Sync::Sync::eSyncOkWithDataChange;

  return OpenAB_Sync::Sync::eSyncOkWithoutDataChange;
}

void OneWaySync::updateIndexDB()
{
  LOG_FUNC();
  indexDB.clear();
  OpenAB_Storage::StorageItemIterator * it = storage->newStorageItemIterator();
  if (NULL == it)
  {
    dbError = true;
    return;
  }

  OpenAB_Storage::StorageItem * e;
  while (NULL != (e = it->next()))
  {
    if(cancelSync)
      return;

    OpenAB_Storage::StorageItem *aa = new OpenAB_Storage::StorageItem(*e);
    LOG_DEBUG() << "id:" << e->id << " Name:" << e->item->getIndex()->toString()<<std::endl;
    LOG_DEBUG()<<"STORAGE ITEM "<<aa<<std::endl;
    indexDB[e->item->getIndex()].push_back(aa);
    LOG_DEBUG() <<" IDB:" << (int)indexDB.size()<<std::endl;
  }
  delete it;
}

void OneWaySync::processItems(unsigned int phaseNum)
{
  LOG_FUNC();

  OpenAB::SmartPtr<OpenAB::PIMItem> item;

  unsigned int numOfProcessedContacts = phaseNum*source->getTotalCount();
  unsigned int totalNumOfContacts = source->getTotalCount()*phases.size();
  float progress = 0.0f;
  OpenAB::TimeStamp currentTime(true);
  OpenAB::TimeStamp lastSyncProgressEventTime(true);
  OpenAB::TimeStamp progressEventTime(params.sync_progress_time, 0);


  progress = float(numOfProcessedContacts)/float(totalNumOfContacts);

  if(params.cb)
    params.cb->syncProgress(phases.at(phaseNum).name, progress, numOfProcessedContacts);

  OpenAB_Source::Source::eGetItemRet ret;
  while ( (ret = source->getItem(item)) == source->eGetItemRetOk)
  {
    if(cancelSync)
      return;

    numOfProcessedContacts++;

    currentTime.setNow();

    if((currentTime - lastSyncProgressEventTime) > progressEventTime)
    {
      lastSyncProgressEventTime = currentTime;
      if(params.cb)
      {
        progress = 0.0;
        if (totalNumOfContacts != 0)
        {
          progress = float(numOfProcessedContacts)/float(totalNumOfContacts);
        }
        params.cb->syncProgress(phases.at(phaseNum).name, progress, numOfProcessedContacts);
      }
    }

    LOG_DEBUG() << "Processing item: " << item->getIndex()->toString() <<" num: "<<(int)indexDB[item->getIndex()].size()<<std::endl;

    vectorElem::iterator it;
    vectorElem::iterator it_first_not_found = indexDB[item->getIndex()].end();
    bool to_be_added = true;
    for (it = indexDB[item->getIndex()].begin(); it != indexDB[item->getIndex()].end(); ++it)
    {
      bool equal = false;
      equal = item->getIndex()->compare(*(*it)->item->getIndex());

      if(equal)
      {
        LOG_DEBUG() << "Contact Match"<<std::endl;
        if ((*it)->ITEM_NOT_FOUND == (*it)->status)
        {
          (*it)->status = (*it)->ITEM_FOUND;
          to_be_added = false;
          break;
        }
      }
      else
      {
        LOG_DEBUG() << "Contact DOES NOT Match"<<std::endl;
        LOG_DEBUG() << item->getIndex()->toStringFull()<<std::endl;
        LOG_DEBUG() << (*it)->item->getIndex()->toStringFull()<<std::endl;
      }

      /* Mark the fist element not found as candidate to be eventually modified */
      if ((*it)->ITEM_NOT_FOUND == (*it)->status && it_first_not_found == indexDB[item->getIndex()].end())
      {
        it_first_not_found = it;
      }
    }
    if (to_be_added)
    {
      /* Here the contact has not been found in the DB */

      if (it_first_not_found != indexDB[item->getIndex()].end())
      {
        (*it_first_not_found)->status = (*it_first_not_found)->ITEM_MODIFIED;
        (*it_first_not_found)->item = item;
        
        pthread_mutex_lock(&globalStats.mutex);
        globalStats.modified++;
        pthread_mutex_unlock(&globalStats.mutex);
        
        pthread_mutex_lock(&phaseStats.mutex);
        phaseStats.modified++;
        pthread_mutex_unlock(&phaseStats.mutex);

        modifyItem((*it_first_not_found)->id, item);
      }
      else
      {
        OpenAB_Storage::StorageItem* ie = new OpenAB_Storage::StorageItem("", item);
        ie->status = ie->ITEM_ADDED;
        indexDB[item->getIndex()].push_back(ie);
        
        pthread_mutex_lock(&globalStats.mutex);
        globalStats.added++;
        pthread_mutex_unlock(&globalStats.mutex);
        
        pthread_mutex_lock(&phaseStats.mutex);
        phaseStats.added++;
        pthread_mutex_unlock(&phaseStats.mutex);

        addItem(item);
      }
    }
    if(dbError)
    {
      LOG_ERROR() << "Error during database operation"<<std::endl;
      return;
    }
  }
  if(ret == source->eGetItemRetError)
  {
    LOG_ERROR()<<"Input error"<<std::endl;
    inputError = true;
    return;
  }

  flushInsertions();
  flushModifications();
  LOG_DEBUG()<<"Num of vcards "<<numOfProcessedContacts<<std::endl;
}

void OneWaySync::cleanStorage()
{
  std::vector<std::string> idsToBeRemoved;
  /* remove NOT_FOUND contacts */
  for (dbIndexElem::iterator itDB = indexDB.begin(); itDB != indexDB.end(); ++itDB)
  {
    for (vectorElem::iterator it = itDB->second.begin(); it != itDB->second.end(); ++it)
    {
      if ((*it)->ITEM_NOT_FOUND == (*it)->status)
      {
        idsToBeRemoved.push_back((*it)->id);
        (*it)->status = (*it)->ITEM_REMOVED;
        
        pthread_mutex_lock(&globalStats.mutex);
        globalStats.removed++;
        pthread_mutex_unlock(&globalStats.mutex);

        pthread_mutex_lock(&phaseStats.mutex);
        phaseStats.removed++;
        pthread_mutex_unlock(&phaseStats.mutex);
      }
    }
  }
  if(!idsToBeRemoved.empty())
  {
    if (storage->eRemoveItemFail == storage->removeItems(idsToBeRemoved))
    {
      dbError = true;
    }
  }
}

void OneWaySync::addItem(const OpenAB::SmartPtr<OpenAB::PIMItem> & item)
{
  itemsToBeAdded.push_back(ItemDesc("", item));
  if (itemsToBeAdded.size() > params.batch_size)
  {
    flushInsertions();
  }
}

void OneWaySync::modifyItem(const std::string& id, const OpenAB::SmartPtr<OpenAB::PIMItem> & item)
{
  LOG_DEBUG()<<"[OneWaySync] Modify item "<<id<<std::endl;
  itemsToBeModified.push_back(ItemDesc(id, item));
  if (itemsToBeModified.size() > params.batch_size)
  {
    flushModifications();
  }
}

bool OneWaySync::flushInsertions()
{
  if(itemsToBeAdded.empty())
    return true;

  std::vector<std::string> newIds;
  std::vector<std::string> revisions;
  std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > items;
  for(unsigned int i = 0; i < itemsToBeAdded.size(); ++i)
  {
    items.push_back(itemsToBeAdded[i].item);
  }

  if (storage->eAddItemOk != storage->addItems(items, newIds, revisions))
  {
    dbError = true;
    return false;
  }

  itemsToBeAdded.clear();
  return true;
}

bool OneWaySync::flushModifications()
{
  if(itemsToBeModified.empty())
    return true;

  std::vector<std::string> ids;
  std::vector<std::string> revisions;
  std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > items;
  for(unsigned int i = 0; i < itemsToBeModified.size(); ++i)
  {
    ids.push_back(itemsToBeModified[i].id);
    items.push_back(itemsToBeModified[i].item);
  }

  if (storage->eModifyItemOk != storage->modifyItems(items, ids, revisions))
  {
    dbError = true;
    return false;
  }

  itemsToBeModified.clear();
  return true;
}

namespace {

class OneWaySyncFactory : OpenAB_Sync::Factory
{
  public:
    OneWaySyncFactory()
        : OpenAB_Sync::Factory::Factory("OneWay")
    {
      LOG_FUNC();
    }
    ;
    virtual ~OneWaySyncFactory()
    {
      LOG_FUNC();
    }
    ;

    OpenAB_Sync::Sync * newIstance(const OpenAB_Sync::Parameters & params)
    {
      OpenAB_Sync_params p;
      LOG_FUNC();
      OpenAB::Variant param = params.getValue("remote_plugin");
      if (param.invalid()){
        LOG_ERROR() << "Parameter 'remote_plugin' not found"<<std::endl;
        return NULL;
      }
      p.input_plugin = param.getString();

      param = params.getValue("local_plugin");
      if (param.invalid()){
        LOG_ERROR() << "Parameter 'local_plugin' not found"<<std::endl;
        return NULL;
      }
      p.ab_plugin = param.getString();
      p.input_params = params.remoteSourcePluginParams;
      p.ab_params = params.localStoragePluginParams;

      p.cb = NULL;
      param = params.getValue("callback");
      if (!param.invalid()){
        if (param.getType() != OpenAB::Variant::POINTER)
        {
          LOG_ERROR() << "Parameter 'callback' has to be of POINTER type"<<std::endl;
          return NULL;
        }
        p.cb = (OpenAB_Sync::Sync::SyncCallback*) param.getPointer();
        LOG_INFO()<<"Callback pointer "<<p.cb<<std::endl;
      }

      p.sync_type = OpenAB_Sync_params::eSyncAll;
      p.sync_progress_time = 0.2f;
      if (!params.getValue("sync_progress_frequency").invalid())
      {
        p.sync_progress_time = params.getValue("sync_progress_frequency").getDouble();
      }

      LOG_INFO()<<"sync_progress_time="<<p.sync_progress_time<<std::endl;

      p.batch_size = 100;
      param = params.getValue("batch_size");
      if (!param.invalid()){
        if (param.getType() != OpenAB::Variant::INTEGER)
        {
          LOG_ERROR() << "Parameter 'batch_size' has to be of INTEGER type"<<std::endl;
          return NULL;
        }
        p.batch_size = param.getInt();
      }
      LOG_INFO()<<"Batch size "<<p.batch_size<<std::endl;


      OneWaySync * fi =new OneWaySync(p);
      if (NULL == fi)
      {
        LOG_ERROR() << "Cannot Initialize OneWaySync"<<std::endl;
        return NULL;
      }

      return fi;
    }
};
}

REGISTER_PLUGIN_FACTORY(OneWaySyncFactory);
