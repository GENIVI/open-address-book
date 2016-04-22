/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file test_OpenAB_Sync_PBAP.cpp
 * @include test_OpenAB_Sync_PBAP.cpp
 */

/*
 # Build:
   g++ test_PBAP_download.cpp `pkg-config OpenAB --libs --cflags` -o test_pbap_download
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include <OpenAB.hpp>

int main(int argc, char* argv[])
{

  OpenAB::OpenAB_init();

  if (argc < 3)
  {
    printf("Use %s <mac> <chunk size>\n",argv[0]);
  }
  else
  {
    char * filename = argv[1];
    unsigned int chunkSize = atoi(argv[2]);
    OpenAB_Source::Parameters params;
    params.setValue("MAC", filename);
    params.setValue("filter", "FN,PHOTO");
    params.setValue("batch_download_time", chunkSize);
    OpenAB_Source::Source * input = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("PBAP", params);
    input->init();

    OpenAB::SmartPtr<OpenAB::PIMItem> vCard;
  // while(OpenAB_Source::Source::eGetItemRetOk == input->getItem(vCard))
   // {
      ;//LOG_DEBUG()<<vCard->getRawData()<<std::endl<<std::endl;
    //}
    std::string line;
    while(std::getline(std::cin, line))
    {
      if(line.compare("p") == 0)
      {
        input->suspend();
        printf("Suspend\n");
      }
      if(line.compare("r") == 0)
      {
        input->resume();
        printf("Resume\n");
      }
      if(line.compare("c") == 0)
      {
        input->cancel();
        printf("Cancel\n");
      }
      if(line.compare("e") == 0)
      {
        break;
      }
    }
    OpenAB::PluginManager::getInstance().freePluginInstance(input);
  }
}
