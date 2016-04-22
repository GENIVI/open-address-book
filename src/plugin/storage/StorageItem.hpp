/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file StorageItem.hpp
 */

#ifndef STORAGE_ITEM_HPP_
#define STORAGE_ITEM_HPP_

#include <PIMItem/PIMItem.hpp>
#include <string>

namespace OpenAB_Storage {

/**
 * @brief This object associates @ref OpenAB::PIMItem with its unique ID from OpenAB_Storage::Storage.
 * Additionally it stores status flag used in synchronization process.
 */
class StorageItem
{
  public:
    /**
     * @brief Default constructor.
     */
    StorageItem():status(StorageItem::ITEM_NOT_FOUND){};

    /**
     * @brief Constructor.
     * @param [in] i unique ID of item.
     * @param [in] v raw pointer to OpenAB::PIMItem.
     * Ownership of v pointer is moved to StorageItem (it will be converted to OpenAB::SmartPtr and will be freed automatically).
     */
    StorageItem(std::string i,
                OpenAB::PIMItem* v)
      : id(i)
      , item(v)
      , status(StorageItem::ITEM_NOT_FOUND) {};

    /**
     * @brief Constructor.
     * @param [in] i unique ID of item.
     * @param [in] v OpenAB::SmartPtr to OpenAB::PIMItem.
     */
    StorageItem(std::string i,
                OpenAB::SmartPtr<OpenAB::PIMItem> v)
      : id(i)
      , item(v)
      , status(StorageItem::ITEM_NOT_FOUND) {};

    /**
     * @brief Copy constructor.
     * @param [in] other instance to copy from.
     */
    StorageItem(const StorageItem& other)
    {
      id = other.id;
      item = other.item;
      status = other.status;
    }

    /**
     * @brief Destructor.
     */
    ~StorageItem()
    {
    }

    /**
     * @brief Comparison operator.
     * Compares id, status and OpenAB::PIMItemIndex of both instance.
     * @param [in] lhs instance to compare with
     */
    bool operator==(const StorageItem& lhs)
        {return id      == lhs.id      &&
                item->getIndex()->compare(*lhs.item->getIndex()) &&
                status  == lhs.status ;};

    std::string   id;      /**< @brief OpenAB_Storage::Storage unique ID of the item*/
    OpenAB::SmartPtr<OpenAB::PIMItem> item;    /**< @brief PIMItem*/

    enum{
      ITEM_ADDED,
      ITEM_MODIFIED,
      ITEM_FOUND,
      ITEM_REMOVED,
      ITEM_NOT_FOUND
    }status;
};


/**
 * @brief The StorageItemIterator is mainly used to quickly browse the Storage contents
 */
class StorageItemIterator
{
  public:
    StorageItemIterator(){}
    virtual ~StorageItemIterator(){}

    /**
     * @brief  Retrieve the next StorageItem
     * @return The next StorageItem or NULL if no more StorageItem are available
     */
    virtual StorageItem*  next() = 0;
    /**
     * @brief  Retrieve the current StorageItem
     * @return The current StorageItem
     */
    virtual StorageItem   operator*() = 0;
    /**
     * @brief  Retrieve the current StorageItem
     * @return The current StorageItem or NULL if no more StorageItem are available
     */
    virtual StorageItem*  operator->() = 0;

    /**
     * @brief Retrieve number of items in interator
     * @return number of items
     */
    virtual unsigned int getSize() const = 0;
};

}

#endif /*  StorageItemIterator  */
