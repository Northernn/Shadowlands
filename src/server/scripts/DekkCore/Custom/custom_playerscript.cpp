/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
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

#include "Conversation.h"
#include "Creature.h"
#include "Channel.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include "DBCEnums.h"
#include "GameEventMgr.h"
#include "GameObjectAI.h"
#include "MiscPackets.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "SpellAuras.h"
#include "SpellHistory.h"
#include "PhasingHandler.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "World.h"
#include "WorldStatePackets.h"
#include "ScriptMgr.h"
#include "PetBattle.h"
#include "Common.h"
#include "BattlePetSystem.h"
#include <Configuration/Config.h>

#define Playedtimetochat 1500
#define mutetimecencure 300
#define mutetimeantispam 10

enum miscsl
{
    SPELL_CREATE_GARRISON_ARTIFACT_1 = 309389,
    SPELL_CREATE_GARRISON_ARTIFACT_2 = 309390,
    SPELL_CREATE_GARRISON_ARTIFACT_3 = 309391,
    SPELL_CREATE_GARRISON_SL         = 322277,
};

// 355352
struct go_shadowlands_covenant_map : public GameObjectAI
{
    go_shadowlands_covenant_map(GameObject* go) : GameObjectAI(go) { }

    bool OnGossipHello(Player* player) override
    {
        if (player->GetQuestStatus(62000) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(57878) == QUEST_STATUS_INCOMPLETE)// Choosing Your Purpose
            player->SendPlayerChoice(player->GetGUID(), 644);

        return true;
    }
};

class PlayerScript_WeeklyQuests : public PlayerScript
{
public:
    PlayerScript_WeeklyQuests() : PlayerScript("PlayerScript_WeeklyQuests") {}

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        if (!player)
            return;

        if (!sGameEventMgr->IsActiveEvent(130))
        {
           // player->RemoveActiveQuest(62631, true); // Evento Bonificacion Misiones de mundo
            player->RemoveAura(225788);
        }
        else
            player->CastSpell(player, 225788, false);

        if (!sGameEventMgr->IsActiveEvent(131))
        {
          //  player->RemoveActiveQuest(62637); // Evento Bonificacion Campos de Batalla
            player->RemoveAura(186403);
        }
        else
            player->CastSpell(player, 186403, false);

        if (!sGameEventMgr->IsActiveEvent(132))
        {
        //    player->RemoveActiveQuest(62639); // Evento Bonificacion Mascotas
            player->RemoveAura(186406);
        }
        else
            player->CastSpell(player, 186406, false);

        if (!sGameEventMgr->IsActiveEvent(133))
        {
           // player->RemoveActiveQuest(62640); // Evento Bonificacion Escaramusas
            player->RemoveAura(186401);
        }
        else
            player->CastSpell(player, 186401, false);

        if (!sGameEventMgr->IsActiveEvent(134))
        {
           // player->RemoveActiveQuest(62638); // Evento Bonificacion Mamorras Miticas
            player->RemoveAura(225787);
        }
        else
            player->CastSpell(player, 225787, false);
    }

    void OnMapChanged(Player* player) override
    {
        if (!player)
            return;

        if (!sGameEventMgr->IsActiveEvent(130))
            player->RemoveAura(225788);
        if (!sGameEventMgr->IsActiveEvent(131))
            player->RemoveAura(186403);
        if (!sGameEventMgr->IsActiveEvent(132))
            player->RemoveAura(186406);
        if (!sGameEventMgr->IsActiveEvent(133))
            player->RemoveAura(186401);
        if (!sGameEventMgr->IsActiveEvent(134))
            player->RemoveAura(225787);

    }
};

enum Create_Garrison
{
    SPELL_CREATE_WAR_CAMPAIGN_H = 273381,
    SPELL_CREATE_WAR_CAMPAIGN_A = 273382
};

class player_level_rewards : public PlayerScript
{
public:
    player_level_rewards() : PlayerScript("player_level_rewards") {}

