/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>
#include "genericInterface.hpp"
class Plugin2 : public GenericInterface
{
  public:
    void doSomething()
    {
      std::cout << "I'm doing something on Plugin2" << std::endl;
    }
    void doSomethingElse()
    {
      std::cout << "I'm doing something else on Plugin2" << std::endl;
    }
};
class Plugin2Factory : GenericFactory
{
  public:
    Plugin2Factory() : GenericFactory::Factory("Plugin2") {};

    ~Plugin2Factory(){}

    GenericInterface * newIstance(const GenericParams & params)
    {
      return new Plugin2();
    };
};

REGISTER_PLUGIN_FACTORY(Plugin2Factory)
