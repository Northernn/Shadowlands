#pragma once

#include "Common.h"
#include "DatabaseEnvFwd.h"

class Player;
class Item;
struct MapChallengeModeEntry;

const uint8 SECOND_AFFIX_LEVEL = 7;
const uint8 THIRD_AFFIX_LEVEL  = 14;
const uint8 FOURTH_AFFIX_LEVEL = 31; // Disabled for now

enum KeystoneExpansionItem : uint32
{
    KeystoneLegion      = 138019,
    KeystoneBFA         = 158923,
    KeystoneSLDF        = 180653,
    KeystoneTimewalker  = 187786,
};

struct MythicKeystoneInfo
{
    MythicKeystoneInfo();
    MythicKeystoneInfo(Item* keystone);
    bool IsValid() const { return challengeEntry != nullptr && Level >= 2; }

    MapChallengeModeEntry const* challengeEntry = nullptr;
    uint8 Level = 0;
    uint8 Affix1 = 0;
    uint8 Affix2 = 0;
    uint8 Affix3 = 0;
    uint8 Affix4 = 0;
    uint32 KeystoneItemID;
    ObjectGuid KeystoneGUID;
};

class TC_GAME_API PlayerChallenge
{
public:
    PlayerChallenge(Player* player): _player(player) {}
    ~PlayerChallenge() {}

    Item* CreateMythicKeystone(uint32 challengeLevel = 2, uint32 challengeId = 0);
    void ModifyMythicKeystoneLevel(Item* item, int8 levelUpdate = 1, bool changeChallengeId = true);
    void ModifyMythicKeystoneLevel(MythicKeystoneInfo& keystoneInfo, int8 levelUpdate /*= 1*/, bool changeChallengeId = true);
    void ResetMythicKeystones();

    float GetOverallSeasonScore();
    float GetScoreForMap(uint32 challengeId);

    WorldPackets::MythicPlus::DungeonScoreSummary BuildDungeonScoreSummary();
    WorldPackets::MythicPlus::DungeonScoreData BuildDungeonScoreData();

private:
    Player* _player;
};
