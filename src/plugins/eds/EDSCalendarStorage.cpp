/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file EDSCalendarStorage.cpp
 */

#include "OpenAB_eds_global.h"
#include "EDSCalendarStorage.hpp"
#include "EDSCalendarStorageCommon.hpp"
#include <helpers/StringHelper.hpp>
#include <string.h>
#include <sstream>

std::set<std::string> EDSCalendarStorage::currentEventTimeZones;

EDSCalendarStorage::EDSCalendarStorage(const std::string & db,
                                       OpenAB::PIMItemType type)
    : OpenAB_Storage::CalendarStorage(type),
      database(db),
      registry(NULL),
      source(NULL),
      client(NULL),
      sourceIterator(NULL)
{
  LOG_FUNC();
}

EDSCalendarStorage::~EDSCalendarStorage()
{
  LOG_FUNC();
  if (NULL != sourceIterator)
  {
    delete sourceIterator;
  }

  databaseFile.close();

  if (NULL != client)
    g_object_unref(client);
  if (NULL != source)
    g_object_unref(source);
  if (NULL != registry)
    g_object_unref(registry);
}

enum OpenAB_Storage::Storage::eInit EDSCalendarStorage::init()
{
  LOG_FUNC() << "DB: " << database<<std::endl;

  GError *gerror = NULL;

  // 1. Get access to all databases in EDS.
  registry = e_source_registry_new_sync(NULL, &gerror);
  if (!registry)
  {
    LOG_ERROR() << "e_source_registry not found " << GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    return eInitFail;
  }

  // 2. Look up one particular database.
  // special use case - "system" db - use system storage
  std::string dbName = database;
  if (dbName == "system")
  {
    if (OpenAB::eEvent == getItemType())
    {
      source = e_source_registry_ref_default_calendar(registry);
    }
    else if (OpenAB::eTask == getItemType())
    {
      source = e_source_registry_ref_default_task_list(registry);
    }
  }
  else
  {
    source = e_source_registry_ref_source(registry, dbName.c_str());
  }

  if (!source)
  {
    LOG_ERROR() << "e_source not found"<<std::endl;
    GERROR_FREE(gerror);
    return eInitFail;
  }

  GERROR_FREE(gerror);

  const gchar *userDataDir = NULL;
  userDataDir = e_get_user_data_dir();
  gchar* dirname = NULL;

  if (OpenAB::eEvent == getItemType())
  {
    client = (ECalClient *) e_cal_client_connect_sync(source, E_CAL_CLIENT_SOURCE_TYPE_EVENTS, NULL, &gerror);
    dirname = g_build_filename(userDataDir, "calendar", dbName.c_str(), "calendar.ics", NULL);

  }
  else
  {
    client = (ECalClient *) e_cal_client_connect_sync(source, E_CAL_CLIENT_SOURCE_TYPE_TASKS, NULL, &gerror);
    dirname = g_build_filename(userDataDir, "tasks", dbName.c_str(), "tasks.ics", NULL);
  }

  databaseFileName = dirname;
  g_free(dirname);

  if (gerror)
  {
    LOG_ERROR() << "Error e_cal_client_connect_sync results: " << GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    return eInitFail;
  }
  LOG_VERBOSE() << "e_cal_client_connect_sync\n"<<std::endl;

  return eInitOk;
}

enum OpenAB_Source::Source::eGetItemRet EDSCalendarStorage::getItem(OpenAB::SmartPtr<OpenAB::PIMItem> & item)
{
  if (NULL == sourceIterator)
  {
    sourceIterator = (EDSCalendarStorageItemIterator*)newStorageItemIterator();
    if (NULL == sourceIterator)
    {
      return eGetItemRetError;
    }
  }

  OpenAB_Storage::StorageItem* it = sourceIterator->next();
  if (it == NULL)
  {
    delete sourceIterator;
    sourceIterator = NULL;
    return eGetItemRetEnd;
  }

