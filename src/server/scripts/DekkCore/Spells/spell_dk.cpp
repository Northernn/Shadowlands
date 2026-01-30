/*
 * This file is part of DekkCore team Devs
 *
 */

#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "Player.h"
#include "PetAI.h"
#include "Pet.h"
#include "DynamicObject.h"
#include "ScriptMgr.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SpellHistory.h"
#include "Containers.h"
#include "Log.h"
#include "Spell.h"
#include "ObjectMgr.h"
#include <numeric>

enum DeathKnightSpells
{
    SPELL_DK_ARMY_FLESH_BEAST_TRANSFORM = 127533,
    SPELL_DK_ARMY_GEIST_TRANSFORM = 127534,
    SPELL_DK_ARMY_NORTHREND_SKELETON_TRANSFORM = 127528,
    SPELL_DK_ARMY_SKELETON_TRANSFORM = 127527,
    SPELL_DK_PERMAFROST = 207200,
    SPELL_DK_ARMY_SPIKED_GHOUL_TRANSFORM = 127525,
    SPELL_DK_ARMY_SUPER_ZOMBIE_TRANSFORM = 127526,
    SPELL_DK_BLOOD_PLAGUE = 55078,
    SPELL_DK_BLOOD_PRESENCE = 48263,
    SPELL_DK_BLOOD_SHIELD_MASTERY = 77513,
    SPELL_DK_BLOOD_SHIELD_ABSORB = 77535,
    SPELL_DK_CHAINS_OF_ICE = 45524,
    SPELL_DK_CORPSE_EXPLOSION_TRIGGERED = 43999,
    SPELL_DK_UNHOLY_MUTATION = 201934,
    SPELL_DK_DEATH_AND_DECAY_DAMAGE = 52212,
    SPELL_DK_DEATH_AND_DECAY_SLOW = 143375,
    SPELL_DK_DEATH_COIL_BARRIER = 115635,
    SPELL_DK_DEATH_COIL_DAMAGE = 47632,
    SPELL_DK_HEMOSTATIS_AURA = 273947,
    SPELL_DK_DEATH_COIL_HEAL = 47633,
    SPELL_DK_GATHERING_STORM_DAMAGE = 211805,
    SPELL_DK_DEATH_GRIP = 49576,
    SPELL_DK_REMORSELESS_WINTER = 196770,
    SPELL_DK_DEATH_GRIP_PULL = 49575,
    SPELL_DK_LAST_SURPRISE_POWER = 278489,
    SPELL_DK_DEATH_COIL = 47541,
    SPELL_DK_DEATH_GRIP_VISUAL = 55719,
    SPELL_DK_DEATH_GRIP_TAUNT = 57603,
    SPELL_DK_NECROTIC_WOUND = 223929,
    SPELL_DK_DEATH_STRIKE = 49998,
    SPELL_DK_DECOMPOSING_AURA = 199720,
    SPELL_DK_DEEP_CUTS_DEBUFF = 272685,
    SPELL_DK_PATH_OF_FROST = 3714,
    SPELL_DK_WANDERING_PLAGUE_SELECTOR = 200093,
    SPELL_DK_DECOMPOSING_AURA_DAMAGE = 199721,
    SPELL_DK_ENHANCED_DEATH_COIL = 157343,
    SPELL_DK_DEEP_CUTS_POWER = 272684,
    SPELL_DK_FROST_FEVER = 55095,
    SPELL_DK_GHOUL_EXPLODE = 47496,
    SPELL_DK_BLOOD_BOIL = 50842,
    SPELL_DK_GLYPH_OF_ABSORB_MAGIC = 159415,
    SPELL_DK_GLYPH_OF_ANTI_MAGIC_SHELL = 58623,
    SPELL_DK_GLYPH_OF_ARMY_OF_THE_DEAD = 58669,
    SPELL_DK_BLOD_HEARTBREAKER_ENERGIZE = 210738,
    SPELL_AURA_DK_HEARTBREAKER = 221536,
    SPELL_DK_GLYPH_OF_DEATH_COIL = 63333,
    SPELL_DK_REANIMATION_DAMAGE = 210141,
    SPELL_DK_REANIMATION = 210128,
    SPELL_DK_GLYPH_OF_DEATH_AND_DECAY = 58629,
    SPELL_DK_GLYPH_OF_FOUL_MENAGERIE = 58642,
    SPELL_DK_KILLING_MACHINE = 51124,
    SPELL_DK_GLYPH_OF_REGENERATIVE_MAGIC = 146648,
    SPELL_DK_GLYPH_OF_RUNIC_POWER_TRIGGERED = 159430,
    SPELL_DK_GLYPH_OF_SWIFT_DEATH = 146645,
    SPELL_DK_DEATH_CHAIN_VISUAL = 203203,
    SPELL_DK_GLYPH_OF_THE_GEIST = 58640,
    SPELL_DK_GLYPH_OF_THE_SKELETON = 146652,
    SPELL_DK_IMPROVED_BLOOD_PRESENCE = 50371,
    SPELL_DK_IMPROVED_SOUL_REAPER = 157342,
    SPELL_DK_BURSTING_SORES = 207264,
    SPELL_DK_BURSTING_SORES_EXPLOSION = 207267,
    SPELL_DK_RUNIC_POWER_ENERGIZE = 49088,
    SPELL_DK_SCENT_OF_BLOOD = 49509,
    SPELL_DK_SCENT_OF_BLOOD_TRIGGERED = 50421,
    SPELL_DK_SCOURGE_STRIKE_TRIGGERED = 70890,
    SPELL_DK_SHADOW_OF_DEATH = 164047,
    SPELL_DK_SOUL_REAPER_DAMAGE = 114867,
    SPELL_DK_SOUL_REAPER_HASTE = 114868,
    spell_dk_soul_reaper = 343294,
    SPELL_DK_T15_DPS_4P_BONUS = 138347,
    SPELL_DK_UNHOLY_PRESENCE = 48265,
    SPELL_DK_WILL_OF_THE_NECROPOLIS = 206967,
    SPELL_DK_BLOOD_BOIL_TRIGGERED = 65658,
    SPELL_DK_BLOOD_GORGED_HEAL = 50454,
    SPELL_DK_DEATH_STRIKE_ENABLER = 89832,
    SPELL_DK_FROST_PRESENCE = 48266,
    SPELL_DK_IMPROVED_FROST_PRESENCE = 50385,
    SPELL_DK_IMPROVED_FROST_PRESENCE_TRIGGERED = 50385,
    SPELL_DK_IMPROVED_UNHOLY_PRESENCE = 50392,
    SPELL_DK_IMPROVED_UNHOLY_PRESENCE_TRIGGERED = 55222,
    SPELL_DK_RUNE_TAP = 48982,
    SPELL_DK_CORPSE_EXPLOSION_VISUAL = 51270,
    SPELL_DK_TIGHTENING_GRASP = 206970,
    SPELL_DK_TIGHTENING_GRASP_SLOW = 143375,
    SPELL_DK_MASTER_OF_GHOULS = 52143,
    SPELL_DK_GHOUL_AS_GUARDIAN = 46585,
    SPELL_DK_CONTROL_UNDEATH = 111673,
    SPELL_DK_CRYPT_FEVER = 199722,
    SPELL_DK_GHOUL_AS_PET = 52150,
    SPELL_DK_ROILING_BLOOD = 108170,

    SPELL_DK_CHILBLAINS = 50041,
    SPELL_DK_CHAINS_OF_ICE_ROOT = 53534,
    SPELL_DK_PLAGUE_LEECH = 123693,
    SPELL_DK_PERDITION = 123981,
    SPELL_DK_SHROUD_OF_PURGATORY = 116888,
    SPELL_DK_PURGATORY_INSTAKILL = 123982,
    SPELL_DK_BLOOD_RITES = 50034,
    SPELL_DK_DEATH_SIPHON_HEAL = 116783,
    SPELL_DK_BLOOD_CHARGE = 114851,
    SPELL_DK_BOOD_TAP = 45529,
    SPELL_DK_PILLAR_OF_FROST = 51271,
    SPELL_DK_CONVERSION = 119975,
    SPELL_DK_WEAKENED_BLOWS = 115798,
    SPELL_DK_SCARLET_FEVER = 81132,
    SPELL_DK_SCENT_OF_BLOOD_AURA = 50421,
    SPELL_DK_DESECRATED_GROUND = 118009,
    SPELL_DK_DESECRATED_GROUND_IMMUNE = 115018,
    SPELL_DK_ARMY_OF_DEATH = 42651,
    SPELL_DK_ASPHYXIATE = 108194,
    SPELL_DK_DARK_INFUSION_STACKS = 91342,
    SPELL_DK_DARK_INFUSION_AURA = 93426,
    SPELL_DK_RUNIC_CORRUPTION_REGEN = 51460,
    SPELL_DK_RUNIC_EMPOWERMENT = 81229,
    SPELL_DK_GOREFIENDS_GRASP_GRIP_VISUAL = 114869,
    SPELL_DK_SLUDGE_BELCHER_AURA = 207313,
    SPELL_DK_SLUDGE_BELCHER_ABOMINATION = 212027,
    SPELL_DK_RAISE_DEAD = 46584,
    SPELL_DK_RAISE_DEAD_GHOUL = 52150,
    SPELL_DK_GEIST_TRANSFORM = 121916,
    SPELL_DK_ANTI_MAGIC_BARRIER = 205725,
    SPELL_DK_RUNIC_CORRUPTION = 51462,
    SPELL_DK_NECROSIS = 207346,
    SPELL_DK_FROZEN_CENTER = 204135,
    SPELL_DK_FROZEN_CENTER_ROOT = 233395,
    SPELL_DK_NECROSIS_EFFECT = 216974,
    SPELL_DK_ALL_WILL_SERVE = 194916,
    SPELL_DK_ALL_WILL_SERVE_SUMMON = 196910,
    SPELL_DK_BREATH_OF_SINDRAGOSA = 152279,
    SPELL_DK_DEATH_GRIP_ONLY_JUMP = 146599,
    SPELL_DK_HEART_STRIKE = 206930,
    SPELL_DK_FESTERING_WOUND = 194310,
    SPELL_DK_FESTERING_WOUND_DAMAGE = 194311,
    SPELL_DK_BONE_SHIELD = 195181,
    SPELL_DK_BLOOD_MIRROR_DAMAGE = 221847,
    SPELL_DK_BLOOD_MIRROR = 206977,
    SPELL_DK_BONESTORM_HEAL = 196545,
    SPELL_DK_GLACIAL_ADVANCE = 194913,
    SPELL_DK_GLACIAL_ADVANCE_DAMAGE = 195975,
    SPELL_DK_HOWLING_BLAST = 49184,
    SPELL_DK_HOWLING_BLAST_AREA_DAMAGE = 237680,
    SPELL_DK_RIME_BUFF = 59052,
    SPELL_DK_NORTHREND_WINDS = 204088,
    SPELL_DK_REMORSELESS_WINTER_SLOW_DOWN = 211793,
    SPELL_DK_EPIDEMIC = 207317,
    SPELL_DK_EPIDEMIC_DAMAGE_SINGLE = 212739,
    SPELL_DK_EPIDEMIC_DAMAGE_AOE = 215969,
    SPELL_DK_VIRULENT_PLAGUE = 191587,
    SPELL_DK_WANDERING_PLAGUE_DISEASE = 199969,
    SPELL_DK_UNHOLY_BLIGHT = 115994,
    SPELL_DK_VIRULENT_ERUPTION = 191685,
    SPELL_DK_OUTBREAK_PERIODIC = 196782,
    SPELL_DK_DEFILE = 152280,
    SPELL_DK_DEFILE_DAMAGE = 156000,
    SPELL_DK_DEFILE_DUMMY = 156004,
    SPELL_DK_DEFILE_MASTERY = 218100,
    SPELL_DK_UNHOLY_FRENZY = 207289,
    SPELL_DK_UNHOLY_FRENZY_BUFF = 207290,
    SPELL_DK_PESTILENT_PUSTULES = 194917,
    SPELL_DK_CASTIGATOR = 207305,
    SPELL_DK_UNHOLY_VIGOR = 196263,
    SPELL_DK_RECENTLY_USED_DEATH_STRIKE = 180612,
    SPELL_DK_FROST = 137006,
    SPELL_DK_DEATH_STRIKE_OFFHAND = 66188,
    SPELL_DK_VAMPIRIC_BLOOD = 55233,
    SPELL_DK_CRIMSOM_SCOURGE = 81136,
    // 8.0
    SPELL_DK_CLAWING_SHADOWS = 207311,
    SPELL_DK_INFECTED_CLAWS = 207272,
    SPELL_DK_SUMMON_DEFILE = 169018, //npc 82521
    SPELL_DK_HARBINGER_OF_THE_DOOM = 276023,
    SPELL_DK_PESTILENCE = 277234,
    SPELL_DK_ANTIMAGIC_ZONE_DAMAGE_TAKEN = 145629,
    SPELL_DK_DARK_SIMULACRUM = 77606,
    SPELL_DK_DARK_SIMULACRUM_PROC = 77616,
    SPELL_DK_ICECAP = 207126,
    SPELL_DK_COLD_HEART_CHARGE = 281209,
    SPELL_DK_COLD_HEART_DAMAGE = 281210,
    SPELL_DK_OSSUARY_MOD_MAX_POWER = 219786,
    SPELL_DK_OSSUARY_MOD_POWER_COST = 219788,
    SPELL_DK_GRIP_OF_THE_DEAD = 273952,
    SPELL_DK_GRIP_OF_THE_DEAD_PERIODIC = 273980,
    SPELL_DK_HOWLING_BLAST_AOE = 237680,
    SPELL_DK_GRIP_OF_THE_DEAD_SLOW = 273977,
    SPELL_DK_VORACIOUS = 273953,
    SPELL_DK_VORACIOUS_MOD_LEECH = 274009,
    SPELL_DK_RED_THIRST = 205723,
    SPELL_DK_PURGATORY = 114556,
    SPELL_DK_BONESTORM = 194844,
    SPELL_DK_INEXORABLE_ASSAULT_DUMMY = 253593,
    SPELL_DK_INEXORABLE_ASSAULT_STACK = 253595,
    SPELL_DK_INEXORABLE_ASSAULT_DAMAGE = 253597,
    SPELL_DK_INEXORABLE_ASSAULT = 253595,
    SPELL_DK_FROSTSCYTHE = 207230,
    SPELL_DK_AVALANCHE = 207142,
    SPELL_DK_AVALANCHE_DAMAGE = 207150,
    SPELL_DK_RIME = 59057,
    SPELL_DK_CRYPT_FEVER_DAMAGE = 199723,
    SPELL_DK_RAZORICE_MOD_DAMAGE_TAKEN = 51714,
    SPELL_DK_ARMY_OF_THE_DAMNED = 276837,
    SPELL_DK_RUNIC_CORRUPTION_MOD_RUNES = 51460,
    SPELL_DK_ARMY_OF_THE_DEAD = 42650,
    SPELL_DK_LAST_SURPRISE_DAMAGE = 279606,
    SPELL_DK_APOCALYPSE = 275699,
    SPELL_DK_ARMY_OF_DEATH_TRIGGER = 42650,
    SPELL_DK_DARK_TRANSFORMATION = 63560,
    SPELL_DK_COLD_HEART = 281209,
    SPELL_DK_SOUL_REAPER_MOD_HASTE = 69410,
    SPELL_DK_VOLATILE_SHIELDING = 207188,
    SPELL_DK_DEATH_GRIP_DUMMY = 243912,
    SPELL_DK_DEATH_GRIP_JUMP = 49575,
    SPELL_DK_BLOOD = 137008,
    SPELL_DK_OBLITERATE = 49020,
    SPELL_DK_FROST_SHIELD = 207203,
    SPELL_DK_FROST_STRIKE_MAIN_HAND = 222026,
    SPELL_DK_DEATH_STRIKE_HEAL = 282751, //45470, that heal
    SPELL_DK_FROST_SCYTHE = 207230,
    SPELL_DK_KILLING_MACHINE_PROC = 51124,
    SPELL_DK_MARK_OF_BLOOD_HEAL = 206945,
    SPELL_DK_OBLITERATION = 281238,
    SPELL_DK_OBLITERATION_RUNE_ENERGIZE = 281327,
    SPELL_DK_RAISE_DEAD_SUMMON = 52150,
    SPELL_DK_RUNIC_RETURN = 61258,
    SPELL_DK_SLUDGE_BELCHER = 207313,
    SPELL_DK_SLUDGE_BELCHER_SUMMON = 212027,
    SPELL_DK_UNHOLY = 137007,
    SPELL_DK_VOLATILE_SHIELDING_DAMAGE = 207194
};

enum DeathKnightStorageEntries
{
    STORAGE_DK_DEATH_STRIKE
};