    void OnLevelChanged(Player* player, uint8 oldLevel) override
    {
        if (oldLevel <= 20 && player->GetLevel() >= 21) // Create WarCampaing
        {
            player->GetTeam() == ALLIANCE ? player->CastSpell(player, SPELL_CREATE_WAR_CAMPAIGN_A, false) : player->CastSpell(player, SPELL_CREATE_WAR_CAMPAIGN_H, false);
        }

        if (oldLevel <= 9 && player->GetLevel() >= 10)
        {
            switch (player->GetRace()) // Heritage Armor
            {
            case RACE_VOID_ELF:
                if (player->GetQuestStatus(49928) == QUEST_STATUS_NONE)
                    if (const Quest* quest = sObjectMgr->GetQuestTemplate(49928))
                        player->AddQuest(quest, nullptr);
                break;
            case RACE_LIGHTFORGED_DRAENEI:
                if (player->GetQuestStatus(49782) == QUEST_STATUS_NONE)
                    if (const Quest* quest = sObjectMgr->GetQuestTemplate(49782))
                        player->AddQuest(quest, nullptr);
                break;
            case RACE_NIGHTBORNE:
                if (player->GetQuestStatus(49784) == QUEST_STATUS_NONE)
                    if (const Quest* quest = sObjectMgr->GetQuestTemplate(49784))
                        player->AddQuest(quest, nullptr);
                break;
            case RACE_HIGHMOUNTAIN_TAUREN:
                if (player->GetQuestStatus(49783) == QUEST_STATUS_NONE)
                    if (const Quest* quest = sObjectMgr->GetQuestTemplate(49783))
                        player->AddQuest(quest, nullptr);
                break;
            case RACE_DARK_IRON_DWARF:
                if (player->GetQuestStatus(51483) == QUEST_STATUS_NONE)
                    if (const Quest* quest = sObjectMgr->GetQuestTemplate(51483))
                        player->AddQuest(quest, nullptr);
                break;
            case RACE_MAGHAR_ORC:
                if (player->GetQuestStatus(51484) == QUEST_STATUS_NONE)
                    if (const Quest* quest = sObjectMgr->GetQuestTemplate(51484))
                        player->AddQuest(quest, nullptr);
                break;
            case RACE_ZANDALARI_TROLL:
                if (player->GetQuestStatus(53721) == QUEST_STATUS_NONE)
                    if (const Quest* quest = sObjectMgr->GetQuestTemplate(53721))
                        player->AddQuest(quest, nullptr);
                break;
            case RACE_KUL_TIRAN:
                if (player->GetQuestStatus(53722) == QUEST_STATUS_NONE)
                    if (const Quest* quest = sObjectMgr->GetQuestTemplate(53722))
                        player->AddQuest(quest, nullptr);
                break;
            case RACE_VULPERA:
                if (player->GetQuestStatus(58435) == QUEST_STATUS_NONE)
                    if (const Quest* quest = sObjectMgr->GetQuestTemplate(58435))
                        player->AddQuest(quest, nullptr);
                break;
            case RACE_MECHAGNOME:
                if (player->GetQuestStatus(58436) == QUEST_STATUS_NONE)
                    if (const Quest* quest = sObjectMgr->GetQuestTemplate(58436))
                        player->AddQuest(quest, nullptr);
                break;
            default:
                break;
            }
        }
    }
};

class Boss_Announcer : public PlayerScript
{
public:
    Boss_Announcer() : PlayerScript("Boss_Announcer") {}

    void OnCreatureKill(Player* player, Creature* boss)
    {
        if (sConfigMgr->GetBoolDefault("Boss.Announcer.Enable", true))
        {
            if (boss->isWorldBoss())
            {
                std::string plr = player->GetName();
                std::string boss_n = boss->GetName();
                std::string tag_colour = "7bbef7";
                std::string plr_colour = "7bbef7";
                std::string boss_colour = "ff0000";
                std::ostringstream stream;
                stream << "|CFF" << tag_colour <<

                    "|rThe group of|r""|r|cff" << plr_colour << " " << plr << "|r Killed "" |CFF" << boss_colour << "[" << boss_n << "]|r " "World Boss" << "! Congratulations!";
                sWorld->SendServerMessage(SERVER_MSG_STRING, stream.str().c_str());
            }
        }
    };
};

class DemonHunterTalentTempFix : public PlayerScript
{
public:
    DemonHunterTalentTempFix() : PlayerScript("DemonHunterTalentTempFix") { }

    void OnLogin(Player* player, bool /* firstLogin */) override
    {
        if (player->GetClass() == CLASS_DEMON_HUNTER && !player->HasSpell(200749))
            player->LearnSpell(200749, false);
    }

    void OnLevelChanged(Player* player, uint8 /*oldLevel*/) override
    {
        if (player->GetClass() == CLASS_DEMON_HUNTER && !player->HasSpell(200749))
            player->LearnSpell(200749, false);
    }
};

class OnWodGarrisonArrival : public PlayerScript
{
public:
    OnWodGarrisonArrival() : PlayerScript("OnWodGarrisonArrival") { }

    enum Pregarrquest
    {
        QUEST_A_STEP_THREE_PROPHET           = 34575,
        QUEST_H_THE_HOME_OF_THE_FROST_WOLVES = 33868,
    };

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        if (player->GetQuestStatus(QUEST_A_STEP_THREE_PROPHET))
            return;

        if (player->GetQuestStatus(QUEST_H_THE_HOME_OF_THE_FROST_WOLVES))
            return;

        if (player->IsInAlliance() && (player->GetZoneId() == 6719))
            player->AddQuest(sObjectMgr->GetQuestTemplate(QUEST_A_STEP_THREE_PROPHET), nullptr);

        if (player->IsInHorde() && (player->GetZoneId() == 6720))
            player->AddQuest(sObjectMgr->GetQuestTemplate(QUEST_H_THE_HOME_OF_THE_FROST_WOLVES), nullptr);
    }
};

