/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file TwoWaySync.cpp
 */

#include <string>
#include <algorithm>
#include <sys/time.h>

#include <helpers/PluginManager.hpp>
#include <plugin/source/Source.hpp>
#include <plugin/storage/Storage.hpp>

#include <PIMItem/Contact/PIMContactItemIndex.hpp>

#include <OpenAB.hpp>
#include "TwoWaySync.hpp"

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


TwoWaySync::TwoWaySync(OpenAB_Sync_params & p)
    : globalStats(),
      params(p),
      localStorage(NULL),
      remoteStorage(NULL),
      dbError(false),
      inputError(false),
      threadCreated(false),
      syncInProgress(false),
      lastSyncResult(eSyncFail),
      cancelSync(false)
{
  pthread_mutex_init(&syncMutex, NULL);
  pthread_mutex_init(&globalStats.mutex, NULL);
  LOG_FUNC();
}

TwoWaySync::~TwoWaySync()
{
  LOG_FUNC();
  if (NULL != localStorage)
  {
    LOG_FUNC() << "Delete Local Storage"<<std::endl;;
    OpenAB::PluginManager::getInstance().freePluginInstance(localStorage);
    localStorage = NULL;
  }

  if (NULL != remoteStorage)
  {
    LOG_FUNC() << "Delete Remote Storage"<<std::endl;;
    OpenAB::PluginManager::getInstance().freePluginInstance(remoteStorage);
    remoteStorage = NULL;
  }

  LOG_FUNC() << "END"<<std::endl;;
  if (threadCreated)
  {
    pthread_join(syncThread, NULL);
  }
  pthread_mutex_destroy(&syncMutex);
  pthread_mutex_destroy(&globalStats.mutex);
}

enum OpenAB_Sync::Sync::eInit TwoWaySync::init()
{
  LOG_FUNC();

  /* Check and Load OpenAB_Input plugin */
  LOG_VERBOSE() << "Starting Input Plugin: " << params.local_plugin<<std::endl;
  if(!OpenAB::PluginManager::getInstance().isPluginAvailable(params.local_plugin))
  {
    LOG_ERROR()<<params.local_plugin<<" not available"<<std::endl;
    return OpenAB_Sync::Sync::eInitFail;
  }


  /* Check and Load OpenAB_AB plugin */
  LOG_VERBOSE() << "Starting Remote Plugin: " << params.remote_plugin<<std::endl;
  if(!OpenAB::PluginManager::getInstance().isPluginAvailable(params.remote_plugin))
  {
    LOG_ERROR()<<params.remote_plugin<<" not available"<<std::endl;
    return OpenAB_Sync::Sync::eInitFail;
  }

  localStorage = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>(params.local_plugin, params.local_storage_params);
  if (NULL == localStorage)
  {
    LOG_ERROR() << "Cannot create Local Storage object"<<std::endl;;
    return OpenAB_Sync::Sync::eInitFail;
  }
  
  remoteStorage = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>(params.remote_plugin, params.remote_storage_params);
  if (NULL == localStorage)
  {
    LOG_ERROR() << "Cannot create Remote Storage object"<<std::endl;;
    return OpenAB_Sync::Sync::eInitFail;
  }

  if (!params.metadata.empty())
  {
    metadata.fromJSON(params.metadata);
  }

  return OpenAB_Sync::Sync::eInitOk;
}

void TwoWaySync::synchronize()
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

void* TwoWaySync::threadSync(void *ptr)
{
  TwoWaySync* sync = static_cast<TwoWaySync*>(ptr);
  OpenAB_Sync::Sync::eSync res = sync->doSynchronize();
  pthread_mutex_lock(&sync->syncMutex);
  sync->syncInProgress = false;
  pthread_mutex_unlock(&sync->syncMutex);
  if (sync->params.cb)
    sync->params.cb->syncFinished(res);

  return NULL;
}