//81136
class spell_dk_crimsom_scourge : public SpellScriptLoader
{
public:
    spell_dk_crimsom_scourge() : SpellScriptLoader("spell_dk_crimsom_scourge") { }

    class spell_dk_crimsom_scourge_AuraScript : public AuraScript
    {

        bool CheckProc(ProcEventInfo& eventInfo)
        {
            Unit* target = GetTarget();
            target->HasAura(SPELL_DK_BLOOD_PLAGUE);
            return true;
        }

        void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
        {
            Unit* caster = GetCaster();
            if (!caster)
                return;

            if (roll_chance_i(40)) {
                caster->CastSpell(caster, 81141, true);
            }

        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_dk_crimsom_scourge_AuraScript::CheckProc);
            OnEffectProc += AuraEffectProcFn(spell_dk_crimsom_scourge_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
        }

    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_crimsom_scourge_AuraScript();
    }
};

// 197531 - Bloodworms
class spell_dk_bloodworms : public SpellScriptLoader
{
public:
    spell_dk_bloodworms() : SpellScriptLoader("spell_dk_bloodworms") { }

    class spell_dk_bloodworms_SpellScript : public SpellScript
    {
        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.clear();

            if (Unit* caster = GetCaster())
            {
                for (auto itr : caster->m_Controlled)
                    if (Unit* unit = ObjectAccessor::GetUnit(*caster, itr->GetGUID()))
                        if (unit->GetEntry() == 99773)
                            targets.push_back(unit);
            }
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dk_bloodworms_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };
    SpellScript* GetSpellScript() const override
    {
        return new spell_dk_bloodworms_SpellScript();
    }

};

// 43264 - Periodic Taunt
/// 6.x, does this belong here or in spell_generic? apply this in creature_template_addon? sniffs say this is always cast army of the dead ghouls.
class spell_dk_army_periodic_taunt : public SpellScriptLoader
{
public:
    spell_dk_army_periodic_taunt() : SpellScriptLoader("spell_dk_army_periodic_taunt") { }

    class spell_dk_army_periodic_taunt_SpellScript : public SpellScript
    {
        bool Load() override
        {
            return GetCaster()->IsGuardian();
        }

        SpellCastResult CheckCast()
        {
            if (Unit* owner = GetCaster()->GetOwner())
                if (!owner->HasAura(SPELL_DK_GLYPH_OF_ARMY_OF_THE_DEAD))
                    return SPELL_CAST_OK;

            return SPELL_FAILED_SPELL_UNAVAILABLE;
        }

        void Register() override
        {
            OnCheckCast += SpellCheckCastFn(spell_dk_army_periodic_taunt_SpellScript::CheckCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_dk_army_periodic_taunt_SpellScript();
    }
};

/// Blood Shield - 77535
class spell_dk_blood_shield : public SpellScriptLoader
{
public:
    spell_dk_blood_shield() : SpellScriptLoader("spell_dk_blood_shield") { }

    class spell_dk_blood_shield_AuraScript : public AuraScript
    {
        enum eSpells
        {
            T17Blood4P = 165571
        };

        void AfterAbsorb(AuraEffect* p_AurEff, DamageInfo& /*p_DmgInfo*/, uint32& p_AbsorbAmount)
        {
            if (Unit* l_Target = GetTarget())
            {
                /// While Vampiric Blood is active, your Blood Shield cannot be reduced below 3% of your maximum health.
                if (AuraEffect* l_AurEff = l_Target->GetAuraEffect(eSpells::T17Blood4P, EFFECT_0))
                {
                    int32 l_FutureAbsorb = p_AurEff->GetAmount() - p_AbsorbAmount;
                    int32 l_MinimaAbsorb = l_Target->CountPctFromMaxHealth(l_AurEff->GetAmount());

                    /// We need to add some absorb amount to correct the absorb amount after that, and set it to 3% of max health
                    if (l_FutureAbsorb < l_MinimaAbsorb)
                    {
                        int32 l_AddedAbsorb = l_MinimaAbsorb - l_FutureAbsorb;
                        p_AurEff->ChangeAmount(p_AurEff->GetAmount() + l_AddedAbsorb);
                    }
                }
            }
        }

        void Register() override
        {
            AfterEffectAbsorb += AuraEffectAbsorbFn(spell_dk_blood_shield_AuraScript::AfterAbsorb, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_blood_shield_AuraScript();
    }
};

class spell_dk_item_t17_frost_4p_driver : public AuraScript
{
    enum eSpells
    {
        FrozenRuneblade = 170202
    };

