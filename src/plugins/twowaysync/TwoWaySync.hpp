/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file TwoWaySync.hpp
 */

#ifndef TWO_WAY_SYNC_HPP
#define TWO_WAY_SYNC_HPP

#include <pthread.h>
#include "plugin/sync/Sync.hpp"

/**
 * @defgroup  TwoWaySync TwoWay Sync Plugin
 * @ingroup SyncPlugin
 *
 * @brief The TwoWay Sync plugin is responsible to perform two-way synchronization of PIM items.
 *
 * Two-way synchronization can be done only between two instances of OpenAB::Storage (identified as local and remote).
 * Two-way synchronization uses metadata information to store information about association between items in OpenAB::Storage instances.
 *
 * During synchronization process different cases can occur:
 *  - @b Case 1: There is no metadata information - in that case do full synchronization and produce initial metadata.
 *    During this case common items on both OpenAB::Storage will be found and items not contained in one of OpenAB::Storage instances will be sent to other. During that case no items will be removed from local or remote OpenAB::Storage.
 *  - @b Case 2: There is already metadata information - in that case ask OpenAB::Sources for lists of current item revisions and compare them with information from metadata.
 *    For each item there are 9 possible states in which it may be:
 *    * Item was removed in both local and remote OpenAB::Storage - in that case remove information about item from metadata.
 *    * Item was removed in local/remote but was not modified in remote/local - in that case remove item from remote/local and remove information about item from metadata.
 *    * Item was removed in local/remote but was modified in remote/local - in that case restore item on local/remote
 *    * Item was not changed in local/remote but was changed in remote/local - in that case propagate modifications to local/remote.
 *    * Item was changed in local/remote and was changed in remote/local - in that case create duplicate item on remote/local so user can decide how to merget two items.
 *    * Item was not modified either in local or remote - in that case no action is needed.
 * After synchronization updated metadata is sent using callback method OpenAB_Sync::Sync::SyncCallback::metadataUpdated
 *
 ** ## Parameters ##
 * Parameters:
 * | Type     | Name | Description                 | Mandatory |
 * |:---------|:     |:----------------------------|:          |
 * |String    |"remote_plugin" | name of OpenAB_Source::Source plugin to use | Yes |
 * |Struct    |remoteSourcePluginParams | parameters of OpenAB_Source::Source plugin to use | Yes |
 * |Struct    |remoteStoragePluginParams | parameters of OpenAB_Storage::Storage plugin to use | Yes |
 * |String    |"local_plugin" | name of OpenAB_Storage::Storage plugin to use | Yes |
 * |Struct    |localStoragePluginParams | parameters of OpenAB_Storage::Storage plugin | Yes |
 * |Struct    |localSorcePluginParams | parameters of OpenAB_Source::Source plugin | Yes |
 * |Pointer   |"callback"      | pointer to OpenAB_Sync::Sync::SyncCallback   | No       |
 * |Float     |"sync_progress_frequency" | interval of OpenAB_Sync::Sync::SyncCallback::syncProgress() emission in seconds | No |
 * |Integer   | "batch_size" | size of batches to be used on Storage operations | No |
 * |Stringr   | "metadata" | last synchronization meta data information in JSON format | No |
 *
 * @todo Add support for pausing/resumig/canceling operation
 * @todo Add possibility to sleep after processing each item to lower CPU consumption during sync
 */

/**
 * @brief Set of parameters required to perform the synchronization
 */
struct OpenAB_Sync_params
{
    std::string                     local_plugin; /**< @brief The name of the Input plugin */
    OpenAB_Source::Parameters          local_input_params; /**< @brief The parameters required by the Input plugin used*/
    OpenAB_Storage::Parameters         local_storage_params; /**< @brief The parameters required by the Input plugin used*/

    std::string                     remote_plugin;    /**< @brief The name of the AB (Address Book) plugin */
    OpenAB_Source::Parameters          remote_input_params; /**< @brief The parameters required by the Input plugin used*/
    OpenAB_Storage::Parameters         remote_storage_params; /**< @brief The parameters required by the Input plugin used*/

    OpenAB_Sync::Sync::SyncCallback *  cb;           /**< @brief The Callback object */
    enum SyncType
    {
      eSyncIncremental,
      eSyncAll,
      eSyncText
    }                               sync_type;
    float                           sync_progress_time;
    unsigned int                    batch_size;
    std::string                     metadata;
};

/**
 * @brief Sync Class providing all the Sync controls
 */
