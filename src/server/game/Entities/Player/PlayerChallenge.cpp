#include "ChallengeMode.h"
#include "ChallengeModeMgr.h"
#include "Chat.h"
#include "Containers.h"
#include "CustomObjectMgr.h"
#include "DatabaseEnv.h"
#include "PlayerChallenge.h"
#include "Player.h"
#include "Map.h"
#include "MythicPlusPacketsCommon.h"
#include "Item.h"
#include "World.h"
#include "GameTime.h"
#include "WorldStateMgr.h"

MythicKeystoneInfo::MythicKeystoneInfo()
{
    challengeEntry = nullptr;
    Level = 0;
    Affix1 = 0;
    Affix2 = 0;
    Affix3 = 0;
    Affix4 = 0;
    KeystoneItemID = 0;
    KeystoneGUID = ObjectGuid::Empty;
}

MythicKeystoneInfo::MythicKeystoneInfo(Item* keystone)
{
    challengeEntry = sMapChallengeModeStore.LookupEntry(keystone->GetModifier(ItemModifier::ITEM_MODIFIER_CHALLENGE_MAP_CHALLENGE_MODE_ID));
    Level = keystone->GetModifier(ItemModifier::ITEM_MODIFIER_CHALLENGE_KEYSTONE_LEVEL);
    Affix1 = keystone->GetModifier(ItemModifier::ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_1);
    Affix2 = keystone->GetModifier(ItemModifier::ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_2);
    Affix3 = keystone->GetModifier(ItemModifier::ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_3);
    Affix4 = keystone->GetModifier(ItemModifier::ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_4);
    KeystoneItemID = keystone->GetEntry();
    KeystoneGUID = keystone->GetGUID();
}

Item* PlayerChallenge::CreateMythicKeystone(uint32 challengeLevel /*= 2*/, uint32 challengeId /*= 0*/)
{
    uint32 itemId = KeystoneExpansionItem::KeystoneSLDF;
    uint8 count = 1;
    uint32 noSpaceForCount = 0;
    ItemPosCountVec dest;
    InventoryResult msg = _player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);
    if (msg != EQUIP_ERR_OK)
        count -= noSpaceForCount;

    if (count == 0 || dest.empty())
    {
        /// @todo Send to mailbox if no space
        ChatHandler(_player->GetSession()).PSendSysMessage("You don't have any space in your bags.");
        return nullptr;
    }

    if (Item* item = _player->StoreNewItem(dest, itemId, true, GenerateItemRandomBonusListId(itemId)))
    {
        item->SetModifier(ITEM_MODIFIER_CHALLENGE_MAP_CHALLENGE_MODE_ID, challengeId ? challengeId : sChallengeModeMgr->GetRandomChallengeId());
        item->SetModifier(ITEM_MODIFIER_CHALLENGE_KEYSTONE_LEVEL, challengeLevel);

        item->SetModifier(ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_1, sWorldStateMgr->GetValue(WS_CHALLENGE_AFFIXE1_RESET_TIME, nullptr));

        if (challengeLevel >= SECOND_AFFIX_LEVEL)
            item->SetModifier(ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_2, sWorldStateMgr->GetValue(WS_CHALLENGE_AFFIXE2_RESET_TIME, nullptr));

        if (challengeLevel >= THIRD_AFFIX_LEVEL)
            item->SetModifier(ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_3, sWorldStateMgr->GetValue(WS_CHALLENGE_AFFIXE3_RESET_TIME, nullptr));

        if (challengeLevel >= FOURTH_AFFIX_LEVEL)
            item->SetModifier(ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_4, sWorldStateMgr->GetValue(WS_CHALLENGE_AFFIXE4_RESET_TIME, nullptr));

        item->SetState(ITEM_CHANGED, _player);
        _player->SendNewItem(item, count, true, false);
        return item;
    }

    return nullptr;
}