  item = it->item;

  return eGetItemRetOk;
}

enum OpenAB_Source::Source::eSuspendRet EDSCalendarStorage::suspend()
{
  return eSuspendRetNotSupported;
}

enum OpenAB_Source::Source::eResumeRet EDSCalendarStorage::resume()
{
  return eResumeRetNotSupported;
}

enum OpenAB_Source::Source::eCancelRet EDSCalendarStorage::cancel()
{
  return eCancelRetNotSupported;
}

int EDSCalendarStorage::getTotalCount() const
{
  /*if (contactsIterator)
    return contactsIterator->getSize();
*/
  return 0;
}

void EDSCalendarStorage::findTimeZonesCb(icalparameter* param, void* data)
{
  if (NULL == data)
  {
    return;
  }

  //get time zone id and insert it in set of timezones found in current component
  const char* value = icalparameter_get_tzid(param);
  currentEventTimeZones.insert(value);
}

GSList* EDSCalendarStorage::toICalComponentsList(const std::vector<std::string> & iCals,
                                                 const OpenAB::PIMItem::IDs& ids,
                                                 std::vector<icaltimezone*>& timezones)
{
  GSList * events = NULL;

  currentEventTimeZones.clear();
  for(unsigned int i = 0; i < iCals.size(); ++i)
  {
    std::stringstream ss(iCals[i]);

    //In case of recurring events one iCalendar can contain couple of VEVENT objects, get them all
    while (1)
    {
      std::string event = EDSCalendarStorageCommon::cutVObject(ss);

      if (event.empty())
      {
        break;
      }

      icalcomponent* newEvent = icalcomponent_new_from_string(event.c_str());
      if (!newEvent)
      {
        LOG_DEBUG() << "Cannot parse vevent" << std::endl;
        g_slist_free_full(events, (GDestroyNotify) icalcomponent_free);
        return NULL;
      }

      icalcomponent_foreach_tzid(newEvent, EDSCalendarStorage::findTimeZonesCb, newEvent);
      icalcomponent_strip_errors(newEvent);
      if (ids.size() > i)
        icalcomponent_set_uid(newEvent, ids[i].c_str());

      events = g_slist_append(events, newEvent);
    }
  }

  std::set<std::string>::iterator it;
  for(it = currentEventTimeZones.begin(); it != currentEventTimeZones.end(); ++it)
  {
    icaltimezone* tz = icaltimezone_get_builtin_timezone((*it).c_str());
    if (tz)
    {
      timezones.push_back(tz);
    }
  }

  return events;
}

enum OpenAB_Storage::Storage::eAddItem EDSCalendarStorage::addObject(const std::string & iCal,
                                                                  OpenAB::PIMItem::ID & newId,
                                                                  OpenAB::PIMItem::Revision & revision)
{
  std::vector<std::string> iCals;
  iCals.push_back(iCal);

  OpenAB::PIMItem::IDs newIds;
  OpenAB::PIMItem::Revisions revisions;

  if (eAddItemOk != addObjects(iCals, newIds, revisions))
  {
    return eAddItemFail;
  }

  if (newIds.size() != 1 || revision.size() != 1)
  {
    return eAddItemFail;
  }

  newId = newIds.front();
  revision = revisions.front();

  return eAddItemOk;
}

