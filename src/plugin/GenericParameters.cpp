/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file GenericParameters.cpp
 */

#include "GenericParameters.hpp"
#include <json-c/json.h>

namespace OpenAB_Plugin {

GenericParameters::GenericParameters()
  : OpenAB_Plugin::Parameters()
{

}

GenericParameters::GenericParameters(const std::string& json)
   : OpenAB_Plugin::Parameters()
{
  fromJSON(json);
}

GenericParameters::~GenericParameters()
{

}

std::string GenericParameters::toJSON() const
{
  std::string json;
  json_object* jobj = json_object_new_object();
  std::map<std::string, OpenAB::Variant>::const_iterator it;
  for(it = config.begin(); it != config.end(); ++it)
  {
    json_object* jValue = NULL;
    switch((*it).second.getType())
    {
      case OpenAB::Variant::INTEGER:
        jValue = json_object_new_int((*it).second.getInt());
        break;
      case OpenAB::Variant::DOUBLE:
        jValue = json_object_new_double((*it).second.getDouble());
        break;
      case OpenAB::Variant::STRING:
        jValue = json_object_new_string((*it).second.getString().c_str());
        break;
      case OpenAB::Variant::BOOL:
        jValue = json_object_new_boolean((*it).second.getBool());
        break;
      default:
        continue;
    }
    json_object_object_add(jobj, (*it).first.c_str(), jValue);
  }

  json = json_object_to_json_string(jobj);
  json_object_put(jobj);

  return json;
}

bool GenericParameters::fromJSON(const std::string& json)
{
  config.clear();
  if (std::string::npos == json.find_first_of('{'))
  {
    LOG_DEBUG()<<"Cannot parse json"<<std::endl;
    return false;
  }

  json_object* jobj = json_tokener_parse(json.c_str());

  if (NULL == jobj)
  {
    LOG_DEBUG()<<"Cannot parse json"<<std::endl;
    return false;
  }

  enum json_type type;
  json_object_object_foreach(jobj, key, val)
  {
    OpenAB::Variant value;
    type = json_object_get_type(val);
    switch(type)
    {
      case json_type_boolean:
        value.set((bool)json_object_get_boolean(val));
        break;
      case json_type_double:
        value.set(json_object_get_double(val));
        break;
      case json_type_int:
        value.set(json_object_get_int(val));
        break;
      case json_type_string:
        value.set(json_object_get_string(val));
        break;
      default:
        break;
    }
    if(!value.invalid())
    {
      config.insert(std::pair<std::string, OpenAB::Variant>(key, value));
    }
  }
  json_object_put(jobj);
  return true;
}

void GenericParameters::setValue(const std::string& key, const OpenAB::Variant& value)
{
  config[key] = value;
}

OpenAB::Variant GenericParameters::getValue(const std::string& key) const
{
  if(config.end() == config.find(key))
  {
    return OpenAB::Variant();
  }

  return (*config.find(key)).second;
}

void GenericParameters::removeKey(const std::string& key)
{
  if(config.end() != config.find(key))
      config.erase(key);
}

std::vector<std::string> GenericParameters::getAllKeys() const
{
  std::vector<std::string> result;

  std::map<std::string, OpenAB::Variant>::const_iterator it;
  for(it = config.begin(); it != config.end(); ++it)
  {
    result.push_back((*it).first);
  }

  return result;
}
} // namespace OpenAB_Plugin
