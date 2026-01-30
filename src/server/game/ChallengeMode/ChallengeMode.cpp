/*
 * Copyright (C) 2008-2020 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "CellImpl.h"
#include "ChallengeMode.h"
#include "ChallengeModeMgr.h"
#include "MiscPackets.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "GameTables.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "GroupMgr.h"
#include "Guild.h"
#include "InstancePackets.h"
#include "Item.h"
#include "Map.h"
#include "MiscPackets.h"
#include "MotionMaster.h"
#include "ObjectMgr.h"
#include "WorldStatePackets.h"
#include "ObjectAccessor.h"
#include "TemporarySummon.h"
#include "PlayerChallenge.h"
#include "CustomObjectMgr.h"
#include "GameTime.h"

class ChallengeModeWorker
{
public:
    ChallengeModeWorker(Challenge* challenge) : _challenge(challenge) { }

    void Visit(std::unordered_map<ObjectGuid, Creature*>& creatureMap)
    {
        for (auto const& p : creatureMap)
        {
            if (p.second->IsInWorld() && !p.second->IsPet())
            {
                if (!p.second->IsAlive())
                    p.second->Respawn();

                _challenge->CastChallengeCreatureSpell(p.second);
            }
        }
    }

    template<class T>
    void Visit(std::unordered_map<ObjectGuid, T*>&) { }

private:
    Challenge* _challenge;
};

Challenge::Challenge(InstanceMap* map, Player* player, Scenario* scenario, MythicKeystoneInfo mythicKeystone)
    : InstanceScript(map), _instanceScript(map->GetInstanceScript()),
    _challengeLevel(mythicKeystone.Level), _challengeEntry(mythicKeystone.challengeEntry), _scenario(scenario)
{
    _checkStart = true;
    _canRun = true;
    _keyOwner = player->GetGUID();
    _challengeTimer = 0;
    _affixQuakingTimer = 0;
    _deathCount = 0;
    _complete = false;
    _run = false;
    _hordeSpawn = player->IsInHorde();
    m_itemGuid = mythicKeystone.KeystoneGUID;

    ASSERT(map);
    _map = map;
    _mapID = _map->GetId();

    _affixes.fill(0);
    _affixes[0] = mythicKeystone.Affix1;
    _affixes[1] = mythicKeystone.Affix2;
    _affixes[2] = mythicKeystone.Affix3;
    _affixes[3] = mythicKeystone.Affix4;

    Group* group = player->GetGroup();
    if (group)
    {
        _groupGUID = group->GetGUID();

        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            if (Player* member = itr->GetSource())
                if (_keyOwner != member->GetGUID())
                    _challengers.emplace_back(member->GetGUID());
    }

    if (!_challengeEntry)
    {
        ChatHandler(player->GetSession()).PSendSysMessage("Error: Is not a challenge map.");
        _canRun = false;
        return;
    }

    _challengers.emplace_back(_keyOwner);

    _rewardLevel = CHALLENGE_NOT_IN_TIMER;

    _affixCount = 0;
    for (auto const& affix : _affixes)
    {
        _affixesTest.set(affix);

        if (affix != 0)
            ++_affixCount;
    }

    for (uint8 i = 0; i < CHALLENGE_TIMER_LEVEL_3; i++)
        _chestTimers[i] = _challengeEntry->CriteriaCount[i];

    sCustomObjectMgr->SetChallengeLevelInfoIfNeed(mythicKeystone, _challengeLevelInfo);
}

Challenge::~Challenge()
{
    if (InstanceScript* script = GetInstanceScript())
        script->SetChallenge(nullptr);

    _instanceScript = nullptr;
}

void Challenge::OnPlayerEnterForScript(Player* player)
{
    if (!player)
        return;

    CastChallengePlayerSpell(player);

    m_Functions.AddFunction([player, this]() -> void
    {
        if (IsRunning() && !IsComplete())
        {
            SendChallengeModeStart(player);
            SendStartElapsedTimer(player);
        }
    }, m_Functions.CalculateTime(500));  
}

void Challenge::OnPlayerLeaveForScript(Player* player)
{
    if (!player)
        return;

    player->SetDungeonDifficultyID(DIFFICULTY_MYTHIC);
    player->SendDungeonDifficulty();
    player->RemoveAura(SPELL_CHALLENGER_BURDEN);
}

void Challenge::OnPlayerDiesForScript(Player* /*player*/)
{
    ++_deathCount;
    ModChallengeTimer();

    SendStartElapsedTimer();

    WorldPackets::Misc::UpdateDeathCount packet;
    packet.DeathCount = _deathCount;
    BroadcastPacket(packet.Write());
}

