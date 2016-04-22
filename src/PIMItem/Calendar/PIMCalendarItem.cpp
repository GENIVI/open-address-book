/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file PIMCalendarItem.cpp
 */

#include "PIMCalendarItem.hpp"
#include "helpers/StringHelper.hpp"
#include "PIMCalendarItemIndex.hpp"
#include <algorithm>

namespace OpenAB {

ItemField::ItemField(){}
ItemField::ItemField(const std::string& v) :
    value(v) {}

ItemField::~ItemField(){}

bool ItemField::parse(std::string rawFieldString, bool forceParams)
{
  std::size_t position;
  std::string param;

  // check if field has any parameters
  position = rawFieldString.find_first_of(':');

  if (!forceParams)
  {
    //if not store whole string field as value of field
    if(position == std::string::npos)
    {
      value = rawFieldString;
      return true;
    }

    //if not, cut value part, and process what left as parameters
    value = rawFieldString.substr(position+1);
    rawFieldString.erase(position);
  }

  //parse parameters
  std::size_t oldPosition = 0;
  while((position = rawFieldString.find_first_of(';', oldPosition)) != std::string::npos)
  {
    param = rawFieldString.substr(oldPosition, position - oldPosition);
    oldPosition = position+1;
    processParam(param);
  }

  param = rawFieldString.substr(oldPosition);
  processParam(param);

  return true;
}

void ItemField::setValue (const std::string& v)
{
  value = v;
}

void ItemField::processParam(std::string paramLine)
{
  std::size_t position = paramLine.find_first_of('=');
  //cut param name
  std::string paramName = paramLine.substr(0, position);
  std::set<std::string> paramValues;
  paramLine.erase(0, position+1);

  //split what left using ',' as a separator and use as values of parameter
  //ignore duplicated and empty values
  std::vector<std::string> values = OpenAB::tokenize(paramLine, ',', true, false);
  std::vector<std::string>::iterator it;
  for (it = values.begin(); it != values.end(); ++it)
  {
    OpenAB::eraseAllOccurences((*it), '"');
    paramValues.insert((*it));
  }

  //if we found any values add param to field
  if(!paramValues.empty())
  {
    addParam(paramName, paramValues);
  }
}

void ItemField::addParam(std::string paramName, std::set<std::string> values)
{
  if(paramName.compare(0, 2, "x-") == 0)
  {
    //don't store extension parameters, e.g. X-EVOLUTION-E164
    return;
  }
  if(params.find(paramName) != params.end())
  {
    //this are some additional values for param that was already added
    std::set<std::string>::iterator it;
    for(it = values.begin(); it != values.end(); ++it)
    {
      params[paramName].insert((*it));
    }
  }
  else
  {
    params.insert(std::pair<std::string, std::set<std::string> >(paramName, values));
  }
}

std::string ItemField::toString() const
{
  std::string res;

  std::map<std::string, std::set<std::string> >::const_iterator it1;
  std::set<std::string>::iterator it2;

  bool firstP = true;
  for(it1 = params.begin(); it1 != params.end(); ++it1)
  {
    if(firstP)
    {
      firstP = false;
    }
    else{
      res += ";";
    }

    res+=(*it1).first;
    res+="=";
    bool first = true;
    for(it2 = (*it1).second.begin(); it2 != (*it1).second.end(); ++it2)
    {
      if(first)
      {
        first = false;
      }
      else
      {
        res+=",";
      }
      res+=(*it2);
    }
  }

  if(!firstP)
    res+=":";
  res+=value;

  return res;
}

std::string ItemField::getValue() const
{
  return value;
}

std::map<std::string, std::set<std::string> > ItemField::getParams() const
{
  return params;
}

bool ItemField::operator<(const ItemField& other) const
{
  return toString() < other.toString();
}

KeyValueItem::KeyValueItem() :
    inSubcomponent(false)
{

}

KeyValueItem::~KeyValueItem()
{

}

bool KeyValueItem::processSubcomponent()
{
  if (inSubcomponent)
  {
    //is end of current subcomponent found
    if ("end" == currentFieldName && currentSubcomponentName == currentFieldValue)
    {
      inSubcomponent = false;

      KeyValueItem* subComponent = KeyValueItemFactory::createItem(currentSubcomponentName);
      subComponent->parse(currentSubcomponentData);

      subcomponents[currentSubcomponentName].push_back(subComponent);
      return true;
    }
    else
    {
      currentSubcomponentData += currentLine;
      currentSubcomponentData += "\n";
      return true;
    }
  }

  //subcomponent is found
  if ("begin" == currentFieldName && currentFieldValue != "vcalendar")
  {
    inSubcomponent = true;
    currentSubcomponentName = currentFieldValue;
    currentSubcomponentData.clear();
    return true;
  }

  return false;
}

bool KeyValueItem::processField()
{
  //ignored fields
  if (currentFieldName == "rev" ||
      currentFieldName == "prodid" ||
      currentFieldName.compare(0, 12, "x-evolution-") == 0 ||
      (currentFieldName == "begin" && currentFieldValue == "vcalendar") ||
      (currentFieldName == "end" && currentFieldValue == "vcalendar"))
  {
    return true;
  }

  ItemField field;
  field.parse(currentFieldValue);
  fields[currentFieldName].push_back(field);

  return false;
}

bool KeyValueItem::parse(const std::string& rawData)
{
  fields.clear();
  std::string data = rawData;

  linearize(data);

  std::stringstream stream(data);

  while (getUnfoldedLine(stream, currentLine))
  {
    unquoteSpecialCharacters(currentLine);
    trimWhitespaces(currentLine);
    if (currentLine.length() == 0)
      continue;

    //split line, parse field
    std::size_t pos1 = currentLine.find_first_of(':');
    std::size_t pos2 = currentLine.find_first_of(';');

    if (pos2 < pos1)
      pos1 = pos2;

    currentFieldName = currentLine.substr(0, pos1);

    //1. to lower case
    for (std::string::size_type i = 0; i < currentFieldName.length(); ++i)
    {
      currentFieldName[i] = std::tolower(currentFieldName[i]);
    }

    currentFieldValue = currentLine.substr(pos1 + 1);
    currentFieldValueOriginal = currentFieldValue;

    if (currentFieldValue.empty())
    {
      continue;
    }

    std::string::size_type fieldValueLen = currentFieldValue.size();
    for (std::string::size_type i = 0; i < fieldValueLen; ++i)
    {
      currentFieldValue[i] = std::tolower(currentFieldValue[i]);
    }

    if (processSubcomponent())
    {
      continue;
    }

    if (processField())
    {
      continue;
    }
  }

  //sort all of the fields in alphabetic order of their values, so if they occur in vCards
  //in different order, vCards still can be recognized as totally equal
  std::map<std::string, std::vector<ItemField> >::iterator it;
  for (it = fields.begin(); it != fields.end(); ++it)
  {
    std::sort((*it).second.begin(), (*it).second.end());
  }

  return true;
}

std::map<std::string, KeyValueItemFactory*> KeyValueItemFactory::factories;

void KeyValueItemFactory::registerFactory(KeyValueItemFactory* factory, const std::string& componentType)
{
  factories[componentType] = factory;
}

KeyValueItem* KeyValueItemFactory::createItem(const std::string& componentType)
{
  if (factories.find(componentType) != factories.end())
  {
    return factories[componentType]->createItem();
  }

  //return default key value item
  return new KeyValueItem();
}

CREATE_KEY_VALUE_ITEM_FACTORY(VStandardTimeZoneKeyValueItem, "standard");
CREATE_KEY_VALUE_ITEM_FACTORY(VDayLightTimeZoneKeyValueItem, "daylight");

PIMCalendarItem::PIMCalendarItem(PIMItemType t) :
    PIMItem(t),
    KeyValueItem()
{
}

PIMCalendarItem::~PIMCalendarItem()
{
}

bool PIMCalendarItem::parse(const std::string& iCal)
{
  iCalendar = iCal;

  return KeyValueItem::parse(iCalendar);
}

SmartPtr<PIMItemIndex> PIMCalendarItem::getIndex()
{
  PIMCalendarItemIndex *newIndex = NULL;
  std::map<std::string, std::vector<SmartPtr<KeyValueItem> > >::iterator subCmpIt = subcomponents.end();
  if (type == eEvent)
  {
    newIndex = new PIMCalendarEventItemIndex();

    //find vevent subcomponent
    subCmpIt = subcomponents.find("vevent");
  }
  else
  {
    newIndex = new PIMCalendarTaskItemIndex();
    //find vtodo subcomponent
    subCmpIt = subcomponents.find("vtodo");
  }

  if (subCmpIt == subcomponents.end())
  {
    LOG_ERROR()<<"Cannot find vevent/vtodo component in:"<<std::endl<<getRawData()<<std::endl;
    return newIndex;
  }

  KeyValueItem* vevent = (*subCmpIt).second.at(0);

  std::vector<PIMItemIndex::PIMItemCheck> checks = PIMCalendarItemIndex::getAllChecks();
  std::vector<PIMItemIndex::PIMItemCheck>::iterator it;
  for (it = checks.begin(); it != checks.end(); ++it)
  {
   std::map<std::string, std::vector<ItemField> >::iterator it2;
   std::map<std::string, std::vector<ItemField> >& fi = vevent->getFields();
   it2 = fi.find((*it).fieldName);
   if (it2 != fi.end())
   {
     std::vector<ItemField>::iterator it3;
     for (it3 = (*it2).second.begin(); it3 != (*it2).second.end(); ++it3)
     {
       if ((*it).fieldRole == PIMItemIndex::PIMItemCheck::eKey)
         newIndex->addKeyField((*it).fieldName, (*it3).toString());
       else
         newIndex->addConflictField((*it).fieldName, (*it3).toString());
     }
   }
  }
  return SmartPtr<PIMItemIndex>(newIndex);
}

std::string PIMCalendarItem::getRawData() const
{
  return iCalendar;
}

PIMCalendarEventItem::PIMCalendarEventItem()
  : PIMCalendarItem(eEvent)
{
}

PIMCalendarEventItem::~PIMCalendarEventItem()
{
}

PIMCalendarTaskItem::PIMCalendarTaskItem()
    : PIMCalendarItem(eTask)
{
}

PIMCalendarTaskItem::~PIMCalendarTaskItem()
{
}


} // namespace OpenAB
