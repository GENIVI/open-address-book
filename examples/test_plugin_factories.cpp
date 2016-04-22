/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file test_plugin_factories.cpp
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <OpenAB.hpp>

int main(int argc, char* argv[])
{
  OpenAB::OpenAB_init();

  std::map<std::string, std::string> availablePlugins = OpenAB::PluginManager::getInstance().getListOfPlugins();
  std::map<std::string, std::string>::iterator it = availablePlugins.begin();
  LOG_DEBUG()<<"Available plugins:"<<std::endl;
  for(; it != availablePlugins.end(); ++it)
  {
    LOG_DEBUG()<<(*it).first<<" from "<<(*it).second<<std::endl;
  }

  printf("\nAddressbook Plugins:\n");
  for(OpenAB_Storage::Factory::typeList::iterator it =
      OpenAB_Storage::Factory::factories.begin();
      it != OpenAB_Storage::Factory::factories.end();
      ++it){
    printf("%s\n",it->getName().c_str());
  }

  printf("\nInput Plugins:\n");
  for(OpenAB_Source::Factory::typeList::iterator it =
      OpenAB_Source::Factory::factories.begin();
      it != OpenAB_Source::Factory::factories.end();
      ++it){
    printf("%s\n",it->getName().c_str());
  }


  if (argc < 2)
  {
    printf("Use %s <vCardFile>\n",argv[0]);
  }
  else
  {
    printf("\nTest Factory\n");
    OpenAB_Source::Parameters p;
    p.setValue("filename", argv[1]);
    printf("Config: %s\n", p.toJSON().c_str());
    OpenAB_Source::Source * source;
    source = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>(std::string("File"), p);
    if (NULL == source)
    {
      printf("Cannot initialize input object\n");
      return -1;
    }
    source->init();

    OpenAB::SmartPtr<OpenAB::PIMItem> vCard;
    while (source->eGetItemRetOk == source->getItem(vCard))
    {
      printf("###########-------------------- > vCard:\n%s######################---------------->END\n", vCard->getRawData().c_str());
    }

    OpenAB::PluginManager::getInstance().freePluginInstance(source);

    printf("\nTest Error Factory\n");
    if (NULL != OpenAB_Source::Factory::factories["Does Not Exist"])
    {
      source = OpenAB_Source::Factory::factories["Does Not Exist"]->newIstance(p);
      if (NULL == source)
      {
        printf("Cannot initialize input object\n");
      }
      source->init();
      delete source;
    }
  }
}

