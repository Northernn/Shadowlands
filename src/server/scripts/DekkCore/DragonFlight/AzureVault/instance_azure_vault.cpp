/*
 * This file is part of the DekkCore Project. See AUTHORS file for Copyright information
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

#include "AreaBoundary.h"
#include "InstanceScript.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "MotionMaster.h"
#include "GenericMovementGenerator.h"
#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "azure_vault.h"

BossBoundaryData const boundaries =
{
    { DATA_LEYMOR, new CircleBoundary(Position(-5129.39f, 1253.30f), 75.0f) }
};

ObjectData const creatureData[] =
{
    { BOSS_LEYMOR,              DATA_LEYMOR             },
    { BOSS_AZUREBLADE,          DATA_AZUREBLADE         },
    { BOSS_TELASH_GREYWING,     DATA_TELASH_GREYWING    },
    { BOSS_UMBRELSKUL,          DATA_UMBRELSKUL         },
    { 0,                        0                       }  // END
};

DoorData const doorData[] =
{
    { GO_ARCANE_VAULTS_DOOR_LEYMOR_EXIT,        DATA_LEYMOR,  DOOR_TYPE_PASSAGE },
    { GO_ARCANE_VAULTS_DOOR_LEYMOR_ENTRANCE,    DATA_LEYMOR,  DOOR_TYPE_ROOM    },
    { 0,                                        0,            DOOR_TYPE_ROOM    }  // END
};

DungeonEncounterData const encounters[] =
{
    { DATA_LEYMOR,          {{ 2582 }} },
    { DATA_AZUREBLADE,      {{ 2585 }} },
    { DATA_TELASH_GREYWING, {{ 2583 }} },
    { DATA_UMBRELSKUL,      {{ 2584 }} },
};

class instance_azure_vault : public InstanceMapScript
{
    public:
        instance_azure_vault() : InstanceMapScript(AVScriptName, 2515) { }

        struct instance_azure_vault_InstanceMapScript: public InstanceScript
        {
            instance_azure_vault_InstanceMapScript(InstanceMap* map) : InstanceScript(map), _leymorIntroDone(false)
            {
                SetHeaders(DataHeader);
                SetBossNumber(EncounterCount);
                LoadObjectData(creatureData, nullptr);
                LoadDoorData(doorData);
                LoadBossBoundaries(boundaries);
                LoadDungeonEncounterData(encounters);
            }

            bool HandleGetStartPosition(Position& entrancePosition) const override
            {
                entrancePosition.Relocate(-5049.214844f, 1023.291565f, 594.216003f, 2.378918f);
                return true;
            }

            uint32 GetData(uint32 dataId) const override
            {
                switch (dataId)
                {
                    case DATA_LEYMOR_INTRO_DONE:
                        return _leymorIntroDone ? 1 : 0;
                    default:
                        break;
                }
                return 0;
            }

            void SetData(uint32 dataId, uint32 /*value*/) override
            {
                switch (dataId)
                {
                    case DATA_LEYMOR_INTRO_DONE:
                        _leymorIntroDone = true;
                        break;
                    default:
                        break;
                }
            }

        private:
            bool _leymorIntroDone;
        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const override
        {
            return new instance_azure_vault_InstanceMapScript(map);
        }
};

/// NPC: Sindragosa - 197081
struct npc_sindragosa_197081 : public ScriptedAI
{
public:
    npc_sindragosa_197081(Creature* creature) : ScriptedAI(creature) { }

    enum eSindragosa
    {
        PortalToAzureBoss01 = 393902,
        PortalToAzureBoss02 = 389770,
        PortalToAzureBoss03 = 389773,
    };