void Challenge::OnCreatureCreateForScript(Creature* creature)
{
    if (!creature || creature->IsTrigger() || creature->IsControlledByPlayer())
        return;

    Unit* owner = creature->GetAnyOwner();
    if (owner && owner->IsPlayer())
        return;

    CastChallengeCreatureSpell(creature);
}

void Challenge::OnCreatureUpdateDifficulty(Creature* creature)
{
    if (!creature || creature->IsTrigger() || creature->IsControlledByPlayer())
        return;

    Unit* owner = creature->GetAnyOwner();
    if (owner && owner->IsPlayer())
        return;

    CastChallengeCreatureSpell(creature);

    if (!creature->IsDungeonBoss())
    {
        if (HasAffix(Infested) && !creature->IsSummon() && roll_chance_f(30.0f)) // Only for real creature summon copy
        {
            std::list<Creature*> creatures;
            Trinity::UnitAuraCheck check(true, SPELL_CHALLENGER_SIMBIOTE_OF_GHUUN);
            Trinity::CreatureListSearcher<Trinity::UnitAuraCheck> searcher(creature, creatures, check);
            Cell::VisitAllObjects(creature, searcher, 20.0f);

            if(creatures.empty())
                creature->CastSpell(creature, SPELL_CHALLENGER_SIMBIOTE_OF_GHUUN, true);
        }            
    }       
}

void Challenge::OnCreatureRemoveForScript(Creature* /*creature*/) { }

void Challenge::EnterCombatForScript(Creature* creature, Unit* /*enemy*/)
{
    if (!creature || creature->IsTrigger() || creature->IsControlledByPlayer())
        return;

    Unit* owner = creature->GetAnyOwner();
    if (owner && owner->IsPlayer())
        return;

    if (!creature->HasAura(SPELL_CHALLENGER_MIGHT))
        CastChallengeCreatureSpell(creature);
}

void Challenge::CreatureDiesForScript(Creature* creature, Unit* killer)
{
    if (!creature || !killer || !killer->IsPlayer() || creature->IsTrigger() || creature->isAnySummons() || creature->IsControlledByPlayer() || creature->IsDungeonBoss())
        return;

    if (creature->IsOnVehicle(creature))
        return;

    Unit* owner = creature->GetAnyOwner();
    if (owner && owner->IsPlayer())
        return;

    if (creature->GetCreatureType() != CREATURE_TYPE_CRITTER)
    {
        if (HasAffix(Affixes::Bolstering))
            creature->CastSpell(creature, SPELL_CHALLENGER_BOLSTERING, true);

        if (HasAffix(Affixes::Sanguine))
            creature->CastSpell(creature, SPELL_CHALLENGER_SANGUINE, true);

        if (HasAffix(Affixes::Bursting))
            creature->CastSpell(creature, SPELL_CHALLENGER_BURSTING, true);
    }

    if (HasAffix(Affixes::Infested) && creature->HasAura(SPELL_CHALLENGER_SIMBIOTE_OF_GHUUN))
    {
        std::list<Creature*> creatures;
       // creature->GetCreatureListInGrid(creatures, 500.0f);
        if (!creatures.empty())
        {
            creatures.remove_if([creature](Creature* object) -> bool
            {
                if (object == nullptr)
                    return true;

                if (object == creature)
                    return true;

                if (!creature->IsValidAssistTarget(object))
                    return true;

                if (object->IsTrigger() || object->IsControlledByPlayer() || !object->IsHostileToPlayers() || object->GetCreatureType() == CREATURE_TYPE_CRITTER)
                    return true;

                if (object->IsDungeonBoss())
                    return true;

                if (object->isAnySummons())
                    return true;

                if (object->HasAura(SPELL_CHALLENGER_SIMBIOTE_OF_GHUUN))
                    return true;                

                return false;
            });

            if (!creatures.empty())
            {
                creatures.sort(Trinity::ObjectDistanceOrderPred(creature));
                Creature* creatureSelected = creatures.front();
                
                for (uint8 i = 0; i < 2; i++)
                {
                    if (auto spawn = creature->SummonCreature(NPC_SPAWN_OF_GHUUN, creature->GetRandomNearPosition(2.0f), TEMPSUMMON_MANUAL_DESPAWN))
                    {
                        spawn->SetReactState(REACT_PASSIVE);
                        m_Functions.AddFunction([spawn, creatureSelected]() -> void
                        {
                            if (!spawn || !creatureSelected)
                                return;

                            spawn->FollowTarget(creatureSelected);
                            spawn->GetAI()->SetGUID(creatureSelected->GetGUID());

                        }, m_Functions.CalculateTime(3.5f * float(IN_MILLISECONDS)));
                    }
                }
            }
        }
    }    
}

