/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file OneWaySync.hpp
 */

#ifndef ONE_WAY_SYNC_HPP
#define ONE_WAY_SYNC_HPP


#include <pthread.h>
#include "plugin/sync/Sync.hpp"

/**
 * @defgroup  OneWaySync OneWay Sync Plugin
 * @ingroup SyncPlugin
 *
 * @brief The OneWay Sync plugin is responsible to perform one-way synchronization of PIM items.
 *
 * Data is synchronized between remote source and local storage.
 * The full synchronization process consists of couple phases (defined by user), each phase consists of few steps:
 *
 * @dot
 * digraph G {
 *
 *     subgraph cluster2 {
 *        label="Sync Phase";
 *
 *        node [shape=record, fontname=Helvetica, fontsize=10];
 *        input [label="Storage"];
 *        idb   [label="PIMItemIndex"];
 *
 *        subgraph cluster0 {
 *          label="Sync Workflow";
 *          node [ style=filled ];
 *          s1 [ label="Step 1" ];
 *          s2 [ label="Step 2" ];
 *          s3 [ label="Step 3" ];
 *          s1 -> s2 -> s3;
 *        }
 *
 *        subgraph cluster1 {
 *          label="Storage";
 *          abit  [ label="Iterator" ];
 *          abadd [ label="Add" ];
 *          abmod [ label="Modify" ];
 *          abdel [ label="Delete" ];
 *          abit -> abadd -> abmod -> abdel [style=invis];
 *        }
 *
 *        start [style=invis;]
 *        start -> input [style=invis];
 *        start -> idb [style=invis];
 *        start -> s1 [style=invis];
 *        start -> abit [style=invis];
 *
 *        s1      -> idb   [weight=0];
 *        s1      -> abit  [weight=0];
 *        abit    -> s1    [weight=0];
 *        idb     -> s2    [weight=0,dir=both];
 *        s2      -> abadd [weight=0];
 *        s2      -> abmod [weight=0];
 *        input   -> s2    [weight=0];
 *        idb:s   -> s3:w  [weight=0];
 *        s3      -> abdel [weight=0];
 *    }
 * }
 * @enddot
 *
 * - @b Step1: Using the @ref OpenAB_Storage::StorageItemIterator of the Storage used, build a reference list of PIMItems to be used in the next steps to identify the modified and delete items.
 *
 * - @b Step2: Listen the stream of PIMItems received from the Source plugin (OpenAB_Source::Source::getItem).
 *   - perform all the required checks to find a match in the Ztorage
 *   - mark the item as to be added or modified in the reference list (defined in the step 1)
 *   - add or modify the item using the Storage APIs (OpenAB_Storage::Storage::addItems, OpenAB_Storage::Storage::modifyItems)
 *
 * - @b Step3: Delete all the unmarked items from the Storage (OpenAB_Storage::Storage::deleteItems)
 *
 * Each phase can define PIMItem fields that should be ignored both during getting items from OpenAB_Source::Source and during comparison of them with contents of OpenAB_Storage::Storage.
 *
 * ## Parameters ##
 * Parameters:
 * | Type     | Name | Description                 | Mandatory |
 * |:---------|:     |:----------------------------|:          |
 * |String    |"remote_plugin" | name of OpenAB_Source::Source plugin to use | Yes |
 * |Struct    |remoteSourcePluginParams | parameters of OpenAB_Source::Source plugin to use | Yes |
 * |String    |"local_plugin" | name of OpenAB_Storage::Storage plugin to use | Yes |
 * |Struct    |localStoragePluginParams | parameters of OpenAB_Storage::Storage plugin | Yes |
 * |Pointer   |"callback"      | pointer to OpenAB_Sync::Sync::SyncCallback   | No       |
 * |Float     |"sync_progress_frequency" | interval of OpenAB_Sync::Sync::SyncCallback::syncProgress() emission in seconds | No |
 * |Integer   | "batch_size" | size of batches to be used on Storage operations | No |
 *
 * @todo Input: define signal for sync statistics
 * @todo Add possibility to sleep after processing each item to lower CPU consumption during sync
 */

/**
 * @brief Set of parameters required to perform the synchronization
 */
struct OpenAB_Sync_params
{
    std::string                     input_plugin; /**< @brief The name of the Input plugin */
    OpenAB_Source::Parameters          input_params; /**< @brief The parameters required by the Input plugin used*/
    std::string                     ab_plugin;    /**< @brief The name of the AB (Address Book) plugin */
    OpenAB_Storage::Parameters         ab_params;    /**< @brief The parameters required by the AB plugin used*/
    OpenAB_Sync::Sync::SyncCallback *  cb;           /**< @brief The Callback object */
    enum SyncType
    {
      eSyncIncremental,
      eSyncAll,
      eSyncText
    }                               sync_type;
    float                           sync_progress_time;
    unsigned int                    batch_size;
};

/**
 * @brief Sync Class providing all the Sync controls
 */
class OneWaySync : public OpenAB_Sync::Sync
{
  public:
    OneWaySync(OpenAB_Sync_params&);
    ~OneWaySync();

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
    OneWaySync(OneWaySync const &other);

    /**
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    OneWaySync& operator=(OneWaySync const &other);

    void updateIndexDB();
    void processItems(unsigned int phaseNum);
    void cleanStorage();

    void addItem(const OpenAB::SmartPtr<OpenAB::PIMItem> & item);
    void modifyItem(const std::string& id, const OpenAB::SmartPtr<OpenAB::PIMItem> & item);
    bool flushInsertions();
    bool flushModifications();

    enum eSync doSynchronize();
    static void* threadSync(void*);

    struct SyncStats {
        void clean(){
          added    = 0;
          modified = 0;
          removed  = 0;
        };
        unsigned int added;
        unsigned int modified;
        unsigned int removed;
        pthread_mutex_t mutex;

    };

    SyncStats phaseStats;
    SyncStats globalStats;

    OpenAB_Sync_params       params;

    OpenAB_Source::Source*   source;
    OpenAB_Storage::Storage* storage;

    typedef std::vector< OpenAB::SmartPtr<OpenAB_Storage::StorageItem> > vectorElem;
    typedef std::map< OpenAB::SmartPtr<OpenAB::PIMItemIndex> , vectorElem > dbIndexElem;
    dbIndexElem indexDB;

    struct ItemDesc
    {
        ItemDesc(const std::string& _id, OpenAB::SmartPtr<OpenAB::PIMItem> vcard) :
        id (_id),
        item (vcard){}

      std::string id;
      OpenAB::SmartPtr<OpenAB::PIMItem> item;
    };

    std::vector<ItemDesc> itemsToBeAdded;
    std::vector<ItemDesc> itemsToBeModified;

    bool dbError;
    bool inputError;

    pthread_t syncThread;
    pthread_mutex_t syncMutex;
    bool threadCreated;
    bool syncInProgress;
    eSync lastSyncResult;
    bool cancelSync;
};

#endif /* ONE_WAY_SYNC_HPP */
