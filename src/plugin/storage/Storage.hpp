/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file Storage.hpp
 */

#ifndef PIM_STORAGE_HPP_
#define PIM_STORAGE_HPP_

#include <map>
#include <string>
#include <vector>
#include <plugin/Plugin.hpp>
#include <helpers/SmartPtr.hpp>
#include <plugin/GenericParameters.hpp>
#include <plugin/source/Source.hpp>
#include "StorageItem.hpp"


/*!
 * @brief namespace OpenAB_Storage
 */
namespace OpenAB_Storage {

/**
 * @defgroup StoragePlugin Storage Plugin
 * @ingroup pluginGroup
 *
 * Interface: @ref OpenAB_Storage::Storage
 *
 ## Overview ##
 *
 * The Storage plugin interface provide all the required functionalities to manage an storage of PIM items.
 *
 * Storage are extensions of OpenAB::Source that can store items on local storage or in remote databases, all API calls are are synchronous and should block until requested operation finishes.
 *
 * Generic interface is modeled over @ref SyncPlugin specific requirements, it define a placeholder for all the functionalities required by the Sync Plugins.
 *
 *
 * Each type of PIMItem (@ref OpenAB::PIMItemType) can define their own interface for accessing Storage providing functions specific to given item type (@ref OpenAB_Storage::ContactsStorage).
 * These interfaces can be used later to receive contents of Storage in more complex manner (eg. using search queries).
 ## API Details ##
 *
 ### Storage functions ###
 *
 * These functions are required to manage the basic Storage operations:
 *
 *  - @ref OpenAB_Storage::Storage::addItem    ( @copybrief OpenAB_Storage::Storage::addItem )
 *  - @ref OpenAB_Storage::Storage::removeItem ( @copybrief OpenAB_Storage::Storage::removeItem )
 *  - @ref OpenAB_Storage::Storage::modifyItem ( @copybrief OpenAB_Storage::Storage::modifyItem )
 *  - @ref OpenAB_Storage::Storage::getItem    ( @copybrief OpenAB_Storage::Storage::getItem(const OpenAB::PIMItem::ID & id, OpenAB::SmartPtr<OpenAB::PIMItem>& item) )
 *
 *  Additionally versions of above functions that supports batching are required:
 *  - @ref OpenAB_Storage::Storage::addItems    ( @copybrief OpenAB_Storage::Storage::addItems )
 *  - @ref OpenAB_Storage::Storage::removeItems ( @copybrief OpenAB_Storage::Storage::removeItems )
 *  - @ref OpenAB_Storage::Storage::modifyItems ( @copybrief OpenAB_Storage::Storage::modifyItems )
 *  - @ref OpenAB_Storage::Storage::getItems    ( @copybrief OpenAB_Storage::Storage::getItems )
 *
 *  These functions are required to manage more complicated Storage tasks, needed mostly during synchronization:
 *
 *  - @ref OpenAB_Storage::Storage::getLatestSyncToken    ( @copybrief OpenAB_Storage::Storage::getLatestSyncToken )
 *  - @ref OpenAB_Storage::Storage::getRevisions          ( @copybrief OpenAB_Storage::Storage::getRevisions)
 *  - @ref OpenAB_Storage::Storage::getChangedRevisions   ( @copybrief OpenAB_Storage::Storage::getChangedRevisions)
 *
 *
 *
 ### Quick Browse functions ###
 *
 * These functions are required to quickly build/retrieve a complete summary of Storage.
 *
 * To achieve this, an iterator ( @ref OpenAB_Storage::StorageItemIterator ) is used.
 *
 * This iterator can be generated using the function:
 *
 *  - @ref OpenAB_Storage::Storage::newStorageItemIterator ( @copybrief OpenAB_Storage::Storage::newStorageItemIterator )
 *
 * The iterator can be parsed using this function:
 *
 *  - @ref OpenAB_Storage::StorageItemIterator::next    ( @copybrief OpenAB_Storage::Storage::IndexElemIterator::next )
 *
 *  It return an OpenAB_Storage::StorageItem will be returned
 *
 *  - @ref OpenAB_Storage::StorageItem    ( @copybrief OpenAB_Storage::StorageItem )
 *
 *  This element can be used to summarize the item using few keywords.
 *
 *  It is mainly used to associate an item to the Storage Unique ID.
 *
 */

/**
 * @brief Use generic parameters.
 */
typedef OpenAB_Plugin::GenericParameters Parameters;

/**
 * @class Storage
 *
 * @brief Documentation for Storage plugin interface.
 * Generic interface is modeled over @ref SyncPlugin specific requirements,
 * it define a palceholder for all the functionalities required by the Sync Plugins.
 */
class Storage : public OpenAB_Source::Source
{
  public:
    /**
     * @brief Constructor.
     * @param [in] t type of provided PIM item (@ref OpenAB::PIMItem)
     */
    Storage(OpenAB::PIMItemType t)
      : OpenAB_Source::Source(t)
      {};

