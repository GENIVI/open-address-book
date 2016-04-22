/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file Sync.hpp
 */

#ifndef SYNC_HPP_
#define SYNC_HPP_

#include <map>
#include <string>
#include <vector>
#include <plugin/Plugin.hpp>
#include <plugin/GenericParameters.hpp>
#include <plugin/source/Source.hpp>
#include <plugin/storage/Storage.hpp>

/*!
 * @brief namespace Sync
 */
namespace OpenAB_Sync {

/**
 * @defgroup SyncPlugin Sync Plugin
 * @ingroup pluginGroup
 *
 * Sync Interface: @ref Sync
 *
 ## Overview ##
 *
 * The Sync plugin interface provides all the required functionalities to
 * synchronize PIM Items (@ref OpenAB::PIMItem) from either between Source (@ref OpenAB_Source::Source) and Storage (@ref OpenAB_Storage::Storage) or two Storage plugins.
 * Sync plugins are expected to add/remove/modify contents of Storage based on Source/ another Storage data.
 * Sync plugins are item type independent.
 *
 ##API Details ##
 *
 ### Sync functionalities ###
 *
 * Synchronizing items is main functionality required from Sync:
 *  - @ref Sync::synchronize (@copybrief Sync::synchronize)
 *
 * Additionaly Sync can support set of additional functionalities (which depends of support of them in used OpenAB_Source::Source plugin):
 *  - @ref Sync::suspend (@copybrief Sync::suspend)
 *  - @ref Sync::resume (@copybrief Sync::resume)
 *  - @ref Sync::cancel (@copybrief Sync::cancel)
 *
 *
 ### Quick Browse functions ###
 * To synchronize contents of OpenAB_Source::Source with OpenAB_Storage::Storage, Sync needs to be initialized using function:
 *  - @ref Sync::init (@copybrief Sync::init)
 *
 * initialization will use parameters provided during creation of Sync instance
 * (@ref Parameters, @ref OpenAB::PluginManager::getPluginInstance).
 * Each Sync plugin can have its own specific parameters (please refer to given plugin documentation).
 * Sync plugins configuration need to provide parameters for source and storage plugins (please refer
 * to given plugin documentation if local and/or remote source/storage parameters are mandatory):
 *  - @ref "source_plugin" key with name of OpenAB_Source::Source plugin to be used
 *  - @ref Parameters::sourcePluginParams (@copybrief Parameters::sourcePluginParams)
 *  - @ref "storage_plugin" key with name of OpenAB_Storage::Storage plugin to be used
 *  - @ref Parameters::storagePluginParams (@copybrief Parameters::storagePluginParams)
 *
 * Next step is the definition of synchronization phases using functions:
 *  - @ref Sync::addPhase (@copybrief Sync::addPhase)
 *  - @ref Sync::clearPhases (@copybrief Sync::clearPhases)
 *
 * At least one phase needs to be defined.
 * After that synchronization can be started using @ref Sync::synchronize function.
 * If "cb" parameter was provided with valid instance of Sync::SyncCallback it will be notified
 *  about progress and results of synchronization.
 */

/**
 * @brief Generic storage for Sync plugin parameters.
 * Allows to store map of parameters with different types (@ref OpenAB::Variant)
 * and parameters specific to OpenAB_Source::Source and OpenAB_Storage::Storage plugins.
 * Additionally allow for serialization and deserialization to/from JSON format.
 */
class Parameters : public OpenAB_Plugin::GenericParameters
{
  public:
    /**
     * @brief Default constructor.
     */
    Parameters();

    /**
     * @brief Deserializes parameters from JSON sting.
     * @param [in] json JSON string to deserialize from.
     */
    Parameters(const std::string& json);

    /**
     *  @brief Destructor, virtual by default.
     */
    ~Parameters();

    /**
     * @brief Parameters to be used by local OpenAB_Source::Source plugin.
     */
    OpenAB_Source::Parameters localSourcePluginParams;

    /**
     * @brief Parameters to be used by remote OpenAB_Source::Source plugin.
     */
    OpenAB_Source::Parameters remoteSourcePluginParams;

    /**
     * @brief parameters to be used by local OpenAB_Storage::Storage plugin
     * @todo make this field private, add getter and setter for this field
     */
    OpenAB_Storage::Parameters localStoragePluginParams;

    /**
     * @brief Parameters to be used by remote OpenAB_Storage::Storage plugin.
     */
    OpenAB_Storage::Parameters remoteStoragePluginParams;
};

/*!
 * @brief Documentation for class Sync plugin interface
 */
class Sync
{
  public:
    /**
     * @brief Constructor
     */
    Sync(){}

    /**
     *  @brief Destructor, virtual by default.
     */
    virtual ~Sync(){}

    /** @enum
     * init() return code
     */
    enum eInit{
      eInitOk,    /**< Sync successfully initialized */
      eInitFail   /**< Failure during the Sync initialization */
    };

