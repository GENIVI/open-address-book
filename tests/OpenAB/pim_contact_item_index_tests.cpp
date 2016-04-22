/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */
#include <gtest/gtest.h>
#include <string>
#include "PIMItem/Contact/PIMContactItem.hpp"
#include "PIMItem/Contact/PIMContactItemIndex.hpp"

namespace OpenAB
{

class PIMContactItemIndexTests: public ::testing::Test
{
public:
    PIMContactItemIndexTests() : ::testing::Test()
    {
    }

    ~PIMContactItemIndexTests()
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

//Quite comprehensive vcard for testing purposes
static const char* vcard0 = \
"BEGIN:VCARD\n\r"
"VERSION:3.0\n\r"
"N:Surname;Name;Middle;Perfix;Suffix\n\r"
"FN:Prefix Name Middle Surname Suffix\n\r"
"ORG:Bubba Shrimp Co.\n\r"
"TITLE:Shrimp Man\n\r"
"PHOTO;VALUE=URI;TYPE=GIF:http://www.example.com/dir_photos/my_photo.gif\n\r"
"TEL;TYPE=WORK,VOICE;X-EVOLUTION-E164=891234,\"+49\":(111) 555-100\n\r"
"ADR;TYPE=WORK;TYPE=HOME,OFFICE:;;100 Waters Edge;Baytown;LA;0;United States of America\n\r"
"EMAIL;TYPE=PREF;TYPE=INTERNET:name.surname@example.com\n\r"
"GEO:39.95;-75.1667\n\r"
"BDAY:19700310\n\r"
"REV:123\n\r"
"UID:id1234\n\r"
"PRODID:OPENAB\n\r"
"X-EVOLUTION-LABEL:label\n\r"
"END:VCARD\n\r";

//only N and FN are different from vcard0 -> contacts shouldn't match and should be treated as not  the same
static const char* vcard1 = \
"BEGIN:VCARD\n\r"
"VERSION:3.0\n\r"
"N:Surname2;Name2;Middle2;Perfix2;Suffix2\n\r"
"FN:Prefix2 Name2 Middle2 Surname2 Suffix2\n\r"
"ORG:Bubba Shrimp Co.\n\r"
"TITLE:Shrimp Man\n\r"
"PHOTO;VALUE=URI;TYPE=GIF:http://www.example.com/dir_photos/my_photo.gif\n\r"
"TEL;TYPE=WORK,VOICE;X-EVOLUTION-E164=891234,\"+49\":(111) 555-100\n\r"
"ADR;TYPE=WORK;TYPE=HOME,OFFICE:;;100 Waters Edge;Baytown;LA;0;United States of America\n\r"
"EMAIL;TYPE=PREF;TYPE=INTERNET:name.surname@example.com\n\r"
"GEO:39.95;-75.1667\n\r"
"BDAY:19700310\n\r"
"REV:123\n\r"
"UID:id1234\n\r"
"PRODID:OPENAB\n\r"
"X-EVOLUTION-LABEL:label\n\r"
"END:VCARD\n\r";

//the same key field "fn" changed other fields
static const char* vcardMatchingWithVCard0 = \
"BEGIN:VCARD\n\r"
"VERSION:3.0\n\r"
"N:Surname;Name;Middle;Perfix;Suffix\n\r"
"FN:Prefix Name Middle Surname Suffix\n\r"
"ORG:Bubba Shrimp Co.\n\r"
"TITLE:Shrimp Man\n\r"
"PHOTO;VALUE=URI;TYPE=GIF:http://www.example.com/dir_photos/my_photo.jpeg\n\r"
"TEL;TYPE=WORK,VOICE;X-EVOLUTION-E164=891234,\"+49\":(333) 888-222\n\r"
"ADR;TYPE=WORK;TYPE=HOME,OFFICE:;;987 Waters Edge;Baytown;LA;0;United States of America\n\r"
"EMAIL;TYPE=PREF;TYPE=INTERNET:test@example.com\n\r"
"GEO:25.95;-64.1667\n\r"
"BDAY:19750310\n\r"
"REV:654\n\r"
"UID:id1234\n\r"
"PRODID:OPENAB\n\r"
"X-EVOLUTION-LABEL:label\n\r"
"END:VCARD\n\r";

//Exactly the same content as vcard0, but different fields order
static const char* vcard0DifferentFieldsOrder = \
"BEGIN:VCARD\n\r"
"ORG:Bubba Shrimp Co.\n\r"
"VERSION:3.0\n\r"
"N:Surname;Name;Middle;Perfix;Suffix\n\r"
"FN:Prefix Name Middle Surname Suffix\n\r"
"TITLE:Shrimp Man\n\r"
"EMAIL;TYPE=PREF;TYPE=INTERNET:name.surname@example.com\n\r"
"TEL;TYPE=WORK,VOICE;X-EVOLUTION-E164=891234,\"+49\":(111) 555-100\n\r"
"ADR;TYPE=WORK;TYPE=HOME,OFFICE:;;100 Waters Edge;Baytown;LA;0;United States of America\n\r"
"GEO:39.95;-75.1667\n\r"
"X-EVOLUTION-LABEL:label\n\r"
"BDAY:19700310\n\r"
"PRODID:OPENAB\n\r"
"REV:123\n\r"
"UID:id1234\n\r"
"PHOTO;VALUE=URI;TYPE=GIF:http://www.example.com/dir_photos/my_photo.gif\n\r"
"END:VCARD\n\r";

//Exactly the same content as vcard0, but different params order
static const char* vcard0DifferentParamsOrder = \
"BEGIN:VCARD\n\r"
"VERSION:3.0\n\r"
"N:Surname;Name;Middle;Perfix;Suffix\n\r"
"FN:Prefix Name Middle Surname Suffix\n\r"
"ORG:Bubba Shrimp Co.\n\r"
"TITLE:Shrimp Man\n\r"
"PHOTO;VALUE=URI;TYPE=GIF:http://www.example.com/dir_photos/my_photo.gif\n\r"
"TEL;TYPE=VOICE,WORK;X-EVOLUTION-E164=891234,\"+49\":(111) 555-100\n\r"
"ADR;TYPE=OFFICE,HOME;TYPE=WORK:;;100 Waters Edge;Baytown;LA;0;United States of America\n\r"
"EMAIL;TYPE=INTERNET;TYPE=PREF:name.surname@example.com\n\r"
"GEO:39.95;-75.1667\n\r"
"BDAY:19700310\n\r"
"REV:123\n\r"
"UID:id1234\n\r"
"PRODID:OPENAB\n\r"
"X-EVOLUTION-LABEL:label\n\r"
"END:VCARD\n\r";

//the same as vcard0 except email and bday fields
static const char* vcard2 = \
"BEGIN:VCARD\n\r"
"VERSION:3.0\n\r"
"N:Surname;Name;Middle;Perfix;Suffix\n\r"
"FN:Prefix Name Middle Surname Suffix\n\r"
"ORG:Bubba Shrimp Co.\n\r"
"TITLE:Shrimp Man\n\r"
"PHOTO;VALUE=URI;TYPE=GIF:http://www.example.com/dir_photos/my_photo.gif\n\r"
"TEL;TYPE=WORK,VOICE;X-EVOLUTION-E164=891234,\"+49\":(111) 555-100\n\r"
"ADR;TYPE=WORK;TYPE=HOME,OFFICE:;;100 Waters Edge;Baytown;LA;0;United States of America\n\r"
"EMAIL;TYPE=PREF;TYPE=INTERNET:example@example.com\n\r"
"GEO:39.95;-75.1667\n\r"
"BDAY:19750515\n\r"
"REV:123\n\r"
"UID:id1234\n\r"
"PRODID:OPENAB\n\r"
"X-EVOLUTION-LABEL:label\n\r"
"END:VCARD\n\r";

TEST_F(PIMContactItemIndexTests, testConstructor)
{
  PIMContactItem testItem;
  ASSERT_TRUE(testItem.parse(vcard0));
  OpenAB::SmartPtr<PIMItemIndex> index = testItem.getIndex();
  ASSERT_TRUE(index.getPointer());
  ASSERT_EQ(OpenAB::eContact, index->getType());
}

/**
 * Tests for managing checks
 */
TEST_F(PIMContactItemIndexTests, testNoChecks)
{
  PIMContactItemIndex::clearAllChecks();
  std::vector<PIMItemIndex::PIMItemCheck> checks = PIMContactItemIndex::getAllChecks();
  ASSERT_TRUE(checks.empty());

  PIMContactItem testItem;
  ASSERT_TRUE(testItem.parse(vcard0));
  OpenAB::SmartPtr<PIMItemIndex> index = testItem.getIndex();
  ASSERT_TRUE(index.getPointer());
  //index should be empty as there are no checks defined
  ASSERT_EQ("", index->toString());
  ASSERT_EQ("", index->toStringFull());
}

TEST_F(PIMContactItemIndexTests, testAddingChecks)
{
  std::vector<PIMItemIndex::PIMItemCheck> checks;
  //make sure that there are no default checks defined
  PIMContactItemIndex::clearAllChecks();
  checks = PIMContactItemIndex::getAllChecks();
  ASSERT_TRUE(checks.empty());

  //add two new checks
  ASSERT_TRUE(PIMContactItemIndex::addCheck("fn", PIMItemIndex::PIMItemCheck::eKey));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("tel", PIMItemIndex::PIMItemCheck::eConflict));