enum OpenAB_Storage::Storage::eAddItem EDSCalendarStorage::addObjects(const std::vector<std::string> & iCals,
                                                                   OpenAB::PIMItem::IDs & newIds,
                                                                   OpenAB::PIMItem::Revisions & revisions)
{
  GError *gerror = NULL;
  GSList * events = NULL;
  GSList * new_uids;

  OpenAB::PIMItem::IDs emptyIds;
  std::vector<icaltimezone*> timezones;
  events = toICalComponentsList(iCals, emptyIds, timezones);

  if (NULL == events)
  {
    LOG_DEBUG() << "Cannot parse vevent" << std::endl;
    return eAddItemFail;
  }

  std::vector<icaltimezone*>::iterator it;
  for (it = timezones.begin(); it != timezones.end(); ++it)
  {
    e_cal_client_add_timezone_sync (client, (*it), NULL, &gerror);
    GERROR_FREE(gerror);
  }

  if (!e_cal_client_create_objects_sync (client, events, &new_uids, NULL, &gerror))
  {
    LOG_ERROR() << "Error e_cal_client_create_object_sync results: " << GERROR_MESSAGE(gerror)<<std::endl;
    g_slist_free_full(events, (GDestroyNotify) icalcomponent_free);
    GERROR_FREE(gerror);
    return eAddItemFail;
  }



  GSList *l = new_uids;
  newIds.clear();
  while(l)
  {
    //check if we've added ocurring event in that case do not duplicate UID
    std::string id = (const gchar*)l->data;
    if (!OpenAB::contains(newIds, id))
    {
      newIds.push_back(id);
      LOG_DEBUG()<<"UID "<<id<<std::endl;
    }

    l = g_slist_next(l);
  }

  g_slist_free_full(events, (GDestroyNotify) icalcomponent_free);
  e_client_util_free_string_slist(new_uids);

  revisions = getRevisions(newIds);

  return eAddItemOk;
}

enum OpenAB_Storage::Storage::eModifyItem EDSCalendarStorage::modifyObject(const std::string & iCal,
                                                                        const OpenAB::PIMItem::ID & id,
                                                                        OpenAB::PIMItem::Revision & revision)
{
  std::vector<std::string> iCals;
  iCals.push_back(iCal);

  OpenAB::PIMItem::IDs ids;
  ids.push_back(id);

  OpenAB::PIMItem::Revisions revisions;

  if (eModifyItemOk != modifyObjects(iCals, ids, revisions))
  {
    return eModifyItemFail;
  }

  if (revision.size() != 1)
  {
    return eModifyItemFail;
  }

  revision = revisions.front();
  return eModifyItemOk;
}

enum OpenAB_Storage::Storage::eModifyItem EDSCalendarStorage::modifyObjects(const std::vector<std::string> & iCals,
                                                                         const OpenAB::PIMItem::IDs & ids,
                                                                         OpenAB::PIMItem::Revisions & revisions)
{
  GError *gerror = NULL;
  GSList * events = NULL;

  std::vector<icaltimezone*> timezones;
  events = toICalComponentsList(iCals, ids, timezones);

  if (NULL == events)
  {
    LOG_DEBUG() << "Cannot parse vevent" << std::endl;
    return eModifyItemFail;
  }

  std::vector<icaltimezone*>::iterator it;
  for (it = timezones.begin(); it != timezones.end(); ++it)
  {
    e_cal_client_add_timezone_sync (client, (*it), NULL, &gerror);
    GERROR_FREE(gerror);
  }

  if (!e_cal_client_modify_objects_sync (client, events, E_CAL_OBJ_MOD_THIS, NULL, &gerror))
  {
    LOG_ERROR() << "Error e_cal_client_modify_objects_sync results: " << GERROR_MESSAGE(gerror)<<std::endl;
    g_slist_free_full(events, (GDestroyNotify) icalcomponent_free);
    GERROR_FREE(gerror);
    return eModifyItemFail;
  }

  g_slist_free_full(events, (GDestroyNotify) icalcomponent_free);

  revisions = getRevisions(ids);
  return eModifyItemOk;
}

enum OpenAB_Storage::Storage::eRemoveItem EDSCalendarStorage::removeObject(const OpenAB::PIMItem::ID & id)
{
  GError *gerror = NULL;
  if (!e_cal_client_remove_object_sync(client, id.c_str(), NULL, E_CAL_OBJ_MOD_ALL, NULL, &gerror))
  {
    LOG_ERROR() << "Error e_cal_client_remove_object_sync results: " << GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    return eRemoveItemFail;
  }
  GERROR_FREE(gerror);
  return eRemoveItemOk;

