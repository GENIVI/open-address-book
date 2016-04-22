/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file DAVHelper.hpp
 */

#ifndef DAVHELPER_HPP_
#define DAVHELPER_HPP_

#include <string>
#include <vector>
#include <map>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

/*!
 * @brief Helper class handling DAV responses parsing.
 */
class DAVHelper
{
  public:
    /*!
     *  @brief Constructor.
     */
    DAVHelper();

    /*!
     *  @brief Destructor, virtual by default.
     */
    virtual ~DAVHelper();

    /*!
     * @brief DAV response status code - uses the same values as HTTP response codes.
     */
    typedef unsigned int DAVStatusCode;

    /*!
     * @brief DAV PropStat
     */
    struct DAVPropStat
    {
        std::map<std::string, std::string> properties;
        DAVStatusCode status;
    };

    /*!
     * @brief DAV Response
     */
    struct DAVResponse
    {
        std::string href;
        std::vector<DAVPropStat> properties;
        DAVStatusCode status;
        std::map<std::string, std::string> error;
        /*!
         * @brief Checks if response contains given property.
         * In case of properties which contains nested properties,
         * they cane be accessed by name in form property_name:nested_property_name etc.
         * Properties can have no value assigned.
         * @param [in] prop property name
         * @return true if property is present in response and its status code is 200 (HTTP OK), false otherwise.
         */
        bool hasProperty(const std::string& prop);

        /*!
         * @brief Gets value of given property.
         * For properties naming see documentation of hasProperty().
         * @param [in] prop property name
         * @return property value or empty string if given property is not present in response
         * @note please note that based only on result of this function it is not possible to
         * say if property is not present in response or it has no value assigned,
         * this function should be always used after successful hasProperty() check.
         */
        std::string getProperty(const std::string& prop);

        std::vector<std::string> getPropertiesNames();

        bool hasError(const std::string& error);
    };

    /*!
     * @brief Parses multistatus DAV response.
     * @param [in] xml DAV response in XML format.
     * @param [out] responses vector where parsed responses will be stored
     * @return true if response was parsed successfully, false otherwise.
     */
    bool parseDAVMultistatus(const std::string &xml, std::vector<DAVResponse>& responses);

    bool parseDAVMultistatus(const std::string &xml,
                             std::vector<DAVResponse>& responses,
                             std::string& syncToken);

    static const std::string PROPERTY_ETAG;
    static const std::string PROPERTY_CTAG;
    static const std::string PROPERTY_SYNC_TOKEN;
    static const std::string PROPERTY_RESOURCE_TYPE;
    static const std::string PROPERTY_RESOURCE_TYPE_ADDRESSBOOK;
    static const std::string PROPERTY_RESOURCE_TYPE_CALENDAR;
    static const std::string PROPERTY_ADDRESSBOOK_HOME_SET_HREF;
    static const std::string PROPERTY_CALENDAR_HOME_SET_HREF;
    static const std::string PROPERTY_CURRENT_USER_PRINCIPAL_HREF;
    static const std::string PROPERTY_ADDRESS_DATA;
    static const std::string PROPERTY_CALENDAR_DATA;
    static const std::string PROPERTY_DISPLAY_NAME;
    static const std::string PROPERTY_SUPPORTED_CALENDAR_COMPONENT_SET_EVENT;
    static const std::string PROPERTY_SUPPORTED_CALENDAR_COMPONENT_SET_TODO;
    static const std::string PROPERTY_SUPPORTED_CALENDAR_COMPONENT_SET_JOURNAL;
    static const std::string ERROR_UID_CONFLICT;
  private:
    /*!
     *  @brief Copy constructor, private unimplemented to prevent misuse.
     */
    DAVHelper(DAVHelper const &other);

    /*!
     *  @brief Assignment operator, private unimplemented to prevent misuse.
     */
    DAVHelper& operator=(DAVHelper const &other);

    void parseDAVSubProperty(xmlDocPtr doc, xmlNodePtr node,
                             const std::string& prefix, std::map<std::string, std::string>& result);
    std::map<std::string, std::string> parseDAVProperty(xmlDocPtr doc, xmlNodePtr node);
    DAVStatusCode parseDAVStatus(xmlDocPtr doc, xmlNodePtr node);
    std::string parseDAVHref(xmlDocPtr doc, xmlNodePtr node);
    DAVPropStat parseDAVPropStat(xmlDocPtr doc, xmlNodePtr node);
    DAVResponse parseDAVResponse(xmlDocPtr doc, xmlNodePtr node);

    xmlNsPtr davNamespace;
};

#endif // DAVHELPER_HPP_