class TwoWaySync : public OpenAB_Sync::Sync
{
  public:
    TwoWaySync(OpenAB_Sync_params&);
    ~TwoWaySync();

    enum OpenAB_Sync::Sync::eInit init();

    void synchronize();

    enum OpenAB_Sync::Sync::eCancel cancel();

    enum OpenAB_Sync::Sync::eSuspend suspend();

    enum OpenAB_Sync::Sync::eResume resume();

    void getStats(unsigned int& locallyAdded,
                  unsigned int& locallyModified,
                  unsigned int& locallyRemoved,
                  unsigned int& remotelyAdded,
                  unsigned int& remotelyModified,
                  unsigned int& remotelyRemoved);

  private:
    /**
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    TwoWaySync(TwoWaySync const &other);

    /**
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    TwoWaySync& operator=(TwoWaySync const &other);

    void buildLocalIndexDB();
    void cleanLocalIndexDB();

    void initMetadata();
    void saveMetadata();


    bool checkRemoteChanges();
    bool checkLocalChanges();

    void firstTimeSync();
    void fullSync();

    void addLocalItem(const OpenAB::SmartPtr<OpenAB::PIMItem>& item);
    void modifyLocalItem(const std::string& id, const OpenAB::SmartPtr<OpenAB::PIMItem>& item);
    void removeLocalItem(const std::string& id);

    void addRemoteItem(const OpenAB::SmartPtr<OpenAB::PIMItem>& item);
    void modifyRemoteItem(const std::string& id, const OpenAB::SmartPtr<OpenAB::PIMItem>& item);
    void removeRemoteItem(const std::string& id);

    bool flushLocalInsertions();
    bool flushLocalModifications();
    bool flushLocalRemovals();

    bool flushRemoteInsertions();
    bool flushRemoteModifications();
    bool flushRemoteRemovals();

    enum eSync doSynchronize();
    static void* threadSync(void*);

    struct SyncStats {
        void clean(){
          locallyAdded    = 0;
          locallyModified = 0;
          locallyRemoved  = 0;

          remotelyAdded   = 0;
          remotelyModified = 0;
          remotelyRemoved  = 0;
        };
        unsigned int locallyAdded;
        unsigned int locallyModified;
        unsigned int locallyRemoved;

        unsigned int remotelyAdded;
        unsigned int remotelyModified;
        unsigned int remotelyRemoved;

        pthread_mutex_t mutex;

    };

    SyncStats globalStats;

    OpenAB_Sync_params       params;

    OpenAB_Storage::Storage* localStorage;

    OpenAB_Storage::Storage* remoteStorage;

    typedef std::vector< OpenAB::SmartPtr<OpenAB_Storage::StorageItem> > vectorElem;
    typedef std::map< OpenAB::SmartPtr<OpenAB::PIMItemIndex> , vectorElem > dbIndexElem;
    dbIndexElem indexDB;

    //typedef std::map<std::string, OpenAB::SmartPtr<OpenAB_Storage::StorageItem> LocalIndex;
    //LocalIndex localIndex;
   // std::vector<OpenAB::SmartPtr<OpenAB_Storage::StorageItem> > localChanges;

    struct ItemDesc
    {
        ItemDesc(const std::string& _id, OpenAB::SmartPtr<OpenAB::PIMItem> vcard) :
        id (_id),
        item (vcard){}

      std::string id;
      OpenAB::SmartPtr<OpenAB::PIMItem> item;
    };

    std::vector<ItemDesc> localItemsToBeAdded;
    std::vector<ItemDesc> localItemsToBeModified;
    std::vector<std::string> localItemsToBeRemoved;

    std::vector<ItemDesc> remoteItemsToBeAdded;
    std::vector<ItemDesc> remoteItemsToBeModified;
    std::vector<std::string> remoteItemsToBeRemoved;

    std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > localyAddedItems;
    std::map<std::string, OpenAB::SmartPtr<OpenAB::PIMItem> > localyModifiedItems;

    std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > remotelyAddedItems;
    std::map<std::string, OpenAB::SmartPtr<OpenAB::PIMItem> > remotelyModifiedItems;

    bool dbError;
    bool inputError;

    pthread_t syncThread;
    pthread_mutex_t syncMutex;
    bool threadCreated;
    bool syncInProgress;
    eSync lastSyncResult;
    bool cancelSync;

    OpenAB_Sync::SyncMetadata metadata;
};

#endif /* TWO_WAY_SYNC_HPP */
