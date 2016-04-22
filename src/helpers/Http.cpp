/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file Http.cpp
 */

#include "helpers/Http.hpp"
#include <string.h>
#include <sstream>
#include "helpers/StringHelper.hpp"
#include "helpers/Log.hpp"

namespace OpenAB
{


HttpMessage::HttpMessage() :
  requestType (HttpMessage::POST),
  redirectionEnabled (false),
  responseCode (0),
  basicAuthenticationEnabled(false),
  digestAuthenticationEnabled(false)
{

}

HttpMessage::~HttpMessage()
{

}

void HttpMessage::setRequestType(HttpMessage::RequestType type)
{
  if (CUSTOM != type)
  {
    requestType = type;
  }
}

void HttpMessage::setRequestType(const std::string& custom)
{
  requestType = CUSTOM;
  customRequestType = custom;
}

HttpMessage::RequestType HttpMessage::getRequestType() const
{
  return requestType;
}

std::string HttpMessage::getCustomRequestType() const
{
  return customRequestType;
}

void HttpMessage::appendHeader(const std::string& key,
                               const std::string& value)
{
  headers.push_back(std::make_pair(key, value));
}

const HttpMessage::Headers& HttpMessage::getHeaders() const
{
  return headers;
}

void HttpMessage::setData(const std::string& d)
{
  data = d;
}

std::string HttpMessage::getData() const
{
  return data;
}

void HttpMessage::setFollowRedirection(bool follow)
{
  redirectionEnabled = follow;
}

bool HttpMessage::followRedirection() const
{
  return redirectionEnabled;
}

void HttpMessage::setURL(const std::string& u)
{
  url = u;
}

std::string HttpMessage::getURL() const
{
  return url;
}

void HttpMessage::enableBasicHttpAuthentication(bool enable)
{
  basicAuthenticationEnabled = enable;
  if (enable)
  {
    digestAuthenticationEnabled = false;
  }
}

bool HttpMessage::basicHttpAuthenticationEnabled()
{
  return basicAuthenticationEnabled;
}

void HttpMessage::enableDigestHttpAuthentication(bool enable)
{
  digestAuthenticationEnabled = enable;
  if (enable)
  {
    basicAuthenticationEnabled = false;
  }
}

bool HttpMessage::digestHttpAuthenticationEnabled()
{
  return digestAuthenticationEnabled;
}

void HttpMessage::setCredentials(const std::string& log, const std::string& pass)
{
  login = log;
  password = pass;
}

void HttpMessage::getCredentials(std::string& log, std::string& pass)
{
  log = login;
  pass = password;
}

void HttpMessage::setResponse(const std::string& resp)
{
  response = resp;
}

std::string HttpMessage::getResponse() const
{
  return response;
}

void HttpMessage::setResponseHeaders(const std::string& headers)
{
  responseHeaders.clear();
  std::vector<std::string> h = OpenAB::tokenize(headers, '\n', false, false);
  for (unsigned int i = 0; i < h.size(); ++i)
  {
    std::string::size_type idx = h[i].find_first_of(':');
    if (std::string::npos != idx)
    {
      std::string key = h[i].substr(0, idx);
      std::string value = h[i].substr(idx+1);
      trimWhitespaces(key);
      trimWhitespaces(value);
      responseHeaders.push_back(std::pair<std::string, std::string>(key, value));
    }
  }
}

HttpMessage::Headers HttpMessage::getResponseHeaders() const
{
  return responseHeaders;
}

void HttpMessage::setResponseCode(long c)
{
  responseCode = c;
}

long HttpMessage::getResponseCode() const
{
  return responseCode;
}

void HttpMessage::setErrorString(const std::string& error)
{
  errorString = error;
}

std::string HttpMessage::getErrorString() const
{
  return errorString;
}

std::string HttpMessage::responseCodeDescription(long code)
{
  switch(code)
  {
    case OK:
      return "The request has succeeded";
    case CREATED:
      return "The request has succeeded and a new resource was created";
    case ACCEPTED:
      return "The request was accepted for processing";
    case NO_CONTENT:
      return "The request was fulfilled, but no content was returned";
    case MULTISTATUS:
      return "The request has succeeded and WebDAV multistatus XML was returned";
    case MOVED_PERMAMENTLY:
      return "The requested resource has been moved to new URI";
    case BAD_REQUEST:
      return "The server does not understood request";
    case UNAUTHORIZED:
      return "The request requires user to be authenticated";
    case FORBIDDEN:
      return "The server rejects request";
    case NOT_FOUND:
      return "The requested URI wasn't found";
    case PRECONDITION_FAILED:
      return "The precondition given in request was not met";
    default:
      return "Unknown code";
  }
}

HttpSession::HttpSession() :
    curl (NULL),
    internalHeader (NULL),
    writeBuffer(NULL),
    currentWriteOffset(0),
    traceEnabled(false)
{
}

HttpSession::~HttpSession()
{
  cleanup();
}

bool HttpSession::init()
{
  curl_global_init(CURL_GLOBAL_ALL);

  if (NULL != curl)
  {
    curl_easy_cleanup (curl);
    curl = NULL;
  }

  curl = curl_easy_init();

  curl_easy_setopt (curl, CURLOPT_VERBOSE, 1);
  curl_easy_setopt (curl, CURLOPT_STDERR, stdout);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  return (curl != NULL);
}

void HttpSession::cleanup()
{
  if (NULL != curl)
  {
    curl_easy_cleanup (curl);
    curl = NULL;
  }
  if (NULL != internalHeader)
  {
    curl_slist_free_all (internalHeader);
    internalHeader = NULL;
  }
}

void HttpSession::reset() {
  if (NULL != curl)
  {
    curl_easy_reset (curl);
  }
  if (NULL != internalHeader)
  {
    curl_slist_free_all (internalHeader);
    internalHeader = NULL;
  }
}

void HttpSession::enableTrace(bool enable)
{
  traceEnabled = enable;
}

bool HttpSession::execute(HttpMessage* msg)
{
  reset();
 // curl_easy_setopt (curl, CURLOPT_VERBOSE, 1);
 // curl_easy_setopt (curl, CURLOPT_STDERR, stdout);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  if (traceEnabled)
  {
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, printTrace);
    curl_easy_setopt(curl, CURLOPT_DEBUGDATA, NULL);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  }
  else
  {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
  }

