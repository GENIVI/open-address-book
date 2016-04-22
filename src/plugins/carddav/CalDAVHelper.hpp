/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file CalDAVHelper.hpp
 */

#ifndef CALDAVHELPER_HPP_
#define CALDAVHELPER_HPP_

#include "helpers/Http.hpp"
#include "DAVHelper.hpp"
#include "PIMItem/PIMItem.hpp"

/*!
 * @brief Documentation for class CalDAVHelper.
 * This class implements CalDAV client using libcurl.
 */
class CalDAVHelper
{
  public:
    /*!
     *  @brief Constructor.
     *  @param [in] serverUrl address of CalDAV server or direct address of calendar to be used.
     *  @param [in] isCalendarUrl indicates if url provided as serverUrl is calendar url or server url.
     *  @param [in] httpSession CURL http session object to be used for HTTP requests.
     *  @param [in] httpAuthorizer initialized httpAuthorizer required by given server.
     */
    CalDAVHelper(const std::string& serverUrl,
                 bool isCalendarUrl,
                 OpenAB::HttpSession* httpSession,
                 OpenAB::HttpAuthorizer* httpAuthorizer);

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~CalDAVHelper();

    /*!
     * @brief Queries principal url - url used by given user to send CalDAV requests.
     * @note If direct URL for calendar was not provided, it needs to be called before calling other functions from this class.
     * @return true if principal URL was found successfully, false otherwise.
     */
    bool findPrincipalUrl();

    /*!
     * @brief Queries address book set - url that can be further queried about user calendars details..
     * @note: If direct URL for calendar was not provided, it should be called after @ref findPrincipalUrl and before @ref findCalendars.
     * @return true if calendar home set was found successfully, false otherwise.
     */
    bool findCalendarHomeSet();

    /*!
     * @brief Queries calendars information.
     * Queried information about calendars can be obtained by @ref getCalendars, for each calendar information like display name and supported items type is queried.
     * @note: If direct URL for calendar was not provided, it should be called after @ref findAddressbookSet and before @ref queryCalendarMetadata.
     * @return true if calendars were found successfully, false otherwise.
     */
    bool findCalendars();

    struct CalendarInfo;
    /*!
     * @brief Queries calendar infromation.
     * In opposite to @ref findCalendars, this function queries detailed information about given calendar and return them directly to caller.
     * This function won't alter results of @ref getCalendars function.
     * @param [in] calendarURL url of calendar which details should be queried
     * @param [out] info CalendarInfo structure filled with calendar details.
     * @return true if calendar details were queried successfully, false otherwise.
     */
    bool queryCalendarInfo(const std::string& calendarURL,
                           CalendarInfo& info);

    /*!
     * @brief Queries calendar metadata (current revision and sync token).
     * After calling this function sync token can be obtained by using @ref getSyncToken().
     * @param [in] calendarURL calendar to be queried.
     * @return true if address book metadata was queried successfully, false otherwise.
     */
    bool queryCalendarMetadata(const std::string& calendarURL);

    /*!
     * @brief Queries events/tasks metadata (list of IDs and revisions).
     * After calling this function metadata can be obtained by using @ref getContactsMetadata() and @ref getTotalCount().
     * @param [in] calendarURL calendar to be queried.
     * @return true if contacts metadata was queried successfully, false otherwise.
     */
    bool queryEventsMetadata(const std::string& calendarURL);

    /*!
     * @brief Queries only metadata of events/tasks that were modified since provided sync token was created.
     * After calling this function metadata can be obtained by using @ref getEventsMetadata() and @ref getTotalCount().
     * @param [in] calendarURL calendar to be queried.
     * @param [in] syncToken last synchronization token
     * @param [out] removed list of removed items from server
     * @return true if metadata was queried successfully, false otherwise.
     */
    bool queryChangedEventsMetadata(const std::string& calendarURL,
                                    const std::string& syncToken,
                                    std::vector<OpenAB::PIMItem::ID>& removed);

    /*!
     * @brief Downloads iCalendar objects from metadata.
     * @param [in] calendarURL calendar to be used.
     * @param [in] offset offset of events/tasks in metadata.
     * @param [in] size number of events/tasks to be downloaded.
     * @param [out] icals downloaded iCalendar objects in the same order as in metadata.
     * @note before calling this either @ref queryEventsMetadata or @ref queryChangedEventsMetadata needs to be called
     * to populate metadata information.
     * This function is preferable way of downloading all contents off calendar in batches.
     * @return true if objects were downloaded successfully.
     */
    bool downloadEvents(const std::string& calendarURL,
                        unsigned int offset, unsigned int size,
                        std::vector<std::string>& icals);

    /*!
     * @brief Downloads iCalendar objects with given uris.
     * @param [in] calendarURL calendar to be used.
     * @param [in] uris list of iCalendar uri (from EventMetadata) to be downloaded.
     * @param [out] icals downloaded iCalendar list in the same order as provided uris.
     * @return true if objects were downloaded successfully.
     */
    bool downloadEvents(const std::string& calendarURL,
                        std::vector<std::string>& uris,
                        std::vector<std::string>& icals);

