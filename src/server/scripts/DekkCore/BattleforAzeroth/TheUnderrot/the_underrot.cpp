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

#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "the_underrot.h"

// 138281
struct npc_underrot_faceless_corruptor : public ScriptedAI
{
    npc_underrot_faceless_corruptor(Creature* creature) : ScriptedAI(creature) { }

    void JustDied(Unit* /*killer*/) override
    {
        if (InstanceScript* instanceScript = me->GetInstanceScript())
        {
           // ScriptParam param = me->GetScriptParam(1);

          //  if (param.numericValue == 1)
                instanceScript->SetData(DATA_FACELESS_CORRUPTOR_1, 1);
         //   else if (param.numericValue == 2)
                instanceScript->SetData(DATA_FACELESS_CORRUPTOR_2, 1);
        }
    }
};

// 279271
class aura_underrot_open_web_door : public AuraScript
{
    void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (InstanceScript* instanceScript = GetTarget()->GetInstanceScript())
            instanceScript->SetData(DATA_EVENT_HERZEL, DONE);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(aura_underrot_open_web_door::HandleEffectRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_the_underrot()
{
    RegisterCreatureAI(npc_underrot_faceless_corruptor);
    RegisterSpellScript(aura_underrot_open_web_door);
}
