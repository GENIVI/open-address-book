/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */
#include <gtest/gtest.h>
#include <string>
#include "PIMItem/Contact/PIMContactItem.hpp"
#include "PIMItem/Contact/Pict.hpp"

namespace OpenAB
{

class PIMContactItemTests: public ::testing::Test
{
public:
    PIMContactItemTests() : ::testing::Test()
    {
    }

    ~PIMContactItemTests()
    {
    }

protected:
    // Sets up the test fixture.
    virtual void SetUp()
    {
    }

    // Tears down the test fixture.
    virtual void TearDown()
    {

    }
};

static const char* testVcard0 = \
"BEGIN:VCARD\n\r"
"VERSION:3.0\n\r"
"N:Surname;Name;Middle;Perfix;Suffix\n\r"
"FN:Prefix Name Middle Surname Suffix\n\r"
"ORG:Bubba Shrimp Co.\n\r"
"TITLE:Shrimp Man\n\r"
"PHOTO;VALUE=URI;TYPE=GIF:http://www.example.com/dir_photos/my_photo.gif\n\r"
//additional X-EVOLUTION param that should be ignored
"TEL;TYPE=WORK,VOICE;X-EVOLUTION-E164=891234,\"+49\":(111) 555-100\n\r"
//type params written in two different ways
"ADR;TYPE=WORK;TYPE=HOME,OFFICE:;;100 Waters Edge;Baytown;LA;0;United States of America\n\r"
"EMAIL;TYPE=PREF;TYPE=INTERNET:name.surname@example.com\n\r"
"GEO:39.95;-75.1667\n\r"
"BDAY:19700310\n\r"
//fieldst that should be ignored at all
"REV:123\n\r"
"UID:id1234\n\r"
"PRODID:OPENAB\n\r"
"X-EVOLUTION-LABEL:label\n\r"
"END:VCARD\n\r";

//wrong value - only possible is URI
static const char* testVcardMisformattedPhoto1 = \
"BEGIN:VCARD\n\r"
"VERSION:3.0\n\r"
"N:Surname;Name;Middle;Perfix;Suffix\n\r"
"PHOTO;VALUE=URI;VALUE=URL;TYPE=GIF:http://www.example.com/dir_photos/my_photo.gif;\n\r"
"END:VCARD\n\r";

//wrong encoding - only possible is "b"
static const char* testVcardMisformattedPhoto2 = \
"BEGIN:VCARD\n\r"
"VERSION:3.0\n\r"
"N:Surname;Name;Middle;Perfix;Suffix\n\r"
"PHOTO;ENCODING=a;TYPE=JPEG:MIICajCCAdOgAwIBAgICBEUwDQYJKoZIhvcN\n\r"
"END:VCARD\n\r";

//two different encodings
static const char* testVcardMisformattedPhoto3 = \
"BEGIN:VCARD\n\r"
"VERSION:3.0\n\r"
"N:Surname;Name;Middle;Perfix;Suffix\n\r"
"PHOTO;ENCODING=b;ENCODING=a;TYPE=JPEG:MIICajCCAdOgAwIBAgICBEUwDQYJKoZIhvcN\n\r"
"END:VCARD\n\r";

//no value or encoding param
static const char* testVcardMisformattedPhoto4 = \
"BEGIN:VCARD\n\r"
"VERSION:3.0\n\r"
"N:Surname;Name;Middle;Perfix;Suffix\n\r"
"PHOTO;TYPE=JPEG:MIICajCCAdOgAwIBAgICBEUwDQYJKoZIhvcN\n\r"
"END:VCARD\n\r";

static const char* testVcardWithEmbeddedPhoto = \
"BEGIN:VCARD\n\r"
"VERSION:3.0\n\r"
"N:aaa\n\r"
"PHOTO;ENCODING=B;TYPE=JPEG:/9j/4AAQSkZJRgABAQEASABIAAD//gATQ3JlYXRlZCB3aXRoIEdJ\n\r"
" TVD/2wBDAAMCAgMCAgMDAwME\n\r"
" AwMEBQgFBQQEBQoHBwYIDAoMDAsKCwsNDhIQDQ4RDgsLEBYQERMUFRUVDA8XGBYUGBIUFRT/2wBD\n\r"
" AQMEBAUEBQkFBQkUDQsNFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQU\n\r"
" FBQUFBQUFBT/wgARCAAgACADAREAAhEBAxEB/8QAFgABAQEAAAAAAAAAAAAAAAAABgcI/8QAFAEB\n\r"
" AAAAAAAAAAAAAAAAAAAAAP/aAAwDAQACEAMQAAABy6IxIJSJiISiQiYiEokImIhKJD//xAAWEAEB\n\r"
" AQAAAAAAAAAAAAAAAAADADH/2gAIAQEAAQUCHQggh0IIIdCCCLQggv/EABQRAQAAAAAAAAAAAAAA\n\r"
" AAAAAED/2gAIAQMBAT8BB//EABQRAQAAAAAAAAAAAAAAAAAAAED/2gAIAQIBAT8BB//EABQQAQAA\n\r"
" AAAAAAAAAAAAAAAAAED/2gAIAQEABj8CB//EABYQAQEBAAAAAAAAAAAAAAAAAEEAEP/aAAgBAQAB\n\r"
" PyGDGOBjHExjmYxv/9oADAMBAAIAAwAAABCSAQCQCQT/xAAUEQEAAAAAAAAAAAAAAAAAAABA/9oA\n\r"
" CAEDAQE/EAf/xAAUEQEAAAAAAAAAAAAAAAAAAABA/9oACAECAQE/EAf/xAAWEAEBAQAAAAAAAAAA\n\r"
" AAAAAAAAAaH/2gAIAQEAAT8QvSyyytrLLK0ssspsss//2Q==\n\r"
"\n\r"
"END:VCARD\n\r";


static const char* testVCardUnqoteSpecialCharacters = \
"BEGIN:VCARD\n\r"
"VERSION:3.0\n\r"
"N:Surname\\,\\ z\\a;Name;Middle;Perfix;Suffix\n\r"
"END:VCARD\n\r";

static const char* testParseInfiniteLoopIssue = \
"BEGIN:VCARD\n"
"VERSION:3.0\n"
"N:Surname;Name;Middle;Perfix;Suffix\n"
"END:VCARD\n";


TEST_F(PIMContactItemTests, testConstructor)
{
  PIMContactItem testItem;
  ASSERT_EQ(OpenAB::eContact, testItem.getType());
}

TEST_F(PIMContactItemTests, testParsing)
{
  PIMContactItem testItem;
  ASSERT_TRUE(testItem.parse(testVcard0));
  ASSERT_EQ(testVcard0, testItem.getRawData());

  std::map<std::string, std::vector<VCardField> > fields = testItem.getFields();
  //there should be exactly 16 fields
  ASSERT_EQ(16, fields.size());

  //check "N" field
  std::map<std::string, std::vector<VCardField> >::iterator it;
  it = fields.find("n");
  ASSERT_TRUE(it != fields.end());
  //there was only one "N" field
  ASSERT_EQ(1, (*it).second.size());
  ASSERT_EQ("surname;name;middle;perfix;suffix", (*it).second[0].getValue());

  //check "FN" field
  it = fields.find("fn");
  ASSERT_TRUE(it != fields.end());
  //there was only one "FN" field
  ASSERT_EQ(1, (*it).second.size());
  ASSERT_EQ("prefix name middle surname suffix", (*it).second[0].getValue());

  //check "ORG" field
  it = fields.find("org");
  ASSERT_TRUE(it != fields.end());
  //there was only one "ORG" field
  ASSERT_EQ(1, (*it).second.size());
  ASSERT_EQ("bubba shrimp co.", (*it).second[0].getValue());

  //check "TITLE" field
  it = fields.find("title");
  ASSERT_TRUE(it != fields.end());
  //there was only one "TITLE" field
  ASSERT_EQ(1, (*it).second.size());
  ASSERT_EQ("shrimp man", (*it).second[0].getValue());

  //check "GEO" field
  it = fields.find("geo");
  ASSERT_TRUE(it != fields.end());
  //there was only one "GEO" field
  ASSERT_EQ(1, (*it).second.size());
  ASSERT_EQ("39.95;-75.1667", (*it).second[0].getValue());

  //check "BDAY" field
  it = fields.find("bday");
  ASSERT_TRUE(it != fields.end());
  //there was only one "BDAY" field
  ASSERT_EQ(1, (*it).second.size());
  ASSERT_EQ("19700310", (*it).second[0].getValue());

  //check "PHOTO" field
  it = fields.find("photo");
  ASSERT_TRUE(it != fields.end());
  //there was only one "photo" field
  ASSERT_EQ(1, (*it).second.size());
  ASSERT_EQ("http://www.example.com/dir_photos/my_photo.gif", (*it).second[0].getValue());

  //check "TEL" field
  it = fields.find("tel");
  ASSERT_TRUE(it != fields.end());
  //there was only one "tel" field
  ASSERT_EQ(1, (*it).second.size());
  ASSERT_EQ("(111) 555-100", (*it).second[0].getValue());
  std::map<std::string, std::set<std::string> > fieldParams;
  fieldParams = (*it).second[0].getParams();
  //there should be only "type" param
  ASSERT_EQ(1, fieldParams.size());
  std::map<std::string, std::set<std::string> >::iterator it2;
  it2 = fieldParams.find("type");
  ASSERT_TRUE(it2 != fieldParams.end());
  //there should be only one values for "type" param - work, perf should be discarded
  ASSERT_EQ(2, fieldParams["type"].size());
  std::set<std::string>::iterator it3 = fieldParams["type"].begin();
  for (;it3 != fieldParams["type"].end(); ++it3)
  {
    ASSERT_TRUE((*it3) == "voice" || (*it3) == "work");
  }

  //check "ADR" field
  it = fields.find("adr");
  ASSERT_TRUE(it != fields.end());
  //there was only one "adr" field
  ASSERT_EQ(1, (*it).second.size());
  ASSERT_EQ(";;100 waters edge;baytown;la;0;united states of america", (*it).second[0].getValue());
  fieldParams = (*it).second[0].getParams();
  //there should be only "type" param
  ASSERT_EQ(1, fieldParams.size());
  it2 = fieldParams.find("type");
  ASSERT_TRUE(it2 != fieldParams.end());
  //there should be one value for "type" param - work, voice
  ASSERT_EQ(3, fieldParams["type"].size());
  it3 = fieldParams["type"].begin();
  for (;it3 != fieldParams["type"].end(); ++it3)
  {
    ASSERT_TRUE((*it3) == "work" || (*it3) == "home" || (*it3) == "office");
  }

  //check "EMAIL" field
  it = fields.find("email");
  ASSERT_TRUE(it != fields.end());
  //there was only one "email" field
  ASSERT_EQ(1, (*it).second.size());
  ASSERT_EQ("name.surname@example.com", (*it).second[0].getValue());
  fieldParams = (*it).second[0].getParams();
  //there should be only "type" param
  ASSERT_EQ(1, fieldParams.size());
  it2 = fieldParams.find("type");
  ASSERT_TRUE(it2 != fieldParams.end());
  //there should be only two values for "type" param - internet, perf should NOT be discarded
  ASSERT_EQ(2, fieldParams["type"].size());
  it3 = fieldParams["type"].begin();
  for (;it3 != fieldParams["type"].end(); ++it3)
  {
    ASSERT_TRUE((*it3) == "internet" || (*it3) == "pref");
  }
}

TEST_F(PIMContactItemTests, testParsingInfiniteLoopIssue)
{
  PIMContactItem testItem;
  ASSERT_TRUE(testItem.parse(testParseInfiniteLoopIssue));
}

TEST_F(PIMContactItemTests, testMisformattedPhoto)
{
  PIMContactItem item;
  ASSERT_FALSE(item.parse(testVcardMisformattedPhoto1));
  ASSERT_FALSE(item.parse(testVcardMisformattedPhoto2));
  ASSERT_FALSE(item.parse(testVcardMisformattedPhoto3));
  ASSERT_FALSE(item.parse(testVcardMisformattedPhoto4));
}

TEST_F(PIMContactItemTests, testEmbeddedPhoto)
{
  PIMContactItem item;
  ASSERT_TRUE(item.parse(testVcardWithEmbeddedPhoto));
  std::map<std::string, std::vector<VCardField> > fields = item.getFields();
  ASSERT_EQ("3276", (*fields["photo"].begin()).getValue());
}

TEST_F(PIMContactItemTests, testUnqoteSpecialCharacters)
{
  PIMContactItem item;
  ASSERT_TRUE(item.parse(testVCardUnqoteSpecialCharacters));
  std::map<std::string, std::vector<VCardField> > fields = item.getFields();
  ASSERT_EQ("surname, z\\a;name;middle;perfix;suffix", (*fields["n"].begin()).getValue());
}

TEST_F(PIMContactItemTests, testPhotoCheckSumEmbedded)
{
  VCardField field;
  field.parse("encoding=b:MTIzNDU2Nzg5MAo=");
  ASSERT_EQ(535, VCardPhoto::GetCheckSum(field));
}

TEST_F(PIMContactItemTests, testPhotoCheckSumEmbeddedWrongEncoding)
{
  VCardField field;
  field.parse("encoding=a:MTIzNDU2Nzg5MAo=");
  ASSERT_EQ(0, VCardPhoto::GetCheckSum(field));
}

TEST_F(PIMContactItemTests, testPhotoCheckSumEmbeddedBase64Error)
{
  VCardField field;
  field.parse("encoding=b:M T I zNDU2Nzg5MAo=");
  ASSERT_EQ(0, VCardPhoto::GetCheckSum(field));
}

TEST_F(PIMContactItemTests, testPhotoCheckSumUriCorrect)
{
  VCardField field;
  field.parse("value=uri:file://test_photo.jpeg");
  ASSERT_EQ(24910, VCardPhoto::GetCheckSum(field));
}


TEST_F(PIMContactItemTests, testPhotoCheckSumUriNotLocal)
{
  VCardField field;
  field.parse("value=uri:http://someurl.com/someimage.png");
  ASSERT_EQ(0, VCardPhoto::GetCheckSum(field));
}

TEST_F(PIMContactItemTests, testPhotoCheckSumNoUri)
{
  VCardField field;
  field.parse("value=url:file:///tmp/somenonexistingfile.png");
  ASSERT_EQ(0, VCardPhoto::GetCheckSum(field));
}

TEST_F(PIMContactItemTests, testPhotoCheckSumUriDoesNotExist)
{
  VCardField field;
  field.parse("value=uri:file:///tmp/somenonexistingfile.png");
  ASSERT_EQ(0, VCardPhoto::GetCheckSum(field));
}

TEST_F(PIMContactItemTests, testPhotoCheckSumMisformattedField)
{
  VCardField field;
  field.parse("///tmp/somenonexistingfile.png");
  ASSERT_EQ(0, VCardPhoto::GetCheckSum(field));
}

TEST_F(PIMContactItemTests, testUrlDecode)
{
  //decode space character
  ASSERT_EQ(std::string(" "), urlDecode("%20"));

  //decode " character
  ASSERT_EQ(std::string("\""), urlDecode("%22"));

  //decode % character
  ASSERT_EQ(std::string("%"), urlDecode("%25"));

  //decode - character
  ASSERT_EQ(std::string("-"), urlDecode("%2D"));

  //decode . character
  ASSERT_EQ(std::string("."), urlDecode("%2E"));

  //decode < character
  ASSERT_EQ(std::string("<"), urlDecode("%3C"));

  //decode > character
  ASSERT_EQ(std::string(">"), urlDecode("%3E"));

  //decode \ character
  ASSERT_EQ(std::string("\\"), urlDecode("%5C"));

  //decode ^ character
  ASSERT_EQ(std::string("^"), urlDecode("%5E"));

  //decode _ character
  ASSERT_EQ(std::string("_"), urlDecode("%5F"));

  //decode ` character
  ASSERT_EQ(std::string("`"), urlDecode("%60"));

  //decode { character
  ASSERT_EQ(std::string("{"), urlDecode("%7B"));

  //decode } character
  ASSERT_EQ(std::string("}"), urlDecode("%7D"));

  //decode | character
  ASSERT_EQ(std::string("|"), urlDecode("%7C"));

  //decode ~ character
  ASSERT_EQ(std::string("~"), urlDecode("%7E"));

  //decode sample photo uri
  //image location from EDS is double encoded
  ASSERT_EQ(std::string("file:///var/ias/pim/db/data/evolution/addressbook/pim-manager-htc/photos/pas_id_5396E6A700000081_photo-file0.image/jpeg"), urlDecode(urlDecode("file:///var/ias/pim/db/data/evolution/addressbook/pim-manager-htc/photos/pas_id_5396E6A700000081_photo-file0.image%252Fjpeg")));
}

TEST_F(PIMContactItemTests, testBase64Decode)
{
  const char* base64 = "MTIzNDU2Nzg5MAo=";
  unsigned char decoded[15] = {0};
  size_t decodedLen = 15;
  const char* plain = "1234567890";


  ASSERT_EQ(0, base64decode(base64, strlen(base64), decoded, &decodedLen));
  ASSERT_EQ(11, decodedLen);
  ASSERT_EQ(0, strncmp(plain, (const char*)decoded, 10));
}

TEST_F(PIMContactItemTests, testBase64DecodeWhitespaces)
{
  const char* base64 = "\tMTI\t\tzNDU2Nzg5MAo=";
  unsigned char decoded[15] = {0};
  size_t decodedLen = 15;
  const char* plain = "1234567890";


  ASSERT_EQ(0, base64decode(base64, strlen(base64), decoded, &decodedLen));
  ASSERT_EQ(11, decodedLen);
  ASSERT_EQ(0, strncmp(plain, (const char*)decoded, 10));
}

TEST_F(PIMContactItemTests, testBase64DecodeInvalidCharacters)
{
  const char* base64 = " MTI zNDU2Nzg5MAo=";
  unsigned char decoded[15] = {0};
  size_t decodedLen = 15;
  const char* plain = "1234567890";


  ASSERT_EQ(1, base64decode(base64, strlen(base64), decoded, &decodedLen));
  ASSERT_EQ(15, decodedLen);
}

TEST_F(PIMContactItemTests, testBase64DecodeBufferOverflow)
{
  const char* base64 = "MTIzNDU2Nzg5MAo=";
  unsigned char decoded[15] = {0};
  size_t decodedLen = 4;
  const char* plain = "1234567890";


  ASSERT_EQ(1, base64decode(base64, strlen(base64), decoded, &decodedLen));
  ASSERT_EQ(4, decodedLen);
}

TEST_F(PIMContactItemTests, testBase64EncodeCase1)
{
  const char* base64 = "MTIzNDU2Nzg5MA==";
  unsigned char encoded[20] = {0};
  size_t encodedLen = 20;
  const char* plain = "1234567890";

  unsigned char decoded[15] = {0};
  size_t decodedLen = 15;

  ASSERT_EQ(0, base64encode(plain, strlen(plain), encoded, &encodedLen));
  ASSERT_EQ(16, encodedLen);
  ASSERT_EQ(0, strcmp(base64, (const char*)encoded));

  ASSERT_EQ(0, base64decode((const char*)encoded, encodedLen, decoded, &decodedLen));
  ASSERT_EQ(10, decodedLen);
  ASSERT_EQ(0, strcmp(plain, (const char*)decoded));
}

TEST_F(PIMContactItemTests, testBase64EncodeCase2)
{
  const char* base64 = "MTIzNDU2Nzg5";
  unsigned char encoded[20] = {0};
  size_t encodedLen = 20;
  const char* plain = "123456789";

  unsigned char decoded[15] = {0};
  size_t decodedLen = 15;

  ASSERT_EQ(0, base64encode(plain, strlen(plain), encoded, &encodedLen));
  ASSERT_EQ(12, encodedLen);
  ASSERT_EQ(0, strcmp(base64, (const char*)encoded));

  ASSERT_EQ(0, base64decode((const char*)encoded, encodedLen, decoded, &decodedLen));
  ASSERT_EQ(9, decodedLen);
  ASSERT_EQ(0, strcmp(plain, (const char*)decoded));
}

TEST_F(PIMContactItemTests, testBase64EncodeCase3)
{
  const char* base64 = "MTIzNDU2Nzg=";
  unsigned char encoded[20] = {0};
  size_t encodedLen = 20;
  const char* plain = "12345678";

  unsigned char decoded[15] = {0};
  size_t decodedLen = 15;

  ASSERT_EQ(0, base64encode(plain, strlen(plain), encoded, &encodedLen));
  ASSERT_EQ(12, encodedLen);
  ASSERT_EQ(0, strcmp(base64, (const char*)encoded));

  ASSERT_EQ(0, base64decode((const char*)encoded, encodedLen, decoded, &decodedLen));
  ASSERT_EQ(8, decodedLen);
  ASSERT_EQ(0, strcmp(plain, (const char*)decoded));
}

TEST_F(PIMContactItemTests, testBase64EncodeBufferOverflow)
{
  unsigned char encoded[10] = {0};
  size_t encodedLen = 10;
  const char* plain = "1234567890";


  ASSERT_EQ(1, base64encode(plain, strlen(plain), encoded, &encodedLen));
  ASSERT_EQ(10, encodedLen);
}

TEST_F(PIMContactItemTests, testSetId)
{
	OpenAB::PIMItem::ID id = "id123";
	OpenAB::PIMContactItem pim;
	pim.setId(id, false);
	ASSERT_EQ(id, pim.getId());
	pim.setId(id, true);

}

TEST_F(PIMContactItemTests, testRevision)
{
	OpenAB::PIMContactItem pim;
	OpenAB::PIMItem::Revision rev = "revisionItem";
	pim.setRevision(rev);
	ASSERT_EQ(rev, pim.getRevision());

}

TEST_F(PIMContactItemTests, testSubstituteVCardUID)
{
	OpenAB::PIMItem::ID newUID = "234532";
	OpenAB::PIMContactItem pim;
	pim.parse("UID: 123456");
	pim.setId(newUID, true);
	ASSERT_EQ("UID:" + newUID, pim.getRawData());
}
/**
 * add tests for use cases:
 *  - vcards from different phones
 */

}