void Challenge::OnGameObjectCreateForScript(GameObject* go)
{
}

void Challenge::OnGameObjectRemoveForScript(GameObject* go)
{
}

void Challenge::OnUnitCharmed(Unit* unit, Unit* /*charmer*/)
{
    if (!unit || !unit->ToCreature())
        return;

    unit->RemoveAura(SPELL_CHALLENGER_BOLSTERING);
    unit->RemoveAura(SPELL_CHALLENGER_RAGING);
    unit->RemoveAura(SPELL_CHALLENGER_MIGHT);
}

void Challenge::OnUnitRemoveCharmed(Unit* unit, Unit* /*charmer*/)
{
    if (!unit || !unit->ToCreature())
        return;

    CastChallengeCreatureSpell(unit->ToCreature());
}

void Challenge::Update(uint32 diff)
{
    m_Functions.Update(diff);

    if (IsComplete()) // Stop update if complete
        return;

    if (!IsRunning())
        return;

    _challengeTimer += diff;

    if (_affixQuakingTimer)
    {
        if (_affixQuakingTimer <= diff)
        {
            _map->DoOnPlayers([&](Player* plr)
            {
                if (plr->IsInWorld() && !plr->HasAura(SPELL_CHALLENGER_QUAKE) && !plr->HasUnitFlag2(UNIT_FLAG2_NO_ACTIONS) && !plr->IsOnVehicle(plr))
                    plr->AddAura(SPELL_CHALLENGER_QUAKE, plr);
            });
            _affixQuakingTimer = 20 * IN_MILLISECONDS;
        }
        else
            _affixQuakingTimer -= diff;
    }
}

bool Challenge::CanStart()
{
    if (IsRunning())
        return true;

    return _map->GetPlayerCount() == _challengers.size(); // wait when group complete
}

