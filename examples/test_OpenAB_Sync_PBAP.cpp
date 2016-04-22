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
   g++ test_OpenAB_Sync_PBAP.cpp `pkg-config OpenAB --libs --cflags` -o test_OpenAB_Sync_PBAP.out
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>
#include <unistd.h>

#include <OpenAB.hpp>

bool finished = false;

const char* toString(enum OpenAB_Sync::Sync::eSync s)
{
  switch(s)
  {
    case OpenAB_Sync::Sync::eSyncOkWithDataChange:
      return "eSyncOkWithDataChange";
      break;
    case OpenAB_Sync::Sync::eSyncOkWithoutDataChange:
      return "eSyncOkWithoutDataChange";
      break;
    case OpenAB_Sync::Sync::eSyncCancelled:
      return "eSyncCancelled";
      break;
    case OpenAB_Sync::Sync::eSyncAlreadyInProgress:
      return "eSyncAlreadyInProgress";
      break;
    case OpenAB_Sync::Sync::eSyncFail:
      return "eSyncFail";
      break;
  }
  return "";
}

class TestCallback : public OpenAB_Sync::Sync::SyncCallback
{
  public:
    TestCallback():OpenAB_Sync::Sync::SyncCallback(){};
    void print(const std::string&s){
      printf("Callback: %s\n",s.c_str());
    }

    void syncFinished(const enum OpenAB_Sync::Sync::eSync& result)
    {
      printf("Sync finished: %s\n", toString(result));
      finished = true;
    }

    void syncProgress(const std::string& phase, double progress, unsigned int numProcessedItems)
    {
      printf("Sync progress (%s): %f - %i items\n", phase.c_str(), progress, numProcessedItems);
    }

    void syncPhaseStarted(const std::string& phase)
    {
      printf("Sync phase started: %s\n", phase.c_str());
    }

    void syncPhaseFinished(const std::string& phase)
    {
      printf("Sync phase finished: %s\n", phase.c_str());
    }
}cb;



int main(int argc, char* argv[])
{

  OpenAB::OpenAB_init();
  OpenAB::Logger::OutLevel() = OpenAB::Logger::Error;

  if (argc < 3)
  {
    printf("Use %s <EDS_DB> <mac> <sync type>\n",argv[0]);
  }
  else
  {
    OpenAB_Sync::Parameters params;
    params.setValue("source_plugin", "PBAP");
    params.remoteSourcePluginParams.setValue("MAC", argv[2]);

    params.setValue("storage_plugin", "EDS");
    params.localStoragePluginParams.setValue("db", argv[1]);

    params.setValue("callback", &cb);
    params.setValue("sync_progress_frequency", 5.0);

    OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", params);
    if ( s == NULL)
    {
      printf("Error allocating Plugin Instance .. FAILURE\n");
      return -1;

    }

    std::vector<std::string> ignore;
    if (argc > 3)
    {
      if(strcmp(argv[3], "incremental") == 0)
      {
        ignore.push_back("photo");
        s->addPhase("incremental_text", ignore);

        ignore.clear();
        s->addPhase("incremental_photo", ignore);
      }
      else if(strcmp(argv[3], "all") == 0)
      {
        s->addPhase("all", ignore);
      }
      else if(strcmp(argv[3], "text") == 0)
      {
        ignore.push_back("photo");
        s->addPhase("text_only", ignore);
      }
    }
    else
    {
      s->addPhase("all", ignore);
    }

    if ( s->eInitOk != s->init() )
    {
      printf("Initialization FAIL\n");
      return -1;
    }
    s->synchronize();

    std::string line;
    while(std::getline(std::cin, line) && !finished)
    {
      if(line.compare("p") == 0)
      {
        s->suspend();
        printf("Suspend\n");
      }
      if(line.compare("r") == 0)
      {
        s->resume();
        printf("Resume\n");
      }
      if(line.compare("c") == 0)
      {
        s->cancel();
        printf("Cancel\n");
      }
      if(line.compare("e") == 0)
      {

        break;
      }
    }
    while(!finished)
         usleep(100000);
    OpenAB::PluginManager::getInstance().freePluginInstance(s);
    
    return 0;
  }
  return 0;
}