    /**
     * @brief Initializes Sync
     * @return the status code
     */
    virtual enum eInit init() = 0;

    /** @enum
     * synchronize() return code
     */
    enum eSync{
      eSyncOkWithDataChange,    /**< Synchronization finished successfully with data change in OpenAB_Storage::Storage */
      eSyncOkWithoutDataChange, /**< Synchronization finished successfully without any data change in OpenAB_Storage::Storage */
      eSyncCancelled,           /**< Synchronization was cancelled */
      eSyncAlreadyInProgress,   /**< Synchronization is already in progress */
      eSyncFail                 /**< Synchronization failed */
    };

    /**
     * @brief Synchronizes data
     */
    virtual void synchronize() = 0;

    /** @enum
     * cancel() return code
     */
    enum eCancel {
      eCancelOk,
      eCancelNotInProgress,
      eCancelFail
    };

    /**
     * @brief Cancels synchronization
     * @return the status code
     */
    virtual enum eCancel cancel() = 0;

    /** @enum
     * suspend() return code
     */
    enum eSuspend {
      eSuspendOk,
      eSuspendNotInProgress,
      eSuspendFail
    };

    /**
     * @brief Suspends synchronization (if used OpenAB_Source::Source plugin supports it)
     * @return the status code
     */
    virtual enum eSuspend suspend() = 0;

    /** @enum
     * resume() return code
     */
    enum eResume {
      eResumeOk,
      eResumeNotSuspended,
      eResumeFail
    };

    /**
     * @brief Resumes synchronization (if used OpenAB_Source::Source plugin supports it)
     * @return the status code
     */
    virtual enum eResume resume() = 0;

    /**
     * @brief Returns statistics of synchronization
     * @param [out] locallyAdded number of items added during synchronization in local storage
     * @param [out] locallyModified number of items modified during synchronization in local storage
     * @param [out] locallyRemoved number of items removed during synchronization from local storage
     * @param [out] remotelyAdded number of items added during synchronization in remote storage
     * @param [out] remotelyModified number of items modified during synchronization in remote storage
     * @param [out] remotyleRemoved number of items removed during synchronization from remote storage
     */
    virtual void getStats(unsigned int& locallyAdded,
                          unsigned int& locallyModified,
                          unsigned int& locallyRemoved,
                          unsigned int& remotelyAdded,
                          unsigned int& remotelyModified,
                          unsigned int& remotelyRemoved) = 0;

    /**
     * @brief Adds new phase to synchronization process.
     * @param [in] name name of phase
     * @param [in] ignoreFields vector of OpenAB::PIMItem fields that should be ignored by OpenAB_Source::Source plugin.
     * @note Purpose of phases is to allow more efficient synchronization.
     * @note Each phase can ignore some OpenAB::PIMItem fields provided by OpenAB_Source::Source plugin (if it supports ignore feature).
     * @note As an example during contacts synchronization one phase can download only textual information and second one also photo data.
     * @return true if phase was added successfully, false if phase with the same name already exists.
     */
    bool addPhase(const std::string& name, const std::vector<std::string>& ignoreFields);

    /**
     * @brief Removes all previously defined synchronization phases.
     */
    void clearPhases();

    /**
     * @brief Virtual class that provide all the callback routines to control the synchronization
     */
    class SyncCallback
    {
      public:
        SyncCallback(){};
        virtual ~SyncCallback(){};
        /**
         * @brief Dummy funtion used to provide debug messages from Sync.
         * @param [in] msg debug message
         */
        virtual void print(const std::string& msg) = 0;

        /**
         * @brief Informs about end of synchronization.
         * @param [in] result result of synchronization
         */
        virtual void syncFinished(const enum OpenAB_Sync::Sync::eSync& result) = 0;

        /**
         * @brief Informs about progress of synchronization.
         * @param [in] phaseName name of current synchronization phase
         * @param [in] progress percentage progress of whole synchronization process (including all phases).
         */
        virtual void syncProgress(const std::string& phaseName, double progress, unsigned int numProcessedItems) = 0;

        /**
         * @brief Informs about start of next synchronization phase.
         * @param [in] name name of phase.
         */
        virtual void syncPhaseStarted(const std::string& name) = 0;

        /**
         * @brief Informs about end of synchronization phase.
         * @param [in] name name of phase.
         */
        virtual void syncPhaseFinished(const std::string& name) = 0;

        /**
         * @brief Informs that synchronization metadata has been updated.
         * @param [in] metadata metadata information in JSON format.
         */
        virtual void metadataUpdated(const std::string& ){}
    };

  protected:
    struct Phase
    {
      Phase(const std::string& n, const std::vector<std::string> & ignore) :
        name(n),
        ignoredFields(ignore) {}

      std::string name;
      std::vector<std::string> ignoredFields;
    };
    std::vector<Phase> phases;

  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    Sync(Sync const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    Sync& operator=(Sync const &other);
};

//typedef OpenAB_Plugin::Factory< Sync , Parameters > Factory;

/*!
 * @brief Documentation for class SyncMetadata.
 * This class is designed to store and keep track of synchronization metadata information.
 */
class SyncMetadata
{
  public:
    SyncMetadata();
    ~SyncMetadata();

