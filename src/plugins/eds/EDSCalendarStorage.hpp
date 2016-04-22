/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file EDSCalendarStorage.hpp
 */

#include <plugin/storage/CalendarStorage.hpp>
#include "EDSCalendarStorageItemIterator.hpp"
#include <string>
#include <fstream>
#include <set>

#ifndef OpenAB_PLUGIN_EDS_CALENDAR_HPP_
#define OPENAB_PLUGIN_EDS_CALENDAR_HPP_

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
 * | Type | Name     | Description                 | Mandatory |
 * |:-----|:     ----|:----------------------------|:          |
 * |String | "db"      | EDS source name             | Yes       |
 * |Integer | "item_type"| Type of items - OpenAB::eEvent or OpenAB::eTask (set to integer equal to OpenAB::PIMItemType enum values) | Yes |
 *
 */

class EDSCalendarStorage : public OpenAB_Storage::CalendarStorage
{
  public:
    /*!
     * @brief Constructor
     * @param [in] sourceName name of EDS's source to use
     * @param [in] type type of items to use (either OpenAB::eEvent or OpenAB::eTask)
     */
    EDSCalendarStorage(const std::string& sourceName,
                       OpenAB::PIMItemType type);

    ~EDSCalendarStorage();

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

    enum OpenAB_Storage::Storage::eGetSyncToken getLatestSyncToken(std::string&);

    OpenAB_Storage::StorageItemIterator* newStorageItemIterator();

  private:
    /*!
     * @brief Get revision of object with given id
     * @param [in] id id of object
     * @return revision of object or empty string if object with given id was not found.
     */
    OpenAB::PIMItem::Revision getRevision(const OpenAB::PIMItem::ID& id);

    /*!
     * @brief Get revisions of objects with given ids
     * @param [in] ids ids of object
     * @return revisions of objects or empty string if object with given id was not found.
     */
    OpenAB::PIMItem::Revisions getRevisions(const OpenAB::PIMItem::IDs& ids);

    /*!
     * @brief Converts provided list of iCals object into list of icalcomponent objects.
     * For recurring events it creates multiple icalcomponents for each instance
     *  (each of them has the same UID but different RECURRENCE-ID)
     * @param [in] iCals list of iCalendar strings
     * @param [in] ids optional list of ids, in case where UID in provided iCals will not match UID from ids it will be substituted.
     * @return list of icalcomponents objects
     */
    GSList* toICalComponentsList(const std::vector<std::string> & iCals,
                                 const OpenAB::PIMItem::IDs& ids,
                                 std::vector<icaltimezone*>& timezones);

    std::string       database;
    ESourceRegistry * registry;
    ESource         * source;
    ECalClient      * client;
    EDSCalendarStorageItemIterator* sourceIterator;
    std::string       databaseFileName;
    std::ifstream     databaseFile;

    static void findTimeZonesCb(icalparameter* param, void* data);

    static std::set<std::string>       currentEventTimeZones;
};
#endif /* OpenAB_PLUGIN_EDS_CALENDAR_HPP_ */