  checks = PIMContactItemIndex::getAllChecks();
  ASSERT_FALSE(checks.empty());
  ASSERT_EQ(2, checks.size());
  //order of checks should be the same as order of adding them
  ASSERT_EQ("fn", checks.at(0).fieldName);
  ASSERT_EQ(PIMItemIndex::PIMItemCheck::eKey, checks.at(0).fieldRole);
  //newly added checks should be by default enabled
  ASSERT_TRUE(checks.at(0).enabled);

  ASSERT_EQ("tel", checks.at(1).fieldName);
  ASSERT_EQ(PIMItemIndex::PIMItemCheck::eConflict, checks.at(1).fieldRole);
  ASSERT_TRUE(checks.at(1).enabled);

  PIMContactItem testItem;
  ASSERT_TRUE(testItem.parse(vcard0));
  OpenAB::SmartPtr<PIMItemIndex> index = testItem.getIndex();
  ASSERT_TRUE(index.getPointer());
  //now index shouldn't be empty as there are checks defined
  ASSERT_NE("", index->toString());
  ASSERT_NE("", index->toStringFull());
}

TEST_F(PIMContactItemIndexTests, testAddDuplicatedCheck)
{
  std::vector<PIMItemIndex::PIMItemCheck> checks;
  //make sure that there are no default checks defined
  PIMContactItemIndex::clearAllChecks();
  checks = PIMContactItemIndex::getAllChecks();
  ASSERT_TRUE(checks.empty());

  //add two new checks
  ASSERT_TRUE(PIMContactItemIndex::addCheck("fn", PIMItemIndex::PIMItemCheck::eKey));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("tel", PIMItemIndex::PIMItemCheck::eConflict));

