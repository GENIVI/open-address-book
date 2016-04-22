/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file oab_source_pbap_tests_main.cpp
 */
#include <gtest/gtest.h>
#include "helpers/PluginManager.hpp"
#include "PIMItem/Contact/PIMContactItem.hpp"

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);

  OpenAB::Logger::OutLevel() = OpenAB::Logger::Debug;
  OpenAB::Logger::setDefaultLogger(NULL);
  printf ("%p\n", OpenAB::Logger::getDefaultLogger());
  OpenAB::PIMContactItem item;

  OpenAB_Source::Parameters p;
  p.setValue("MAC", "12:34:56:78:90:12");
  OpenAB::PluginManager::getInstance().scanDirectory("../src/.libs");
  OpenAB_Source::Source* s = OpenAB::PluginManager::getInstance().getPluginInstance<OpenAB_Source::Source>("PBAP", p);

  int res = RUN_ALL_TESTS();

  OpenAB::PluginManager::getInstance().freePluginInstance(s);
  return res;
}
