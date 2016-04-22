/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * @file GDataContactsConverter.cpp
 */

#include "GDataContactsConverter.hpp"
#include <sstream>
#include <helpers/Log.hpp>
#include <algorithm>
#include <PIMItem/Contact/Pict.hpp>
#include <helpers/StringHelper.hpp>

std::vector<GDataContactsConverter::FieldParseFunction> GDataContactsConverter::fieldParsers;

std::string substringAfter(const std::string& str, const char c)
{
  std::string substr = "";
  std::size_t idx = str.find_last_of(c);
  if (idx != std::string::npos)
  {
    substr = str.substr(idx+1);
    std::transform(substr.begin(), substr.end(), substr.begin(), ::toupper);
  }
  return substr;
}

std::string parseRelationType(const std::string& relation)
{
  return substringAfter(relation, '#');
}

std::string parsePhotoType(const std::string& mimeType)
{
  return substringAfter(mimeType, '/');
}

void appendType(const std::string& rel, bool pref, std::ostringstream& oss)
{
  std::string label = parseRelationType(rel);
  if (!label.empty())
  {
    std::vector<std::string> labels = OpenAB::tokenize(label, '_', true,false);
    std::vector<std::string>::iterator it = labels.begin();
    for (; it != labels.end(); ++it)
    {
      oss<<";TYPE="<<(*it);
    }
  }
  if (pref)
  {
    oss<<";TYPE=PREF";
  }
}

/*!
 * @brief Trim vCard lines to 76 characters, wrap too long lines.
 */
std::string reformatVCard(const std::string& vCard)
{
  std::stringstream oss;
  int lenLine = 0;
  for (unsigned int i = 0; i < vCard.size(); ++i)
  {
    if (vCard[i] == '\n')
    {
      lenLine = 0;
    }
    if (vCard[i] == '\r')
    {
      continue;
    }
    oss<<vCard[i];
    lenLine++;
    if (lenLine > 75)
    {
      oss<<std::endl;
      oss<<" ";
      lenLine = 1;
    }
  }
  return oss.str();
}

bool parseName(GDataContactsContact* contact, std::ostringstream& oss)
{
  const gchar* structuredName[5] = {NULL};
  unsigned int formatedNameOrder[5] = {3, 1, 2, 0, 4};

  GDataGDName* name = gdata_contacts_contact_get_name(contact);
  if (name)
  {
    structuredName[0] = gdata_gd_name_get_family_name(name);
    structuredName[1] = gdata_gd_name_get_given_name(name);
    structuredName[2] = gdata_gd_name_get_additional_name(name);
    structuredName[3] = gdata_gd_name_get_prefix(name);
    structuredName[4] = gdata_gd_name_get_suffix(name);
  }

  oss<<"N:";
  for (unsigned int i = 0; i < 5; ++i)
  {
    if (NULL != structuredName[i])
    {
      oss<<structuredName[i];
    }
    if (i != 4)
      oss<<";";
  }
  oss<<std::endl;

  oss<<"FN:";
  bool first = true;
  for (unsigned int i = 0; i < 5; ++i)
  {
    int j = formatedNameOrder[i];
    if (structuredName[j])
    {
      if (!first)
      {
        oss<<" ";
      }
      else
      {
        first = false;
      }

      oss<<structuredName[j];
    }
  }

  oss<<std::endl;

  return true;
}

bool parseNickname(GDataContactsContact* contact, std::ostringstream& oss)
{
  const gchar* nickname = gdata_contacts_contact_get_nickname(contact);
  if (nickname)
  {
    oss<<"NICKNAME:"<<nickname<<std::endl;
  }

  return true;
}

bool parseBday(GDataContactsContact* contact, std::ostringstream& oss)
{
  GDate* birthday = g_date_new();
  if (gdata_contacts_contact_get_birthday(contact, birthday))
  {
    oss<<"BDAY:"<<g_date_get_year(birthday)<<"-";
    oss<<std::setw(2)<<std::setfill('0')<<(int)g_date_get_month(birthday)<<"-";
    oss<<(int)g_date_get_day(birthday)<<std::endl;
    oss<<std::setw(0);
  }
  g_date_free(birthday);

  return true;
}

bool parseTitleOrg(GDataContactsContact* contact, std::ostringstream& oss)
{
  GList* organizations = gdata_contacts_contact_get_organizations(contact);
  GList* o = organizations;

  for (;o != NULL; o = o->next)
  {
    GDataGDOrganization* org = (GDataGDOrganization*) o->data;
    if (org)
    {
      const gchar* orgName = gdata_gd_organization_get_name(org);
      const gchar* title = gdata_gd_organization_get_title(org);

      if (orgName)
      {
        oss<<"ORG:"<<orgName<<std::endl;
      }
      if (title)
      {
        oss<<"TITLE:"<<title<<std::endl;
      }
    }
  }

  return true;
}

bool parseWebiste(GDataContactsContact* contact, std::ostringstream& oss)
{
  GList* websites = gdata_contacts_contact_get_websites(contact);
  GList* w = websites;

  for (;w != NULL; w = w->next)
  {
    GDataGContactWebsite* website = (GDataGContactWebsite*) w->data;
    if (website)
    {
      const gchar* url = gdata_gcontact_website_get_uri (website);

      if (url)
      {
        oss<<"URL:"<<url<<std::endl;
      }
    }
  }

  return true;
}

bool parseEmail(GDataContactsContact* contact, std::ostringstream& oss)
{
  GList* emails = gdata_contacts_contact_get_email_addresses(contact);
  GList* e = emails;

  for (;e != NULL; e = e->next)
  {
    GDataGDEmailAddress* email = (GDataGDEmailAddress*) e->data;
    if (email)
    {
      const gchar* adr = gdata_gd_email_address_get_address (email);
      const gchar* rel = gdata_gd_email_address_get_relation_type (email);
      gboolean pref = gdata_gd_email_address_is_primary (email);

      if (adr)
      {
        oss<<"EMAIL";
        appendType (rel, pref, oss);
        oss<<":"<<adr<<std::endl;
      }
    }
  }

  return true;
}