enum OpenAB_Sync::Sync::eCancel TwoWaySync::cancel()
{
  eCancel res;
  pthread_mutex_lock(&syncMutex);
  if (syncInProgress)
  {

    cancelSync = true;
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

enum OpenAB_Sync::Sync::eSuspend TwoWaySync::suspend()
{
  LOG_FUNC();

  eSuspend res = eSuspendOk;
  pthread_mutex_lock(&syncMutex);
  if (syncInProgress)
  {
    remoteStorage->suspend();
    localStorage->suspend();
  }
  else
  {
    res = eSuspendNotInProgress;
  }
  pthread_mutex_unlock(&syncMutex);

  return res;
}

enum OpenAB_Sync::Sync::eResume TwoWaySync::resume()
{
  LOG_FUNC();

  eResume res = eResumeOk;
  pthread_mutex_lock(&syncMutex);
  if (syncInProgress)
  {
    remoteStorage->resume();
    localStorage->resume();
  }
  else
  {
    res = eResumeNotSuspended;
  }
  pthread_mutex_unlock(&syncMutex);

  return res;
}

void TwoWaySync::getStats(unsigned int& locallyAdded,
              unsigned int& locallyModified,
              unsigned int& locallyRemoved,
              unsigned int& remotelyAdded,
              unsigned int& remotelyModified,
              unsigned int& remotelyRemoved)
{
  pthread_mutex_lock(&globalStats.mutex);
  
  locallyAdded = globalStats.locallyAdded;
  locallyModified = globalStats.locallyModified;
  locallyRemoved = globalStats.locallyRemoved;

  remotelyAdded = globalStats.remotelyAdded;
  remotelyModified = globalStats.remotelyModified;
  remotelyRemoved = globalStats.remotelyRemoved;
  pthread_mutex_unlock(&globalStats.mutex);
}

enum OpenAB_Sync::Sync::eSync TwoWaySync::doSynchronize()
{
  LOG_FUNC();
  if (NULL == localStorage)
  {
    LOG_ERROR() << "Local Storage Plugin has not been initialized/defined"<<std::endl;
    return OpenAB_Sync::Sync::eSyncFail;
  }

  if (localStorage->eInitOk != localStorage->init())
  {
    return OpenAB_Sync::Sync::eSyncFail;
  }

  if (NULL == remoteStorage)
  {
    LOG_ERROR() << "Remote Storage Plugin has not been initialized/defined"<<std::endl;
    return OpenAB_Sync::Sync::eSyncFail;
  }

  if (remoteStorage->eInitOk != remoteStorage->init())
  {
    return OpenAB_Sync::Sync::eSyncFail;
  }

  //Clean global stats
  pthread_mutex_lock(&globalStats.mutex);
  globalStats.clean();
  pthread_mutex_unlock(&globalStats.mutex);



 /* unsigned int phaseNum = 0;
  std::vector<OpenAB_Sync::Sync::Phase>::iterator it;
  for(it = phases.begin(); it != phases.end(); ++it)
  {
    switch (storage->getItemType())
    {
      case OpenAB::eContact:
        OpenAB::PIMContactItemIndex::enableAllChecks();
        break;
      case OpenAB::eCalendar:
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
        case OpenAB::eCalendar:
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
    }*/

    /* Clean Statistics values */
    dbError = false;

   /* while((remoteSource->eInitOk != ( initRes = remoteSource->init())) && numRetries--)
    {
      usleep(100000);
    }
    if(localSource->eInitOk != initRes)
    {
      LOG_ERROR() << "Cannot initialize remote input object"<<std::endl;
      return OpenAB_Sync::Sync::eSyncFail;
    }*/
    //========================================================

    if (params.metadata.empty())
    {
      firstTimeSync();
    }
    else
    {
      fullSync();
    }

    CHECK_INPUT_ERROR();
    CHECK_DB_ERROR();
    CHECK_CANCEL();

  if (globalStats.locallyAdded != 0 ||
      globalStats.locallyModified != 0 ||
      globalStats.locallyRemoved != 0 ||
      globalStats.remotelyAdded != 0 ||
      globalStats.remotelyModified != 0 ||
      globalStats.remotelyRemoved != 0)
        return OpenAB_Sync::Sync::eSyncOkWithDataChange;

      return OpenAB_Sync::Sync::eSyncOkWithoutDataChange;

}

void TwoWaySync::buildLocalIndexDB()
{
  LOG_FUNC();
  indexDB.clear();
  OpenAB_Storage::StorageItemIterator * it = localStorage->newStorageItemIterator();
  OpenAB_Storage::StorageItem * e;
  while (NULL != (e = it->next()))
  {
    if(cancelSync)
      return;

    OpenAB_Storage::StorageItem *aa = new OpenAB_Storage::StorageItem(*e);
    LOG_DEBUG()<<"Building local index "<<e->item->getIndex()->toStringFull()<<std::endl;
    indexDB[e->item->getIndex()].push_back(aa);
  }
  delete it;
}

void TwoWaySync::cleanLocalIndexDB()
{
  indexDB.clear();
}

void TwoWaySync::firstTimeSync()
{
  unsigned int numOfProcessedContacts = 0;
  unsigned int totalNumOfContacts = 0;
  float progress = 0.0f;
  OpenAB::TimeStamp currentTime(true);
  OpenAB::TimeStamp lastSyncProgressEventTime(true);
  OpenAB::TimeStamp progressEventTime(params.sync_progress_time, 0);


  progress = 0.0f;

  if(params.cb)
    params.cb->syncProgress("", progress, numOfProcessedContacts);


  buildLocalIndexDB();

  OpenAB::SmartPtr<OpenAB::PIMItem> item;

  OpenAB_Storage::StorageItemIterator * storageIt = remoteStorage->newStorageItemIterator();

  totalNumOfContacts = storageIt->getSize();
  OpenAB_Storage::StorageItem * e;
  while (NULL != (e = storageIt->next()))
  {
    item = e->item;

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
        params.cb->syncProgress("checking remote changes", progress, numOfProcessedContacts);
      }
    }
    LOG_DEBUG()<<"Processing item "<<e->item->getIndex()->toStringFull()<<std::endl;
    vectorElem::iterator it;
    vectorElem::iterator it_first_not_found = indexDB[item->getIndex()].end();
    bool to_be_added = true;
    for (it = indexDB[item->getIndex()].begin(); it != indexDB[item->getIndex()].end(); ++it)
    {
      bool equal = false;
      equal = item->getIndex()->compare(*(*it)->item->getIndex());

      if(equal)
      {
        if ((*it)->ITEM_NOT_FOUND == (*it)->status)
        {
          (*it)->status = (*it)->ITEM_FOUND;

          to_be_added = false;

          metadata.addItem(item->getId(), item->getRevision(),
                           (*it)->item->getId(), (*it)->item->getRevision());
          break;
        }
      }

      LOG_DEBUG()<<"Possible match found but not equal"<<std::endl<<item->getIndex()->toStringFull()<<std::endl<<(*it)->item->getIndex()->toStringFull()<<std::endl;
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
        // merge contacts
        LOG_DEBUG()<<"Contacts needs to be merged"<<std::endl;
        //update local and remote, add metadata record
        (*it_first_not_found)->status = (*it_first_not_found)->ITEM_MODIFIED;
        (*it_first_not_found)->item = item;
      }
      else
      {
        OpenAB_Storage::StorageItem* ie = new OpenAB_Storage::StorageItem("", item);
        ie->status = ie->ITEM_ADDED;
        indexDB[item->getIndex()].push_back(ie);
        addLocalItem(item);
      }
    }
  }

  params.cb->syncProgress("saving local changes", 0.0f, 0);
  flushLocalInsertions();
  flushLocalModifications();
  params.cb->syncProgress("saving local changes", 1.0f, 0);

  dbIndexElem::iterator it1;
  vectorElem::iterator it2;
  totalNumOfContacts = indexDB.size();
  numOfProcessedContacts = 0;

  for (it1 = indexDB.begin(); it1 != indexDB.end(); ++it1)
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
        params.cb->syncProgress("checking local changes", progress, numOfProcessedContacts);
      }
    }

    for (it2 = (*it1).second.begin(); it2 != (*it1).second.end(); ++it2)
    {
      if ((*it2)->status == (*it2)->ITEM_NOT_FOUND)
      {
        addRemoteItem((*it2)->item);
      }
    }
  }
  params.cb->syncProgress("saving remote changes", 0.0f, 0);
  flushRemoteInsertions();
  flushRemoteModifications();
  params.cb->syncProgress("saving remote changes", 1.0f, 0);

  std::string token;
  if (OpenAB_Storage::Storage::eGetSyncTokenOk == localStorage->getLatestSyncToken(token))
  {
    metadata.setLocalSyncToken(token);
  }
  else
  {
    metadata.setLocalSyncToken("");
  }

  if (OpenAB_Storage::Storage::eGetSyncTokenOk == remoteStorage->getLatestSyncToken(token))
  {
    metadata.setRemoteSyncToken(token);
  }
  else
  {
    metadata.setRemoteSyncToken("");
  }

  if(params.cb)
      params.cb->metadataUpdated(metadata.toJSON());
  cleanLocalIndexDB();
}

