/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file Pict.cpp
 */

#include "Pict.hpp"

#include <cerrno>
#include <cstring>
#include <cstdio>
#include <fcntl.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdint.h>

#include <helpers/Log.hpp>

#define WHITESPACE 64
#define EQUALS     65
#define INVALID    66

static const unsigned char base64DecodeMap[] =
{ 66, 66, 66, 66, 66, 66, 66, 66, 66, 64, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
  66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 62, 66, 66, 66, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
  66, 66, 66, 65, 66, 66, 66, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
  25, 66, 66, 66, 66, 66, 66, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
  48, 49, 50, 51, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
  66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
  66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
  66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
  66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66 };


static const unsigned char base64EncodeMap[] =
{
 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
 '8', '9', '+', '/'};

int base64decode(const char *in, size_t inLen, unsigned char *out, size_t *outLen)
{
  const char *end = in + inLen;
  size_t buf = 1, len = 0;

  while (in < end)
  {
    unsigned char c = base64DecodeMap[(unsigned int)(*in++)];
    switch (c)
    {
      case WHITESPACE:
        continue; /* skip whitespace */
      case INVALID:
        return 1; /* invalid input, return error */
      case EQUALS: /* pad character, end of data */
        in = end;
        continue;
      default:
        buf = buf << 6 | c;

        /* If the buffer is full, split it into bytes */
        if (buf & 0x1000000)
        {
          if ((len += 3) > *outLen)
            return 1; /* buffer overflow */
          *out++ = buf >> 16;
          *out++ = buf >> 8;
          *out++ = buf;
          buf = 1;
        }
    }
  }

  if (buf & 0x40000)
  {
    if ((len += 2) > *outLen)
      return 1; /* buffer overflow */
    *out++ = buf >> 10;
    *out++ = buf >> 2;
  }
  else if (buf & 0x1000)
  {
    if (++len > *outLen)
      return 1; /* buffer overflow */
    *out++ = buf >> 4;
  }

  *outLen = len; /* modify to reflect the actual output size */

  return 0;
}

int base64encode(const char *in, size_t inLen, unsigned char *out, size_t *outLen)
{
  const char *end = in + inLen;
  size_t buf = 1, len = 0;
  char block[4];
  uint32_t *blockRaw = (uint32_t*)block;
  unsigned char encoded;
  int paddingSize = 0;

  block[0] = 0;
  while (in < end)
  {
    block[3] = (*in++);
    if (in < end)
    {
      block[2] = (*in++);
    }
    else
    {
      block[2] = 0;
      block[1] = 0;
      paddingSize = 2;
    }
    if (in < end)
    {
      block[1] = (*in++);
    }
    else if(paddingSize == 0)
    {
      block[1] = 0;
      paddingSize = 1;
    }

    len +=4;
    if (len > *outLen)
      return 1; //buffer overflow

    buf = 0x3F << 26;
    encoded = base64EncodeMap[(*blockRaw & buf) >> 26];
    *out++ = encoded;

    buf = 0x3F << 20;
    encoded = base64EncodeMap[(*blockRaw & buf) >> 20];
    *out++ = encoded;

    buf = 0x3F << 14;
    encoded = base64EncodeMap[(*blockRaw & buf) >> 14];
    *out++ = encoded;

    buf = 0x3F << 8;
    encoded = base64EncodeMap[(*blockRaw & buf) >> 8];
    *out++ = encoded;

    if (paddingSize == 1)
    {
      *(out - 1) = '=';
    }
    else if (paddingSize == 2)
    {
      *(out - 1) = '=';
      *(out - 2) = '=';
    }
  }
  *out = '\0';
  *outLen = len; /* modify to reflect the actual output size */

  return 0;
}

std::string urlDecode(const std::string & SRC)
{
  std::string ret;
  char ch;
  std::string::size_type i;
  unsigned int ii;
  for (i = 0; i < SRC.length(); i++)
  {
    if (int(SRC.at(i)) == 37)
    {
      sscanf(SRC.substr(i + 1, 2).c_str(), "%x", &ii);
      ch = static_cast<char>(ii);
      ret += ch;
      i = i + 2;
    }
    else
    {
      ret += SRC.at(i);
    }
  }
  return (ret);
}

