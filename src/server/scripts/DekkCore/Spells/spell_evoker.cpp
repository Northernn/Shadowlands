/*
 * Copyright 2023 DekkCore
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

#include "Containers.h"
#include "Log.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellHistory.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "Player.h"
#include "TemporarySummon.h"
#include "Unit.h"

enum EvokerSpells
{
    SPELL_EVOKER_ENERGIZING_FLAME = 400006,
    SPELL_EVOKER_GLIDE_KNOCKBACK = 358736,
    SPELL_EVOKER_HOVER = 358267,
    SPELL_EVOKER_LIVING_FLAME = 361469,
    SPELL_EVOKER_LIVING_FLAME_DAMAGE = 361500,
    SPELL_EVOKER_LIVING_FLAME_HEAL = 361509,
    SPELL_EVOKER_SOAR_RACIAL = 369536,
    SPELL_SKYWARD_ASCENT = 367033,
    SPELL_SURGE_FORWARD = 369541,
    SPELL_EVOKER_PYRE_DAMAGE = 357212,
    SPELL_EVOKER_PERMEATING_CHILL_TALENT = 370897,
    SPELL_EVOKER_DREAM_BREATH_3EMPO = 355936,
    SPELL_EVOKER_DREAM_BREATH_5EMPO = 382614,
    SPELL_EVOKER_DREAM_BREATH_HEAL_3EMPO = 355941,
    SPELL_EVOKER_DREAM_BREATH_HEAL_5EMPO = 376788,
    SPELL_EVOKER_SPIRITBLOOM_3EMPO = 367226,
    SPELL_EVOKER_SPIRITBLOOM_5EMPO = 382731,
    SPELL_EVOKER_SPIRITBLOOM_HEAL_MAIN_TARGET = 367230,
    SPELL_EVOKER_SPIRITBLOOM_HEAL_OTHER_TARGET = 367231,
    SPELL_EVOKER_FIRE_BREATH_3EMPO = 357208,
    SPELL_EVOKER_FIRE_BREATH_5EMPO = 382266,
    SPELL_EVOKER_FIRE_BREATH_DMG = 357209,
    SPELL_EVOKER_ETERNITY_SURGE_3EMPO = 359073,
    SPELL_EVOKER_ETERNITY_SURGE_5EMPO = 382411,
    SPELL_EVOKER_ETERNITY_SURGE_DMG = 359077,
    SPELL_EVOKER_UPHEAVAL_3EMPO = 396286,
    SPELL_EVOKER_UPHEAVAL_5EMPO = 408092,
    SPELL_EVOKER_UPHEAVAL_DMG = 396288,
    SPELL_EVOKER_UPHEAVAL_VISUAL_1 = 410295,
    SPELL_EVOKER_UPHEAVAL_VISUAL_2 = 410296,
    SPELL_EVOKER_UPHEAVAL_VISUAL_3 = 410297,
    SPELL_EVOKER_UPHEAVAL_VISUAL_4 = 410298,
    SPELL_EVOKER_LANDSLIDE = 358385,
    SPELL_EVOKER_LANDSLIDE_AT = 363800,
    SPELL_EVOKER_LANDSLIDE_ROOT = 355689,
    SPELL_EVOKER_DEEP_BREATH = 357210,
    SPELL_EVOKER_DEEP_BREATH_DMG = 353759,
    SPELL_EVOKER_DEEP_BREATH_DMG_AURA = 362010,
    SPELL_EVOKER_DEEP_BREATH_LANDING = 362019,
    SPELL_EVOKER_RECALL = 371807,
    SPELL_EVOKER_RECALL_AT = 371816,
    SPELL_EVOKER_RECALL_ENABLE = 371817,
    SPELL_EVOKER_RECALL_EFFECT_HIDE = 371868,
    SPELL_EVOKER_PAST_SELF_CLONE = 363940,
    SPELL_EVOKER_PAST_SELF_TIMEFREEZE = 371869,
};

enum EvokerSpellLabels
{
    SPELL_LABEL_EVOKER_BLUE = 1465,
};

class RaidCheck
{
public:
    explicit RaidCheck(Unit const* caster) : _caster(caster) { }

    bool operator()(WorldObject* obj) const
    {
        if (Unit* target = obj->ToUnit())
            return !_caster->IsInRaidWith(target);

        return true;
    }

private:
    Unit const* _caster;
};

// 358733 - Glide (Racial)
class spell_evo_glide : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_EVOKER_GLIDE_KNOCKBACK, SPELL_EVOKER_HOVER, SPELL_EVOKER_SOAR_RACIAL });
    }

    SpellCastResult CheckCast()
    {
        Unit* caster = GetCaster();

        if (!caster->IsFalling())
            return SPELL_FAILED_NOT_ON_GROUND;

        return SPELL_CAST_OK;
    }

    void HandleCast()
    {
        Player* caster = GetCaster()->ToPlayer();
        if (!caster)
            return;

        caster->CastSpell(caster, SPELL_EVOKER_GLIDE_KNOCKBACK, true);

        caster->GetSpellHistory()->StartCooldown(sSpellMgr->AssertSpellInfo(SPELL_EVOKER_HOVER, GetCastDifficulty()), 0, nullptr, false, 250ms);
        caster->GetSpellHistory()->StartCooldown(sSpellMgr->AssertSpellInfo(SPELL_EVOKER_SOAR_RACIAL, GetCastDifficulty()), 0, nullptr, false, 250ms);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_evo_glide::CheckCast);
        OnCast += SpellCastFn(spell_evo_glide::HandleCast);
    }
};

// 362969 - Azure Strike (blue)
class spell_evo_azure_strike : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Trinity::Containers::RandomResize(targets, GetEffectInfo(EFFECT_0).CalcValue());
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_evo_azure_strike::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
    }
};

// 361469 - Living Flame (Red)
class spell_evo_living_flame : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_EVOKER_LIVING_FLAME_DAMAGE, SPELL_EVOKER_LIVING_FLAME_HEAL });
    }

    void HandleTarget(SpellEffIndex /*effIndex*/)
    {
        if (GetCaster()->IsFriendlyTo(GetHitUnit()))
            GetCaster()->CastSpell(GetHitUnit(), SPELL_EVOKER_LIVING_FLAME_HEAL, true);
        else
            GetCaster()->CastSpell(GetHitUnit(), SPELL_EVOKER_LIVING_FLAME_DAMAGE, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_evo_living_flame::HandleTarget, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 361500 - Living Flame (damage)
class spell_evo_living_flame_damage : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_EVOKER_ENERGIZING_FLAME, SPELL_EVOKER_LIVING_FLAME });
    }

    void HandleManaRestored(SpellEffIndex /*effIndex*/)
    {
        if (AuraEffect* auraEffect = GetCaster()->GetAuraEffect(SPELL_EVOKER_ENERGIZING_FLAME, EFFECT_0))
        {
            SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(SPELL_EVOKER_LIVING_FLAME, GetCastDifficulty());

            Optional<SpellPowerCost> cost = spellInfo->CalcPowerCost(POWER_MANA, false, GetCaster(), GetSpellInfo()->GetSchoolMask(), nullptr);

            if (!cost)
                return;

            int32 manaRestored = CalculatePct(cost->Amount, auraEffect->GetAmount());
            GetCaster()->ModifyPower(POWER_MANA, manaRestored);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_evo_living_flame_damage::HandleManaRestored, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 351239 - Visage (Racial)
class spell_cosmic_evoker_visage : public SpellScript
{
    void HandleOnCast()
    {
        Unit* caster = GetCaster();

        if (caster->HasAura(372014))
        {
            // Dracthyr Form
            caster->RemoveAurasDueToSpell(372014);
            caster->CastSpell(caster, 97709, true);
            caster->SendPlaySpellVisual(caster, 118328, 0, 0, 60, false);
            caster->SetDisplayId(108590);
        }
        else
        {
            // Visage Form
            if (caster->HasAura(97709))
                caster->RemoveAurasDueToSpell(97709);
            caster->CastSpell(caster, 372014, true);
            caster->SendPlaySpellVisual(caster, 118328, 0, 0, 60, false);
            caster->SetDisplayId(104597);
        }
    }

    void Register()
    {
        OnCast += SpellCastFn(spell_cosmic_evoker_visage::HandleOnCast);
    }
};

// 355936, 382614, 355941, 376788 - Dream Breath
class spell_evo_dream_breath : public SpellScript
{
    void HandleOnEmpowerEnd()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        int8 empower = GetSpell()->GetEmpowerReleasedStage();
        if (!empower)
            return;

        float directHealAmount = caster->GetTotalSpellPowerValue(SPELL_SCHOOL_MASK_ALL, false) * 0.768f;
        float baseHotAmount = caster->GetTotalSpellPowerValue(SPELL_SCHOOL_MASK_ALL, false) * 0.384f;
        float hotAmount = baseHotAmount;
        int hotDuration = 0;

        switch (empower)
        {
        case 1:
            hotDuration = 16000;
            break;
        case 2:
            hotDuration = 12000;
            directHealAmount += baseHotAmount * 2;
            break;
        case 3:
            hotDuration = 8000;
            directHealAmount += baseHotAmount * 4;
            break;
        }

        hotAmount = baseHotAmount / (hotDuration / 1000);

        CastSpellExtraArgs args;
        args.SetTriggerFlags(TRIGGERED_FULL_MASK);
        args.AddSpellMod(SPELLVALUE_BASE_POINT0, hotAmount);
        args.AddSpellMod(SPELLVALUE_BASE_POINT2, directHealAmount);
        args.AddSpellMod(SPELLVALUE_DURATION, hotDuration);

        GetSpellInfo()->Id == SPELL_EVOKER_DREAM_BREATH_3EMPO ?
            caster->CastSpell(caster, SPELL_EVOKER_DREAM_BREATH_HEAL_3EMPO, args)
            :
            caster->CastSpell(caster, SPELL_EVOKER_DREAM_BREATH_HEAL_5EMPO, args);
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Trinity::Containers::RandomResize(targets, 5);
    }

    void Register() override
    {
        OnEmpowerEnd += SpellCastFn(spell_evo_dream_breath::HandleOnEmpowerEnd);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_evo_dream_breath::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ALLY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_evo_dream_breath::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_evo_dream_breath::FilterTargets, EFFECT_2, TARGET_UNIT_CONE_ALLY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_evo_dream_breath::FilterTargets, EFFECT_2, TARGET_UNIT_DEST_AREA_ALLY);
    }
};