  checks = PIMContactItemIndex::getAllChecks();
  ASSERT_EQ(2, checks.size());

  //adding the same check again should fail
  ASSERT_FALSE(PIMContactItemIndex::addCheck("fn", PIMItemIndex::PIMItemCheck::eKey));
  checks = PIMContactItemIndex::getAllChecks();
  //number of defined checks shouldn't change
  ASSERT_EQ(2, checks.size());

  //adding check for the same field but with different role also should fail
  ASSERT_FALSE(PIMContactItemIndex::addCheck("fn", PIMItemIndex::PIMItemCheck::eConflict));
  checks = PIMContactItemIndex::getAllChecks();
  //number of defined checks shouldn't change
  ASSERT_EQ(2, checks.size());
}

TEST_F(PIMContactItemIndexTests, testRemoveCheck)
{
  std::vector<PIMItemIndex::PIMItemCheck> checks;
  //make sure that there are no default checks defined
  PIMContactItemIndex::clearAllChecks();
  checks = PIMContactItemIndex::getAllChecks();
  ASSERT_TRUE(checks.empty());

  //add two new checks
  ASSERT_TRUE(PIMContactItemIndex::addCheck("fn", PIMItemIndex::PIMItemCheck::eKey));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("tel", PIMItemIndex::PIMItemCheck::eConflict));

  checks = PIMContactItemIndex::getAllChecks();
  ASSERT_EQ(2, checks.size());

  //remove first check
  ASSERT_TRUE(PIMContactItemIndex::removeCheck("fn"));
  checks = PIMContactItemIndex::getAllChecks();
  //number of defined checks should change
  ASSERT_EQ(1, checks.size());

  //only check for tel field should left
  ASSERT_EQ("tel", checks.at(0).fieldName);
  ASSERT_EQ(PIMItemIndex::PIMItemCheck::eConflict, checks.at(0).fieldRole);
  ASSERT_TRUE(checks.at(0).enabled);

  //remove second check
  ASSERT_TRUE(PIMContactItemIndex::removeCheck("tel"));
  checks = PIMContactItemIndex::getAllChecks();
  //now there should be no defined checks
  ASSERT_EQ(0, checks.size());
}