class CloseGossipWindowHackFix : public PlayerScript
{
public:
    CloseGossipWindowHackFix() : PlayerScript("CloseGossipWindowHackFix") { }

    // Called when player accepts some quest
    void OnQuestAccept(Player* player, Quest const* /*quest*/) override
    {
        player->PlayerTalkClass->SendCloseGossip();
        player->PlayerTalkClass->GetInteractionData().Reset();
    }

    // Called when player rewards some quest
    void OnQuestReward(Player* player, Quest const* /*quest*/) override
    {
        player->PlayerTalkClass->SendCloseGossip();
        player->PlayerTalkClass->GetInteractionData().Reset();
    }
};

class ps_sl_CreateGarrison : public PlayerScript
{
public:
    ps_sl_CreateGarrison() : PlayerScript("ps_sl_CreateGarrison") { }


    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        if (player->GetMapId() == 2222)
        {
            player->CastSpell(player, SPELL_CREATE_GARRISON_ARTIFACT_1, true);
            player->CastSpell(player, SPELL_CREATE_GARRISON_ARTIFACT_2, true);
            player->CastSpell(player, SPELL_CREATE_GARRISON_ARTIFACT_3, true);
            player->CastSpell(player, SPELL_CREATE_GARRISON_SL, true);
            player->SetPlayerLocalFlag(PlayerLocalFlags::PLAYER_LOCAL_FLAG_ACCOUNT_SECURED);
        }
    }
};

class DragonflightArrival : public PlayerScript
{
public:
    DragonflightArrival() : PlayerScript("DragonflightArrival") { }

    enum dfquest
    {
        QUEST_THE_DRAGON_ISLES_AWAIT_ALLIANCE = 65436,
        QUEST_THE_DRAGON_ISLES_AWAIT_HORDE = 65435,
    };

    void OnLogin(Player* player, bool firstLogin) override
    {

        if (firstLogin)
        {
            switch (player->GetTeam())
            {
            case ALLIANCE:

                if (player->GetLevel() == 10)
                    player->AddQuest(sObjectMgr->GetQuestTemplate(QUEST_THE_DRAGON_ISLES_AWAIT_ALLIANCE), nullptr);
                player->ForceCompleteQuest(QUEST_THE_DRAGON_ISLES_AWAIT_ALLIANCE);

                break;

            case HORDE:
                if (player->GetLevel() == 10)
                    player->AddQuest(sObjectMgr->GetQuestTemplate(QUEST_THE_DRAGON_ISLES_AWAIT_HORDE), nullptr);
                player->ForceCompleteQuest(QUEST_THE_DRAGON_ISLES_AWAIT_HORDE);
                break;
            }
        }
    }
};

class LearnRiding : public PlayerScript
{
public:
    LearnRiding() : PlayerScript("LearnRiding") { }

    void CheckRidingSkill(Player* player)
    {
        enum
        {
            APPRENTINCE_RIDING = 33388,
            JOURNEYMAN_RIDING = 33391,
            EXPERT_RIDING = 34090,
            MASTER_RIDING = 90265,
            DRAGONRIDING_BASICS = 376777,
            DRAGONRIDING_WHIRLING_SURGE = 361584,
            DRAGONRIDING_LIFT_OFF = 374763,
            QUEST_TIME_TO_FLY_HORDE = 75874,
            QUEST_TIME_TO_FLY_ALLIANCE = 75877,
        };

        std::map<int8, uint32> levelToRiding
        {
            {10, APPRENTINCE_RIDING},
            {20, JOURNEYMAN_RIDING},
            {30, EXPERT_RIDING},
            {40, MASTER_RIDING},
            {60, DRAGONRIDING_BASICS},
            {60, DRAGONRIDING_WHIRLING_SURGE},
            {60, DRAGONRIDING_LIFT_OFF},
        };

        for (auto riding = levelToRiding.begin(); riding != levelToRiding.end(); riding++)
            if (player->GetLevel() >= riding->first && !player->HasSpell(riding->second))
                player->LearnSpell(riding->second, true);

        if (player->GetLevel() >= 30)
        {
            if (player->IsInHorde() && player->GetQuestStatus(QUEST_TIME_TO_FLY_HORDE) != QUEST_STATUS_REWARDED && !player->HasQuest(QUEST_TIME_TO_FLY_HORDE))
                if (const Quest* quest = sObjectMgr->GetQuestTemplate(QUEST_TIME_TO_FLY_HORDE))
                    player->AddQuest(quest, player);

            player->SetSkill(762, 375, 375, 375);

            if (player->IsInAlliance() && player->GetQuestStatus(QUEST_TIME_TO_FLY_ALLIANCE) != QUEST_STATUS_REWARDED && !player->HasQuest(QUEST_TIME_TO_FLY_ALLIANCE))
                if (const Quest* quest = sObjectMgr->GetQuestTemplate(QUEST_TIME_TO_FLY_ALLIANCE))
                    player->AddQuest(quest, player);
        }
    }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        CheckRidingSkill(player);
    }

    void OnLevelChanged(Player* player, uint8 /*oldLevel*/) override
    {
        CheckRidingSkill(player);
    }
};

