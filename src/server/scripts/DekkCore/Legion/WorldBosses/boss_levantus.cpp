/*
* Copyright 2021
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
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"
#include "ScriptedFollowerAI.h"
#include "Player.h"
#include "SpellInfo.h"
#include "WorldSession.h"
#include "SpellScript.h"
#include "MoveSpline.h"
#include "MoveSplineFlag.h"
#include "MoveSplineInit.h"
#include "Conversation.h"
#include "Group.h"
#include "GroupMgr.h"
#include "Vehicle.h"
#include "GridNotifiersImpl.h"
#include "AreaTriggerAI.h"
#include "AreaTriggerPackets.h"
#include "CellImpl.h"

Position const Waypointspawn[6] =
{
 { -1110.553f, 6949.951f, 0.0006f },
 { -1082.276f, 7001.35f, 0.0006f },
 { -1131.501f, 7029.581f, 0.0006f },
 { -1175.358f, 6998.841f, 0.0006f },
 { -1173.099f, 6950.416f, 0.0006f },
 { -1106.778f, 6947.764f, 0.0006f },
};

class boss_levantus : public CreatureScript
{
public:
    boss_levantus() : CreatureScript("boss_levantus") { }

    struct boss_levantusAI : ScriptedAI
    {
        boss_levantusAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        EventMap _events;

        void Reset() override
        {
            me->SetControlled(true, UNIT_STATE_ROOT);
            me->SetHealth(me->GetMaxHealth());
            me->GetMotionMaster()->MoveTargetedHome();

            _events.Reset();
        }

        void JustEngagedWith(Unit* who) override
        {
            _events.ScheduleEvent(1, 5s);
            _events.ScheduleEvent(2, 8s);
            _events.ScheduleEvent(4, 35s);
            _events.ScheduleEvent(5, 15s);
            _events.ScheduleEvent(6, 25s);
        }

        void EnterEvadeMode(EvadeReason /*why*/) override
        {
            me->SetControlled(false, UNIT_STATE_ROOT);
            me->GetMotionMaster()->MoveTargetedHome();
            Reset();

            _EnterEvadeMode();
        }

        void JustDied(Unit* killer) override
        {
        }

        void UpdateAI(uint32 diff)
        {
            _events.Update(diff);

            if (!UpdateVictim())
                return;

            if (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                case 1:
                {
                    for (uint32 i = 1; i < 7; i++) // +1
                    {
                        me->CastSpell(Position(Waypointspawn[i].GetPositionX() + urand(0, 25), Waypointspawn[i].GetPositionY() + urand(0, 25), Waypointspawn[i].GetPositionZ()), 217212, true);
                    }

                    _events.ScheduleEvent(1, 10s);
                    break;
                }
                case 2:
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random))
                        me->CastSpell(target, 217361, true);
                    _events.ScheduleEvent(2, 5s);
                    break;
                case 3:
                    if (Unit* target = SelectTarget(SelectTargetMethod::MaxDistance))
                        me->CastSpell(target, 217229, true);
                    break;
                case 4:
                    if (Unit* target = SelectTarget(SelectTargetMethod::MinDistance))
                    {
                        if (target->GetDistance(me) < 30)
                            me->CastSpell(target, 217249, true); // todo
                        else
                            _events.ScheduleEvent(3, 1s);
                    }
                    _events.ScheduleEvent(4, 60s);
                    break;
                case 5:
                {
                    DoCast(217344);
                    _events.ScheduleEvent(5, 30s);
                    break;
                }
                case 6:
                {
                    DoCast(217235);
                    _events.ScheduleEvent(6, 30s);
                    break;
                }
                default:
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_levantusAI(creature);
    }
};

void AddSC_boss_levantus()
{
    new boss_levantus();
}
