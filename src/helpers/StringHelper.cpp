/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file StringHelper.cpp
 */

#include "Log.hpp"
#include "StringHelper.hpp"
#include <algorithm>

namespace OpenAB {

bool contains(const std::vector<std::string>& vec, const std::string& str)
{
  std::vector<std::string>::const_iterator it = vec.begin();
  for (; it != vec.end(); ++it)
  {
    if (str == (*it))
    {
      return true;
    }
  }
  return false;
}

void trimWhitespaces (std::string& str)
{
  size_t pos = str.find_first_not_of("\t\n\v\f\r");
  str.erase(0, pos);
  pos = str.find_last_not_of("\t\n\v\f\r");
  if ( std::string::npos != pos ) // only if trailing whitespace found
  {
    str.erase(pos+1);
  }
}

void trimSpaces (std::string& str)
{
  size_t pos = str.find_first_not_of(" \t\n\v\f\r");
  str.erase(0, pos);
  pos = str.find_last_not_of(" \t\n\v\f\r");
  if ( std::string::npos != pos ) // only if trailing whitespace found
  {
    str.erase(pos+1);
  }
}

std::vector<std::string> tokenize(const std::string& str, char delimiter, bool unique, bool leaveEmptyTokens)
{
  std::vector<std::string> result;
  std::string::size_type tokenStart = 0;
  std::string::size_type tokenStop = 0;
  std::string::size_type tokenLen = 0;

  while (std::string::npos != tokenStop)
  {
    tokenStop = str.find(delimiter, tokenStop);
    if (std::string::npos == tokenStop)
    {
      tokenLen = tokenStop;
    }
    else
    {
      tokenLen = tokenStop - tokenStart;
      tokenStop++;
    }
    std::string token = str.substr(tokenStart, tokenLen);
    tokenStart = tokenStop;

    if (!token.empty() || leaveEmptyTokens)
    {
      trimWhitespaces(token);
      if (unique && contains(result, token))
      {
        continue;
      }
      result.push_back(token);
    }
  }

  return result;
}

void eraseAllOccurences(std::string& str, char toRemove)
{
  str.erase(std::remove(str.begin(), str.end(), toRemove), str.end());
}

std::string cut(const std::string& str,
                const std::string& begin,
                const std::string& end,
                std::string::size_type& pos)
{
  std::string result;

  std::string::size_type tokenStart = 0;
  std::string::size_type tokenStop = 0;
  std::string::size_type tokenLen = 0;

  tokenStart = str.find(begin, pos);
  if (std::string::npos == tokenStart)
  {
    pos = std::string::npos;
    return result;
  }

  tokenStop = str.find(end, tokenStart);
  if (std::string::npos == tokenStop)
  {
    pos = std::string::npos;
    return result;
  }

  tokenLen = tokenStop - tokenStart;
  result = str.substr(tokenStart + begin.length(), tokenLen - begin.length());
  pos = tokenStop;

  return result;
}

std::vector<std::string> parserURL(const std::string& url)
{
  std::vector<std::string> parsedURL;
  parsedURL.resize(4);

  std::string::size_type tokenStart = 0;
  std::string::size_type tokenStop = 0;

  //parse scheme
  tokenStop = url.find("://", tokenStart);
  if (std::string::npos != tokenStop)
  {
    parsedURL[0] = url.substr(0, tokenStop);
    tokenStart = tokenStop + 3;
  }

  //parse host
  tokenStop = url.find("/", tokenStart);
  if (std::string::npos != tokenStop)
  {
    parsedURL[1] = url.substr(tokenStart, tokenStop - tokenStart);
    tokenStart = tokenStop + 1;
  }
  else
  {
    //provided url is just host name
    parsedURL[1] = url.substr(tokenStart);
    return parsedURL;
  }

  //parse path
  tokenStop = url.find("?", tokenStart);
  if (std::string::npos != tokenStop)
  {
    parsedURL[2] = url.substr(tokenStart, tokenStop - tokenStart);
    tokenStart = tokenStop + 1;
  }
  else
  {
    //url does not contains query
    parsedURL[2] = url.substr(tokenStart);
    return parsedURL;
  }

  //what rest is query
  parsedURL[3] = url.substr(tokenStart);

  return parsedURL;
}

std::string parseURLHostPart(const std::string& url)
{
  std::string parsed;
  std::vector<std::string> parsedURL = OpenAB::parserURL(url);
  if (!parsedURL[0].empty())
  {
    parsed = parsedURL[0] + "://" + parsedURL[1];
  }
  else
  {
    parsed = parsedURL[1];
  }

  return parsed;
}

void substituteAll (std::string& str, const std::string& from, const std::string& to)
{
  std::string::size_type startPos = 0;
  startPos = str.find(from, startPos);
  while (startPos != std::string::npos)
  {
    str.replace(startPos, from.length(), to);
    startPos += to.length();
    startPos = str.find(from, startPos);
  }
}

bool beginsWith (const std::string& str, const std::string& substr)
{
  if (!substr.empty() && 0 == str.compare(0, substr.length(), substr))
  {
    return true;
  }
  return false;
}

bool endsWith (const std::string& str, const std::string& substr)
{
  if (!substr.empty() && 0 == str.compare(str.length() - substr.length(), substr.length(), substr))
  {
    return true;
  }
  return false;
}

bool getUnfoldedLine(std::istream& is, std::string& str)
{
  std::string line;
  str.erase();
  while (is.good())
  {
    std::getline(is, line);
    str += line;
    if (is.eof())
      return true;
    char c;
    is.get(c);
    if (' ' != c)
    {
      is.putback(c);
      return true;
    }
    OpenAB::trimWhitespaces(str);
  }
  return false;
}

void linearize(std::string& str)
{
  size_t match = 0;
  while (std::string::npos != (match = str.find("\r\n ", match + 1)))
  {
    str.erase(match, 3);
  }
  while (std::string::npos != (match = str.find("\n ", match + 1)))
  {
    str.erase(match, 2);
  }
}

void unquoteSpecialCharacters(std::string& str)
{
  size_t type_start = str.find_first_of(":", 0);
  if (std::string::npos != type_start)
  {
    size_t match = type_start + 1;
    while (std::string::npos != (match = str.find("\\", type_start)))
    {
      if (',' == str.at(match + 1) || ' ' == str.at(match + 1))
      {
        str.erase(match, 1);
      }
      type_start = match + 1;
    }
  }
}

std::string percentDecode(const std::string& uri)
{
  std::string decoded = uri;

  for (unsigned int i = 0; i < decoded.length(); i++)
  {
    if (decoded[i] == '%')
    {

      std::string code = decoded.substr(i+1, 2);
      if (code.length() != 2)
      {
        continue;
      }
      unsigned int c;
      std::stringstream ss;
      ss << std::hex << code;
      ss >> c;

      ss.str("");
      ss.clear();
      ss<<(char)c;

      decoded.replace(i, 3, ss.str());
    }
  }

  return decoded;
}


} // namespace IasOpenAB