// 367226, 382731 - Spiritbloom
class spell_evo_spiritbloom : public SpellScript
{
    void HandleOnEmpowerEnd()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        int8 empower = GetSpell()->GetEmpowerReleasedStage();
        if (!empower)
            return;

        Unit* target = ObjectAccessor::GetUnit(*caster, caster->GetTarget());
        if (!target)
            target = GetExplTargetUnit();

        caster->CastSpell(target, SPELL_EVOKER_SPIRITBLOOM_HEAL_MAIN_TARGET, true);

        std::list<Unit*> subTargets;
        target->GetFriendlyUnitListInRange(subTargets, 30.0f);
        subTargets.remove(target);
        Trinity::Containers::RandomResize(subTargets, empower - 1);

        for (auto& subtarget : subTargets)
            caster->CastSpell(subtarget, SPELL_EVOKER_SPIRITBLOOM_HEAL_OTHER_TARGET, true);
    }

    void Register() override
    {
        OnEmpowerEnd += SpellCastFn(spell_evo_spiritbloom::HandleOnEmpowerEnd);
    }
};

// 357208, 382266 - Fire Breath
class spell_evo_fire_breath : public SpellScript
{
    void HandleOnEmpowerEnd()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        int8 empower = GetSpell()->GetEmpowerReleasedStage();
        if (!empower)
            return;

