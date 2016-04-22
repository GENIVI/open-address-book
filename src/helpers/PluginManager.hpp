/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file PluginManager.hpp
 */

#ifndef PLUGINMANAGER_H_
#define PLUGINMANAGER_H_

#include <string>
#include <vector>
#include <map>

/*!
 * @brief namespace OpenAB
 */
namespace OpenAB {
/**
 * @brief PluginManager provides way to manage OpenAB plugins.
 * Allows to discover modules that are providing OpenAB plugins, and load/unload them.
 */
class PluginManager
{
  public:

    /*!
     * @brief Returns instance of PluginManager.
     */
    static PluginManager& getInstance();

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~PluginManager();

    /*!
     * @brief Scans directory for OpenAB modules.
     * Updates list of available plugins.
     * @note This function does not load the modules.
     *
     * @param [in] path path of dictionary to scan
     */
    void scanDirectory(const std::string& path);

    /*!
     * @brief Returns default location of OpenAB modules.
     *
     * @returns path to default OpenAB modules directory
     */
    std::string getDefaultModulesDirectory() const;

    /*!
     * @brief List all available plugins with information about which modules are providing them.
     * Keys in returned map are pluigins names and their
     * values are modules names that are providing them
     * e.g. PBAP /usr/lib/OpenAB/libOpenAB_plugin_input_pbap.so
     *
     * @returns Map of all available plugins
     */
    std::map<std::string, std::string> getListOfPlugins() const;

    /*!
     * @brief Checks if given plugin is provided by any module.
     *
     * @param [in] pluginName name of plugin which availability should be checked
     * @returns true if plugin is provided by one of the modules, false otherwise.
     */
    bool isPluginAvailable(const std::string& pluginName) const;

    /*!
     * @brief Creates instance of a plugin using plugin specific parameters to initialize it.
     * @note This is preferred way for creating new instances of plugins,
     * as it automatically keeps track of modules that are providing plugins,
     * loading and unloading them on demand, keeping memory footprint as low as possible
     *
     * Example of use: OpenAB_Input::Input* instance = getPluginInstance<OpenAB_Input::Input>("PBAP", params);
     *
     * @param [in] pluginName name of plugin which instance should be created
     * @param [in] params plugin specific parameters for initializng new instance
     * @returns newly created instance of plugin. When no longer needed it should be freed using freePluginInstance().
     */
    template<typename __C, typename __P>
    __C* getPluginInstance(const std::string& pluginName, const __P& params);

    /*!
     * @brief Frees plugin instance created by getPluginInstance function.
     * Apart from removing instance it will check if module that was providing
     * given instance of plugin is still required to be keep in memory.
     *
     * @param [in] instance instance of plugin to be freed
     */
    template<typename __C>
    void freePluginInstance(__C* instance);

#ifdef TESTING
    void clean()
    {
      pluginsInfo.clear();
      loadedModulesInfo.clear();
      pluginInstancesInfo.clear();
    }

    std::map<void*, std::string> getPluginInstancesInfo()
    {
      return pluginInstancesInfo;
    }

    std::vector<std::string> getLoadedModules()
    {
      std::vector<std::string> result;
      std::map<std::string, LoadedModulesInfo>::iterator it;
      for (it = loadedModulesInfo.begin(); it != loadedModulesInfo.end(); ++it)
      {
        result.push_back((*it).first);
      }

      return result;
    }

#endif
  private:
    /*!
     *  @brief Private constructor, singleton class.
     */
    PluginManager();

    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    PluginManager(PluginManager const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    PluginManager& operator=(PluginManager const &other);

    /*!
     * @brief Loads OpenAB module.
     *
     * @param [in] modulePath path for module to be loaded.
     * @returns true if module was loaded successfully, false otherwise.
     */
    bool loadModule(const std::string& modulePath);

    /*!
     * @brief Unloads OpenAB module.
     *
     * @param [in] modulePath path for module to be unloaded.
     * @returns true if module was unloaded successfully, false otherwise.
     */
    bool unloadModule(const std::string& modulePath);

    /*!
     * @brief Returns path for module that is providing given plugin
     * @note please ensure before calling using isPluginAvailable that plugin is provided by any known module
     *
     * @param [in] pluginName name of plugin for which module name should be returned
     * @returns path for module providing given plugin, or empty string if plugin is not provided by any known module
     */
    std::string getPluginModuleName(const std::string& pluginName) const;

    /*!
     * @brief Callback function called by OpenAB_Plugin::Factory after loading new plugin
     *
     * @param [in] name name of newly loaded plugin
     */
    static void pluginLoadedCb(const std::string& name);

    static std::vector<std::string> loadedPlugins;


    typedef std::map<std::string, std::string> PluginsInfo;

    /*!
     * @brief Map of all know plugins.
     * Keys in this map are plugin names and values are modules path providing given plugin
     */
    PluginsInfo pluginsInfo;

    struct LoadedModulesInfo
    {
        LoadedModulesInfo() :
          refCount(0),
          module(NULL)
        {}

        LoadedModulesInfo(void* p) :
          refCount(0),
          module(p)
        {}

        unsigned int refCount;
        void* module;
    };

    /*!
     * @brief Map of all modules currently loaded in memory.
     * Keys in this map are modules path and values are LoadedModulesInfo entries,
     * holding info about module itself and its reference count
     */
    std::map<std::string, LoadedModulesInfo> loadedModulesInfo;

    /*!
     * @brief Map of all created instances of plugins.
     * Keys in this map are instances pointers and values are plugin names.
     */
    std::map<void*, std::string> pluginInstancesInfo;

    static PluginManager instance;
};

} // namespace OpenAB
#include <helpers/PluginManagerTemplates.hpp>

#endif // PLUGINMANAGER_H_
