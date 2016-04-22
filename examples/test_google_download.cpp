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
#include <string>
#include <iostream>

#include <OpenAB.hpp>

int main(int argc, char* argv[])
{

  OpenAB::OpenAB_init();

  if (argc < 4)
  {
    printf("Use %s <client id> <client secret> <refresh token>\n",argv[0]);
  }
  else
  {
    OpenAB_Source::Parameters params;
    params.setValue("client_id", argv[1]);
    params.setValue("client_secret", OpenAB::SecureString(argv[2]));
    params.setValue("refresh_token", OpenAB::SecureString(argv[3]));

    OpenAB_Source::Source * source = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("Google", params);
    if (OpenAB_Source::Source::eInitOk != source->init())
    {
      printf("Cannot initialize source\n");
      OpenAB::PluginManager::getInstance().freePluginInstance(source);
      return 1;
    }

    LOG_DEBUG()<<"Number of vcards "<<source->getTotalCount()<<std::endl;
    OpenAB::SmartPtr<OpenAB::PIMItem> vCard;
    while(OpenAB_Source::Source::eGetItemRetOk == source->getItem(vCard))
    {
      LOG_DEBUG()<<vCard->getRawData()<<std::endl<<std::endl;
    }
    LOG_DEBUG()<<"Number of vcards "<<source->getTotalCount()<<std::endl;

    OpenAB::PluginManager::getInstance().freePluginInstance(source);
  }
}