        float baseDirectDamage = caster->GetTotalSpellPowerValue(SPELL_SCHOOL_MASK_ALL, false) * 1.134f;
        float baseDamageOverTime = caster->GetTotalSpellPowerValue(SPELL_SCHOOL_MASK_ALL, false) * 0.274f;
        float finalDirectDamage = baseDirectDamage / 2;
        float finalDamageOverTime = baseDamageOverTime * 10;
        int dotDuration = 0;

        switch (empower)
        {
        case 1:
            dotDuration = 24000;
            break;
        case 2:
            dotDuration = 18000;
            finalDirectDamage = baseDirectDamage * 0.66f;
            break;
        case 3:
            dotDuration = 12000;
            finalDirectDamage = baseDirectDamage * 0.33f;
            break;
        }

        finalDamageOverTime = baseDamageOverTime / (dotDuration / 1000);

        CastSpellExtraArgs args;
        args.SetTriggerFlags(TRIGGERED_FULL_MASK);
        args.AddSpellMod(SPELLVALUE_BASE_POINT0, finalDirectDamage);
        args.AddSpellMod(SPELLVALUE_BASE_POINT1, finalDamageOverTime);
        args.AddSpellMod(SPELLVALUE_DURATION, 24000 - (empower * 6000));

        caster->CastSpell(caster, SPELL_EVOKER_FIRE_BREATH_DMG, args);
    }

    void Register() override
    {
        OnEmpowerEnd += SpellCastFn(spell_evo_fire_breath::HandleOnEmpowerEnd);
    }
};