TEST_F(PIMContactItemIndexTests, testRemoveNonExistingCheck)
{
  std::vector<PIMItemIndex::PIMItemCheck> checks;
  //make sure that there are no default checks defined
  PIMContactItemIndex::clearAllChecks();
  checks = PIMContactItemIndex::getAllChecks();
  ASSERT_TRUE(checks.empty());

  //add two new checks
  ASSERT_TRUE(PIMContactItemIndex::addCheck("fn", PIMItemIndex::PIMItemCheck::eKey));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("tel", PIMItemIndex::PIMItemCheck::eConflict));

  checks = PIMContactItemIndex::getAllChecks();
  ASSERT_EQ(2, checks.size());

  //removing non existing check should fail
  ASSERT_FALSE(PIMContactItemIndex::removeCheck("email"));
  checks = PIMContactItemIndex::getAllChecks();
  //number of defined checks shouldn't change
  ASSERT_EQ(2, checks.size());
}

TEST_F(PIMContactItemIndexTests, testEnableDisableCheck)
{
  std::vector<PIMItemIndex::PIMItemCheck> checks;
  //make sure that there are no default checks defined
  PIMContactItemIndex::clearAllChecks();
  checks = PIMContactItemIndex::getAllChecks();
  ASSERT_TRUE(checks.empty());

  //add two new checks
  ASSERT_TRUE(PIMContactItemIndex::addCheck("fn", PIMItemIndex::PIMItemCheck::eKey));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("tel", PIMItemIndex::PIMItemCheck::eConflict));

  checks = PIMContactItemIndex::getAllChecks();
  ASSERT_EQ(2, checks.size());
  //order of checks should be the same as order of adding them
  //newly added checks should be by default enabled
  ASSERT_TRUE(checks.at(0).enabled);
  ASSERT_TRUE(checks.at(1).enabled);

  //disable check for "fn" field
  ASSERT_TRUE(PIMContactItemIndex::disableCheck("fn"));
  checks = PIMContactItemIndex::getAllChecks();
  //number of defined checks shouldn't change
  ASSERT_EQ(2, checks.size());
  //check for "fn" should be now disabled
  ASSERT_FALSE(checks.at(0).enabled);
  ASSERT_TRUE(checks.at(1).enabled);

  //disabling alredy disabled check shouldn't fail and should not change anything
  ASSERT_TRUE(PIMContactItemIndex::disableCheck("fn"));
  checks = PIMContactItemIndex::getAllChecks();
  ASSERT_EQ(2, checks.size());
  ASSERT_FALSE(checks.at(0).enabled);
  ASSERT_TRUE(checks.at(1).enabled);

  //disable check for "tel" field
  ASSERT_TRUE(PIMContactItemIndex::disableCheck("tel"));
  checks = PIMContactItemIndex::getAllChecks();
  //number of defined checks shouldn't change
  ASSERT_EQ(2, checks.size());
  //check for "fn" should be now disabled
  ASSERT_FALSE(checks.at(0).enabled);
  ASSERT_FALSE(checks.at(1).enabled);

  //=====================================
  //enable again check for "tel"
  ASSERT_TRUE(PIMContactItemIndex::enableCheck("tel"));
  checks = PIMContactItemIndex::getAllChecks();
  //number of defined checks shouldn't change
  ASSERT_EQ(2, checks.size());
  //check for "fn" should be now enabled
  ASSERT_FALSE(checks.at(0).enabled);
  ASSERT_TRUE(checks.at(1).enabled);

  //enable check for "tel" again - there should be no error and effect
  ASSERT_TRUE(PIMContactItemIndex::enableCheck("tel"));
  checks = PIMContactItemIndex::getAllChecks();
  //number of defined checks shouldn't change
  ASSERT_EQ(2, checks.size());
  //check for "fn" should be now enabled
  ASSERT_FALSE(checks.at(0).enabled);
  ASSERT_TRUE(checks.at(1).enabled);

  //enable check for "fn" again
  ASSERT_TRUE(PIMContactItemIndex::enableCheck("fn"));
  checks = PIMContactItemIndex::getAllChecks();
  //number of defined checks shouldn't change
  ASSERT_EQ(2, checks.size());
  //check for "fn" should be now enabled
  ASSERT_TRUE(checks.at(0).enabled);
  ASSERT_TRUE(checks.at(1).enabled);
}

