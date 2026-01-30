/*
Copyright 2023 Fluxurion
 */

#include "ScriptMgr.h"
#include "CellImpl.h"
#include "CombatAI.h"
#include "CreatureTextMgr.h"
#include "GameEventMgr.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "GridNotifiersImpl.h"
#include "Log.h"
#include "MotionMaster.h"
#include "MoveSplineInit.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "PassiveAI.h"
#include "Player.h"
#include "QuestDef.h"
#include "ScriptedEscortAI.h"
#include "ScriptedGossip.h"
#include "SpellAuras.h"
#include "SpellHistory.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Unit.h"

struct npc_training_dummy_healing : NullCreatureAI
{
    npc_training_dummy_healing(Creature* creature) : NullCreatureAI(creature) { }

    uint32 m_ResetTimer;

    void Reset() override
    {
        m_ResetTimer = 0;

        if (!me->IsAlive())
            me->Respawn(true);

        me->SetHealth(me->GetCreateHealth() * 0.3f);
        me->SetRegenerateHealth(false);
        me->AddUnitState(UnitState::UNIT_STATE_STUNNED);
    }

    void HealReceived(Unit* /*p_Healer*/, uint32& /*p_HealAmount*/) override
    {
        m_ResetTimer = 20 * TimeConstants::IN_MILLISECONDS;
    }

    void UpdateAI(uint32 const p_Diff) override
    {
        if (m_ResetTimer)
        {
            if (m_ResetTimer <= p_Diff)
                Reset();
            else
                m_ResetTimer -= p_Diff;
        }
    }
};

void AddSC_npc_training_dummy_healing()
{
    RegisterCreatureAI(npc_training_dummy_healing);
};
