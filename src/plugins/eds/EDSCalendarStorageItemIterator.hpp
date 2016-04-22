/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file EDSCalendarStorageItemIterator.hpp
 */

#ifndef EDSCALENDARSTORAGEITEMITERATOR_HPP_
#define EDSCALENDARSTORAGEITEMITERATOR_HPP_

#include <plugin/storage/StorageItem.hpp>
#include <fstream>
#include <list>
#include <set>
#include "OpenAB_eds_global.h"

/*!
 * @brief Documentation for class EDSCalendarStorageItemIterator
 */
class EDSCalendarStorageItemIterator : public OpenAB_Storage::StorageItemIterator
{
  public:
    /*!
     *  @brief Constructor.
     */
    EDSCalendarStorageItemIterator();

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~EDSCalendarStorageItemIterator();

    enum eCursorInit{
      eCursorInitOK,
      eCursorInitFail
    };
    enum eCursorInit cursorInit(const std::string& dbFileName);

    OpenAB_Storage::StorageItem* next();

    OpenAB_Storage::StorageItem  operator*();
    OpenAB_Storage::StorageItem* operator->();

    unsigned int getSize() const;

  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    EDSCalendarStorageItemIterator(EDSCalendarStorageItemIterator const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    EDSCalendarStorageItemIterator& operator=(EDSCalendarStorageItemIterator const &other);

    enum eFetchEvents {
      eFetchEventsOK,
      eFetchEventsEND,
      eFetchEventsFail
    };
    enum eFetchEvents fetchEvents(int fetchsize);

    OpenAB_Storage::StorageItem    elem;
    std::ifstream               databaseFile;
    int                         total;
    std::list<std::string>      events;
};

#endif // EDSCALENDARSTORAGEITEMITERATOR_HPP_