void PlayerChallenge::ModifyMythicKeystoneLevel(Item* item, int8 levelUpdate /*= 1*/, bool changeChallengeId /*= true*/)
{
    MythicKeystoneInfo keystoneInfo = MythicKeystoneInfo(item);
    if (!keystoneInfo.IsValid())
        return;

    ModifyMythicKeystoneLevel(keystoneInfo, levelUpdate, changeChallengeId);

    item->SetModifier(ITEM_MODIFIER_CHALLENGE_MAP_CHALLENGE_MODE_ID, keystoneInfo.challengeEntry->ID);
    item->SetModifier(ITEM_MODIFIER_CHALLENGE_KEYSTONE_LEVEL, keystoneInfo.Level);

    item->SetModifier(ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_1, keystoneInfo.Affix1);
    item->SetModifier(ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_2, keystoneInfo.Affix2);
    item->SetModifier(ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_3, keystoneInfo.Affix3);
    item->SetModifier(ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_4, keystoneInfo.Affix4);

    item->SetState(ITEM_CHANGED, _player);
    item->SendUpdateToPlayer(_player);
}

void PlayerChallenge::ModifyMythicKeystoneLevel(MythicKeystoneInfo& keystoneInfo, int8 levelUpdate /*= 1*/, bool changeChallengeId /*= true*/)
{
    if (changeChallengeId)
    {
        uint32 newChallengeId = sChallengeModeMgr->GetRandomChallengeId();
        keystoneInfo.challengeEntry = sMapChallengeModeStore.LookupEntry(newChallengeId);
    }

    uint32 newChallengeLevel = keystoneInfo.Level + levelUpdate;

    if (newChallengeLevel < 2)
        newChallengeLevel = 2;

    keystoneInfo.Level = newChallengeLevel;

    keystoneInfo.Affix1 = sWorldStateMgr->GetValue(ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_1, nullptr);

    if (newChallengeLevel >= SECOND_AFFIX_LEVEL)
        keystoneInfo.Affix2 = sWorldStateMgr->GetValue(ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_2, nullptr);

    if (newChallengeLevel >= THIRD_AFFIX_LEVEL)
        keystoneInfo.Affix3 = sWorldStateMgr->GetValue(ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_3, nullptr);

    if (newChallengeLevel >= FOURTH_AFFIX_LEVEL)
        keystoneInfo.Affix4 = sWorldStateMgr->GetValue(ITEM_MODIFIER_CHALLENGE_KEYSTONE_AFFIX_ID_4, nullptr);
}

void PlayerChallenge::ResetMythicKeystones()
{
    uint32 count = 1;

    _player->DestroyItemCount(uint32(KeystoneExpansionItem::KeystoneLegion), count, true);
    _player->DestroyItemCount(uint32(KeystoneExpansionItem::KeystoneBFA), count, true);
    _player->DestroyItemCount(uint32(KeystoneExpansionItem::KeystoneSLDF), count, true);
    _player->DestroyItemCount(uint32(KeystoneExpansionItem::KeystoneTimewalker), count, true);
}

float PlayerChallenge::GetOverallSeasonScore()
{
    float overallScore = 0.f;

    auto memberChallengeDatas = sChallengeModeMgr->GetBestForMemberByMajorAffix(_player->GetGUID());
    for (auto affixChallengeData : memberChallengeDatas)
        overallScore += sChallengeModeMgr->GetScoreForMap(affixChallengeData.second);

    return overallScore;
}

float PlayerChallenge::GetScoreForMap(uint32 challengeId)
{
    auto bestChallengesForMap = sChallengeModeMgr->GetBestForMemberByMajorAffixForMap(_player->GetGUID(), challengeId);
    return sChallengeModeMgr->GetScoreForMap(bestChallengesForMap);
}

WorldPackets::MythicPlus::DungeonScoreSummary PlayerChallenge::BuildDungeonScoreSummary()
{
    WorldPackets::MythicPlus::DungeonScoreSummary dungeonScore;

    auto memberChallengeDatas = sChallengeModeMgr->GetBestForMemberByMajorAffix(_player->GetGUID());

    dungeonScore.OverallScoreCurrentSeason = .0f;
    dungeonScore.LadderScoreCurrentSeason = .0f;

    dungeonScore.Runs.reserve(memberChallengeDatas.size());

    for (auto affixChallengeData : memberChallengeDatas)
    {
        WorldPackets::MythicPlus::DungeonScoreMapSummary dungeonScoreMapSummary;

        for (auto challengeData : affixChallengeData.second)
        {
            if (dungeonScoreMapSummary.MapScore < challengeData.second->Score)
            {
                dungeonScoreMapSummary.ChallengeModeID = challengeData.second->ChallengeID;
                dungeonScoreMapSummary.MapScore = challengeData.second->Score;
                dungeonScoreMapSummary.BestRunLevel = challengeData.second->ChallengeLevel;
                dungeonScoreMapSummary.BestRunDurationMS = challengeData.second->RecordTime;
                dungeonScoreMapSummary.FinishedSuccess = challengeData.second->TimerLevel != 0;
            }
        }

        dungeonScoreMapSummary.MapScore = sChallengeModeMgr->GetScoreForMap(affixChallengeData.second);
        dungeonScore.OverallScoreCurrentSeason += dungeonScoreMapSummary.MapScore;
        dungeonScore.LadderScoreCurrentSeason += dungeonScoreMapSummary.MapScore;
        dungeonScore.Runs.emplace_back(dungeonScoreMapSummary);
    }

    return dungeonScore;
}

