/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file Variant.cpp
 */

#include "Variant.hpp"
#include <helpers/Log.hpp>

namespace OpenAB {

Variant::Variant()
  : dataType(INVALID)
{

}

Variant::Variant(char value) :
    dataType(CHAR)
{
  data.c = value;
}

Variant::Variant(bool value) :
    dataType(BOOL)
{
  data.b = value;
}

Variant::Variant(int32_t value) :
    dataType(INTEGER)
{
  data.i = value;
}

Variant::Variant(u_int32_t value) :
    dataType(INTEGER)
{
  data.i = value;
}

Variant::Variant(double value) :
    dataType(DOUBLE)
{
  data.d = value;
}

Variant::Variant(const std::string& value) :
    dataType(STRING)
{
  stringData = value;
}

Variant::Variant(const OpenAB::SecureString& value) :
    dataType(SECURE_STRING)
{
  secureStringData = value;
}


Variant::Variant(void* value) :
    dataType(POINTER)
{
  data.p = value;
}

Variant::Variant(const char* value) :
    dataType(STRING)
{
  stringData = std::string(value);
}

Variant::Variant(const Variant &other)
{
  clear();
  dataType = other.dataType;
  data = other.data;
  stringData = other.stringData;
  secureStringData = other.secureStringData;
}

Variant& Variant::operator=(Variant const &other)
{
  clear();
  dataType = other.dataType;
  data = other.data;
  stringData = other.stringData;
  secureStringData = other.secureStringData;

  return *this;
}

bool Variant::operator==(Variant const &other)
{
  if (dataType != other.dataType)
    return false;

  switch(dataType)
  {
    case CHAR:
      return (data.c == other.data.c);
    case BOOL:
      return (data.b == other.data.b);
    case INTEGER:
      return (data.i == other.data.i);
    case DOUBLE:
      return (data.d == other.data.d);
    case POINTER:
      return (data.p == other.data.p);
    case STRING:
      return (stringData == other.stringData);
    case SECURE_STRING:
      return (secureStringData == other.secureStringData);
    default:
      return false;
  }
}

void Variant::set(char value)
{
  clear();
  dataType = CHAR;
  data.c = value;
}

void Variant::set(bool value)
{
  clear();
  dataType = BOOL;
  data.b = value;
}

void Variant::set(int32_t value)
{
  clear();
  dataType = INTEGER;
  data.i = value;
}

void Variant::set(u_int32_t value)
{
  clear();
  dataType = INTEGER;
  data.i = value;
}

void Variant::set(double value)
{
  clear();
  dataType = DOUBLE;
  data.d = value;
}

void Variant::set(const std::string& value)
{
  clear();
  dataType = STRING;
  stringData = value;
}

void Variant::set(const OpenAB::SecureString& value)
{
  clear();
  dataType = SECURE_STRING;
  secureStringData = value;
}

void Variant::set(void* value)
{
  clear();
  dataType = POINTER;
  data.p = value;
}

void Variant::set(const char* value)
{
  clear();
  dataType = STRING;
  stringData = std::string(value);
}

char Variant::getChar() const
{
  if (dataType != CHAR)
    return 0;
  return data.c;
}

bool Variant::getBool() const
{
  if (dataType != BOOL)
    return false;
  return data.b;
}

int32_t Variant::getInt() const
{
  if (dataType != INTEGER)
    return 0;
  return data.i;
}

double Variant::getDouble() const
{
  if (dataType != DOUBLE)
    return 0.0;
  return data.d;
}

std::string Variant::getString() const
{
  if (dataType != STRING)
    return std::string();

  return stringData;
}

OpenAB::SecureString Variant::getSecureString() const
{
  if (dataType != SECURE_STRING)
    return OpenAB::SecureString();

  return secureStringData;
}

void* Variant::getPointer() const
{
  if (dataType != POINTER)
    return NULL;
  return data.p;
}

void Variant::clear()
{
  dataType = INVALID;
  secureStringData.clear();
  stringData.clear();
  data.p = 0;
}

Variant::DataType Variant::getType() const
{
  return dataType;
}

bool Variant::invalid() const
{
  return (dataType == INVALID);
}

Variant::~Variant()
{
  clear();
}

} // namespace OpenAB