    /*!
     * @brief Creates new event/task.
     * @note Provided iCalendar object needs to have UID field set.
     * It is up to caller of this function to ensure that new object
     * UID is unique on the server (otherwise server will reject request).
     * @param [in] calendarURL calendar to be used.
     * @param [in] ical iCalendar object to be created.
     * @param [out] uri URI of newly created item.
     * @param [out] etag revision of newly created item.
     * @return true if item was created successfully, false otherwise
     * (e.g. provided iCalendar couldn't be parsed,
     *  it didn't contain UID or UID already exists on server).
     */
    bool addEvent(const std::string& calendarURL,
                  const std::string& ical,
                  std::string& uri,
                  std::string& etag);

    /*!
     * @brief Removes event/task.
     * @note: there is no need to provide calendar URL as it will be part of item URI
     * @param [in] uri id of item to be removed.
     * @param [in] etag revision of item to be removed, if revision of given item
     * does not match on the server, removal will fail
     * (not all servers supports that).
     * @return true if contact was removed successfully.
     */
    bool removeEvent(const std::string& uri,
                     const std::string& etag="");

    /*!
     * @brief Modifies event/task.
     * @note: there is no need to provide calendar URL as it will be part of item URI
     * @param [in] uri id of item to be updated.
     * @param [in] ical iCalendar object to be uploaded.
     * @param [out] etag new revision of updated item.
     * @return true if item was modified successfully.
     */
    bool modifyEvent(const std::string& uri,
                     const std::string& ical,
                     std::string& etag);

    /*!
     * @brief Returns total count of items in metadata downlaoded by @ref queryEventsMetadata or @ref queryChangedEventsMetadata.
     * @return total count of events/tasks in metadata.
     */
    unsigned int getTotalCount() const
    {
      return eventsMetadata.size();
    }

    /*!
     * @brief Returns sync token queried by @ref queryCalendarMetadata().
     * @return sync token
     */
    std::string getSyncToken() const
    {
      return calendarSyncToken;
    }

    /*!
     * @brief Simple struct describing items in metadata
     */
    typedef struct
    {
        /*Current ETAG (revision) of item*/
        std::string etag;
        /*Item Uri that uniquely identifies item on server*/
        std::string uri;
    } EventMetadata;

    typedef std::vector<EventMetadata> EventsMetadata;

    /*!
     * @brief Returns contacts metadata downlaoded by @ref queryContactsMetadata or @ref queryChangedEventsMetadata.
     * @note that calls to @ref queryContactsMetadata and @ref queryChangedEventsMetadata will overwrite results.
     * @return events/tasks metadata.
     */
    EventsMetadata getEventsMetadata() const
    {
      return eventsMetadata;
    }

    /*!
     * @enum Possible types of calendar items
     */
    enum CalendarItemTypes
    {
      EVENT = 0,  //< Event
      TODO,       //< Task
      JOURNAL     //< Journal/Memo
    };

    /*!
     * @brief Class describing calendar
     */
    class CalendarInfo
    {
      public:
        CalendarInfo();
        CalendarInfo(const std::string& url,
                     const std::string& name,
                     const std::vector<CalendarItemTypes>& types);
        ~CalendarInfo();

        /*!
         * @brief Returns URL of calendar.
         * @return URL of calendar
         */
        std::string getUrl() const;

        /*!
         * @brief Returns display name of calendar.
         * @return display name of calendar.
         */
        std::string getDisplayName() const;

        /*!
         * @brief Checks if calendar support given type of items.
         * @return true, if calendar supports given type, false otherwise.
         */
        bool supportsType(CalendarItemTypes type) const;

        /*!
         * @brief Returns list of all supported types.
         * @return list of all supported types.
         */
        std::vector<CalendarItemTypes> getSupportedCalendarTypes() const;

      private:
        std::string url;
        std::string displayName;

        std::vector<CalendarItemTypes> supportedTypes;
    };

    typedef std::vector<CalendarInfo> Calendars;

    /*!
     * @brief Returns list of available calendars.
     * @return list of available calendars.
     */
    Calendars getCalendars() const
    {
      return calendars;
    }

  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    CalDAVHelper(CalDAVHelper const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    CalDAVHelper& operator=(CalDAVHelper const &other);

    //CalDAV server url
    std::string       serverUrl;

    //Parsed part of server url containing only host address
    std::string       serverHostUrl;

    std::string       principalUrl;
    std::string       principalCalendarHomeSetUrl;
    std::string       principalCalendarSetHostUrl;
    std::string       principalCalendarUrl;

    //list of available calendars
    Calendars calendars;

    /*!
     * @brief Helper function that returns index of provided uri in list of provided uris.
     * @param [in] uris list of uris to be searched
     * @param [in] uri to be search for
     * @returns index of uri in uris, or -1 if uri was not found.
     */
    int getIndexFromUris(const std::vector<std::string>& uris, const std::string& uri);

    /*!
     * @brief Helper function that contains common logic for parsing
     * calendar information returned by server, used by
     * @ref findCalendars and @ref queryCalendarInfo functions.
     * @param [in] server response string for PROPFIND request with supported-calendar-component-set property.
     * @param [out] Calendars list to be filled with information from response.
     * return true if request was parsed successfully, false otherwise.
     */
    bool processCalendarsInfo(const std::string& davResponse,
                              Calendars& cals);

    DAVHelper            davHelper;
    OpenAB::HttpSession*    httpSession;
    OpenAB::HttpAuthorizer* httpAuthorizer;

    EventsMetadata eventsMetadata;
    std::string calendarCTag;
    std::string calendarSyncToken;

    std::string userAgent;
};

#endif // CALDAVHELPER_HPP_
