/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file DAVHelper.cpp
 */

#include "DAVHelper.hpp"
#include <helpers/Log.hpp>
#include <helpers/StringHelper.hpp>
#include <string.h>

const std::string DAVHelper::PROPERTY_ETAG = "getetag";
const std::string DAVHelper::PROPERTY_CTAG = "getctag";
const std::string DAVHelper::PROPERTY_SYNC_TOKEN = "sync-token";
const std::string DAVHelper::PROPERTY_RESOURCE_TYPE = "resourcetype";
const std::string DAVHelper::PROPERTY_RESOURCE_TYPE_ADDRESSBOOK = "resourcetype:addressbook";
const std::string DAVHelper::PROPERTY_RESOURCE_TYPE_CALENDAR = "resourcetype:calendar";
const std::string DAVHelper::PROPERTY_ADDRESSBOOK_HOME_SET_HREF = "addressbook-home-set:href";
const std::string DAVHelper::PROPERTY_CALENDAR_HOME_SET_HREF = "calendar-home-set:href";
const std::string DAVHelper::PROPERTY_CURRENT_USER_PRINCIPAL_HREF = "current-user-principal:href";
const std::string DAVHelper::PROPERTY_ADDRESS_DATA = "address-data";
const std::string DAVHelper::PROPERTY_CALENDAR_DATA = "calendar-data";
const std::string DAVHelper::PROPERTY_DISPLAY_NAME = "displayname";
const std::string DAVHelper::PROPERTY_SUPPORTED_CALENDAR_COMPONENT_SET_EVENT = "supported-calendar-component-set:comp:VEVENT";
const std::string DAVHelper::PROPERTY_SUPPORTED_CALENDAR_COMPONENT_SET_TODO = "supported-calendar-component-set:comp:VTODO";
const std::string DAVHelper::PROPERTY_SUPPORTED_CALENDAR_COMPONENT_SET_JOURNAL = "supported-calendar-component-set:comp:VJOURNAL";


const std::string DAVHelper::ERROR_UID_CONFLICT = "no-uid-conflict";


DAVHelper::DAVHelper()
{
}

DAVHelper::~DAVHelper()
{
}

void DAVHelper::parseDAVSubProperty(xmlDocPtr doc, xmlNodePtr node,
                                        const std::string& currentName, std::map<std::string, std::string>& result)
{
  node = node->xmlChildrenNode;

  while (NULL != node)
  {
    std::string name = currentName;
    //if node has children nodes - call recursively with updated name
    if (node->xmlChildrenNode)
    {
      //do not append ':' at the begining of property name
      if (!name.empty())
      {
        name += ":";
      }
      name += (char*)node->name;
      parseDAVSubProperty(doc, node, name, result);
    }
    else
    {
      //element has no child nodes if it is text element - it can be value of
      //property or sometimes empty lines (containing \n \t and spaces) that should be ignored
      if (xmlNodeIsText(node))
      {
        xmlChar* text = xmlNodeListGetString(doc, node, 1);
        if (text)
        {
          std::string checkEmpty = (char*)text;
          OpenAB::substituteAll(checkEmpty, " ", "");
          OpenAB::substituteAll(checkEmpty, "\n", "");
          OpenAB::substituteAll(checkEmpty, "\t", "");
          if (!checkEmpty.empty())
          {
            //store string as property value
            result[name] = (char*)text;
          }
          xmlFree(text);
        }
      }
      else if (node->type == XML_CDATA_SECTION_NODE)
      {
        result[name] = (char*)node->content;
      }
      else
      {
        //property has no value
        if (!name.empty())
        {
          name += ":";
        }
        name += (char*) node->name;
        if (strcmp((char*)node->name, "comp") == 0)
        {
          name += ":";
          name += (char*) xmlGetProp(node, (xmlChar*)"name");
        }
        result[name] = "";
      }
    }
    node = node->next;
  }
}

std::map<std::string, std::string> DAVHelper::parseDAVProperty(xmlDocPtr doc, xmlNodePtr node)
{
  std::map<std::string, std::string> result;
  std::string currentName;

  parseDAVSubProperty(doc, node, currentName, result);

  return result;
}

DAVHelper::DAVStatusCode DAVHelper::parseDAVStatus(xmlDocPtr doc, xmlNodePtr node)
{
  node = node->xmlChildrenNode;
  std::string statusString;

  if (NULL != node)
  {
    xmlChar* str = xmlNodeListGetString(doc, node, 1);
    if (str)
    {
      statusString = (char*)str;
      xmlFree(str);
    }
  }

  //Status should be in form HTTP1.1 <code> <code description>,
  //so there should be at least 3 elements separated by spaces (might by more as description can also contains spaces)
  std::vector<std::string> tokens = OpenAB::tokenize(statusString, ' ', false, false);

  if (tokens.size() < 3)
  {
    return 0;
  }
  else
  {
    //HTTP code is always second token
    return atoi(tokens.at(1).c_str());
  }
}

std::string DAVHelper::parseDAVHref(xmlDocPtr doc, xmlNodePtr node)
{
  node = node->xmlChildrenNode;
  std::string href;

  if (NULL != node)
  {
    xmlChar* str = xmlNodeListGetString(doc, node, 1);
    href = (char*) str;
    xmlFree(str);
  }

  //iCloud encodes hrefs using double percent encoding.
  return OpenAB::percentDecode(OpenAB::percentDecode(href));
}

