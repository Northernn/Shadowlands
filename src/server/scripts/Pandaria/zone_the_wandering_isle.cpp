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

#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "TaskScheduler.h"

enum TraineeMisc
{
    SAY_FINISH_FIGHT                    = 0,

    SPELL_BLACKOUT_KICK                 = 109080,

    QUEST_29524_KILLCREDIT              = 54586,

    POINT_DESPAWN                       = 0,

    NPC_HUOJIN_TRAINEE_MALE             = 54586,
    NPC_HUOJIN_TRAINEE_FEMALE           = 65470,
    NPC_TUSHUI_TRAINEE_MALE             = 54587,
    NPC_TUSHUI_TRAINEE_FEMALE           = 65471,
};

Position const TraineeEndpoints[] = {
    { 1465.3872f, 3283.8604f, 137.69096f },
    { 1431.401f, 3264.001f, 136.02579f },
    { 1397.2067f, 3276.5618f, 133.84508f },
    { 1441.566f, 3232.8013f, 135.01802f },
    { 1403.632f, 3229.1094f, 132.14877f },
    { 1347.1927f, 3286.5842f, 131.94803f },
    { 1365.1865f, 3338.9502f, 128.57233f },
    { 1349.6024f, 3315.0574f, 130.97443f },
    { 1335.4618f, 3344.019f, 130.42047f },
    { 1360.1198f, 3378.02f, 127.34183f },
    { 1435.8524f, 3355.6423f, 173.77744f },
    { 1432.7031f, 3385.1572f, 184.4187f },
    { 1452.6094f, 3373.3315f, 187.0402f },
    { 1426.7778f, 3364.7517f, 184.39569f },
    { 1450.3646f, 3361.264f, 184.42484f },
};

Emote constexpr TraineeEmotes[5] =
{
    EMOTE_ONESHOT_MONKOFFENSE_ATTACKUNARMED,
    EMOTE_ONESHOT_MONKOFFENSE_SPECIALUNARMED,
    EMOTE_ONESHOT_MONKOFFENSE_PARRYUNARMED,
    EMOTE_ONESHOT_PALMSTRIKE,
    EMOTE_ONESHOT_MONKOFFENSE_ATTACKUNARMEDOFF,
};

class TushuiTraineeSearch
{
public:
    TushuiTraineeSearch(Creature* leader, float maxDist) : _leader(leader), _maxDist(maxDist) { }

    bool operator()(Creature const* target) const
    {
        if (target->GetEntry() != NPC_TUSHUI_TRAINEE_MALE && target->GetEntry() != NPC_TUSHUI_TRAINEE_FEMALE)
            return false;
        if (target->IsInCombat())
            return false;
        if (target->IsInEvadeMode())
            return false;
        if (target->GetDistance(_leader) >= _maxDist)
            return false;
        if (target->isDead())
            return false;

        return true;
    }

private:
    Creature* _leader;
    float _maxDist;
};

void HandleEmoteNearbyTushuiTrainees(Creature* leader, Emote emote)
{
    std::list<Creature*> traineeList;
    TushuiTraineeSearch check(leader, 10.0f);
    Trinity::CreatureListSearcher<TushuiTraineeSearch> searcher(leader, traineeList, check);
    Cell::VisitGridObjects(leader, searcher, 10.0f);

    for (Creature* trainee : traineeList)
        trainee->HandleEmoteCommand(emote);
}

// 61411 - Instructor Zhi
struct npc_instructor_zhi : public ScriptedAI
{
    npc_instructor_zhi(Creature* creature) : ScriptedAI(creature) { }

    void JustAppeared() override
    {
        _scheduler.Schedule(Seconds(6), [this](TaskContext task)
        {
            Emote emote = Trinity::Containers::SelectRandomContainerElement(TraineeEmotes);
            me->HandleEmoteCommand(emote);

            task.Schedule(Seconds(1), [this, emote](TaskContext /*task*/)
            {
                HandleEmoteNearbyTushuiTrainees(me, emote);
            });
            task.Repeat(Seconds(6));
        });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
};

void AddSC_zone_the_wandering_isle()
{
    RegisterCreatureAI(npc_instructor_zhi);
}