  long responseCode;
  //set request type
  switch (msg->getRequestType())
  {
    case HttpMessage::CUSTOM:
    {
      curl_easy_setopt (curl, CURLOPT_CUSTOMREQUEST, msg->getCustomRequestType().c_str());
      break;
    }
    case HttpMessage::POST:
    {
      curl_easy_setopt (curl, CURLOPT_POST, 1);
      break;
    }
    case HttpMessage::GET:
    {
      curl_easy_setopt (curl, CURLOPT_HTTPGET, 1);
      break;
    }
    case HttpMessage::PUT:
    {
      curl_easy_setopt (curl, CURLOPT_PUT, 1);
      break;
    }
  }
  //set data
  if (HttpMessage::POST == msg->getRequestType())
  {
    curl_easy_setopt (curl, CURLOPT_COPYPOSTFIELDS, msg->getData().c_str());
  }
  else if ((HttpMessage::PUT == msg->getRequestType() ||
           HttpMessage::CUSTOM == msg->getRequestType()) &&
           !msg->getData().empty())
  {
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, writeData);
    curl_easy_setopt(curl, CURLOPT_READDATA, this);

    curl_easy_setopt(curl, CURLOPT_SEEKFUNCTION, writeSeek);
    curl_easy_setopt(curl, CURLOPT_SEEKDATA, this);

    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);

    currentWriteOffset = 0;
    writeBuffer = msg->getData().c_str();
  }
  //set url
  curl_easy_setopt(curl, CURLOPT_URL, msg->getURL().c_str());

  /*if (msg->followRedirection())
  {
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  }*/
  //set headers
  if (!msg->getHeaders().empty())
  {
    if (NULL != internalHeader)
    {
      curl_slist_free_all (internalHeader);
      internalHeader = NULL;
    }
    HttpMessage::Headers::const_iterator it;
    for (it = msg->getHeaders().begin(); it != msg->getHeaders().end(); ++it)
    {
      std::stringstream temp;
      temp << (*it).first<<": "<<(*it).second;
      internalHeader = curl_slist_append(internalHeader, temp.str().c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, internalHeader);
  }

  //set authentication
  if (msg->basicHttpAuthenticationEnabled() || msg->digestHttpAuthenticationEnabled())
  {
    if (msg->basicHttpAuthenticationEnabled())
    {
      curl_easy_setopt(curl, CURLOPT_HTTPAUTH,  CURLAUTH_BASIC);
    }
    else
    {
      curl_easy_setopt(curl, CURLOPT_HTTPAUTH,  CURLAUTH_DIGEST);
    }
    std::string login;
    std::string password;
    msg->getCredentials(login, password);
    curl_easy_setopt(curl, CURLOPT_USERNAME, login.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
  }
  //set response callback
  curl_easy_setopt (curl, CURLOPT_WRITEDATA, this);
  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, &readResponse);
  curl_easy_setopt (curl, CURLOPT_HEADERDATA, this);
  curl_easy_setopt (curl, CURLOPT_HEADERFUNCTION, &readResponseHeaders);
  responseBuffer.clear();
  responseHeaders.clear();
  CURLcode res = curl_easy_perform (curl);
  if (res != CURLE_OK)
  {
    msg->setErrorString(curl_easy_strerror(res));
    return false;
  }
  curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &responseCode);
  msg->setResponseCode(responseCode);
  msg->setResponse(responseBuffer);
  msg->setResponseHeaders(responseHeaders);

  if (301 == responseCode && msg->followRedirection())
  {
    char* redirectionUrl = NULL;
    curl_easy_getinfo (curl, CURLINFO_REDIRECT_URL, &redirectionUrl);
    msg->setURL(redirectionUrl);
    return execute(msg);
  }
  responseBuffer.clear();
  responseHeaders.clear();

  return true;
}