void Challenge::Start()
{
    if (!CanStart())
        return;

    if (!CanRun())
        return;

    Player* keyOwner = ObjectAccessor::FindPlayer(_keyOwner);
    if (!keyOwner)
        return;

    Item* keystone = keyOwner->GetItemByGuid(m_itemGuid);
    if (!keystone)
        return;

    keyOwner->GetPlayerChallenge()->ModifyMythicKeystoneLevel(keystone, -1, false);

    Position startPosition;
    if(!sChallengeModeMgr->GetStartPosition(keyOwner->GetInstanceScript(), startPosition))
        return;

    WorldPackets::Misc::ChangePlayerDifficultyResult startChangeDifficultyPacket;
    startChangeDifficultyPacket.Result = uint8(ChangeDifficultyResult::Start);
    startChangeDifficultyPacket.InstanceMapID = GetInstanceScript()->instance->ToInstanceMap()->GetInstanceId();
    BroadcastPacket(startChangeDifficultyPacket.Write());

    _map->DoOnPlayers([&](Player* player)
    {
         player->TeleportTo(_map->GetId(), startPosition);
         _scenario->SendScenarioState(player);
    });

    SummonGobject(true);

    if (InstanceScript* script = GetInstanceScript())
    {
        if (GameObject* challengeOrb = _map->GetGameObject(script->GetChallengeModeOrb()))
        {
            challengeOrb->SetGoState(GO_STATE_ACTIVE);
            challengeOrb->SetFlag(GO_FLAG_NODESPAWN);
        }
    }

    // Apply aura on all creatures
    ChallengeModeWorker worker(this);
    TypeContainerVisitor<ChallengeModeWorker, MapStoredObjectTypesContainer> visitor(worker);
    visitor.Visit(instance->GetObjectsStore());

    m_Functions.AddFunction([this]() -> void
    {
        SendChallengeInstanceEncounterStart();

        WorldPackets::Misc::ChangePlayerDifficultyResult completeChangeDifficultyPacket;
        completeChangeDifficultyPacket.Result = uint8(ChangeDifficultyResult::Complete);
        completeChangeDifficultyPacket.MapID = GetInstanceScript()->instance->GetId();
        completeChangeDifficultyPacket.DifficultyRecID = Difficulty::DIFFICULTY_MYTHIC_KEYSTONE;
        BroadcastPacket(completeChangeDifficultyPacket.Write());

        BroadcastPacket(WorldPackets::Misc::Reset(_mapID).Write());
        SendStartTimer();
        SendChallengeModeStart();
    }, m_Functions.CalculateTime(2 * IN_MILLISECONDS));

    m_Functions.AddFunction([this]() -> void
    {
        if (!this)
            return;

        _challengeTimer = 0;

        SendStartElapsedTimer();

        if (InstanceScript* script = GetInstanceScript())
        {
            if (GameObject* challengeDoor = _map->GetGameObject(script->GetChallengeModeDoor()))
                challengeDoor->Delete();

            if (GameObject* chest = _map->GetGameObject(script->GetChallengeModeChest()))
                chest->Delete();

            script->OnChallengeStart();
        }
      
        _map->DoOnPlayers([&](Player* player)
        {
             CastChallengePlayerSpell(player);
        });

        _run = true;

    }, m_Functions.CalculateTime(12 * IN_MILLISECONDS));

    _checkStart = false;

    if (HasAffix(Affixes::Quaking))
        _affixQuakingTimer = 20 * IN_MILLISECONDS;
}