// 359073, 382411, 359077 - Eternity Surge
class spell_evo_eternity_surge : public SpellScript
{
    void HandleOnEmpowerEnd()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        int8 empower = GetSpell()->GetEmpowerReleasedStage();
        if (!empower)
            return;

        Unit* target = ObjectAccessor::GetUnit(*caster, caster->GetTarget());
        if (!target)
            return;

        caster->CastSpell(target, SPELL_EVOKER_ETERNITY_SURGE_DMG, true);
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Unit* target = ObjectAccessor::GetUnit(*caster, caster->GetTarget());
        if (!target)
            return;

        int8 stage = caster->m_unitData->SpellEmpowerStage;

        targets.remove(target);
        Trinity::Containers::RandomResize(targets, stage);
        targets.push_back(target);
    }

    void Register() override
    {
        OnEmpowerEnd += SpellCastFn(spell_evo_eternity_surge::HandleOnEmpowerEnd);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_evo_eternity_surge::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
    }
};

// 396286, 408092 - Upheaval
class spell_evo_upheaval : public SpellScript
{
    void HandleOnEmpowerEnd()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        int8 empower = GetSpell()->GetEmpowerReleasedStage();
        if (!empower)
            return;

        // I REWRITE LATER THIS IS FOR TEST

        uint32 visualSpellID = SPELL_EVOKER_UPHEAVAL_VISUAL_1;
        switch (empower)
        {
        case 2:
            visualSpellID = SPELL_EVOKER_UPHEAVAL_VISUAL_2;
            break;
        case 3:
            visualSpellID = SPELL_EVOKER_UPHEAVAL_VISUAL_3;
        case 4:
            visualSpellID = SPELL_EVOKER_UPHEAVAL_VISUAL_4;
            break;
        }

        caster->CastSpell(GetExplTargetUnit(), visualSpellID, true);
        caster->CastSpell(GetExplTargetUnit(), SPELL_EVOKER_UPHEAVAL_DMG, true);
    }

    void Register() override
    {
        OnEmpowerEnd += SpellCastFn(spell_evo_upheaval::HandleOnEmpowerEnd);
    }
};

// 393568 - Pyre
class spell_evo_pyre : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_EVOKER_PYRE_DAMAGE });
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetHitUnit()->GetPosition(), SPELL_EVOKER_PYRE_DAMAGE, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_evo_pyre::HandleDamage, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 370455 - Charged Blast
class spell_evo_charged_blast : public AuraScript
{
    bool CheckProc(ProcEventInfo& procInfo)
    {
        return procInfo.GetSpellInfo() && procInfo.GetSpellInfo()->HasLabel(SPELL_LABEL_EVOKER_BLUE);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_evo_charged_blast::CheckProc);
    }
};

// 381773 - Permeating Chill
class spell_evo_permeating_chill : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_EVOKER_PERMEATING_CHILL_TALENT });
    }

    bool CheckProc(ProcEventInfo& procInfo)
    {
        SpellInfo const* spellInfo = procInfo.GetSpellInfo();
        if (!spellInfo)
            return false;

        if (!spellInfo->HasLabel(SPELL_LABEL_EVOKER_BLUE))
            return false;

        if (!procInfo.GetActor()->HasAura(SPELL_EVOKER_PERMEATING_CHILL_TALENT))
            if (!spellInfo->IsAffected(SPELLFAMILY_EVOKER, { 0x40, 0, 0, 0 })) // disintegrate
                return false;

        return true;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_evo_permeating_chill::CheckProc);
    }
};

/// ID: 375088 Dragonrage
class spell_evo_dragonrage : public SpellScript
{
    enum Spells
    {
        SPELL_PYRE = 357211,
    };

