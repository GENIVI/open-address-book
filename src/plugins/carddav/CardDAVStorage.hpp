/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file CardDAVStorage.hpp
 */

#ifndef CARDDAV_H_
#define CARDDAV_H_

#include <plugin/storage/ContactsStorage.hpp>
#include <list>
#include "helpers/Http.hpp"
#include "CardDAVHelper.hpp"

class CardDAVStorageItemIterator;

/**
 * @defgroup CardDAVStorage CardDAV Storage Plugin
 * @ingroup StoragePlugin
 *
 * @brief Provides OpenAB::PIMContactItem items using CardDAV protocol (tested with Google Contacts, iCloud).
 *
 * Plugin Name: "CardDAV"
 *
 * Parameters:
 * | Type       | Name  |  Description                                         | Mandatory |
 * |:-----------|:      |:-----------------------------------------------------| :         |
 * | String | "server_url"    | CardDAV server address                      | Yes       |
 * | String | "login"         | User login                                  | Yes        |
 * | String | "password"      | User password                               | Yes        |
 * | String | "client_id"     | Id of client application (registered in Google)     | Yes       |
 * | String | "client_secret" | Secret of client application (registered in Google) | Yes       |
 * | String | "refresh_token" | OAuth2 user refresh token                           | Yes       |
 *
 *@note "login" and "password" pair or
 *  triple "client_id", "client_secret" and "refresh_token"
 *  are mandatory to provide, depending on authorization method used by CardDAV server.
 *  When using OAuth2 refresh token it has to have required scope (for Google contacts this is https://www.googleapis.com/auth/carddav).
 *
 */
class CardDAVStorage : public OpenAB_Storage::ContactsStorage
{
  public:
    /*!
     *  @brief Constructor.
     */
    CardDAVStorage(const std::string& url,
                  const std::string& login,
                  const OpenAB::SecureString& password);

    CardDAVStorage(const std::string& url,
                  const std::string& clientId,
                  const OpenAB::SecureString& clientSecret,
                  const OpenAB::SecureString& refreshToken);

    virtual ~CardDAVStorage();

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

    enum OpenAB_Storage::Storage::eGetRevisions getRevisions(std::map<std::string, std::string>& revisions);

    enum OpenAB_Storage::Storage::eGetRevisions getChangedRevisions(const std::string& token,
                                                            std::map<std::string, std::string>& revisions,
                                                            std::vector<OpenAB::PIMItem::ID>& removed);

    enum OpenAB_Storage::Storage::eGetSyncToken getLatestSyncToken(std::string& token);

    OpenAB_Storage::StorageItemIterator* newStorageItemIterator();

  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    CardDAVStorage(CardDAVStorage const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    CardDAVStorage& operator=(CardDAVStorage const &other);

    void construct();

    void cleanup();

    bool downloadVCards(unsigned int offset, unsigned int size);

    std::string       serverUrl;

    std::string       userLogin;
    OpenAB::SecureString userPassword;

    std::string       clientId;
    OpenAB::SecureString clientSecret;
    OpenAB::SecureString refreshToken;

    std::string syncToken;

    OpenAB::HttpSession curlSession;
    OpenAB::HttpAuthorizer* authorizer;
    CardDAVHelper* cardDAVHelper;
    CardDAVStorageItemIterator* sourceIterator;
};

class CardDAVStorageItemIterator : public OpenAB_Storage::StorageItemIterator
{
  public:
    /*!
     *  @brief Constructor.
     */
    CardDAVStorageItemIterator();

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~CardDAVStorageItemIterator();

    enum eCursorInit{
      eCursorInitOK,
      eCursorInitFail
    };
    enum eCursorInit cursorInit(CardDAVHelper* cardDavHelper);

    OpenAB_Storage::StorageItem* next();

    OpenAB_Storage::StorageItem  operator*();
    OpenAB_Storage::StorageItem* operator->();

    unsigned int getSize() const;

    /*OpenAB_Source::Source::eSuspendRet suspend();
    OpenAB_Source::Source::eResumeRet resume();
    OpenAB_Source::Source::eCancelRet cancel();*/

  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    CardDAVStorageItemIterator(CardDAVStorageItemIterator const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    CardDAVStorageItemIterator& operator=(CardDAVStorageItemIterator const &other);

    enum eFetchContacts {
      eFetchContactsOK,
      eFetchContactsEND,
      eFetchContactsFail
    };
    enum eFetchContacts fetchContacts(int fetchsize);

    static void* downloadThreadFunc(void* ptr);
    bool downloadVCards(unsigned int offset, unsigned int size);
    OpenAB_Storage::StorageItem    elem;
    CardDAVHelper*              cardDavHelper;
    unsigned                    total;
    unsigned int                offset;
    unsigned int                offsetOfCachedVCards;
    std::list<OpenAB::SmartPtr<OpenAB::PIMContactItem> >  cachedContacts;
    std::vector<CardDAVHelper::ContactMetadata> contactsMetadata;
    bool                 paused;
    bool                 cancelled;
    pthread_t            downloadThread;
    bool                 threadCreated;
    pthread_mutex_t      mutex;
    pthread_cond_t       bufferReadyCond;
    OpenAB_Source::Source::eGetItemRet transferStatus;
};

#endif // CARDDAV_H_
