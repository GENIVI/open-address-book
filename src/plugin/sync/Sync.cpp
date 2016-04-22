/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file Sync.cpp
 */

#include "Sync.hpp"
#include <helpers/Log.hpp>
#include <json-c/json.h>
#include <string.h>

namespace OpenAB_Sync {

Parameters::Parameters()
  : OpenAB_Plugin::GenericParameters()
{

}

Parameters::Parameters(const std::string& json)
   : OpenAB_Plugin::GenericParameters(json)
{

}

Parameters::~Parameters()
{

}

bool Sync::addPhase(const std::string& name,
                    const std::vector<std::string> &ignoreFields)
{
  std::vector<Phase>::iterator it;
  for (it = phases.begin(); it != phases.end(); ++it)
  {
    if ((*it).name == name)
    {
      LOG_ERROR()<<"[Sync] "<<"addPhase: phase "<<name<<" already defined"<<std::endl;
      return false;
    }
  }

  phases.push_back(Phase(name, ignoreFields));
  return true;
}

void Sync::clearPhases()
{
  phases.clear();
}

SyncMetadata::SyncMetadata()
{

}

SyncMetadata::~SyncMetadata()
{
  remoteRevisions.clear();
  localRevisions.clear();
  remoteToLocalIdMapping.clear();
}

void SyncMetadata::addItem(const std::string& remoteId, const std::string& remoteRevision,
                           const std::string& localId, const std::string& localRevision)
{
  LOG_DEBUG()<<"Adding item rid "<<remoteId<<" localId "<<localId<<std::endl;
  remoteRevisions[remoteId] = remoteRevision;
  localRevisions[localId] = localRevision;
  remoteToLocalIdMapping[remoteId] = localId;
}

void SyncMetadata::removeItem(const std::string& remoteId,
                              const std::string& localId)
{
  std::map<std::string, std::string>::iterator it;
  it = remoteRevisions.find(remoteId);
  if (remoteRevisions.end() != it)
  {
    remoteRevisions.erase(it);
  }

  it = localRevisions.find(localId);
  if (localRevisions.end() != it)
  {
    localRevisions.erase(it);
  }

  it = remoteToLocalIdMapping.find(remoteId);
  if (remoteToLocalIdMapping.end() != it)
  {
    remoteToLocalIdMapping.erase(it);
  }
}

void SyncMetadata::updateLocalRevision(const std::string& uid, const std::string& revision)
{
  localRevisions[uid] = revision;
}

void SyncMetadata::updateRemoteRevision(const std::string& uid, const std::string& revision)
{
  remoteRevisions[uid] = revision;
}

std::string SyncMetadata::getRemoteRevision(const std::string& uid) const
{
  std::map<std::string, std::string>::const_iterator it;
  it = remoteRevisions.find(uid);
  if (remoteRevisions.end() == it)
  {
    return "";
  }

  return (*it).second;
}

std::string SyncMetadata::getLocalRevision(const std::string& uid) const
{
  std::map<std::string, std::string>::const_iterator it;
  it = localRevisions.find(uid);
  if (localRevisions.end() == it)
  {
    return "";
  }

  return (*it).second;
}

bool SyncMetadata::hasLocalId(const std::string& uid) const
{
  if (localRevisions.end() == localRevisions.find(uid))
  {
    return false;
  }
  return true;
}

bool SyncMetadata::hasRemoteId(const std::string& uid) const
{
  if (remoteRevisions.end() == remoteRevisions.find(uid))
  {
    return false;
  }
  return true;
}

std::string SyncMetadata::getRemoteSyncToken() const
{
  return remoteSyncToken;
}

void SyncMetadata::setRemoteSyncToken(const std::string& token)
{
  remoteSyncToken = token;
}

std::string SyncMetadata::getLocalSyncToken() const
{
  return localSyncToken;
}

void SyncMetadata::setLocalSyncToken(const std::string& token)
{
  localSyncToken = token;
}

std::string SyncMetadata::toJSON() const
{
  std::string json;
  json_object* jobj = json_object_new_object();

  json_object* localRevisionsObj = json_object_new_object();
  json_object* remoteRevisionsObj = json_object_new_object();
  json_object* localRemoteMappingObj = json_object_new_object();

  json_object* localSyncT = json_object_new_string(localSyncToken.c_str());
  json_object* remoteSyncT = json_object_new_string(remoteSyncToken.c_str());

  json_object_object_add(jobj, "LocalSyncToken", localSyncT);
  json_object_object_add(jobj, "RemoteSyncToken", remoteSyncT);

  std::map<std::string, std::string>::const_iterator it;
  for (it = localRevisions.begin(); it != localRevisions.end(); ++it)
  {
    json_object* jValue = NULL;
    jValue = json_object_new_string((*it).second.c_str());
    json_object_object_add(localRevisionsObj, (*it).first.c_str(), jValue);
  }

  json_object_object_add(jobj, "LocalRevisions", localRevisionsObj);

  for (it = remoteRevisions.begin(); it != remoteRevisions.end(); ++it)
  {
    json_object* jValue = NULL;
    jValue = json_object_new_string((*it).second.c_str());
    json_object_object_add(remoteRevisionsObj, (*it).first.c_str(), jValue);
  }

  json_object_object_add(jobj, "RemoteRevisions", remoteRevisionsObj);

  for (it = remoteToLocalIdMapping.begin(); it != remoteToLocalIdMapping.end(); ++it)
  {
    json_object* jValue = NULL;
    jValue = json_object_new_string((*it).second.c_str());
    json_object_object_add(localRemoteMappingObj, (*it).first.c_str(), jValue);
  }

  json_object_object_add(jobj, "RemoteToLocalMapping", localRemoteMappingObj);

  json = json_object_to_json_string(jobj);
  json_object_put(jobj);

  return json;
}

bool SyncMetadata::fromJSON(const std::string& json)
{
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

  json_object_object_foreach(jobj, key, val)
  {
    LOG_DEBUG()<<"PARSING  "<<key<<std::endl;
    if (0 == strcmp(key, "LocalRevisions"))
    {
      json_object_object_foreach(val, key2, val2)
      {
        localRevisions[key2] = json_object_get_string(val2);
      }
    }
    else if (0 == strcmp(key, "RemoteRevisions"))
    {
      json_object_object_foreach(val, key2, val2)
      {
        remoteRevisions[key2] = json_object_get_string(val2);
      }
    }
    else if (0 == strcmp(key, "RemoteToLocalMapping"))
    {
      json_object_object_foreach(val, key2, val2)
      {
        remoteToLocalIdMapping[key2] = json_object_get_string(val2);
      }
    }
    else if (0 == strcmp(key, "LocalSyncToken"))
    {
      localSyncToken = json_object_get_string(val);
    }
    else if (0 == strcmp(key, "RemoteSyncToken"))
    {
      remoteSyncToken = json_object_get_string(val);
    }
  }
  json_object_put(jobj);

  LOG_DEBUG()<<"Number of local revisions "<<localRevisions.size()<<std::endl;
  LOG_DEBUG()<<"Number of remote revisions "<<remoteRevisions.size()<<std::endl;
  LOG_DEBUG()<<"Number of remote to local mapping "<<remoteToLocalIdMapping.size()<<std::endl;
  return true;
}

void SyncMetadata::resetLocalState(enum SyncMetadata::SyncMetadataState state)
{
  localState.clear();

  std::map<std::string, std::string>::const_iterator it;
  for (it = localRevisions.begin(); it != localRevisions.end(); ++it)
  {
    localState.insert(std::pair<std::string, OpenAB_Sync::SyncMetadata::SyncMetadataState>((*it).first, state));
  }
}

void SyncMetadata::resetRemoteState(enum SyncMetadata::SyncMetadataState state)
{
  remoteState.clear();

  std::map<std::string, std::string>::const_iterator it;
  for (it = remoteRevisions.begin(); it != remoteRevisions.end(); ++it)
  {
    remoteState.insert(std::pair<std::string, OpenAB_Sync::SyncMetadata::SyncMetadataState>((*it).first, state));
  }
}

void SyncMetadata::setRemoteState(const std::string& uid, SyncMetadata::SyncMetadataState state)
{
  remoteState[uid] = state;
}

void SyncMetadata::setLocalState(const std::string& uid, SyncMetadata::SyncMetadataState state)
{
  localState[uid] = state;
}

std::map<std::string, std::string> SyncMetadata::getItemsWithState(SyncMetadataState remoteSt, SyncMetadataState localSt)
{
  std::map<std::string, std::string> result;

  std::map<std::string, std::string>::const_iterator it;
  for (it = remoteToLocalIdMapping.begin(); it != remoteToLocalIdMapping.end(); ++it)
  {
    if (remoteState[(*it).first] == remoteSt &&
        localState[(*it).second] == localSt)
    {
      result[(*it).first] = (*it).second;
    }
  }

  return result;
}

} // namespace OpenAB_Sync

EXPORT_PLUGIN_INTERFACE(OpenAB_Sync, Sync, Parameters);
