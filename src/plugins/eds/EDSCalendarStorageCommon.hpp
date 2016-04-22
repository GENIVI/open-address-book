/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file EDSCalendarStorageCommon.hpp
 */

#ifndef EDSCALENDARSTORAGECOMMON_HPP_
#define EDSCALENDARSTORAGECOMMON_HPP_

#include <PIMItem/Calendar/PIMCalendarItem.hpp>
#include <string>
#include <libical/ical.h>
/*!
 * @brief Documentation for class EDSCalendarStorageCommon
 */
class EDSCalendarStorageCommon
{
  public:
    /*!
     * @biref Cuts VEVENT or VTODO part out of VCALENDARs stream
     * @param [in] stream stream from which VEVENT or VTODO should be cut
     * @return next VEVENT or VTODO string or empty string if these components were not found in stream
     */
    static std::string cutVObject(std::istream& stream);

    /*!
     * @brief Processes VEVENT or VTODO components obtained from EDS
     * Adds VTIMEZONE component and creates new VCALENDAR object
     * In case of recurring events all instances are merged to one VCALENDAR object
     * @param [in] newEvents icalcomponent with instances of given event.
     * @return OpenAB::PIMCalendarItem or NULL if components couldn't be processed.
     */
    static OpenAB::PIMCalendarItem* processObject(const std::vector<icalcomponent*>& newEvents);

    /*!
     * @brief Processes VEVENT or VTODO components obtained from EDS
     * Adds VTIMEZONE component and creates new VCALENDAR object
     * In case of recurring events all instances are merged to one VCALENDAR object
     * @param [in] iCals iCalendar objects with instances of given event
     * @return OpenAB::PIMCalendarItem or NULL if component couldn't be processed.
     */
    static OpenAB::PIMCalendarItem* processObject(const std::vector<std::string>& iCals);

  private:
    /*!
     *  @brief Constructor.
     */
    EDSCalendarStorageCommon();

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~EDSCalendarStorageCommon();

    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    EDSCalendarStorageCommon(EDSCalendarStorageCommon const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    EDSCalendarStorageCommon& operator=(EDSCalendarStorageCommon const &other);

    /*!
     * @brief Callback function for libecal that will be called
     * for each timezone found in VEVENT or VTODO component
     */
    static void findTimeZonesCb(icalparameter* param, void* data);

    static std::set<std::string>       currentEventTimeZones;
};

#endif // EDSCALENDARSTORAGECOMMON_HPP_