class System_Taberna : public PlayerScript
{
public:
    System_Taberna() : PlayerScript("System_Taberna") { }

    void AddIcon(Player* player, std::string& msg, uint32 lang)
    {
        if (player->isGMChat())
            return;
        if (player->GetTeamId() == 0)
            msg = "|TInterface/FriendsFrame/PlusManz-Alliance:20:21|t" + msg;
        else if (player->GetTeamId() == 1)
            msg = "|TInterface/FriendsFrame/PlusManz-Horde:20:21|t" + msg;

    };

    void OnChat(Player* player, uint32 /*type*/, uint32 lang, std::string& msg, Channel* channel) override
    {
        std::string canal = channel->GetName();
        std::transform(canal.begin(), canal.end(), canal.begin(), ::tolower);
        if (canal == "|cFFFFF569taberna|r")  AddIcon(player, msg, lang);
    }
};

class PromotionNewPlayers : public PlayerScript
{
public:
    PromotionNewPlayers() :PlayerScript("PromotionNewPlayers") {}

    enum Values
    {
        April2023 = 1422777600,  ///< Timestamp of 1st of April 2023.
        May252023 = 1427065200,  ///< Timestamp of 25nd of May 2023.
        June152023 = 1426374000,  ///< Timestamp of 15nd of June 2023.
        SwiftNetherDrake = 37015,   ///< SpellId.
        ImperialQuilen = 85870,   ///< ItemId
        ApprenticeRidingSkill = 33388
    };

    void OnLogin(Player* p_Player, bool /*firstLogin*/) override
    {
        if (time(NULL) < Values::April2023 && p_Player->GetTotalPlayedTime() == 0)
        {
            p_Player->LearnSpell(Values::ApprenticeRidingSkill, false);
            p_Player->LearnSpell(Values::SwiftNetherDrake, false);
        }

        if (time(nullptr) < Values::May252023
            && p_Player->GetSession()->GetAccountJoinDate() >= Values::May252023
            && p_Player->GetTotalPlayedTime() == 0)
            p_Player->AddItem(Values::ImperialQuilen, 1);
    }
};

class AccountAchievements : public PlayerScript
{
    static const bool limitrace = true; // This set to true will only achievements from chars on the same team, do what you want. NOT RECOMMANDED TO BE CHANGED!!!
    static const bool limitlevel = true; // This checks the player's level and will only add achievements to players of that level.
    int minlevel = 70; // It's set to players of the level 60. Requires limitlevel to be set to true.
    int setlevel = 1; // Dont Change

public:
    AccountAchievements() : PlayerScript("AccountAchievements") { }

    void OnLogin(Player* pPlayer, bool /*firstlogin*/)
    {
        if (sConfigMgr->GetBoolDefault("Accountwide.Achievements", true)) {
            std::vector<uint32> Guids;
            QueryResult result1 = CharacterDatabase.PQuery("SELECT guid, race FROM characters WHERE account = {}", pPlayer->GetSession()->GetAccountId());
            if (!result1)
                return;

            do
            {
                Field* fields = result1->Fetch();

                //uint32 guid = fields[0].GetUInt32();
                uint32 race = fields[1].GetUInt8();

                if ((Player::TeamForRace(race) == Player::TeamForRace(pPlayer->GetRace())) || !limitrace)
                    Guids.push_back(result1->Fetch()[0].GetUInt32());

            } while (result1->NextRow());

            std::vector<uint32> Achievement;

            for (auto& i : Guids)
            {
                QueryResult result2 = CharacterDatabase.PQuery("SELECT achievement FROM character_achievement WHERE guid = {}", i);
                if (!result2)
                    continue;

                do
                {
                    Achievement.push_back(result2->Fetch()[0].GetUInt32());
                } while (result2->NextRow());
            }

            for (auto& i : Achievement)
            {
                auto sAchievement = sAchievementStore.LookupEntry(i);
                AddAchievements(pPlayer, sAchievement->ID);
            }
        }

        else {
            return;
        }
    }

    void AddAchievements(Player* player, uint32 AchievementID)
    {
        if (limitlevel)
            setlevel = minlevel;

        if (player->GetLevel() >= setlevel)
            player->CompletedAchievement(sAchievementStore.LookupEntry(AchievementID));
    }
};

class PlayerScriptPetBattle : public PlayerScript
{
public:
    PlayerScriptPetBattle() : PlayerScript("PlayerScriptPetBattle") { }

    void OnMovementInform(Player* player, uint32 mveType, uint32 id) override
    {
        if (player && mveType == POINT_MOTION_TYPE && id == PET_BATTLE_ENTER_MOVE_SPLINE_ID)
        {
            m_Mutex.lock();
            m_DelayedPetBattleStart[player->GetGUID()] = getMSTime() + 1000;
            m_Mutex.unlock();
        }
    }

