/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file carddav_helper_tester.cpp
 */
#include <string.h>
#include <OpenAB.hpp>
#include "helpers/SecureString.hpp"
#include "CardDAVHelper.hpp"
#include "helpers/BasicHttpAuthorizer.hpp"
#include "helpers/OAuth2HttpAuthorizer.hpp"

const char* testVCard = "BEGIN:VCARD\n"
    "VERSION:3.0\n"
    "N:Test contact\n"
    "FN:Test contact\n"
    "TEL:1234567\n"
    "UID:123123123\n"
    "END:VCARD";

const char* testVCard2 = "BEGIN:VCARD\n"
    "VERSION:3.0\n"
    "N:Test contact2\n"
    "FN:Test contact2\n"
    "TEL:1234567\n"
    "UID:123123123\n"
    "END:VCARD";

int main(int argc, char* argv[])
{
  OpenAB::Logger::OutLevel() = OpenAB::Logger::Debug;

  if (argc < 2)
  {
    printf ("Use %s <authentication method>\n", argv[0]);
    return 1;
  }

  OpenAB::HttpSession httpSession;
  httpSession.init();

  OpenAB::HttpAuthorizer* httpAuthorizer = NULL;

  OpenAB_Source::Parameters params;
  if (0 == strcmp (argv[1], "password"))
  {
    if (argc < 5)
    {
      printf("Use %s password <server url> <user login> <user password>\n",argv[0]);
      return 1;
    }
    httpAuthorizer = new OpenAB::BasicHttpAuthorizer();
    ((OpenAB::BasicHttpAuthorizer*)httpAuthorizer)->setCredentials(argv[3], argv[4]);
  }
  else
  {
    if (argc < 6)
    {
      printf("Use %s oauth2 <server url> <client id> <client secret> <refresh token>\n",argv[0]);
      return 1;
    }
    httpAuthorizer = new OpenAB::OAuth2HttpAuthorizer();
    ((OpenAB::OAuth2HttpAuthorizer*)httpAuthorizer)->authorize(argv[3], argv[4], argv[5]);
  }

  CardDAVHelper helper(argv[2], &httpSession, httpAuthorizer);

  if (!helper.findPrincipalUrl())
  {
    LOG_ERROR() << "Cannot connect to CardDAV server"<<std::endl;
    return 1;
  }

  if (!helper.findAddressbookSet())
  {
    LOG_ERROR() << "Cannot connect to CardDAV server"<<std::endl;
    return 1;
  }

  if (!helper.findAddressbooks())
  {
    LOG_ERROR() << "Cannot connect to CardDAV server"<<std::endl;
    return 1;
  }

  helper.queryAddressbookMetadata();

  LOG_DEBUG()<<"============"<<std::endl;
  std::vector<OpenAB::PIMItem::ID> removed;
  helper.queryChangedContactsMetadata(argv[5], removed);
/*
  std::string uri;
  std::string etag;
  if (helper.addContact(testVCard, uri, etag))
  {
    LOG_DEBUG()<<"Contact added sucesfully with uri "<<uri<<std::endl;
  }
  else
  {
    LOG_DEBUG()<<"Cannot create contact"<<std::endl;
  }

  if (helper.modifyContact(uri, testVCard2, etag))
  {
    LOG_DEBUG()<<"Contact update sucesfully with etag "<<etag<<std::endl;
  }
  else
  {
    LOG_DEBUG()<<"Cannot update contact"<<std::endl;
  }
*/
 /* if (helper.modifyContact(uri, testVCard2, etag))
  {
    LOG_DEBUG()<<"Contact update sucesfully with etag "<<etag<<std::endl;
  }
  else
  {
    LOG_DEBUG()<<"Cannot update contact"<<std::endl;
  }*/

/*
  std::string etag2="123";
  if (helper.modifyContact(uri, testVCard2, etag2))
  {
    LOG_DEBUG()<<"Contact update sucesfully with etag "<<etag2<<std::endl;
  }
  else
  {
    LOG_DEBUG()<<"Cannot update contact"<<std::endl;
  }
*/
  /*if (helper.removeContact(uri, etag2))
  {
    LOG_DEBUG()<<"Contact removed sucesfully"<<std::endl;
  }
  else
  {
    LOG_DEBUG()<<"Cannot remove contact"<<std::endl;
  }*/

  return 0;
}
