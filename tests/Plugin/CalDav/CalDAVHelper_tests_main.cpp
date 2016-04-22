/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file CalDAVHelper_tests_main.cpp
 */
#include <gtest/gtest.h>
#include <plugins/carddav/CalDAVHelper.hpp>
#include <helpers/BasicHttpAuthorizer.hpp>
#include <helpers/OAuth2HttpAuthorizer.hpp>

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);

  OpenAB::Logger::OutLevel() = OpenAB::Logger::Debug;
  CalDAVHelper* helper;

  int res = RUN_ALL_TESTS();

  return res;
}
