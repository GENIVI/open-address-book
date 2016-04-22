/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file BasicHttpAuthorizer.cpp
 */

#include "BasicHttpAuthorizer.hpp"
#include "PIMItem/Contact/Pict.hpp"
#include <sstream>
#include <string.h>

namespace OpenAB
{
BasicHttpAuthorizer::BasicHttpAuthorizer()
{
}

BasicHttpAuthorizer::~BasicHttpAuthorizer()
{
}

void BasicHttpAuthorizer::setCredentials(const std::string& login,
                                         const OpenAB::SecureString& password)
{
  this->login = login;
  this->pass = password;
}

bool BasicHttpAuthorizer::authorizeMessage (HttpMessage* msg)
{
  if (NULL == msg)
  {
    return false;
  }
  msg->enableBasicHttpAuthentication(true);
  std::string passwd(pass.str());
  msg->setCredentials(login, passwd);
  pass.clearStr();
  return true;
}
} //namespace OpenAB
