/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 # Static Library Example:
 #  - Quick build from this folder:
        g++ *.cpp -I../../../src/ ../../../src/logging/Log.cpp -o staticExample.out

 #  - Reference build (using the proper pkg-config):
        g++ *.cpp `pkg-config OpenAB_plugin --libs --cflags` -o staticExample.out

 # Shared Library Example:
 #  - Step1: Build all the plugins in a single shared library "libPlugin.so"
        g++ plugin*.cpp `pkg-config OpenAB_plugin --libs --cflags` -shared -fPIC -o libPlugin.so

 #  - Step2: Build the executable enabling the shared library loading code "EXAMPLE_SHARED"
        g++ main.cpp genericInterface.cpp \
            `pkg-config OpenAB_plugin --libs --cflags` \
            -DEXAMPLE_SHARED \
            -o sharedExample.out
*/

#include <dlfcn.h>
#include <iostream>
#include "genericInterface.hpp"
#include <OpenAB.hpp>

int main(int argc, char* argv[])
{
#ifdef EXAMPLE_SHARED
  OpenAB_Log::Log::OutLevel() = OpenAB_Log::Log::Debug;
  //Scan directory to find plugins
  OpenAB::PluginManager::getInstance().scanDirectory(".");
#endif

  /* The parameters required by the new instance must be defined here */
  GenericParams p;

  //Ask PluginManager to create new instance of plugin
  GenericInterface * i1 = OpenAB::PluginManager::getInstance().getPluginInstance<GenericInterface>("Plugin1", p);
  /* The Generic Interface, through the "Plugin1" can now be used */
  i1->doSomething();
  i1->doSomethingElse();
  //Inform PluginManager that you are no longer using plugin instance
  OpenAB::PluginManager::getInstance().freePluginInstance(i1);

  GenericInterface * i2 = OpenAB::PluginManager::getInstance().getPluginInstance<GenericInterface>("Plugin2", p);
  i2->doSomething();
  i2->doSomethingElse();
  OpenAB::PluginManager::getInstance().freePluginInstance(i2);
}