  return eRemoveItemFail;
}

enum OpenAB_Storage::Storage::eRemoveItem EDSCalendarStorage::removeObjects(const OpenAB::PIMItem::IDs & ids)
{
  GError *gerror = NULL;
  GSList *events_ids = NULL;

  for(unsigned int i = 0; i < ids.size(); ++i)
  {
    ECalComponentId *componentId = e_cal_component_id_new (ids[i].c_str(), NULL);
    events_ids = g_slist_append(events_ids, (void*)componentId);
  }

  if (!e_cal_client_remove_objects_sync(client, events_ids, E_CAL_OBJ_MOD_ALL, NULL, &gerror))
  {
    LOG_ERROR() << "Error e_cal_client_remove_objects_sync results: " << GERROR_MESSAGE(gerror)<<std::endl;
    GERROR_FREE(gerror);
    g_slist_free_full(events_ids, (GDestroyNotify) e_cal_component_free_id );
    return eRemoveItemFail;
  }

  g_slist_free_full(events_ids, (GDestroyNotify) e_cal_component_free_id );

  return eRemoveItemOk;
}

enum OpenAB_Storage::Storage::eGetItem EDSCalendarStorage::getEvent(const OpenAB::PIMItem::ID & id,
                                                                 OpenAB::SmartPtr<OpenAB::PIMCalendarEventItem> & item)
{
  if (getItemType() != OpenAB::eEvent)
  {
    return eGetItemFail;
  }

  GError* gerror = NULL;
  GSList* events = NULL;
  std::vector<icalcomponent*> iCalComponents;

  if (!e_cal_client_get_objects_for_uid_sync(client, id.c_str(), &events, NULL, &gerror))
  {
    LOG_ERROR() << "Error e_cal_client_get_objects_for_uid_sync result: " << GERROR_MESSAGE(gerror) << std::endl;
    GERROR_FREE(gerror);
    return eGetItemFail;
  }

  GSList* current = events;
  while(current)
  {
    ECalComponent* component = (ECalComponent*)current->data;
    iCalComponents.push_back(icalcomponent_new_clone(e_cal_component_get_icalcomponent(component)));
    current = current->next;
  }

  item = (OpenAB::PIMCalendarEventItem*)EDSCalendarStorageCommon::processObject(iCalComponents);

  e_cal_client_free_ecalcomp_slist(events);
  return eGetItemOk;
}

enum OpenAB_Storage::Storage::eGetItem EDSCalendarStorage::getEvents(const OpenAB::PIMItem::IDs & ids,
                                                                  std::vector<OpenAB::SmartPtr<OpenAB::PIMCalendarEventItem> > & items)
{
  for (unsigned int i = 0; i < ids.size(); ++i)
  {
    OpenAB::SmartPtr<OpenAB::PIMCalendarEventItem> item;
    if (eGetItemOk != getEvent(ids[i], item))
    {
      return eGetItemFail;
    }
    items.push_back(item);
  }

  return eGetItemOk;
}

enum OpenAB_Storage::Storage::eGetItem EDSCalendarStorage::getTask(const OpenAB::PIMItem::ID & id,
                                                                OpenAB::SmartPtr<OpenAB::PIMCalendarTaskItem> & item)
{
  if (getItemType() != OpenAB::eTask)
  {
    return eGetItemFail;
  }
  GError* gerror = NULL;

  std::vector<icalcomponent*> events;
  icalcomponent* event = NULL;
  if (!e_cal_client_get_object_sync(client, id.c_str(), NULL, &event, NULL, &gerror))
  {
    LOG_ERROR() << "Error e_cal_client_get_object_sync result: " << GERROR_MESSAGE(gerror) << std::endl;
    GERROR_FREE(gerror);
    return eGetItemFail;
  }

  events.push_back(event);

  item = (OpenAB::PIMCalendarTaskItem*)EDSCalendarStorageCommon::processObject(events);

  return eGetItemOk;
}

