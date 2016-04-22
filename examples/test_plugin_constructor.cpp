/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file test_plugin_constructor.cpp
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <plugin/addressbook/Addressbook.hpp>
#include <plugin/input/Input.hpp>

#include <OpenAB.hpp>

int main(int argc, char* argv[])
{

  printf("PreInit\n");
  OpenAB::OpenAB_init();
  printf("PostInit\n");

  printf("\nAddressbook Plugins:\n");
  for(OpenAB_AB::Factory::typeList::iterator it =
      OpenAB_AB::Factory::factories.begin();
      it != OpenAB_AB::Factory::factories.end();
      ++it){
    printf("%s\n",it->getName().c_str());
  }

  printf("\nInput Plugins:\n");
  for(OpenAB_Input::Factory::typeList::iterator it =
      OpenAB_Input::Factory::factories.begin();
      it != OpenAB_Input::Factory::factories.end();
      ++it){
    printf("%s\n",it->getName().c_str());
  }

}

