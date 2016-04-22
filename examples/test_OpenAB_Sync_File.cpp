/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file test_OpenAB_Sync_File.cpp
 * @include test_OpenAB_Sync_File.cpp
 */

/*
 # Build:
   g++ test_OpenAB_Sync_File.cpp `pkg-config OpenAB --libs --cflags` -o test_OpenAB_Sync_File.out
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
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

  if (argc < 3)
  {
    printf("Use %s <EDS_DB> <vCardFile>\n",argv[0]);
  }
  else
  {
    char * eds_db   = argv[1];

    OpenAB_Sync::Parameters params;
    params.setValue("source_plugin", "File");
    params.remoteSourcePluginParams.setValue("filename", argv[2]);

    params.setValue("storage_plugin", "EDS");
    params.localStoragePluginParams.setValue("db", eds_db);

    params.setValue("callback", &cb);

    OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>("OneWay", params);
    if (s == NULL)
    {
      printf("Initialization FAIL.. memory allocation failed\n");
      return -1;
    }
    std::vector<std::string> ignore;
    s->addPhase("all", ignore);
    if ( s->eInitOk != s->init() )
    {
      printf("Initialization FAIL\n");
      return -1;
    }
    s->synchronize();
    while(!finished)
      usleep(100000);

    OpenAB::PluginManager::getInstance().freePluginInstance(s);

  }
  return 0;
}

