/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file PIMItemIndex.cpp
 */

#include "PIMItemIndex.hpp"
#include <helpers/Log.hpp>

namespace OpenAB {

PIMItemIndex::PIMItemIndex(OpenAB::PIMItemType t)
  : type(t)
{
}

PIMItemIndex::~PIMItemIndex()
{
}

OpenAB::PIMItemType PIMItemIndex::getType() const
{
  return type;
}

bool PIMItemIndex::compareVectors(const std::vector<std::string>& v1,
                                  const std::vector<std::string>& v2) const
{
  if (v1.size() != v2.size())
  {
    return false;
  }

  for (unsigned int i = 0; i < v1.size(); ++i)
  {
    if (v1[i] != v2[i])
    {
      return false;
    }
  }
  return true;
}

void PIMItemIndex::addKeyField(const std::string& name,
                               const std::string& value)
{
  key_fields_names.push_back(name);
  key_fields.push_back(value);
  if (!cached_to_string.empty())
  {
    cached_to_string.clear();
  }
}

void PIMItemIndex::addConflictField(const std::string& name,
                                    const std::string& value)
{
  conflict_fields_names.push_back(name);
  conflict_fields.push_back(value);
  if (!cached_to_string.empty())
  {
    cached_to_string.clear();
  }
}

} // namespace OpenAB
