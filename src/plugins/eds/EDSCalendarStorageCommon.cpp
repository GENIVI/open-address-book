/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file EDSCalendarStorageCommon.cpp
 */

#include <helpers/StringHelper.hpp>
#include "EDSCalendarStorageCommon.hpp"

std::set<std::string> EDSCalendarStorageCommon::currentEventTimeZones;

EDSCalendarStorageCommon::EDSCalendarStorageCommon()
{
}

EDSCalendarStorageCommon::~EDSCalendarStorageCommon()
{
}

std::string EDSCalendarStorageCommon::cutVObject(std::istream& stream)
{
  std::string line;
  std::string event;
  bool inEvent = false;
  bool inTodo = false;
  while (std::getline(stream, line))
  {
    OpenAB::trimWhitespaces(line);

    if (line == "BEGIN:VEVENT" && !inEvent && !inTodo)
    {
      inEvent = true;
    }
    if (line == "BEGIN:VTODO" && !inEvent && !inTodo)
    {
      inTodo = true;
    }

    if (inEvent || inTodo)
    {
      event += line;
      event += "\n";

      if (inEvent && line == "END:VEVENT")
      {
        inEvent = false;
        break;
      }
      if (inTodo && line == "END:VTODO")
      {
        inTodo = false;
        break;
      }
    }
  }

  return event;
}

void EDSCalendarStorageCommon::findTimeZonesCb(icalparameter* param, void* data)
{
  if (NULL == data)
  {
    return;
  }

  //get time zone id and insert it in set of timezones found in current component
  const char* value = icalparameter_get_tzid(param);
  currentEventTimeZones.insert(value);
}

OpenAB::PIMCalendarItem* EDSCalendarStorageCommon::processObject(const std::vector<std::string>& iCals)
{
  //Create icalcomponent from provided iCalendar strings
  std::vector<icalcomponent*> iCalComponents;
  std::vector<std::string>::const_iterator it;
  for (it = iCals.begin(); it != iCals.end(); ++it)
  {
    LOG_DEBUG()<<"Processing objects "<<(*it)<<std::endl;
    icalcomponent* newEvent = icalcomponent_new_from_string((*it).c_str());
    if (!newEvent)
    {
      LOG_DEBUG()<<"Cannot parse vevent"<<std::endl;
      std::vector<icalcomponent*>::iterator it2;
      for (it2 = iCalComponents.begin(); it2 != iCalComponents.end(); ++it2)
      {
        icalcomponent_free((*it2));
        (*it2) = NULL;
      }
      return NULL;
    }
    iCalComponents.push_back(newEvent);
  }

  //process icalcomponent object
  return processObject(iCalComponents);
}

OpenAB::PIMCalendarItem* EDSCalendarStorageCommon::processObject(const std::vector<icalcomponent*>& newEvents)
{
  LOG_FUNC();
  OpenAB::PIMItemType type = OpenAB::eEvent;
  currentEventTimeZones.clear();

  icalcomponent* calendarComponent = icalcomponent_new_vcalendar();
  icalcomponent_add_property(calendarComponent, icalproperty_new_version("2.0"));

  std::string uid;
  std::string revision;

  std::vector<icalcomponent*>::const_iterator it;
  for (it = newEvents.begin(); it != newEvents.end(); ++it)
  {
    //remove error fields that libical inserts
    icalcomponent_strip_errors((*it));

    //get uid and revision
    if (uid.empty())
    {
      uid = icalcomponent_get_uid((*it));
    }
    revision += icaltime_as_ical_string(icalcomponent_get_dtstamp((*it)));

    //find all references to timezones
    icalcomponent_foreach_tzid((*it), EDSCalendarStorageCommon::findTimeZonesCb, (*it));

    icalcomponent_add_component(calendarComponent, (*it));

    //check type of object
    if (icalcomponent_isa((*it)) == ICAL_VEVENT_COMPONENT)
    {
      type = OpenAB::eEvent;
    }
    else if (icalcomponent_isa((*it)) == ICAL_VTODO_COMPONENT)
    {
      type = OpenAB::eTask;
    }
  }

  //create VTIMEZONE component with definitions of all
  //timezones that are referenced in processed component
  std::set<std::string>::iterator it2;
  for(it2 = currentEventTimeZones.begin(); it2 != currentEventTimeZones.end(); ++it2)
  {
    icaltimezone* tz = icaltimezone_get_builtin_timezone((*it2).c_str());

    if (tz)
    {
      icalcomponent* vtimezone = icaltimezone_get_component(tz);
      icalproperty* tzprop = icalcomponent_get_first_property(vtimezone, ICAL_TZID_PROPERTY);
      icalproperty_set_value_from_string(tzprop, (*it2).c_str(), "NO");
      icalcomponent_add_component(calendarComponent, vtimezone);
      icaltimezone_free_builtin_timezones();
    }
  }

  //convert icalcomponent back to iCalendar string
  std::string iCal = icalcomponent_as_ical_string(calendarComponent);
  OpenAB::PIMCalendarItem* newItem;

  //create new PIMCalendarItem based on new iCalendar string
  if (OpenAB::eEvent == type)
  {
    newItem = new OpenAB::PIMCalendarEventItem();
  }
  else if (OpenAB::eTask == type)
  {
    newItem = new OpenAB::PIMCalendarTaskItem();
  }

  icalcomponent_free(calendarComponent);

  newItem->parse(iCal);
  newItem->setId(uid);
  newItem->setRevision(revision);

  return newItem;
}