void Challenge::Complete()
{
    if (IsComplete())
        return;

    _complete = true;

    Player* keyOwner = ObjectAccessor::FindPlayer(_keyOwner);
    Item* keystone = nullptr;
    if (keyOwner)
        keystone = keyOwner->GetItemByGuid(m_itemGuid);

    HitTimer();

    WorldPackets::Misc::StopElapsedTimer stopElapsedTimer;
    stopElapsedTimer.TimerID = WorldPackets::Misc::StartTimer::TimerType::ChallengeMode;
    BroadcastPacket(stopElapsedTimer.Write());

    if (Group* group = sGroupMgr->GetGroupByGUID(_groupGUID))
    {
        group->SetDungeonDifficultyID(DIFFICULTY_MYTHIC);

        //if(keyOwner)
        //    if(group->IsGuildGroup() && group->CanRecieveGuildChallengeCredit())
        //        if (Guild* guild = keyOwner->GetGuild())
        //            guild->CompleteGuildChallenge(GuildChallengeType::ChallengeDungeonChallenge, keyOwner);
    }

    if (_rewardLevel > 0)
    {
        if (keystone)
        {
            keyOwner->GetPlayerChallenge()->ModifyMythicKeystoneLevel(keystone, 1 + _rewardLevel);
        }
        else
        {
            MythicKeystoneInfo keystoneInfo = MythicKeystoneInfo();
            keyOwner->GetPlayerChallenge()->ModifyMythicKeystoneLevel(keystoneInfo, _challengeLevel + 1 + _rewardLevel);

            auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHALLENGE_KEY);
            stmt->setUInt32(0, keystoneInfo.challengeEntry->ID);
            stmt->setUInt32(1, keystoneInfo.Level);
            stmt->setUInt32(2, keystoneInfo.Affix1);
            stmt->setUInt32(3, keystoneInfo.Affix2);
            stmt->setUInt32(4, keystoneInfo.Affix2);
            stmt->setUInt32(5, keystoneInfo.Affix4);
            stmt->setUInt32(5, keystoneInfo.Affix4);
            stmt->setUInt64(6, m_itemGuid.GetCounter());
            CharacterDatabase.Execute(stmt);
        }
    }

    float score = CalculateScore();

    ChallengeData* challengeData = new ChallengeData();
    ObjectGuid::LowType guid = sObjectMgr->GenerateScenarioId();
    challengeData->ID = ObjectGuid::Create<HighGuid::Scenario>(_mapID, _challengeEntry->ID, guid).GetCounter();
    challengeData->MythicSeason     = sChallengeModeMgr->GetCurrentMythicPlusSeason()->ID;
    challengeData->DisplaySeason    = sChallengeModeMgr->GetCurrentDisplaySeason()->ID;
    challengeData->MajorAffix       = _affixes[0];
    challengeData->ChallengeID      = _challengeEntry->ID;
    challengeData->ChallengeLevel   = _challengeLevel;
    challengeData->GuildID          = 0; // Will be set lower
    challengeData->StartDate        = time(nullptr) - GetChallengeTimerToNow();
    challengeData->CompleteDate     = time(nullptr);
    challengeData->RecordTime       = _challengeTimer;
    challengeData->TimerLevel       = _rewardLevel;
    challengeData->Affixes          = _affixes;
    challengeData->Score            = score;

    if (InstanceScript* script = GetInstanceScript())
        challengeData->ChestID = script->GetChallengeModeChest().GetEntry();
    else
        challengeData->ChestID = 0;

    std::map<ObjectGuid::LowType /*guild*/, uint32> guildCounter;
    _map->DoOnPlayers([&](Player* player)
    {
        ChallengeMember member;
        member.playerGuid = player->GetGUID();
        member.specId = player->GetPrimarySpecialization();

        if (Guild* guild = player->GetGuild())
            member.guildGuid = guild->GetGUID();

        member.ItemLevel = player->GetAverageItemLevel();
        member.Race = player->GetRace();

        if (player->GetGuildId())
            guildCounter[player->GetGuildId()]++;

        SendChallengeBonusLootPrompt(player);

        challengeData->members.insert(member);

        std::array<bool, 2> areNewRecords = sChallengeModeMgr->AddChallengeForMember(member.playerGuid, challengeData);

        player->UpdateMythicPlusData();

        sChallengeModeMgr->GenerateAchievementByMythicLevel(player, _challengeLevel);
        player->UpdateCriteria(CriteriaType::CompleteChallengeMode, _mapID, _rewardLevel);
        player->UpdateCriteria(CriteriaType::CompleteAnyChallengeMode, _challengeLevel);

        player->RemoveAura(SPELL_CHALLENGER_BURDEN);
        player->CombatStop();

        SendChallengeModeComplete(player, score, areNewRecords);
    });

    for (auto const& v : guildCounter)
        if (v.second >= 3)
            challengeData->GuildID = v.first;

    sChallengeModeMgr->SetChallengeMapData(challengeData->ID, challengeData);
    sChallengeModeMgr->CheckBestMapId(challengeData);
    sChallengeModeMgr->CheckBestGuildMapId(challengeData);
    sChallengeModeMgr->SaveChallengeToDB(challengeData);

    if (GetInstanceScript())
        GetInstanceScript()->OnChallengeComplete();
}

void Challenge::HitTimer()
{
    if (GetChallengeTimer() < _chestTimers[2])
        _rewardLevel = CHALLENGE_TIMER_LEVEL_3; // 3 chests + 3 levels
    else if (GetChallengeTimer() < _chestTimers[1])
        _rewardLevel = CHALLENGE_TIMER_LEVEL_2; // 2 chests + 2 levels
    else if (GetChallengeTimer() < _chestTimers[0])
        _rewardLevel = CHALLENGE_TIMER_LEVEL_1; // 1 chest + 1 level
    else
        _rewardLevel = CHALLENGE_NOT_IN_TIMER;

    if (!_map)
        return;

    for (auto const& guid : _challengers)
        _countItems[guid] = 0;

    switch (_rewardLevel)
    {
        case CHALLENGE_TIMER_LEVEL_3: // 3 chests + 3 levels
        case CHALLENGE_TIMER_LEVEL_2: // 2 chests + 2 levels
        case CHALLENGE_TIMER_LEVEL_1: // 1 chest + 1 level
        case CHALLENGE_NOT_IN_TIMER:  // 0 chest
        {
            if (InstanceScript* script = GetInstanceScript())
            {
                SummonGobject(false);

                float _chance = sChallengeModeMgr->GetChanceItem(_rewardLevel, _challengeLevel);
                auto countItems = int32(_chance / 100.0f);
                _chance -= countItems * 100.0f;

                if (roll_chance_f(_chance))
                    countItems++;

                while (countItems > 0)
                {
                    GuidVector temp;
                    temp.reserve(_challengers.size());
                    for (auto c : _challengers)
                        temp.emplace_back(c);
                    //auto _tempList = _challengers;
                    Trinity::Containers::RandomResize(temp, countItems);

                    for (auto const& guid : temp)
                    {
                        countItems--;
                        _countItems[guid] += 1;
                    }
                }
            }
        }
        default:
            break;
    }
}

