/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */
/**
 * @file PIMContactItemIndex.cpp
 */

#include "PIMItem/Contact/PIMContactItemIndex.hpp"
#include <helpers/Log.hpp>

namespace OpenAB {

std::vector<PIMItemIndex::PIMItemCheck> PIMContactItemIndex::fields_desc;
bool PIMContactItemIndex::anyCheckDisabled = false;

PIMContactItemIndex::PIMContactItemIndex() :
    PIMItemIndex(OpenAB::eContact)
{
}

PIMContactItemIndex::~PIMContactItemIndex()
{
}


bool PIMContactItemIndex::compare(const PIMItemIndex& other) const
{
  if (getType() != other.getType())
  {
    return false;
  }

  return compare((const PIMContactItemIndex&) other);
}

bool PIMContactItemIndex::compare(const PIMContactItemIndex& other) const
{
  //ensure that eKey fields are the same
  if (*this != other)
  {
    return false;
  }

  if (!anyCheckDisabled)
  {
    return compareVectors(conflict_fields, other.conflict_fields);
  }

  std::vector<std::string> cfModified1;
  std::vector<std::string> cfModified2;
  std::vector<std::string> disabledChecks;

  std::vector<PIMItemCheck>::const_iterator it;
  for (it = fields_desc.begin(); it != fields_desc.end(); ++it)
  {
    if ((*it).enabled  == false)
    {
      //LOG_DEBUG()<<"[PIMItemIndex] "<<"compare: disabling check "<<(*it).fieldName<<std::endl;
      disabledChecks.push_back((*it).fieldName);
    }
  }

  for (unsigned int i = 0; i < conflict_fields_names.size(); ++i)
  {
    bool excluded = false;
    for (unsigned int j = 0; j < disabledChecks.size(); ++j)
    {
      //LOG_DEBUG()<<"[PIMItemIndex] "<<"compare: Comparing "<<conflict_fields_names[i]<<" with "<<disabledChecks[j]<<" = "<<(conflict_fields_names[i] != disabledChecks[j])<<std::endl;
      if (conflict_fields_names[i] == disabledChecks[j])
      {
        excluded = true;
        break;
      }
    }
    if (!excluded)
    {
      //LOG_DEBUG()<<"[PIMItemIndex] "<<"compare: adding check for contact1 "<<conflict_fields_names[i]<<std::endl;
      cfModified1.push_back(conflict_fields[i]);
    }
  }

  for (unsigned int i = 0; i < other.conflict_fields_names.size(); ++i)
  {
    bool excluded = false;
    for (unsigned int j = 0; j < disabledChecks.size(); ++j)
    {
      //LOG_DEBUG()<<"[PIMItemIndex] "<<"compare: Comparing "<<other.conflict_fields_names[i]<<" with "<<disabledChecks[j]<<" = "<<(other.conflict_fields_names[i] != disabledChecks[j])<<std::endl;

      if (other.conflict_fields_names[i] == disabledChecks[j])
      {
        excluded = true;
        break;
      }
    }
    if (!excluded)
    {
      //LOG_DEBUG()<<"[PIMItemIndex] "<<"compare: adding check for contact2 "<<other.conflict_fields_names[i]<<std::endl;
      cfModified2.push_back(other.conflict_fields[i]);
    }
  }

  return compareVectors(cfModified1, cfModified2);
}

bool PIMContactItemIndex::operator==(const PIMItemIndex& other) const
{
  if (getType() != other.getType())
  {
    return false;
  }
  return operator==((const PIMContactItemIndex&)other);
}

bool PIMContactItemIndex::operator!=(const PIMItemIndex& other) const
{
  if (getType() != other.getType())
  {
    return true;
  }
  return operator!=((const PIMContactItemIndex&)other);
}

bool PIMContactItemIndex::operator==(const PIMContactItemIndex& other) const
{
  return compareVectors(key_fields, other.key_fields);
}

bool PIMContactItemIndex::operator!=(const PIMContactItemIndex& other) const
{
  return !compareVectors(key_fields, other.key_fields);
}

bool PIMContactItemIndex::operator<(const PIMItemIndex& other) const
{
  if (getType() != other.getType())
  {
    return false;
  }
  return operator<((const PIMContactItemIndex&)other);
}

bool PIMContactItemIndex::operator<(const PIMContactItemIndex& other) const
{
  return toString() < other.toString();
  /*if (key_fields.size() > other.key_fields.size())
  {
    return false;
  }
  else if (key_fields.size() < other.key_fields.size())
  {
    return true;
  }

  for (unsigned int i = 0; i < key_fields.size(); ++i)
  {
    if (key_fields[i] >= other.key_fields[i])
    {
      return false;
    }
  }
  return true;*/
}

std::string PIMContactItemIndex::toString() const
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

std::string PIMContactItemIndex::toStringFull() const
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

void PIMContactItemIndex::clearAllChecks()
{
  fields_desc.clear();
}

std::vector<PIMItemIndex::PIMItemCheck> PIMContactItemIndex::getAllChecks()
{
  return fields_desc;
}

bool PIMContactItemIndex::addCheck(const std::string& fieldName,
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

bool PIMContactItemIndex::removeCheck(const std::string& fieldName)
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

bool PIMContactItemIndex::disableCheck(const std::string& fieldName)
{
  std::vector<PIMItemCheck>::iterator it;
  for (it = fields_desc.begin(); it != fields_desc.end(); ++it)
  {
    if ((*it).fieldName  == fieldName)
    {
      (*it).enabled = false;
      anyCheckDisabled = true;
      return true;
    }
  }
  LOG_ERROR()<<"[PIMItemIndex] "<<"disableCheck: Check for field "<<fieldName<<" doesn't exists"<<std::endl;
  return false;
}

bool PIMContactItemIndex::enableCheck(const std::string& fieldName)
{
  bool res = false;
  std::vector<PIMItemCheck>::iterator it;
  for (it = fields_desc.begin(); it != fields_desc.end(); ++it)
  {
    if ((*it).fieldName  == fieldName)
    {
      (*it).enabled = true;
      res = true;
      break;
    }
  }

  anyCheckDisabled = false;
  for (it = fields_desc.begin(); it != fields_desc.end(); ++it)
  {
    if (!(*it).enabled)
    {
      anyCheckDisabled = true;
      break;
    }
  }

  if (!res)
  {
    LOG_ERROR()<<"[PIMItemIndex] "<<"enableCheck: Check for field "<<fieldName<<" doesn't exists"<<std::endl;
  }

  return res;
}

void PIMContactItemIndex::enableAllChecks()
{
  std::vector<PIMItemCheck>::iterator it;
  for (it = fields_desc.begin(); it != fields_desc.end(); ++it)
  {
    (*it).enabled = true;
  }

  anyCheckDisabled = false;
}
} // namespace OpenAB