bool parsePhone(GDataContactsContact* contact, std::ostringstream& oss)
{
  GList* phones = gdata_contacts_contact_get_phone_numbers(contact);
  GList* p = phones;

  for (;p != NULL; p = p->next)
  {
    GDataGDPhoneNumber* phone = (GDataGDPhoneNumber*) p->data;
    if (phone)
    {
      const gchar* number = gdata_gd_phone_number_get_number (phone);
      const gchar* rel = gdata_gd_phone_number_get_relation_type (phone);
      gboolean pref = gdata_gd_phone_number_is_primary (phone);

      if (number)
      {
        oss<<"TEL";
        appendType (rel, pref, oss);
        oss<<":"<<number<<std::endl;
      }
    }
  }

  return true;
}

bool parseAddress(GDataContactsContact* contact, std::ostringstream& oss)
{
  GList* addresses = gdata_contacts_contact_get_postal_addresses(contact);
  GList* a = addresses;

  for (;a != NULL; a = a->next)
  {
    GDataGDPostalAddress* address = (GDataGDPostalAddress*) a->data;
    if (address)
    {
      const gchar* structuredAdr[7] = {NULL};
      structuredAdr[0] = gdata_gd_postal_address_get_po_box (address);
      structuredAdr[1] = NULL; //extended address
      structuredAdr[2] = gdata_gd_postal_address_get_street (address);
      structuredAdr[3] = gdata_gd_postal_address_get_city (address);
      structuredAdr[4] = gdata_gd_postal_address_get_region (address);
      structuredAdr[5] = gdata_gd_postal_address_get_postcode (address);
      structuredAdr[6] = gdata_gd_postal_address_get_country (address);

      const gchar* rel = gdata_gd_postal_address_get_relation_type (address);
      gboolean pref = gdata_gd_postal_address_is_primary (address);

      if (structuredAdr[0] ||
          structuredAdr[1] ||
          structuredAdr[2] ||
          structuredAdr[3] ||
          structuredAdr[4] ||
          structuredAdr[5] ||
          structuredAdr[6])
      {
        oss<<"ADR";
        appendType (rel, pref, oss);
        oss<<":";

        for (unsigned int i = 0; i < 7; ++i)
        {
          if (NULL != structuredAdr[i])
          {
            oss<<structuredAdr[i];
          }
          if (i != 6)
            oss<<";";
        }

        oss<<std::endl;
      }
    }
  }

  return true;
}

bool parsePhoto(guint8* photoData,
                gsize photoDataLen,
                gchar* photoType,
                std::ostringstream& oss)
{
  unsigned char* outBuf = new unsigned char[photoDataLen*2];
  size_t outBufLen = photoDataLen*2;
  if (0 != base64encode((const char*)photoData, (size_t)photoDataLen, outBuf, &outBufLen))
  {
    delete[] outBuf;
    return false;
  }


  outBuf[outBufLen] = '\0';
  oss<<"PHOTO;ENCODING=B;TYPE="<<parsePhotoType(photoType)<<":"<<outBuf<<std::endl;

  delete[] outBuf;
  return true;
}

bool parseNote(GDataContactsContact* contact, std::ostringstream& oss)
{
  const gchar* note = gdata_entry_get_content (GDATA_ENTRY(contact));
  if (note)
  {
    oss << "NOTE:" << note << std::endl;
  }

  return true;
}

GDataContactsConverter::GDataContactsConverter()
{

}

GDataContactsConverter::~GDataContactsConverter()
{
}

void GDataContactsConverter::populateParsers()
{
  if (fieldParsers.empty())
 {
   fieldParsers.push_back(parseName);
   fieldParsers.push_back(parseNickname);
   fieldParsers.push_back(parseBday);
   fieldParsers.push_back(parseTitleOrg);
   fieldParsers.push_back(parseWebiste);
   fieldParsers.push_back(parseEmail);
   fieldParsers.push_back(parsePhone);
   fieldParsers.push_back(parseAddress);
   fieldParsers.push_back(parseNote);
 }
}

bool GDataContactsConverter::convertCommon(GDataContactsContact* contact, std::ostringstream& oss)
{
  bool success = false;
  populateParsers();

  oss<<"BEGIN:VCARD"<<std::endl;
  oss<<"VERSION:3.0"<<std::endl;

  std::vector<FieldParseFunction>::iterator it;
  for (it = fieldParsers.begin(); it != fieldParsers.end(); ++it)
  {
    success = (*it)(contact, oss);
    if (!success)
    {
      break;
    }
  }

  return success;
}

bool GDataContactsConverter::convert(GDataContactsContact* contact, std::string& vCard)
{

  bool success = true;
  std::ostringstream oss;

  if (NULL == contact)
  {
    return false;
  }

  success = convertCommon(contact, oss);

  oss<<"END:VCARD";
  if (success)
  {
    vCard = reformatVCard(oss.str());
  }

  return success;
}

bool GDataContactsConverter::convert(GDataContactsContact* contact,
                                     guint8* photoData,
                                     gsize photoDataLen,
                                     gchar* photoType,
                                     std::string& vCard)
{
  bool success = true;
  std::ostringstream oss;

  if (NULL == contact)
  {
    return false;
  }

  success = convertCommon(contact, oss);
  parsePhoto(photoData, photoDataLen, photoType, oss);

  oss<<"END:VCARD";
  if (success)
  {
    vCard = reformatVCard(oss.str());
  }

  return success;
}