std::array<int32, 4> Challenge::GetAffixes() const
{
    return _affixes;
}

bool Challenge::HasAffix(Affixes affix)
{
    return _affixesTest.test(size_t(affix));
}

uint32 Challenge::GetChallengeTimerToNow() const
{
    return (getMSTime() - _challengeTimer) / IN_MILLISECONDS;
}

void Challenge::BroadcastPacket(WorldPacket const* data) const
{
    _map->DoOnPlayers([&](Player* player)
    {
        player->SendDirectMessage(data);
    });
}

uint64 Challenge::GetChallengeTimer()
{
    if (!_challengeTimer)
        return 0;

    return _challengeTimer / IN_MILLISECONDS;
}

void Challenge::ModChallengeTimer()
{
    _challengeTimer += 5 * IN_MILLISECONDS;
}

void Challenge::ResetGo()
{
    SummonGobject(true);

    if (InstanceScript* script = GetInstanceScript())
    {
        if (GameObject* challengeOrb = _map->GetGameObject(script->GetChallengeModeOrb()))
        {
            challengeOrb->SetGoState(GO_STATE_READY);
            challengeOrb->RemoveFlag(GO_FLAG_NODESPAWN);
        }
    }
}

void Challenge::SendStartTimer(Player* player)
{
    WorldPackets::Misc::StartTimer packet;
    packet.Type      = WorldPackets::Misc::StartTimer::TimerType::ChallengeMode;
    packet.TimeLeft  = 9s;
    packet.TotalTime = 10s;
    if (player)
        player->SendDirectMessage(packet.Write());
    else
        BroadcastPacket(packet.Write());
}

void Challenge::SendStartElapsedTimer(Player* player)
{
    WorldPackets::Misc::StartElapsedTimer packet;

    packet.Timer.TimerID = WorldPackets::Misc::StartTimer::TimerType::ChallengeMode;
    packet.Timer.CurrentDuration = GetChallengeTimer();
 
    if (player)
        player->SendDirectMessage(packet.Write());
    else
        BroadcastPacket(packet.Write());
}

void Challenge::SendChallengeModeStart(Player* player /*= nullptr*/)
{
    if (!_challengeEntry)
        return;

    WorldPackets::Misc::Start packet;
    packet.MapID                 = _mapID;
    packet.ChallengeId           = _challengeEntry->ID;
    packet.ChallengeLevel        = _challengeLevel;
    packet.Affixes               = _affixes;
    packet.IsKeyCharged          = true; //Always true since 735

    if (player)
        player->SendDirectMessage(packet.Write());
    else
        BroadcastPacket(packet.Write());
}

void Challenge::SendChallengeModeComplete(Player* player, float score, std::array<bool, 2> areNewRecords)
{
    WorldPackets::Misc::Complete complete;
    complete.Run.MapChallengeModeID = _challengeEntry->ID;
    complete.Run.Completed = true;
    complete.Run.Level = _challengeLevel;
    complete.Run.DurationMs = _challengeTimer;
    complete.Run.RunScore = score;
    complete.Run.CompletionDate = time(nullptr);
    complete.Run.KeystoneAffixIDs = _affixes;

    complete.NewOverallScore = player->GetPlayerChallenge()->GetOverallSeasonScore();
    complete.IsMapRecord = areNewRecords[0];
    complete.IsAffixRecorded = areNewRecords[1];
    complete.Members.resize(_challengers.size());

    for (uint32 i = 0; i < _challengers.size(); ++i)
    {
        auto const& guid = _challengers[i];
        if (auto player = ObjectAccessor::FindConnectedPlayer(guid))
        {
            complete.Members[i].Member = guid;
            complete.Members[i].Name = player->GetName();
        }
    }

    player->SendDirectMessage(complete.Write());
}