DAVHelper::DAVPropStat DAVHelper::parseDAVPropStat(xmlDocPtr doc, xmlNodePtr node)
{
  DAVPropStat propStat;

  node = node->xmlChildrenNode;

  while (NULL != node)
  {
    if (!xmlStrcmp(node->name, (const xmlChar*) "prop") &&
        (node->ns == davNamespace || node->ns == xmlSearchNsByHref(doc, node, (const xmlChar*)"DAV:")))
    {
      propStat.properties = parseDAVProperty(doc, node);
    }
    else if (!xmlStrcmp(node->name, (const xmlChar*) "status") &&
        (node->ns == davNamespace || node->ns == xmlSearchNsByHref(doc, node, (const xmlChar*)"DAV:")))
    {
      propStat.status = parseDAVStatus(doc, node);
    }
    node = node->next;
  }
  return propStat;
}

DAVHelper::DAVResponse DAVHelper::parseDAVResponse (xmlDocPtr doc, xmlNodePtr node)
{
  DAVResponse response;
  node = node->xmlChildrenNode;

  while (NULL != node)
  {
    if (!xmlStrcmp(node->name, (const xmlChar*)"propstat") &&
        (node->ns == davNamespace || node->ns == xmlSearchNsByHref(doc, node, (const xmlChar*)"DAV:")))
    {
      DAVPropStat propstat = parseDAVPropStat (doc, node);
      response.properties.push_back(propstat);
    }
    else if (!xmlStrcmp(node->name, (const xmlChar*)"href") &&
        (node->ns == davNamespace || node->ns == xmlSearchNsByHref(doc, node, (const xmlChar*)"DAV:")))
    {
      response.href = parseDAVHref (doc, node);
    }
    else if (!xmlStrcmp(node->name, (const xmlChar*)"status") &&
        (node->ns == davNamespace || node->ns == xmlSearchNsByHref(doc, node, (const xmlChar*)"DAV:")))
    {
      response.status = parseDAVStatus (doc, node);
    }
    else if (!xmlStrcmp(node->name, (const xmlChar*) "error"))
    {
      response.error = parseDAVProperty(doc, node);
    }
    node = node->next;
  }
  return response;
}


bool DAVHelper::parseDAVMultistatus (const std::string& xml,
                                     std::vector<DAVResponse>& responses)
{
  std::string syncToken;
  return parseDAVMultistatus(xml, responses, syncToken);
}

bool DAVHelper::parseDAVMultistatus (const std::string& xml,
                                     std::vector<DAVResponse>& responses,
                                     std::string& syncToken)
{
  xmlDocPtr doc;
  xmlNodePtr node;

  doc = xmlParseMemory(xml.c_str(), xml.length());
  if (NULL == doc)
  {
    LOG_ERROR()<<"Cannot parse xml: "<<xml<<std::endl;
    return false;
  }

  node = xmlDocGetRootElement(doc);
  if (NULL == node)
  {
    LOG_ERROR()<<"Xml empty: "<<xml<<std::endl;
    xmlFreeDoc(doc);
    return false;
  }

  davNamespace = xmlSearchNsByHref(doc, node, (const xmlChar*)"DAV:");
  if (NULL == davNamespace)
  {
    LOG_ERROR()<<"Wrong xml type: "<<xml<<std::endl;
    xmlFreeDoc(doc);
    return false;
  }

  if (xmlStrcmp(node->name, (const xmlChar*)"multistatus") ||
      node->ns != davNamespace)
  {
    LOG_ERROR()<<"Not multistatus xml"<<std::endl;
    xmlFreeDoc(doc);
    return false;
  }

  node = node->xmlChildrenNode;

  while (NULL != node)
  {
    if (!xmlStrcmp(node->name, (const xmlChar*)"response") &&
        (node->ns == davNamespace || node->ns == xmlSearchNsByHref(doc, node, (const xmlChar*)"DAV:")))
    {
      DAVResponse response = parseDAVResponse (doc, node);
      responses.push_back (response);
    }
    else if(!xmlStrcmp(node->name, (const xmlChar*)"sync-token") &&
        (node->ns == davNamespace || node->ns == xmlSearchNsByHref(doc, node, (const xmlChar*)"DAV:")))
    {
      xmlChar* text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
      if (text)
      {
        syncToken = (char*)text;
        xmlFree(text);
      }
    }
    node = node->next;
  }
  xmlFreeDoc(doc);
  return true;
}

bool DAVHelper::DAVResponse::hasProperty(const std::string& propName)
{
  std::vector<DAVHelper::DAVPropStat>::iterator it;
  for (it = properties.begin(); it != properties.end(); ++it)
  {
    if ((*it).status == 200)
    {
      std::map<std::string, std::string>::iterator prop;
      prop = (*it).properties.find(propName);
      if ((*it).properties.end() != prop)
      {
        return true;
      }
    }
  }
  return false;
}

std::vector<std::string> DAVHelper::DAVResponse::getPropertiesNames()
{
  std::vector<std::string> names;
  std::vector<DAVHelper::DAVPropStat>::iterator it;
  for (it = properties.begin(); it != properties.end(); ++it)
  {
    if ((*it).status == 200)
    {
      std::map<std::string, std::string>::iterator prop;
      for (prop = (*it).properties.begin(); prop != (*it).properties.end(); ++prop)
      {
        names.push_back((*prop).first);
      }

    }
  }
  return names;
}

std::string DAVHelper::DAVResponse::getProperty(const std::string& propName)
{
  std::vector<DAVHelper::DAVPropStat>::iterator it;
  for (it = properties.begin(); it != properties.end(); ++it)
  {
    if ((*it).status == 200)
    {
      std::map<std::string, std::string>::iterator prop;
      prop = (*it).properties.find(propName);
      if ((*it).properties.end() != prop)
      {
        return (*prop).second;
      }
    }
  }
  return "";
}

bool DAVHelper::DAVResponse::hasError(const std::string& err)
{
  if (error.end() != error.find(err))
  {
    return true;
  }
  return false;
}
