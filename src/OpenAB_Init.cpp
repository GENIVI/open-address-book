/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file OpenAB_Init.cpp
 */

#include <dirent.h>
#include <stdio.h>
#include <list>
#include <dlfcn.h>
#include <unistd.h>
#include <string>
#include <iostream>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "OpenAB.hpp"

namespace OpenAB {

void OpenAB_init()
{
  Logger::OutLevel() = Logger::Debug;
  LOG_FUNC();
  PluginManager& pluginManager = PluginManager::getInstance();
  pluginManager.scanDirectory(pluginManager.getDefaultModulesDirectory());

  PIMContactItemIndex::addCheck("n_family", PIMItemIndex::PIMItemCheck::eKey);
  PIMContactItemIndex::addCheck("n_given", PIMItemIndex::PIMItemCheck::eKey);
  PIMContactItemIndex::addCheck("n_middle", PIMItemIndex::PIMItemCheck::eKey);
  PIMContactItemIndex::addCheck("n_prefix", PIMItemIndex::PIMItemCheck::eConflict);
  PIMContactItemIndex::addCheck("n_suffix", PIMItemIndex::PIMItemCheck::eConflict);
  PIMContactItemIndex::addCheck("tel", PIMItemIndex::PIMItemCheck::eConflict);
  PIMContactItemIndex::addCheck("email", PIMItemIndex::PIMItemCheck::eConflict);
  PIMContactItemIndex::addCheck("adr", PIMItemIndex::PIMItemCheck::eConflict);
  PIMContactItemIndex::addCheck("role", PIMItemIndex::PIMItemCheck::eConflict);
  PIMContactItemIndex::addCheck("title", PIMItemIndex::PIMItemCheck::eConflict);
  PIMContactItemIndex::addCheck("nickname", PIMItemIndex::PIMItemCheck::eConflict);
  PIMContactItemIndex::addCheck("photo", PIMItemIndex::PIMItemCheck::eConflict);
  PIMContactItemIndex::addCheck("bday", PIMItemIndex::PIMItemCheck::eConflict);
  PIMContactItemIndex::addCheck("geo", PIMItemIndex::PIMItemCheck::eConflict);
  PIMContactItemIndex::addCheck("org", PIMItemIndex::PIMItemCheck::eConflict);
  PIMContactItemIndex::addCheck("note", PIMItemIndex::PIMItemCheck::eConflict);
  PIMContactItemIndex::addCheck("url", PIMItemIndex::PIMItemCheck::eConflict);
  PIMContactItemIndex::addCheck("categories", PIMItemIndex::PIMItemCheck::eConflict);

#ifdef AB_CALENDAR_ENABLED
  PIMCalendarItemIndex::addCheck("uid", PIMItemIndex::PIMItemCheck::eKey);
  //PIMCalendarItemIndex::addCheck("created", PIMItemIndex::PIMItemCheck::eConflict);
  PIMCalendarItemIndex::addCheck("attendee", PIMItemIndex::PIMItemCheck::eConflict);
  PIMCalendarItemIndex::addCheck("description", PIMItemIndex::PIMItemCheck::eConflict);
  PIMCalendarItemIndex::addCheck("summary", PIMItemIndex::PIMItemCheck::eConflict);
  PIMCalendarItemIndex::addCheck("dtstart", PIMItemIndex::PIMItemCheck::eConflict);
  PIMCalendarItemIndex::addCheck("dtend", PIMItemIndex::PIMItemCheck::eConflict);
  PIMCalendarItemIndex::addCheck("attach", PIMItemIndex::PIMItemCheck::eConflict);
  PIMCalendarItemIndex::addCheck("sequence", PIMItemIndex::PIMItemCheck::eConflict);
  //PIMCalendarItemIndex::addCheck("dtstamp", PIMItemIndex::PIMItemCheck::eConflict);
  PIMCalendarItemIndex::addCheck("location", PIMItemIndex::PIMItemCheck::eConflict);
#endif
}

}

