/*
 * Copyright 2021 DekkCore
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

#include "MotionMaster.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "tanaan_intro.h"

/// Passive Scene Object
class playerScript_taste_of_iron : public PlayerScript
{
public:
    playerScript_taste_of_iron() : PlayerScript("playerScript_taste_of_iron") { }

    void OnSceneCancel(Player* player, uint32 sceneInstanceId) override
    {
        if (!player->GetSceneMgr().HasScene(sceneInstanceId, TanaanSceneObjects::SceneShootingGallery))
            return;

        ResetAurasOnCancel(player);
    }

    void OnQuestAbandon(Player* player, const Quest* quest) override
    {
        if (player && quest && quest->GetQuestId() == TanaanQuests::QuestATasteOfIron)
        {
            player->GetSceneMgr().CancelSceneByPackageId(TanaanSceneObjects::SceneShootingGallery);
            ResetAurasOnCancel(player);
        }
    }

    void OnSceneTriggerEvent(Player* player, uint32 sceneInstanceId, std::string triggerEvent) override
    {
        if (!player->GetSceneMgr().HasScene(sceneInstanceId, TanaanSceneObjects::SceneShootingGallery))
            return;

        if (triggerEvent == "Credit")
            player->KilledMonsterCredit(TanaanKillCredits::CreditIronHordeSlain);
        else if (triggerEvent == "CancelGame")
        {
            player->GetSceneMgr().CancelScene(sceneInstanceId);
            ResetAurasOnCancel(player);
        }
    }

    void ResetAurasOnCancel(Player* player)
    {
        player->AddAura(TanaanPhases::PhaseFinalSideCanons, player);
        player->AddAura(TanaanPhases::PhaseFinalCanonDeco, player);
        player->RemoveAurasDueToSpell(TanaanSpells::SpellTasteOfIronGameAura);
    }
};

/// Passive Scene Object
class playerScript_the_home_stretch : public PlayerScript
{
public:
    playerScript_the_home_stretch() : PlayerScript("playerScript_the_home_stretch")
    {
        m_timer = 1000;
    }

    #define MAX_IRON_BASTION_PROGRESS 6
    uint32 m_timer;

    std::vector<float> m_minX =
    {
        4032.81f,
        3948.38f,
        3868.31f,
        3787.45f,
        3752.40f,
        3662.74f
    };

    std::vector<float> m_Auras =
    {
        TanaanSpells::SpellIronBastionProgressA,
        TanaanSpells::SpellIronBastionProgressB,
        TanaanSpells::SpellIronBastionProgressC,
        TanaanSpells::SpellIronBastionProgressD,
        TanaanSpells::SpellIronBastionProgressE,
        TanaanSpells::SpellIronBastionProgressF
    };

    void OnQuestReward(Player* player, Quest const* quest) override
    {
        if (player && quest && (quest->GetQuestId() == TanaanQuests::QuestTheHomeStretchAlly || quest->GetQuestId() == TanaanQuests::QuestTheHomeStretchHorde))
        {
            player->GetSceneMgr().CancelSceneByPackageId(TanaanSceneObjects::SceneFinaleIronBastion);

            if (player->GetTeamId() == TEAM_ALLIANCE)
                player->GetSceneMgr().PlaySceneByPackageId(TanaanSceneObjects::SceneAllianceBoat, SceneFlag::None);
                 player->TeleportTo(MAP_DRAENOR, 2308.9621f, 454.9409f, 6.0f, player->GetOrientation());
            if (auto achievementEntry = sAchievementStore.LookupEntry(TanaanAchievements::AchievementWelcomeToDreanorA))
                player->CompletedAchievement(achievementEntry);
            else
                player->GetSceneMgr().PlaySceneByPackageId(TanaanSceneObjects::SceneHordeBoat, SceneFlag::None);
                player->TeleportTo(MAP_DRAENOR, 5538.213379f, 5015.2690f, 13.0f, player->GetOrientation());
                if (auto achievementEntry = sAchievementStore.LookupEntry(TanaanAchievements::AchievementWelcomeToDreanorH))
                player->CompletedAchievement(achievementEntry);
        }
    }

    void OnQuestAbandon(Player* player, const Quest* quest) override
    {
        if (player && quest && (quest->GetQuestId() == TanaanQuests::QuestTheHomeStretchAlly || quest->GetQuestId() == TanaanQuests::QuestTheHomeStretchHorde))
            player->GetSceneMgr().CancelSceneByPackageId(TanaanSceneObjects::SceneFinaleIronBastion);
    }

    void OnSceneStart(Player* player, uint32 scenePackageID, uint32 /*sceneInstanceId*/) override
    {
        if (scenePackageID != TanaanSceneObjects::SceneFinaleIronBastion)
            return;

        Clean(player);
    }

    void OnSceneComplete(Player* player, uint32 sceneInstanceId) override
    {
        if (!player->GetSceneMgr().HasScene(sceneInstanceId, TanaanSceneObjects::SceneFinaleIronBastion))
            return;

        player->KilledMonsterCredit(TanaanKillCredits::CreditBoatsReached);

        if (player->GetPositionX() > 3558.0f)
            player->TeleportTo(MAP_TANAAN_JUNGLE_INTRO, 3558.0f, -2119.70f, 17.74f, 3.396844f);

        Clean(player);
    }

    void OnSceneCancel(Player* player, uint32 sceneInstanceId) override
    {
        if (!player->GetSceneMgr().HasScene(sceneInstanceId, TanaanSceneObjects::SceneFinaleIronBastion))
            return;

        Clean(player);
    }

    void OnSceneTriggerEvent(Player* player, uint32 sceneInstanceId, std::string triggerEvent) override
    {
        if (!player->GetSceneMgr().HasScene(sceneInstanceId, TanaanSceneObjects::SceneFinaleIronBastion))
            return;

        if (triggerEvent == "Damage")
            player->EnvironmentalDamage(DAMAGE_FIRE, urand(11230, 14320));  // TODO: verify that this is proper damage for SL
    }

    void OnUpdate(Player* player, uint32 diff) override
    {
        if (m_timer > diff)
        {
            m_timer -= diff;
            return;
        }

        m_timer = 1000;

       // if (!player->GetSceneMgr().HasSceneWithPackageId(TanaanSceneObjects::SceneFinaleIronBastion))
          //  return;

        for (uint8 i = 0; i < MAX_IRON_BASTION_PROGRESS; ++i)
            if (player->GetPositionX() < m_minX[i] && !player->HasAura(m_Auras[i]))
                player->AddAura(m_Auras[i], player);
    }

    void Clean(Player* player)
    {
        for (uint8 i = 0; i < MAX_IRON_BASTION_PROGRESS; ++i)
            player->RemoveAurasDueToSpell(m_Auras[i]);
    }
};