TEST_F(PIMContactItemIndexTests, testEnableDisableNonExistingCheck)
{
  PIMContactItemIndex::clearAllChecks();
  ASSERT_FALSE(PIMContactItemIndex::disableCheck("tel"));
  ASSERT_FALSE(PIMContactItemIndex::enableCheck("tel"));
}

TEST_F(PIMContactItemIndexTests, testEnableAllChecks)
{
  std::vector<PIMItemIndex::PIMItemCheck> checks;
  //make sure that there are no default checks defined
  PIMContactItemIndex::clearAllChecks();
  checks = PIMContactItemIndex::getAllChecks();
  ASSERT_TRUE(checks.empty());

  //add two new checks
  ASSERT_TRUE(PIMContactItemIndex::addCheck("fn", PIMItemIndex::PIMItemCheck::eKey));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("tel", PIMItemIndex::PIMItemCheck::eConflict));

  checks = PIMContactItemIndex::getAllChecks();
  ASSERT_EQ(2, checks.size());
  //order of checks should be the same as order of adding them
  //newly added checks should be by default enabled
  ASSERT_TRUE(checks.at(0).enabled);
  ASSERT_TRUE(checks.at(1).enabled);

  ASSERT_TRUE(PIMContactItemIndex::disableCheck("fn"));
  ASSERT_TRUE(PIMContactItemIndex::disableCheck("tel"));
  checks = PIMContactItemIndex::getAllChecks();
  //number of defined checks shouldn't change
  ASSERT_EQ(2, checks.size());
  //both checks should be now disabled
  ASSERT_FALSE(checks.at(0).enabled);
  ASSERT_FALSE(checks.at(1).enabled);

  PIMContactItemIndex::enableAllChecks();
  checks = PIMContactItemIndex::getAllChecks();
  ASSERT_EQ(2, checks.size());
  //both checks should be enabled now
  ASSERT_TRUE(checks.at(0).enabled);
  ASSERT_TRUE(checks.at(1).enabled);
}

/**
 * Tests for comparing indexes
 */
