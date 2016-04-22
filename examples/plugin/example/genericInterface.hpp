/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <plugin/plugin.hpp>
#include <string>

/* Generic interface
 *
 * providing two virtual functions:
 * doDomething
 * doSomethingElse
 *
 * These two API will be implemented inside the plugins.
 *
 * NOTE:
 * A possible standard implementation may be included here
 * or in the genericinterface.cpp file. */
class GenericInterface
{
  public:
    GenericInterface(){};
    virtual void doSomething() = 0;
    virtual void doSomethingElse() = 0;
};

/* Generic Params
 *
 * Is the definition of all the parameters used to create the Generic Interface.
 * To simplify this example (considering that no parameters are required)
 * a simple std::string is used. */
typedef std::string GenericParams;

/*
 * Declare plugin interface, after that new interface instances can be created with:
 * OpenAB::PluginManager::getInstance().getPluginInstance<GenericInterface>("PluginName", <params>);
 */
DECLARE_PLUGIN_INTERFACE(, GenericInterface, GenericParams);