    bool OnGossipHello(Player* player) override
    {
        ClearGossipMenuFor(player);

        auto instance = me->GetInstanceScript();

        bool leymorDone = instance && instance->GetBossState(DATA_LEYMOR) == DONE;
        bool azurebladeDone = instance && instance->GetBossState(DATA_AZUREBLADE) == DONE;
        bool telashDone = instance && instance->GetBossState(DATA_TELASH_GREYWING) == DONE;

        if (leymorDone || player->IsGameMaster())
        {
            AddGossipItemFor(player, GossipOptionNpc::None, "Take me to the Arcane Conservatory.", GOSSIP_SENDER_MAIN, 0);
            {
                player->CastSpell(player, eSindragosa::PortalToAzureBoss01, true);
                CloseGossipMenuFor(player);
            };
        }
        if (azurebladeDone || player->IsGameMaster())
        {
            AddGossipItemFor(player, GossipOptionNpc::None, "Take me to the Mausoleum of Legends.", GOSSIP_SENDER_MAIN, 1);
            {
                player->CastSpell(player, eSindragosa::PortalToAzureBoss02, true);
                CloseGossipMenuFor(player);
            };
        }
        if (telashDone || player->IsGameMaster())
        {
            AddGossipItemFor(player, GossipOptionNpc::None, "Take me to the Crystal Chambers.", GOSSIP_SENDER_MAIN, 2);
            {
                player->CastSpell(player, eSindragosa::PortalToAzureBoss03, true);
                CloseGossipMenuFor(player);
            };
        }

        SendGossipMenuFor(player, 590032, me); ///< Sindragosa TextID - Verified 48069
        return true;
    }
};

/// NPC - Polymorphic Rune - 195638
struct npc_polymorphic_rune_195638 : public ScriptedAI
{
public:
    npc_polymorphic_rune_195638(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
    }

    enum ePolymorphicRune
    {
        Polymorphed = 386368,
    };

    void UpdateAI(uint32 /*diff*/) override
    {
        if (_didPolymorph && !me->HasUnitState(UNIT_STATE_CASTING))
        {
            me->DespawnOrUnsummon();
        }
    }

    // @todo - handle it by 26445  areatrigger
    void OnUnitRelocation(Unit* who) override
    {
        if (_didPolymorph)
            return;

        if (!who->IsPlayer())
            return;

        if (who->GetDistance2d(me) > 0.66f)
            return;

        me->CastSpell(me, Polymorphed);
        _didPolymorph = true;
    }

    bool _didPolymorph = false;
};

Position const BookOfTranslocationPathOne[4] =
{
    { -5303.6753f, 1191.0173f, 515.0979f },
    { -5311.243f, 1167.6875f, 522.0624f },
    { -5312.632f, 1139.7882f, 519.5894f },
    { -5312.578f, 1119.7812f, 509.12387f },
};

using BookOfTranslocationPathOneSize = std::extent<decltype(BookOfTranslocationPathOne)>;

// NPC - Book of Translocation - 194602
struct npc_book_of_translocation_194602 : public ScriptedAI
{
public:
    npc_book_of_translocation_194602(Creature* creature) : ScriptedAI(creature) { }

    enum eBookOfTranslocation
    {
        Translocate = 384106, ///< Already has interrupt flags if they logout while on path.
    };

    bool OnGossipHello(Player* player) override
    {
        ClearGossipMenuFor(player);

        AddGossipItemFor(player, GossipOptionNpc::None, "Proceed onward.", GOSSIP_SENDER_MAIN, 0);
        {
            // cast 384106

            // (MovementMonsterSpline)(MovementSpline)Flags: 6294016 (Flying, Catmullrom, CanSwim, UncompressedPath)

            //Points : X : -5303.6753f, 1191.0173f, 515.0979 f
            //Points : X : -5311.243 f, 1167.6875f, 522.0624 f
            //Points : X : -5312.632 f, 1139.7882f, 519.5894 f
            //Points : X : -5312.578 f, 1119.7812f, 509.12387f

            player->CastSpell(player, eBookOfTranslocation::Translocate, true);

            player->GetMotionMaster()->MoveSmoothPath(1, BookOfTranslocationPathOne, BookOfTranslocationPathOneSize::value, false, 22.0f);
                {
                    player->RemoveAurasDueToSpell(eBookOfTranslocation::Translocate);
                };
        };

        SendGossipMenuFor(player, 590026, me); ///< Verified 48069
        return true;
    }
};

Position const BookOfTranslocationPathTwo[9] =
{
    { -5337.512f, 1067.356f, 508.03104f },
    { -5337.4165f, 1087.4584f, 488.28918f },
    { -5319.3403f, 1089.3959f, 467.1379f },
    { -5300.6685f, 1069.1389f, 448.19083f },
    { -5310.837f, 1042.0209f, 441.6145f },
    { -5351.0557f, 1030.3785f, 437.05338f },
    { -5377.0747f, 1059.2344f, 435.65186f },
    { -5363.7954f, 1107.0173f, 428.52472f },
    { -5345.809f, 1120.3125f, 425.16507f }
};

