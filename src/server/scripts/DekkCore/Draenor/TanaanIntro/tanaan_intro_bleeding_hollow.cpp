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

#include "GameObject.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "SpellInfo.h"
#include "tanaan_intro.h"
#include "TemporarySummon.h"

/// Passive Scene Object
class playerScript_cost_of_war : public PlayerScript
{
public:
    playerScript_cost_of_war() : PlayerScript("playerScript_cost_of_war") {}

    void OnSceneComplete(Player* player, uint32 sceneInstanceId) override
    {
        if (!player->GetSceneMgr().HasScene(sceneInstanceId, TanaanSceneObjects::SceneCostOfWarEscort))
            return;

        player->CompleteQuest(TanaanQuests::QuestCostOfWar);
    }
};

/// Passive Scene Object
class playerScript_blaze_of_glory : public PlayerScript
{
public:
    playerScript_blaze_of_glory() : PlayerScript("playerScript_blaze_of_glory") { }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        if (player->HasQuest(TanaanQuests::QuestBlazeOfGlory))
            player->GetSceneMgr().PlaySceneByPackageId(BlazeOfGloryData::SceneId, SceneFlag::None);
    }

    void OnQuestAccept(Player* player, const Quest* quest) override
    {
        if (quest->GetQuestId() == TanaanQuests::QuestBlazeOfGlory)
            player->GetSceneMgr().PlaySceneByPackageId(BlazeOfGloryData::SceneId, SceneFlag::None);
    }

    void OnQuestReward(Player* player, const Quest* quest) override
    {
        if (player && quest && quest->GetQuestId() == TanaanQuests::QuestBlazeOfGlory)
            player->GetSceneMgr().CancelSceneByPackageId(BlazeOfGloryData::SceneId);
    }

    void OnSceneTriggerEvent(Player* player, uint32 p_SceneInstanceID, std::string p_Event) override
    {
        if (!player->GetSceneMgr().HasScene(p_SceneInstanceID, BlazeOfGloryData::SceneId))
            return;

        if (p_Event == "Visual" && player->HasQuest(TanaanQuests::QuestBlazeOfGlory))
            player->AddAura(BlazeOfGloryData::SpellTrailOfFlameVisual, player);
        else if (p_Event == "Clear")
            player->RemoveAura(BlazeOfGloryData::SpellTrailOfFlameVisual);
        else if (p_Event == "Credit")
            player->KilledMonsterCredit(81760);
    }
};

/// Passive Scene Object
class playerScript_tanaan_holdout : public PlayerScript
{
public:
    playerScript_tanaan_holdout() : PlayerScript("playerScript_tanaan_holdout") { }

    void OnSceneComplete(Player* player, uint32 p_SceneInstanceID) override
    {
        if (!player->GetSceneMgr().HasScene(p_SceneInstanceID, TanaanSceneObjects::SceneHoldout))
            return;

        // Set les bonnes phases
        player->AddAura(TanaanPhases::PhaseAfterBlazeGlobal, player);
        player->AddAura(TanaanPhases::PhaseAfterBlazeAriok, player);

        if (player->GetTeamId() == TEAM_ALLIANCE)
            player->AddAura(TanaanPhases::PhaseAfterBlazeAlliance, player);
        else
            player->AddAura(TanaanPhases::PhaseAfterBlazeHorde, player);
    }
};

/// 78507 - Bleeding Hollow Savage
class npc_bleeding_hollow_sauvage : public CreatureScript
{
public:
    npc_bleeding_hollow_sauvage() : CreatureScript("npc_bleeding_hollow_sauvage") { }

    struct npc_bleeding_hollow_sauvageAI : public ScriptedAI
    {
        npc_bleeding_hollow_sauvageAI(Creature* creature) : ScriptedAI(creature) { }

        void SpellHit(WorldObject* p_Caster, const SpellInfo* spellInfo) override
        {
            if (spellInfo->Id == TanaanSpells::SpellArcaneBarrage)
            {
               // p_Caster->DealDamage(me, me->GetHealth());
                me->DespawnOrUnsummon();
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_bleeding_hollow_sauvageAI(creature);
    }
};

/// 83670 - Blood Ritual Orb
class npc_blood_ritual_orb : public CreatureScript
{
public:
    npc_blood_ritual_orb() : CreatureScript("npc_blood_ritual_orb") { }

    struct npc_blood_ritual_orbAI : public ScriptedAI
    {
        npc_blood_ritual_orbAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            me->SetDisableGravity(true);
        }

        bool OnGossipHello(Player* player) override
        {
            if (player->GetQuestStatus(TanaanQuests::QuestAltarAltercation) == QUEST_STATUS_INCOMPLETE)
            {
                    player->KilledMonsterCredit(TanaanKillCredits::CreditBloodOrb);
                    me->DestroyForPlayer(player);

                    player->GetSceneMgr().PlaySceneByPackageId(TanaanSceneObjects::SceneSneakyArmy, SceneFlag::None);
                    {
                        player->GetSceneMgr().PlaySceneByPackageId(TanaanSceneObjects::SceneKilRoggRevealed, SceneFlag::None);

                        player->RemoveAurasDueToSpell(TanaanPhases::PhaseAfterBlazeGlobal);
                        player->RemoveAurasDueToSpell(TanaanPhases::PhaseAfterBlazeAlliance);
                        player->RemoveAurasDueToSpell(TanaanPhases::PhaseAfterBlazeHorde);
                    }
                }

            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_blood_ritual_orbAI(creature);
    }
};

void AddSC_tanaan_intro_bleeding_hollow()
{
    new npc_bleeding_hollow_sauvage();
    new npc_blood_ritual_orb();
    new playerScript_cost_of_war();
    new playerScript_blaze_of_glory();
    new playerScript_tanaan_holdout();
}