    /**
     *  @brief Destructor, virtual by default.
     */
    virtual ~Storage(){};

    /** @enum
     * addItem()/addItems() return code
     *
     */
    enum eAddItem{
      eAddItemOk,   /**< @brief Item/Items correctly added */
      eAddItemFail  /**< @brief Failure during the operation */
    };

    /**
     * @brief Adds a new item (@ref OpenAB::PIMItem) to the Storage.
     *
     * @param [in] item PIMItem that should be added
     * (PIMItem type needs to be the same as type of
     * item supported by Storage (@ref Storage::getItemType()).
     * @param [out] newId The newly added item ID.
     * @param [out] revision The revision of newly added item - can be empty if revisions are not supported.
     * @return the status code
     */
    virtual enum eAddItem addItem(const OpenAB::SmartPtr<OpenAB::PIMItem>& item,
                                  OpenAB::PIMItem::ID & newId,
                                  OpenAB::PIMItem::Revision & revision) = 0;

    /**
     * @brief Adds new items (@ref OpenAB::PIMItem) to the Storage.
     *
     * @param [in] items The vector of items that should be added.
     * (PIMItem type needs to be the same as type of
     * item supported by Storage (@ref Storage::getItemType()).
     * @param [out] newIds The newly added items ID in the same order as provided items.
     * @param [out] revisions The revisions of newly added items (in the same order as provided items).
     * @return the status code
     */
    virtual enum eAddItem addItems(const std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > & items,
                                   OpenAB::PIMItem::IDs & newIds,
                                   OpenAB::PIMItem::Revisions & revisions) = 0;


    /** @enum
     * modifyContact() return code
     *
     */
    enum eModifyItem{
          eModifyItemOk,   /**< @brief Item was correctly modified */
          eModifyItemFail  /**< @brief Failure during the operation */
    };

    /**
     * @brief Modifies item (@ref OpenAB::PIMItem) in the Storage
     *
     * @param [in] item The item that should be modified.
     * (PIMItem type needs to be the same as type of
     * item supported by Storage (@ref Storage::getItemType()).
     * @param [in] id The ID of the item that should be modified.
     * @param [out] revision The updated revision of modified items.
     * @return the status code
     */
    virtual enum eModifyItem modifyItem(const OpenAB::SmartPtr<OpenAB::PIMItem>& item,
                                        const OpenAB::PIMItem::ID & id,
                                        OpenAB::PIMItem::Revision & revision) = 0;

    /**
     * @brief Modifies items (@ref OpenAB::PIMItem) in the Storage
     *
     * @param [in] items The vector of items to be modified.
     * (PIMItem type needs to be the same as type of
     * item supported by Storage (@ref Storage::getItemType()).
     * @param [in] ids The vector of ID of the items that must be modified (in the same order as provided items).
     * @param [out] revisions The updated revisions of modified items (in the same order as provided items).
     * @return the status code
     */
    virtual enum eModifyItem modifyItems(const std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > & items,
                                         const OpenAB::PIMItem::IDs & ids,
                                         OpenAB::PIMItem::Revisions &revisions) = 0;

