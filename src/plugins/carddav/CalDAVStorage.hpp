/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file CalDAVStorage.hpp
 */

#ifndef CARDDAV_H_
#define CARDDAV_H_

#include <plugin/storage/CalendarStorage.hpp>
#include <list>
#include "helpers/Http.hpp"
#include "CalDAVHelper.hpp"

class CalDAVStorageItemIterator;

/**
 * @defgroup CalDAVStorage CalDAV Storage Plugin
 * @ingroup StoragePlugin
 *
 * @brief Provides OpenAB::PIMCalendarItem items using CalDAV protocol (tested with Google Contacts, iCloud).
 *
 * Plugin Name: "CardDAV"
 *
 * Parameters:
 * | Type       | Name  | Description                                 | Mandatory |
 * |:-----------|:      |:--------------------------------------------| :         |
 * | String  | "server_url"    | CalDAV server address                      | No, if calendar_url is provided       |
 * | String  | "calendar_url"  | CalDAV calendar address                    | No, if server_url is provided |
 * | String  | "calendar_name" | CalDAV calendar name                       | No |
 * | Integer | "item_type"     | Type of items - OpenAB::eEvent or OpenAB::eTask (set to integer equal to OpenAB::PIMItemType enum values) | Yes |
 * | String  | "login"         | User login                                  | Yes        |
 * | String  | "password"      | User password                               | Yes        |
 * | String  | "client_id"     | Id of client application (registered in Google)     | Yes       |
 * | String  | "client_secret" | Secret of client application (registered in Google) | Yes       |
 * | String  | "refresh_token" | OAuth2 user refresh token                           | Yes       |
 *
 * CalDAV can support multiple calendars for single account, when only "server_url" is provided,
 * first found calendar will be used.
 * Alternatively "calendar_name" can be provided to select first matching calendar of
 * given name (some services e.g. Google allows to create multiple calendars with the
 * same name). Last option is to provide direct address of calendar to use using
 * "calendar_url" (in that case "serverl_url" parameter is not mandatory).
 *
 *@note "login" and "password" pair or
 *  triple "client_id", "client_secret" and "refresh_token"
 *  are mandatory to provide, depending on authorization method used by CardDAV server.
 *  When using OAuth2 refresh token it has to have required scope (for Google calendar this is https://www.googleapis.com/auth/calendar).
 *
 */
class CalDAVStorage : public OpenAB_Storage::CalendarStorage
{
  public:
    /*!
     *  @brief Constructor.
     *  @param[in] url server url
     *  @param[in] login user login
     *  @param[in] password user password
     *  @param[in] calendarURL direct calendar url
     *  @param[in] calendarName optional name of calendar to be used
     *  @param[in] type type of items to use (either OpenAB::eEvent or OpenAB::eTask)
     */
    CalDAVStorage(const std::string& url,
                  const std::string& login,
                  const OpenAB::SecureString& password,
                  const std::string& calendarURL,
                  const std::string& calendarName,
                  OpenAB::PIMItemType type);

    /*!
     *  @brief Constructor.
     *  @param[in] url server url
     *  @param[in] clientId OAuth2 application id
     *  @param[in] clientSecret OAuth2 application secret
     *  @param[in] refreshToken OAuth2 user refresh token
     *  @param[in] calendarURL direct calendar url
     *  @param[in] calendarName optional name of calendar to be used
     *  @param[in] type type of items to use (either OpenAB::eEvent or OpenAB::eTask)
     */
    CalDAVStorage(const std::string& url,
                  const std::string& clientId,
                  const OpenAB::SecureString& clientSecret,
                  const OpenAB::SecureString& refreshToken,
                  const std::string& calendarURL,
                  const std::string& calendarName,
                  OpenAB::PIMItemType type);

    virtual ~CalDAVStorage();

    enum OpenAB_Source::Source::eInit init();

    enum OpenAB_Source::Source::eGetItemRet getItem(OpenAB::SmartPtr<OpenAB::PIMItem> & item);

    enum OpenAB_Source::Source::eSuspendRet suspend();

    enum OpenAB_Source::Source::eResumeRet resume();

    enum OpenAB_Source::Source::eCancelRet cancel();

    int getTotalCount() const;

