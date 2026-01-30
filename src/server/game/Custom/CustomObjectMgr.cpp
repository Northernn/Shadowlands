#include "CustomObjectMgr.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "PlayerChallenge.h"
#include "Unit.h"
#include "SpellInfo.h"

CustomObjectMgr::~CustomObjectMgr()
{
    for (auto& itr : _groupChallengeLevelInfo)
        delete itr.second;
    for (auto& itr : _soloChallengeLevelInfo)
        delete itr.second;
}

void CustomObjectMgr::LoadFromDB()
{
    LoadCustomChallengeInfo();
}

void CustomObjectMgr::LoadCustomChallengeInfo()
{
    for (auto& itr : _groupChallengeLevelInfo)
        delete itr.second;
    for (auto& itr : _soloChallengeLevelInfo)
        delete itr.second;

    _groupChallengeLevelInfo.clear();
    _soloChallengeLevelInfo.clear();
    _customChallengeDungeonsByKeystone.clear();

    if (auto result = WorldDatabase.Query("SELECT ChallengeLevel, HPScalingPerPlayer, DMGScalingPerPlayer FROM z_group_challenge_level_info"))
    {
        do
        {
            auto fields = result->Fetch();

            ChallengeLevelInfo* levelInfo = new ChallengeLevelInfo();
        
            levelInfo->BaseHPScaling       = fields[1].GetFloat();
            levelInfo->BaseDmgScaling      = fields[2].GetFloat();
            levelInfo->HPScalingPerPlayer  = fields[3].GetFloat();
            levelInfo->DMGScalingPerPlayer = fields[4].GetFloat();

            _groupChallengeLevelInfo[fields[0].GetUInt32()] = levelInfo;
        } while (result->NextRow());
    }

    if (auto result = WorldDatabase.Query("SELECT ChallengeLevel, HPScalingPerPlayer, DMGScalingPerPlayer FROM z_solo_challenge_level_info"))
    {
        do
        {
            auto fields = result->Fetch();

            ChallengeLevelInfo* levelInfo = new ChallengeLevelInfo();
        
            levelInfo->BaseHPScaling       = fields[1].GetFloat();
            levelInfo->BaseDmgScaling      = fields[2].GetFloat();
            levelInfo->HPScalingPerPlayer  = fields[3].GetFloat();
            levelInfo->DMGScalingPerPlayer = fields[4].GetFloat();

            _groupChallengeLevelInfo[fields[0].GetUInt32()] = levelInfo;
        } while (result->NextRow());
    }

    if (auto result = WorldDatabase.Query("SELECT Keystone, ChallengeID FROM z_custom_challenge_dungeons"))
    {
        do
        {
            auto fields = result->Fetch();

            if (sMapChallengeModeStore.LookupEntry(fields[1].GetUInt32()))
                _customChallengeDungeonsByKeystone[fields[0].GetUInt32()].push_back(fields[1].GetUInt32());

        } while (result->NextRow());
    }
}

void CustomObjectMgr::ModifySpellDmg(Unit* unit, SpellInfo const* spellInfo, uint32& damage)
{
    if (!unit->IsCreature())
        return;

    if (!spellInfo)
        return;

    auto entry = unit->GetEntry();

    auto itr = _customSpellDmgs.find(entry);
    if (itr == _customSpellDmgs.end())
        return;

    auto spellId = spellInfo->Id;
    auto itr2 = itr->second.find(spellId);
    if (itr2 == itr->second.end())
        return;

    auto const& customDamage = itr2->second;

    if (customDamage.Max > 0 && customDamage.Max >= customDamage.Min)
        damage = urand(customDamage.Min, customDamage.Max);

    if (customDamage.PctModifier > 0.0f)
        damage *= customDamage.PctModifier;
}

void CustomObjectMgr::GenerateCustomDungeonForKeystone(MythicKeystoneInfo* keystoneInfo)
{
    /*auto it = _customChallengeDungeonsByKeystone.find(keystoneInfo->KeystoneItemID);
    if (it == _customChallengeDungeonsByKeystone.end() || it->second.empty())
        return;

    if (it->second.size() == 1)
    {
        keystoneInfo->ID = it->second.back();
        return;
    }

    uint32 oldID = keystoneInfo->ID;
    while (oldID == keystoneInfo->ID)
    {
        keystoneInfo->ID = Trinity::Containers::SelectRandomContainerElement(it->second);
    }*/
}

void CustomObjectMgr::SetChallengeLevelInfoIfNeed(MythicKeystoneInfo keystoneInfo, ChallengeLevelInfo* levelInfo)
{
    /*switch (keystoneInfo.Type)
    {
        case KeystoneType::Timewalking:
        case KeystoneType::Group:
        {
            auto it = _groupChallengeLevelInfo.find(keystoneInfo.Level);
            if (it != _groupChallengeLevelInfo.end())
                levelInfo = it->second;
            break;
        }
        case KeystoneType::Solo:
        {
            auto it = _soloChallengeLevelInfo.find(keystoneInfo.Level);
            if (it != _soloChallengeLevelInfo.end())
                levelInfo = it->second;
            break;
        }
    }*/
}