    /**
     * @brief Adds new record to metadata.
     * It will bind remote and local items, detected as identical.
     * @param remoteId id of remote item
     * @param remoteRevision revision of remote item
     * @param localId id of local item
     * @param localRevision revision of local item
     */
    void addItem(const std::string& remoteId, const std::string& remoteRevision,
                 const std::string& localId, const std::string& localRevision);

    /**
     * @brief Removes record from metadata.
     * It will remove information about remote and local items binding.
     * @param remoteId id of remote item
     * @param localId id of local item
     */
    void removeItem(const std::string& remoteId, const std::string& localId);

    /**
     * @brief Updates revision of remote item.
     * @param uid id of remote item
     * @param revision new revision of remote item
     */
    void updateRemoteRevision(const std::string& uid, const std::string& revision);

    /**
     * @brief Updates revision of local item.
     * @param uid id of local item
     * @param revision new revision of local item
     */
    void updateLocalRevision(const std::string& uid, const std::string& revision);

    /**
     * @brief Returns revision of remote item.
     * @param uid id of remote item.
     * @return revision of remote item,
     * or empty string if there is no metadata information about item with given id.
     */
    std::string getRemoteRevision(const std::string& uid) const;

    /**
     * @brief Returns revision of local item.
     * @param uid id of local item.
     * @return revision of local item,
     * or empty string if there is no metadata information about item with given id.
     */
    std::string getLocalRevision(const std::string& uid) const;

    /**
     * @brief Checks if there is metadata information for local item with given id.
     * @param uid id of local item.
     * @return true if there is metadata for given item, false otherwise.
     */
    bool hasLocalId(const std::string& uid) const;

    /**
     * @brief Checks if there is metadata information for remote item with given id.
     * @param uid if of remote item.
     * @return true if there is metadata for given item, false otherwise.
     */
    bool hasRemoteId(const std::string& uid) const;

    /**
     * @brief Converts metadata into JSON format, so it can be passed around as string.
     * @return JSON formatted metadata
     */
    std::string toJSON() const;

    /**
     * @brief Load metadata from JSON format.
     * @param json json encoded metadata.
     * @return true if metadata was read correctly from JSON, false otherwise.
     */
    bool fromJSON(const std::string& json);

    /**
     * @enum State of items from metadata during synchronization.
     */
    enum SyncMetadataState {
      NotPresent= 0,  /**< Item is no longer present in local/remote source/storage */
      NotChanged,     /**< Item is present in local/remote source/storage and its content has not been changed */
      Modified        /**< Item is present in local/remote source/storage, but its content has been changed */
    };


    /**
     * @brief Resets state of all remote items
     * @param [in] st state to be set for all remote items.
     */
    void resetRemoteState(SyncMetadataState st);

    /**
     * @brief Resets state of all local items
     * @param [in] st state to be set for all local items.
     */
    void resetLocalState(SyncMetadataState st);

    /**
     * @brief Sets state of remote item.
     * @param uid id of remote item
     * @param state new state of item
     */
    void setRemoteState(const std::string& uid, SyncMetadataState state);

    /**
     * @brief Sets state of remote item.
     * @param uid id of remote item
     * @param state new state of item
     */
    void setLocalState(const std::string& uid, SyncMetadataState state);

    /**
     * @brief Queries all metadata record which items are in given states
     * @param localState state of local item
     * @param remoteState state of remote item
     * @return map of remote items ids with associated to them local items ids.
     */
    std::map<std::string, std::string> getItemsWithState(SyncMetadataState localState, SyncMetadataState remoteState);

    /**
     * @brief Returns sync token of local storage after last synchronization.
     * @return sync token or empty string if not available.
     */
    std::string getLocalSyncToken() const;
 
    /**
     * @brief Sets sync token of local storage to be stored in metadata.
     * @param token sync token to be set
     */
    void setLocalSyncToken(const std::string& token);

     /**
     * @brief Returns sync token of remote storage after last synchronization.
     * @return sync token or empty string if not available.
     */
    std::string getRemoteSyncToken() const;

    /**
     * @brief Sets sync token of remote storage to be stored in metadata.
     * @param token sync token to be set
     */
    void setRemoteSyncToken(const std::string& token);

  private:
    std::string remoteSyncToken;
    std::string localSyncToken;
    std::map<std::string, std::string> remoteRevisions;
    std::map<std::string, std::string> localRevisions;
    std::map<std::string, std::string> remoteToLocalIdMapping;

    std::map<std::string, SyncMetadataState> remoteState;
    std::map<std::string, SyncMetadataState> localState;
};
} // namespace OpenAB_Sync

DECLARE_PLUGIN_INTERFACE(OpenAB_Sync, Sync, Parameters);

#endif // SYNC_HPP_