using BookOfTranslocationPathTwoSize = std::extent<decltype(BookOfTranslocationPathTwo)>;

struct npc_book_of_translocation_194712 : public ScriptedAI
{
public:
    npc_book_of_translocation_194712(Creature* creature) : ScriptedAI(creature) { }

    enum eBookOfTranslocation
    {
        Translocate = 384347, ///< Already has interrupt flags if they logout while on path.
    };

    bool OnGossipHello(Player* player) override
    {
        ClearGossipMenuFor(player);

        AddGossipItemFor(player, GossipOptionNpc::None, "Proceed onward.", GOSSIP_SENDER_MAIN, 0);
        {
            player->CastSpell(player, eBookOfTranslocation::Translocate, true);

            player->GetMotionMaster()->MoveSmoothPath(1, BookOfTranslocationPathTwo, BookOfTranslocationPathTwoSize::value, false, 22.0f);
                {
                    player->RemoveAurasDueToSpell(eBookOfTranslocation::Translocate);
                };
        };

        SendGossipMenuFor(player, 590027, me); ///< Verified 48069
        return true;
    }
};

Position const BookOfTranslocationPathThree[4] =
{
    { -5337.212f, 1009.4636f, 430.3666f },
    { -5344.2397f, 985.07294f, 437.19678f },
    { -5344.9204f, 964.0278f, 435.74966f },
    { -5337.5415f, 947.16144f, 430.3666f }
};

using BookOfTranslocationPathThreeSize = std::extent<decltype(BookOfTranslocationPathThree)>;

struct npc_book_of_translocation_194713 : public ScriptedAI
{
public:
    npc_book_of_translocation_194713(Creature* creature) : ScriptedAI(creature) { }

    enum eBookOfTranslocation
    {
        Translocate = 384348, ///< Already has interrupt flags if they logout while on path.
    };

    bool OnGossipHello(Player* player) override
    {
        ClearGossipMenuFor(player);

        AddGossipItemFor(player, GossipOptionNpc::None, "Proceed onward.", GOSSIP_SENDER_MAIN, 0);
        {
            player->CastSpell(player, eBookOfTranslocation::Translocate, true);

            player->GetMotionMaster()->MoveSmoothPath(1, BookOfTranslocationPathThree, BookOfTranslocationPathThreeSize::value, false, 22.0f);
                {
                    player->RemoveAurasDueToSpell(eBookOfTranslocation::Translocate);
                };
        };

        SendGossipMenuFor(player, 590028, me); ///< Verified 48069
        return true;
    }
};

Position const BookOfTranslocationPathFour[4] =
{
    { -5413.002f, 991.00696f, 346.43228f },
    { -5405.328f, 1008.5521f, 351.44327f },
    { -5383.4272f, 1032.1459f, 348.3228f },
    { -5366.299f, 1040.7935f, 341.58167f }
};

using BookOfTranslocationPathFourSize = std::extent<decltype(BookOfTranslocationPathFour)>;

struct npc_book_of_translocation_194714 : public ScriptedAI
{
public:
    npc_book_of_translocation_194714(Creature* creature) : ScriptedAI(creature) { }

    enum eBookOfTranslocation
    {
        Translocate = 384349, ///< Already has interrupt flags if they logout while on path.
    };

    bool OnGossipHello(Player* player) override
    {
        ClearGossipMenuFor(player);

        AddGossipItemFor(player, GossipOptionNpc::None, "Proceed onward.", GOSSIP_SENDER_MAIN, 0);
        {
            player->CastSpell(player, eBookOfTranslocation::Translocate, true);

            player->GetMotionMaster()->MoveSmoothPath(1, BookOfTranslocationPathFour, BookOfTranslocationPathFourSize::value, false, 22.0f);
                {
                    player->RemoveAurasDueToSpell(eBookOfTranslocation::Translocate);
                };
        };

        SendGossipMenuFor(player, 590029, me); ///< Verified 48069
        return true;
    }
};

