/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 */
/**
 * @file sync_tests.cpp
 */
#include <gtest/gtest.h>
#include <string>
#include "plugin/sync/Sync.hpp"

class SyncPluginTests: public ::testing::Test
{
public:
	SyncPluginTests() : ::testing::Test()
	{

	}
	~SyncPluginTests()
	{

	}
protected:
	virtual void SetUp()
	{

	}
	virtual void TearDown()
	{

	}

};

TEST_F(SyncPluginTests, testAddItem)
{
	OpenAB_Sync::SyncMetadata smd;
	std::string rID = "r123";
	std::string lID = "l123";
	std::string rRev = "revisionR";
	std::string lRev = "revisionL";
	smd.addItem(rID, rRev, lID, lRev);
	ASSERT_EQ(rRev, smd.getRemoteRevision(rID));
}

TEST_F(SyncPluginTests, testRemoveItem)
{
	OpenAB_Sync::SyncMetadata smd;
	std::string rID = "r123";
	std::string lID = "l123";
	std::string rRev = "revisionR";
	std::string lRev = "revisionL";
	smd.addItem(rID, rRev, lID, lRev);
	smd.removeItem(rID, lID);
	ASSERT_EQ("", smd.getRemoteRevision(rID));
	ASSERT_EQ("", smd.getLocalRevision(lID));
}

TEST_F(SyncPluginTests, testUpdateRevisions)
{
	OpenAB_Sync::SyncMetadata smd;
	std::string rID = "r123";
	std::string lID = "l123";
	std::string rRev = "revisionR";
	std::string lRev = "revisionL";
	smd.addItem(rID, rRev, lID, lRev);
	smd.updateLocalRevision(lID, "newRevisionL");
	ASSERT_EQ("newRevisionL", smd.getLocalRevision(lID));
	smd.updateRemoteRevision(rID, "newRevisionR");
	ASSERT_EQ("newRevisionR", smd.getRemoteRevision(rID));
}

TEST_F(SyncPluginTests, testHasId)
{
	OpenAB_Sync::SyncMetadata smd;
	std::string rID = "r123";
	std::string lID = "l123";
	std::string rRev = "revisionR";
	std::string lRev = "revisionL";
	smd.addItem(rID, rRev, lID, lRev);
	ASSERT_TRUE(smd.hasLocalId(lID));
	ASSERT_TRUE(smd.hasRemoteId(rID));
	smd.removeItem(rID, lID);
	ASSERT_FALSE(smd.hasLocalId(lID));
	ASSERT_FALSE(smd.hasRemoteId(rID));
}

TEST_F(SyncPluginTests, testSyncTokens)
{
	OpenAB_Sync::SyncMetadata smd;
	std::string rToken = "remoteToken";
	std::string lToken = "localToken";
	smd.setRemoteSyncToken(rToken);
	smd.setLocalSyncToken(lToken);
	ASSERT_EQ(rToken, smd.getRemoteSyncToken());
	ASSERT_EQ(lToken, smd.getLocalSyncToken());
}

TEST_F(SyncPluginTests, testToJSON)
{
	OpenAB_Sync::SyncMetadata smd;
	std::string rID = "r123";
	std::string lID = "l123";
	std::string rRev = "revisionR";
	std::string lRev = "revisionL";
	smd.addItem(rID, rRev, lID, lRev);
	std::string rToken = "remoteToken";
	std::string lToken = "localToken";
	smd.setRemoteSyncToken(rToken);
	smd.setLocalSyncToken(lToken);
	ASSERT_EQ("{ \"LocalSyncToken\": \"localToken\", "
			  "\"RemoteSyncToken\": \"remoteToken\", \"LocalRevisions\": "
			  "{ \"l123\": \"revisionL\" }, \"RemoteRevisions\": { \"r123\": \"revisionR\" },"
			  " \"RemoteToLocalMapping\": { \"r123\": \"l123\" } }"
              , smd.toJSON());
}

TEST_F(SyncPluginTests, testFromJSON)
{
	OpenAB_Sync::SyncMetadata smd;
	std::string rID = "r123";
	std::string lID = "l123";
	std::string rRev = "revisionR";
	std::string lRev = "revisionL";
	smd.addItem(rID, rRev, lID, lRev);
	std::string rToken = "remoteToken";
	std::string lToken = "localToken";
	smd.setRemoteSyncToken(rToken);
	smd.setLocalSyncToken(lToken);
	std::string json = smd.toJSON();
	ASSERT_TRUE(smd.fromJSON(json));
	std::string jsonFail = "";
	ASSERT_FALSE(smd.fromJSON(jsonFail));
	jsonFail = "{";
	ASSERT_FALSE(smd.fromJSON(jsonFail));
}

TEST_F(SyncPluginTests, testSetStates)
{
	OpenAB_Sync::SyncMetadata smd;
	std::string rID = "r123";
	std::string lID = "l123";
	std::string rRev = "revisionR";
	std::string lRev = "revisionL";
	smd.addItem(rID, rRev, lID, lRev);
	std::string rToken = "remoteToken";
	std::string lToken = "localToken";
	smd.setRemoteSyncToken(rToken);
	smd.setLocalSyncToken(lToken);
	OpenAB_Sync::SyncMetadata::SyncMetadataState orgState = OpenAB_Sync::SyncMetadata::NotChanged;
	smd.setRemoteState(rID, orgState);
	smd.setLocalState(lID, orgState);
	std::map<std::string, std::string> result = smd.getItemsWithState(orgState, orgState);
	ASSERT_EQ(result[rID], lID);
}

TEST_F(SyncPluginTests, testResetStates)
{
	OpenAB_Sync::SyncMetadata smd;
	std::string rID = "r123";
	std::string lID = "l123";
	std::string rRev = "revisionR";
	std::string lRev = "revisionL";
	smd.addItem(rID, rRev, lID, lRev);
	std::string rToken = "remoteToken";
	std::string lToken = "localToken";
	smd.setRemoteSyncToken(rToken);
	smd.setLocalSyncToken(lToken);
	OpenAB_Sync::SyncMetadata::SyncMetadataState orgState = OpenAB_Sync::SyncMetadata::NotChanged;
	OpenAB_Sync::SyncMetadata::SyncMetadataState changeState = OpenAB_Sync::SyncMetadata::NotPresent;
	smd.setRemoteState(rID, orgState);
	smd.setLocalState(lID, orgState);
	smd.resetRemoteState(changeState);
	std::map<std::string, std::string> result = smd.getItemsWithState(changeState, orgState);
	ASSERT_EQ(result[rID], lID);
	smd.resetLocalState(changeState);
	result = smd.getItemsWithState(changeState, changeState);
	ASSERT_EQ(result[rID], lID);
}
