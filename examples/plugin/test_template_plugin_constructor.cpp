/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file test_template_plugin_constructor.cpp
 *
 * to build:
 *   g++ test_template_plugin_constructor.cpp `pkg-config OpenAB --libs --cflags` -o test_template_plugin_constructor.out
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <plugin/plugin.hpp>

#include <OpenAB.hpp>

namespace {

/**
 * Definition of first testing "A" interface
 */
  class Obj_A{
    public:
      Obj_A(std::string name):name(name){};
      void doIt(){printf("Name: %s\n",name.c_str());};
    private:
      std::string name;
  };

  struct Obj_A_params{
    int a;
    int b;
    std::string c;
  };

  typedef OpenAB_Plugin::Factory< Obj_A , Obj_A_params > FactoryTest;

  template<>
  FactoryTest::typeList FactoryTest::factories;
}



namespace {

  /**
   * implementation of "TestFactory1" for the "A" interface
   */
  class FactoryN1: public FactoryTest{
    public:
      FactoryN1():FactoryTest("TestFactory1"){};
      Obj_A * newIstance(Obj_A_params &p){return NULL;};
  }factoryn1;

  /**
   * implementation of "TestFactory2" for the "A" interface
   */
  class FactoryN2: public FactoryTest{
    public:
      FactoryN2():FactoryTest("TestFactory2"){};
      Obj_A * newIstance(Obj_A_params &p){return NULL;};
  }factoryn2;



  class Obj_A_3 : public Obj_A{
    public:
      Obj_A_3():Obj_A("Test_Obj_A_3"){};
  };

  /**
   * implementation of "TestFactory3" for the "A" interface
   */
  class FactoryN3: public FactoryTest{
    public:
      FactoryN3():FactoryTest("TestFactory3"){};
      Obj_A * newIstance(Obj_A_params &p){return new Obj_A_3();};
  }factoryn3;
}


int main(int argc, char* argv[])
{

  printf("PreInit\n");
  OpenAB::OpenAB_init();
  printf("PostInit\n");

  printf("\nDatabases Plugins:\n");

  if (NULL == FactoryTest::factories["TestFactory1"])
  {
    printf("Not Exist");
  }
  else
  {
    printf("Factory %s\n", FactoryTest::factories["TestFactory1"]->getName().c_str());
  }
  FactoryTest::factories["test"];

  Obj_A_params p;
  FactoryTest::factories["TestFactory3"]->newIstance(p)->doIt();
}

