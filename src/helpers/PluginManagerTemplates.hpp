/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin/storage/Storage.hpp"
#include "plugin/source/Source.hpp"

namespace OpenAB
{
template<typename __C, typename __P>
__C* PluginManager::getPluginInstance(const std::string& pluginName, const __P& params)
{
  __C* newInstance = NULL;
  if (NULL == OpenAB_Plugin::Factory< __C , __P >::factories[pluginName])
  {
    if (isPluginAvailable(pluginName))
    {
      LOG_INFO() << "Loading module "<<pluginsInfo[pluginName]<<std::endl;
      loadModule(pluginsInfo[pluginName]);
    }
  }

  if (NULL != OpenAB_Plugin::Factory< __C , __P >::factories[pluginName])
  {
    loadedModulesInfo[pluginsInfo[pluginName]].refCount++;
    newInstance = OpenAB_Plugin::Factory< __C , __P >::factories[pluginName]->newIstance(params);
    if (newInstance)
    {
      pluginInstancesInfo.insert(std::pair<void*, std::string>(newInstance, pluginName));
    }
  }

  return newInstance;
}

template<typename __C>
void PluginManager::freePluginInstance(__C* instance)
{
  if (pluginInstancesInfo.find(instance) == pluginInstancesInfo.end())
  {
    LOG_ERROR() << "Plugin instance "<<(long long)instance<<" was not created using getPluginInstance"<<std::endl;
    return;
  }
  std::string moduleName = getPluginModuleName(pluginInstancesInfo[instance]);
  loadedModulesInfo[moduleName].refCount--;
  pluginInstancesInfo.erase(instance);
  delete instance;
  if (loadedModulesInfo[moduleName].refCount <= 0)
  {
    LOG_INFO() << "Unloading module "<<moduleName<<std::endl;
    // Find a way to unregister gobject types, so module can be safetly unloaded.
    //unloadModule(moduleName);
  }
}

}