    void OnUpdate(Player* player, uint32 /*diff*/) override
    {
        m_Mutex.lock();

        if (m_DelayedPetBattleStart.find(player->GetGUID()) != m_DelayedPetBattleStart.end())
        {
            if (m_DelayedPetBattleStart[player->GetGUID()] > getMSTime())
            {
                m_DelayedPetBattleStart.erase(m_DelayedPetBattleStart.find(player->GetGUID()));

                if (PetBattle* battle = sPetBattleSystem->GetBattle(player->_petBattleId))
                {
                    player->SetUnitFlag(UNIT_FLAG_PACIFIED | UNIT_FLAG_IMMUNE_TO_NPC); // Immuned only to NPC
                    player->SetControlled(true, UNIT_STATE_ROOT);
                    battle->Begin();
                }
            }
        }

        m_Mutex.unlock();
    }

    std::map<ObjectGuid, uint32> m_DelayedPetBattleStart;
    std::mutex m_Mutex;
};

#define FACTION_SPECIFIC 0
using namespace std;
const char* CLASS_ICON;
const char* RACE_ICON;

std::string GetNameLink(Player* player)
{
    std::string name = player->GetName();
    std::string color;
    std::string icon;
    switch (player->GetRace())
    {
    case RACE_NIGHTBORNE:
    case RACE_PANDAREN_ALLIANCE:
    case RACE_PANDAREN_HORDE:
    case RACE_PANDAREN_NEUTRAL:
    case RACE_BLOODELF:
        if (player->GetGender() == GENDER_MALE)
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Bloodelf_Male:15|t";
        else
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Bloodelf_Female:15|t";
        break;
        // Done - Dranei
    case RACE_HIGHMOUNTAIN_TAUREN:
    case RACE_VOID_ELF:
    case RACE_LIGHTFORGED_DRAENEI:
    case RACE_ZANDALARI_TROLL:
    case RACE_KUL_TIRAN:
    case RACE_DRAENEI:
        if (player->GetGender() == GENDER_FEMALE)
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Draenei_Female:15|t";
        else
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Draenei_Male:15|t";
        break;
    case RACE_DARK_IRON_DWARF:
    case RACE_VULPERA:
    case RACE_MAGHAR_ORC:
    case RACE_MECHAGNOME:
    case RACE_DRACTHYR_ALLIANCE:
    case RACE_DRACTHYR_HORDE:
    case RACE_DWARF:
        if (player->GetGender() == GENDER_FEMALE)
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Dwarf_Female:15|t";
        else
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Dwarf_Male:15|t";
        break;
        // Done - Gnome
    case RACE_GNOME:
        if (player->GetGender() == GENDER_FEMALE)
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Gnome_Female:15|t";
        else
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Gnome_Male:15|t";
        break;
        // Done - Human
    case RACE_HUMAN:
        if (player->GetGender() == GENDER_FEMALE)
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Human_Female:15|t";
        else
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Human_Male:15|t";
        break;
    case RACE_NIGHTELF:
        if (player->GetGender() == GENDER_FEMALE)
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Nightelf_Female:15|t";
        else
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Nightelf_Male:15|t";
        break;
    case RACE_ORC:
        if (player->GetGender() == GENDER_FEMALE)
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Orc_Female:15|t";
        else
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Orc_Male:15|t";
        break;
        // Done - Tauren
    case RACE_TAUREN:
        if (player->GetGender() == GENDER_FEMALE)
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Tauren_Female:15|t";
        else
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Tauren_Male:15|t";
        break;
    case RACE_TROLL:
        if (player->GetGender() == GENDER_FEMALE)
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Troll_Female:15|t";
        else
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Troll_Male:15|t";
        break;
    case RACE_UNDEAD_PLAYER:
        if (player->GetGender() == GENDER_FEMALE)
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Undead_Female:15|t";
        else
            RACE_ICON = "|TInterface/ICONS/Achievement_Character_Undead_Male:15|t";
        break;
    }

    switch (player->GetClass())
    {
    case CLASS_DEATH_KNIGHT:
        color = "|cffC41F3B";
        //CLASS_ICON = "|TInterface\\icons\\Spell_Deathknight_ClassIcon:15|t|r";
        break;
    case CLASS_DRUID:
        color = "|cffFF7D0A";
        // CLASS_ICON = "|TInterface\\icons\\Ability_Druid_Maul:15|t|r";
        break;
    case CLASS_HUNTER:
        color = "|cffABD473";
        // CLASS_ICON = "|TInterface\\icons\\INV_Weapon_Bow_07:15|t|r";
        break;
    case CLASS_MAGE:
        color = "|cff69CCF0";
        //CLASS_ICON = "|TInterface\\icons\\INV_Staff_13:15|t|r";
        break;
    case CLASS_PALADIN:
        color = "|cffF58CBA";
        // CLASS_ICON = "|TInterface\\icons\\INV_Hammer_01:15|t|r";
        break;
    case CLASS_PRIEST:
        color = "|cffFFFFFF";
        // CLASS_ICON = "|TInterface\\icons\\INV_Staff_30:15|t|r";
        break;
    case CLASS_ROGUE:
        color = "|cffFFF569";
        // CLASS_ICON = "|TInterface\\icons\\INV_ThrowingKnife_04:15|t|r";
        break;
    case CLASS_SHAMAN:
        color = "|cff0070DE";
        // CLASS_ICON = "|TInterface\\icons\\Spell_Nature_BloodLust:15|t|r";
        break;
    case CLASS_WARLOCK:
        color = "|cff9482C9";
        //  CLASS_ICON = "|TInterface\\icons\\Spell_Nature_FaerieFire:15|t|r";
        break;
    case CLASS_WARRIOR:
        color = "|cffC79C6E";
        // CLASS_ICON = "|TInterface\\icons\\INV_Sword_27.png:15|t|r";
        break;
        case CLASS_EVOKER:
        color = "|cffC79C6E";
        break;
        case CLASS_MONK:
            color = "|cffC79C6E";
            break;
        case CLASS_DEMON_HUNTER:
            color = "|cffC79C6E";
            break;
        // CLASS_ICON = "|TInterface\\icons\\INV_Sword_27.png:15|t|r";
        break;
    }
    return "|Hplayer:" + name + "|h" + RACE_ICON + "|cffFFFFFF[" + color + name + "|cffFFFFFF]|h|r";
}