void TwoWaySync::fullSync()
{
  metadata.resetLocalState(OpenAB_Sync::SyncMetadata::NotPresent);
  metadata.resetRemoteState(OpenAB_Sync::SyncMetadata::NotPresent);
  localyAddedItems.clear();
  localyModifiedItems.clear();
  remotelyAddedItems.clear();
  remotelyModifiedItems.clear();

  OpenAB::PIMItem::IDs addedItemsIDs;
  OpenAB::PIMItem::IDs modifiedItemsIDs;

  std::map<std::string, std::string>::iterator revisionsIt;
  std::map<std::string, std::string> localRevisons;
  std::map<std::string, std::string> remoteRevisions;

  std::string token;
  std::vector<OpenAB::PIMItem::ID> removed;

  if (OpenAB_Storage::Storage::eGetRevisionsOk == localStorage->getChangedRevisions(metadata.getLocalSyncToken(), localRevisons, removed))
  {
    LOG_DEBUG()<<"Have "<<localRevisons.size()<<" local incremental changes"<<std::endl;
    metadata.resetLocalState(OpenAB_Sync::SyncMetadata::NotChanged);
    std::vector<OpenAB::PIMItem::ID>::iterator it;
    for (it = removed.begin(); it != removed.end(); ++it)
    {
      metadata.setLocalState((*it), OpenAB_Sync::SyncMetadata::NotPresent);
    }
  }
  else
  {
    localStorage->getRevisions(localRevisons);
  }

  removed.clear();
  if (OpenAB_Storage::Storage::eGetRevisionsOk == remoteStorage->getChangedRevisions(metadata.getRemoteSyncToken(), remoteRevisions, removed))
  {
    LOG_DEBUG()<<"Have "<<remoteRevisions.size()<<" remote incremental changes"<<std::endl;

    metadata.resetRemoteState(OpenAB_Sync::SyncMetadata::NotChanged);
    std::vector<OpenAB::PIMItem::ID>::iterator it;
    for (it = removed.begin(); it != removed.end(); ++it)
    {
      metadata.setRemoteState((*it), OpenAB_Sync::SyncMetadata::NotPresent);
    }
  }
  else
  {
    remoteStorage->getRevisions(remoteRevisions);
  }

  for (revisionsIt = localRevisons.begin(); revisionsIt != localRevisons.end(); ++revisionsIt)
  {
    LOG_DEBUG()<<"Local revisions: "<<(*revisionsIt).first<<" "<<(*revisionsIt).second<<std::endl;
    if (metadata.hasLocalId((*revisionsIt).first))
    {
      if (metadata.getLocalRevision((*revisionsIt).first) != (*revisionsIt).second)
      {
        modifiedItemsIDs.push_back((*revisionsIt).first);
        metadata.setLocalState((*revisionsIt).first, OpenAB_Sync::SyncMetadata::Modified);
      }
      else
      {
        metadata.setLocalState((*revisionsIt).first, OpenAB_Sync::SyncMetadata::NotChanged);
      }
    }
    else
    {
      addedItemsIDs.push_back((*revisionsIt).first);
    }
  }

  std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > tempItems;
  if (!addedItemsIDs.empty())
  {
    if (localStorage->eGetItemOk != localStorage->getItems(addedItemsIDs, localyAddedItems))
    {
      LOG_ERROR()<<"Cannot retrieve local items"<<std::endl;
      return;
    }
    addedItemsIDs.clear();
  }
  if (!modifiedItemsIDs.empty())
  {
    if (localStorage->eGetItemOk != localStorage->getItems(modifiedItemsIDs, tempItems))
    {
      LOG_ERROR() << "Cannot retrieve local items" << std::endl;
      inputError = true;
      return;
    }

    for (unsigned int i = 0; i < tempItems.size(); ++i)
    {
      localyModifiedItems[modifiedItemsIDs[i]] = tempItems[i];
    }
    tempItems.clear();
    modifiedItemsIDs.clear();
  }

  for (revisionsIt = remoteRevisions.begin(); revisionsIt != remoteRevisions.end(); ++revisionsIt)
  {
    if (metadata.hasRemoteId((*revisionsIt).first))
    {
      if (metadata.getRemoteRevision((*revisionsIt).first) != (*revisionsIt).second)
      {
        modifiedItemsIDs.push_back((*revisionsIt).first);
        metadata.setRemoteState((*revisionsIt).first, OpenAB_Sync::SyncMetadata::Modified);
      }
      else
      {
        metadata.setRemoteState((*revisionsIt).first, OpenAB_Sync::SyncMetadata::NotChanged);
      }
    }
    else
    {
      addedItemsIDs.push_back((*revisionsIt).first);
    }
  }

  if (!addedItemsIDs.empty())
  {
    if (OpenAB_Storage::Storage::eGetItemFail == remoteStorage->getItems(addedItemsIDs, remotelyAddedItems))
    {
      LOG_ERROR()<<"Cannot retrieve remote items"<<std::endl;
      return;
    }
    addedItemsIDs.clear();
  }
  if (!modifiedItemsIDs.empty())
  {
    if (OpenAB_Storage::Storage::eGetItemFail == remoteStorage->getItems(modifiedItemsIDs, tempItems))
    {
      LOG_ERROR()<<"Cannot retrieve remote items"<<std::endl;
      inputError = true;
      return;
    }

    for (unsigned int i = 0; i < tempItems.size(); ++i)
    {
      remotelyModifiedItems[modifiedItemsIDs[i]] = tempItems[i];
    }
    tempItems.clear();
    modifiedItemsIDs.clear();
  }

  //resolve metadata
  std::map<std::string, std::string> items;

  items = metadata.getItemsWithState(OpenAB_Sync::SyncMetadata::NotPresent, OpenAB_Sync::SyncMetadata::NotPresent);
  std::map<std::string, std::string>::const_iterator it;

  LOG_DEBUG()<<"ITEMS REMOVED IN BOTH LOCAL AND REMOTE"<<std::endl;
  for (it = items.begin(); it != items.end(); ++it)
  {
    // remove from metadata
    metadata.removeItem((*it).first, (*it).second);
    LOG_DEBUG()<<(*it).first<<"   "<<(*it).second<<std::endl;
  }

  items = metadata.getItemsWithState(OpenAB_Sync::SyncMetadata::NotPresent, OpenAB_Sync::SyncMetadata::NotChanged);
  LOG_DEBUG()<<"ITEMS REMOVED IN REMOTE"<<std::endl;
  for (it = items.begin(); it != items.end(); ++it)
  {
    //Remove from local storage and from metadata
    removeLocalItem((*it).second);
    metadata.removeItem((*it).first, (*it).second);
    LOG_DEBUG()<<(*it).first<<"   "<<(*it).second<<std::endl;
  }

  items = metadata.getItemsWithState(OpenAB_Sync::SyncMetadata::NotChanged, OpenAB_Sync::SyncMetadata::NotPresent);
  LOG_DEBUG()<<"ITEMS REMOVED IN LOCAL"<<std::endl;
  for (it = items.begin(); it != items.end(); ++it)
  {
    //Remove from remote storage and from metadata
    removeRemoteItem((*it).first);
    metadata.removeItem((*it).first, (*it).second);
    LOG_DEBUG()<<(*it).first<<"   "<<(*it).second<<std::endl;
  }

  items = metadata.getItemsWithState(OpenAB_Sync::SyncMetadata::NotPresent, OpenAB_Sync::SyncMetadata::Modified);
  LOG_DEBUG()<<"ITEMS REMOVED IN REMOTE BUT CHANGED IN LOCAL"<<std::endl;
  for (it = items.begin(); it != items.end(); ++it)
  {
    //Remove from local storage and from metadata
    metadata.removeItem((*it).first, (*it).second);
    addRemoteItem(localyModifiedItems[(*it).second]);
    LOG_DEBUG()<<(*it).first<<"   "<<(*it).second<<std::endl;
  }

  items = metadata.getItemsWithState(OpenAB_Sync::SyncMetadata::Modified, OpenAB_Sync::SyncMetadata::NotPresent);
  LOG_DEBUG()<<"ITEMS REMOVED IN LOCAL BUT CHANGED IN REMOTE"<<std::endl;
  for (it = items.begin(); it != items.end(); ++it)
  {
    //Remove from remote storage and from metadata
    metadata.removeItem((*it).first, (*it).second);
    addLocalItem(remotelyModifiedItems[(*it).first]);
    LOG_DEBUG()<<(*it).first<<"   "<<(*it).second<<std::endl;
  }

  items = metadata.getItemsWithState(OpenAB_Sync::SyncMetadata::NotChanged, OpenAB_Sync::SyncMetadata::Modified);
  LOG_DEBUG()<<"ITEMS MODIFIED IN LOCAL"<<std::endl;

  for (it = items.begin(); it != items.end(); ++it)
  {
    //update remote
    metadata.updateLocalRevision((*it).second, localyModifiedItems[(*it).second]->getRevision());
    modifyRemoteItem((*it).first, localyModifiedItems[(*it).second]);
    LOG_DEBUG() << (*it).first << "   " << (*it).second << std::endl;
  }

  items = metadata.getItemsWithState(OpenAB_Sync::SyncMetadata::Modified, OpenAB_Sync::SyncMetadata::NotChanged);
  LOG_DEBUG()<<"ITEMS MODIFIED IN REMOTE"<<std::endl;

  for (it = items.begin(); it != items.end(); ++it)
  {
    //update local
    metadata.updateRemoteRevision((*it).first, remotelyModifiedItems[(*it).first]->getRevision());
    modifyLocalItem((*it).second, remotelyModifiedItems[(*it).first]);
    LOG_DEBUG() << (*it).first << "   " << (*it).second << std::endl;
  }

  items = metadata.getItemsWithState(OpenAB_Sync::SyncMetadata::Modified, OpenAB_Sync::SyncMetadata::Modified);
  LOG_DEBUG()<<"ITEMS MODIFIED IN REMOTE AND LOCAL"<<std::endl;

  for (it = items.begin(); it != items.end(); ++it)
  {
    //find a way for creating new UID when needed
    //create duplicate keeping two conflicting versions and let user to merge items
    std::string newLocalId;
    std::string newLocalRevision;

    std::string newRemoteId;
    std::string newRemoteRevision;
    std::string oldID = localyModifiedItems[(*it).second]->getId();
    oldID += "(conflicted)";
    localyModifiedItems[(*it).second]->setId(oldID, true);

    localStorage->addItem(remotelyModifiedItems[(*it).first], newLocalId, newLocalRevision);
    remoteStorage->addItem(localyModifiedItems[(*it).second], newRemoteId, newRemoteRevision);

    metadata.removeItem((*it).first, (*it).second);
    metadata.addItem((*it).first, remotelyModifiedItems[(*it).first]->getRevision(),
                     newLocalId, newLocalRevision);

    metadata.addItem(newRemoteId, newRemoteRevision,
                     (*it).second, localyModifiedItems[(*it).second]->getRevision());

    pthread_mutex_lock(&globalStats.mutex);
    globalStats.locallyAdded++;
    globalStats.remotelyAdded++;
    pthread_mutex_unlock(&globalStats.mutex);

    LOG_DEBUG() << (*it).first << "   " << (*it).second << std::endl;
  }

  items = metadata.getItemsWithState(OpenAB_Sync::SyncMetadata::NotChanged, OpenAB_Sync::SyncMetadata::NotChanged);

  std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> >::iterator it1;
  std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> >::iterator it2;

  for (it1 = localyAddedItems.begin(); it1 != localyAddedItems.end();)
  {
    LOG_DEBUG()<<"Have locally added item "<<(*it1)->getId()<<std::endl;
    bool matchFound = false;
    std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> >::iterator toErase = remotelyAddedItems.end();
    for (it2 = remotelyAddedItems.begin(); it2 != remotelyAddedItems.end(); ++it2)
    {
      if ((*it1)->getIndex() == (*it2)->getIndex())
      {
        if ((*it1)->getIndex()->compare(*(*it2)->getIndex()))
        {
          //both are exactly the same
          metadata.addItem((*it2)->getId(), (*it2)->getRevision(),
                           (*it1)->getId(), (*it1)->getRevision());
          matchFound = true;
          toErase = it2;
          break;
        }
        //otherwise create duplicates and let user to merge items manually
      }
    }
    if (matchFound)
    {
      it1 = localyAddedItems.erase(it1);
      remotelyAddedItems.erase(toErase);
    }
    else
    {
      addRemoteItem((*it1));
      it1++;
    }
  }

  for (it2 = remotelyAddedItems.begin(); it2 != remotelyAddedItems.end(); ++it2)
  {
    LOG_DEBUG()<<"Have remotely added item "<<(*it2)->getId()<<std::endl;
    addLocalItem((*it2));
  }

  flushLocalInsertions();
  flushLocalModifications();
  flushLocalRemovals();
  flushRemoteInsertions();
  flushRemoteModifications();
  flushRemoteRemovals();

  if (OpenAB_Storage::Storage::eGetSyncTokenOk == localStorage->getLatestSyncToken(token))
  {
    metadata.setLocalSyncToken(token);
  }
  else
  {
    metadata.setLocalSyncToken("");
  }

  if (OpenAB_Storage::Storage::eGetSyncTokenOk == remoteStorage->getLatestSyncToken(token))
  {
    metadata.setRemoteSyncToken(token);
  }
  else
  {
    metadata.setRemoteSyncToken("");
  }

  if(params.cb)
    params.cb->metadataUpdated(metadata.toJSON());
}