TEST_F(PIMContactItemIndexTests, testCompareIndexOfTheSameVCard)
{
  //Add default set of checks
  PIMContactItemIndex::clearAllChecks();
  ASSERT_TRUE(PIMContactItemIndex::addCheck("fn", PIMItemIndex::PIMItemCheck::eKey));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("tel", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("email", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("adr", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("role", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("title", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("nickname", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("photo", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("bday", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("geo", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("org", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("note", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("url", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("categories", PIMItemIndex::PIMItemCheck::eConflict));

  PIMContactItem contact1;
  PIMContactItem contact2;
  //both contacts are exactly the same
  ASSERT_TRUE(contact1.parse(vcard0));
  ASSERT_TRUE(contact2.parse(vcard0));

  SmartPtr<PIMItemIndex> contact1Idx = contact1.getIndex();
  SmartPtr<PIMItemIndex> contact2Idx = contact2.getIndex();
  ASSERT_TRUE(contact1Idx.getPointer());
  ASSERT_TRUE(contact2Idx.getPointer());

  //compare two indexes directly using == operator, that checks if two contacts matches
  //as both contacts are the same indexes should match
  ASSERT_TRUE(contact1Idx == contact2Idx);
  //compare should be alternate
  ASSERT_TRUE(contact2Idx == contact1Idx);

  //fully compare contacts, again it should be alternate
  //as both contacts are the same indexes should be equal
  ASSERT_TRUE(contact1Idx->compare(*contact2Idx));
  ASSERT_TRUE(contact2Idx->compare(*contact1Idx));
}

TEST_F(PIMContactItemIndexTests, testCompareIndexOfTheDifferentVCards)
{
  //Add default set of checks
  PIMContactItemIndex::clearAllChecks();
  ASSERT_TRUE(PIMContactItemIndex::addCheck("fn", PIMItemIndex::PIMItemCheck::eKey));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("tel", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("email", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("adr", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("role", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("title", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("nickname", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("photo", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("bday", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("geo", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("org", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("note", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("url", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("categories", PIMItemIndex::PIMItemCheck::eConflict));

  PIMContactItem contact1;
  PIMContactItem contact2;
  //both contacts differs
  ASSERT_TRUE(contact1.parse(vcard0));
  ASSERT_TRUE(contact2.parse(vcard1));

  SmartPtr<PIMItemIndex> contact1Idx = contact1.getIndex();
  SmartPtr<PIMItemIndex> contact2Idx = contact2.getIndex();
  ASSERT_TRUE(contact1Idx.getPointer());
  ASSERT_TRUE(contact2Idx.getPointer());

  //compare two indexes directly using == operator, that checks if two contacts matches
  //as both contacts are totally different indexes shouldn't match
  ASSERT_FALSE(contact1Idx == contact2Idx);
  //compare should be alternate
  ASSERT_FALSE(contact2Idx == contact1Idx);

  //fully compare contacts, again it should be alternate
  //as both contacts are totally different indexes shouldn't be equal
  ASSERT_FALSE(contact1Idx->compare(*contact2Idx));
  ASSERT_FALSE(contact2Idx->compare(*contact1Idx));
}

TEST_F(PIMContactItemIndexTests, testCompareIndexOfTheMatchingVCards)
{
  //Add default set of checks
  PIMContactItemIndex::clearAllChecks();
  ASSERT_TRUE(PIMContactItemIndex::addCheck("fn", PIMItemIndex::PIMItemCheck::eKey));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("tel", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("email", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("adr", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("role", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("title", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("nickname", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("photo", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("bday", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("geo", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("org", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("note", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("url", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("categories", PIMItemIndex::PIMItemCheck::eConflict));

  PIMContactItem contact1;
  PIMContactItem contact2;
  //both contacts differs
  ASSERT_TRUE(contact1.parse(vcard0));
  ASSERT_TRUE(contact2.parse(vcardMatchingWithVCard0));

  SmartPtr<PIMItemIndex> contact1Idx = contact1.getIndex();
  SmartPtr<PIMItemIndex> contact2Idx = contact2.getIndex();
  ASSERT_TRUE(contact1Idx.getPointer());
  ASSERT_TRUE(contact2Idx.getPointer());

  //compare two indexes directly using == operator, that checks if two contacts matches
  //as both contacts has the same FN field they should match
  ASSERT_TRUE(contact1Idx == contact2Idx);
  //compare should be alternate
  ASSERT_TRUE(contact2Idx == contact1Idx);

  //fully compare contacts, again it should be alternate
  //as both contacts are not totally the same they shouldn't be equal
  ASSERT_FALSE(contact1Idx->compare(*contact2Idx));
  ASSERT_FALSE(contact2Idx->compare(*contact1Idx));
}

TEST_F(PIMContactItemIndexTests, testCompareIndexOfTheSameVCardsWithDifferentFiledsOrded)
{
  //Add default set of checks
  PIMContactItemIndex::clearAllChecks();
  ASSERT_TRUE(PIMContactItemIndex::addCheck("fn", PIMItemIndex::PIMItemCheck::eKey));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("tel", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("email", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("adr", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("role", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("title", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("nickname", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("photo", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("bday", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("geo", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("org", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("note", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("url", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("categories", PIMItemIndex::PIMItemCheck::eConflict));

  PIMContactItem contact1;
  PIMContactItem contact2;
  //both contacts differs
  ASSERT_TRUE(contact1.parse(vcard0));
  ASSERT_TRUE(contact2.parse(vcard0DifferentFieldsOrder));

  SmartPtr<PIMItemIndex> contact1Idx = contact1.getIndex();
  SmartPtr<PIMItemIndex> contact2Idx = contact2.getIndex();
  ASSERT_TRUE(contact1Idx.getPointer());
  ASSERT_TRUE(contact2Idx.getPointer());

  //compare two indexes directly using == operator, that checks if two contacts matches
  //as both contacts has the same FN field they should match
  ASSERT_TRUE(contact1Idx == contact2Idx);
  //compare should be alternate
  ASSERT_TRUE(contact2Idx == contact1Idx);

  //fully compare contacts, again it should be alternate
  //as both contacts are the same, but have different order of fields,
  //but that shouldn't matter, they should be equal
  ASSERT_TRUE(contact1Idx->compare(*contact2Idx));
  ASSERT_TRUE(contact2Idx->compare(*contact1Idx));
}

TEST_F(PIMContactItemIndexTests, testCompareIndexOfTheSameVCardsWithDifferentParamsOrder)
{
  //Add default set of checks
  PIMContactItemIndex::clearAllChecks();
  ASSERT_TRUE(PIMContactItemIndex::addCheck("fn", PIMItemIndex::PIMItemCheck::eKey));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("tel", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("email", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("adr", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("role", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("title", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("nickname", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("photo", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("bday", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("geo", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("org", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("note", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("url", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("categories", PIMItemIndex::PIMItemCheck::eConflict));

  PIMContactItem contact1;
  PIMContactItem contact2;
  //both contacts differs
  ASSERT_TRUE(contact1.parse(vcard0));
  ASSERT_TRUE(contact2.parse(vcard0DifferentParamsOrder));

  SmartPtr<PIMItemIndex> contact1Idx = contact1.getIndex();
  SmartPtr<PIMItemIndex> contact2Idx = contact2.getIndex();
  ASSERT_TRUE(contact1Idx.getPointer());
  ASSERT_TRUE(contact2Idx.getPointer());

  //compare two indexes directly using == operator, that checks if two contacts matches
  //as both contacts has the same FN field they should match
  ASSERT_TRUE(contact1Idx == contact2Idx);
  //compare should be alternate
  ASSERT_TRUE(contact2Idx == contact1Idx);

  //fully compare contacts, again it should be alternate
  //as both contacts are the same, but have different order of params,
  //but that shouldn't matter, they should be equal
  ASSERT_TRUE(contact1Idx->compare(*contact2Idx));
  ASSERT_TRUE(contact2Idx->compare(*contact1Idx));
}

TEST_F(PIMContactItemIndexTests, testCompareTwoDifferentVCardsIgnoringSomeFields)
{
  //Add default set of checks
  PIMContactItemIndex::clearAllChecks();
  ASSERT_TRUE(PIMContactItemIndex::addCheck("fn", PIMItemIndex::PIMItemCheck::eKey));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("tel", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("email", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("adr", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("role", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("title", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("nickname", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("photo", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("bday", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("geo", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("org", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("note", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("url", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("categories", PIMItemIndex::PIMItemCheck::eConflict));

  PIMContactItem contact1;
  PIMContactItem contact2;
  //both contacts differs
  ASSERT_TRUE(contact1.parse(vcard0));
  ASSERT_TRUE(contact2.parse(vcard2));

  SmartPtr<PIMItemIndex> contact1Idx = contact1.getIndex();
  SmartPtr<PIMItemIndex> contact2Idx = contact2.getIndex();
  ASSERT_TRUE(contact1Idx.getPointer());
  ASSERT_TRUE(contact2Idx.getPointer());

  //compare two indexes directly using == operator, that checks if two contacts matches
  //as both contacts has the same FN field they should match
  ASSERT_TRUE(contact1Idx == contact2Idx);
  //compare should be alternate
  ASSERT_TRUE(contact2Idx == contact1Idx);

  //fully compare contacts, again it should be alternate
  //email and bday fields differ
  ASSERT_FALSE(contact1Idx->compare(*contact2Idx));
  ASSERT_FALSE(contact2Idx->compare(*contact1Idx));

  //disable cheks for email and bday and compare contacts again
  ASSERT_TRUE(PIMContactItemIndex::disableCheck("email"));
  ASSERT_TRUE(PIMContactItemIndex::disableCheck("bday"));

  contact1Idx = contact1.getIndex();
  contact2Idx = contact2.getIndex();
  ASSERT_TRUE(contact1Idx.getPointer());
  ASSERT_TRUE(contact2Idx.getPointer());

  //now indexes should be the same
  ASSERT_TRUE(contact1Idx->compare(*contact2Idx));
  ASSERT_TRUE(contact2Idx->compare(*contact1Idx));


  //enable all checks again
  PIMContactItemIndex::enableAllChecks();
  contact1Idx = contact1.getIndex();
  contact2Idx = contact2.getIndex();
  ASSERT_TRUE(contact1Idx.getPointer());
  ASSERT_TRUE(contact2Idx.getPointer());

  //now indexes shouldn't be the same
  ASSERT_FALSE(contact1Idx->compare(*contact2Idx));
  ASSERT_FALSE(contact2Idx->compare(*contact1Idx));


  //disable only one check and check if they are still not the same
  ASSERT_TRUE(PIMContactItemIndex::disableCheck("email"));
  contact1Idx = contact1.getIndex();
  contact2Idx = contact2.getIndex();
  ASSERT_TRUE(contact1Idx.getPointer());
  ASSERT_TRUE(contact2Idx.getPointer());

  //now indexes shouldn't be the same
  ASSERT_FALSE(contact1Idx->compare(*contact2Idx));
  ASSERT_FALSE(contact2Idx->compare(*contact1Idx));
}

TEST_F(PIMContactItemIndexTests, testMapOfIndexes)
{
  //Add default set of checks
  PIMContactItemIndex::clearAllChecks();
  ASSERT_TRUE(PIMContactItemIndex::addCheck("fn", PIMItemIndex::PIMItemCheck::eKey));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("tel", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("email", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("adr", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("role", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("title", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("nickname", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("photo", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("bday", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("geo", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("org", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("note", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("url", PIMItemIndex::PIMItemCheck::eConflict));
  ASSERT_TRUE(PIMContactItemIndex::addCheck("categories", PIMItemIndex::PIMItemCheck::eConflict));


}
TEST_F(PIMContactItemIndexTests, testCompareTwoDifferentVCardsRemovingCheckForSomeFields)
{
  std::map<SmartPtr<PIMItemIndex>, std::vector<std::string> > vCards;
  PIMContactItem contact1;
  PIMContactItem contact2;
  PIMContactItem contact3;

  //vcard0 and vcard2 matches
  ASSERT_TRUE(contact1.parse(vcard0));
  ASSERT_TRUE(contact2.parse(vcard1));
  ASSERT_TRUE(contact3.parse(vcard2));

  SmartPtr<PIMItemIndex> contact1Idx = contact1.getIndex();
  SmartPtr<PIMItemIndex> contact2Idx = contact2.getIndex();
  SmartPtr<PIMItemIndex> contact3Idx = contact3.getIndex();
  ASSERT_TRUE(contact1Idx.getPointer());
  ASSERT_TRUE(contact2Idx.getPointer());
  ASSERT_TRUE(contact3Idx.getPointer());

  vCards[contact1Idx].push_back(vcard0);
  vCards[contact2Idx].push_back(vcard1);
  vCards[contact3Idx].push_back(vcard2);

  //vcard0 and vcard2 should be added to the same map key
  ASSERT_EQ(2, vCards[contact1Idx].size());
  ASSERT_EQ(2, vCards[contact3Idx].size());
  ASSERT_EQ(1, vCards[contact2Idx].size());

  std::vector<std::string>::iterator it;
  for (it = vCards[contact1Idx].begin(); it != vCards[contact1Idx].end(); ++it)
  {
    ASSERT_TRUE((*it) == vcard0 || (*it) == vcard2);
  }

  for (it = vCards[contact2Idx].begin(); it != vCards[contact2Idx].end(); ++it)
  {
    ASSERT_TRUE((*it) == vcard1);
  }
}

/**
 * Add use cases for PIM Index:
 *  - comparing two the same vcards
 *  - comparing two different vcards
 *  - comparing two matching vcards
 *  - comparing the same vcards (different order of fields)
 *  - comparing the same vcards (different order of params)
 *  - comparing different vcards (ignoring fields)
 *
 */
}