void _SendWorldChat(Player* player, string message)
{
    size_t stringpos;

    if (message.find("|TInterface") != string::npos)
        return;

    if (message.find("\n") != string::npos)
        return;

    if ((stringpos = message.find("|Hquest:")) != string::npos)
        return;

    if ((stringpos = message.find("|Htrade:")) != string::npos)
        return;

    if ((stringpos = message.find("|Htalent:")) != string::npos)
        return;

    if ((stringpos = message.find("|Henchant:")) != string::npos)
        return;

    if ((stringpos = message.find("|Hachievement:")) != string::npos)
        return;

    if ((stringpos = message.find("|Hglyph:")) != string::npos)
        return;

    if ((stringpos = message.find("|Hspell:")) != string::npos)
        return;

    if ((stringpos = message.find("Hitem:")) != string::npos)
        return;

    if (message.find("|c") != string::npos && message.find("|r") != string::npos)
        return;

    if (message.find("|c") != string::npos && message.find("|h") != string::npos)
        return;

    uint8 cheksSize = 118;//Change these if you want to add more words to the array.
    std::string checks[118];//Change these if you want to add more words to the array.
    // Strony (Sites)
    // Strony (Sites)
    checks[0] = "http://";
    checks[1] = ".com";
    checks[2] = ".net";
    checks[3] = ".org";
    checks[4] = "www.";
    checks[5] = "wow-";
    checks[6] = "-wow";
    checks[7] = "no-ip";
    checks[8] = ".zapto";
    checks[9] = ".biz";
    checks[10] = ".servegame";
    checks[11] = ".ir";
    checks[12] = "com.br";

    checks[13] = "h t t p : / /";
    checks[14] = ". c o m";
    checks[15] = ". n e t";
    checks[16] = ". o r g";
    checks[17] = "w w w .";
    checks[18] = " w o w -";
    checks[19] = "- w o w";
    checks[20] = "n o - i p";
    checks[21] = ". z a p t o";
    checks[22] = ". b i z";
    checks[23] = ". s e r v e g a m e";
    checks[24] = ". b r";
    checks[25] = "c o m . b r";

    checks[26] = "h  t  t  p  :  /  /";
    checks[27] = ".  c  o  m";
    checks[28] = ".  n  e  t";
    checks[29] = ".  o  r  g";
    checks[30] = "w  w  w  .";
    checks[31] = " w  o  w  -";
    checks[32] = "-  w  o  w";
    checks[33] = "n  o  -  i  p";
    checks[34] = ".  z  a  p  t  o";
    checks[35] = ".  b  i  z";
    checks[36] = ".  s  e  r  v  e  g  a  m  e";
    checks[37] = ".  b  r";
    checks[38] = "c  o  m  .  b  r";

    checks[39] = "h   t   t   p   :   /   /";
    checks[40] = ".   c   o   m";
    checks[41] = ".   n   e   t";
    checks[42] = ".   o   r   g";
    checks[43] = "w   w   w   .";
    checks[44] = " w   o   w   -";
    checks[45] = "-   w   o   w";
    checks[46] = "n   o   -   i   p";
    checks[47] = ".   z   a   p   t   o";
    checks[48] = ".   b   i   z";
    checks[49] = ".   s   e   r   v   e   g   a   m   e";
    checks[50] = ".   b   r";
    checks[51] = "   c   o   m   .   b   r";

    checks[52] = "h    t    t    p   :   /   /";
    checks[53] = ".    c    o    m";
    checks[54] = ".    n    e   t";
    checks[55] = ".    o    r    g";
    checks[56] = "w    w    w    .";
    checks[57] = "w    o    w    -";
    checks[58] = "-    w    o    w";
    checks[59] = "n    o    -    i    p";
    checks[60] = ".    z    a    p    t    o";
    checks[61] = ".    b    i     z";
    checks[62] = ".    s    e    r    v    e    g    a    m    e";
    checks[63] = ".    b    r";
    checks[64] = "c    o    m    .    b    r";

    checks[65] = "trevon";
    checks[66] = "megawow";
    checks[67] = "fatalwow";
    checks[68] = "uniforgiven-wow";
    checks[69] = "wow-autolouco";
    checks[70] = "heaven-wow";
    checks[71] = "fireballwow";
    checks[72] = "wowbrasilpa";
    checks[73] = "fatalitywow";
    checks[74] = "demonic-wow";
    checks[75] = "revenge-wow";
    checks[76] = "heavenwow";
    checks[77] = "logon.";
    checks[78] = "linebr";
    checks[79] = "azralon";
    checks[80] = "ultra";
    checks[81] = "ultra-wow";

    checks[82] = "t r e v o n";
    checks[83] = "m e g a w o w";
    checks[84] = "f a t a l w o w";
    checks[85] = "u n i f o r g i v e n - w o w";
    checks[86] = "w o w - a u t o l o u c o";
    checks[87] = "h e a v e n - w o w";
    checks[88] = "f i r e b a l l w o w";
    checks[89] = "w o w b r a s i l  p a";
    checks[90] = "f a t a l i t y w o w";
    checks[91] = "d e m o n i c - w o w";
    checks[92] = "r e v e n g e - w o w";
    checks[93] = "h e a v e n w o w";
    checks[94] = "u n d e a d - w o w";
    checks[95] = "l i n e b r";
    checks[96] = "a z r a l o n";
    checks[97] = "b l a c k - w o w";
    checks[98] = "t r e v o n w o w";

    checks[99] = "t  r  e  v  o  n";
    checks[100] = "m  e  g  a  w  o  w";
    checks[101] = "f  a  t  a  l  w  o  w";
    checks[102] = "u  n  i  f  o  r  g  i  v  e  n  -  w  o  w";
    checks[103] = "w  o  w  -  a  u  t  o   l o  u  c  o";
    checks[104] = "h  e  a  v  e  n  -  w  o  w";
    checks[105] = "f  i  r  e  b  a  l  l  w  o  w";
    checks[106] = "w  o  w  b  r  a  s  i  l  p  a";
    checks[107] = "f  a  t  a  l  i  t  y  w  o  w";
    checks[108] = "d  e  m  o  n  i  c  -  w  o  w";
    checks[109] = "r  e  v  e  n  g  e  -  w  o  w";
    checks[110] = "h  e  a  v  e  n  w  o  w";
    checks[111] = "u  n  d  e  a  d  -  w  o  w";
    checks[112] = "l  i  n  e  b  r";
    checks[113] = "a  z  r  a  l  o  n";
    checks[114] = "b  l  a  c  k  -  w  o  w";
    checks[115] = "t  r  e  v  o  n  w  o  w";

    checks[116] = " [The Lightbringer's Redemption]"; // old source code will crashed with this macro we cencured this
    checks[117] = "[The Lightbringer's Redemption]"; // old source code will crashed with this macro we cencured this

    for (int i = 0; i < cheksSize; ++i)
    {
        if (message.find(checks[i]) != string::npos)
        {
            std::string say = "";
            std::string str = "";
            say = message;
            sWorld->SendGMText(17000, player->GetName().c_str(), say.c_str()); // sned passive report to gm
            say = "";
            ChatHandler(player->GetSession()).PSendSysMessage("Links or Bad Words are not allowed on the server.");
            LoginDatabasePreparedStatement* mt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_MUTE_TIME);
            int64 muteTime = time(NULL) + mutetimecencure; // muted player if use bad words
            player->GetSession()->m_muteTime = muteTime;
            mt->setInt64(0, muteTime);
            return;
        }
    }

    string msg;
    ostringstream chat_string;

    if (player->GetTotalPlayedTime() <= Playedtimetochat) // New If - Played Time Need For Use This Cmd
    {
        std::string adStr = secsToTimeString(Playedtimetochat - player->GetTotalPlayedTime());
        player->GetSession()->SendNotification("You Must %s seconds played To use world chat!", adStr.c_str());
        return;
    }

    // if you have vip script can enable this option and will work eazy like amdwow
    /*switch (player->GetSession()->GetVipLevel()) // vip cases aded
    {
    case 1: // Vip Rank 1
        msg += "|cffffffff[VIP Bronze]";
        break;
    case 2: // Vip Rank 1
        msg += "|cffbbbbbb[VIP Silver]";
        break;
    case 3: // Vip Rank 1
        msg += "|cffff00ff[VIP Gold]";
        break;
    case 4: // Vip Rank 1
        msg += "|cffff6060[VIP Platinum]";
        break;
    case 5: // Vip Rank 1
        msg += "|cff0000ff[VIP Diamond]";
        break;
    case 6: // Vip Rank 1
        msg += "|cffff0000[VIP Warlord]";
        break;
    }*/
    switch (player->GetSession()->GetSecurity())
    {
        // Player
    case SEC_PLAYER:
        if (player->GetTeam() == ALLIANCE)
        {
            msg += "|cff00ff00[World] ";
            msg += "|cfffa9900[Player]";
            msg += "|cff0000ff|TInterface\\pvpframe\\pvp-currency-alliance:17|t|r ";
            msg += GetNameLink(player);
            msg += ":|cfffaeb00";
        }

        else
        {
            msg += "|cff00ff00[World] ";
            msg += "|cfffa9900[Player]";
            msg += "|cffff0000|TInterface\\pvpframe\\pvp-currency-horde:17|t|r ";
            msg += GetNameLink(player);
            msg += ":|cfffaeb00";
        }
        break;
        // Moderator
    case SEC_MODERATOR:
        msg += "|cff00ff00[World]";
        msg += "|cffFF8C00[Moderator]";
        msg += " |TINTERFACE/CHATFRAME/UI-CHATICON-BLIZZ:15|t|r ";
        msg += GetNameLink(player);
        msg += ":|cfffaeb00";
        break;
    case SEC_GAMEMASTER:
        msg += "|cff00ff00[World]";
        msg += "|cffFF8C00[GM]";
        msg += " |TINTERFACE/CHATFRAME/UI-CHATICON-BLIZZ:15|t|r ";
        msg += GetNameLink(player);
        msg += ":|cfffaeb00";
        break;
    case SEC_ADMINISTRATOR:
        msg += "|cff00ff00[World]";
        msg += "|cffFF8C00[Head GM]";
        msg += " |TINTERFACE/CHATFRAME/UI-CHATICON-BLIZZ:15|t|r ";
        msg += GetNameLink(player);
        msg += ":|cfffaeb00";
        break;
    case SEC_CONSOLE:
        msg += "|cff00ff00[World]";
        msg += "|cffFF0000[ConSOle]";
        msg += " |TINTERFACE/CHATFRAME/UI-CHATICON-BLIZZ:15|t|r ";
        msg += GetNameLink(player);
        msg += ":|cfffaeb00";
        break;
    }

    chat_string << msg << " " << message;

    char c_msg[1024];

    snprintf(c_msg, 1024, chat_string.str().c_str());

    if (FACTION_SPECIFIC)
    {
        SessionMap sessions = sWorld->GetAllSessions();
        for (SessionMap::iterator itr = sessions.begin(); itr != sessions.end(); ++itr)
            if (Player* plr = itr->second->GetPlayer())
                if (plr->GetTeam() == player->GetTeam())
                    sWorld->SendServerMessage(SERVER_MSG_STRING, msg.c_str(), plr);
    }
    else
        sWorld->SendGlobalText(c_msg, NULL);
}

