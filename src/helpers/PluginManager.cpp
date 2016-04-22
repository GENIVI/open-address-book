/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file PluginManager.cpp
 */
#include <helpers/Log.hpp>
#include <dlfcn.h>
#include <dirent.h>
#include <list>
#include <unistd.h>
#include <string>
#include "PluginManager.hpp"

#include "plugin/storage/Storage.hpp"
#include "plugin/source/Source.hpp"
#include "plugin/sync/Sync.hpp"

namespace OpenAB {

PluginManager PluginManager::instance;

std::vector<std::string> PluginManager::loadedPlugins;

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
  std::map<void*, std::string>::iterator it;
  if(pluginInstancesInfo.size() > 0)
  {
    //we cannot free instances here as we don't know exact type of plugins
    LOG_WARNING()<<"There are still not freed plugin instances"<<std::endl;
  }
}

void PluginManager::pluginLoadedCb(const std::string& name)
{
  loadedPlugins.push_back(name);
}

void PluginManager::scanDirectory(const std::string& path)
{
  //listen for loading callback of known types of plugins
  OpenAB_Source::Factory::setPluginLoadedCallback(pluginLoadedCb);
  OpenAB_Storage::Factory::setPluginLoadedCallback(pluginLoadedCb);
  OpenAB_Sync::Factory::setPluginLoadedCallback(pluginLoadedCb);

  DIR* d;
  struct dirent *dir;
  std::list<void *> dl;

  //scan directory, load all modules and check if they are providing any known plugins.
  d = opendir(path.c_str());
  if (d)
  {
    while (NULL != (dir = readdir(d)))
    {
      std::string name = dir->d_name;

      if (name.size() > 18 &&
          0 == name.compare(name.size()-3, 3, ".so") &&
          0 == name.compare(0, 14, "libOpenAB_plugin_" ) )
      {
        LOG_DEBUG()<< "Found module: " << dir->d_name<<std::endl;

        loadedPlugins.clear();

        name.insert(0,path + std::string("/"));
        LOG_DEBUG() << "Checking plugins provided by module" << name<<std::endl;
        void * dlib = dlopen(name.c_str(), RTLD_NOW);
        if (dlib == NULL)
        {
          LOG_ERROR() << dlerror()<<std::endl;
          continue;
        }
        LOG_DEBUG() << " - " << name << " Loaded";
        dlclose(dlib);

        std::vector<std::string>::iterator it;
        for (it = loadedPlugins.begin(); it != loadedPlugins.end(); ++it)
        {
          pluginsInfo.insert(std::pair<std::string, std::string>((*it), name));
        }
      }
    }
  }
  else
  {
    LOG_ERROR()<<"Cannot open directory "<<path<<std::endl;
  }
  OpenAB_Source::Factory::setPluginLoadedCallback(NULL);
  OpenAB_Storage::Factory::setPluginLoadedCallback(NULL);
  OpenAB_Sync::Factory::setPluginLoadedCallback(NULL);

  std::map<std::string, std::string>::iterator it = pluginsInfo.begin();
  LOG_INFO()<<"Available plugins:"<<std::endl;
  for(; it != pluginsInfo.end(); ++it)
  {
    LOG_INFO()<<(*it).first<<" from "<<(*it).second<<std::endl;
  }

  closedir(d);
}

std::string PluginManager::getDefaultModulesDirectory() const
{
  std::string path;
#ifdef PKGDIR
  path = PKGDIR;
#else
  path = "./";
#endif
  return path;
}

bool PluginManager::isPluginAvailable(const std::string& pluginName) const
{
  if (pluginsInfo.find(pluginName) != pluginsInfo.end())
    return true;
  return false;
}

std::map<std::string, std::string> PluginManager::getListOfPlugins() const
{
  return pluginsInfo;
}

bool PluginManager::loadModule(const std::string& modulePath)
{
  if (loadedModulesInfo.find(modulePath) != loadedModulesInfo.end())
  {
    LOG_INFO()<<"Module "<<modulePath<<" is already loaded"<<std::endl;
    return true;
  }
  void * module = dlopen(modulePath.c_str(), RTLD_NOW);
  if (module == NULL)
  {
    LOG_ERROR() << "Cannot load " <<modulePath<<" module: "<<dlerror()<<std::endl;
    return false;
  }
  LoadedModulesInfo newModuleInfo(module);
  loadedModulesInfo.insert(std::pair<std::string, LoadedModulesInfo>(modulePath, newModuleInfo));

  return true;
}

bool PluginManager::unloadModule(const std::string& modulePath)
{
  if (loadedModulesInfo.find(modulePath) == loadedModulesInfo.end())
  {
    LOG_ERROR() << "Module "<<modulePath<<"is not loaded"<<std::endl;
    return false;
  }

  if (loadedModulesInfo[modulePath].refCount > 0)
  {
    LOG_ERROR() << "Plugin instances provide by "<<modulePath<<" module are still in use"<<std::endl;
    return false;
  }

  dlclose(loadedModulesInfo[modulePath].module);
  loadedModulesInfo.erase(modulePath);

  return true;
}

std::string PluginManager::getPluginModuleName(const std::string& pluginName) const
{
  PluginsInfo::const_iterator it = pluginsInfo.find(pluginName);
  if (it != pluginsInfo.end())
    return (*it).second;
  return std::string();
}
PluginManager& PluginManager::getInstance()
{
  return instance;
}


} // namespace OpenAB
