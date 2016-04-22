/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file test_vcard_parsing.cpp
 * @include test_vcard_parsing.cpp
 */

/*
 # Build:
   g++ test_vcard_parsing.cpp `pkg-config OpenAB --libs --cflags` -o test_vcard_parsing.out
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <OpenAB.hpp>

int main(int argc, char* argv[])
{

  OpenAB::OpenAB_init();

  if (argc < 3)
  {
    printf("Use %s <1s vcard file> <2nd vcard file>\n",argv[0]);
  }
  else
  {
    char * filename = argv[1];
    OpenAB_Source::Parameters params;
    params.setValue("filename", filename);

    OpenAB_Source::Source * input1 = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>(std::string("File"), params);
    input1->init();

    OpenAB::SmartPtr<OpenAB::PIMItem> contact1;
    input1->getItem(contact1);

    params.setValue("filename", argv[2]);
    OpenAB_Source::Source * input2 = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>(std::string("File"), params);
    input2->init();
    OpenAB::SmartPtr<OpenAB::PIMItem> contact2;
    input2->getItem(contact2);

    OpenAB::SmartPtr<OpenAB::PIMItemIndex> index1 = contact1->getIndex();
    OpenAB::SmartPtr<OpenAB::PIMItemIndex> index2 = contact2->getIndex();
    LOG_DEBUG()<<index1->toString()<<std::endl;
    LOG_DEBUG()<<index1->toStringFull()<<std::endl;
    LOG_DEBUG()<<index2->toString()<<std::endl;
    LOG_DEBUG()<<index2->toStringFull()<<std::endl;

    if(index1==index2)
    {
      LOG_DEBUG()<<"VCards are the same";
    }
    else
    {
      LOG_DEBUG()<<"VCards are different";
    }

    OpenAB::PluginManager::getInstance().freePluginInstance(input1);
    OpenAB::PluginManager::getInstance().freePluginInstance(input2);
  }
}
