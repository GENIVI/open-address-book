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

  if (argc < 2)
  {
    printf("Use %s <mac>\n",argv[0]);
  }
  else
  {
    char * filename = argv[1];
    OpenAB_Source::Parameters params;
    params.setValue("filename", filename);
    OpenAB_Source::Source * source = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("File", params);
    source->init();

    LOG_DEBUG()<<"Number of vcards "<<source->getTotalCount()<<std::endl;
    OpenAB::SmartPtr<OpenAB::PIMItem> vCard;
    while(OpenAB_Source::Source::eGetItemRetOk == source->getItem(vCard))
    {
      LOG_DEBUG()<<vCard->getRawData()<<std::endl<<std::endl;
    }
    OpenAB::PluginManager::getInstance().freePluginInstance(source);
  }
}
