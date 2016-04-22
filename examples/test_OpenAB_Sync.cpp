/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file test_OpenAB_Sync_Google.cpp
 * @include test_OpenAB_Sync_Google.cpp
 */

/*
 # Build:
   g++ test_OpenAB_Sync_Google.cpp `pkg-config OpenAB --libs --cflags` -o test_OpenAB_Sync_PBAP.out
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <popt.h>
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

    void metadataUpdated(const std::string& metadata)
    {
      std::ofstream metadataFile (".metadata");
      metadataFile<<metadata;
      metadataFile.close();
    }
}cb;



int main(int argc, char* argv[])
{
  OpenAB::OpenAB_init();
  OpenAB::Logger::OutLevel() = OpenAB::Logger::Debug;

  std::map<std::string, std::string> availablePlugins = OpenAB::PluginManager::getInstance().PluginManager::getListOfPlugins();
  std::string pluginsDescription = "Available plugins: ";
  std::map<std::string, std::string>::const_iterator it;
  for (it = availablePlugins.begin(); it != availablePlugins.end();++it)
  {
    pluginsDescription += (*it).first + " ";
  }


  const char* syncType = NULL;

  const char* localPlugin = NULL;
  const char* remotePlugin = NULL;

  const char* localParams = NULL;
  const char* remoteParams = NULL;

  poptContext optCon;
  struct poptOption optionsTable[] = {
    {"syncType", ' ', POPT_ARG_STRING, &syncType, ' ', "Available Synchronization type: OneWay or TwoWay", "Type of synchronization to be performed"},
    {"localPlugin", ' ', POPT_ARG_STRING, &localPlugin, ' ', pluginsDescription.c_str(), "Name of plugin to be used for local storage"},
    {"remotePlugin", ' ', POPT_ARG_STRING, &remotePlugin, ' ', pluginsDescription.c_str(), "Name of plugin to be used for remote source/storage"},
    {"localParams", ' ', POPT_ARG_STRING, &localParams, ' ', "Please refer to OpenAB plugins documentation for parameters specific to given plugin", "JSON encoded parameters for local plugin"},
    {"remoteParams", ' ', POPT_ARG_STRING, &remoteParams, ' ', "Please refer to OpenAB plugins documentation for parameters specific to given plugin", "JSON encoded parameters for remote plugin"},
    POPT_AUTOHELP
    POPT_TABLEEND
  };
  optCon = poptGetContext(NULL, argc, (const char **) argv, optionsTable, 0);

  char opt;
  while ((opt = (char)poptGetNextOpt(optCon)) >= 0)
  {

  }

  poptFreeContext(optCon);

  if (syncType == NULL || localPlugin == NULL || remotePlugin == NULL || localParams == NULL || remoteParams == NULL)
  {
    LOG_ERROR()<<"Please provide all required parameters"<<std::endl;
    return 1;
  }

  OpenAB_Sync::Parameters params;
  params.setValue("remote_plugin", remotePlugin);
  if (!params.remoteSourcePluginParams.fromJSON(remoteParams))
  {
    LOG_ERROR()<<"Cannot parse remote plugin parameters: "<<remoteParams<<std::endl;
    return 1;
  }
  params.remoteStoragePluginParams = params.remoteSourcePluginParams;

  params.setValue("local_plugin", localPlugin);
  if (!params.localSourcePluginParams.fromJSON(localParams))
  {
   LOG_ERROR()<<"Cannot parse local plugin parameters: "<<localParams<<std::endl;
   return 1;
  }
  params.localStoragePluginParams = params.localSourcePluginParams;

  params.setValue("callback", &cb);

  std::string metadata;
  std::string line;
  std::ifstream metadataFile (".metadata");
  if (metadataFile.is_open())
  {
    while (std::getline(metadataFile, line))
    {
      metadata += line;
    }

    metadataFile.close();
    unlink(".metadata");
    params.setValue("metadata", metadata);
  }

  OpenAB_Sync::Sync* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Sync::Sync>(syncType, params);

  if (NULL == s)
  {
    LOG_ERROR()<<"Cannot initialize synchronization plugin"<<std::endl;
    return 1;
  }

  std::vector<std::string> ignore;
  s->addPhase("empty", ignore);

  if ( s->eInitOk != s->init() )
  {
    LOG_ERROR()<<"Initialization failed"<<std::endl;
    OpenAB::PluginManager::getInstance().freePluginInstance(s);
    return 1;
  }
  s->synchronize();

   /* while(std::getline(std::cin, line) && !finished)
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
    }*/
    while(!finished)
         usleep(100000);

    unsigned int lAdd, lMod, lRem, rAdd, rMod, rRem;
    s->getStats(lAdd, lMod, lRem, rAdd, rMod, rRem);
    LOG_DEBUG()<<"Synchronization statistics:"<<std::endl;
    LOG_DEBUG()<<"Locally added items: "<<lAdd<<std::endl;
    LOG_DEBUG()<<"Locally modified items: "<<lMod<<std::endl;
    LOG_DEBUG()<<"Locally removed items: "<<lRem<<std::endl;
    LOG_DEBUG()<<std::endl;
    LOG_DEBUG()<<"Remotely added items: "<<rAdd<<std::endl;
    LOG_DEBUG()<<"Remotely modified items: "<<rMod<<std::endl;
    LOG_DEBUG()<<"Remotely removed items: "<<rRem<<std::endl;

    OpenAB::PluginManager::getInstance().freePluginInstance(s);

}
