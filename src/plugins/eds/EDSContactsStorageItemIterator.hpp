/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file EDSContactsStorageItemIterator.hpp
 */

#ifndef EDSCONTACTSSTORAGEITEMITERATOR_HPP_
#define EDSCONTACTSSTORAGEITEMITERATOR_HPP_

#include <plugin/storage/StorageItem.hpp>
#include "OpenAB_eds_global.h"
/*!
 * @brief Documentation for class EDSStorageItemIterator
 */
class EDSContactsStorageItemIterator : public OpenAB_Storage::StorageItemIterator
{
  public:
    /*!
     *  @brief Constructor.
     */
    EDSContactsStorageItemIterator();

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~EDSContactsStorageItemIterator();

    enum eCursorInit{
      eCursorInitOK,
      eCursorInitFail
    };
    enum eCursorInit cursorInit(EBookClient*);

    OpenAB_Storage::StorageItem* next();

    OpenAB_Storage::StorageItem  operator*();
    OpenAB_Storage::StorageItem* operator->();

    unsigned int getSize() const;

  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    EDSContactsStorageItemIterator(EDSContactsStorageItemIterator const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    EDSContactsStorageItemIterator& operator=(EDSContactsStorageItemIterator const &other);

    enum eFetchContacts {
      eFetchContactsOK,
      eFetchContactsEND,
      eFetchContactsFail
    };
    enum eFetchContacts fetchContacts(int fetchsize);

    OpenAB_Storage::StorageItem    elem;
    EBookClientCursor *         cursor;
    GSList *                    contacts;
    int                         total;
};

#endif // EDSCONTACTSSTORAGEITEMITERATOR_HPP_
