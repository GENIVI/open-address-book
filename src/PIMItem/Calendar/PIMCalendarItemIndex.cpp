/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file PIMCalendarItemIndex.cpp
 */

#include "PIMItem/Calendar/PIMCalendarItemIndex.hpp"
#include <helpers/Log.hpp>

namespace OpenAB {

std::vector<PIMItemIndex::PIMItemCheck> PIMCalendarItemIndex::fields_desc;

PIMCalendarItemIndex::PIMCalendarItemIndex(PIMItemType t) :
    PIMItemIndex(t)
{
}

PIMCalendarItemIndex::~PIMCalendarItemIndex()
{
}


bool PIMCalendarItemIndex::compare(const PIMItemIndex& other) const
{
  if (getType() != other.getType())
  {
    return false;
  }

  return compareWith(other);
}

bool PIMCalendarItemIndex::operator==(const PIMItemIndex& other) const
{
  if (getType() != other.getType())
  {
    return false;
  }
  return equal(other);
}

bool PIMCalendarItemIndex::operator!=(const PIMItemIndex& other) const
{
  if (getType() != other.getType())
  {
    return true;
  }
  return notEqual(other);
}

bool PIMCalendarItemIndex::operator<(const PIMItemIndex& other) const
{
  if (getType() != other.getType())
  {
    return false;
  }
  return lessThan((const PIMCalendarItemIndex&)other);
}



std::string PIMCalendarItemIndex::toString() const
{
  if (cached_to_string.empty())
  {
    std::stringstream ss;
    for (unsigned int i = 0; i < key_fields.size(); ++i)
    {
      ss << key_fields[i]<<" : ";
    }

    cached_to_string = ss.str();
  }
  return cached_to_string;
}

std::string PIMCalendarItemIndex::toStringFull() const
{
  std::stringstream ss;
  for(unsigned int i = 0; i < key_fields.size(); ++i)
  {
    ss << key_fields[i] << " : ";
  }
  for(unsigned int i = 0; i < conflict_fields.size(); ++i)
  {
    ss << conflict_fields[i] << " : ";
  }
  return ss.str();
}

void PIMCalendarItemIndex::clearAllChecks()
{
  fields_desc.clear();
}

std::vector<PIMItemIndex::PIMItemCheck> PIMCalendarItemIndex::getAllChecks()
{
  return fields_desc;
}

bool PIMCalendarItemIndex::addCheck(const std::string& fieldName,
                                    PIMItemCheck::eFieldRole role)
{
  std::vector<PIMItemCheck>::iterator it;
  for (it = fields_desc.begin(); it != fields_desc.end(); ++it)
  {
    if ((*it).fieldName  == fieldName)
    {
      LOG_ERROR()<<"[PIMItemIndex] "<<"addCheck: Check for field "<<fieldName<<" already exists"<<std::endl;
      return false;
    }
  }
  PIMItemCheck newCheck(fieldName, role);
  fields_desc.push_back(newCheck);
  return true;
}

bool PIMCalendarItemIndex::removeCheck(const std::string& fieldName)
{
  std::vector<PIMItemCheck>::iterator it;
  for (it = fields_desc.begin(); it != fields_desc.end(); ++it)
  {
    if ((*it).fieldName  == fieldName)
    {
      fields_desc.erase(it);
      return true;
    }
  }
  LOG_ERROR()<<"[PIMItemIndex] "<<"removeCheck: Check for field "<<fieldName<<" doesn't exists"<<std::endl;
  return false;
}

bool PIMCalendarItemIndex::compareWith(const PIMItemIndex& other) const
{
  //ensure that eKey fields are the same
  if (*this != other)
  {
    return false;
  }
  PIMCalendarItemIndex* otherCalendarItem = (PIMCalendarItemIndex*) &other;

  return compareVectors(conflict_fields, otherCalendarItem->conflict_fields);
}


bool PIMCalendarItemIndex::equal(const PIMItemIndex& other) const
{
  PIMCalendarItemIndex* otherCalendarItem = (PIMCalendarItemIndex*) &other;

  return compareVectors(key_fields, otherCalendarItem->key_fields);
}

bool PIMCalendarItemIndex::notEqual(const PIMItemIndex& other) const
{
  PIMCalendarItemIndex* otherCalendarItem = (PIMCalendarItemIndex*) &other;

  return !compareVectors(key_fields, otherCalendarItem->key_fields);
}


bool PIMCalendarItemIndex::lessThan(const PIMItemIndex& other) const
{
  return toString() < other.toString();
}

PIMCalendarEventItemIndex::PIMCalendarEventItemIndex()
  : PIMCalendarItemIndex(eEvent)
{

}

PIMCalendarEventItemIndex::~PIMCalendarEventItemIndex()
{

}

PIMCalendarTaskItemIndex::PIMCalendarTaskItemIndex()
  : PIMCalendarItemIndex(eTask)
{

}

PIMCalendarTaskItemIndex::~PIMCalendarTaskItemIndex()
{

}

} // namespace OpenAB