/// Passive Scene Object
class playerScript_tanaan_scene_boat : public PlayerScript
{
public:
    playerScript_tanaan_scene_boat() : PlayerScript("playerScript_tanaan_scene_boat") { }

    void OnSceneTriggerEvent(Player* player, uint32 sceneInstanceId, std::string triggerEvent) override
    {
        if (triggerEvent == "Teleport")
        {
            if (player->GetSceneMgr().HasScene(sceneInstanceId, TanaanSceneObjects::SceneAllianceBoat))
            {
                player->TeleportTo(MAP_DRAENOR, 2308.9621f, 454.9409f, 6.0f, player->GetOrientation());
                if (auto achievementEntry = sAchievementStore.LookupEntry(TanaanAchievements::AchievementWelcomeToDreanorA))
                    player->CompletedAchievement(achievementEntry);
            }
            else if (player->GetSceneMgr().HasScene(sceneInstanceId, TanaanSceneObjects::SceneHordeBoat))
            {
                player->TeleportTo(MAP_DRAENOR, 5538.213379f, 5015.2690f, 13.0f, player->GetOrientation());
                if (auto achievementEntry = sAchievementStore.LookupEntry(TanaanAchievements::AchievementWelcomeToDreanorH))
                player->CompletedAchievement(achievementEntry);
            }
        }
    }

    void OnSceneCancel(Player* player, uint32 sceneInstanceId) override
    {
        if (player->GetSceneMgr().HasScene(sceneInstanceId, TanaanSceneObjects::SceneAllianceBoat))
        {
            player->TeleportTo(MAP_DRAENOR, 2308.9621f, 454.9409f, 6.0f, player->GetOrientation());
            if (auto achievementEntry = sAchievementStore.LookupEntry(TanaanAchievements::AchievementWelcomeToDreanorA))
            player->CompletedAchievement(achievementEntry);
        }
        else if (player->GetSceneMgr().HasScene(sceneInstanceId, TanaanSceneObjects::SceneHordeBoat))
        {
            player->TeleportTo(MAP_DRAENOR, 5538.213379f, 5015.2690f, 13.0f, player->GetOrientation());
            if (auto achievementEntry = sAchievementStore.LookupEntry(TanaanAchievements::AchievementWelcomeToDreanorH))
            player->CompletedAchievement(achievementEntry);
        }
    }
};

/// 86039 - Gogluk
class npc_tanaan_gogluk : public CreatureScript
{
public:
    npc_tanaan_gogluk() : CreatureScript("npc_tanaan_gogluk")
    {
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_tanaan_goglukAI(creature);
    }

    struct npc_tanaan_goglukAI : public ScriptedAI
    {
        npc_tanaan_goglukAI(Creature* creature) : ScriptedAI(creature) { }

        void SetPassengersFightingOrDespawn(uint8 p_Seat, bool p_Despawn, Vehicle* p_Vehicle, Unit* target = nullptr)
        {
            if (Unit* l_Passenger = p_Vehicle->GetPassenger(p_Seat))
            {
                if (Creature* creature = l_Passenger->ToCreature())
                {
                    if (creature->GetAI())
                    {
                        if (!p_Despawn)
                        {
                            creature->SetInCombatWith(target);
                            creature->AI()->DoAction(1);
                        }
                        else
                            creature->DespawnOrUnsummon();
                    }
                }
            }
        }