    void OnProc(AuraEffect* /*p_AurEff*/, ProcEventInfo& p_EventInfo)
    {
        PreventDefaultAction();

        Unit* l_Caster = GetCaster();
        if (!l_Caster)
            return;

        SpellInfo const* l_ProcSpell = p_EventInfo.GetDamageInfo()->GetSpellInfo();
        if (!l_ProcSpell)
            return;

        Unit* l_Target = p_EventInfo.GetActionTarget();
        if (!l_Target || l_Target == l_Caster)
            return;

        /// While Pillar of Frost is active, your special attacks trap a soul in your rune weapon.
        l_Caster->CastSpell(l_Target, eSpells::FrozenRuneblade, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_dk_item_t17_frost_4p_driver::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

/// Item - Death Knight T17 Frost 4P Driver (Periodic) - 170205
class spell_dk_item_t17_frost_4p_driver_periodic : public SpellScriptLoader
{
public:
    spell_dk_item_t17_frost_4p_driver_periodic() : SpellScriptLoader("spell_dk_item_t17_frost_4p_driver_periodic") { }

    class spell_dk_item_t17_frost_4p_driver_periodic_AuraScript : public AuraScript
    {
        enum eSpells
        {
            FrozenRunebladeMainHand = 165569,
            FrozenRunebladeOffHand = 178808,
            FrozenRunebladeStacks = 170202
        };

        void OnTick(AuraEffect const* /*p_AurEff*/)
        {
            Unit* l_Caster = GetCaster();
            if (l_Caster == nullptr)
                return;

            Unit* l_Target = l_Caster->GetVictim();
            if (l_Target == nullptr)
                return;

            if (Player* l_Player = l_Caster->ToPlayer())
            {
                if (Aura* l_Aura = l_Player->GetAura(eSpells::FrozenRunebladeStacks))
                {
                    if (Item* l_MainHand = l_Player->GetItemByPos(INVENTORY_SLOT_BAG_0, EquipmentSlots::EQUIPMENT_SLOT_MAINHAND))
                        l_Player->CastSpell(l_Target, eSpells::FrozenRunebladeMainHand, true);

                    if (Item* l_OffHand = l_Player->GetItemByPos(INVENTORY_SLOT_BAG_0, EquipmentSlots::EQUIPMENT_SLOT_OFFHAND))
                        l_Player->CastSpell(l_Target, eSpells::FrozenRunebladeOffHand, true);

                    l_Aura->DropCharge();
                }
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_item_t17_frost_4p_driver_periodic_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_item_t17_frost_4p_driver_periodic_AuraScript();
    }
};

// 48792 - Icebound Fortitude
/// 6.x
class spell_dk_icebound_fortitude : public SpellScriptLoader
{
public:
    spell_dk_icebound_fortitude() : SpellScriptLoader("spell_dk_icebound_fortitude") { }

    class spell_dk_icebound_fortitude_AuraScript : public AuraScript
    {
        void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (GetUnitOwner()->HasAura(SPELL_DK_IMPROVED_BLOOD_PRESENCE))
                amount += 30; /// todo, figure out how tooltip is updated
        }

        void Register() override
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_icebound_fortitude_AuraScript::CalculateAmount, EFFECT_2, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_icebound_fortitude_AuraScript();
    }
};

//343294
class spell_dk_soul_reaper : public AuraScript
{
    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        if (GetCaster() && GetTarget() && GetTarget()->isDead())
            GetCaster()->CastSpell(nullptr, SPELL_DK_SOUL_REAPER_MOD_HASTE, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_soul_reaper::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 115994 - Unholy Blight, triggered by 115989
class spell_dk_unholy_blight : public AuraScript
{
    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        return;
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_unholy_blight::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 206967 - Will of the Necropolis
class spell_dk_will_of_the_necropolis : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        if (!sSpellMgr->GetSpellInfo(SPELL_DK_WILL_OF_THE_NECROPOLIS, DIFFICULTY_NONE))
            return false;
        return true;
    }

    void CalculateAmount(AuraEffect const* /*p_AuraEffect*/, int32& p_Amount, bool& /*p_CanBeRecalculated*/)
    {
        p_Amount = -1;
    }

    void Absorb(AuraEffect* /*auraEffect*/, DamageInfo& dmgInfo, uint32& absorbAmount)
    {
        absorbAmount = 0;

        if (GetTarget()->GetHealthPct() < GetEffect(EFFECT_2)->GetBaseAmount())
            absorbAmount = CalculatePct(dmgInfo.GetDamage(), GetEffect(EFFECT_1)->GetBaseAmount());
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_will_of_the_necropolis::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_will_of_the_necropolis::Absorb, EFFECT_0);
    }
};

// Dark transformation - transform pet spell - 63560
class spell_dk_dark_transformation_form : public SpellScriptLoader
{
public:
    spell_dk_dark_transformation_form() : SpellScriptLoader("spell_dk_dark_transformation_form") { }

    class spell_dk_dark_transformation_form_SpellScript : public SpellScript
    {
        void HandleOnHit()
        {
            if (Player* _player = GetCaster()->ToPlayer())
            {
                if (Unit* pet = GetHitUnit())
                {
                    if (pet->HasAura(SPELL_DK_DARK_INFUSION_STACKS))
                    {
                        _player->RemoveAura(SPELL_DK_DARK_INFUSION_STACKS);
                        pet->RemoveAura(SPELL_DK_DARK_INFUSION_STACKS);
                    }
                }
            }
        }

        void Register() override
        {
            OnHit += SpellHitFn(spell_dk_dark_transformation_form_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_dk_dark_transformation_form_SpellScript();
    }
};


// Desecrated ground - 118009
class spell_dk_desecrated_ground : public SpellScriptLoader
{
public:
    spell_dk_desecrated_ground() : SpellScriptLoader("spell_dk_desecrated_ground") { }

    class spell_dk_desecrated_ground_AuraScript : public AuraScript
    {
        void OnTick(AuraEffect const* /* aurEff */)
        {
            if (GetCaster())
                if (DynamicObject* dynObj = GetCaster()->GetDynObject(SPELL_DK_DESECRATED_GROUND))
                    if (GetCaster()->GetDistance(dynObj) <= 8.0f)
                        GetCaster()->CastSpell(GetCaster(), SPELL_DK_DESECRATED_GROUND_IMMUNE, true);
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_desecrated_ground_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_desecrated_ground_AuraScript();
    }
};

static constexpr uint32 SPELL_VISUAL_ID_HOWLING_BLAST = 66812;

// 49184 - Howling Blast
// 237680 - Howling Blast
class spell_dk_howling_blast : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_HOWLING_BLAST_AREA_DAMAGE, SPELL_DK_FROST_FEVER });
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([&](WorldObject const* target)
            {
                if (GetSpellInfo()->Id == SPELL_DK_HOWLING_BLAST_AREA_DAMAGE)
                {
                    if (GetSpell()->m_customArg.has_value())
                        return target->GetGUID() == std::any_cast<ObjectGuid>(GetSpell()->m_customArg);
                }
                else
                    return GetExplTargetUnit() != target;

        return false;
            });
    }

    void HandleFrostFever(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(GetHitUnit(), SPELL_DK_FROST_FEVER);
    }

    void HandleAreaDamage(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetExplTargetUnit(), SPELL_DK_HOWLING_BLAST_AREA_DAMAGE, CastSpellExtraArgs().SetCustomArg(GetExplTargetUnit()->GetGUID()));
    }

    void HandleSpellVisual(SpellEffIndex /*effIndex*/)
    {
        if (!GetSpell()->m_customArg.has_value())
            return;

        if (Unit* caster = GetCaster())
            if (Unit* primaryTarget = ObjectAccessor::GetUnit(*caster, std::any_cast<ObjectGuid>(GetSpell()->m_customArg)))
                primaryTarget->SendPlaySpellVisual(GetHitUnit(), SPELL_VISUAL_ID_HOWLING_BLAST, 0, 0, 0.f, 0.f);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dk_howling_blast::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget += SpellEffectFn(spell_dk_howling_blast::HandleFrostFever, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        if (m_scriptSpellId == SPELL_DK_HOWLING_BLAST_AREA_DAMAGE)
            OnEffectHitTarget += SpellEffectFn(spell_dk_howling_blast::HandleSpellVisual, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        else
            OnEffectLaunchTarget += SpellEffectFn(spell_dk_howling_blast::HandleAreaDamage, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

// Remorseless Winter - 196771
class spell_dk_remorseless_winter_damage : public SpellScript
{
    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        if (Unit* unit = GetHitUnit())
            GetCaster()->CastSpell(unit, SPELL_DK_REMORSELESS_WINTER_SLOW_DOWN, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_remorseless_winter_damage::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// Pillar of Frost - 51271
class spell_dk_pillar_of_frost : public SpellScriptLoader
{
public:
    spell_dk_pillar_of_frost() : SpellScriptLoader("spell_dk_pillar_of_frost") { }

    class spell_dk_pillar_of_frost_AuraScript : public AuraScript
    {
        void OnRemove(AuraEffect const*  /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Player* _player = GetTarget()->ToPlayer())
                _player->ApplySpellImmune(SPELL_DK_PILLAR_OF_FROST, IMMUNITY_MECHANIC, MECHANIC_KNOCKOUT, false);
        }

        void OnApply(AuraEffect const*  /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Player* _player = GetTarget()->ToPlayer())
                _player->ApplySpellImmune(SPELL_DK_PILLAR_OF_FROST, IMMUNITY_MECHANIC, MECHANIC_KNOCKOUT, true);
        }

        void Register() override
        {
            OnEffectApply += AuraEffectApplyFn(spell_dk_pillar_of_frost_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_dk_pillar_of_frost_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_pillar_of_frost_AuraScript();
    }
};

// Death Siphon - 108196
class spell_dk_death_siphon : public SpellScriptLoader
{
public:
    spell_dk_death_siphon() : SpellScriptLoader("spell_dk_death_siphon") { }

    class spell_dk_death_siphon_SpellScript : public SpellScript
    {
        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            if (Player* _player = GetCaster()->ToPlayer())
            {
                if (GetHitUnit())
                {
                    float bp = GetHitDamage();
                    CastSpellExtraArgs args;
                    args.AddSpellBP0(bp);
                    args.SetTriggerFlags(TRIGGERED_FULL_MASK);
                    _player->CastSpell(_player, SPELL_DK_DEATH_SIPHON_HEAL, args);
                }
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_dk_death_siphon_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_dk_death_siphon_SpellScript();
    }
};

// Purgatory - 116888
class spell_dk_purgatory : public SpellScriptLoader
{
public:
    spell_dk_purgatory() : SpellScriptLoader("spell_dk_purgatory") { }

    class spell_dk_purgatory_AuraScript : public AuraScript
    {
        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Player* _player = GetTarget()->ToPlayer())
            {
                AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                if (removeMode == AURA_REMOVE_BY_EXPIRE)
                    _player->CastSpell(_player, SPELL_DK_PURGATORY_INSTAKILL, true);
            }
        }

        void Register() override
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_dk_purgatory_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_HEAL_ABSORB, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_purgatory_AuraScript();
    }
};

// Purgatory - 114556
class spell_dk_purgatory_absorb : public SpellScriptLoader
{
public:
    spell_dk_purgatory_absorb() : SpellScriptLoader("spell_dk_purgatory_absorb") { }

    class spell_dk_purgatory_absorb_AuraScript : public AuraScript
    {
        void CalculateAmount(AuraEffect const* /*auraEffect*/, int32& amount, bool& /*canBeRecalculated*/)
        {
            amount = -1;
        }

        void Absorb(AuraEffect* /*auraEffect*/, DamageInfo& dmgInfo, uint32& absorbAmount)
        {
            Unit* target = GetTarget();

            if (dmgInfo.GetDamage() < target->GetHealth())
                return;

            // No damage received under Shroud of Purgatory
            if (target->ToPlayer()->HasAura(SPELL_DK_SHROUD_OF_PURGATORY))
            {
                absorbAmount = dmgInfo.GetDamage();
                return;
            }

            if (target->ToPlayer()->HasAura(SPELL_DK_PERDITION))
                return;

            float bp = dmgInfo.GetDamage();
            CastSpellExtraArgs args;
            args.AddSpellBP0(bp);
            args.SetTriggerFlags(TRIGGERED_FULL_MASK);
            target->CastSpell(target, SPELL_DK_SHROUD_OF_PURGATORY, args);
            target->CastSpell(target, SPELL_DK_PERDITION, TRIGGERED_FULL_MASK);
            target->SetHealth(1);
            absorbAmount = dmgInfo.GetDamage();
        }

        void Register() override
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_purgatory_absorb_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_purgatory_absorb_AuraScript::Absorb, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_purgatory_absorb_AuraScript();
    }
};

//45524
class spell_dk_chilblains : public SpellScript
{
    void HandleOnHit()
    {
        if (Player* player = GetCaster()->ToPlayer())
            if (Unit* target = GetHitUnit())
                if (player->HasAura(SPELL_DK_CHILBLAINS))
                    player->CastSpell(target, SPELL_DK_CHAINS_OF_ICE_ROOT, true);

        if (GetCaster()->HasAura(SPELL_DK_COLD_HEART_CHARGE))
            if (Aura* coldHeartCharge = GetCaster()->GetAura(SPELL_DK_COLD_HEART_CHARGE))
            {
                uint8 stacks = coldHeartCharge->GetStackAmount();
                SetHitDamage(GetHitDamage() * stacks);
                GetCaster()->CastSpell(GetHitUnit(), SPELL_DK_COLD_HEART_DAMAGE, true);
                coldHeartCharge->ModStackAmount(-stacks);
            }
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_dk_chilblains::HandleOnHit);
    }
};

// Outbreak - 77575
class spell_dk_outbreak : public SpellScript
{
    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        if (Unit* target = GetHitUnit())
            if (!target->HasAura(SPELL_DK_OUTBREAK_PERIODIC, GetCaster()->GetGUID()))
                GetCaster()->CastSpell(target, SPELL_DK_OUTBREAK_PERIODIC, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_outbreak::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// Outbreak - 196782
class aura_dk_outbreak_periodic : public AuraScript
{
    void HandleDummyTick(AuraEffect const* /*aurEff*/)
    {
        if (Unit* caster = GetCaster())
        {
            std::list<Unit*> friendlyUnits;
            GetTarget()->GetFriendlyUnitListInRange(friendlyUnits, 10.f);

            for (Unit* unit : friendlyUnits)
                if (!unit->HasUnitFlag(UNIT_FLAG_IMMUNE_TO_PC) && unit->IsInCombatWith(caster))
                    caster->CastSpell(unit, SPELL_DK_VIRULENT_PLAGUE, true);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_dk_outbreak_periodic::HandleDummyTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 48707 - Anti-Magic Shell (on self)
class spell_dk_anti_magic_shell_self : public SpellScriptLoader
{
public:
    spell_dk_anti_magic_shell_self() : SpellScriptLoader("spell_dk_anti_magic_shell_self") { }

    class spell_dk_anti_magic_shell_self_AuraScript : public AuraScript
    {
        uint32 absorbPct;
        bool Load() override
        {
            absorbPct = GetSpellInfo()->GetEffect(EFFECT_0).CalcValue(GetCaster());
            return true;
        }

        void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
        {
            amount = GetUnitOwner()->CountPctFromMaxHealth(40);
        }

        void Absorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
        {
            absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
        }

        void Trigger(AuraEffect* aurEff, DamageInfo& /*dmgInfo*/, uint32& absorbAmount)
        {
            Unit* target = GetTarget();
            // Patch 6.0.2 (October 14, 2014): Anti-Magic Shell now restores 2 Runic Power per 1% of max health absorbed.
            float damagePerRp = target->CountPctFromMaxHealth(1) / 2.0f;
            float energizeAmount = (absorbAmount / damagePerRp) * 10.0f;
            CastSpellExtraArgs args;
            args.AddSpellBP0(energizeAmount);
            args.SetTriggerFlags(TRIGGERED_FULL_MASK);
            args.SetTriggeringAura(aurEff);
            target->CastSpell(target, SPELL_DK_RUNIC_POWER_ENERGIZE, args);
        }

        void Register() override
        {
            OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_shell_self_AuraScript::Absorb, EFFECT_0);
            AfterEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_shell_self_AuraScript::Trigger, EFFECT_0);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_anti_magic_shell_self_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_anti_magic_shell_self_AuraScript();
    }
};

// Plague Leech
class spell_dk_plague_leech : public SpellScriptLoader
{
public:
    spell_dk_plague_leech() : SpellScriptLoader("spell_dk_plague_leech") {}

    class spell_dk_plague_leech_SpellScript : public SpellScript
    {
        SpellCastResult CheckClass()
        {
            if (Unit* target = GetExplTargetUnit())
            {
                uint8 diseases = target->GetDiseasesByCaster(GetCaster()->GetGUID());
                uint8 requiredDiseases = GetCaster()->HasAura(152281) ? 1 : 2;
                if (diseases < requiredDiseases)
                {
                    //SetCustomCastResultMessage(159);
                    return SPELL_FAILED_CUSTOM_ERROR;
                }
            }

            return SPELL_CAST_OK;
        }

        void HandleOnHit()
        {
            Player* player = GetCaster()->ToPlayer();
            if (!player)
                return;

            std::vector<uint8> runes;
            for (uint8 i = 0; i < MAX_RUNES; ++i)
                if (player->GetRuneCooldown(i) == player->GetRuneBaseCooldown())
                    runes.push_back(i);

            if (!runes.empty())
            {
                for (uint8 i = 0; i < 2 && !runes.empty(); i++)
                {
                    std::vector<uint8>::iterator itr = runes.begin();
                    std::advance(itr, urand(0, runes.size() - 1));
                    uint32 runesState = player->GetRunesState();
                    player->SetRuneCooldown((*itr), 0);
                   // if ((player->GetRunesState() & ~runesState) != 0)
                   //     player->AddRunePower((player->GetRunesState() & ~runesState));

                    runes.erase(itr);
                }

                GetHitUnit()->GetDiseasesByCaster(GetCaster()->GetGUID(), true);
            }
        }

        void Register() override
        {
            OnCheckCast += SpellCheckCastFn(spell_dk_plague_leech_SpellScript::CheckClass);
            OnHit += SpellHitFn(spell_dk_plague_leech_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_dk_plague_leech_SpellScript();
    }
};

// 48266 - Blood Presence
// 48263 - Frost Presence
// 48265 - Unholy Presence
class spell_dk_presence : public SpellScriptLoader
{
public:
    spell_dk_presence() : SpellScriptLoader("spell_dk_presence") { }

    class spell_dk_presence_AuraScript : public AuraScript
    {
        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_DK_FROST_PRESENCE, DIFFICULTY_NONE)
                || !sSpellMgr->GetSpellInfo(SPELL_DK_UNHOLY_PRESENCE, DIFFICULTY_NONE)
                || !sSpellMgr->GetSpellInfo(SPELL_DK_IMPROVED_FROST_PRESENCE, DIFFICULTY_NONE)
                || !sSpellMgr->GetSpellInfo(SPELL_DK_IMPROVED_UNHOLY_PRESENCE, DIFFICULTY_NONE)
                || !sSpellMgr->GetSpellInfo(SPELL_DK_IMPROVED_UNHOLY_PRESENCE_TRIGGERED, DIFFICULTY_NONE)
                || !sSpellMgr->GetSpellInfo(SPELL_DK_IMPROVED_FROST_PRESENCE_TRIGGERED, DIFFICULTY_NONE))
                return false;

            return true;
        }

        void HandleImprovedFrostPresence(AuraEffect const* /* aurEff */, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            if (AuraEffect* impAurEff = target->GetAuraEffect(SPELL_DK_IMPROVED_FROST_PRESENCE, EFFECT_0))
                impAurEff->SetAmount(impAurEff->CalculateAmount(GetCaster()));
        }

        void HandleImprovedUnholyPresence(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            if (AuraEffect const* impAurEff = target->GetAuraEffect(SPELL_DK_IMPROVED_UNHOLY_PRESENCE, EFFECT_0))
                if (!target->HasAura(SPELL_DK_IMPROVED_UNHOLY_PRESENCE_TRIGGERED))
                    target->CastSpell(target, SPELL_DK_IMPROVED_UNHOLY_PRESENCE_TRIGGERED, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellBP0(impAurEff->GetAmount()).SetTriggeringAura(aurEff));
        }

        void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            if (AuraEffect* impAurEff = target->GetAuraEffect(SPELL_DK_IMPROVED_FROST_PRESENCE, EFFECT_0))
                impAurEff->SetAmount(0);
            target->RemoveAura(SPELL_DK_IMPROVED_UNHOLY_PRESENCE_TRIGGERED);
        }

        void Register() override
        {
            if (m_scriptSpellId == SPELL_DK_FROST_PRESENCE)
                AfterEffectApply += AuraEffectApplyFn(spell_dk_presence_AuraScript::HandleImprovedFrostPresence, EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
            if (m_scriptSpellId == SPELL_DK_UNHOLY_PRESENCE)
                AfterEffectApply += AuraEffectApplyFn(spell_dk_presence_AuraScript::HandleImprovedUnholyPresence, EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);

            AfterEffectRemove += AuraEffectRemoveFn(spell_dk_presence_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_presence_AuraScript();
    }

    class spell_dk_presence_SpellScript : public SpellScript
    {
        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_DK_SCOURGE_STRIKE_TRIGGERED, DIFFICULTY_NONE))
                return false;
            return true;
        }

        void HandleAfterHit()
        {
            Unit* caster = GetCaster();
            if (GetHitUnit())
            {
                uint32 runicPower = caster->GetPower(POWER_RUNIC_POWER);
                if (AuraEffect* aurEff = caster->GetAuraEffect(58647, EFFECT_0))
                    runicPower = CalculatePct(runicPower, aurEff->GetAmount());
                else
                    runicPower = 0;

                caster->SetPower(POWER_RUNIC_POWER, runicPower);
            }
        }

        void Register() override
        {
            AfterHit += SpellHitFn(spell_dk_presence_SpellScript::HandleAfterHit);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_dk_presence_SpellScript();
    }
};

// Chains of Ice
class spell_dk_chains_of_ice : public SpellScriptLoader
{
public:
    spell_dk_chains_of_ice() : SpellScriptLoader("spell_dk_chains_of_ice") { }

    class spell_dk_chains_of_ice_SpellScript : public SpellScript
    {
        void HandleOnHit()
        {
            Unit* caster = GetCaster();
            if (Unit* target = GetHitUnit())
            {
                if (caster->HasAura(152281))
                    caster->CastSpell(target, 155159, true);
                else
                    caster->CastSpell(target, SPELL_DK_FROST_FEVER, true);
            }
        }

        void Register() override
        {
            OnHit += SpellHitFn(spell_dk_chains_of_ice_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_dk_chains_of_ice_SpellScript();
    }
};


// Icy touch
class spell_dk_icy_touch : public SpellScript
{
    void HandleOnHit()
    {
        Unit* caster = GetCaster();
        if (Unit* target = GetHitUnit())
        {
            if (caster->HasAura(152281))
                caster->CastSpell(target, 155159, true);
            else
                caster->CastSpell(target, SPELL_DK_FROST_FEVER, true);
        }
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_dk_icy_touch::HandleOnHit);
    }
};

// Anti-Magic Barrier - 205725
class spell_dk_anti_magic_barrier : public SpellScriptLoader
{
public:
    spell_dk_anti_magic_barrier() : SpellScriptLoader("spell_dk_anti_magic_barrier") { }

    class spell_dk_anti_magic_barrier_AuraScript : public AuraScript
    {
        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_DK_ANTI_MAGIC_BARRIER, DIFFICULTY_NONE))
                return false;
            return true;
        }

        void CalcAmount(AuraEffect const* aurEff, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Unit* caster = GetCaster())
                amount = int32((caster->GetMaxHealth() * 25.0f) / 100.0f);

//            aurEff->GetCaster()->DealDamage(aurEff->GetCaster(), 6969); //debug
        }

        void Register() override
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_anti_magic_barrier_AuraScript::CalcAmount, EFFECT_0, SPELL_AURA_MOD_INCREASE_HEALTH_2);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_anti_magic_barrier_AuraScript();
    }
};

// Breath of Sindragosa - 152279
class spell_dk_breath_of_sindragosa : public SpellScriptLoader
{
public:
    spell_dk_breath_of_sindragosa() : SpellScriptLoader("spell_dk_breath_of_sindragosa") { }

    class spell_dk_breath_of_sindragosa_AuraScript : public AuraScript
    {
        void OnTick(AuraEffect const* /*p_AurEff*/)
        {
            Unit* l_Caster = GetCaster();
            if (l_Caster == nullptr)
                return;

            Player* l_Player = l_Caster->ToPlayer();
            if (l_Player == nullptr)
                return;

            l_Caster->ModifyPower(POWER_RUNIC_POWER, -130);
            /*if (l_Caster->ToPlayer())
                l_Caster->ToPlayer()->SendPowerUpdate(POWER_RUNIC_POWER, l_Caster->GetPower(POWER_RUNIC_POWER));*/

            if (l_Caster->GetPower(POWER_RUNIC_POWER) <= 130)
                l_Caster->RemoveAura(SPELL_DK_BREATH_OF_SINDRAGOSA);

        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_breath_of_sindragosa_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_breath_of_sindragosa_AuraScript();
    }
};

//47568 - Empower rune weapon
class spell_dk_empower_rune_weapon : public SpellScriptLoader
{
public:
    spell_dk_empower_rune_weapon() : SpellScriptLoader("spell_dk_empower_rune_weapon") { }

    class spell_dk_empower_rune_weapon_SpellScript : public SpellScript
    {
        void HandleOnHit()
        {
            if (Unit* caster = GetCaster())
            {
                if (Player* player = caster->ToPlayer())
                {
                    for (int i = 0; i < MAX_RUNES; ++i)
                        player->SetRuneCooldown(i, 0);
                    player->ResyncRunes();
                }
            }
        }

        void Register() override
        {
            OnHit += SpellHitFn(spell_dk_empower_rune_weapon_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_dk_empower_rune_weapon_SpellScript();
    }
};

// Runic Empowerment - 81229
class spell_dk_runic_empowerment : public PlayerScript
{
public:
    spell_dk_runic_empowerment() : PlayerScript("spell_dk_runic_empowerment") {}

    enum eSpells
    {
        RunicEmpowerment = 81229,
    };

    void OnModifyPower(Player* p_Player, Powers p_Power, int32 p_OldValue, int32& p_NewValue, bool p_Regen, bool p_After)
    {
        if (p_After)
            return;

        if (p_Player->GetClass() != CLASS_DEATH_KNIGHT || p_Power != POWER_RUNIC_POWER || p_Regen || p_NewValue > p_OldValue)
            return;

        if (AuraEffect* l_RunicEmpowerment = p_Player->GetAuraEffect(eSpells::RunicEmpowerment, EFFECT_0))
        {
            /// 1.00% chance per Runic Power spent
            float l_Chance = (l_RunicEmpowerment->GetAmount() / 100.0f);

            if (roll_chance_f(l_Chance))
            {
                std::list<uint8> l_LstRunesUsed;

                for (uint8 i = 0; i < MAX_RUNES; ++i)
                {
                    if (p_Player->GetRuneCooldown(i))
                        l_LstRunesUsed.push_back(i);
                }

                if (l_LstRunesUsed.empty())
                    return;

                uint8 l_RuneRandom = Trinity::Containers::SelectRandomContainerElement(l_LstRunesUsed);

                p_Player->SetRuneCooldown(l_RuneRandom, 0);
                p_Player->ResyncRunes();
            }
        }
    }
};

// Gorefiend's Grasp - 108199
class spell_dk_gorefiends_grasp : public SpellScriptLoader
{
public:
    spell_dk_gorefiends_grasp() : SpellScriptLoader("spell_dk_gorefiends_grasp") { }

    class spell_dk_gorefiends_grasp_SpellScript : public SpellScript
    {
        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (Player* _player = GetCaster()->ToPlayer())
            {
                if (Unit* target = GetHitUnit())
                {
                    std::list<Unit*> tempList;
                    std::list<Unit*> gripList;

                    _player->GetAttackableUnitListInRange(tempList, 20.0f);

                    for (auto itr : tempList)
                    {
                        if (itr->GetGUID() == _player->GetGUID())
                            continue;

                        if (!_player->IsValidAttackTarget(itr))
                            continue;

                        if (itr->IsImmunedToSpell(GetSpellInfo(), GetCaster()))
                            continue;

                        if (!itr->IsWithinLOSInMap(_player))
                            continue;

                        gripList.push_back(itr);
                    }

                    for (auto itr : gripList)
                    {
                        itr->CastSpell(target, SPELL_DK_DEATH_GRIP_ONLY_JUMP, true);
                        itr->CastSpell(target, SPELL_DK_GOREFIENDS_GRASP_GRIP_VISUAL, true);
                    }
                }
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_dk_gorefiends_grasp_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_dk_gorefiends_grasp_SpellScript();
    }
};

// Bone Shield - 195181
class spell_dk_bone_shield : public SpellScriptLoader
{
public:
    spell_dk_bone_shield() : SpellScriptLoader("spell_dk_bone_shield") { }

    class spell_dk_bone_shield_AuraScript : public AuraScript
    {
        int32 procDelay = 0;

        void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
        {
            amount = -1;
        }

        void Absorb(AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& absorbAmount)
        {
            absorbAmount = 0;
            Unit* target = GetTarget();
            if (!target)
                return;

            int32 absorbPerc = GetSpellInfo()->GetEffect(EFFECT_4).CalcValue(target);
            int32 absorbStack = 1;
            if (AuraEffect* aurEff = target->GetAuraEffect(211078, EFFECT_0)) // Spectral Deflection
            {
                if (target->CountPctFromMaxHealth(aurEff->GetAmount()) < dmgInfo.GetDamage())
                {
                    absorbPerc *= 2;
                    absorbStack *= 2;
                    ModStackAmount(-1);
                }
            }
            if (AuraEffect* aurEff = target->GetAuraEffect(192558, EFFECT_0)) // Skeletal Shattering
            {
                if (Player const* thisPlayer = target->ToPlayer())
                    if (roll_chance_f(thisPlayer->m_activePlayerData->SpellCritPercentage))
                        absorbPerc += aurEff->GetAmount();
            }
            absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPerc);

            if (Player* _player = target->ToPlayer())
            {
                if ((dmgInfo.GetSchoolMask() & SPELL_SCHOOL_MASK_NORMAL) && !procDelay)
                {
                //    if (AuraEffect const* aurEff = _player->GetAuraEffect(251876, EFFECT_0)) // Item - Death Knight T21 Blood 2P Bonus
                  //      _player->GetSpellHistory()->ModifyCooldown(49028, aurEff->GetAmount() * absorbStack);

                    if (_player->HasSpell(221699)) // Blood Tap
                        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(221699, DIFFICULTY_NONE))
                            _player->GetSpellHistory()->ModifyCooldown(221699, Seconds(1000 * spellInfo->GetEffect(EFFECT_1).CalcValue(target) * absorbStack));

                    ModStackAmount(-1);
                    procDelay = 2000;
                }
            }
        }

        void OnStackChange(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            if (!target)
                return;

            if (AuraEffect* aurEff = target->GetAuraEffect(219786, EFFECT_0)) // Ossuary
            {
                if (GetStackAmount() >= aurEff->GetAmount())
                {
                    if (!target->HasAura(219788))
                        target->CastSpell(target, 219788, true);
                }
                else
                    target->RemoveAurasDueToSpell(219788);
            }
        }

        void OnUpdate(uint32 diff)
        {
            if (!procDelay)
                return;

            procDelay -= diff;

            if (procDelay <= 0)
                procDelay = 0;
        }

        void Register() override
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_bone_shield_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_bone_shield_AuraScript::Absorb, EFFECT_0);
            OnEffectApply += AuraEffectApplyFn(spell_dk_bone_shield_AuraScript::OnStackChange, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            OnAuraUpdate += AuraUpdateFn(spell_dk_bone_shield_AuraScript::OnUpdate);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_bone_shield_AuraScript();
    }
};

// Tombstone - 219809
class spell_dk_tombstone : public SpellScriptLoader
{
public:
    spell_dk_tombstone() : SpellScriptLoader("spell_dk_tombstone") { }

    class spell_dk_tombstone_AuraScript : public AuraScript
    {
        void CalcAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
        {
            amount = 0;
            if (Unit* caster = GetCaster())
            {
                if (Aura* aura = caster->GetAura(195181))
                {
                    uint32 stack = aura->GetStackAmount();
                    uint32 maxStack = GetSpellInfo()->GetEffect(EFFECT_4).CalcValue(caster);
                    if (stack > maxStack)
                        stack = maxStack;

                    amount = caster->CountPctFromMaxHealth(GetSpellInfo()->GetEffect(EFFECT_3).CalcValue(caster)) * stack;
                    if (Player* _player = caster->ToPlayer())
                    {
                        if (_player->HasSpell(221699)) // Blood Tap
                            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(221699, DIFFICULTY_NONE))
                                _player->CastSpell(_player, 221699, 1000 * spellInfo->GetEffect(EFFECT_1).CalcValue(caster) * stack);

                        if (AuraEffect const* aurEff = caster->GetAuraEffect(251876, EFFECT_0)) // Item - Death Knight T21 Blood 2P Bonus
                            _player->CastSpell(_player, 49028, aurEff->GetAmount() * stack);

                        aura->ModStackAmount(-1 * stack, AURA_REMOVE_BY_ENEMY_SPELL);
                    }
                }
            }
        }

        void Register() override
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_tombstone_AuraScript::CalcAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_tombstone_AuraScript();
    }
};

// Marrowrend - 195182
class spell_dk_marrowrend : public SpellScript
{
    void HandleOnCast()
    {
        if (Unit* caster = GetCaster())
        {
            caster->CastSpell(nullptr, SPELL_DK_BONE_SHIELD, true);
            if (Aura* boneShield = caster->GetAura(SPELL_DK_BONE_SHIELD))
                boneShield->SetStackAmount(3);
        }
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_dk_marrowrend::HandleOnCast);
    }
};

//206977
class spell_dk_blood_mirror : public SpellScriptLoader
{
public:
    spell_dk_blood_mirror() : SpellScriptLoader("spell_dk_blood_mirror") { }

    class spell_dk_blood_mirror_AuraScript : public AuraScript
    {
        void CalcAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
        {
            amount = -1;
        }

        void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            Unit* caster = GetCaster();
            if (!target || !caster)
                return;
           // if (caster->HasAura(SPELL_DK_BLOOD_MIRROR))
            //    caster->GetAura(SPELL_DK_BLOOD_MIRROR)->VariableStorage.Set("targetGUID", target->GetGUID());
        }

        void HandleAbsorb(AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& absorbAmount)
        {
            absorbAmount = dmgInfo.GetDamage() * (aurEff->GetBaseAmount() / 100);

            Unit* caster = GetCaster();
           // ObjectGuid const procTargetGUID = GetAura()->VariableStorage.GetValue<ObjectGuid>("targetGUID");
          //  if (!caster || procTargetGUID.IsEmpty())
           //     return;

         //   Unit* target = ObjectAccessor::GetUnit(*caster, procTargetGUID);
           // if (!target)
             //   return;

         //   caster->CastCustomSpell(SPELL_DK_BLOOD_MIRROR_DAMAGE, SPELLVALUE_BASE_POINT0, absorbAmount, target, true);
        }

        void Register() override
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_blood_mirror_AuraScript::CalcAmount, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
            AfterEffectApply += AuraEffectApplyFn(spell_dk_blood_mirror_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_blood_mirror_AuraScript::HandleAbsorb, EFFECT_1);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_blood_mirror_AuraScript();
    }
};

//194909 - Frozen Pulse
//Updated to 7.1
class spell_dk_frozen_pulse : public SpellScriptLoader
{
public:
    spell_dk_frozen_pulse() : SpellScriptLoader("spell_dk_frozen_pulse") { }

    class spell_dk_frozen_pulse_AuraScript : public AuraScript
    {
        bool HandleProc(ProcEventInfo& /*procInfo*/)
        {
            Unit* caster = GetCaster();

            if (!caster)
                return false;

            if (caster->GetPower(POWER_RUNES) > GetSpellInfo()->GetEffect(EFFECT_1).BasePoints)
                return false;

            return true;
        }

        void Register() override
        {
            DoCheckProc += AuraCheckProcFn(spell_dk_frozen_pulse_AuraScript::HandleProc);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_frozen_pulse_AuraScript();
    }
};

//194844 - Bonestorm
class spell_dk_bonestorm : public SpellScriptLoader
{
public:
    spell_dk_bonestorm() : SpellScriptLoader("spell_dk_bonestorm") { }

    class spell_dk_bonestorm_AuraScript : public AuraScript
    {
        int32 m_ExtraSpellCost;

        bool Load() override
        {
            Unit* caster = GetCaster();
            if (!caster)
                return false;

            int availablePower = std::min(caster->GetPower(POWER_RUNIC_POWER), 90);

            //Round down to nearest multiple of 10
            m_ExtraSpellCost = availablePower - (availablePower % 10);
            return true;
        }

        void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            int32 m_newDuration = GetDuration() + (m_ExtraSpellCost / 10);
            SetDuration(m_newDuration);

            if (Unit* caster = GetCaster())
            {
                int32 m_newPower = caster->GetPower(POWER_RUNIC_POWER) - m_ExtraSpellCost;
                if (m_newPower < 0)
                    m_newPower = 0;
                caster->SetPower(POWER_RUNIC_POWER, m_newPower);
                /*if (Player* player = caster->ToPlayer())
                    player->SendPowerUpdate(POWER_RUNIC_POWER, m_newPower);*/
            }
        }

        void HandlePeriodic(AuraEffect const* /*aurEff*/)
        {
            Unit* caster = GetCaster();
            if (!caster)
                return;

            caster->CastSpell(caster, SPELL_DK_BONESTORM_HEAL, true);
        }

        void Register() override
        {
            OnEffectApply += AuraEffectApplyFn(spell_dk_bonestorm_AuraScript::HandleApply, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_bonestorm_AuraScript::HandlePeriodic, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dk_bonestorm_AuraScript();
    }
};

// 194913 - Glacial Advance
class spell_dk_glacial_advance : public SpellScript
{
    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();

        Position castPosition = caster->GetPosition();
        Position collisonPos = caster->GetFirstCollisionPosition(GetEffectInfo().CalcRadius(nullptr, SpellTargetIndex::TargetB), DIFFICULTY_NONE);
        float maxDistance = caster->GetDistance(collisonPos);

        for (float dist = 0.0f; dist <= maxDistance; dist += 1.5f)
        {
            caster->GetScheduler().Schedule(Milliseconds(uint32(dist / 1.5f * 50.0f)), [caster, castPosition, dist](TaskContext context)
            {
                Position targetPosition = castPosition;
                caster->MovePosition(targetPosition, dist, 0.f);
                caster->CastSpell(targetPosition, SPELL_DK_GLACIAL_ADVANCE_DAMAGE, true);
            });
        }
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_dk_glacial_advance::HandleHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 49020 - Obliterate
class spell_dk_obliterate : public SpellScript
{
    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        if (GetCaster()->HasAura(SPELL_DK_ICECAP))
            if (GetCaster()->GetSpellHistory()->HasCooldown(SPELL_DK_PILLAR_OF_FROST))
                GetCaster()->GetSpellHistory()->ModifyCooldown(SPELL_DK_PILLAR_OF_FROST, Seconds(-3000));

        if (GetCaster()->HasAura(SPELL_DK_INEXORABLE_ASSAULT_STACK))
            GetCaster()->CastSpell(GetHitUnit(), SPELL_DK_INEXORABLE_ASSAULT_DAMAGE, true);

        if (GetCaster()->HasAura(SPELL_DK_RIME) && roll_chance_f(45))
            GetCaster()->CastSpell(nullptr, SPELL_DK_RIME_BUFF, true);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_dk_obliterate::HandleHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 207317 - Epidemic
class spell_dk_epidemic : public SpellScript
{
    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        if (Unit* target = GetHitUnit())
        {
            if (Aura* aura = target->GetAura(SPELL_DK_VIRULENT_PLAGUE, GetCaster()->GetGUID()))
            {
                target->RemoveAura(aura);
                GetCaster()->CastSpell(target, SPELL_DK_EPIDEMIC_DAMAGE_SINGLE, true);
                GetCaster()->CastSpell(target, SPELL_DK_EPIDEMIC_DAMAGE_AOE, true);
            }
        }

        PreventHitDamage();
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_epidemic::HandleHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 215969 - Epidemic AOE
class spell_dk_epidemic_aoe : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (Unit* target = GetHitUnit())
            if (Unit* caster = GetCaster())
                if (caster->GetDistance2d(target) > 30.0f)
                    targets.remove(target);

        if (targets.size() > 7)
            targets.resize(7);
    }

    void HandleOnHitMain(SpellEffIndex /*effIndex*/)
    {
        if (Unit* target = GetHitUnit())
            explicitTarget = target->GetGUID();
    }

    void HandleOnHitAOE(SpellEffIndex /*effIndex*/)
    {
        if (Unit* target = GetHitUnit())
            if (target->GetGUID() == explicitTarget)
                PreventHitDamage();
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dk_epidemic_aoe::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
        OnEffectHitTarget += SpellEffectFn(spell_dk_epidemic_aoe::HandleOnHitMain, EFFECT_0, SPELL_EFFECT_DUMMY);
        OnEffectHitTarget += SpellEffectFn(spell_dk_epidemic_aoe::HandleOnHitAOE, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
private:
    ObjectGuid explicitTarget;
};

// 191587 - Virulent Plague
class aura_dk_virulent_plague : public AuraScript
{
    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        uint32 eruptionChances = GetEffectInfo(EFFECT_1).BasePoints;
        if (roll_chance_i(eruptionChances))
            GetAura()->Remove(AURA_REMOVE_BY_DEATH);
    }

    void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
        if (removeMode == AURA_REMOVE_BY_DEATH)
            if (Unit* caster = GetCaster())
                caster->CastSpell(GetTarget(), SPELL_DK_VIRULENT_ERUPTION, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_dk_virulent_plague::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        AfterEffectRemove += AuraEffectRemoveFn(aura_dk_virulent_plague::HandleEffectRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

// 152280 - Defile
class aura_dk_defile : public AuraScript
{
    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        if (Unit* caster = GetCaster())
        {
            for (AreaTrigger* at : caster->GetAreaTriggers(GetId()))
            {
                if (at->GetInsideUnits().size())
                    caster->CastSpell(caster, SPELL_DK_DEFILE_MASTERY, true);
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_dk_defile::HandlePeriodic, EFFECT_2, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 55090 - Scourge Strike
class spell_dk_scourge_strike : public SpellScript
{
    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (Unit* target = GetHitUnit())
        {
            if (Aura* festeringWoundAura = target->GetAura(SPELL_DK_FESTERING_WOUND, GetCaster()->GetGUID()))
            {
                if (caster->HasAura(SPELL_DK_UNHOLY_FRENZY))
                    caster->CastSpell(caster, SPELL_DK_UNHOLY_FRENZY_BUFF, true);

                if (Aura* pestilentPustulesAura = caster->GetAura(SPELL_DK_PESTILENT_PUSTULES))
                    if (festeringWoundAura->GetStackAmount() >= pestilentPustulesAura->GetSpellInfo()->GetEffect(EFFECT_0).BasePoints)
                        caster->ModifyPower(POWER_RUNES, 1);

                uint8 festeringWoundBurst = 1;
                if (Aura* castiragorAura = caster->GetAura(SPELL_DK_CASTIGATOR))
                    festeringWoundBurst += castiragorAura->GetSpellInfo()->GetEffect(EFFECT_1).BasePoints;

                festeringWoundBurst = std::min(festeringWoundBurst, festeringWoundAura->GetStackAmount());

                for (uint8 i = 0; i < festeringWoundBurst; ++i)
                {
                    caster->CastSpell(target, SPELL_DK_FESTERING_WOUND_DAMAGE, true);
                    festeringWoundAura->ModStackAmount(-1);
                }
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_scourge_strike::HandleOnHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// Spell 152280
// At 6212
struct at_dk_defile : AreaTriggerAI
{
    at_dk_defile(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) { }

    void OnCreate(Spell const* /*creatingSpell*/) override
    {
        at->GetCaster()->CastSpell(at->GetPosition(), SPELL_DK_SUMMON_DEFILE, true);
    }

    void OnUnitEnter(Unit* unit) override
    {
        if (Unit* caster = at->GetCaster())
            caster->CastSpell(unit, SPELL_DK_DEFILE_DUMMY, true);
    }

    void OnUnitExit(Unit* unit) override
    {
        unit->RemoveAurasDueToSpell(SPELL_DK_DEFILE_DUMMY);
    }
};

// 195758 - Blighted Rune Weapon
class spell_dk_blighted_rune_weapon : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        if (!sSpellMgr->GetSpellInfo(SPELL_DK_FESTERING_WOUND, DIFFICULTY_NONE))
            return false;
        return true;
    }

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        if (Unit* target = GetHitUnit())
            GetCaster()->CastSpell(target, SPELL_DK_FESTERING_WOUND, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_blighted_rune_weapon::HandleHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// Spell 199720
// At 199720
struct at_dk_decomposing_aura : AreaTriggerAI
{
    at_dk_decomposing_aura(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) { }

    void OnUnitExit(Unit* unit) override
    {
        unit->RemoveAurasDueToSpell(SPELL_DK_DECOMPOSING_AURA_DAMAGE, at->GetCasterGuid());
    }
};

enum Misc
{
    NPC_DK_DANCING_RUNE_WEAPON = 27893
};

//207311
class spell_dk_clawing_shadows : public SpellScript
{
    void HandleOnHit()
    {
        Unit* caster = GetCaster();
        Unit* target = caster->ToPlayer()->GetSelectedUnit();

        if (!caster || !target)
            return;

        caster->CastSpell(target, SPELL_DK_FESTERING_WOUND_DAMAGE, true);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_dk_clawing_shadows::HandleOnHit);
    }
};

//82521
struct npc_dk_defile : public ScriptedAI
{
    npc_dk_defile(Creature* creature) : ScriptedAI(creature) {
        SetCombatMovement(false);
        me->SetReactState(REACT_PASSIVE);
        me->SetUnitFlag(UnitFlags(UNIT_FLAG_UNINTERACTIBLE | UNIT_FLAG_NON_ATTACKABLE));
        me->AddUnitState(UNIT_STATE_ROOT);
    }

    void Reset() override
    {
        me->DespawnOrUnsummon(11s);
    }
};

//156004
class spell_dk_defile_aura : public AuraScript
{
    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        Unit* caster = GetCaster();
        if (!target || !caster)
            return;

        caster->GetScheduler().Schedule(1s, [caster, target](TaskContext context)
        {
            if (!target || !caster)
                return;

            caster->CastSpell(target, SPELL_DK_DEFILE_DAMAGE, true);
            if (target->HasAura(156004) && caster)
                context.Repeat(1s);
            else
                context.CancelAll();
        });
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_dk_defile_aura::HandleApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

//5070
struct at_dk_antimagic_zone : AreaTriggerAI
{
    at_dk_antimagic_zone(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) { }

    void OnUnitEnter(Unit* unit) override
    {
        //TODO: Improve unit targets
        if (unit->IsPlayer() && !unit->IsHostileTo(at->GetCaster()))
        {
            if (!unit->HasAura(SPELL_DK_ANTIMAGIC_ZONE_DAMAGE_TAKEN))
                unit->AddAura(SPELL_DK_ANTIMAGIC_ZONE_DAMAGE_TAKEN, unit);
        }
    }

    void OnUnitExit(Unit* unit) override
    {
        if (unit->HasAura(SPELL_DK_ANTIMAGIC_ZONE_DAMAGE_TAKEN))
            unit->RemoveAura(SPELL_DK_ANTIMAGIC_ZONE_DAMAGE_TAKEN);
    }
};

class dark_simulacrum : public PlayerScript
{
public:
    dark_simulacrum() : PlayerScript("dark_simulacrum") { }

    void OnSpellCast(Player* player, Spell* spell, bool) override
    {
        if (player->HasAura(SPELL_DK_DARK_SIMULACRUM))
        {
            if (spell->IsTriggeredByAura(spell->GetSpellInfo()))
                return;

            if (SpellInfo const* spellInfo = spell->GetSpellInfo())
            {
                if (Unit* target = player->GetSelectedUnit())
                {
                    //if (target->IsValidAttackTarget(player))
                    //    target->CastSpell(player, spellInfo, true);
                }
            }
        }
    }
};

//49143
class spell_dk_frost_strike : public SpellScript
{
    void HandleOnHit()
    {
        Unit* caster = GetCaster();
        Unit* target = caster->GetVictim();

        if (!caster || !target)
            return;

        if (caster->HasAura(SPELL_DK_ICECAP))
            if (caster->GetSpellHistory()->HasCooldown(SPELL_DK_PILLAR_OF_FROST))
                caster->GetSpellHistory()->ModifyCooldown(SPELL_DK_PILLAR_OF_FROST, Seconds(-3000));
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_dk_frost_strike::HandleOnHit);
    }
};

//273980
class aura_grip_of_the_dead : public AuraScript
{
    void OnTick(AuraEffect const* /*aurEff*/)
    {
        if (Unit* target = GetTarget())
            if (Unit* caster = GetCaster())
                caster->CastSpell(target, SPELL_DK_GRIP_OF_THE_DEAD_SLOW, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_grip_of_the_dead::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

//253593
class aura_inexorable_assault : public AuraScript
{
    void OnPeriodic(AuraEffect const* /*aurEff*/)
    {
        if (GetCaster())
            GetCaster()->CastSpell(nullptr, SPELL_DK_INEXORABLE_ASSAULT_STACK, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_inexorable_assault::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

//207230
class spell_dk_frostscythe : public SpellScript
{
    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        if (GetCaster()->HasAura(SPELL_DK_INEXORABLE_ASSAULT_STACK))
            GetCaster()->CastSpell(GetHitUnit(), SPELL_DK_INEXORABLE_ASSAULT_DAMAGE, true);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_dk_frostscythe::HandleHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

//47468
class spell_dk_ghoul_claw : public SpellScript
{
    void HandleOnHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetExplTargetUnit();

        if (!caster || !target)
            return;

        if (Unit* owner = caster->GetOwner()->ToPlayer())
        {
            if (owner->HasAura(SPELL_DK_INFECTED_CLAWS))
                if (roll_chance_f(30))
                    caster->CastSpell(target, SPELL_DK_FESTERING_WOUND_DAMAGE, true);
        }
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_dk_ghoul_claw::HandleOnHit);
    }
};

//194311
class spell_dk_festering_wound_damage : public SpellScript
{
    void HandleOnHit()
    {
        if (GetCaster()->HasAura(SPELL_DK_PESTILENT_PUSTULES) && roll_chance_f(10))
            GetCaster()->CastSpell(nullptr, SPELL_DK_RUNIC_CORRUPTION_MOD_RUNES, true);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_dk_festering_wound_damage::HandleOnHit);
    }
};

//207289
class spell_dk_unholy_frenzy : public AuraScript
{
    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        Unit* caster = GetCaster();
        if (!target || !caster)
            return;

        caster->GetScheduler().Schedule(100ms, [caster, target](TaskContext context)
        {
            if (!target || !caster)
                return;

            if (target->HasAura(156004))
                caster->CastSpell(target, SPELL_DK_FESTERING_WOUND_DAMAGE, true);

            if (caster->HasAura(156004))
                context.Repeat(2s);
            else
                context.CancelAll();
        });
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_dk_unholy_frenzy::HandleApply, EFFECT_0, SPELL_AURA_MELEE_SLOW, AURA_EFFECT_HANDLE_REAL);
    }
};

//293891
class spell_dk_death_gate_293891 : public SpellScript
{
    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        Unit* target = GetHitUnit();

        if (Player* player = target->ToPlayer())
        {

                int QUEST_SCARLET_ARMIES_APPROACH = 12757; // only time death gate teles to classic ebon hold.
                int QUEST_LIGHT_OF_DAWN = 12801; // lights hope chappel fight. after this death gate teles to final phase of classic ebon hold.

                /* If player is over level 45 tele to legion ebon hold */
                if (player->GetLevel() >= 45)
                {
                    player->TeleportTo(1220, -1503.367f, 1052.059f, 260.396f, player->GetOrientation()); // legion ebon hold
                }

                /* If on quest 12757 "Scarlet enemies approach" */
                else if ((player->GetQuestStatus(QUEST_LIGHT_OF_DAWN) == QUEST_STATUS_NONE) && (player->GetQuestStatus(QUEST_SCARLET_ARMIES_APPROACH) == QUEST_STATUS_NONE) && (!player->IsAlliedRace()) || (player->HasQuest(QUEST_SCARLET_ARMIES_APPROACH) && (!player->IsAlliedRace())))
                {
                    player->TeleportTo(609, 2368.0444f, -5656.1748f, 382.2804f, player->GetOrientation()); // classic ebon hold
                }

                /* If quest 12801 "Light of Dawn" is completed OR if player is alliedrace*/
                else if ((player->GetQuestStatus(QUEST_LIGHT_OF_DAWN) == QUEST_STATUS_REWARDED) && (player->GetQuestStatus(QUEST_SCARLET_ARMIES_APPROACH) == QUEST_STATUS_REWARDED) || (player->IsAlliedRace()))
                {
                    player->TeleportTo(0, 2368.0444f, -5656.1748f, 382.2804f, player->GetOrientation()); // final phase of classic ebon hold
                }

        }

    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_dk_death_gate_293891::HandleHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 70656 - Advantage (T10 4P Melee Bonus)
class spell_dk_advantage_t10_4p : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (Unit* caster = eventInfo.GetActor())
        {
            Player* player = caster->ToPlayer();
            if (!player || caster->GetClass() != CLASS_DEATH_KNIGHT)
                return false;

            for (uint8 i = 0; i < player->GetMaxPower(POWER_RUNES); ++i)
                if (player->GetRuneCooldown(i) == 0)
                    return false;

            return true;
        }

        return false;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dk_advantage_t10_4p::CheckProc);
    }
};

// 48707 - Anti-Magic Shell
class spell_dk_anti_magic_shell : public AuraScript
{
public:
    spell_dk_anti_magic_shell()
    {
        absorbPct = 0;
        maxHealth = 0;
        absorbedAmount = 0;
    }

    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_DK_RUNIC_POWER_ENERGIZE, SPELL_DK_VOLATILE_SHIELDING })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } });
    }

    bool Load() override
    {
        absorbPct = GetEffectInfo(EFFECT_1).CalcValue(GetCaster());
        maxHealth = GetCaster()->GetMaxHealth();
        absorbedAmount = 0;
        return true;
    }

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        amount = CalculatePct(maxHealth, absorbPct);
    }

    void Trigger(AuraEffect* aurEff, DamageInfo& /*dmgInfo*/, uint32& absorbAmount)
    {
        absorbedAmount += absorbAmount;

        if (!GetTarget()->HasAura(SPELL_DK_VOLATILE_SHIELDING))
        {
            CastSpellExtraArgs args(aurEff);
            args.AddSpellMod(SPELLVALUE_BASE_POINT0, CalculatePct(absorbAmount, 2 * absorbAmount * 100 / maxHealth));
            GetTarget()->CastSpell(GetTarget(), SPELL_DK_RUNIC_POWER_ENERGIZE, args);
        }
    }

    void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (AuraEffect const* volatileShielding = GetTarget()->GetAuraEffect(SPELL_DK_VOLATILE_SHIELDING, EFFECT_1))
        {
            CastSpellExtraArgs args(volatileShielding);
            args.AddSpellMod(SPELLVALUE_BASE_POINT0, CalculatePct(absorbedAmount, volatileShielding->GetAmount()));
            GetTarget()->CastSpell(nullptr, SPELL_DK_VOLATILE_SHIELDING_DAMAGE, args);
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_anti_magic_shell::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        AfterEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_shell::Trigger, EFFECT_0);
        AfterEffectRemove += AuraEffectRemoveFn(spell_dk_anti_magic_shell::HandleEffectRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
    }

private:
    int32 absorbPct;
    int32 maxHealth;
    uint32 absorbedAmount;
};

static uint32 const ArmyTransforms[]
{
    SPELL_DK_ARMY_FLESH_BEAST_TRANSFORM,
    SPELL_DK_ARMY_GEIST_TRANSFORM,
    SPELL_DK_ARMY_NORTHREND_SKELETON_TRANSFORM,
    SPELL_DK_ARMY_SKELETON_TRANSFORM,
    SPELL_DK_ARMY_SPIKED_GHOUL_TRANSFORM,
    SPELL_DK_ARMY_SUPER_ZOMBIE_TRANSFORM
};

// 127517 - Army Transform
/// 6.x, does this belong here or in spell_generic? where do we cast this? sniffs say this is only cast when caster has glyph of foul menagerie.
class spell_dk_army_transform : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_GLYPH_OF_FOUL_MENAGERIE });
    }

    bool Load() override
    {
        return GetCaster()->IsGuardian();
    }

    SpellCastResult CheckCast()
    {
        if (Unit* owner = GetCaster()->GetOwner())
            if (owner->HasAura(SPELL_DK_GLYPH_OF_FOUL_MENAGERIE))
                return SPELL_CAST_OK;

        return SPELL_FAILED_SPELL_UNAVAILABLE;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetCaster(), Trinity::Containers::SelectRandomContainerElement(ArmyTransforms), true);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_dk_army_transform::CheckCast);
        OnEffectHitTarget += SpellEffectFn(spell_dk_army_transform::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 50842 - Blood Boil
class spell_dk_blood_boil : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_BLOOD_PLAGUE });
    }

    void HandleEffect()
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_DK_BLOOD_PLAGUE, true);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_dk_blood_boil::HandleEffect);
    }
};

// 49028 - Dancing Rune Weapon
/// 7.1.5
class spell_dk_dancing_rune_weapon : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        if (!sObjectMgr->GetCreatureTemplate(NPC_DK_DANCING_RUNE_WEAPON))
            return false;
        return true;
    }