    /** @enum
     * removeContact() return code
     *
     */
    enum eRemoveItem{
          eRemoveItemOk,   /**< @brief Item was correctly removed */
          eRemoveItemFail  /**< @brief Failure during the operation */
    };

    /**
     * @brief Removes item from the Storage
     *
     * @param [in] id The ID of the item that must be removed.
     * @return the status code
     */
    virtual enum eRemoveItem removeItem(const OpenAB::PIMItem::ID & id) = 0;

    /**
     * @brief Removes items from the Storage
     *
     * @param [in] ids The vector of ID of the items that must be removed.
     * @return the status code
     */
    virtual enum eRemoveItem removeItems(const OpenAB::PIMItem::IDs & ids) = 0;

    /** @enum
     * getItem() return code
     *
     */
    enum eGetItem{
      eGetItemOk,   /**< @brief Item was correctly found */
      eGetItemFail  /**< @brief Failure during the operation */
    };

    virtual enum eGetItemRet getItem(OpenAB::SmartPtr<OpenAB::PIMItem> &item) = 0;

    /**
     * @brief Get the item from the Storage
     *
     * @param [in] id The ID of the item that must be retrieved.
     * @param [out] item The item that was retrieved.
     * @return the status code
     */
    virtual enum eGetItem getItem(const OpenAB::PIMItem::ID & id, OpenAB::SmartPtr<OpenAB::PIMItem>& item) = 0;

    /**
     * @brief Get the items from the Storage
     *
     * @param [in] ids The IDs of the items that must be retrieved.
     * @param [out] items The items that was retrieved.
     * @return the status code
     */
    virtual enum eGetItem getItems(const OpenAB::PIMItem::IDs & ids, std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > & items) = 0;

    enum eGetSyncToken {
      eGetSyncTokenOk,   /**< @brief Token was correctly retrieved */
      eGetSyncTokenFail  /**< @brief Failure during the operation, or tokens are not supported */
    };

    /**
     * @brief If storage supports tracking of items changes,
     * it returns latest status identifier of the storage.
     *
     * @param [out] token latest status identifier
     * @return eGetSyncTokenFail if storage does not supports changes tracking, eGetSyncTokenOk otherwise.
     */
    virtual OpenAB_Storage::Storage::eGetSyncToken getLatestSyncToken(std::string& token) = 0;

    enum eGetRevisions {
      eGetRevisionsOk,   /**< @brief Revisions was correctly retrieved */
      eGetRevisionsFail  /**< @brief Failure during the operation */
    };

    /**
     * @brief Gets revision of the items from the Storage.
     *
     * @param [out] revisions map containing items IDs and their current revisions.
     * @return the status code
     */
    virtual OpenAB_Storage::Storage::eGetRevisions getRevisions(std::map<std::string, std::string>& revisions) = 0;

    /**
     * @brief Gets revisions of items changed since Storage was in state identified by token.
     *
     * @param [in] token database token since changes should be returned (@ref getLatestSyncToken())
     * @param [out] revisions revisions map containing items IDs and their current revisions
     * @param [out] removed IDs vector of removed items
     * @return the status code
     */
    virtual OpenAB_Storage::Storage::eGetRevisions getChangedRevisions(const std::string& token,
                                                                    std::map<std::string, std::string>& revisions,
                                                                    std::vector<OpenAB::PIMItem::ID>& removed) = 0;


    /**
     * @brief Retrieve an new Iterator to quickly parse the list of items
     *
     * @return the new Iterator for this Storage
     */
    virtual StorageItemIterator* newStorageItemIterator() = 0;


  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    Storage(Storage const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    Storage& operator=(Storage const &other);
};

} // namespace OpenAB_Storage

DECLARE_PLUGIN_INTERFACE(OpenAB_Storage, Storage, Parameters);

#endif // PIM_STORAGE_HPP_