        void JustEngagedWith(Unit* attacker) override
        {
            if (me->IsVehicle())
            {
                if (Vehicle* l_Vehicle = me->GetVehicleKit())
                {
                    SetPassengersFightingOrDespawn(0, false, l_Vehicle, attacker);
                    SetPassengersFightingOrDespawn(1, false, l_Vehicle, attacker);
                }
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (me->IsVehicle())
            {
                if (Vehicle* l_Vehicle = me->GetVehicleKit())
                {
                    SetPassengersFightingOrDespawn(0, true, l_Vehicle);
                    SetPassengersFightingOrDespawn(1, true, l_Vehicle);
                }
            }
        }

        void EnterEvadeMode(EvadeReason /*why*/) override
        {
            if (me->IsVehicle())
            {
                if (Vehicle* l_Vehicle = me->GetVehicle())
                {
                    if (Unit* l_Passenger = l_Vehicle->GetPassenger(0))
                        l_Passenger->CombatStop(true);

                    if (Unit* l_Passenger = l_Vehicle->GetPassenger(1))
                        l_Passenger->CombatStop(true);
                }
            }
        }
    };
};

/// 86690/86691 - Gogluk Adds
class npc_tanaan_gogluk_adds : public CreatureScript
{
public:
    npc_tanaan_gogluk_adds() : CreatureScript("npc_tanaan_gogluk_adds")
    {
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_tanaan_gogluk_addsAI(creature);
    }

    struct npc_tanaan_gogluk_addsAI : public ScriptedAI
    {
        npc_tanaan_gogluk_addsAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_Events;

        enum eEvents
        {
            EventCannonBarrage = 1,
            EventMachineGun = 2
        };

        void DoAction(int32 const /*action*/) override
        {
            switch (me->GetEntry())
            {
                case TanaanCreatures::NpcCannonTurret:
                    m_Events.ScheduleEvent(eEvents::EventCannonBarrage, 1s);
                    break;
                case TanaanCreatures::NpcGunTurret:
                    m_Events.ScheduleEvent(eEvents::EventMachineGun, 3s);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            m_Events.Update(diff);

            switch (m_Events.ExecuteEvent())
            {
                case eEvents::EventCannonBarrage:
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random))
                        me->CastSpell(target, TanaanSpells::SpellCannonBarrage, false);
                    m_Events.ScheduleEvent(eEvents::EventCannonBarrage, 80s);
                    break;
                case eEvents::EventMachineGun:
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random))
                        me->CastSpell(target, TanaanSpells::SpellMachineGun, true);
                    m_Events.ScheduleEvent(eEvents::EventMachineGun, 8s);
                    break;
                default:
                    break;
            }
        }
    };
};

/// 231261 - Worldbreaker Side Turret
class gob_worldbreaker_side_turret : public GameObjectScript
{
public:
    gob_worldbreaker_side_turret() : GameObjectScript("gob_worldbreaker_side_turret")
    {
    }

    struct gob_worldbreaker_side_turretAI : public GameObjectAI
    {
        gob_worldbreaker_side_turretAI(GameObject* p_Go) : GameObjectAI(p_Go)
        {
        }

        bool OnGossipHello(Player* player) override
        {
            if (player->GetQuestStatus(TanaanQuests::QuestATasteOfIron))
            {
                player->RemoveAurasDueToSpell(TanaanPhases::PhaseFinalSideCanons);
                player->GetSceneMgr().RecreateScene(TanaanSceneObjects::SceneShootingGallery, SceneFlag::NotCancelable);
                player->AddAura(TanaanSpells::SpellTasteOfIronGameAura, player); // Aura pour lancer le scene object
                player->KilledMonsterCredit(TanaanKillCredits::CreditEnterWorldbreakerTurret);
            }
            return true;
        }

    };

    GameObjectAI* GetAI(GameObject* p_Go) const override
    {
        return new gob_worldbreaker_side_turretAI(p_Go);
    }
};

/// 232538 - Main Cannon Trigger
struct gob_main_cannon_trigger : public GameObjectAI
{
    gob_main_cannon_trigger(GameObject* go) : GameObjectAI(go) { }

    bool OnGossipHello(Player* player) override
    {
        if (player->GetQuestStatus(TanaanQuests::QuestATasteOfIron))
        {
            player->GetSceneMgr().CancelSceneByPackageId(TanaanSceneObjects::SceneShootingGallery);
            player->AddAura(TanaanPhases::PhaseFinalSideCanons, player);
            player->AddAura(TanaanPhases::PhaseFinalCanonDeco, player);

            player->KillCreditGO(TanaanKillCredits::CreditMainCannonTrigger);
            player->CastSpell(player, TanaanSpells::SpellFinaleCinematic, true);
        }

        return true;
    }
};

void AddSC_tanaan_intro_finale()
{
    new npc_tanaan_gogluk();
    new npc_tanaan_gogluk_adds();
    new gob_worldbreaker_side_turret();
    RegisterGameObjectAI(gob_main_cannon_trigger);
    new playerScript_taste_of_iron();
    new playerScript_the_home_stretch();
    new playerScript_tanaan_scene_boat();
}