void TwoWaySync::addLocalItem(const OpenAB::SmartPtr<OpenAB::PIMItem> & item)
{
  pthread_mutex_lock(&globalStats.mutex);
  globalStats.locallyAdded++;
  pthread_mutex_unlock(&globalStats.mutex);

  localItemsToBeAdded.push_back(ItemDesc("", item));
  if (localItemsToBeAdded.size() > params.batch_size)
  {
    flushLocalInsertions();
  }
}

void TwoWaySync::modifyLocalItem(const std::string& id, const OpenAB::SmartPtr<OpenAB::PIMItem> & item)
{
  pthread_mutex_lock(&globalStats.mutex);
  globalStats.locallyModified++;
  pthread_mutex_unlock(&globalStats.mutex);

  localItemsToBeModified.push_back(ItemDesc(id, item));
  if (localItemsToBeModified.size() > params.batch_size)
  {
    flushLocalModifications();
  }
}

void TwoWaySync::removeLocalItem(const std::string& id)
{
  pthread_mutex_lock(&globalStats.mutex);
  globalStats.locallyRemoved++;
  pthread_mutex_unlock(&globalStats.mutex);

  localItemsToBeRemoved.push_back(id);
  if (localItemsToBeRemoved.size() > params.batch_size)
  {
    flushLocalRemovals();
  }
}

