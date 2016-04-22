/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */
/**
 * @file pluginManager_tests.cpp
 */
#include <gtest/gtest.h>
#include <string>
#include "helpers/PluginManager.hpp"
#include "TestLogger.hpp"

class PluginManagerTests: public ::testing::Test
{
public:
    PluginManagerTests() : ::testing::Test()
    {
    }

    ~PluginManagerTests()
    {
    }

protected:
    // Sets up the test fixture.
    virtual void SetUp()
    {
      OpenAB::PluginManager::getInstance().clean();
    }

    // Tears down the test fixture.
    virtual void TearDown()
    {

    }

};

TEST_F(PluginManagerTests, scanNonExistingDir)
{
  OpenAB::PluginManager& instance = OpenAB::PluginManager::getInstance();
  std::map<std::string, std::string> plugins  = instance.getListOfPlugins();
  //assure that there are no plugins discovered yet
  ASSERT_EQ(0, plugins.size());
  instance.scanDirectory("/someNonExistingDirectory");
  plugins  = instance.getListOfPlugins();
  ASSERT_EQ(0, plugins.size());
  ASSERT_FALSE(instance.isPluginAvailable("TestSourcePlugin"));
}

TEST_F(PluginManagerTests, scanTestPlugins)
{
  OpenAB::PluginManager& instance = OpenAB::PluginManager::getInstance();
  std::map<std::string, std::string> plugins  = instance.getListOfPlugins();
  //assure that there are no plugins discovered yet
  ASSERT_EQ(0, plugins.size());
  ASSERT_FALSE(instance.isPluginAvailable("TestSourcePlugin"));
  instance.scanDirectory(".libs");
  plugins = instance.getListOfPlugins();
  //now some plugins should have been discovered
  ASSERT_TRUE(plugins.size() > 0);
  ASSERT_TRUE(instance.isPluginAvailable("TestSourcePlugin"));
}

TEST_F(PluginManagerTests, DISABLED_testPluginCreation)
{
  OpenAB::PluginManager& instance = OpenAB::PluginManager::getInstance();
  std::map<std::string, std::string> plugins  = instance.getListOfPlugins();
  //assure that there are no plugins discovered yet
  ASSERT_EQ(0, plugins.size());
  ASSERT_FALSE(instance.isPluginAvailable("TestSourcePlugin"));

  OpenAB_Source::Parameters param;
  //try to create plugin instance - it will fail as no plugins were discovered yey
  OpenAB_Source::Source* sourceInstance =
      instance.getPluginInstance<OpenAB_Source::Source>("TestSourcePlugin", param);
  ASSERT_EQ(NULL, sourceInstance);

  //assure that no module was loaded and no instance was created
  ASSERT_EQ(0, instance.getLoadedModules().size());
  ASSERT_EQ(0, instance.getPluginInstancesInfo().size());


  OpenAB_TESTS::TestLogger logger;
  OpenAB::Logger::setDefaultLogger(&logger);
  //try to free invalid plugin instance - there should be error message logged.
  instance.freePluginInstance(sourceInstance);
  ASSERT_EQ("  Error : Plugin instance 0 was not created using getPluginInstance",
            logger.lastMessage);

  OpenAB::Logger::setDefaultLogger(NULL);

  instance.scanDirectory(".libs");
  ASSERT_TRUE(instance.isPluginAvailable("TestSourcePlugin"));

  //try creating plugin instance after scanning plugins
  sourceInstance =
        instance.getPluginInstance<OpenAB_Source::Source>("TestSourcePlugin", param);
  ASSERT_TRUE(NULL != sourceInstance);

  //assure that plugin module is loaded into memory and instance was created
  ASSERT_EQ(1, instance.getLoadedModules().size());
  ASSERT_EQ(1, instance.getPluginInstancesInfo().size());

  //free instance
  instance.freePluginInstance(sourceInstance);

  //now there should be no module loaded into memory and no instances
  ASSERT_EQ(0, instance.getLoadedModules().size());
  ASSERT_EQ(0, instance.getPluginInstancesInfo().size());
}

TEST_F(PluginManagerTests, DISABLED_testModulesReferenceCounting)
{
  OpenAB::PluginManager& instance = OpenAB::PluginManager::getInstance();
  instance.scanDirectory(".libs");
  ASSERT_TRUE(instance.isPluginAvailable("TestSourcePlugin"));

  ASSERT_EQ(0, instance.getLoadedModules().size());
  ASSERT_EQ(0, instance.getPluginInstancesInfo().size());


  OpenAB_Source::Parameters param;
  //create two plugin instances
  OpenAB_Source::Source* sourceInstance1 =
        instance.getPluginInstance<OpenAB_Source::Source>("TestSourcePlugin", param);
  ASSERT_TRUE(NULL != sourceInstance1);

  OpenAB_Source::Source* sourceInstance2 =
          instance.getPluginInstance<OpenAB_Source::Source>("TestSourcePlugin", param);
  ASSERT_TRUE(NULL != sourceInstance2);

  //there should be one module loaded and two instances created
  ASSERT_EQ(1, instance.getLoadedModules().size());
  ASSERT_EQ(2, instance.getPluginInstancesInfo().size());

  //free one of instances, module still should be in memory as sourceInstance1 is still valid
  instance.freePluginInstance(sourceInstance2);

  ASSERT_EQ(1, instance.getLoadedModules().size());
  ASSERT_EQ(1, instance.getPluginInstancesInfo().size());

  //free second instance, now module should been unloaded from memory
  instance.freePluginInstance(sourceInstance1);
  ASSERT_EQ(0, instance.getLoadedModules().size());
  ASSERT_EQ(0, instance.getPluginInstancesInfo().size());
}

TEST_F(PluginManagerTests, testDefaultPluginDir)
{
  OpenAB::PluginManager& instance = OpenAB::PluginManager::getInstance();
  ASSERT_NE("", instance.getDefaultModulesDirectory());
}
