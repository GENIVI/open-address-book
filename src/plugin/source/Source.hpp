/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file Source.hpp
 */

#ifndef SOURCE_HPP_
#define SOURCE_HPP_

#include <map>
#include <string>
#include <vector>
#include <plugin/Plugin.hpp>
#include <plugin/GenericParameters.hpp>
#include <PIMItem/PIMItem.hpp>

/*!
 * @brief namespace OpenAB_Source
 */
namespace OpenAB_Source {

/**
 * @defgroup SourcePlugin Source Plugin
 * @ingroup pluginGroup
 *
 * Source Interface: @ref Source
 *
 ## Overview ##
 *
 * The Source plugin interface provides all the required functionalities to
 * provide PIM items.
 * Source plugins are expected to provide stream of PIM items that can be used
 * by Sync::Sync plugins (@ref OpenAB_Sync::Sync).
 * Each source can provide only one type of PIM Item (@ref OpenAB::PIMItemType)
 *
 ## API Details ##
 *
 ### Source functionalities ###
 *
 * Providing PIM item stream is main functionality required from Source:
 *  - @ref Source::getItem (@copybrief Source::getItem)
 *
 * Additionaly Source can support set of additional functionalities:
 *  - @ref Source::suspend (@copybrief Source::suspend)
 *  - @ref Source::resume (@copybrief Source::resume)
 *  - @ref Source::cancel (@copybrief Source::cancel)
 *
 *  - @ref Source::getTotalCount (@copybrief Source::getTotalCount)
 *
 ### Quick Browse functions ###
 * These functions are required to quickly retrieve a complete set of PIM
 *  Items from Source.
 * To achieve this, Source needs to be initialized using function:
 *  - @ref Source::init (@copybrief Source::init)
 *
 * initialization will use parameters provided during creation of Source instance
 * (@ref Parameters, @ref OpenAB::PluginManager::getPluginInstance).
 * Each Source plugin can have its own specific parameters.
 * To retrieve all item call @ref Source::getItem function until it will return
 * @ref Source::eGetItemRetEnd.
 *
 */

/**
 * @brief Use generic parameters.
 */
typedef OpenAB_Plugin::GenericParameters Parameters;


/**
 * @brief Documentation for Source plugin interface.
 */
class Source
{
  public:
    /**
     * @brief Constructor
     * @param [in] t type of provided PIM Item (@ref OpenAB::PIMItem)
     */
    Source(OpenAB::PIMItemType t) :
      itemType(t) {};

    /**
     *  @brief Destructor, virtual by default.
     */
    virtual ~Source(){};

    /** @enum
     * init() return code
     */
    enum eInit{
      eInitOk,   /**< Source successfully initialized */
      eInitFail  /**< Failure during the Source initialization */
    };

    /**
     * @brief Initializes Source
     * @return the status code
     */
    virtual enum eInit init() = 0;

    /** @enum
     * suspend() return codes
     */
    enum eSuspendRet{
      eSuspendRetOk,           /**< Source successfully suspended */
      eSuspendRetFail,         /**< Source suspension failed*/
      eSuspendRetNotSupported  /**< Source suspension not supported*/
    };

    /**
     * @brief Suspends Source, if such operation is supported
     * @return the status code
     */
    virtual enum eSuspendRet suspend() = 0;

    /** @enum
     * resume() return codes
     */
    enum eResumeRet{
      eResumeRetOk,           /**< Source successfully resumed*/
      eResumeRetFail,         /**< Source resume failed*/
      eResumeRetNotSupported  /**< Source resume not supported*/
    };

    /**
     * @brief Resumes Source, if such operation is supported
     * @return the status code
     */
    virtual enum eResumeRet resume() = 0;

    /** @enum
     * cancel() return codes
     */
    enum eCancelRet{
      eCancelRetOk,           /**< Source successfully cancelled*/
      eCancelRetFail,         /**< Source cancel failed*/
      eCancelRetNotSupported  /**< Source cancellation not supported*/
    };

    /**
     * @brief Cancels Source, if such operation is supported.
     * After canceling Source next call to getVCard() should
     * return eGetItemRetError status.
     * @return the status code
     */
    virtual enum eCancelRet cancel() = 0;

    /** @enum
     * getItem() return codes
     */
    enum eGetItemRet {
      eGetItemRetOk,   /**< VCard received successfully*/
      eGetItemRetEnd,  /**< No more VCards available*/
      eGetItemRetError /**< Error occurred or input was cancelled*/
    };

    /**
     * @brief Gets PIM Item from Source.
     * If Source is suspended getItem() should block until
     * Source will be resumed or cancelled.
     * @param [out] item item received from Source
     * @return the status code
     * @note item is returned as smart pointer, its contents will be deleted automatically when no more references to it will exist,
     * it's not advised to dereference and use direct pointer
     */
    virtual enum eGetItemRet getItem(OpenAB::SmartPtr<OpenAB::PIMItem> &item) = 0;

    /**
     * @brief Returns total count of items available from Source, if such information is available.
     * @return total count of items available or -1
     * if such information is unavailable
     */
    virtual int getTotalCount() const = 0;

    /**
     * @brief Returns type of PIM Item supported by Source
     * @return type of supported PIM Item (@ref OpenAB::PIMItem)
     */
    OpenAB::PIMItemType getItemType() const
    {
      return itemType;
    }

  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    Source(Source const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    Source& operator=(Source const &other);

    OpenAB::PIMItemType itemType;
};

} // namespace OpenAB_Source

DECLARE_PLUGIN_INTERFACE(OpenAB_Source, Source, Parameters);

#endif // SOURCE_HPP_