bool TwoWaySync::flushLocalInsertions()
{
  if(localItemsToBeAdded.empty())
    return true;

  OpenAB::PIMItem::IDs newIds;
  OpenAB::PIMItem::Revisions newRevisions;
  std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > items;
  for(unsigned int i = 0; i < localItemsToBeAdded.size(); ++i)
  {
    items.push_back(localItemsToBeAdded[i].item);
  }

  if (localStorage->eAddItemOk != localStorage->addItems(items, newIds, newRevisions))
  {
    dbError = true;
    return false;
  }

  for(unsigned int i = 0; i < localItemsToBeAdded.size(); ++i)
  {
    metadata.addItem(localItemsToBeAdded[i].item->getId(), localItemsToBeAdded[i].item->getRevision(),
                     newIds[i], newRevisions[i]);
  }

  localItemsToBeAdded.clear();
  return true;
}

bool TwoWaySync::flushLocalModifications()
{
  if(localItemsToBeModified.empty())
    return true;

  OpenAB::PIMItem::IDs ids;
  OpenAB::PIMItem::Revisions newRevisions;
  std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > items;
  for(unsigned int i = 0; i < localItemsToBeModified.size(); ++i)
  {
    ids.push_back(localItemsToBeModified[i].id);
    items.push_back(localItemsToBeModified[i].item);
  }

  if (localStorage->eModifyItemOk != localStorage->modifyItems(items, ids, newRevisions))
  {
    dbError = true;
    return false;
  }

  for(unsigned int i = 0; i < localItemsToBeModified.size(); ++i)
  {
    metadata.updateLocalRevision(ids[i], newRevisions[i]);
  }

  localItemsToBeModified.clear();
  return true;
}