WorldPackets::MythicPlus::DungeonScoreData PlayerChallenge::BuildDungeonScoreData()
{
    WorldPackets::MythicPlus::DungeonScoreData dungeonScore;
    dungeonScore.TotalRuns = 24;

    WorldPackets::MythicPlus::DungeonScoreSeasonData seasonData;
    seasonData.Season = 10;
    seasonData.SeasonScore = 0.f;
    seasonData.LadderScore = 0.f;

    std::vector<WorldPackets::MythicPlus::DungeonScoreMapData> SeasonMaps;
    auto memberChallengeDatas = sChallengeModeMgr->GetBestForMemberByMajorAffix(_player->GetGUID());
    for (auto affixChallengeData : memberChallengeDatas)
    {
        WorldPackets::MythicPlus::DungeonScoreMapData dungeonScoreMapData;
        dungeonScoreMapData.MapChallengeModeID = affixChallengeData.first;
        dungeonScoreMapData.OverAllScore = sChallengeModeMgr->GetScoreForMap(affixChallengeData.second);

        for (auto const& challengeData : affixChallengeData.second)
        {
            WorldPackets::MythicPlus::DungeonScoreBestRunForAffix dungeonRun;
            dungeonRun.KeystoneAffixID          = challengeData.first;
            dungeonRun.Run.MapChallengeModeID   = challengeData.second->ChallengeID;
            dungeonRun.Run.Completed            = challengeData.second->CompleteDate != 0;
            dungeonRun.Run.Level                = challengeData.second->ChallengeLevel;
            dungeonRun.Run.DurationMs           = challengeData.second->RecordTime;
            dungeonRun.Run.Season               = 0;
            dungeonRun.Run.RunScore             = challengeData.second->Score;
            dungeonRun.Run.StartDate            = challengeData.second->StartDate;
            dungeonRun.Run.CompletionDate       = challengeData.second->CompleteDate;
            dungeonRun.Run.KeystoneAffixIDs     = challengeData.second->Affixes;

            for (auto const& member : challengeData.second->members)
            {
                WorldPackets::MythicPlus::MythicPlusMember mythicPlusMember;

                mythicPlusMember.GuildClubMemberID = 0;
                mythicPlusMember.GUID = member.playerGuid;
                mythicPlusMember.GuildGUID = member.guildGuid;
                mythicPlusMember.NativeRealmAddress = GetVirtualRealmAddress();
                mythicPlusMember.VirtualRealmAddress = GetVirtualRealmAddress();
                mythicPlusMember.ChrSpecializationID = member.specId;
                mythicPlusMember.RaceID = member.Race;
                mythicPlusMember.ItemLevel = member.ItemLevel;
                mythicPlusMember.CovenantID = 0;
                mythicPlusMember.SoulbindID = 0;

                dungeonRun.Run.Members.push_back(mythicPlusMember);
            }

            dungeonScoreMapData.BestRuns.push_back(dungeonRun);
        }

        seasonData.SeasonMaps.push_back(dungeonScoreMapData);
        seasonData.SeasonScore += dungeonScoreMapData.OverAllScore;

        seasonData.LadderMaps.push_back(dungeonScoreMapData);
        seasonData.LadderScore += dungeonScoreMapData.OverAllScore;
    }

    dungeonScore.Seasons.push_back(seasonData);

    return dungeonScore;
}
