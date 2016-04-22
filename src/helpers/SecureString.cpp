/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file SecureString.cpp
 */

#include "SecureString.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sstream>

namespace OpenAB {

SecureString::SecureString() :decoded (NULL)
{

}

SecureString::SecureString(const char* str) :decoded (NULL)
{
  size_t len = strlen(str);
  content.resize(len);
  generateKey(len);
  for (unsigned int i = 0; i < len; ++i)
  {
    content[i] = str[i] ^ key[i];
  }
}

SecureString::SecureString(const std::string& str) :decoded (NULL)
{
  size_t len = str.length();
  content.resize(len);
  generateKey(len);
  for (unsigned int i = 0; i < len; ++i)
  {
    content[i] = str[i] ^ key[i];
  }
}

SecureString::SecureString(const SecureString& other) :decoded (NULL)
{
  content = other.content;
  key = other.key;
}

SecureString& SecureString::operator= (const SecureString &other)
{
  clear();
  content = other.content;
  key = other.key;

  return *this;
}

void SecureString::generateKey(unsigned int len)
{
  key.resize(len);
  for (unsigned int i = 0; i < len; ++i)
  {
    key[i] = rand()%255;
  }
}

char* SecureString::str() const
{
  clearStr();
  unsigned int len = content.size();

  decoded = (char*) malloc (len + 1);
  decoded[len] = '\0';

  if (key.size() == len)
  {
    for (unsigned int i = 0; i < len; ++i)
    {
      decoded[i] = key[i] ^ content[i];
    }
  }

  return decoded;
}

void SecureString::clear()
{
  clearStr();
  key.clear();
  content.clear();
}

void SecureString::clearStr() const
{
  if (NULL != decoded)
  {
    memset(decoded, 0, strlen(decoded));
    free (decoded);
    decoded = NULL;
  }
}

bool SecureString::operator== (const SecureString& other) const
{
  return (key == other.key && content == other.content);
}

SecureString::~SecureString()
{
  clear();
}

} // namespace OpenAB