bool TwoWaySync::flushLocalRemovals()
{
  if(localItemsToBeRemoved.empty())
    return true;

  if (localStorage->eRemoveItemOk != localStorage->removeItems(localItemsToBeRemoved))
  {
    dbError = true;
    return false;
  }

  localItemsToBeRemoved.clear();
  return true;
}

void TwoWaySync::addRemoteItem(const OpenAB::SmartPtr<OpenAB::PIMItem> & item)
{
  pthread_mutex_lock(&globalStats.mutex);
  globalStats.remotelyAdded++;
  pthread_mutex_unlock(&globalStats.mutex);

  remoteItemsToBeAdded.push_back(ItemDesc("", item));
  if (remoteItemsToBeAdded.size() > params.batch_size)
  {
    flushRemoteInsertions();
  }
}

void TwoWaySync::modifyRemoteItem(const std::string& id, const OpenAB::SmartPtr<OpenAB::PIMItem> & item)
{
  pthread_mutex_lock(&globalStats.mutex);
  globalStats.remotelyModified++;
  pthread_mutex_unlock(&globalStats.mutex);

  remoteItemsToBeModified.push_back(ItemDesc(id, item));
  if (remoteItemsToBeModified.size() > params.batch_size)
  {
    flushRemoteModifications();
  }
}