void Challenge::SendChallengeInstanceEncounterStart()
{
    InstanceScript* script = GetInstanceScript();
    if (!script)
        return;

    uint32 resInterval = script->GetCombatResurrectionChargeInterval();
    script->InitializeCombatResurrections(1, resInterval);
    script->SendEncounterStart(1, 5, resInterval, resInterval, false);
}

void Challenge::SendChallengeModeNewPlayerRecord(Player* player)
{
    WorldPackets::Misc::NewPlayerRecord newRecord;

    newRecord.ChallengeId    = _challengeEntry->ID;
    newRecord.Duration       = _challengeTimer;
    newRecord.ChallengeLevel = _challengeLevel;

    player->SendDirectMessage(newRecord.Write());
}

void Challenge::SummonGobject(bool door)
{
    float x, y, z, o, rot0, rot1, rot2, rot3;
    if (sChallengeModeMgr->GetChallengeDoorOrChestPosition(_mapID, x, y, z, o, rot0, rot1, rot2, rot3, door, IsHordeChallenge()))
    {
        if (uint32 entry = sChallengeModeMgr->GetDoorOrChestByMap(_mapID, door, IsHordeChallenge()))
            if (InstanceScript* script = GetInstanceScript())
                script->instance->SummonGameObject(entry, Position(x, y, z, o), QuaternionData(rot0, rot1, rot2, rot3), 0);
    }
    else
    {
        if (auto script = GetInstanceScript())
            script->SummonChallengeGameObject(door);
        // let instance script handle it.
    }
}

void Challenge::SetInstanceScript(InstanceScript* instanceScript)
{
    _instanceScript = instanceScript;
}

InstanceScript* Challenge::GetInstanceScript() const
{
    return _instanceScript;
}

uint8 Challenge::GetItemCount(ObjectGuid guid) const
{
    auto itr = _countItems.find(guid);
    if (itr == _countItems.end())
        return 0;

    return itr->second;
}

uint8 Challenge::GetLevelBonus() const
{
    switch (_rewardLevel)
    {
        case CHALLENGE_TIMER_LEVEL_3: // 3 chests + 3 levels
            return 2;
        case CHALLENGE_TIMER_LEVEL_2: // 2 chests + 2 levels
            return 1;
        case CHALLENGE_TIMER_LEVEL_1: // 1 chest + 1 level
        case CHALLENGE_NOT_IN_TIMER:  // 0 chest
        default:
            return 0;
    }
}

int32 Challenge::GetDamageMultiplier(uint8 challengeLevel)
{
    if (_challengeLevelInfo)
    {
        uint32 challengers = _challengers.size();
        return _challengeLevelInfo->BaseDmgScaling + (_challengeLevelInfo->DMGScalingPerPlayer * (float)challengers);
    }

    if (GtChallengeModeDamageEntry const* challengeDamage = sChallengeModeDamageTable.GetRow(challengeLevel))
        return uint32(100.f * (challengeDamage->Scalar - 1.f));

    return 1;
}

int32 Challenge::GetHealthMultiplier(uint8 challengeLevel)
{
    if (_challengeLevelInfo)
    {
        uint32 challengers = _challengers.size();
        return _challengeLevelInfo->BaseHPScaling + (_challengeLevelInfo->HPScalingPerPlayer * (float)challengers);
    }

    if (GtChallengeModeHealthEntry const* challengeHealth = sChallengeModeHealthTable.GetRow(challengeLevel))
        return uint32(100.f * (challengeHealth->Scalar - 1.f));

    return 1;
}

