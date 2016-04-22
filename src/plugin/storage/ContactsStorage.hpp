/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file ContactsStorage.hpp
 */

#ifndef CONTACTSSTORAGE_H_
#define CONTACTSSTORAGE_H_

#include "Storage.hpp"
#include <PIMItem/Contact/PIMContactItem.hpp>

/*!
 * @brief namespace OpenAB_Storage
 */
namespace OpenAB_Storage {

/*!
 * @brief Documentation for ContactsStorage interface.
 * Provides functionalities specific to Storage of OpenAB::eContact items.
 * @todo extend interface (locale handling, search queries etc)
 * @todo define new return codes for contact specific operations.
 */
class ContactsStorage : public Storage
{
  public:
    /*!
     *  @brief Constructor.
     */
    ContactsStorage();

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~ContactsStorage();

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
    virtual enum Storage::eAddItem addContact( const std::string& vCard,
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
    virtual enum Storage::eAddItem addContacts( const std::vector<std::string> &vCards,
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
    virtual enum Storage::eModifyItem modifyContact( const std::string& vCard,
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
    virtual enum Storage::eModifyItem modifyContacts( const std::vector<std::string> &vCards,
                                                      const OpenAB::PIMItem::IDs& ids,
                                                      OpenAB::PIMItem::Revisions& revisions) = 0;

    /**
     * @brief Removes contact from the ContactsStorage
     *
     * @param [in] id The ID of the contact that must be removed.
     * @return the status code
     */
    virtual enum Storage::eRemoveItem removeContact( const OpenAB::PIMItem::ID& id) = 0;

    /**
     * @brief Removes contacts from the ContactsStorage
     *
     * @param [in] ids The vector of ID of the contacts that must be removed.
     * @return the status code
     */
    virtual enum Storage::eRemoveItem removeContacts( const OpenAB::PIMItem::IDs& ids) = 0;

    /**
     * @brief Get the contact from the Storage
     *
     * @param [in] id The ID of the contact that must be retrieved.
     * @param [out] item The OpenAB::PIMContactItem that was retrieved.
     * @return the status code
     */
    virtual enum Storage::eGetItem getContact (const OpenAB::PIMItem::ID & id,
                                               OpenAB::SmartPtr<OpenAB::PIMContactItem> & item) = 0;

    /**
     * @brief Get the contacts from the Storage
     *
     * @param [in] id The IDs of the contacts that must be retrieved.
     * @param [out] items The OpenAB::PIMContactItem that was retrieved.
     * @return the status code
     */
    virtual enum eGetItem getContacts(const OpenAB::PIMItem::IDs & ids,
                                      std::vector<OpenAB::SmartPtr<OpenAB::PIMContactItem> > & items) = 0;

  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    ContactsStorage(ContactsStorage const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    ContactsStorage& operator=(ContactsStorage const &other);
};

} // namespace OpenAB_Storage

#endif // CONTACTSSTORAGE_H_
