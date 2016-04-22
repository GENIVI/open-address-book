/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file test_file_download.cpp
 * @include test_file_download.cpp
 */

/*
 # Build:
   g++ test_file_download.cpp `pkg-config OpenAB --libs --cflags` -o test_file_download
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include <OpenAB.hpp>
#include "helpers/SecureString.hpp"

int main(int argc, char* argv[])
{
  OpenAB::OpenAB_init();
  OpenAB::Logger::OutLevel() = OpenAB::Logger::Debug;

  if (argc < 2)
  {
    printf ("Use %s <authentication method>\n", argv[0]);
    return 1;
  }

  OpenAB_Source::Parameters params;
  if (0 == strcmp (argv[1], "password"))
  {
    if (argc < 5)
    {
      printf("Use %s password <server url> <user login> <user password>\n",argv[0]);
      return 1;
    }
    params.setValue("server_url", argv[2]);
    params.setValue("login", argv[3]);
    params.setValue("password", OpenAB::SecureString(argv[4]));
  }
  else
  {
    if (argc < 6)
    {
      printf("Use %s oauth2 <server url> <client id> <client secret> <refresh token>\n",argv[0]);
      return 1;
    }
    params.setValue("server_url", argv[2]);
    params.setValue("client_id", argv[3]);
    params.setValue("client_secret", OpenAB::SecureString(argv[4]));
    params.setValue("refresh_token", OpenAB::SecureString(argv[5]));
  }

  OpenAB_Source::Source * source = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("CardDAV", params);
  if (!source || OpenAB_Source::Source::eInitOk != source->init())
  {
    printf("Cannot initialize source\n");
    OpenAB::PluginManager::getInstance().freePluginInstance(source);
    return 1;
  }

  LOG_DEBUG()<<"Number of vcards "<<source->getTotalCount()<<std::endl;
  OpenAB::SmartPtr<OpenAB::PIMItem> vCard;
  while(OpenAB_Source::Source::eGetItemRetOk == source->getItem(vCard))
  {
    LOG_DEBUG()<<"Contact uri "<<vCard->getId()<<std::endl;
    LOG_DEBUG()<<"Contact revision "<<vCard->getRevision()<<std::endl;
    LOG_DEBUG()<<vCard->getRawData()<<std::endl<<std::endl;
  }

  OpenAB::PluginManager::getInstance().freePluginInstance(source);

  OpenAB_Storage::Storage * storage = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>("CardDAV", params);

      if (NULL == storage)
      {
       LOG_ERROR()<<"Cannot create storage"<<std::endl;
       return 1;
      }

      storage->init();
      std::map<std::string, std::string> revisions;
      storage->getRevisions(revisions);

      std::map<std::string, std::string>::iterator it;
      for (it = revisions.begin(); it != revisions.end(); ++it)
      {
        LOG_DEBUG()<<"UID: "<<(*it).first<<" REV: "<<(*it).second<<std::endl;
      }

      OpenAB::PluginManager::getInstance().freePluginInstance(storage);
  return 0;
}
