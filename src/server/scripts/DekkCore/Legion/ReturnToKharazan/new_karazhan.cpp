/*
 * Copyright 2021 DEKKCORE
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

#include "ScriptMgr.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "SpellAuras.h"
#include "ScriptedGossip.h"
#include "ScriptedCreature.h"
#include "new_karazhan.h"
#include "DB2Stores.h"

constexpr uint32 SPELL_MEDIVH_ECHO      = 229074;
constexpr uint32 SPELL_MEDIVH_PRESSENCE = 229077;
constexpr uint32 NIGHTBANE_ACHIEVEMENT   = 11430;

enum Actions
{
    ACTION_SUMMON_NIGHTBANE     = 1,
    ACTION_REPLY_ARCHANAGOS     = 2,
    ACTION_REPLY_2_ARCHANAGOS   = 3,
    ACTION_REPLY_3_ARCHANAGOS   = 4,
};

class go_door_entrance : public GameObjectScript
{
    public:
        go_door_entrance() : GameObjectScript("go_door_entrance")
        {}

        struct go_door_entranceAI : public GameObjectAI
        {
            go_door_entranceAI(GameObject* go) : GameObjectAI(go) { }

            bool OnGossipHello(Player* /*player*/) override
            {
                if (!me)
                    return false;

                if (me->GetInstanceScript()->GetData(DATA_NIGHTBANE_EVENT) == NOT_STARTED)
                    me->GetInstanceScript()->SetData(DATA_NIGHTBANE_EVENT, IN_PROGRESS);

                if (me->GetGoState() == GO_STATE_ACTIVE)
                    me->SetGoState(GO_STATE_READY);
                else
                    me->SetGoState(GO_STATE_ACTIVE);

                return true;
            }
        };

        GameObjectAI* GetAI(GameObject* go) const override
        {
            return new go_door_entranceAI(go);
        }
};

class npc_kara_soul_fragment : public CreatureScript
{
    public:
        npc_kara_soul_fragment() : CreatureScript("npc_kara_soul_fragment")
        {}

        struct npc_kara_soul_fragment_AI : public ScriptedAI
        {
            explicit npc_kara_soul_fragment_AI(Creature* me) : ScriptedAI(me)
            {}

            void OnSpellClick(Unit* /*clicker*/, bool /*result*/)
            {
                for (auto & it : me->GetMap()->GetPlayers())
                {
                    if (Player* player = it.GetSource())
                    {
                        if (Aura* echo = player->GetAura(SPELL_MEDIVH_ECHO))
                        {
                            if (echo->GetStackAmount() == 4)
                                player->CastSpell(player, SPELL_MEDIVH_PRESSENCE, true);
                            else
                            {
                                echo->ModStackAmount(1);
                                echo->SetDuration(echo->GetDuration() + echo->GetMaxDuration());
                            }
                        }
                        else
                            DoCast(player, SPELL_MEDIVH_ECHO, true);
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* me) const override
        {
            return new npc_kara_soul_fragment_AI(me);
        }
};

void AddSC_new_karazhan()
{
    new npc_kara_soul_fragment();
    new go_door_entrance();
};