void TwoWaySync::removeRemoteItem(const std::string& id)
{
  pthread_mutex_lock(&globalStats.mutex);
  globalStats.remotelyRemoved++;
  pthread_mutex_unlock(&globalStats.mutex);

  remoteItemsToBeRemoved.push_back(id);
  if (remoteItemsToBeRemoved.size() > params.batch_size)
  {
    flushRemoteRemovals();
  }
}

bool TwoWaySync::flushRemoteInsertions()
{
  if(remoteItemsToBeAdded.empty())
    return true;

  OpenAB::PIMItem::IDs newIds;
  OpenAB::PIMItem::Revisions newRevisions;
  std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > items;
  for(unsigned int i = 0; i < remoteItemsToBeAdded.size(); ++i)
  {
    items.push_back(remoteItemsToBeAdded[i].item);
  }

  if (localStorage->eAddItemOk != remoteStorage->addItems(items, newIds, newRevisions))
  {
    dbError = true;
    return false;
  }

  for(unsigned int i = 0; i < remoteItemsToBeAdded.size(); ++i)
  {
    metadata.addItem(newIds[i], newRevisions[i],
                     remoteItemsToBeAdded[i].item->getId(), remoteItemsToBeAdded[i].item->getRevision());
  }

  remoteItemsToBeAdded.clear();
  return true;
}

