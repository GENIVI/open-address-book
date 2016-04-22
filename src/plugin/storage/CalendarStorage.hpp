/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file CalendarStorage.hpp
 */

#ifndef CALENDARSTORAGE_H_
#define CALENDARSTORAGE_H_

#include "Storage.hpp"
#include <PIMItem/Calendar/PIMCalendarItem.hpp>

/*!
 * @brief namespace OpenAB_Storage
 */
namespace OpenAB_Storage {

//TODO: update documentation
/*!
 * @brief Documentation for ContactsStorage interface.
 * Provides functionalities specific to Storage of OpenAB::eContact items.
 * @todo extend interface (locale handling, search queries etc)
 * @todo define new return codes for contact specific operations.
 */
class CalendarStorage : public Storage
{
  public:
    /*!
     *  @brief Constructor.
     */
    CalendarStorage(OpenAB::PIMItemType type);

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~CalendarStorage();

    enum Storage::eAddItem addItem(const OpenAB::SmartPtr<OpenAB::PIMItem>& item,
                                   OpenAB::PIMItem::ID & newId,
                                   OpenAB::PIMItem::Revision & revision);
    enum Storage::eAddItem addItems(const std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > & items,
                                    OpenAB::PIMItem::IDs & newIds,
                                    OpenAB::PIMItem::Revisions & revisions);
    enum Storage::eModifyItem modifyItem(const OpenAB::SmartPtr<OpenAB::PIMItem>& item,
                                         const OpenAB::PIMItem::ID & id,
                                         OpenAB::PIMItem::Revision & revision);
    enum Storage::eModifyItem modifyItems(const std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > & items,
                                          const OpenAB::PIMItem::IDs & ids,
                                          OpenAB::PIMItem::Revisions & revisions);
    enum Storage::eRemoveItem removeItem(const OpenAB::PIMItem::ID & id);
    enum Storage::eRemoveItem removeItems(const OpenAB::PIMItem::IDs & ids);
    enum Storage::eGetItem getItem(const OpenAB::PIMItem::ID & id, OpenAB::SmartPtr<OpenAB::PIMItem> & item);
    enum Storage::eGetItem getItems(const OpenAB::PIMItem::IDs & id, std::vector<OpenAB::SmartPtr<OpenAB::PIMItem> > & item);

    /**
     * @brief Adds a new contact to the ContactsStorage.
     *
     * @param [in] vCard vCard of contact to should added.
     * @param [out] newId The newly added contact ID.
     * @param [out] revision The revision of newly added contact - can be empty if revisions are not supported.
     * @return the status code
     */
    virtual enum Storage::eAddItem addObject( const std::string& iCal,
                                              OpenAB::PIMItem::ID& newId,
                                              OpenAB::PIMItem::Revision& revision) = 0;

   /**
     * @brief Adds new contacts to the ContactsStorage.
     *
     * @param [in] vCards The vector of contacts' vCards that should be added.
     * @param [out] newIds The newly added contacts ID in the same order as provided vCards.
     * @param [out] revisions The revisions of newly added contacts (in the same order as provided vCards).
     * Can be empty if revisions are not supported.
     * @return the status code
     */
    virtual enum Storage::eAddItem addObjects( const std::vector<std::string> &iCals,
                                               OpenAB::PIMItem::IDs& newIds,
                                               OpenAB::PIMItem::Revisions& revisions) = 0;

    /**
     * @brief Modifies contact in the ContactsStorage
     *
     * @param [in] vCard vCard of contact that should be modified.
     * @param [in] id The ID of the item that should be modified.
     * @param [out] revision The updated revision of modified contact.
     * @return the status code
     */
    virtual enum Storage::eModifyItem modifyObject ( const std::string& iCal,
                                                     const OpenAB::PIMItem::ID& id,
                                                     OpenAB::PIMItem::Revision& revision) = 0;

    /**
     * @brief Modifies contacts in the ContactsStorage
     *
     * @param [in] vCards The vector of contact's vCards that should be modified.
     * @param [in] ids The vector of ID of the contacts that must be modified (in the same order as provided vCards).
     * @param [out] revisions The updated revisions of modified contacts (in the same order as provided vCards).
     * @return the status code
     */
    virtual enum Storage::eModifyItem modifyObjects( const std::vector<std::string> &iCals,
                                                     const OpenAB::PIMItem::IDs& ids,
                                                     OpenAB::PIMItem::Revisions& revisions) = 0;
    /**
     * @brief Removes contact from the ContactsStorage
     *
     * @param [in] id The ID of the contact that must be removed.
     * @return the status code
     */
    virtual enum Storage::eRemoveItem removeObject( const OpenAB::PIMItem::ID& id) = 0;

    /**
     * @brief Removes contacts from the ContactsStorage
     *
     * @param [in] ids The vector of ID of the contacts that must be removed.
     * @return the status code
     */
    virtual enum Storage::eRemoveItem removeObjects( const OpenAB::PIMItem::IDs& ids) = 0;

    /**
     * @brief Get the contact from the Storage
     *
     * @param [in] id The ID of the contact that must be retrieved.
     * @param [out] item The OpenAB::PIMContactItem that was retrieved.
     * @return the status code
     */
    virtual enum Storage::eGetItem getEvent(const OpenAB::PIMItem::ID & id,
                                            OpenAB::SmartPtr<OpenAB::PIMCalendarEventItem> & item) = 0;

    /**
     * @brief Get the contacts from the Storage
     *
     * @param [in] id The IDs of the contacts that must be retrieved.
     * @param [out] items The OpenAB::PIMContactItem that was retrieved.
     * @return the status code
     */
    virtual enum eGetItem getEvents(const OpenAB::PIMItem::IDs & ids,
                                    std::vector<OpenAB::SmartPtr<OpenAB::PIMCalendarEventItem> > & items) = 0;

    /**
     * @brief Get the contact from the Storage
     *
     * @param [in] id The ID of the contact that must be retrieved.
     * @param [out] item The OpenAB::PIMContactItem that was retrieved.
     * @return the status code
     */
    virtual enum Storage::eGetItem getTask(const OpenAB::PIMItem::ID & id,
                                           OpenAB::SmartPtr<OpenAB::PIMCalendarTaskItem> & item) = 0;

    /**
     * @brief Get the contacts from the Storage
     *
     * @param [in] id The IDs of the contacts that must be retrieved.
     * @param [out] items The OpenAB::PIMContactItem that was retrieved.
     * @return the status code
     */
    virtual enum eGetItem getTasks(const OpenAB::PIMItem::IDs & ids,
                                   std::vector<OpenAB::SmartPtr<OpenAB::PIMCalendarTaskItem> > & items) = 0;

  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    CalendarStorage(CalendarStorage const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    CalendarStorage& operator=(CalendarStorage const &other);
};

} // namespace OpenAB_Storage

#endif // CALENDARSTORAGE_H_