    void HandleDummy(SpellEffIndex /*eff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (Unit* target = GetHitUnit())
        {
            caster->CastSpell(target, SPELL_PYRE, true);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_evo_dragonrage::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 358385 - Landslide
class spell_evo_landslide : public SpellScript
{
    class RootEvent : public BasicEvent
    {
    public:
        RootEvent(Unit* caster, Position pos, ObjectGuid originalCastId)
            : _caster(caster), _pos(pos), _originalCastId(originalCastId) { }

        bool Execute(uint64 time, uint32 /*diff*/) override
        {
            _caster->CastSpell(_pos, SPELL_EVOKER_LANDSLIDE_ROOT, CastSpellExtraArgs(TRIGGERED_IGNORE_CAST_IN_PROGRESS).SetOriginalCastId(_originalCastId));
            return true;
        }

    private:
        Unit* _caster;
        Position _pos;
        ObjectGuid _originalCastId;
    };

    void SetDest(SpellDestination& dest)
    {
        Unit* caster = GetCaster();
        Position endPos = dest._position;
        Position curPos = caster->GetPosition();
        float dist = caster->GetDistance(endPos);
        int8 countOfRoot = int8(dist / 4.f);

        for (int8 i = 0; i != countOfRoot; i++)
        {
            curPos = curPos.GetPositionWithDistInOrientation(4.f, caster->GetAngle(endPos));
            Position target = { curPos.GetPositionX(), curPos.GetPositionY(), curPos.GetPositionZ(), caster->GetAngle(endPos) };
            caster->m_Events.AddEvent(new RootEvent(caster, target, GetSpell()->m_castId), caster->m_Events.CalculateTime(Milliseconds(i * 50)));
        }

        //caster->CastSpell(caster, SPELL_EVOKER_LANDSLIDE_AT, true); // this spell is creating it but we need to destroy too
    }

    void Register() override
    {
        OnDestinationTargetSelect += SpellDestinationTargetSelectFn(spell_evo_landslide::SetDest, EFFECT_0, TARGET_DEST_DEST);
    }
};

// 357210 - Deep Breath
class spell_evo_deepbreath : public SpellScript
{
    void HandleBeforeCast()
    {
        Unit* caster = GetCaster();

        const SpellInfo* recallSummon = sSpellMgr->GetSpellInfo(SPELL_EVOKER_RECALL, DIFFICULTY_NONE);

        caster->CastSpell(caster, SPELL_EVOKER_RECALL, true); // summon clone target

        if (Creature* clone = caster->GetSummonedCreatureByEntry(recallSummon->GetEffect(EFFECT_0).MiscValue))
        {
            clone->SetControlled(true, UNIT_STATE_ROOT);
            caster->CastSpell(clone, SPELL_EVOKER_PAST_SELF_CLONE, true); // clone the player to the clone target
            clone->CastSpell(clone, SPELL_EVOKER_PAST_SELF_TIMEFREEZE, true); // golden freeze animation
            clone->SetDisableGravity(true);
            // caster->CastSpell(caster, 371816, true); // spawn areatrigger 25309 (recall)
            //caster->AddAura(SPELL_EVOKER_DEEP_BREATH_DMG_AURA, caster); // why does it removes automatically? well it won't be blizzlike
        }  
    }

    class HitEvent : public BasicEvent
    {
    public:
        HitEvent(Unit* caster, Position endPos, ObjectGuid originalCastId, int8 curPosID, int8 endPosID)
            : _caster(caster), _endPos(endPos), _originalCastId(originalCastId), _curPosID(curPosID), _endPosID(endPosID) { }

        bool Execute(uint64 time, uint32 /*diff*/) override
        {
            CastSpellExtraArgs args;
            args.SetTriggerFlags(TRIGGERED_FULL_MASK);
            args.SetOriginalCaster(_caster->GetGUID());
            args.SetOriginalCastId(_originalCastId);

            if (_curPosID == 3)
                _caster->SendPlaySpellVisualKit(144481, 2, 5000);

            if (_curPosID > 2)
            {
                args.SetTriggerFlags(TRIGGERED_IGNORE_CASTER_AURASTATE);
                _caster->CastSpell(_caster->GetPosition(), SPELL_EVOKER_DEEP_BREATH_DMG, args);
            }

            if (_curPosID == _endPosID - 1)
            {
                if (_caster->HasAura(SPELL_EVOKER_DEEP_BREATH))
                    _caster->RemoveAura(SPELL_EVOKER_DEEP_BREATH);

                _caster->SendCancelSpellVisualKit(144481);
                _caster->SetDisableGravity(false);
                _caster->GetMotionMaster()->Clear();
                _caster->CastSpell(_endPos, SPELL_EVOKER_DEEP_BREATH_LANDING, args);
            }

            return true;
        }