void Challenge::CastChallengeCreatureSpell(Creature* creature)
{
    CastSpellExtraArgs values(TRIGGERED_FULL_MASK);
    values.AddSpellMod(SPELLVALUE_BASE_POINT0, GetHealthMultiplier(_challengeLevel));
    values.AddSpellMod(SPELLVALUE_BASE_POINT1, GetDamageMultiplier(_challengeLevel));

    // Affixes
    values.AddSpellMod(SPELLVALUE_BASE_POINT2,  0); // 6 Raging
    values.AddSpellMod(SPELLVALUE_BASE_POINT3,  0); // 7 Bolstering
    values.AddSpellMod(SPELLVALUE_BASE_POINT4,  0); // 9 Tyrannical
    values.AddSpellMod(SPELLVALUE_BASE_POINT5,  0); //
    values.AddSpellMod(SPELLVALUE_BASE_POINT6,  0); //
    values.AddSpellMod(SPELLVALUE_BASE_POINT7,  0); // 3 Volcanic
    values.AddSpellMod(SPELLVALUE_BASE_POINT8,  0); // 4 Necrotic
    values.AddSpellMod(SPELLVALUE_BASE_POINT9,  0); // 10 Fortified
    values.AddSpellMod(SPELLVALUE_BASE_POINT10, 0); // 8 Sanguine
    values.AddSpellMod(SPELLVALUE_BASE_POINT11, 0); //
    values.AddSpellMod(SPELLVALUE_BASE_POINT12, 0); // 13 Explosive
    values.AddSpellMod(SPELLVALUE_BASE_POINT13, 0); // 11 Bursting
    values.AddSpellMod(SPELLVALUE_BASE_POINT14, 0); // Reaping
    values.AddSpellMod(SPELLVALUE_BASE_POINT15, 0); // 
    values.AddSpellMod(SPELLVALUE_BASE_POINT16, 0); // Spiteful
    values.AddSpellMod(SPELLVALUE_BASE_POINT17, 0); // Storming

    creature->CastSpell(creature, SPELL_CHALLENGER_MIGHT, values);
}

void Challenge::CastChallengePlayerSpell(Player* player)
{
    CastSpellExtraArgs values(TRIGGERED_FULL_MASK);

    // Affixes
    values.AddSpellMod(SPELLVALUE_BASE_POINT1,  0); // Grievous
    values.AddSpellMod(SPELLVALUE_BASE_POINT2,  0); // Skittish
    values.AddSpellMod(SPELLVALUE_BASE_POINT3,  0); // Quaking
    values.AddSpellMod(SPELLVALUE_BASE_POINT4,  0); // Thundering
    values.AddSpellMod(SPELLVALUE_BASE_POINT5,  0); //

    player->CastSpell(player, SPELL_CHALLENGER_BURDEN, values);
}

void Challenge::SendChallengeBonusLootPrompt(Player* player)
{
    if (!player)
        return;

    //BonusRollEntry const* entry = sDB2Manager.GetBonusRollIDByMap(_mapID);
    //if (!entry)
    //    return;
    //
    //CustomSpellValues mod;
    //mod.AddSpellMod(SPELLVALUE_BASE_POINT0, entry->ID);
    //mod.AddSpellMod(SPELLVALUE_BASE_POINT1, player->GetCurrency(entry->CurrencyTypeID));
    //mod.AddSpellMod(SPELLVALUE_BASE_POINT2, GetChallengeLevel());
    //player->CastCustomSpell(SPELL_CHALLENGE_BONUS_LOOT, mod, player, TRIGGERED_FULL_MASK);
}

// https://www.reddit.com/r/wow/comments/13vqsbw/an_accurate_formula_for_m_score_calculation_in
float Challenge::CalculateScore()
{
    if (!IsComplete())
        return .0f;

    // Time-based bonus
    float maxBonusTime = float(_chestTimers[0]) * 0.4f;
    float beatTimerBy = float(_chestTimers[0]) - float(GetChallengeTimer());
    float bonus = beatTimerBy / maxBonusTime;

    // If too much overtime, no score
    if (bonus <= -1.0f)
        return .0f;

    float adjustedKeyLevel = _challengeLevel + std::clamp(bonus, -1.f, 1.f);

    float nLevelsAbove10 = std::max(int32(_challengeLevel) - 10, 0);

    if (!_rewardLevel)
        adjustedKeyLevel -= 1.f;

    return 20 + (adjustedKeyLevel * 5) + (nLevelsAbove10 * 2) + (_affixCount * 10);
}
