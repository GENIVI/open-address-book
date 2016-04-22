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
#include <fstream>

#include <OpenAB.hpp>
#include <plugin/storage/CalendarStorage.hpp>

int main(int argc, char* argv[])
{

  OpenAB::OpenAB_init();

  if (argc < 3)
  {
    printf("Use %s <plugin name> <db name>\n",argv[0]);
  }
  else
  {
    char * dbname = argv[2];
    OpenAB_Source::Parameters params;
    params.setValue("db", dbname);
    OpenAB_Storage::Storage * storage = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Storage::Storage>(argv[1], params);

    if (NULL == storage)
    {
      LOG_ERROR()<<"Cannot create source"<<std::endl;
      return 1;
    }

    if (OpenAB_Source::Source::eInitOk != storage->init())
    {
      LOG_ERROR()<<"Cannot initialize source"<<std::endl;
      OpenAB::PluginManager::getInstance().freePluginInstance(storage);
      return 1;
    }

    LOG_DEBUG()<<"Number of items "<<storage->getTotalCount()<<std::endl;
    OpenAB::SmartPtr<OpenAB::PIMItem> vCard;
    while(OpenAB_Source::Source::eGetItemRetOk == storage->getItem(vCard))
    {
      LOG_DEBUG()<<"1111 Item id "<<vCard->getId()<<std::endl;
      LOG_DEBUG()<<"Item revision "<<vCard->getRevision()<<std::endl;
      LOG_DEBUG()<<vCard->getRawData()<<std::endl<<std::endl;
    }


    OpenAB_Storage::CalendarStorage* cStorage = dynamic_cast<OpenAB_Storage::CalendarStorage*>(storage);

    if (argc > 3)
    {
      OpenAB::SmartPtr<OpenAB::PIMCalendarEventItem> item;
      cStorage->getEvent(argv[3], item);
      LOG_DEBUG()<<item->getRawData()<<std::endl;
    }
    OpenAB::PluginManager::getInstance().freePluginInstance(storage);
  }
}
