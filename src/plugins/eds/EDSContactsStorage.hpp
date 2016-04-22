/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file EDSContactsStorage.hpp
 */

#include <plugin/storage/ContactsStorage.hpp>
#include "EDSContactsStorageItemIterator.hpp"
#include <string>

#ifndef OPENAB_PLUGIN_EDS_CONTACTS_HPP_
#define OPENAB_PLUGIN_EDS_CONTACTS_HPP_

/**
 * @defgroup EDSStoragePlugin EDS Storage Plugin
 * @ingroup StoragePlugin
 *
 * @brief Provides storage for OpenAB::PIMContactItem items using Evolution Data Server.
 *
 *
 * Plugin Name: "EDS"
 *
 * Parameters:
 * | Type   | Name  | Description                 | Mandatory |
 * |:-------|:    --|:----------------------------|:          |
 * | String |  "db" | EDS source name             | Yes       |
 *
 */

class EDSContactsStorage : public OpenAB_Storage::ContactsStorage
{
  public:
    EDSContactsStorage(const std::string&);

    ~EDSContactsStorage();

    enum OpenAB_Source::Source::eInit init();

    enum OpenAB_Source::Source::eGetItemRet getItem(OpenAB::SmartPtr<OpenAB::PIMItem> & item);

    enum OpenAB_Source::Source::eSuspendRet suspend();

    enum OpenAB_Source::Source::eResumeRet resume();

    enum OpenAB_Source::Source::eCancelRet cancel();

    int getTotalCount() const;

    enum eAddItem addContact( const std::string& vCard,
                              OpenAB::PIMItem::ID& newId,
                              OpenAB::PIMItem::Revision& revision);

    enum eAddItem addContacts( const std::vector<std::string> &vCards,
                               OpenAB::PIMItem::IDs& newIds,
                               OpenAB::PIMItem::Revisions& revisions);

    enum eModifyItem modifyContact( const std::string& vCard,
                                    const OpenAB::PIMItem::ID& id,
                                    OpenAB::PIMItem::Revision& revision);

    enum eModifyItem modifyContacts( const std::vector<std::string> &vCard,
                                     const OpenAB::PIMItem::IDs& ids,
                                     OpenAB::PIMItem::Revisions& revisions);

    enum eRemoveItem removeContact( const OpenAB::PIMItem::ID& id);

    enum eRemoveItem removeContacts( const OpenAB::PIMItem::IDs& ids);

    enum eGetItem getContact (const OpenAB::PIMItem::ID & id,
                              OpenAB::SmartPtr<OpenAB::PIMContactItem> & item);

    enum eGetItem getContacts(const OpenAB::PIMItem::IDs & ids,
                              std::vector<OpenAB::SmartPtr<OpenAB::PIMContactItem> > & items);

    //TODO: add comments
    enum OpenAB_Storage::Storage::eGetRevisions getRevisions(std::map<std::string, std::string>& revisions);

    enum OpenAB_Storage::Storage::eGetRevisions getChangedRevisions(const std::string& token,
                                                            std::map<std::string, std::string>& revisions,
                                                            std::vector<OpenAB::PIMItem::ID>& removed);

    enum OpenAB_Storage::Storage::eGetSyncToken getLatestSyncToken(std::string&);

    OpenAB_Storage::StorageItemIterator* newStorageItemIterator();

  private:
    OpenAB::PIMItem::Revision getRevision(const OpenAB::PIMItem::ID& id);
    OpenAB::PIMItem::Revisions getRevisions(const OpenAB::PIMItem::IDs& ids);
    std::string       database;
    ESourceRegistry * registry;
    ESource         * source;
    EBookClient     * client;
    EDSContactsStorageItemIterator* contactsIterator;
};

#endif /* OpenAB_PLUGIN_EDS_CONTACTS_HPP_ */
