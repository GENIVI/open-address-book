/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file CardDAVHelper.hpp
 */

#ifndef CARDDAVHELPER_HPP_
#define CARDDAVHELPER_HPP_

#include "helpers/Http.hpp"
#include "DAVHelper.hpp"
#include "PIMItem/PIMItem.hpp"
/*!
 * @brief Documentation for class CardDAVHelper
 */
class CardDAVHelper
{
  public:
    /*!
     *  @brief Constructor.
     */
    CardDAVHelper(const std::string& serverUrl,
                  OpenAB::HttpSession* httpSession,
                  OpenAB::HttpAuthorizer* httpAuthorizer);

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~CardDAVHelper();

    /*!
     * @brief Query principal url.
     * @note Needs to be called before calling other functions from this class.
     * @return true if principal URL was found successfully, false otherwise.
     */
    bool findPrincipalUrl();

    /*!
     * @brief Query address book set.
     * @note: Should be called after @ref findPrincipalUrl and before @ref findAddressbooks.
     * @return true if address book set was found successfully, false otherwise.
     */
    bool findAddressbookSet();

    /*!
     * @brief Query address books.
     * @note: Should be called after @ref findAddressbookSet and before @ref queryAddressbookMetadata.
     * @return true if address book was found successfully, false otherwise.
     */
    bool findAddressbooks();

    /*!
     * @brief Query address book metadata (current revision and sync token).
     * After calling this function sync token can be obtained by using @ref getSyncToken().
     * @return true if address book metadata was queried successfully, false otherwise.
     */
    bool queryAddressbookMetadata();

    /*!
     * @brief Query contacts metadata (list of IDs and revisions).
     * After calling this function metadata can be obtained by using @ref getContactsMetadata() and @ref getTotalCount().
     * @return true if contacts metadata was queried successfully, false otherwise.
     */
    bool queryContactsMetadata();

    /*!
     * @brief Query contacts metadta only of contact that were modified since sync token.
     * After calling this function metadata can be obtained by using @ref getContactsMetadata() and @ref getTotalCount().
     * @return true if contacts metadata was queried successfully, false otherwise.
     */
    bool queryChangedContactsMetadata(const std::string& syncToken,
                                      std::vector<OpenAB::PIMItem::ID>& removed);

    /*!
     * @brief Download vCards of contacts from metadata.
     * @param [in] offset offset of contact in metadata.
     * @param [in] size number of contacts to be downloaded.
     * @param [out] vcards downloaded list of vcards in the same order as contacts in metadata.
     * @note before calling this either @ref queryContactsMetadata or @ref queryChangedContactsMetadata needs to be called
     * to populate metadata information.
     * @return true if contacts were downloaded successfully.
     */
    bool downloadVCards(unsigned int offset, unsigned int size,
                        std::vector<std::string>& vcards);

    /*!
     * @brief Download vCards of given contacts.
     * @param [in] uris list of contact ids to be downloaded.
     * @param [out] vcards downloaded list of vcards in the same order as provided ids.
     * @return true if contacts were downloaded successfully.
     */
    bool downloadVCards(std::vector<std::string>& uris,
                        std::vector<std::string>& vcards);

    /*!
     * @brief Uploads contact.
     * @param [in] vcard vcard to be uploaded
     * @param [out] uri of newly created contact.
     * @param [out] etag revision of newly created contact.
     * @return true if contact were created successfully.
     */
    bool addContact(const std::string& vcard,
                    std::string& uri,
                    std::string& etag);

    /*!
     * @brief Removes contact.
     * @param [in] uri id of contact to be removed.
     * @param [in] etag revision of contact to be removed, if revision of given contact does not match on the servers, removal will fail (not all servers supports that).
     * @return true if contact were removed successfully.
     */
    bool removeContact(const std::string& uri,
                       const std::string& etag="");

    /*!
     * @brief Modifies contact.
     * @param [in] uri id of contact to be updated.
     * @param [in] vcard vcard to be uploaded
     * @param [out] etag new revision of updated contact.
     * @return true if contact were modified successfully.
     */
    bool modifyContact(const std::string& uri,
                       const std::string& vcard,
                       std::string& etag);

    /*!
     * @brief Returns total count of contacts metadata downlaoded by @ref queryContactsMetadata or @ref queryChangedContactsMetadata.
     * @return total count of contacts metadata.
     */
    unsigned int getTotalCount() const
    {
      return contactsMetadata.size();
    }

    /*!
     * @brief Returns sync token queried by @ref queryAddressbookMetadata().
     * @return sync token
     */
    std::string getSyncToken() const
    {
      return addressbookSyncToken;
    }

    typedef struct
    {
        std::string etag;
        std::string uri;
    } ContactMetadata;

    typedef std::vector<ContactMetadata> ContactsMetadata;

    /*!
     * @brief Returns contacts metadata downlaoded by @ref queryContactsMetadata or @ref queryChangedContactsMetadata.
     * @note that calls to @ref queryContactsMetadata and @ref queryChangedContactsMetadata will overwrite results.
     * @return contacts metadata.
     */
    ContactsMetadata getContactsMetadata() const
    {
      return contactsMetadata;
    }

  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    CardDAVHelper(CardDAVHelper const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    CardDAVHelper& operator=(CardDAVHelper const &other);

    std::string       serverUrl;
    std::string       serverHostUrl;
    std::string       principalUrl;
    std::string       principalAddressbookSetUrl;
    std::string       principalAddressbookSetHostUrl;
    std::string       principalAddressbookUrl;

    DAVHelper         davHelper;
    OpenAB::HttpSession*  httpSession;
    OpenAB::HttpAuthorizer* httpAuthorizer;

    ContactsMetadata contactsMetadata;
    std::string addressbookCTag;
    std::string addressbookSyncToken;
};

#endif // CARDDAVHELPER_HPP_
