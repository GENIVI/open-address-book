/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file test_icalendar_parsing.cpp
 * @include test_icalendar_parsing.cpp
 */

/*
 # Build:
   g++ test_icalendar_parsing.cpp `pkg-config OpenAB --libs --cflags` -o test_icalendar_parsing.out
 */

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>

#include <OpenAB.hpp>

int main(int argc, char* argv[])
{

  OpenAB::OpenAB_init();

  if (argc < 3)
  {
    printf("Use %s <icalendar file> <icalendar file>\n",argv[0]);
  }
  else
  {
    char * filename1 = argv[1];
    char * filename2 = argv[2];
    /*OpenAB_Source::Parameters params;
    params.setValue("filename", filename);

    OpenAB_Source::Source * input1 = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>(std::string("File"), params);
    input1->init();

    OpenAB::SmartPtr<OpenAB::PIMItem> contact1;
    input1->getItem(contact1);*/

    OpenAB::PIMCalendarEventItem item;
    OpenAB::PIMCalendarEventItem item2;

    std::string line;
    std::string iCalendar1;
    std::ifstream iCalendarFile1(filename1);
    if (iCalendarFile1.is_open())
    {
      while (std::getline(iCalendarFile1, line))
      {
        iCalendar1 += line;
        iCalendar1 += "\n";
      }
      iCalendarFile1.close();
    }
    else
    {
      printf("Cannot open %s\n", filename1);
      return 1;
    }

    std::string iCalendar2;
    std::ifstream iCalendarFile2(filename2);
    if (iCalendarFile2.is_open())
    {
      while (std::getline(iCalendarFile2, line))
      {
        iCalendar2 += line;
        iCalendar2 += "\n";
      }
      iCalendarFile2.close();
    }
    else
    {
      printf("Cannot open %s\n", filename2);
      return 1;
    }

    item.parse(iCalendar1);

    item2.parse(iCalendar2);

    OpenAB::SmartPtr<OpenAB::PIMItemIndex> idx1 = item.getIndex();
    OpenAB::SmartPtr<OpenAB::PIMItemIndex> idx2 = item2.getIndex();

    LOG_DEBUG()<<"Index 1: "<<idx1->toString()<<std::endl;
    LOG_DEBUG()<<"Index 1 (full): "<<idx1->toStringFull()<<std::endl;

    LOG_DEBUG()<<"Index 2: "<<idx2->toString()<<std::endl;
    LOG_DEBUG()<<"Index 2 (full): "<<idx2->toStringFull()<<std::endl;

    if (idx1 == idx2)
    {
      LOG_DEBUG()<<"items are matching"<<std::endl;

      if (idx1->compare(*idx2))
      {
        LOG_DEBUG()<<"items are the same"<<std::endl;
      }
    }
  }
}
