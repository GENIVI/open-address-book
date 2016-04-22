/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>
#include "genericInterface.hpp"

/* Plugin1 class
 * This class implement/extend the GenericInterface
 * defined in "genericinterface.hpp". */
class Plugin1 : public GenericInterface
{
  public:
    void doSomething()
    {
      std::cout << "I'm doing something on Plugin1" << std::endl;
    }
    void doSomethingElse()
    {
      std::cout << "I'm doing something else on Plugin1" << std::endl;
    }
};


/* The Factory specialization for this plugin
 * is the base of the Plugin usage.
 * It must provide 3 initialization steps. */
class Plugin1Factory : GenericFactory
{
  public:
    /* Step1: Definition of the Plugin Name in the constructor */
    Plugin1Factory() : GenericFactory::Factory("Plugin1"){};

    ~Plugin1Factory(){}

    /* Step2: newIstance routine where eventually
     *   the chosen parameters can be used*/
    GenericInterface * newIstance(const GenericParams & params)
    {
      return new Plugin1();
    };
};

/* Step3: adding the factory "Plugin1"
 * trasparently to the (GenericFactory) Factories*/
REGISTER_PLUGIN_FACTORY(Plugin1Factory)


