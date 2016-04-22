/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file CardDAVHelper.cpp
 */

#include "CardDAVHelper.hpp"
#include <helpers/Log.hpp>
#include <helpers/StringHelper.hpp>


CardDAVHelper::CardDAVHelper(const std::string& serverUrl,
                             OpenAB::HttpSession* httpSession,
                             OpenAB::HttpAuthorizer* httpAuthorizer) :
  serverUrl(serverUrl),
  httpSession(httpSession),
  httpAuthorizer(httpAuthorizer)
{
  serverHostUrl = OpenAB::parseURLHostPart(serverUrl);
}

CardDAVHelper::~CardDAVHelper()
{
}

bool CardDAVHelper::findPrincipalUrl()
{
  OpenAB::HttpMessage msg;
  msg.setRequestType("PROPFIND");
  msg.appendHeader("Content-Type", "text/xml");
  msg.setData("<D:propfind xmlns:D='DAV:'><D:prop><D:current-user-principal/></D:prop></D:propfind>");
  msg.setURL(serverUrl);
  msg.setFollowRedirection(true);
  httpAuthorizer->authorizeMessage(&msg);
  if (httpSession->execute(&msg))
  {
    if (OpenAB::HttpMessage::MULTISTATUS == msg.getResponseCode())
    {
      std::string resp = msg.getResponse();
      LOG_DEBUG()<<resp<<std::endl;
      std::vector<DAVHelper::DAVResponse> responses;
      if (!davHelper.parseDAVMultistatus(resp, responses))
      {
        LOG_ERROR()<<"Cannot parse server response"<<std::endl;
        return false;
      }

      std::vector<DAVHelper::DAVResponse>::iterator it = responses.begin();
      bool principalUrlFound = false;

      for (; it != responses.end(); ++it)
      {
        if ((*it).hasProperty(davHelper.PROPERTY_CURRENT_USER_PRINCIPAL_HREF))
        {
          principalUrlFound = true;
          principalUrl = (*it).getProperty(davHelper.PROPERTY_CURRENT_USER_PRINCIPAL_HREF);
          break;
        }
      }

      if (!principalUrlFound)
      {
        LOG_ERROR()<<"CardDAV request error: " << "misformatted response" <<std::endl;
        return false;
      }
      if (OpenAB::beginsWith(principalUrl, "/"))
      {
        principalUrl = serverHostUrl + principalUrl;
      }

      LOG_DEBUG()<<"Response "<<principalUrl<<std::endl;
      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
    LOG_ERROR()<<"CardDAV request error: " << msg.getErrorString() <<std::endl;
    return false;
  }
}

bool CardDAVHelper::findAddressbookSet()
{
  OpenAB::HttpMessage msg;
  msg.setRequestType("PROPFIND");
  msg.appendHeader("Content-Type", "text/xml");
  msg.setData("<D:propfind xmlns:D='DAV:' xmlns:C=\"urn:ietf:params:xml:ns:carddav\"><D:prop><C:addressbook-home-set/></D:prop></D:propfind>");
  msg.setURL(principalUrl);
  msg.setFollowRedirection(true);

  httpAuthorizer->authorizeMessage(&msg);

  if (httpSession->execute(&msg))
  {
    if (OpenAB::HttpMessage::MULTISTATUS == msg.getResponseCode())
    {
      std::string resp = msg.getResponse();
      LOG_DEBUG()<<resp<<std::endl;
      std::vector<DAVHelper::DAVResponse> responses;
      if (!davHelper.parseDAVMultistatus(resp, responses))
      {
        LOG_ERROR()<<"Cannot parse server response"<<std::endl;
        return false;
      }

      std::vector<DAVHelper::DAVResponse>::iterator it = responses.begin();
      bool addrebookSetFound = false;

      for (; it != responses.end(); ++it)
      {
        if ((*it).hasProperty(davHelper.PROPERTY_ADDRESSBOOK_HOME_SET_HREF))
        {
          addrebookSetFound = true;
          principalAddressbookSetUrl = (*it).getProperty(davHelper.PROPERTY_ADDRESSBOOK_HOME_SET_HREF);
          break;
        }
      }

      if (!addrebookSetFound)
      {
        LOG_ERROR()<<"CardDAV request error: " << "misformatted response" <<std::endl;
        return false;
      }
      if (OpenAB::beginsWith(principalAddressbookSetUrl, "/"))
      {
        principalAddressbookSetUrl = serverHostUrl + principalAddressbookSetUrl;
      }

      principalAddressbookSetHostUrl = OpenAB::parseURLHostPart(principalAddressbookSetUrl);
      LOG_DEBUG()<<"Response "<<principalAddressbookSetUrl<<std::endl;
      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
    LOG_ERROR()<<"CardDAV request error: " << msg.getErrorString() <<std::endl;
    return false;
  }
}

bool CardDAVHelper::findAddressbooks()
{
  OpenAB::HttpMessage msg;
  msg.setRequestType("PROPFIND");
  msg.appendHeader("Content-Type", "text/xml");
  msg.appendHeader("Depth", "1");
  msg.setData("<d:propfind xmlns:d='DAV:'><d:prop><d:resourcetype /><d:displayname /></d:prop></d:propfind>");
  msg.setURL(principalAddressbookSetUrl);
  msg.setFollowRedirection(true);
  httpAuthorizer->authorizeMessage(&msg);

  if (httpSession->execute(&msg))
  {
    if (OpenAB::HttpMessage::MULTISTATUS == msg.getResponseCode())
    {
      std::string resp = msg.getResponse();
      LOG_DEBUG()<<resp<<std::endl;
      std::vector<DAVHelper::DAVResponse> responses;
      if (!davHelper.parseDAVMultistatus(resp, responses))
      {
        LOG_ERROR()<<"Cannot parse server response"<<std::endl;
        return false;
      }
      std::vector<DAVHelper::DAVResponse>::iterator it = responses.begin();
      bool addrebookFound = false;

      for (; it != responses.end(); ++it)
      {
        if ((*it).hasProperty(davHelper.PROPERTY_RESOURCE_TYPE_ADDRESSBOOK))
        {
          addrebookFound = true;
          principalAddressbookUrl = (*it).href;
          break;
        }
      }

      if (!addrebookFound)
      {
        LOG_ERROR()<<"CardDAV request error: " << "misformatted response" <<std::endl;
        return false;
      }
      if (OpenAB::beginsWith(principalAddressbookUrl, "/"))
      {
        principalAddressbookUrl = principalAddressbookSetHostUrl + principalAddressbookUrl;
      }
      LOG_DEBUG()<<"Response "<<principalAddressbookUrl<<std::endl;
      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
    LOG_ERROR()<<"CardDAV request error: " << msg.getErrorString() <<std::endl;
    return false;
  }
}


bool CardDAVHelper::queryAddressbookMetadata()
{
  OpenAB::HttpMessage msg;
  msg.setRequestType("PROPFIND");
  msg.appendHeader("Content-Type", "text/xml");
  msg.appendHeader("Depth", "0");
  msg.setData("<D:propfind xmlns:D='DAV:'> <D:prop><D:displayname /><D:getctag/><D:sync-token/></D:prop></D:propfind>");
  msg.setURL(principalAddressbookUrl);

  httpAuthorizer->authorizeMessage(&msg);

  if (httpSession->execute(&msg))
  {
    if (OpenAB::HttpMessage::MULTISTATUS == msg.getResponseCode())
    {
      std::string resp = msg.getResponse();
      std::vector<DAVHelper::DAVResponse> responses;
      if (!davHelper.parseDAVMultistatus(resp, responses))
      {
        LOG_ERROR()<<"Cannot parse server response"<<std::endl;
        return false;
      }
      std::vector<DAVHelper::DAVResponse>::iterator it = responses.begin();

      for (; it != responses.end(); ++it)
      {
        if ((*it).hasProperty(davHelper.PROPERTY_CTAG))
        {
          addressbookCTag = (*it).getProperty(davHelper.PROPERTY_CTAG);
        }
        if ((*it).hasProperty(davHelper.PROPERTY_SYNC_TOKEN))
        {
          addressbookSyncToken = (*it).getProperty(davHelper.PROPERTY_SYNC_TOKEN);
        }
      }
      LOG_DEBUG()<<"CTAG: "<<addressbookCTag<<" SyncToken "<<addressbookSyncToken<<std::endl;
      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
   LOG_ERROR()<<"CardDAV request error: " << msg.getErrorString() <<std::endl;
   return false;
  }
}

bool CardDAVHelper::queryContactsMetadata()
{
  contactsMetadata.clear();
  OpenAB::HttpMessage msg;
  msg.setRequestType("PROPFIND");
  msg.appendHeader("Content-Type", "text/xml");
  msg.appendHeader("Depth", "1");
  msg.setData("<D:propfind xmlns:D='DAV:'> <D:prop><D:getetag/><D:resourcetype/></D:prop></D:propfind>");
  msg.setURL(principalAddressbookUrl);

  httpAuthorizer->authorizeMessage(&msg);

  if (httpSession->execute(&msg))
  {
    if (OpenAB::HttpMessage::MULTISTATUS == msg.getResponseCode())
    {
      std::string resp = msg.getResponse();
      std::vector<DAVHelper::DAVResponse> responses;
      if (!davHelper.parseDAVMultistatus(resp, responses))
      {
        LOG_ERROR()<<"Cannot parse server response"<<std::endl;
        return false;
      }
      std::vector<DAVHelper::DAVResponse>::iterator it = responses.begin();

      for (; it != responses.end(); ++it)
      {
        if ((*it).hasProperty(davHelper.PROPERTY_RESOURCE_TYPE) &&
            (*it).getProperty(davHelper.PROPERTY_RESOURCE_TYPE).empty())
        {
          ContactMetadata metadata;
          metadata.uri = (*it).href;
          metadata.etag = (*it).getProperty(davHelper.PROPERTY_ETAG);
          contactsMetadata.push_back(metadata);
        }
      }

      LOG_DEBUG() << "Got  " << contactsMetadata.size() << " contacts"<<std::endl;
      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
   LOG_ERROR()<<"CardDAV request error: " << msg.getErrorString() <<std::endl;
   return false;
  }
}

bool CardDAVHelper::queryChangedContactsMetadata(const std::string& syncToken,
                                                 std::vector<OpenAB::PIMItem::ID>& removed)
{
  if (serverHostUrl == "https://www.googleapis.com")
  {
    return false;
  }
  contactsMetadata.clear();
  OpenAB::HttpMessage msg;
  msg.setRequestType("REPORT");
  msg.appendHeader("Content-Type", "text/xml");
  msg.appendHeader("Depth", "0");
  msg.setURL(principalAddressbookUrl);

  httpAuthorizer->authorizeMessage(&msg);

  std::stringstream oss;
  oss<<"<D:sync-collection xmlns:D='DAV:'><D:sync-token>";
  oss<<syncToken;
  oss<<"</D:sync-token><D:sync-level>1</D:sync-level>";
  oss<<"<D:prop><D:getetag/></D:prop></D:sync-collection>";

  msg.setData(oss.str());

  if (httpSession->execute(&msg))
  {
    if (OpenAB::HttpMessage::MULTISTATUS == msg.getResponseCode())
    {
      std::string resp = msg.getResponse();
      LOG_DEBUG()<<resp<<std::endl;
      std::vector<DAVHelper::DAVResponse> responses;
      if (!davHelper.parseDAVMultistatus(resp, responses, addressbookSyncToken))
      {
        LOG_ERROR()<<"Cannot parse server response"<<std::endl;
        return false;
      }
      std::vector<DAVHelper::DAVResponse>::iterator it = responses.begin();

      for (; it != responses.end(); ++it)
      {
        if ((*it).hasProperty(davHelper.PROPERTY_ETAG))
        {
          ContactMetadata metadata;
          metadata.uri = (*it).href;
          metadata.etag = (*it).getProperty(davHelper.PROPERTY_ETAG);
          contactsMetadata.push_back(metadata);
        }
        else
        {
          removed.push_back((*it).href);
        }
      }

      LOG_DEBUG() << "Got  " << contactsMetadata.size() << " contacts SyncToken "<<addressbookSyncToken<<std::endl;
      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
   LOG_ERROR()<<"CardDAV request error: " << msg.getErrorString() <<std::endl;
   return false;
  }
}

bool CardDAVHelper::downloadVCards(std::vector<std::string>& uris,
                                   std::vector<std::string>& vcards)
{
  OpenAB::HttpMessage msg;
  msg.setRequestType("REPORT");
  msg.setURL(principalAddressbookUrl);
  msg.appendHeader("Content-Type", "text/xml");
  msg.appendHeader("Depth", "1");

  httpAuthorizer->authorizeMessage(&msg);


  std::stringstream oss;
  oss<<"<C:addressbook-multiget xmlns:D='DAV:' xmlns:C='urn:ietf:params:xml:ns:carddav'>";
  oss<<"<D:prop><D:getetag/><C:address-data>";
  oss<<"</C:address-data></D:prop>";

  for (unsigned int i = 0; i < uris.size(); ++i)
  {
    oss<<"<D:href>"<<uris[i]<<"</D:href>";
  }

  oss<<"</C:addressbook-multiget>";

  msg.setData(oss.str());

  if (httpSession->execute(&msg))
  {
    if (msg.MULTISTATUS == msg.getResponseCode())
    {
      std::string resp = msg.getResponse();
      std::vector<DAVHelper::DAVResponse> responses;
      if (!davHelper.parseDAVMultistatus(resp, responses))
      {
        LOG_ERROR()<<"Cannot parse server response"<<std::endl;
        return false;
      }
      std::vector<DAVHelper::DAVResponse>::iterator it = responses.begin();

      for (; it != responses.end(); ++it)
      {
        if ((*it).hasProperty(davHelper.PROPERTY_ADDRESS_DATA))
        {
          std::string vCard = (*it).getProperty(davHelper.PROPERTY_ADDRESS_DATA);
          if (!vCard.empty())
          {
            //Google unnecessary escapes ':' character
            OpenAB::substituteAll(vCard, "\\:", ":");
            //Convert any encoded XML characters (can occur in NOTE field)
            OpenAB::substituteAll(vCard, "&lt;", "<");
            OpenAB::substituteAll(vCard, "&gt;", ">");
            std::istringstream input(vCard);
            std::ostringstream output;

            //Google and iCloud are grouping some fields with custom labels
            //creating new fields that are beginning with "item#.FIELD_NAME" and "item#.LABEL"
            //For now custom labels are ignored and item#.FIELD_NAME fields are converted to FIELD_NAME fields.
            std::string line;
            while (std::getline(input, line))
            {
              if(OpenAB::beginsWith(line, "item"))
              {

                std::string::size_type pos = 0;
                OpenAB::cut(line, "item", ".", pos);
                if (pos != std::string::npos)
                {
                  line = line.substr(pos+1);
                }
              }
              output<<line<<std::endl;
            }

            vcards.push_back(output.str());
          }
        }
      }
      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
    LOG_ERROR()<<"CardDAV request error: " << msg.getErrorString() <<std::endl;
    return false;
  }
}
bool CardDAVHelper::downloadVCards(unsigned int offset, unsigned int size,
                                   std::vector<std::string>& vcards)
{
  OpenAB::HttpMessage msg;
  msg.setRequestType("REPORT");
  msg.setURL(principalAddressbookUrl);
  msg.appendHeader("Content-Type", "text/xml");
  msg.appendHeader("Depth", "1");

  httpAuthorizer->authorizeMessage(&msg);


  std::stringstream oss;
  oss<<"<C:addressbook-multiget xmlns:D='DAV:' xmlns:C='urn:ietf:params:xml:ns:carddav'>";
  oss<<"<D:prop><D:getetag/><C:address-data>";
  oss<<"</C:address-data></D:prop>";

  unsigned int count = (contactsMetadata.size() > (offset + size)) ? (offset + size) : contactsMetadata.size();
  for (unsigned int i = offset; i < count; ++i)
  {
    oss<<"<D:href>"<<contactsMetadata[i].uri<<"</D:href>";
  }

  oss<<"</C:addressbook-multiget>";

  msg.setData(oss.str());

  if (httpSession->execute(&msg))
  {
    if (msg.MULTISTATUS == msg.getResponseCode())
    {
      std::string resp = msg.getResponse();
      std::vector<DAVHelper::DAVResponse> responses;
      if (!davHelper.parseDAVMultistatus(resp, responses))
      {
        LOG_ERROR()<<"Cannot parse server response"<<std::endl;
        return false;
      }
      std::vector<DAVHelper::DAVResponse>::iterator it = responses.begin();

      for (; it != responses.end(); ++it)
      {
        if ((*it).hasProperty(davHelper.PROPERTY_ADDRESS_DATA))
        {
          std::string vCard = (*it).getProperty(davHelper.PROPERTY_ADDRESS_DATA);
          if (!vCard.empty())
          {
            //Google unnecessary escapes ':' character
            OpenAB::substituteAll(vCard, "\\:", ":");
            //Convert any encoded XML characters (can occur in NOTE field)
            OpenAB::substituteAll(vCard, "&lt;", "<");
            OpenAB::substituteAll(vCard, "&gt;", ">");
            std::istringstream input(vCard);
            std::ostringstream output;

            //Google and iCloud are grouping some fields with custom labels
            //creating new fields that are beginning with "item#.FIELD_NAME" and "item#.LABEL"
            //For now custom labels are ignored and item#.FIELD_NAME fields are converted to FIELD_NAME fields.
            std::string line;
            while (std::getline(input, line))
            {
              if(OpenAB::beginsWith(line, "item"))
              {

                std::string::size_type pos = 0;
                OpenAB::cut(line, "item", ".", pos);
                if (pos != std::string::npos)
                {
                  line = line.substr(pos+1);
                }
              }
              output<<line<<std::endl;
            }

            vcards.push_back(output.str());
          }
        }
      }
      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
    LOG_ERROR()<<"CardDAV request error: " << msg.getErrorString() <<std::endl;
    return false;
  }
}

bool CardDAVHelper::addContact(const std::string& vcard,
                               std::string& uri,
                               std::string& etag)
{
  OpenAB::HttpMessage msg;
  msg.setRequestType(msg.POST);
  msg.setData(vcard);
  msg.setURL(principalAddressbookUrl);
  msg.appendHeader("Content-Type", "text/vcard; charset=utf-8");

  httpAuthorizer->authorizeMessage(&msg);

  if (httpSession->execute(&msg))
  {
    if (msg.CREATED == msg.getResponseCode())
    {
      OpenAB::HttpMessage::Headers headers = msg.getResponseHeaders();
      for (unsigned int i = 0; i < headers.size(); ++i)
      {
        if (headers[i].first == "Location")
        {
          uri = headers[i].second;
        }
        else if (headers[i].first == "ETag")
        {
          etag = headers[i].second;
        }
        LOG_DEBUG()<<headers[i].first<<" "<<headers[i].second<<std::endl;
      }
      LOG_ERROR()<<"Contact created with uid: " << uri<<" etag: "<<etag<<std::endl;
      return true;
    }
    else if (msg.MULTISTATUS == msg.getResponseCode())
    {
      std::string resp = msg.getResponse();
      LOG_DEBUG()<<resp<<std::endl;
      std::vector<DAVHelper::DAVResponse> responses;
      if (!davHelper.parseDAVMultistatus(resp, responses))
      {
        LOG_ERROR()<<"Cannot parse server response"<<std::endl;
        return false;
      }
      std::vector<DAVHelper::DAVResponse>::iterator it = responses.begin();

      for (; it != responses.end(); ++it)
      {
        LOG_DEBUG()<<"Response status "<<(*it).status<<std::endl;
        if ((*it).hasProperty(davHelper.PROPERTY_ETAG))
        {
          uri = (*it).href;
          etag = (*it).getProperty(davHelper.PROPERTY_ETAG);
          LOG_ERROR()<<"Contact created with uid: " << uri<<" etag: "<<etag<<std::endl;
          return true;
        }
        else if ((*it).hasError(davHelper.ERROR_UID_CONFLICT))
        {
          LOG_ERROR()<<"Contact with the same UID already exists on server"<<std::endl;
          return false;
        }
        else
        {
          LOG_ERROR()<<"CardDAV request error: " <<std::endl;
          return false;
        }
      }
      return true;
    }
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
    LOG_ERROR()<<"CardDAV request error: " << msg.getResponseCode()<<" "<<msg.getErrorString() <<std::endl;
    return false;
  }
  return true;
}

bool CardDAVHelper::removeContact(const std::string& uri,
                                  const std::string& etag)
{
  OpenAB::HttpMessage msg;
  msg.setRequestType("DELETE");
  msg.setURL(principalAddressbookSetHostUrl + uri);
  if (!etag.empty())
  {
    msg.appendHeader("If-Match", etag);
  }

  LOG_DEBUG()<<"Removing "<<principalAddressbookSetHostUrl + uri<<std::endl;

  httpAuthorizer->authorizeMessage(&msg);

  if (httpSession->execute(&msg))
  {
    if (msg.NO_CONTENT == msg.getResponseCode())
    {
      OpenAB::HttpMessage::Headers headers = msg.getResponseHeaders();
      for (unsigned int i = 0; i < headers.size(); ++i)
      {
        LOG_DEBUG()<<headers[i].first<<" "<<headers[i].second<<std::endl;
      }
      LOG_ERROR()<<"Contact removed: " << msg.getResponseCode()<<" " <<msg.getResponse() <<std::endl;
      return true;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
    LOG_ERROR()<<"CardDAV request error: " << msg.getResponseCode()<<" "<<msg.getErrorString() <<std::endl;
    return false;
  }
}

bool CardDAVHelper::modifyContact(const std::string& uri,
                                  const std::string& vcard,
                                  std::string& etag)
{
  OpenAB::HttpMessage msg;
  msg.setRequestType(OpenAB::HttpMessage::PUT);
  msg.setData(vcard);
  msg.setURL(principalAddressbookSetHostUrl + uri);
  msg.appendHeader("Content-Type", "text/vcard; charset=utf-8");

  if (!etag.empty())
  {
    msg.appendHeader("If-Match", etag);
  }

  LOG_DEBUG()<<"Updating "<<principalAddressbookSetHostUrl + uri<<std::endl;

  httpAuthorizer->authorizeMessage(&msg);

  if (httpSession->execute(&msg))
  {
    OpenAB::HttpMessage::Headers headers = msg.getResponseHeaders();
    for (unsigned int i = 0; i < headers.size(); ++i)
    {
      LOG_DEBUG()<<headers[i].first<<" "<<headers[i].second<<std::endl;
    }
    LOG_ERROR()<<"Contact updated: " << msg.getResponseCode()<<" " <<msg.getResponse() <<std::endl;

    if (msg.NO_CONTENT == msg.getResponseCode())
    {
      OpenAB::HttpMessage::Headers headers = msg.getResponseHeaders();
      for (unsigned int i = 0; i < headers.size(); ++i)
      {
        if (headers[i].first == "ETag")
        {
          etag = headers[i].second;
        }
        LOG_DEBUG()<<headers[i].first<<" "<<headers[i].second<<std::endl;
      }
      LOG_ERROR()<<"Contact updated with uid: " << uri<<" etag: "<<etag<<std::endl;
      return true;
    }
    else if (msg.PRECONDITION_FAILED == msg.getResponseCode())
    {
      LOG_ERROR()<<"Cannot update contact - provided ETag does not match one on server - probably contact was modified before"<<std::endl;
      return false;
    }
    LOG_ERROR()<<"Server returned "<<msg.getResponseCode()<<" code - ";
    LOG_ERROR()<<OpenAB::HttpMessage::responseCodeDescription(msg.getResponseCode())<<std::endl;
    return false;
  }
  else
  {
    LOG_ERROR()<<"CardDAV request error: " << msg.getResponseCode()<<" "<<msg.getErrorString() <<std::endl;
    return false;
  }
}