enum OpenAB_Storage::Storage::eGetItem EDSCalendarStorage::getTasks(const OpenAB::PIMItem::IDs & ids,
                                                                  std::vector<OpenAB::SmartPtr<OpenAB::PIMCalendarTaskItem> > & items)
{
  for (unsigned int i = 0; i < ids.size(); ++i)
  {
    OpenAB::SmartPtr<OpenAB::PIMCalendarTaskItem> item;
    if (eGetItemOk != getTask(ids[i], item))
    {
      return eGetItemFail;
    }
    items.push_back(item);
  }

  return eGetItemOk;
}

OpenAB::PIMItem::Revision EDSCalendarStorage::getRevision(const OpenAB::PIMItem::ID& id)
{
  OpenAB::PIMItem::Revision revision;
  GError* gerror = NULL;
  GSList* events = NULL;
  std::vector<icalcomponent*> iCalComponents;

  if (!e_cal_client_get_objects_for_uid_sync(client, id.c_str(), &events, NULL, &gerror))
  {
    LOG_ERROR() << "Error e_cal_client_get_objects_for_uid_sync result: " << GERROR_MESSAGE(gerror) << std::endl;
    GERROR_FREE(gerror);
    return "";
  }

  GSList* current = events;
  //in case of recurring event, create revision as concatnation of single revision of all event instances,
  //so even if only on instance will be modified then we will be able to detect that whole event needs to be updated.
  while(current)
  {
    ECalComponent* component = (ECalComponent*)current->data;
    icalcomponent* iCalComponent =  e_cal_component_get_icalcomponent(component);
    revision += icaltime_as_ical_string(icalcomponent_get_dtstamp(iCalComponent));
    current = current->next;
  }

  e_cal_client_free_ecalcomp_slist(events);

  return revision;
}

OpenAB::PIMItem::Revisions EDSCalendarStorage::getRevisions(const OpenAB::PIMItem::IDs& ids)
{
  OpenAB::PIMItem::Revisions revisions;
  OpenAB::PIMItem::IDs::const_iterator it;
  for (it = ids.begin(); it != ids.end(); ++it)
  {
    revisions.push_back(getRevision((*it)));
  }

  return revisions;
}

enum OpenAB_Storage::Storage::eGetRevisions EDSCalendarStorage::getRevisions(std::map<std::string, std::string>& revisions)
{
  /*
   * In compare to getRevisions(const OpenAB::PIMItem::IDs& ids)
   * this has to return ids and revisions of all items in db, as
   * EDS does not implement query that returns all objects from db
   * scan db file and get ids list of all object
   * then use getRevisions(const OpenAB::PIMItem::IDs& ids) to query revisions.
   */
  std::ifstream db(databaseFileName.c_str(), std::ios_base::in);
  if (!db.is_open())
  {
    return eGetRevisionsFail;
  }

  std::string line;
  bool inEvent = false;
  std::string uid;
  std::vector<std::string> uids;

  while (std::getline(db, line))
  {
    OpenAB::trimWhitespaces(line);

    if (line == "BEGIN:VEVENT" || line =="BEGIN:VTODO")
    {
      inEvent = true;
      uid = "";
    }
    if (inEvent)
    {
      if (OpenAB::beginsWith(line, "UID:"))
      {
        uid = line.substr(strlen("UID:"));
        OpenAB::trimWhitespaces(uid);
      }

      if (line == "END:VEVENT" || line == "END:VTODO")
      {
        LOG_DEBUG()<<"Found event end"<<std::endl;
        if (!uid.empty())
        {
          uids.push_back(uid);
        }
        inEvent = false;
      }
    }
  }
  OpenAB::PIMItem::Revisions revs = getRevisions(uids);
  for (unsigned int i = 0; i < uids.size(); ++i)
  {
    revisions[uids[i]] = revs[i];
  }