    // This is a port of the old switch hack in Unit.cpp, it's not correct
    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Unit* drw = nullptr;
        for (Unit* controlled : caster->m_Controlled)
        {
            if (controlled->GetEntry() == NPC_DK_DANCING_RUNE_WEAPON)
            {
                drw = controlled;
                break;
            }
        }

        if (!drw || !drw->GetVictim())
            return;

        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo)
            return;

        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetDamage())
            return;

        int32 amount = static_cast<int32>(damageInfo->GetDamage()) / 2;
        SpellNonMeleeDamage log(drw, drw->GetVictim(), spellInfo, { spellInfo->GetSpellXSpellVisualId(drw), 0 }, spellInfo->GetSchoolMask());
        log.damage = amount;
        Unit::DealDamage(drw, drw->GetVictim(), amount, nullptr, SPELL_DIRECT_DAMAGE, spellInfo->GetSchoolMask(), spellInfo, true);
        drw->SendSpellNonMeleeDamageLog(&log);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_dk_dancing_rune_weapon::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

// 43265 - Death and Decay
class spell_dk_death_and_decay : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_TIGHTENING_GRASP, SPELL_DK_TIGHTENING_GRASP_SLOW });
    }

    void HandleDummy()
    {
        if (GetCaster()->HasAura(SPELL_DK_TIGHTENING_GRASP))
            if (WorldLocation const* pos = GetExplTargetDest())
                GetCaster()->CastSpell(*pos, SPELL_DK_TIGHTENING_GRASP_SLOW, true);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_dk_death_and_decay::HandleDummy);
    }
};

