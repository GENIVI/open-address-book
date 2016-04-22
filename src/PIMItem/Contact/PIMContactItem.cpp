/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file PIMContactItem.cpp
 */
#include <helpers/Log.hpp>
#include <helpers/StringHelper.hpp>
#include <sstream>
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "PIMContactItem.hpp"
#include "PIMContactItemIndex.hpp"
#include "Pict.hpp"

namespace OpenAB {

VCardField::VCardField(){}
VCardField::VCardField(const std::string& v) :
    value(v) {}

VCardField::~VCardField(){}

bool VCardField::parse(std::string vCardFieldString)
{
  std::size_t position;
  std::string param;

  // find where value has some parameters
  position = vCardFieldString.find_first_of(':');

  //if not store whole string field as value of field
  if(position == std::string::npos)
  {
    value = vCardFieldString;
    return true;
  }

  value = vCardFieldString.substr(position+1);
  vCardFieldString.erase(position);

  //parse parameters
  std::size_t oldPosition = 0;
  while((position = vCardFieldString.find_first_of(';', oldPosition)) != std::string::npos)
  {
    param = vCardFieldString.substr(oldPosition, position - oldPosition);
    oldPosition = position+1;
    processParam(param);
  }

  param = vCardFieldString.substr(oldPosition);
  processParam(param);

  return true;
}

void VCardField::setValue (const std::string& v)
{
  value = v;
}

void VCardField::processParam(std::string paramLine)
{
  std::size_t position = paramLine.find_first_of('=');
  std::string paramName = paramLine.substr(0, position);
  std::set<std::string> paramValues;
  paramLine.erase(0, position+1);

  std::vector<std::string> values = OpenAB::tokenize(paramLine, ',', true, false);
  std::vector<std::string>::iterator it;
  for (it = values.begin(); it != values.end(); ++it)
  {
    OpenAB::eraseAllOccurences((*it), '"');
    paramValues.insert((*it));
  }

  if(!paramValues.empty())
  {
    addParam(paramName, paramValues);
  }
}

void VCardField::addParam(std::string paramName, std::set<std::string> values)
{
  if(paramName.compare(0, 2, "x-") == 0)
  {
    //don't store extension parameters, e.g. X-EVOLUTION-E164
    return;
  }
  if(params.find(paramName) != params.end())
  {
    //this are some additional values for param that was already added
    std::set<std::string>::iterator it;
    for(it = values.begin(); it != values.end(); ++it)
    {
      params[paramName].insert((*it));
    }
  }
  else
  {
    params.insert(std::pair<std::string, std::set<std::string> >(paramName, values));
  }
}

std::string VCardField::toString() const
{
  std::string res;

  std::map<std::string, std::set<std::string> >::const_iterator it1;
  std::set<std::string>::iterator it2;

  bool firstP = true;
  for(it1 = params.begin(); it1 != params.end(); ++it1)
  {
    if(firstP)
    {
      firstP = false;
    }
    else{
      res += ";";
    }

    res+=(*it1).first;
    res+="=";
    bool first = true;
    for(it2 = (*it1).second.begin(); it2 != (*it1).second.end(); ++it2)
    {
      if(first)
      {
        first = false;
      }
      else
      {
        res+=",";
      }
      res+=(*it2);
    }
  }

  if(!firstP)
    res+=":";
  res+=value;

  return res;
}

std::string VCardField::getValue() const
{
  return value;
}

std::map<std::string, std::set<std::string> > VCardField::getParams() const
{
  return params;
}

bool VCardField::operator<(const VCardField& other) const
{
  return toString() < other.toString();
}


PIMContactItem::PIMContactItem() :
    PIMItem(OpenAB::eContact)
{
}

PIMContactItem::~PIMContactItem()
{
}

/* getVcardLine
 * This routine is added to avoid any parsing error
 * due to the line folding rule of the vCard 3.0
 * rfc2425 - 5.8.1
 */
static bool getVcardLine(std::istream& is, std::string& str)
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

bool PIMContactItem::parse(const std::string& vCard)
{
  fields.clear();
  std::string vcard = vCard;
  this->vCard = vCard;
  //1. linearize vcard
  size_t match = 0;
  while (std::string::npos != (match = vcard.find("\r\n ", match + 1)))
  {
    vcard.erase(match, 3);
  }
  while (std::string::npos != (match = vcard.find("\n ", match + 1)))
  {
    vcard.erase(match, 2);
  }

  //2. read all lines, unfolding and trimming
  std::stringstream stream(vcard);
  std::string line;

  while(getVcardLine(stream, line))
  {
     /* unquote special chars
     /  "\," -> ","
     /  "\ " -> " "
     */
    {
      size_t type_start = line.find_first_of(":", 0);
      if (std::string::npos != type_start)
      {
        size_t match = type_start + 1;
        while (std::string::npos != (match = line.find("\\", type_start)))
        {
          if (',' == line.at(match + 1) || ' ' == line.at(match + 1))
          {
            line.erase(match, 1);
          }
          type_start = match + 1;
        }
      }
    }

    trimWhitespaces(line);

    if (line.length() == 0)
      continue;

    //3. split line, parse field
    std::size_t pos1 = line.find_first_of(':');
    std::size_t pos2 = line.find_first_of(';');
    std::string fieldName;
    std::string fieldValue;

    if(pos2 < pos1) pos1 = pos2;

    fieldName = line.substr(0, pos1);

    //1. to lower case
    for(std::string::size_type i = 0; i < fieldName.length(); ++i)
    {
      fieldName[i] = std::tolower(fieldName[i]);
    }

    //ignored fields
    if(fieldName == "begin" ||
       fieldName == "end" ||
       fieldName == "rev" ||
       fieldName == "uid" ||
       fieldName == "prodid" ||
       fieldName.compare(0, 12, "x-evolution-") == 0)
    {
      continue;
    }

    fieldValue = line.substr(pos1+1);
    std::string::size_type fieldValueLen = fieldValue.size();
    if(fieldName == "photo")
    {
      //do not lowercase photo location
      std::string::size_type uriPos = fieldValue.find("://", 0);
      if(uriPos != std::string::npos)
      {
        fieldValueLen = uriPos;
      }
      else
      {
        uriPos = fieldValue.find_last_of(":");
        if(uriPos != std::string::npos)
        {
          fieldValueLen = uriPos;
        }
      }
    }

    for(std::string::size_type i = 0; i < fieldValueLen; ++i)
    {
      fieldValue[i] = std::tolower(fieldValue[i]);
    }

    VCardField field;
    if (fieldName == "note")
    {
      //do not parse NOTE field as according to RFC 2426 it cannot contain additional params,
      //but value itself can contain some characters that will mislead parser
      field.setValue(fieldValue);
    }
    else
    {
      field.parse(fieldValue);
    }

    fields[fieldName].push_back(field);
  }

  //parse name field and generate new fields from it
  if (fields.find("n") != fields.end())
  {
    std::string name = (*fields["n"].begin()).getValue();
    std::vector<std::string> explodedName = OpenAB::tokenize(name, ';');
    //only if name was properly formatted - there should always be 5 fields, some may be empty
    if (explodedName.size() == 5)
    {
      VCardField family;
      family.parse(explodedName.at(0));
      fields["n_family"].push_back(family);

      VCardField given;
      given.parse(explodedName.at(1));
      fields["n_given"].push_back(given);

      VCardField middle;
      middle.parse(explodedName.at(2));
      fields["n_middle"].push_back(middle);

      VCardField prefix;
      prefix.parse(explodedName.at(3));
      fields["n_prefix"].push_back(prefix);

      VCardField suffix;
      suffix.parse(explodedName.at(4));
      fields["n_suffix"].push_back(suffix);
    }
  }

  //@todo add support for multiple photo fields/logo/sound
  if(fields.find("photo") != fields.end())
  {
    //substitute photo with photo checksum (only for embedded photos and file uris)

    //first check validity of photo field
    bool nonLocalUri = false;
    std::map<std::string, std::set<std::string> > photoParams = (*fields["photo"].begin()).getParams();
    if (photoParams.find("value") != photoParams.end())
    {
      if (photoParams["value"].size() == 1)
      {
        if (std::string::npos == (*fields["photo"].begin()).getValue().find("file://", 0))
        {
          nonLocalUri = true;
        }
      }
      else
      {
        LOG_ERROR()<<"More than one value type for PHOTO field - misformatted - ignoring"<<std::endl;
        fields.clear();
        return false;
      }
    }
    else if (photoParams.find("encoding") != photoParams.end())
    {
      if (!(photoParams["encoding"].size() == 1 && (*photoParams["encoding"].begin()) == "b"))
      {
        LOG_ERROR()<<"Unknown encoding for PHOTO field - misformatted - ignoring"<<std::endl;
        fields.clear();
        return false;
      }
    }
    else
    {
      LOG_ERROR()<<"Missformated PHOTO field ignoring"<<std::endl;
      fields.clear();
      return false;
    }

    if (!nonLocalUri)
    {
      unsigned long checksum = VCardPhoto::GetCheckSum((*fields["photo"].begin()));
      std::stringstream ss;
      ss << checksum;
      VCardField newPhotoField(ss.str());

      fields["photo"].clear();
      fields["photo"].push_back(newPhotoField);
    }
  }

  //sort all of the fields in alphabetic order of their values, so if they occur in vCards
  //in different order, vCards still can be recognized as totally equal
  std::map<std::string, std::vector<VCardField> >::iterator it;
  for (it = fields.begin(); it != fields.end(); ++it)
  {
    std::sort((*it).second.begin(), (*it).second.end());
  }


  return true;
}

SmartPtr<PIMItemIndex> PIMContactItem::getIndex()
{
  PIMContactItemIndex *newIndex = new PIMContactItemIndex();;
  std::vector<PIMItemIndex::PIMItemCheck> checks = PIMContactItemIndex::getAllChecks();
  std::vector<PIMItemIndex::PIMItemCheck>::iterator it;
  for (it = checks.begin(); it != checks.end(); ++it)
  {
    std::map<std::string, std::vector<VCardField> >::iterator it2;
    it2 = fields.find((*it).fieldName);
    if (it2 != fields.end())
    {
      std::vector<VCardField>::iterator it3;
      for (it3 = (*it2).second.begin(); it3 != (*it2).second.end(); ++it3)
      {
        if ((*it).fieldRole == PIMItemIndex::PIMItemCheck::eKey)
          newIndex->addKeyField((*it).fieldName, (*it3).toString());
        else
          newIndex->addConflictField((*it).fieldName, (*it3).toString());
      }
    }
  }
  return SmartPtr<PIMItemIndex>(newIndex);
}

std::string PIMContactItem::getRawData() const
{
  return vCard;
}

void PIMContactItem::setId(const PIMItem::ID& id,
                           bool replace)
{
  this->id = id;
  if (replace)
  {
    substituteVCardUID(id);
  }
}

void PIMContactItem::substituteVCardUID(const std::string newUID)
{
  std::string::size_type uidStart = vCard.find("UID:");
  if (uidStart != std::string::npos)
  {
    uidStart += strlen("UID:");
    std::string::size_type uidEnd = vCard.find("\n", uidStart);
    vCard = vCard.erase(uidStart, uidEnd-uidStart);
    vCard.insert(uidStart, newUID);
  }
}

unsigned long VCardPhoto::GetCheckSum(const VCardField& field)
{
  unsigned long checksum = 0;
  std::map<std::string, std::set<std::string> > params = field.getParams();

  enum eType
  {
    eTypeNONE, eTypeBuffer, eTypeFile
  }type;

  unsigned char* buf = NULL;
  size_t size = 0;
  int fd = -1;
  type = eTypeNONE;

  if (params.find("encoding") != params.end())
  {
    if ((*params["encoding"].begin()) == "b")
    {
      type = eTypeBuffer;
      buf = new unsigned char[field.getValue().size()];
      size = field.getValue().size();
      if (0 != base64decode((char*) field.getValue().c_str(),
                            field.getValue().size(),
                            buf, &size))
      {
        LOG_ERROR() << "base64decode failed"<<std::endl;
        delete[] buf;
        buf = NULL;
        size = 0;

        return checksum;
      }

      /*FILE*pf = fopen("out.jpg", "wb");
      for(unsigned int i = 0; i < size; ++i)
      {
        fwrite(&buf[i], sizeof(char), 1, pf);
      }
      fclose(pf);*/
    }
  }
  else if(params.find("value") != params.end())
  {
    if((*params["value"].begin()) == "uri")
    {
      if(field.getValue().find("file://", 0) != std::string::npos)
      {
        type = eTypeFile;
        std::string uri = field.getValue();
        std::string fileName = urlDecode(uri).substr(7);
        /* Open the file for reading. */
        LOG_DEBUG() << "Open: " << fileName<<std::endl;
        fd = open(fileName.c_str(), O_RDONLY);
        if (fd < 0)
        {
          LOG_ERROR() << "open failed: " << strerror(errno)<<std::endl;
          buf = NULL;
          size = 0;
          return checksum;
        }

        /* Get the size of the file. */
        struct stat st;
        if (fstat(fd, &st) < 0)
        {
          LOG_ERROR() << "fstat failed: " << strerror(errno)<<std::endl;
          buf = NULL;
          size = 0;
          return checksum;
        }
        size = st.st_size;
        LOG_DEBUG() << "File Size: " << (int)size<<std::endl;

        /* Memory-map the file. */
        buf = (unsigned char *) mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (buf == MAP_FAILED)
        {
          LOG_ERROR() << "mmap failed: " << strerror(errno)<<std::endl;
          buf = NULL;
          size = 0;
          return checksum;
        }
      }
    }
  }
  for(size_t i=0; i < size; ++i)
  {
    checksum += buf[i];
  }

  if (eTypeFile == type)
  {
    if (NULL != buf)
      munmap(buf, size);
    if (0 != fd)
      close(fd);
  }
  else if(eTypeBuffer == type)
  {
    if (NULL != buf)
      delete[] buf;
  }

  return checksum;
}

} // namespace OpenAB