  db.close();

  return eGetRevisionsOk;
}

enum OpenAB_Storage::Storage::eGetRevisions EDSCalendarStorage::getChangedRevisions(const std::string&,
                                                                    std::map<std::string, std::string>& ,
                                                                    std::vector<OpenAB::PIMItem::ID>&)
{
  //Implement support for delta notification extension, which can provide such information
  return eGetRevisionsFail;
}

enum OpenAB_Storage::Storage::eGetSyncToken EDSCalendarStorage::getLatestSyncToken(std::string&)
{
  return eGetSyncTokenFail;
}

OpenAB_Storage::StorageItemIterator* EDSCalendarStorage::newStorageItemIterator()
{
  EDSCalendarStorageItemIterator * ie = new EDSCalendarStorageItemIterator();
  if (NULL == ie)
  {
    LOG_ERROR() << "Error Cannot create the EDS IndexElemIterator"<<std::endl;
    return NULL;
  }
  if (ie->eCursorInitOK != ie->cursorInit(databaseFileName))
  {
    LOG_ERROR() << "Error Cannot Init the EDS IndexElemIterator"<<std::endl;
    delete ie;
    return NULL;
  }
  return ie;

  return NULL;
}

namespace EDSCalendarFactory {
  OpenAB_Storage::Storage* createInstance (const OpenAB_Storage::Parameters& params)
  {
    std::string db;
    OpenAB::PIMItemType type;

    LOG_FUNC();
    OpenAB::Variant param = params.getValue("db");
    if (param.invalid() || param.getType() != OpenAB::Variant::STRING)
    {
      LOG_ERROR() << "Parameter 'db' not found"<<std::endl;
      return NULL;
    }
    db = param.getString();

    param = params.getValue("item_type");
    if (param.invalid() || param.getType() != OpenAB::Variant::INTEGER)
    {
      LOG_ERROR()<< "Parameter 'item_type' not found"<<std::endl;
      return NULL;
    }
    switch (param.getInt())
    {
      case OpenAB::eEvent:
          type = OpenAB::eEvent;
          break;
      case OpenAB::eTask:
          type = OpenAB::eTask;
          break;
      default:
        LOG_ERROR()<< "Provided item_type not supported"<<std::endl;
        return NULL;
    }

    EDSCalendarStorage * cal = new EDSCalendarStorage(db, type);
    if (NULL == cal)
    {
      LOG_ERROR() << "Cannot Initialize EDSCalendarStorage"<<std::endl;
      return NULL;
    }
    return cal;
  }

  class EDSCalendarStorageFactory : OpenAB_Storage::Factory
  {
    public:
      /*!
       *  @brief Constructor.
       */
      EDSCalendarStorageFactory():
        Factory::Factory("EDSCalendar"){};

      /*!
       *  @brief Destructor, virtual by default.
       */
      virtual ~EDSCalendarStorageFactory(){};

      OpenAB_Storage::Storage * newIstance(const OpenAB_Storage::Parameters & params)
      {
        return createInstance(params);
      }
  };

  class EDSCalendarSourceFactory : OpenAB_Source::Factory
  {
    public:
      /*!
       *  @brief Constructor.
       */
      EDSCalendarSourceFactory():
        Factory::Factory("EDSCalendar"){};

      /*!
       *  @brief Destructor, virtual by default.
       */
      virtual ~EDSCalendarSourceFactory(){};

      OpenAB_Source::Source * newIstance(const OpenAB_Source::Parameters & params)
      {
        return createInstance(params);
      };
  };
}
namespace EDSCalendarStorageFactory
{
  REGISTER_PLUGIN_FACTORY(EDSCalendarFactory::EDSCalendarStorageFactory);
}

namespace EDSCalendarSourceFactory
{
  REGISTER_PLUGIN_FACTORY(EDSCalendarFactory::EDSCalendarSourceFactory);
}


