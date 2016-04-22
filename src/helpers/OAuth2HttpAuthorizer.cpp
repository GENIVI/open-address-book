/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file OAuth2HttpAuthorizer.cpp
 */

#include "OAuth2HttpAuthorizer.hpp"
#include <json-c/json.h>
#include <helpers/Log.hpp>
#include <string.h>

namespace OpenAB
{
OAuth2HttpAuthorizer::OAuth2HttpAuthorizer()
{
}

OAuth2HttpAuthorizer::~OAuth2HttpAuthorizer()
{
}

bool OAuth2HttpAuthorizer::authorize(const std::string& clientId,
                                 const OpenAB::SecureString& clientSecret,
                                 const OpenAB::SecureString& refreshToken)
{
  HttpSession curl;
  curl.init();

  HttpMessage msg;
  msg.setRequestType(HttpMessage::POST);
  msg.setURL("https://accounts.google.com/o/oauth2/token");

  std::stringstream postParams;
  postParams<<"grant_type=refresh_token&client_id="<<clientId;
  postParams<<"&client_secret="<<clientSecret.str()<<"&refresh_token="<<refreshToken.str();

  msg.setData(postParams.str());

  if (curl.execute(&msg))
  {
    const char* accessToken = NULL;
    const char* tokenType = NULL;

    json_object* jobj = json_tokener_parse (msg.getResponse().c_str());

    if (NULL == jobj)
    {
      return false;
    }

    json_object_object_foreach(jobj, key, val)
    {
      if (strcmp ("access_token", key) == 0)
      {
         accessToken = json_object_get_string(val);
      }
      else if (strcmp ("token_type", key) == 0)
      {
         tokenType = json_object_get_string(val);
      }
    }

    if (NULL == accessToken || NULL == tokenType)
    {
      json_object_put(jobj);
      return false;
    }

    std::string tokenStr = std::string() + std::string(tokenType) + " " + std::string(accessToken);
    json_object_put(jobj);
    token = OpenAB::SecureString (tokenStr);
    return true;
  }
  else
  {
    LOG_ERROR()<<"Authorization error: "<<msg.getErrorString()<<std::endl;
    return false;
  }
}

bool OAuth2HttpAuthorizer::authorizeMessage (HttpMessage* msg)
{
  if (NULL == msg)
  {
    return false;
  }
  msg->appendHeader("Authorization", token.str());
  return true;
}
} //namespace OpenAB
