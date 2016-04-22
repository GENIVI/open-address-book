/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file OpenAB.hpp
 */

#ifndef OpenAB_LIBRARY_HPP
#define OPENAB_LIBRARY_HPP

#include <string>

#include <helpers/Log.hpp>
#include <PIMItem/PIMItem.hpp>
#include <PIMItem/Contact/PIMContactItem.hpp>
#include <PIMItem/Calendar/PIMCalendarItem.hpp>
#include <PIMItem/Calendar/PIMCalendarItemIndex.hpp>
#include <PIMItem/Contact/PIMContactItemIndex.hpp>
#include <plugin/storage/Storage.hpp>
#include <plugin/source/Source.hpp>
#include <plugin/sync/Sync.hpp>
#include <helpers/Variant.hpp>
#include <helpers/PluginManager.hpp>
#include <helpers/SmartPtr.hpp>
#include <helpers/StringHelper.hpp>
#include <helpers/TimeStamp.hpp>

namespace OpenAB {
/**
 * @brief Perform the default initialization steps required by OpenAB
 *
 * The basic initialization consist of:
 * @li Initialization of the log level (def=Info)
 * @code{.cpp}
 *    AB_Log::Log::OutLevel() = OpenAB_Log::Log::Info;
 * @endcode
 *
 * @li Scan of the plugin folder ({prefix}/lib/OpenAB/\*.so)
 *
 * @code{.cpp}
 *    OpenAB::PluginManager& pluginManager = OpenAB::PluginManager::getInstance();
 *    pluginManager.scanDirectory(pluginManager.getDefaultModulesDirectory());
 * @endcode
 *
 * @li Definition of OpenAB::PIMItemIndex::PIMItemCheck for OpenAB::PIMItem classes
 *
 * @code{.cpp}
 *    OpenAB::PIMContactItemIndex::addCheck("fn", OpenAB::PIMItemIndex::PIMItemCheck::eKey);
 *    OpenAB::PIMContactItemIndex::addCheck("tel", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
 *    OpenAB::PIMContactItemIndex::addCheck("email", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
 *    OpenAB::PIMContactItemIndex::addCheck("adr", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
 *    OpenAB::PIMContactItemIndex::addCheck("role", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
 *    OpenAB::PIMContactItemIndex::addCheck("title", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
 *    OpenAB::PIMContactItemIndex::addCheck("nickname", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
 *    OpenAB::PIMContactItemIndex::addCheck("photo", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
 *    OpenAB::PIMContactItemIndex::addCheck("bday", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
 *    OpenAB::PIMContactItemIndex::addCheck("geo", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
 *    OpenAB::PIMContactItemIndex::addCheck("org", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
 *    OpenAB::PIMContactItemIndex::addCheck("note", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
 *    OpenAB::PIMContactItemIndex::addCheck("url", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
 *    OpenAB::PIMContactItemIndex::addCheck("categories", OpenAB::PIMItemIndex::PIMItemCheck::eConflict);
 * @endcode
 *
 * @note OpenAB_init is not mandatory. These steps can be easily integrated in the code to allow any degree of customization (see: test/plugin/example/main.cpp).
 *
 *
 **/
void OpenAB_init();

}

#endif /* OpenAB_LIBRARY_HPP */