Position const BookOfTranslocationPathFive[13] =
{
    { -5336.8003f, 1068.0348f, 346.8186f },
    { -5345.0747f, 1077.6216f, 322.03543f },
    { -5345.3975f, 1047.8281f, 263.0053f },
    { -5327.186f, 1048.0868f, 223.00484f },
    { -5314.4307f, 1063.5504f, 188.28706f },
    { -5314.915f, 1086.5173f, 176.62357f },
    { -5327.868f, 1112.7517f, 175.95555f },
    { -5357.9497f, 1122.1423f, 175.95555f },
    { -5388.0537f, 1098.1841f, 175.95555f },
    { -5393.3613f, 1043.4497f, 175.95555f },
    { -5355.752f, 1009.5226f, 175.95555f },
    { -5321.3335f, 1022.658f, 162.84392f },
    { -5329.5415f, 1044.618f, 160.78316f }
};

using BookOfTranslocationPathFiveSize = std::extent<decltype(BookOfTranslocationPathFive)>;

struct npc_book_of_translocation_194715 : public ScriptedAI
{
public:
    npc_book_of_translocation_194715(Creature* creature) : ScriptedAI(creature) { }

    enum eBookOfTranslocation
    {
        Translocate = 384350, ///< Already has interrupt flags if they logout while on path.
    };

    bool OnGossipHello(Player* player) override
    {
        ClearGossipMenuFor(player);

        AddGossipItemFor(player, GossipOptionNpc::None, "Proceed onward.", GOSSIP_SENDER_MAIN, 0);
        {
            player->CastSpell(player, eBookOfTranslocation::Translocate, true);

            player->GetMotionMaster()->MoveSmoothPath(1, BookOfTranslocationPathFive, BookOfTranslocationPathFiveSize::value, false, 22.0f);
                {
                    player->RemoveAurasDueToSpell(eBookOfTranslocation::Translocate);
                };
        };

        SendGossipMenuFor(player, 590030, me); ///< Verified 48069
        return true;
    }
};

/// Arcane Elemental - 186741
struct npc_arcane_elemental_186741 : public ScriptedAI
{
public:
    npc_arcane_elemental_186741(Creature* creature) : ScriptedAI(creature) { }

    enum eArcaneElementals
    {
        ArcaneBolt = 371306,
        WakingBane = 386546,
    };

    void Reset() override
    {
        scheduler.CancelAll();
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        scheduler.Schedule(Seconds(1), [this](TaskContext context)
            {
                DoCastVictim(ArcaneBolt);

                context.Repeat(5s);
            });

        scheduler.Schedule(Seconds(3), [this](TaskContext context)
            {
                if (me->HasUnitState(UNIT_STATE_CASTING))
                {
                    context.Repeat(1ms);
                    return;
                }

                DoCastRandom(WakingBane, 40.0f);

                context.Repeat(20s);
            });
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        scheduler.Update(diff);

        DoMeleeAttackIfReady();
    }

    TaskScheduler scheduler;
};

/// Rune Seal Keeper - 187155
struct npc_rune_seal_keeper_187155 : public ScriptedAI
{
public:
    npc_rune_seal_keeper_187155(Creature* creature) : ScriptedAI(creature) { }

    enum eRuneSealKeeper
    {
        CondensedFrost = 377503,
        IcyBindings = 377488,
    };

    void Reset() override
    {
        scheduler.CancelAll();
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        scheduler.Schedule(3s, 5s, [this](TaskContext context)
            {
                DoCastVictim(CondensedFrost);
                context.Repeat(15s);
            });
        scheduler.Schedule(8s, 15s, [this](TaskContext context)
            {
                DoCastVictim(IcyBindings);
                context.Repeat(15s);
            });
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (!me->HasUnitState(UNIT_STATE_CASTING))
            scheduler.Update(diff);

        DoMeleeAttackIfReady();
    }

    TaskScheduler scheduler;
};

void AddSC_instance_azure_vault()
{
    new instance_azure_vault();
    RegisterCreatureAI(npc_sindragosa_197081);
    RegisterCreatureAI(npc_polymorphic_rune_195638);
    RegisterCreatureAI(npc_book_of_translocation_194602);
    RegisterCreatureAI(npc_book_of_translocation_194712);
    RegisterCreatureAI(npc_book_of_translocation_194713);
    RegisterCreatureAI(npc_book_of_translocation_194714);
    RegisterCreatureAI(npc_book_of_translocation_194715);
    RegisterCreatureAI(npc_arcane_elemental_186741);
    RegisterCreatureAI(npc_rune_seal_keeper_187155);
}