    enum eAddItem addObject( const std::string& iCal,
                             OpenAB::PIMItem::ID& newId,
                             OpenAB::PIMItem::Revision& revision);

    enum eAddItem addObjects( const std::vector<std::string> &iCals,
                              OpenAB::PIMItem::IDs& newIds,
                              OpenAB::PIMItem::Revisions& revisions);

    enum eModifyItem modifyObject( const std::string& iCal,
                                   const OpenAB::PIMItem::ID& id,
                                   OpenAB::PIMItem::Revision& revision);

    enum eModifyItem modifyObjects( const std::vector<std::string> &iCals,
                                    const OpenAB::PIMItem::IDs& ids,
                                    OpenAB::PIMItem::Revisions& revisions);

    enum eRemoveItem removeObject( const OpenAB::PIMItem::ID& id);

    enum eRemoveItem removeObjects( const OpenAB::PIMItem::IDs& ids);

    enum eGetItem getEvent (const OpenAB::PIMItem::ID & id,
                            OpenAB::SmartPtr<OpenAB::PIMCalendarEventItem> & item);

    enum eGetItem getEvents(const OpenAB::PIMItem::IDs & ids,
                            std::vector<OpenAB::SmartPtr<OpenAB::PIMCalendarEventItem> > & items);

    enum eGetItem getTask (const OpenAB::PIMItem::ID & id,
                           OpenAB::SmartPtr<OpenAB::PIMCalendarTaskItem> & item);

    enum eGetItem getTasks(const OpenAB::PIMItem::IDs & ids,
                           std::vector<OpenAB::SmartPtr<OpenAB::PIMCalendarTaskItem> > & items);


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
    CalDAVStorage(CalDAVStorage const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    CalDAVStorage& operator=(CalDAVStorage const &other);

    void construct();

    void cleanup();

    bool selectFirstCalendar(const CalDAVHelper::Calendars cals,
                             CalDAVHelper::CalendarInfo& selectedCalendar);

    std::string       serverUrl;
    std::string       calendarUrl;
    std::string       calendarName;

    std::string       userLogin;
    OpenAB::SecureString userPassword;

    std::string       clientId;
    OpenAB::SecureString clientSecret;
    OpenAB::SecureString refreshToken;

    std::string syncToken;

    CalDAVHelper::CalendarInfo calendarInfo;

    OpenAB::HttpSession curlSession;
    OpenAB::HttpAuthorizer* authorizer;
    CalDAVHelper* calDavHelper;
    CalDAVStorageItemIterator* sourceIterator;
};

class CalDAVStorageItemIterator : public OpenAB_Storage::StorageItemIterator
{
  public:
    /*!
     *  @brief Constructor.
     */
    CalDAVStorageItemIterator();

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~CalDAVStorageItemIterator();

    enum eCursorInit{
      eCursorInitOK,
      eCursorInitFail
    };
    enum eCursorInit cursorInit(CalDAVHelper* calDAVHelper,
                                const std::string& calendarURL,
                                OpenAB::PIMItemType type);

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
    CalDAVStorageItemIterator(CalDAVStorageItemIterator const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    CalDAVStorageItemIterator& operator=(CalDAVStorageItemIterator const &other);

    enum eFetchEvents {
      eFetchEventsOK,
      eFetchEventsEND,
      eFetchEventsFail
    };
    enum eFetchEvents fetchEvents(int fetchsize);

    static void* downloadThreadFunc(void* ptr);
    bool downloadICals(unsigned int offset, unsigned int size);

    OpenAB_Storage::StorageItem    elem;
    CalDAVHelper*               calDavHelper;
    std::string                 calendarURL;

    unsigned                    total;
    unsigned int                offset;
    unsigned int                offsetOfCachedICals;

    std::list<OpenAB::SmartPtr<OpenAB::PIMCalendarItem> >  cachedEvents;
    std::vector<CalDAVHelper::EventMetadata> eventsMetadata;

    bool                 paused;
    bool                 cancelled;
    bool                 threadCreated;
    pthread_t            downloadThread;
    pthread_mutex_t      mutex;
    pthread_cond_t       bufferReadyCond;
    OpenAB_Source::Source::eGetItemRet transferStatus;
    OpenAB::PIMItemType     itemsType;
};

#endif // CARDDAV_H_