// 43265 - Death and Decay (Aura)
class spell_dk_death_and_decay_AuraScript : public AuraScript
{
    void HandleDummyTick(AuraEffect const* aurEff)
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(GetTarget(), SPELL_DK_DEATH_AND_DECAY_DAMAGE, aurEff);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_death_and_decay_AuraScript::HandleDummyTick, EFFECT_2, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 47541 - Death Coil
class spell_dk_death_coil : public SpellScript
{
    bool Validate(SpellInfo const* /*spell*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_DEATH_COIL_DAMAGE, SPELL_DK_UNHOLY, SPELL_DK_UNHOLY_VIGOR });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        caster->CastSpell(GetHitUnit(), SPELL_DK_DEATH_COIL_DAMAGE, true);
        if (AuraEffect const* unholyAura = caster->GetAuraEffect(SPELL_DK_UNHOLY, EFFECT_6)) // can be any effect, just here to send SPELL_FAILED_DONT_REPORT on failure
            caster->CastSpell(caster, SPELL_DK_UNHOLY_VIGOR, unholyAura);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_death_coil::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 52751 - Death Gate
class spell_dk_death_gate : public SpellScript
{
    SpellCastResult CheckClass()
    {
        if (GetCaster()->GetClass() != CLASS_DEATH_KNIGHT)
        {
            SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_MUST_BE_DEATH_KNIGHT);
            return SPELL_FAILED_CUSTOM_ERROR;
        }

        return SPELL_CAST_OK;
    }

    void HandleScript(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
        Unit* target = GetHitUnit();
        if (!target)
            return;

        if (Player* player = target->ToPlayer())
        {
            if (player->IsManualRecallPositionValid())
            {
                player->ManualRecall();
                player->ClearManualRecallPosition();
                return;
            }
            else
            {
                player->SaveManualRecallPosition();
            }

            int QUEST_SCARLET_ARMIES_APPROACH = 12757; // only time death gate teles to classic ebon hold.
            int QUEST_LIGHT_OF_DAWN = 12801; // lights hope chappel fight. after this death gate teles to final phase of classic ebon hold.
            int QUEST_THE_BATTLE_FOR_EBON_HOLD = 13166;
            /* If player is over level 45 tele to legion ebon hold */
            if ((player->GetQuestStatus(QUEST_THE_BATTLE_FOR_EBON_HOLD) == QUEST_STATUS_REWARDED))
            {
                player->TeleportTo(1220, -1503.367f, 1052.059f, 260.396f, 3.75f); // legion ebon hold
            }

            /* If on quest 12757 "Scarlet enemies approach" */
            else if ((player->GetQuestStatus(QUEST_LIGHT_OF_DAWN) == QUEST_STATUS_NONE) && (player->GetQuestStatus(QUEST_SCARLET_ARMIES_APPROACH) == QUEST_STATUS_NONE) && (!player->IsAlliedRace())
                || (player->HasQuest(QUEST_SCARLET_ARMIES_APPROACH) || (player->IsAlliedRace())))
            {
                player->TeleportTo(609, 2368.0444f, -5656.1748f, 382.2804f, 3.74f); // classic ebon hold
            }

            /* If quest 12801 "Light of Dawn" is completed*/
            else if ((player->GetQuestStatus(QUEST_LIGHT_OF_DAWN) == QUEST_STATUS_REWARDED) && (player->GetQuestStatus(QUEST_SCARLET_ARMIES_APPROACH) == QUEST_STATUS_REWARDED))
            {
                player->TeleportTo(0, 2368.0444f, -5656.1748f, 382.2804f, 3.735f); // final phase of classic ebon hold
            }
        }
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_dk_death_gate::CheckClass);
        OnEffectHitTarget += SpellEffectFn(spell_dk_death_gate::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 49576 - Death Grip Initial
class spell_dk_death_grip_initial : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
            {
                SPELL_DK_DEATH_GRIP_DUMMY,
                SPELL_DK_DEATH_GRIP_JUMP,
                SPELL_DK_BLOOD,
                SPELL_DK_DEATH_GRIP_TAUNT
            });
    }

    SpellCastResult CheckCast()
    {
        Unit* caster = GetCaster();
        // Death Grip should not be castable while jumping/falling
        if (caster->HasUnitState(UNIT_STATE_JUMPING) || caster->HasUnitMovementFlag(MOVEMENTFLAG_FALLING))
            return SPELL_FAILED_MOVING;

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_DK_DEATH_GRIP_DUMMY, true);
        GetHitUnit()->CastSpell(GetCaster(), SPELL_DK_DEATH_GRIP_JUMP, true);
        if (GetCaster()->HasAura(SPELL_DK_BLOOD))
            GetCaster()->CastSpell(GetHitUnit(), SPELL_DK_DEATH_GRIP_TAUNT, true);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_dk_death_grip_initial::CheckCast);
        OnEffectHitTarget += SpellEffectFn(spell_dk_death_grip_initial::HandleDummy, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 48743 - Death Pact
class spell_dk_death_pact : public AuraScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellEffect({ { spellInfo->Id, EFFECT_2 } });
    }

    void HandleCalcAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        if (Unit* caster = GetCaster())
            amount = int32(caster->CountPctFromMaxHealth(GetEffectInfo(EFFECT_2).CalcValue(caster)));
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_death_pact::HandleCalcAmount, EFFECT_1, SPELL_AURA_SCHOOL_HEAL_ABSORB);
    }
};

