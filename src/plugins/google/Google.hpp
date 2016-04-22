/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file Google.hpp
 */

#ifndef GOOGLE_H_
#define GOOGLE_H_

#include <plugin/source/Source.hpp>
#include <gdata/gdata.h>
#include <glib2/OpenAB_glib2_global.h>
#include <list>

/**
 * @defgroup GoogleSource Google Contacts Source Plugin
 * @ingroup SourcePlugin
 *
 * @brief Provides OpenAB::PIMContactItem items from Google Contacts.
 *
 * Plugin Name: "Google"
 *
 * Parameters:
 * | Type       | Name  |  Description                                              | Mandatory |
 * |:-----------|:      |:  --------------------------------------------------------| :         |
 * | String  |  "client_id"     | Id of client application (registered in Google)     | Yes       |
 * | String  |  "client_secret" | Secret of client application (registered in Google) | Yes       |
 * | String  |  "refresh_token" | OAuth2 user refresh token                           | Yes       |
 * | String  |  "ignore_fields" | Comma separated list of vCard fields to be not downloaded (currently only PHOTOS is supported)    | No |
 *
 *@note Source does not support total number of vCards.
 */
class GoogleSource : public OpenAB_Source::Source
{
  public:
    /*!
     *  @brief Constructor.
     */
    GoogleSource(const std::string& login,
                 const OpenAB::SecureString& password,
                 const std::string& filter);

    GoogleSource(const std::string& clientId,
                 const OpenAB::SecureString& clientSecret,
                 const OpenAB::SecureString& refreshToken,
                 const std::string& filter);


    virtual ~GoogleSource();

    enum OpenAB_Source::Source::eInit init();

    enum OpenAB_Source::Source::eGetItemRet getItem(OpenAB::SmartPtr<OpenAB::PIMItem> & item);

    enum OpenAB_Source::Source::eSuspendRet suspend();

    enum OpenAB_Source::Source::eResumeRet resume();

    enum OpenAB_Source::Source::eCancelRet cancel();

    int getTotalCount() const;

  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    GoogleSource(GoogleSource const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    GoogleSource& operator=(GoogleSource const &other);

    /*!
     * @brief Common construction routine.
     * @param [in] ignoreFields string with vCard fields to be ignored.
     */
    void construct(const std::string& ignoreFields);

    void cleanup();

    /*!
     * @brief Converts GData contact to vCard.
     * @param [in] contact contact.
     * @param [in] photoData photo data can be NULL.
     * @param [in] photoDataLen length of photo data, used only when photoData != NULL.
     * @param [in] photoContentType photo format, used only when photoData != NULL.
     * @return converted vCard.
     */
    std::string convertContact(GDataContactsContact* contact,
                               guint8* photoData,
                               gsize photoDataLen,
                               gchar* photoContentType);

    /*!
     * @brief Main function of worker thread.
     */
    static void* downloadThreadFunc(void* ptr);

    /*!
     * @brief Helper function that checks if photos should be downloaded, based on list of ignored vCard fields.
     */
    bool downloadPhoto();

    //User credentials
    std::string       userLogin;
    OpenAB::SecureString userPassword;

    std::string       clientId;
    OpenAB::SecureString clientSecret;
    OpenAB::SecureString refreshToken;

    // list of vCard fields to be ignored
    std::vector<std::string> ignoredFields;

    // current transfer status, updated by downloadThreadFunc
    OpenAB_Source::Source::eGetItemRet transferStatus;

    //GData specific members
    GDataAuthorizer* authorizer;
    GDataContactsService *service;

    GCancellable *cancellable;


    unsigned int totalNumber;
    //buffer of already converted vCards
    std::list<std::string> bufferedVCards;

    pthread_t       downloadThread;
    bool            threadCreated;
    bool            paused;
    bool            cancelled;
    pthread_mutex_t mutex;
    pthread_cond_t  bufferReadyCond;
};

#endif // GOOGLE_H_