bool TwoWaySync::flushRemoteModifications()
{
  if(remoteItemsToBeModified.empty())
    return true;

  OpenAB::PIMItem::IDs ids;
  OpenAB::PIMItem::Revisions newRevisions;
  std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > items;
  for(unsigned int i = 0; i < remoteItemsToBeModified.size(); ++i)
  {
    ids.push_back(remoteItemsToBeModified[i].id);
    items.push_back(remoteItemsToBeModified[i].item);
  }

  if (remoteStorage->eModifyItemOk != remoteStorage->modifyItems(items, ids, newRevisions))
  {
    dbError = true;
    return false;
  }

  for(unsigned int i = 0; i < remoteItemsToBeModified.size(); ++i)
  {
    metadata.updateRemoteRevision(ids[i], newRevisions[i]);
  }

  remoteItemsToBeModified.clear();
  return true;
}

bool TwoWaySync::flushRemoteRemovals()
{
  if(remoteItemsToBeRemoved.empty())
    return true;

  if (remoteStorage->eRemoveItemOk != remoteStorage->removeItems(remoteItemsToBeRemoved))
  {
    dbError = true;
    return false;
  }

  remoteItemsToBeRemoved.clear();
  return true;
}

namespace {

class TwoWaySyncFactory : OpenAB_Sync::Factory
{
  public:
  TwoWaySyncFactory()
        : OpenAB_Sync::Factory::Factory("TwoWay")
    {
      LOG_FUNC();
    }
    ;
    virtual ~TwoWaySyncFactory()
    {
      LOG_FUNC();
    }
    ;

    OpenAB_Sync::Sync * newIstance(const OpenAB_Sync::Parameters & params)
    {
      OpenAB_Sync_params p;
      LOG_FUNC();
      OpenAB::Variant param = params.getValue("local_plugin");
      if (param.invalid()){
        LOG_ERROR() << "Parameter 'local_plugin' not found"<<std::endl;
        return NULL;
      }
      p.local_plugin = param.getString();

      param = params.getValue("remote_plugin");
      if (param.invalid()){
        LOG_ERROR() << "Parameter 'remote_plugin' not found"<<std::endl;
        return NULL;
      }
      p.remote_plugin = param.getString();
      p.local_input_params = params.localSourcePluginParams;
      p.local_storage_params = params.localStoragePluginParams;

      p.remote_input_params = params.remoteSourcePluginParams;
      p.remote_storage_params = params.remoteStoragePluginParams;

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

      p.metadata = "";
      param = params.getValue("metadata");
      if (!param.invalid()){
        if (param.getType() != OpenAB::Variant::STRING)
        {
          LOG_ERROR() << "Parameter 'metadata' has to be of STRING type"<<std::endl;
          return NULL;
        }
        p.metadata = param.getString();
      }

      TwoWaySync * fi =new TwoWaySync(p);
      if (NULL == fi)
      {
        LOG_ERROR() << "Cannot Initialize TwoWaySync"<<std::endl;
        return NULL;
      }

      return fi;
    }
};
}

REGISTER_PLUGIN_FACTORY(TwoWaySyncFactory);