// 49998 - Death Strike
class spell_dk_death_strike : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo(
            {
                SPELL_DK_DEATH_STRIKE_ENABLER,
                SPELL_DK_DEATH_STRIKE_HEAL,
                SPELL_DK_BLOOD_SHIELD_MASTERY,
                SPELL_DK_BLOOD_SHIELD_ABSORB,
                SPELL_DK_FROST,
                SPELL_DK_DEATH_STRIKE_OFFHAND
            })
            && ValidateSpellEffect({ { spellInfo->Id, EFFECT_2 } });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();

        if (AuraEffect* enabler = caster->GetAuraEffect(SPELL_DK_DEATH_STRIKE_ENABLER, EFFECT_0, GetCaster()->GetGUID()))
        {
            // Heals you for 25% of all damage taken in the last 5 sec,
            int32 heal = CalculatePct(enabler->CalculateAmount(GetCaster()), GetEffectInfo(EFFECT_1).CalcValue(GetCaster()));
            // minimum 7.0% of maximum health.
            int32 pctOfMaxHealth = CalculatePct(GetEffectInfo(EFFECT_2).CalcValue(GetCaster()), caster->GetMaxHealth());
            heal = std::max(heal, pctOfMaxHealth);

            caster->CastSpell(caster, SPELL_DK_DEATH_STRIKE_HEAL, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, heal));

            if (AuraEffect const* aurEff = caster->GetAuraEffect(SPELL_DK_BLOOD_SHIELD_MASTERY, EFFECT_0))
                caster->CastSpell(caster, SPELL_DK_BLOOD_SHIELD_ABSORB, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_BASE_POINT0, CalculatePct(heal, aurEff->GetAmount())));

            if (caster->HasAura(SPELL_DK_FROST))
                caster->CastSpell(GetHitUnit(), SPELL_DK_DEATH_STRIKE_OFFHAND, true);
        }
    }

    void TriggerRecentlyUsedDeathStrike()
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_DK_RECENTLY_USED_DEATH_STRIKE, true);
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_dk_death_strike::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
        AfterCast += SpellCastFn(spell_dk_death_strike::TriggerRecentlyUsedDeathStrike);
    }
};

// 89832 - Death Strike Enabler - SPELL_DK_DEATH_STRIKE_ENABLER
class spell_dk_death_strike_enabler : public AuraScript
{
    // Amount of seconds we calculate damage over
    constexpr static uint8 LAST_SECONDS = 5;

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return eventInfo.GetDamageInfo() != nullptr;
    }

    void Update(AuraEffect* /*aurEff*/)
    {
        // Move backwards all datas by one from [23][0][0][0][0] -> [0][23][0][0][0]
        std::move_backward(_damagePerSecond.begin(), std::next(_damagePerSecond.begin(), LAST_SECONDS - 1), _damagePerSecond.end());
        _damagePerSecond[0] = 0;
    }

    void HandleCalcAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
    {
        canBeRecalculated = true;
        amount = int32(std::accumulate(_damagePerSecond.begin(), _damagePerSecond.end(), 0u));
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        _damagePerSecond[0] += eventInfo.GetDamageInfo()->GetDamage();
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dk_death_strike_enabler::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_dk_death_strike_enabler::HandleProc, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_death_strike_enabler::HandleCalcAmount, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_dk_death_strike_enabler::Update, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }

private:
    std::array<uint32, LAST_SECONDS> _damagePerSecond = { };
};

// 85948 - Festering Strike
class spell_dk_festering_strike : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_FESTERING_WOUND });
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_DK_FESTERING_WOUND, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_AURA_STACK, GetEffectValue()));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_festering_strike::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

// 47496 - Explode, Ghoul spell for Corpse Explosion
class spell_dk_ghoul_explode : public SpellScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellInfo({ SPELL_DK_CORPSE_EXPLOSION_TRIGGERED }) && ValidateSpellEffect({ { spellInfo->Id, EFFECT_2 } });
    }

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        SetHitDamage(GetCaster()->CountPctFromMaxHealth(GetEffectInfo(EFFECT_2).CalcValue(GetCaster())));
    }

    void Suicide(SpellEffIndex /*effIndex*/)
    {
        if (Unit* unitTarget = GetHitUnit())
        {
            // Corpse Explosion (Suicide)
            unitTarget->CastSpell(unitTarget, SPELL_DK_CORPSE_EXPLOSION_TRIGGERED, true);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_ghoul_explode::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        OnEffectHitTarget += SpellEffectFn(spell_dk_ghoul_explode::Suicide, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 69961 - Glyph of Scourge Strike
class spell_dk_glyph_of_scourge_strike_script : public SpellScript
{
    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();

        Unit::AuraEffectList const& mPeriodic = target->GetAuraEffectsByType(SPELL_AURA_PERIODIC_DAMAGE);
        for (Unit::AuraEffectList::const_iterator i = mPeriodic.begin(); i != mPeriodic.end(); ++i)
        {
            AuraEffect const* aurEff = *i;
            SpellInfo const* spellInfo = aurEff->GetSpellInfo();
            // search our Blood Plague and Frost Fever on target
            if (spellInfo->SpellFamilyName == SPELLFAMILY_DEATHKNIGHT && spellInfo->SpellFamilyFlags[2] & 0x2 &&
                aurEff->GetCasterGUID() == caster->GetGUID())
            {
                uint32 countMin = aurEff->GetBase()->GetMaxDuration();
                uint32 countMax = spellInfo->GetMaxDuration();

                // this Glyph
                countMax += 9000;

                if (countMin < countMax)
                {
                    aurEff->GetBase()->SetDuration(aurEff->GetBase()->GetDuration() + 3000);
                    aurEff->GetBase()->SetMaxDuration(countMin + 3000);
                }
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_glyph_of_scourge_strike_script::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 206940 - Mark of Blood
class spell_dk_mark_of_blood : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_MARK_OF_BLOOD_HEAL });
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        if (Unit* caster = GetCaster())
            caster->CastSpell(eventInfo.GetProcTarget(), SPELL_DK_MARK_OF_BLOOD_HEAL, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_dk_mark_of_blood::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 207346 - Necrosis
class spell_dk_necrosis : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_NECROSIS_EFFECT });
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(eventInfo.GetProcTarget(), SPELL_DK_NECROSIS_EFFECT, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_dk_necrosis::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 207256 - Obliteration
class spell_dk_obliteration : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_OBLITERATION, SPELL_DK_OBLITERATION_RUNE_ENERGIZE, SPELL_DK_KILLING_MACHINE_PROC })
            && ValidateSpellEffect({ { SPELL_DK_OBLITERATION, EFFECT_1 } });
    }

    void HandleProc(AuraEffect* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        Unit* target = GetTarget();
        target->CastSpell(target, SPELL_DK_KILLING_MACHINE_PROC, aurEff);

        if (AuraEffect const* oblitaration = target->GetAuraEffect(SPELL_DK_OBLITERATION, EFFECT_1))
            if (roll_chance_i(oblitaration->GetAmount()))
                target->CastSpell(target, SPELL_DK_OBLITERATION_RUNE_ENERGIZE, aurEff);
    }

    void Register() override
    {
        AfterEffectProc += AuraEffectProcFn(spell_dk_obliteration::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 121916 - Glyph of the Geist (Unholy)
/// 6.x, does this belong here or in spell_generic? apply this in creature_template_addon? sniffs say this is always cast on raise dead.
class spell_dk_pet_geist_transform : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_GLYPH_OF_THE_GEIST });
    }

    bool Load() override
    {
        return GetCaster()->IsPet();
    }

    SpellCastResult CheckCast()
    {
        if (Unit* owner = GetCaster()->GetOwner())
            if (owner->HasAura(SPELL_DK_GLYPH_OF_THE_GEIST))
                return SPELL_CAST_OK;

        return SPELL_FAILED_SPELL_UNAVAILABLE;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_dk_pet_geist_transform::CheckCast);
    }
};

// 147157 Glyph of the Skeleton (Unholy)
/// 6.x, does this belong here or in spell_generic? apply this in creature_template_addon? sniffs say this is always cast on raise dead.
class spell_dk_pet_skeleton_transform : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_GLYPH_OF_THE_SKELETON });
    }

    SpellCastResult CheckCast()
    {
        if (Unit* owner = GetCaster()->GetOwner())
            if (owner->HasAura(SPELL_DK_GLYPH_OF_THE_SKELETON))
                return SPELL_CAST_OK;

        return SPELL_FAILED_SPELL_UNAVAILABLE;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_dk_pet_skeleton_transform::CheckCast);
    }
};

// 61257 - Runic Power Back on Snare/Root
/// 7.1.5
class spell_dk_pvp_4p_bonus : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_RUNIC_RETURN });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo)
            return false;

        return (spellInfo->GetAllEffectsMechanicMask() & ((1 << MECHANIC_ROOT) | (1 << MECHANIC_SNARE))) != 0;
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        eventInfo.GetActionTarget()->CastSpell(nullptr, SPELL_DK_RUNIC_RETURN, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_dk_pvp_4p_bonus::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_dk_pvp_4p_bonus::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 59057 - Rime
class spell_dk_rime : public AuraScript
{
    bool Validate(SpellInfo const* spellInfo) override
    {
        return ValidateSpellEffect({ { spellInfo->Id, EFFECT_1 } }) && ValidateSpellInfo({ SPELL_DK_FROST_SCYTHE });
    }

    bool CheckProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        float chance = static_cast<float>(GetSpellInfo()->GetEffect(EFFECT_1).CalcValue(GetTarget()));
        if (eventInfo.GetSpellInfo()->Id == SPELL_DK_FROST_SCYTHE)
            chance /= 2.f;

        return roll_chance_f(chance);
    }

    void Register() override
    {
        DoCheckEffectProc += AuraCheckEffectProcFn(spell_dk_rime::CheckProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 55233 - Vampiric Blood
class spell_dk_vampiric_blood : public AuraScript
{
    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        amount = GetUnitOwner()->CountPctFromMaxHealth(amount);
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_vampiric_blood::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_INCREASE_HEALTH_2);
    }
};

// 242057 - Rune Empowered
class spell_dk_t20_2p_rune_empowered : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_PILLAR_OF_FROST, SPELL_DK_BREATH_OF_SINDRAGOSA });
    }

    void HandleProc(AuraEffect* /*aurEff*/, ProcEventInfo& procInfo)
    {
        Spell const* procSpell = procInfo.GetProcSpell();
        if (!procSpell)
            return;

        Aura* pillarOfFrost = GetTarget()->GetAura(SPELL_DK_PILLAR_OF_FROST);
        if (!pillarOfFrost)
            return;

        _runicPowerSpent += procSpell->GetPowerTypeCostAmount(POWER_RUNIC_POWER).value_or(0);
        // Breath of Sindragosa special case
        SpellInfo const* breathOfSindragosa = sSpellMgr->AssertSpellInfo(SPELL_DK_BREATH_OF_SINDRAGOSA, DIFFICULTY_NONE);
        if (procSpell->IsTriggeredByAura(breathOfSindragosa))
        {
            auto powerItr = std::find_if(breathOfSindragosa->PowerCosts.begin(), breathOfSindragosa->PowerCosts.end(),
                [](SpellPowerEntry const* power) { return power->PowerType == POWER_RUNIC_POWER && power->PowerPctPerSecond > 0.0f; });
            if (powerItr != breathOfSindragosa->PowerCosts.end())
                _runicPowerSpent += CalculatePct(GetTarget()->GetMaxPower(POWER_RUNIC_POWER), (*powerItr)->PowerPctPerSecond);
        }

        if (_runicPowerSpent >= 600)
        {
            pillarOfFrost->SetDuration(pillarOfFrost->GetDuration() + 1000);
            _runicPowerSpent -= 600;
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_dk_t20_2p_rune_empowered::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }

    int32 _runicPowerSpent = 0;
};

// 46584 - Raise Dead
class spell_dk_raise_dead : public SpellScriptLoader
{
public:
    spell_dk_raise_dead() : SpellScriptLoader("spell_dk_raise_dead") { }

    class spell_dk_raise_dead_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_raise_dead_SpellScript);

    public:
        spell_dk_raise_dead_SpellScript() { }

    private:
        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_DK_RAISE_DEAD, DIFFICULTY_NONE) ||
                !sSpellMgr->GetSpellInfo(SPELL_DK_SLUDGE_BELCHER_ABOMINATION, DIFFICULTY_NONE) ||
                !sSpellMgr->GetSpellInfo(SPELL_DK_SLUDGE_BELCHER_AURA, DIFFICULTY_NONE) ||
                !sSpellMgr->GetSpellInfo(SPELL_DK_RAISE_DEAD_GHOUL, DIFFICULTY_NONE))
                return false;
            return true;
        }

        bool Load() override
        {
            return GetCaster()->GetTypeId() == TYPEID_PLAYER;
        }

        void HandleDummy()
        {
            Unit* caster = GetCaster();

            if (caster->HasAura(SPELL_DK_SLUDGE_BELCHER_AURA))
                caster->CastSpell((Unit*)nullptr, SPELL_DK_SLUDGE_BELCHER_ABOMINATION, true);
            else
            {
                caster->CastSpell((Unit*)nullptr, SPELL_DK_RAISE_DEAD_GHOUL, true);
                if (caster->HasAura(58640)) //Glyph of the Geist
                {
                    if (Guardian* pet = caster->ToPlayer()->GetGuardianPet())
                        pet->CastSpell(pet, SPELL_DK_GEIST_TRANSFORM, true);
                }
            }

            if (caster->HasAura(SPELL_DK_ALL_WILL_SERVE))
            {
                caster->CastSpell(caster, SPELL_DK_ALL_WILL_SERVE_SUMMON, true);
            }
        }

        void Register() override
        {
            OnCast += SpellCastFn(spell_dk_raise_dead_SpellScript::HandleDummy);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_dk_raise_dead_SpellScript();
    }
};

// 55078 - Blood Plague
// Last Update 8.0.1 Build 28153
class spell_dk_blood_plague : public AuraScript
{
    PrepareAuraScript(spell_dk_blood_plague);

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        Unit* owner = GetUnitOwner();
        if (!caster || !owner)
            return;