class cs_world_chat : public CommandScript
{
public:
    cs_world_chat() : CommandScript("cs_world_chat") {}

    static bool HandleWorldChatCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Player* player = handler->GetSession()->GetPlayer();

        _SendWorldChat(handler->GetSession()->GetPlayer(), args);
        // add mute time for stop spam
        LoginDatabasePreparedStatement* mt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_MUTE_TIME);
        int64 muteTime = time(NULL) + mutetimeantispam;
        player->GetSession()->m_muteTime = muteTime;
        mt->setInt64(0, muteTime);
        return true;
    }
    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> cs_world_chat =
        {
            {"chat", rbac::RBAC_PERM_COMMAND_SAVE, true, &HandleWorldChatCommand, "" },
            {"c", rbac::RBAC_PERM_COMMAND_SAVE, true, &HandleWorldChatCommand, "" },
            {"world", rbac::RBAC_PERM_COMMAND_SAVE, true, &HandleWorldChatCommand, "" },
        };

        return cs_world_chat;
    }
};

void AddCustom_playerscript()
{
    RegisterGameObjectAI(go_shadowlands_covenant_map);
    new PlayerScript_WeeklyQuests();
    RegisterPlayerScript(player_level_rewards);
    RegisterPlayerScript(Boss_Announcer);
    RegisterPlayerScript(DemonHunterTalentTempFix); // TEMP FIX! remove when playerchoice has been fixed properly.
    RegisterPlayerScript(OnWodGarrisonArrival);
    RegisterPlayerScript(CloseGossipWindowHackFix);
    new ps_sl_CreateGarrison();
    new DragonflightArrival();
    new System_Taberna();
    new PromotionNewPlayers();
    new AccountAchievements();
    new PlayerScriptPetBattle;
    new LearnRiding();
    new cs_world_chat();
}