    private:
        Unit* _caster;
        Position _endPos;
        ObjectGuid _originalCastId;
        int8 _curPosID;
        int8 _endPosID;
    };

    void SetDest(SpellDestination& dest)
    {
        Unit* caster = GetCaster();
        Position endPos = dest._position;
        float dist = caster->GetDistance(endPos);
        float angle = caster->GetAngle(endPos);
        int8 countOfHit = int8(dist / 2.4f);

        for (int8 i = 0; i != countOfHit; i++)
        {
            caster->m_Events.AddEvent(new HitEvent(caster, endPos, GetSpell()->m_castId, i, countOfHit), caster->m_Events.CalculateTime(Milliseconds(i * 200)));
        }

        caster->SetDisableGravity(true);
        caster->GetMotionMaster()->MovePoint(0, endPos, true, angle, 12.5f);
    }

    void Register() override
    {
        BeforeCast += SpellCastFn(spell_evo_deepbreath::HandleBeforeCast);
        OnDestinationTargetSelect += SpellDestinationTargetSelectFn(spell_evo_deepbreath::SetDest, EFFECT_0, TARGET_DEST_DEST_GROUND);
    }
};

class spell_evo_deepbreath_aura : public AuraScript
{
    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->CastSpell(caster, SPELL_EVOKER_RECALL_ENABLE, true);
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_evo_deepbreath_aura::OnRemove, EFFECT_1, SPELL_AURA_DISABLE_CASTING_EXCEPT_ABILITIES, AURA_EFFECT_HANDLE_REAL);
    }

private:
    uint32 timer = 200;
};

// 371838 - Recall
class spell_evo_recall : public SpellScript
{
    class TeleportEvent : public BasicEvent
    {
    public:
        TeleportEvent(Unit* caster)
            : _caster(caster) { }

        bool Execute(uint64 time, uint32 /*diff*/) override
        {
            if (_caster->HasAura(SPELL_EVOKER_RECALL_EFFECT_HIDE))
                _caster->RemoveAura(SPELL_EVOKER_RECALL_EFFECT_HIDE);

            return true;
        }

    private:
        Unit* _caster;
    };

    void RelocateDest(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        const SpellInfo* recallSummon = sSpellMgr->GetSpellInfo(SPELL_EVOKER_RECALL, DIFFICULTY_NONE);
        if (!recallSummon)
            return;

        if (Creature* clone = caster->GetSummonedCreatureByEntry(recallSummon->GetEffect(EFFECT_0).MiscValue))
        {
            GetHitDest()->Relocate(clone->GetPosition());
            caster->CastSpell(caster, SPELL_EVOKER_RECALL_EFFECT_HIDE, true);
            caster->m_Events.AddEvent(new TeleportEvent(caster), caster->m_Events.CalculateTime(300ms));
            clone->DespawnOrUnsummon(10ms);
        }
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_evo_recall::RelocateDest, EFFECT_0, SPELL_EFFECT_TELEPORT_UNITS);
    }

};

void AddSC_DekkCore_evoker_spell_scripts()
{
    RegisterSpellScript(spell_evo_glide);
    RegisterSpellScript(spell_evo_azure_strike);
    RegisterSpellScript(spell_evo_living_flame);
    RegisterSpellScript(spell_evo_living_flame_damage);
    RegisterSpellScript(spell_cosmic_evoker_visage);
    RegisterSpellScript(spell_evo_dream_breath);
    RegisterSpellScript(spell_evo_spiritbloom);
    RegisterSpellScript(spell_evo_fire_breath);
    RegisterSpellScript(spell_evo_eternity_surge);
    RegisterSpellScript(spell_evo_upheaval);
    RegisterSpellScript(spell_evo_pyre);
    RegisterSpellScript(spell_evo_charged_blast);
    RegisterSpellScript(spell_evo_permeating_chill);
    RegisterSpellScript(spell_evo_dragonrage);
    RegisterSpellScript(spell_evo_landslide);
    RegisterSpellScript(spell_evo_deepbreath);
    RegisterSpellScript(spell_evo_deepbreath_aura);
    RegisterSpellScript(spell_evo_recall);
}