        if (AuraEffect* aurEff = owner->GetAuraEffect(SPELL_DK_DEEP_CUTS_DEBUFF, EFFECT_0, caster->GetGUID()))
            GetAura()->GetEffect(EFFECT_0)->SetDamage(GetAura()->GetEffect(EFFECT_0)->GetDamage() + aurEff->GetAmount());
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_dk_blood_plague::HandleApply, EFFECT_0, SPELL_AURA_PERIODIC_LEECH, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};

// 196770 - Remorseless Winter
class spell_dk_remorseless_winter : public SpellScript
{
    PrepareSpellScript(spell_dk_remorseless_winter);

    void HandleAfterCast()
    {
        if (Unit* caster = GetCaster())
        {
           // caster->_uniqueBitingColdTargets.clear(); todo thor
            caster->Variables.Remove("BitingCold");
            if (caster->HasAura(SPELL_DK_FROZEN_CENTER))
                caster->CastSpell(caster->GetPosition(), SPELL_DK_FROZEN_CENTER_ROOT, true);
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_dk_remorseless_winter::HandleAfterCast);
    }
};

// 222024 - Obliterate Main Hand
class spell_dk_obliterate_main_hand : public SpellScript
{
    PrepareSpellScript(spell_dk_obliterate_main_hand);

    void HandleCritChance(Unit const* /*victim*/, float& chance)
    {
        if (Unit* caster = GetCaster())
            if (caster->HasAura(SPELL_DK_KILLING_MACHINE))
                chance = 100.f;
    }

    void HandleHit()
    {
        if (Unit* caster = GetCaster())
        {
            if (Player* player = caster->ToPlayer())
                if (!player->GetEquippedItem(EQUIPMENT_SLOT_OFFHAND))
                    player->RemoveAurasDueToSpell(SPELL_DK_KILLING_MACHINE);

            if (caster->HasAura(SPELL_DK_RIME) && roll_chance_f(sSpellMgr->GetSpellInfo(SPELL_DK_RIME, DIFFICULTY_NONE)->GetEffect(EFFECT_1).BasePoints))
                caster->CastSpell(caster, SPELL_DK_RIME_BUFF, true);

            // Icecap
            if (caster->HasAura(SPELL_DK_ICECAP) && IsHitCrit())
            {
                caster->Variables.Set<bool>("IC_HANDLE_CRIT", true);
                caster->GetSpellHistory()->ModifyCooldown(SPELL_DK_PILLAR_OF_FROST, -1s);
            }
        }
    }

    void Register() override
    {
        OnCalcCritChance += SpellOnCalcCritChanceFn(spell_dk_obliterate_main_hand::HandleCritChance);
        OnHit += SpellHitFn(spell_dk_obliterate_main_hand::HandleHit);
    }
};

// 66198 - Obliterate Off Hand
class spell_dk_obliterate_off_hand : public SpellScript
{
    PrepareSpellScript(spell_dk_obliterate_off_hand);

    void HandleCritChance(Unit const* /*victim*/, float& chance)
    {
        if (Unit* caster = GetCaster())
            if (caster->HasAura(SPELL_DK_KILLING_MACHINE))
                chance = 100.f;
    }

    void HandleHit()
    {
        if (Unit* caster = GetCaster())
        {
            // Icecap
            if (caster->HasAura(SPELL_DK_ICECAP) && IsHitCrit() && !caster->Variables.Exist("IC_HANDLE_CRIT"))
                caster->GetSpellHistory()->ModifyCooldown(SPELL_DK_PILLAR_OF_FROST, -1s);

            caster->Variables.Remove("IC_HANDLE_CRIT");
            caster->RemoveAurasDueToSpell(SPELL_DK_KILLING_MACHINE);
        }
    }

    void Register() override
    {
        OnCalcCritChance += SpellOnCalcCritChanceFn(spell_dk_obliterate_off_hand::HandleCritChance);
        OnHit += SpellHitFn(spell_dk_obliterate_off_hand::HandleHit);
    }
};

enum eLegendary
{
    DeathsEmbrace = 334728,
    DeathsEmbraceHeal = 334729,
    GripOfTheEverlasting = 334724,
    GripOfTheEverlastingBuff = 334722,
    Phearomones = 335177,
    DeathTurf = 335180,
    Superstrain = 334974,

    // Blood
    CrimsonRuneWeapon = 334525,
    CrimsonRuneWeaponProc = 334526,
    GorefiendsDomination = 334580,
    GorefiendsDominationEnergize = 350914,
    VampiricAura = 334547,
    VampiricAuraBuff = 334555,

    // Frost
    AbsoluteZero = 334692,
    AbsoluteZeroStun = 334693,
    BitingCold = 334678,
    RageOfTheFrozenChampion = 341724,
    RageOfTheFrozenChampionEnergize = 341725,

    // Unholy
    FrenziedMontrosity = 334888,
    FrenziedMontrosityBuff = 334895,
};

enum eBlood
{
    BloodBoil = 50842,
    DancingRuneWeapon = 49028,
    DeathsCaress = 195292,
    GorefiendsGrasp = 108199,
    HeartStrike = 206930,
    Marrowrend = 195182,
    RuneTap = 194679,
    VampricBlood = 55233,
    CrimsonScourge = 81136,
    MasteryBloodShield = 77513,
    Ossuary = 219786,
};

//spell heart strike 206930
//Last Update 8.0.1 Build 28153
class spell_dk_heart_strike : public SpellScript
{
    PrepareSpellScript(spell_dk_heart_strike);

    void OnHandleEffectHit(SpellEffIndex /*effIndex*/)
    {
        Player* caster = GetCaster()->ToPlayer();
        Unit* target = GetHitUnit();
        Unit* mainTarget = GetExplTargetUnit();
        if (!caster || !target)
            return;

        if (caster->HasAura(SPELL_AURA_DK_HEARTBREAKER))
            if (mainTarget != target)
                caster->CastSpell(target, SPELL_DK_BLOD_HEARTBREAKER_ENERGIZE);

        if (AuraEffect* aurEff = caster->GetAuraEffect(SPELL_DK_DEEP_CUTS_POWER, EFFECT_0))
            caster->CastSpell(caster, SPELL_DK_DEEP_CUTS_DEBUFF, aurEff->GetAmount());

        if (caster->HasAura(GorefiendsDomination))
            caster->GetSpellHistory()->ModifyCooldown(VampricBlood, -2s);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_heart_strike::OnHandleEffectHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 207200 - Permafrost
class aura_dk_permafrost : public AuraScript
{
    PrepareAuraScript(aura_dk_permafrost);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (eventInfo.GetDamageInfo() && (eventInfo.GetDamageInfo()->GetAttackType() == BASE_ATTACK || eventInfo.GetDamageInfo()->GetAttackType() == OFF_ATTACK))
            return true;

        return false;
    }

    void HandleProc(ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* caster = GetCaster();
        if (eventInfo.GetDamageInfo() && caster)
        {
            uint32 shield = CalculatePct(eventInfo.GetDamageInfo()->GetDamage(), sSpellMgr->GetSpellInfo(SPELL_DK_PERMAFROST, DIFFICULTY_NONE)->GetEffect(EFFECT_0).BasePoints);
            caster->CastSpell(caster, SPELL_DK_FROST_SHIELD, shield);
        }
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_dk_permafrost::CheckProc);
        OnProc += AuraProcFn(aura_dk_permafrost::HandleProc);
    }
};

// 194912 - Gathering Storm
class aura_gathering_storm : public AuraScript
{
    PrepareAuraScript(aura_gathering_storm);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return false;

        if (eventInfo.GetSpellInfo())
        {
            bool spells = (eventInfo.GetSpellInfo()->Id == SPELL_DK_OBLITERATE || eventInfo.GetSpellInfo()->Id == SPELL_DK_CHAINS_OF_ICE || eventInfo.GetSpellInfo()->Id == SPELL_DK_PATH_OF_FROST ||
                eventInfo.GetSpellInfo()->Id == SPELL_DK_CONTROL_UNDEATH || eventInfo.GetSpellInfo()->Id == SPELL_DK_HOWLING_BLAST);
            if (spells && caster->HasAura(SPELL_DK_REMORSELESS_WINTER))
                return true;
        }
        return false;
    }

    void HandleProc(ProcEventInfo& eventInfo)
    {
        if (Unit* caster = GetCaster())
        {
            uint8 runes = 1;

            if (eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_DK_OBLITERATE)
                runes = 2;

            if (Aura* aura = caster->GetAura(SPELL_DK_REMORSELESS_WINTER))
                aura->ModDuration(runes * IN_MILLISECONDS, true);

            for (uint8 i = 0; i < runes; i++)
                caster->CastSpell(caster, SPELL_DK_GATHERING_STORM_DAMAGE, true);
        }
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_gathering_storm::CheckProc);
        OnProc += AuraProcFn(aura_gathering_storm::HandleProc);
    }
};

// 194310 - Festering Wound
class aura_festering_wound : public AuraScript
{
    PrepareAuraScript(aura_festering_wound);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        Unit* owner = GetUnitOwner();
        if (!caster || !owner)
            return false;

        if (eventInfo.GetHealInfo() && eventInfo.GetHealInfo()->GetHeal() > 0 && caster->HasAura(SPELL_DK_CRYPT_FEVER))
            return true;

        return false;
    }

    void HandleProc(ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        Unit* owner = GetUnitOwner();
        if (!caster || !owner)
            return;

        caster->CastSpell(owner, SPELL_DK_CRYPT_FEVER_DAMAGE, true);
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetUnitOwner();

        if (!caster || !target)
            return;

        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
        {
            caster->CastSpell(target, SPELL_DK_FESTERING_WOUND_DAMAGE, true);
            if (caster->HasAura(SPELL_DK_BURSTING_SORES))
                caster->CastSpell(target, SPELL_DK_BURSTING_SORES_EXPLOSION, true);
        }
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_festering_wound::CheckProc);
        OnProc += AuraProcFn(aura_festering_wound::HandleProc);
        OnEffectRemove += AuraEffectRemoveFn(aura_festering_wound::HandleRemove, EFFECT_0, SPELL_EFFECT_APPLY_AURA, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};

// 275699 - Apocalypse
class spell_dk_apocalypse : public SpellScript
{
    PrepareSpellScript(spell_dk_apocalypse);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DK_FESTERING_WOUND, SPELL_DK_ARMY_OF_DEATH });
    }

    void HandleEffectHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        if (Aura* aura = target->GetAura(SPELL_DK_FESTERING_WOUND))
        {
            uint8 stacks = (aura->GetStackAmount() > 4) ? sSpellMgr->GetSpellInfo(SPELL_DK_APOCALYPSE, DIFFICULTY_NONE)->GetEffect(EFFECT_1).BasePoints : aura->GetStackAmount();
            for (uint8 i = 0; i < stacks; i++)
            {
                caster->CastSpell(target, SPELL_DK_FESTERING_WOUND_DAMAGE, true);
                caster->CastSpell(caster, SPELL_DK_ARMY_OF_DEATH, true);
            }
            aura->ModStackAmount(-stacks);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_apocalypse::HandleEffectHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 24207 - Ghoul NPC
class npc_pet_dk_ghoul : public ScriptedAI
{
public:
    npc_pet_dk_ghoul(Creature* creature) : ScriptedAI(creature) {}

    enum Events
    {
        EVENT_ATTACK = 1,
    };

    EventMap events;

    void IsSummonedBy(WorldObject* owner) override
    {
        if (!owner)
            return;

        events.ScheduleEvent(EVENT_ATTACK, 4500ms);
    }

    void UpdateAI(uint32 diff) override
    {
        Unit* owner = me->GetOwner();
        if (!owner)
            return;

        if (attack && !owner->IsInCombat())
        {
            EnterEvadeMode(EVADE_REASON_NO_HOSTILES);
            return;
        }

        events.Update(diff);

        Unit* target = GetTarget();
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_ATTACK:
                attack = true;
                break;
            }
        }

        if (!attack)
            return;

        ObjectGuid newtargetGUID = owner->GetTarget();
        if (newtargetGUID.IsEmpty() || newtargetGUID == targetGUID)
        {
            me->GetAI()->AttackStart(target);
            DoMeleeAttackIfReady();
            return;
        }

        if (Unit* newTarget = ObjectAccessor::GetUnit(*me, newtargetGUID))
            if (target != newTarget && me->IsValidAttackTarget(newTarget))
                target = newTarget;

        me->GetAI()->AttackStart(target);
        DoMeleeAttackIfReady();
    }

    void EnterEvadeMode(EvadeReason /*reason*/) override
    {
        if (!attack)
            return;

        if (me->IsInEvadeMode() || !me->IsAlive())
            return;

        Unit* owner = me->GetOwner();

        me->CombatStop(true);
        if (owner && !me->HasUnitState(UNIT_STATE_FOLLOW))
        {
            me->GetMotionMaster()->Clear();
            me->FollowTarget(owner);
        }
    }

    void JustUnsummoned() override
    {
        if (Unit* owner = me->GetOwner())
            if (Aura* aura = owner->GetAura(SPELL_DK_LAST_SURPRISE_POWER))
                me->CastSpell(me ,SPELL_DK_LAST_SURPRISE_DAMAGE, aura->GetEffect(0)->GetAmount());
    }

private:
    Unit* GetTarget() const
    {
        return ObjectAccessor::GetUnit(*me, targetGUID);
    }

    ObjectGuid targetGUID;
    bool attack = false;
};

// 106041 - Zombie NPC
//Last Update 8.0.1 Build 28153
class npc_pet_dk_zombie : public ScriptedAI
{
public:
    npc_pet_dk_zombie(Creature* creature) : ScriptedAI(creature) {}

    void IsSummonedBy(WorldObject* owner) override
    {
        if (!owner)
            return;

        SpellInfo const* reanimation = sSpellMgr->GetSpellInfo(SPELL_DK_REANIMATION, DIFFICULTY_NONE);
        if (!reanimation)
            return;

        targetGUID = owner->Variables.GetValue<ObjectGuid>("ZOMBIE_TARGET", ObjectGuid::Empty);
        me->SetMaxHealth(reanimation->GetEffect(EFFECT_2).BasePoints);
        me->SetReactState(REACT_PASSIVE);
        me->SetControlled(true, UNIT_STATE_ROOT);

        Unit* target = GetTarget();
        if (target && owner->IsValidAttackTarget(target))
        {
            me->GetScheduler().Schedule(1s, [this](TaskContext context)
                {
                    if (Creature* creature = GetContextCreature())
                    {
                        if (auto t = GetTarget())
                        {
                            me->SetControlled(false, UNIT_STATE_ROOT);
                            creature->EngageWithTarget(t);
                            creature->SetWalk(true);
                            for (uint8 move = MOVE_WALK; move < MAX_MOVE_TYPE; move++)
                                me->SetSpeedRate((UnitMoveType)move, 0.5f);
                            creature->GetMotionMaster()->Clear();
                            chase = true;
                        }
                    }
                });
        }
    }

    void UpdateAI(uint32 diff) override
    {
        Unit* owner = me->GetOwner();
        if (!owner || !chase)
            return;

        Unit* target = GetTarget();
        if (!target || (target && !me->IsValidAttackTarget(target)))
        {
            chase = false;
            me->DespawnOrUnsummon(1s);
            return;
        }
        else
        {
            me->EngageWithTarget(target);
            me->GetMotionMaster()->MoveChase(target);
            // Explode when reach target.
            if (me->GetDistance(target) <= 2.0f)
            {
                me->CastSpell(target, SPELL_DK_REANIMATION_DAMAGE, true);
                me->KillSelf(false);
                me->DespawnOrUnsummon(2s);
            }
        }
    }

private:
    Unit* GetTarget() const
    {
        return ObjectAccessor::GetUnit(*me, targetGUID);
    }

    ObjectGuid targetGUID;
    bool chase = false;
};

// 52212 - Death and Decay damage
//Last Update 8.0.1 Build 28153
class spell_dk_death_and_decay_damage : public SpellScript
{
    PrepareSpellScript(spell_dk_death_and_decay_damage);

    void HandleEffectHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        if (AuraEffect* aurEff = target->GetAuraEffect(SPELL_DK_GRIP_OF_THE_DEAD_SLOW, EFFECT_0))
            aurEff->RecalculateAmount();

        if (Aura* aura = caster->GetAura(SPELL_DK_PESTILENCE))
            if (roll_chance_i(aura->GetEffect(0)->GetAmount()))
                caster->CastSpell(target, SPELL_DK_FESTERING_WOUND, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_death_and_decay_damage::HandleEffectHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 276837 - Army of the Damned
class aura_dk_army_of_the_damned : public AuraScript
{
    PrepareAuraScript(aura_dk_army_of_the_damned);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_DK_DEATH_COIL)
            return true;

        return false;
    }