size_t HttpSession::readResponse (void* ptr, size_t size, size_t nmemb, HttpSession* curl)
{
  if (NULL == curl)
  {
    return 0;
  }
  char buffer [4096];
  (void) ptr;
  size_t newDataSize = size * nmemb;
  size_t dataLeft = newDataSize;
  size_t dataRead = 0;

  while (dataLeft)
  {
    size_t toRead = (dataLeft > 4096) ? 4096 : dataLeft;
    memcpy(buffer, (char*)ptr + dataRead, toRead);
    dataRead += toRead;
    dataLeft -= toRead;
    curl->responseBuffer.append(buffer, toRead);
  }

  return newDataSize;
}


size_t HttpSession::readResponseHeaders (void* ptr, size_t size, size_t nmemb, HttpSession* curl)
{
  if (NULL == curl)
  {
    return 0;
  }
  char buffer [4096];
  (void) ptr;
  size_t newDataSize = size * nmemb;
  size_t dataLeft = newDataSize;
  size_t dataRead = 0;

  while (dataLeft)
  {
    size_t toRead = (dataLeft > 4096) ? 4096 : dataLeft;
    memcpy(buffer, (char*)ptr + dataRead, toRead);
    dataRead += toRead;
    dataLeft -= toRead;
    curl->responseHeaders.append(buffer, toRead);
  }

  return newDataSize;
}

size_t HttpSession::writeData(void *ptr, size_t size, size_t nmemb, HttpSession* curl)
{
  if (NULL == curl)
  {
    return 0;
  }

  size_t dataLeft = strlen(curl->writeBuffer) - curl->currentWriteOffset;
  size_t readRequestSize = size * nmemb;

  size_t toRead = (dataLeft > readRequestSize) ? readRequestSize : dataLeft;
  if (toRead > 0)
  {
    memcpy((char*)ptr, curl->writeBuffer + curl->currentWriteOffset, toRead);
    curl->currentWriteOffset += toRead;
  }

  return toRead;
}

int HttpSession::writeSeek(HttpSession *curl, curl_off_t offset, int origin)
{
  (void) origin;
  if (NULL == curl)
  {
    return 0;
  }

  curl->currentWriteOffset = offset;
  return curl->currentWriteOffset;
}

int HttpSession::printTrace(CURL* curl, curl_infotype type, char* data, size_t size, void *userp)
{
  (void)curl;
  (void)size;
  (void)userp;

  LOG_DEBUG()<<"======================== HTTP TRACE ================="<<std::endl;
  switch (type)
  {
    case CURLINFO_HEADER_OUT:
      LOG_DEBUG()<<"Send header"<<std::endl;
      break;
    case CURLINFO_DATA_OUT:
        LOG_DEBUG()<<"Send data"<<std::endl;
        break;
    case CURLINFO_HEADER_IN:
        LOG_DEBUG()<<"Received header"<<std::endl;
        break;
    case CURLINFO_DATA_IN:
        LOG_DEBUG()<<"Received data"<<std::endl;
        break;
    default:
      break;
  }

  LOG_DEBUG()<<data<<std::endl;

  LOG_DEBUG()<<"===================================================="<<std::endl;
  return 0;
}

} //namespace OpenAB