    void HandleProc(ProcEventInfo& eventInfo)
    {
        if (Unit* caster = GetCaster())
        {
            caster->GetSpellHistory()->ModifyCooldown(SPELL_DK_ARMY_OF_DEATH_TRIGGER, -5s);
            caster->GetSpellHistory()->ModifyCooldown(SPELL_DK_APOCALYPSE, -1s);
        }
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_dk_army_of_the_damned::CheckProc);
        OnProc += AuraProcFn(aura_dk_army_of_the_damned::HandleProc);
    }
};

// 253595 - Inexorable Assault Aura
class aura_dk_inexorable_assault_aura : public AuraScript
{
    PrepareAuraScript(aura_dk_inexorable_assault_aura);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        Unit* target = eventInfo.GetActionTarget();
        if (!caster || !target)
            return true;

        if (eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_DK_OBLITERATE)
        {
            if (Aura* aura = caster->GetAura(SPELL_DK_INEXORABLE_ASSAULT))
            {
                aura->ModStackAmount(-1);
                caster->CastSpell(target, SPELL_DK_INEXORABLE_ASSAULT_DAMAGE, true);
            }
        }

        return false;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_dk_inexorable_assault_aura::CheckProc);
    }
};

// 281208 - Cold Heart
class aura_dk_cold_heart : public AuraScript
{
    PrepareAuraScript(aura_dk_cold_heart);

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
            caster->RemoveAura(SPELL_DK_COLD_HEART);
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(aura_dk_cold_heart::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

// 207142 - Avalanche
class aura_dk_avalance : public AuraScript
{
    PrepareAuraScript(aura_dk_avalance);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (Unit* caster = GetCaster())
            if (eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_DK_HOWLING_BLAST && caster->HasAura(SPELL_DK_RIME_BUFF))
                return true;

        return false;
    }

    void HandleProc(ProcEventInfo& eventInfo)
    {
        Unit* caster = GetCaster();
        Unit* target = eventInfo.GetActionTarget();
        if (!caster || !target)
            return;

        caster->CastSpell(target, SPELL_DK_AVALANCHE_DAMAGE, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_dk_avalance::CheckProc);
        OnProc += AuraProcFn(aura_dk_avalance::HandleProc);
    }
};

// 196528 - Bonestorm Damage
class spell_dk_bonestorm_damage : public SpellScript
{
    PrepareSpellScript(spell_dk_bonestorm_damage);

    void HandleOnHit()
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(caster, SPELL_DK_BONESTORM_HEAL, true);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_dk_bonestorm_damage::HandleOnHit);
    }
};

// 273946 - Hemostasis
class aura_dk_hemostatis : public AuraScript
{
    PrepareAuraScript(aura_dk_hemostatis);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (eventInfo.GetSpellInfo() && eventInfo.GetSpellInfo()->Id == SPELL_DK_BLOOD_BOIL)
            return true;

        return false;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_dk_hemostatis::CheckProc);
    }
};

// 273947 - Hemostasis Aura
class aura_dk_hemostatis_aura : public AuraScript
{
    PrepareAuraScript(aura_dk_hemostatis_aura);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        return false;
    }

    void HandleProc(ProcEventInfo& eventInfo)
    {
        if (Unit* caster = GetCaster())
            caster->RemoveAura(SPELL_DK_HEMOSTATIS_AURA);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_dk_hemostatis_aura::CheckProc);
    }
};

// 223829 - Necrotic Strike
class spell_dk_necrotic_strike : public SpellScript
{
    PrepareSpellScript(spell_dk_necrotic_strike);

    void HandleEffectHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        if (Aura* aura = target->GetAura(SPELL_DK_FESTERING_WOUND, caster->GetGUID()))
        {
            int32 amount = CalculatePct(caster->GetTotalAttackPowerValue(BASE_ATTACK), sSpellMgr->GetSpellInfo(SPELL_DK_NECROTIC_WOUND, DIFFICULTY_NONE)->GetEffect(EFFECT_1).BasePoints);
            caster->CastSpell(target, SPELL_DK_NECROTIC_WOUND, amount);
            aura->ModStackAmount(-1);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_necrotic_strike::HandleEffectHit, EFFECT_2, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 191685 - Virulent Plage Damage Last Update 8.0.1 Build 28153
class spell_dk_virulent_plage_damage : public SpellScript
{
    PrepareSpellScript(spell_dk_virulent_plage_damage);

    void HandleEffectHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        // Divide damage by nearby enemy count.
        UnitList list;
        target->GetFriendlyUnitListInRange(list, 8.0f);
        int32 damage = GetHitDamage() / ((list.size() > 0) ? list.size() : 1);
        if (caster->HasAura(SPELL_DK_UNHOLY_MUTATION) && caster->Variables.Exist("MutationDamage"))
        {
            caster->Variables.Remove("MutationDamage");
            damage += CalculatePct(damage, sSpellMgr->GetSpellInfo(SPELL_DK_UNHOLY_MUTATION, DIFFICULTY_NONE)->GetEffect(EFFECT_0).BasePoints);
        }

        SetHitDamage(damage);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_virulent_plage_damage::HandleEffectHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 210128 - Reanimation
class spell_dk_reanimation : public SpellScript
{
    PrepareSpellScript(spell_dk_reanimation);

    void HandlePrepare()
    {
        if (Unit* caster = GetCaster())
        {
            if (caster->GetTarget() != ObjectGuid::Empty)
            {
                if (Unit* target = ObjectAccessor::GetUnit(*caster, caster->GetTarget()))
                    caster->SetInCombatWith(target);
                if (auto zombieTarget = GetExplTargetUnit())
                    caster->Variables.Set<ObjectGuid>("ZOMBIE_TARGET", zombieTarget->GetGUID());
            }
        }
    }

    void Register() override
    {
        OnPrepare += SpellOnPrepareFn(spell_dk_reanimation::HandlePrepare);
    }
};

// 233395 - Frozen Center
class aura_dk_frozen_center : public AuraScript
{
    PrepareAuraScript(aura_dk_frozen_center);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (!eventInfo.GetDamageInfo())
            return false;

        Aura* aura = GetAura();
        int chance = 50;
        // Blizzard mech.
        if (aura && aura->GetDuration() / 1000 >= aura->GetMaxDuration() / 2000 && roll_chance_i(chance))
            return true;
        else if (aura && aura->GetDuration() / 1000 < aura->GetMaxDuration() / 2000)
        {
            uint32 chanceAdd = (aura->GetMaxDuration() / 2000 - aura->GetDuration() / 1000) * 5 + 5;
            chance += chanceAdd;
            if (roll_chance_i(chance))
                return true;
        }

        return false;
    }

    void HandleProc(ProcEventInfo& eventInfo)
    {
        if (WorldObject* owner = GetOwner())
            if (owner->ToUnit())
                owner->ToUnit()->RemoveAura(SPELL_DK_FROZEN_CENTER_ROOT);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_dk_frozen_center::CheckProc);
        OnProc += AuraProcFn(aura_dk_frozen_center::HandleProc);
    }
};

// 203173 - Death Chain
class spell_dk_death_chain : public SpellScript
{
    PrepareSpellScript(spell_dk_death_chain);

    void HandleHitTarget(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        Unit* expTarget = GetExplTargetUnit();
        if (!caster || !target || !expTarget)
            return;

        if (target->GetGUID() != expTarget->GetGUID())
            expTarget->CastSpell(target, SPELL_DK_DEATH_CHAIN_VISUAL, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_death_chain::HandleHitTarget, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// 233396 - Delirium
class aura_dk_delirium : public AuraScript
{
    PrepareAuraScript(aura_dk_delirium);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (eventInfo.GetSpellInfo() && (eventInfo.GetSpellInfo()->Id == SPELL_DK_HOWLING_BLAST || eventInfo.GetSpellInfo()->Id == SPELL_DK_HOWLING_BLAST_AOE ||
            eventInfo.GetSpellInfo()->Id == SPELL_DK_FROST_STRIKE_MAIN_HAND))
            return true;

        return false;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_dk_delirium::CheckProc);
    }
};

// 199969 - Wandering Plague
class aura_dk_wandering_plague : public AuraScript
{
    PrepareAuraScript(aura_dk_wandering_plague);

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_ENEMY_SPELL && GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        // Find nearby enemy to infect.
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        if (!caster || !target)
            return;

        if (caster->Variables.Exist("WP_JUMPS") && caster->Variables.GetValue<uint8>("WP_JUMPS") < 3)
        {
            caster->Variables.Set<ObjectGuid>("WP_MAIN", target->GetGUID());
            caster->CastSpell(target, SPELL_DK_WANDERING_PLAGUE_SELECTOR, true);
            uint8 jump = caster->Variables.GetValue<uint8>("WP_JUMPS") + 1;
            caster->Variables.Set("WP_JUMPS", jump);
        }
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectApplyFn(aura_dk_wandering_plague::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

// 200093 - Wandering Plague Selector
class spell_dk_wandering_plague_selector : public SpellScript
{
    PrepareSpellScript(spell_dk_wandering_plague_selector);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (Unit* caster = GetCaster())
        {
            targets.remove_if([caster](WorldObject* obj)
                {
                    if (caster->Variables.Exist("WP_MAIN") && obj->GetGUID() == caster->Variables.GetValue<ObjectGuid>("WP_MAIN"))
                        return true;

                    if (!obj->ToUnit())
                        return true;

                    if (!caster->IsValidAttackTarget(obj->ToUnit()))
                        return true;

                    if (!obj->ToUnit()->HasAura(SPELL_DK_VIRULENT_PLAGUE, caster->GetGUID()) &&
                        !obj->ToUnit()->HasAura(SPELL_DK_OUTBREAK_PERIODIC, caster->GetGUID()) &&
                        !obj->ToUnit()->HasAura(SPELL_DK_UNHOLY_BLIGHT, caster->GetGUID()))
                        return true;

                    return false;
                });

            if (targets.size() > 0)
                Trinity::Containers::RandomResize(targets, 1);
        }
    }

    void HandleHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        caster->CastSpell(target, SPELL_DK_WANDERING_PLAGUE_DISEASE, true);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dk_wandering_plague_selector::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnHit += SpellHitFn(spell_dk_wandering_plague_selector::HandleHit);
    }
};

void AddSC_DekkCore_deathknight_spell_scripts()
{
    RegisterSpellScript(spell_dk_wandering_plague_selector);
    RegisterSpellScript(aura_dk_wandering_plague);
    RegisterSpellScript(aura_dk_delirium);
    RegisterSpellScript(spell_dk_death_chain);
    RegisterSpellScript(aura_dk_frozen_center);
    RegisterSpellScript(spell_dk_reanimation);
    RegisterSpellScript(spell_dk_virulent_plage_damage);
    RegisterSpellScript(spell_dk_necrotic_strike);
    RegisterSpellScript(aura_dk_hemostatis_aura);
    RegisterSpellScript(aura_dk_hemostatis);
    RegisterSpellScript(spell_dk_bonestorm_damage);
    RegisterSpellScript(aura_dk_avalance);
    RegisterSpellScript(aura_dk_cold_heart);
    RegisterSpellScript(aura_dk_inexorable_assault_aura);
    RegisterSpellScript(aura_dk_army_of_the_damned);
    RegisterSpellScript(spell_dk_death_and_decay_damage);
    RegisterSpellScript(spell_dk_apocalypse);
    RegisterSpellScript(aura_festering_wound);
    RegisterSpellScript(aura_gathering_storm);
    RegisterSpellScript(aura_dk_permafrost);
    RegisterSpellScript(spell_dk_heart_strike);
    RegisterSpellScript(spell_dk_obliterate_off_hand);
    RegisterSpellScript(spell_dk_obliterate_main_hand);
    RegisterSpellScript(spell_dk_remorseless_winter);
    RegisterSpellScript(spell_dk_blood_plague);
    new spell_dk_anti_magic_barrier();
    new spell_dk_anti_magic_shell_self();
    new spell_dk_army_periodic_taunt();
    new spell_dk_blood_mirror();
    new spell_dk_breath_of_sindragosa();
    new spell_dk_bonestorm();
    new spell_dk_bone_shield();
    new spell_dk_chains_of_ice();
    RegisterSpellScript(spell_dk_chilblains);
    new spell_dk_dark_transformation_form();
    new spell_dk_death_siphon();

    new spell_dk_desecrated_ground();
    new spell_dk_empower_rune_weapon();
    new spell_dk_frozen_pulse();
    new spell_dk_gorefiends_grasp();
    RegisterSpellScript(spell_dk_howling_blast);
    new spell_dk_icebound_fortitude();
    RegisterSpellScript(spell_dk_icy_touch);
    RegisterSpellScript(spell_dk_marrowrend);
    RegisterSpellScript(spell_dk_outbreak);
    RegisterSpellScript(aura_dk_outbreak_periodic);
    new spell_dk_pillar_of_frost();
    new spell_dk_plague_leech();
    new spell_dk_blood_shield();
    new spell_dk_presence();
    new spell_dk_purgatory();
    new spell_dk_purgatory_absorb();
    RegisterSpellScript(spell_dk_item_t17_frost_4p_driver);
    new spell_dk_item_t17_frost_4p_driver_periodic();
    RegisterSpellScript(spell_dk_remorseless_winter_damage);
    new spell_dk_runic_empowerment(); //NOT WORKING - Need implementation on PlayerScript :)
    RegisterSpellScript(spell_dk_unholy_blight);
    new spell_dk_tombstone();
    RegisterSpellScript(spell_dk_will_of_the_necropolis);
    RegisterSpellScript(spell_dk_glacial_advance);
    RegisterSpellScript(spell_dk_obliterate);
    RegisterSpellScript(spell_dk_epidemic);
    RegisterSpellScript(spell_dk_epidemic_aoe);
    RegisterSpellScript(aura_dk_virulent_plague);
    RegisterSpellScript(spell_dk_scourge_strike);
    RegisterSpellScript(aura_dk_defile);
    RegisterAreaTriggerAI(at_dk_defile);
    RegisterSpellScript(spell_dk_blighted_rune_weapon);
    RegisterAreaTriggerAI(at_dk_decomposing_aura);
    new spell_dk_crimsom_scourge();
    RegisterSpellScript(spell_dk_clawing_shadows);
    RegisterCreatureAI(npc_dk_defile);
    RegisterSpellScript(spell_dk_defile_aura);
    RegisterAreaTriggerAI(at_dk_antimagic_zone);
    RegisterPlayerScript(dark_simulacrum);
    RegisterSpellScript(spell_dk_frost_strike);
    RegisterSpellScript(aura_grip_of_the_dead);
    RegisterSpellScript(aura_inexorable_assault);
    RegisterSpellScript(spell_dk_frostscythe);
    RegisterSpellScript(spell_dk_ghoul_claw);
    RegisterSpellScript(spell_dk_festering_wound_damage);
    RegisterSpellScript(spell_dk_unholy_frenzy);
    RegisterSpellScript(spell_dk_death_gate_293891);
    RegisterSpellScript(spell_dk_advantage_t10_4p);
    RegisterSpellScript(spell_dk_anti_magic_shell);
    RegisterSpellScript(spell_dk_army_transform);
    RegisterSpellScript(spell_dk_blood_boil);
    RegisterSpellScript(spell_dk_dancing_rune_weapon);
    RegisterSpellAndAuraScriptPair(spell_dk_death_and_decay, spell_dk_death_and_decay_AuraScript);
    RegisterSpellScript(spell_dk_death_coil);
    RegisterSpellScript(spell_dk_death_gate);
    RegisterSpellScript(spell_dk_death_grip_initial);
    RegisterSpellScript(spell_dk_death_pact);
    RegisterSpellScript(spell_dk_death_strike);
    RegisterSpellScript(spell_dk_death_strike_enabler);
    RegisterSpellScript(spell_dk_festering_strike);
    RegisterSpellScript(spell_dk_ghoul_explode);
    RegisterSpellScript(spell_dk_glyph_of_scourge_strike_script);
    RegisterSpellScript(spell_dk_mark_of_blood);
    RegisterSpellScript(spell_dk_necrosis);
    RegisterSpellScript(spell_dk_obliteration);
    RegisterSpellScript(spell_dk_pet_geist_transform);
    RegisterSpellScript(spell_dk_pet_skeleton_transform);
    RegisterSpellScript(spell_dk_pvp_4p_bonus);
    RegisterSpellScript(spell_dk_rime);
    RegisterSpellScript(spell_dk_vampiric_blood);
    RegisterSpellScript(spell_dk_t20_2p_rune_empowered);
    new spell_dk_raise_dead();
    // NPCs
    RegisterCreatureAI(npc_pet_dk_ghoul);
    RegisterCreatureAI(npc_pet_dk_zombie);
}
