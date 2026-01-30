#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "GameObjectAI.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellAuras.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "GameObject.h"
#include "auchindoun.h"

enum eEmotes
{
    EMOTE_HELLO_NYAMI = 3,
};

Position const g_PositionKaatharCrystalPosition = { 1909.75f, 3188.70f, 66.786f, 5.401960f };

enum eventTuulaniIntro
{
    EVENT_TUULANI_INTRO_1 = 3000,
    EVENT_TUULANI_INTRO_2,
    EVENT_TUULANI_INTRO_3,
    EVENT_TUULANI_INTRO_4,
    EVENT_TUULANI_INTRO_5,
    EVENT_TUULANI_INTRO_6,
    EVENT_TUULANI_INTRO_7,
    EVENT_TUULANI_INTRO_8,
    EVENT_TUULANI_INTRO_9,
    EVENT_TUULANI_INTRO_10,
    EVENT_TUULANI_INTRO_11,
    EVENT_TUULANI_INTRO_12,
    EVENT_TUULANI_INTRO_13,
    EVENT_TUULANI_INTRO_14,
    EVENT_TUULANI_INTRO_15,
    EVENT_TUULANI_INTRO_16,
    EVENT_TUULANI_INTRO_17,
    EVENT_TUULANI_INTRO_18,
    EVENT_TUULANI_INTRO_19,
};

/// Tuulani - 79248
struct auchindoun_mob_tuulani : public ScriptedAI
{
    auchindoun_mob_tuulani(Creature* p_Creature) : ScriptedAI(p_Creature)
    {
        m_Instance = p_Creature->GetInstanceScript();
        m_First = true;
    }

    EventMap events;
    InstanceScript* m_Instance;
    bool m_First;
    uint32 m_FirstDiff;

    void Reset() override
    {
        events.Reset();
        m_First = false;
        me->SetFaction(FriendlyFaction);
        Talk(eAuchindounTalks::TUULANITALK1);
        me->SetSpeed(UnitMoveType::MOVE_RUN, 12.0f);
        me->SetSpeed(UnitMoveType::MOVE_WALK, 1.2f);
        me->SetWalk(false);
        me->SetUnitFlag(UnitFlags(UNIT_FLAG_IMMUNE_TO_NPC));

        AddTimedDelayedOperation(400, [this]() -> void
        {  //??????
        me->GetMotionMaster()->MovePoint(eAuchindounMovementInforms::MovementInformTuulani01, g_PositionTuulaniMovements[0]);
        });
    }

    void UpdateAI(uint32 p_Diff) override
    {
        events.Update(p_Diff);
        //UpdateOperations(p_Diff);//????
        switch (events.ExecuteEvent())
        {
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_1:
            if (Creature* l_Nyami = me->FindNearestCreature(NPC_NYAMI,1000.0f,true))
            {
                l_Nyami->SetReactState(ReactStates::REACT_PASSIVE);
                l_Nyami->AddAura(eAuchindounSpells::SpellDarkFire, l_Nyami);
                l_Nyami->CastSpell(l_Nyami, eAuchindounSpells::SpellPrisonAura);
                l_Nyami->AddAura(eAuchindounSpells::SpellLevitateNyami, l_Nyami);
                l_Nyami->SetUnitFlag(UnitFlags(UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC));
                if (Creature* l_Trigger = l_Nyami->FindNearestCreature(eAuchindounCreatures::CreatureLeftCrystalTrigger, 40.0f, true))
                {
                    l_Trigger->AddAura(eAuchindounSpells::SpellVoidFormTriggerBuff, l_Trigger);
                    l_Nyami->CastSpell(l_Trigger, eAuchindounSpells::SpellShadowBeam);
                }
            }
            Talk(eAuchindounTalks::TUULANITALK1);
            me->GetMotionMaster()->MovePoint(eAuchindounMovementInforms::MovementInformTuulani02, g_PositionTuulaniMovements[1]);
            break;
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_2:
            Talk(eAuchindounTalks::TUULANITALK3);
            me->CastSpell(me, eAuchindounSpells::SpellTuulaniUnlock);
            AddTimedDelayedOperation(2500, [this]() -> void
            {
                if (GameObject* door = me->FindNearestGameObject(GameobjectDoorBarrier, 100.0f))
                    door->Delete();
                me->GetMotionMaster()->MovePoint(eAuchindounMovementInforms::MovementInformTuulani03, g_PositionTuulaniMovements[2]);
            });
            break;
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_3:
            if (Creature* l_Guard = me->FindNearestCreature(CreatureAucheniDefender, 50.0f, true))
            {
                l_Guard->SetFacingToObject(me);
                l_Guard->RemoveAura(eAuchindounSpells::SpellKneel);
                l_Guard->AI()->Talk(eAuchindounTalks::AUCHENAIDEFENDERTALK1);
            }
            events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_4, 7s);
            break;
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_4:
            me->GetMotionMaster()->MovePoint(eAuchindounMovementInforms::MovementInformTuulani04, g_PositionTuulaniMovements[4]);
            break;
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_5:
            Talk(eAuchindounTalks::TUULANITALK4);
            events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_6, 4s);
            break;
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_6:
            me->CastSpell(me, eAuchindounSpells::SpellTuulaniUnlock);
            events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_7, 7s);
            break;
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_7:
            if (GameObject* l_NearestHolyWall = me->FindNearestGameObject(eAuchindounObjects::GameobjectHolyWall, 60.0f))
                l_NearestHolyWall->Delete();
            Talk(eAuchindounTalks::TUULANITALK18);
            me->GetMotionMaster()->MovePoint(eAuchindounMovementInforms::MovementInformTuulani05, g_PositionTuulaniMovements[5]);
            break;
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_8:
            Talk(eAuchindounTalks::TUULANITALK5);
            me->GetMotionMaster()->MovePoint(eAuchindounMovementInforms::MovementInformTuulani06, g_PositionTuulaniMovements[6]);
            break;
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_9:
            me->GetMotionMaster()->MovePoint(eAuchindounMovementInforms::MovementInformTuulani07, g_PositionTuulaniMovements[7]);
            break;
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_10:
            me->GetMotionMaster()->MovePoint(eAuchindounMovementInforms::MovementInformTuulani08, g_PositionTuulaniMovements[8]);
            break;
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_11:
            if (instance)
            {
                instance->DoNearTeleportPlayers(me->GetPosition(), false);
                instance->DoPlayScenePackageIdOnPlayers(SpellAuchindounSceneTulaaniReachNyami);
            }
            me->GetMotionMaster()->MovePoint(eAuchindounMovementInforms::MovementInformTuulani09, g_PositionTuulaniMovements[9]);
            events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_12, 5s);
            events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_14, 12s);
            break;
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_12:
            me->GetMotionMaster()->MovePoint(eAuchindounMovementInforms::MovementInformTuulani10, g_PositionTuulaniMovements[10]);
            break;
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_13:
            Talk(eAuchindounTalks::TUULANITALK7);
            me->AddUnitMovementFlag(MovementFlags::MOVEMENTFLAG_ROOT);
            me->AddAura(eAuchindounSpells::SpellTuulaniCapturedVoidPrison, me);
            me->SetUnitFlag(UnitFlags(UNIT_FLAG_REMOVE_CLIENT_CONTROL));
            events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_15, 7s);
            //l_Tuulina->m_Events.AddEvent(new EventTuulaniIntroduction(l_Tuulina, 15, m_InstanceScript), l_Tuulina->m_Events.CalculateTime(7s));14
            break;
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_14:
            break;
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_15:
            if (Creature* l_Nyami = me->FindNearestCreature(CreatureSoulBinderNyami, 200.0f, true))
            {
                l_Nyami->SetReactState(ReactStates::REACT_PASSIVE);
                l_Nyami->AddAura(eAuchindounSpells::SpellDarkFire, l_Nyami);
                l_Nyami->CastSpell(l_Nyami, eAuchindounSpells::SpellPrisonAura);
                l_Nyami->AddAura(eAuchindounSpells::SpellLevitateNyami, l_Nyami);
                l_Nyami->SetUnitFlag(UnitFlags(UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC));
                if (Creature* l_Trigger = l_Nyami->FindNearestCreature(eAuchindounCreatures::CreatureSoulAegis, 40.0f, true))
                {
                    l_Trigger->AddAura(eAuchindounSpells::SpellVoidFormTriggerBuff, l_Trigger);
                    l_Nyami->CastSpell(l_Trigger, eAuchindounSpells::SpellVoidBeam);
                }
                l_Nyami->AI()->Talk(eAuchindounTalks::NYAMITALK2);
            }
            events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_16, 9s);
            break;
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_16:
            if (Creature* l_Nyami = me->FindNearestCreature(CreatureSoulBinderNyami, 200.0f, true))
                l_Nyami->AI()->Talk(eAuchindounTalks::NYAMITALK3);
            events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_17, 9s);
            break;
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_17:
            if (Creature* l_Nyami = me->FindNearestCreature(CreatureSoulBinderNyami, 200.0f, true))
                l_Nyami->AI()->Talk(eAuchindounTalks::NYAMITALK4);
            events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_18, 9s);
            break;
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_18:
            Talk(eAuchindounTalks::TUULANITALK8);
            events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_19, 9s);
            break;
        case eventTuulaniIntro::EVENT_TUULANI_INTRO_19:
            if (Creature* l_Nyami = me->FindNearestCreature(CreatureSoulBinderNyami, 200.0f, true))
                l_Nyami->AI()->Talk(eAuchindounTalks::NYAMITALK5);
            break;
        default:
            break;
        }
    }

    void MovementInform(uint32 /*p_Type*/, uint32 p_Id)
    {
        if (instance)
        {
            switch (p_Id)
            {
            case eAuchindounMovementInforms::MovementInformTuulani01:
                events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_1, 1s);
                break;
            case eAuchindounMovementInforms::MovementInformTuulani02:
                events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_2, 1s);
                break;
            case eAuchindounMovementInforms::MovementInformTuulani03:
                events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_3, 1s);
                break;
            case eAuchindounMovementInforms::MovementInformTuulani04:
                events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_5, 1s);
                break;
            case eAuchindounMovementInforms::MovementInformTuulani05:
                events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_8, 1s);
                break;
            case eAuchindounMovementInforms::MovementInformTuulani06:
                events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_9, 1s);
                break;
            case eAuchindounMovementInforms::MovementInformTuulani07:
                events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_10, 1s);
                break;
            case eAuchindounMovementInforms::MovementInformTuulani08:
                events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_11, 1s);
                break;
            case eAuchindounMovementInforms::MovementInformTuulani09:
                events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_12, 1s);
                break;
            case eAuchindounMovementInforms::MovementInformTuulani10:
                events.ScheduleEvent(eventTuulaniIntro::EVENT_TUULANI_INTRO_13, 1s);
                break;
            default:
                break;
            }
        }
    }
    private:
        InstanceScript* instance;
};

/// Sargerei Soulbinder - 77812
struct auchindoun_mob_sargerei_soulbinder : public ScriptedAI
{
    auchindoun_mob_sargerei_soulbinder(Creature* p_Creature) : ScriptedAI(p_Creature)
    {
        m_Instance = me->GetInstanceScript();
    }

    enum eSargereiSoulbinderEvents
    {
        EventMindShear = 1,
        EventBendWill
    };

    enum eSargereiSoulbinderSpells
    {
        SpellVoidShell = 160312,
        SpellBendWill = 154527
    };

    EventMap events;
    InstanceScript* m_Instance;

    void Reset()
    {
        events.Reset();
        if (Creature* target = me->FindNearestCreature(CreatureSoulAegis, 50.0f))
            me->CastSpell(target, SpellVoidBeam, true);
    }

    void JustEngagedWith(Unit* p_Attacker) override
    {
        events.ScheduleEvent(eSargereiSoulbinderEvents::EventMindShear, 8s);
        events.ScheduleEvent(eSargereiSoulbinderEvents::EventBendWill, 18s);
    }

    void JustDied(Unit* /*p_Killer*/)
    {
        if (m_Instance)
        {
            if (Creature* l_Kaathar = m_Instance->instance->GetCreature(m_Instance->GetGuidData(eAuchindounDatas::DataBossKathaar)))
            {
                if (l_Kaathar->IsAlive() && l_Kaathar->IsAIEnabled())
                    l_Kaathar->AI()->DoAction(eAuchindounActions::ActionCountPre1StBossKill);
            }
        }
    }

    void UpdateAI(uint32 p_Diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(p_Diff);

        if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
            return;

        switch (events.ExecuteEvent())
        {
        case eSargereiSoulbinderEvents::EventMindShear:
            if (Unit* l_Target = SelectTarget(SelectTargetMethod::Random, 0, 50.0f, true))
                me->CastSpell(l_Target, eSargereiSoulbinderSpells::SpellVoidShell);
            events.ScheduleEvent(eSargereiSoulbinderEvents::EventBendWill, 8s);
            break;
        case eSargereiSoulbinderEvents::EventBendWill:
            if (Unit* l_Target = SelectTarget(SelectTargetMethod::Random, 0, 50.0f, true))
                me->CastSpell(l_Target, eSargereiSoulbinderSpells::SpellBendWill);
            events.ScheduleEvent(eSargereiSoulbinderEvents::EventBendWill, 18s);
            break;
        default:
            break;
        }

        DoMeleeAttackIfReady();
    }
};

/// Sargerei Cleric - 77134
struct auchindoun_mob_sargerei_cleric : public ScriptedAI
{
    auchindoun_mob_sargerei_cleric(Creature* p_Creature) : ScriptedAI(p_Creature)
    {
        m_Instance = me->GetInstanceScript();
    }

    enum eSargereiClericEvents
    {
        EventVoidShell = 1
    };

    enum eSargereiCleircSpells
    {
        SpellVoidShell = 160312
    };

    InstanceScript* m_Instance;
    EventMap events;

    void Reset()
    {
        events.Reset();
        if (Creature* target = me->FindNearestCreature(CreatureSoulAegis, 50.0f))
            me->CastSpell(target, SpellVoidBeam, true);
    }

    void JustEngaggedWith(Unit* p_Attacker)
    {
        events.ScheduleEvent(eSargereiClericEvents::EventVoidShell, 15s);
    }

    void JustDied(Unit* /*p_Killer*/)
    {
        if (m_Instance)
        {
            if (Creature* l_Kaathar = m_Instance->instance->GetCreature(m_Instance->GetGuidData(eAuchindounDatas::DataBossKathaar)))
            {
                if (l_Kaathar->IsAlive() && l_Kaathar->IsAIEnabled())
                    l_Kaathar->AI()->DoAction(eAuchindounActions::ActionCountPre1StBossKill);
            }
        }
    }

    void UpdateAI(uint32 p_Diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(p_Diff);

        if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
            return;

        switch (events.ExecuteEvent())
        {
        case eSargereiClericEvents::EventVoidShell:
            me->CastSpell(me, eSargereiCleircSpells::SpellVoidShell);
            events.ScheduleEvent(eSargereiClericEvents::EventVoidShell, 14s);
            break;
        default:
            break;
        }

        DoMeleeAttackIfReady();
    }
};

/// Cackling Pyrmoaniac - 76260
struct auchindoun_mob_cackling_pyromaniac : public ScriptedAI
{
    auchindoun_mob_cackling_pyromaniac(Creature* p_Creature) : ScriptedAI(p_Creature)
    {
        m_Instance = me->GetInstanceScript();
    }

    enum eCacklingPyromaniacSpells
    {
        SpellFelBlast = 174422,
        SpellAbyssalVisual = 159610
    };

    enum eCacklingPyromaniacEvents
    {
        EventFelBlast = 1
    };

    InstanceScript* m_Instance;
    EventMap events;

    void Reset()
    {
        events.Reset();
        me->CastSpell(me, eCacklingPyromaniacSpells::SpellAbyssalVisual);
    }

    void JustEngagedWith(Unit* p_Attacker) override
    {
        events.ScheduleEvent(eCacklingPyromaniacEvents::EventFelBlast, 6s);
    }

    void JustDied(Unit* /*p_Killer*/)
    {
        if (m_Instance != nullptr)
        {
            if (Creature* l_Azzakel = m_Instance->instance->GetCreature(m_Instance->GetGuidData(eAuchindounDatas::DataBossAzzakael)))
                if (l_Azzakel->IsInCombat() && l_Azzakel->IsAlive() && l_Azzakel->IsAIEnabled())
                    l_Azzakel->GetAI()->DoAction(eAuchindounActions::ActionDemonSoulsAchievement);

            if (Creature* l_Trigger = m_Instance->instance->GetCreature(m_Instance->GetGuidData(eAuchindounDatas::DataTriggerAzzakelController)))
                if (l_Trigger->IsWithinDistInMap(me, 30.0f) && l_Trigger->IsAIEnabled())
                    l_Trigger->AI()->DoAction(eAuchindounActions::ActionCountPre3StBossKill);
        }
    }

    void UpdateAI(uint32 p_Diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(p_Diff);

        if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
            return;

        switch (events.ExecuteEvent())
        {
        case eCacklingPyromaniacEvents::EventFelBlast:
            if (Unit* l_Target = me->GetVictim())
                me->CastSpell(l_Target, eCacklingPyromaniacSpells::SpellFelBlast);
            events.ScheduleEvent(eCacklingPyromaniacEvents::EventFelBlast, 6s);
            break;
        default:
            break;
        }

        DoMeleeAttackIfReady();
    }
};

/// Blazing Trickster - 79511, 79510
struct auchindoun_mob_blazing_trickster : public ScriptedAI
{
    auchindoun_mob_blazing_trickster(Creature* p_Creature) : ScriptedAI(p_Creature)
    {
        m_Instance = me->GetInstanceScript();
    }

    enum eBlazingTricksterSpells
    {
        SpellConfligirate = 154981
    };

    enum eBlazingTricksterEvents
    {
        EventConfligrate = 1
    };

    InstanceScript* m_Instance;
    EventMap events;

    void Reset()
    {
        events.Reset();
        me->SetReactState(ReactStates::REACT_AGGRESSIVE);
        me->SetDefaultMovementType(MovementGeneratorType::RANDOM_MOTION_TYPE);
        events.ScheduleEvent(eBlazingTricksterEvents::EventConfligrate, 12s);
    }

    void JustDied(Unit* /*p_Killer*/)
    {
        if (m_Instance != nullptr)
        {
            if (Creature* l_Azzakel = m_Instance->instance->GetCreature(m_Instance->GetGuidData(eAuchindounDatas::DataBossAzzakael)))
                if (l_Azzakel->IsInCombat() && l_Azzakel->IsAlive() && l_Azzakel->IsAIEnabled())
                    l_Azzakel->GetAI()->DoAction(eAuchindounActions::ActionDemonSoulsAchievement);
        }
        if (me->GetEntry() == 79510)
            if (Creature* Azzakel_control = me->FindNearestCreature(87218, 50.0f, true))
                Azzakel_control->AI()->DoAction(2);
    }

    void UpdateAI(uint32 p_Diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(p_Diff);

        if (me->HasUnitState(UnitState::UNIT_STATE_CASTING))
            return;

        switch (events.ExecuteEvent())
        {
        case eBlazingTricksterEvents::EventConfligrate:
            me->CastSpell(me, eBlazingTricksterSpells::SpellConfligirate);
            events.ScheduleEvent(eBlazingTricksterEvents::EventConfligrate, 12s);
            break;
        default:
            break;
        }

        DoMeleeAttackIfReady();
    }
};

/// Warden Hammer - 76655
struct auchindoun_mob_warden_hammer : public ScriptedAI
{
    auchindoun_mob_warden_hammer(Creature* p_Creature) : ScriptedAI(p_Creature)
    {
        m_Instance = me->GetInstanceScript();
    }

    enum eWardenHammerSpells
    {
        SpellWardenHammerLightningVisual = 154775,
        SpellWardenHammerDamage = 154773
    };

    InstanceScript* m_Instance;
    uint32 m_DiffHammer;

    void Reset()
    {
        me->SetFaction(HostileFaction);
        m_DiffHammer = 1;
        me->SetUnitFlag(UnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_UNINTERACTIBLE));
    }

    void UpdateAI(uint32 p_Diff) override
    {
        if (m_DiffHammer <= p_Diff)
        {
            std::list<Player*> l_ListNerbyPlayers;
            me->GetPlayerListInGrid(l_ListNerbyPlayers, 3.0f);
            if (!l_ListNerbyPlayers.empty())
            {
                for (Player* l_Itr : l_ListNerbyPlayers)
                {
                    if (!l_Itr)
                        continue;

                    if (l_Itr && l_Itr->IsInWorld())
                    {
                        me->CastSpell(l_Itr, eWardenHammerSpells::SpellWardenHammerLightningVisual);
                        me->CastSpell(l_Itr, eWardenHammerSpells::SpellWardenHammerDamage);
                    }
                }
            }

            m_DiffHammer = 1;
        }
        else
            m_DiffHammer -= p_Diff;
    }
};

/// Void Mending - 154623
class auchindoun_spell_void_mending : public SpellScript
{
    void HandleDummy(SpellEffIndex /*p_EffIndex*/)
    {
        if (Unit* l_Caster = GetCaster())
        {
            if (Unit* l_Target = GetHitUnit())
                l_Caster->AddAura(eAuchindounSpells::SpellVoidMendingAura, l_Target);
        }
    }

    void Register()
    {
        OnEffectHitTarget += SpellEffectFn(auchindoun_spell_void_mending::HandleDummy, SpellEffIndex::EFFECT_0, SpellEffectName::SPELL_EFFECT_DUMMY);
    }
};

/// Psychic Terrors - 154356
class auchindoun_spell_psychic_terror : public SpellScript
{
    void HandleDummy(SpellEffIndex /*p_EffIndex*/)
    {
        if (Unit* l_Caster = GetCaster())
        {
            if (Unit* l_Target = GetHitUnit())
            {
                std::list<Player*> l_ListPlayers;
                l_Caster->GetPlayerListInGrid(l_ListPlayers, 4.0f);
                if (!l_ListPlayers.empty())
                {
                    for (Player* l_Itr : l_ListPlayers)
                    {
                        l_Itr->AddAura(eAuchindounSpells::SpellPsychicTerrorFear, l_Itr);
                    }
                }
            }
        }
    }

    void Register()
    {
        OnEffectHitTarget += SpellEffectFn(auchindoun_spell_psychic_terror::HandleDummy, SpellEffIndex::EFFECT_0, SpellEffectName::SPELL_EFFECT_DUMMY);
    }
};

/// Warden's Chain - 154683
class auchindoun_spell_warden_chain : public SpellScript
{
    enum eWardenChainSpells
    {
        SpellWardenChainJump = 154639,
        SpellWardenChainDot = 154831
    };

    void HandleDummy(SpellEffIndex /*p_EffIndex*/)
    {
        if (Unit* l_Caster = GetCaster())
        {
            if (Unit* l_Target = GetExplTargetUnit())
            {
                l_Caster->AddAura(eWardenChainSpells::SpellWardenChainJump, l_Target);
                l_Caster->AddAura(eWardenChainSpells::SpellWardenChainDot, l_Target);
            }
        }
    }

    void Register()
    {
        OnEffectHitTarget += SpellEffectFn(auchindoun_spell_warden_chain::HandleDummy, SpellEffIndex::EFFECT_0, SpellEffectName::SPELL_EFFECT_DUMMY);
    }
};

/// Warden Chain Aura - 154831
class auchindoun_warden_chain_aura : public AuraScript
{
    enum eWardenChainAuras
    {
        SpellWardenChainRoot = 154263
    };

    void HandlePeriodic(AuraEffect const* p_AurEff)
    {
        if (Unit* l_Target = GetTarget())
            l_Target->AddAura(eWardenChainAuras::SpellWardenChainRoot, l_Target);
    }

    void Register()
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(auchindoun_warden_chain_aura::HandlePeriodic, SpellEffIndex::EFFECT_0, AuraType::SPELL_AURA_PERIODIC_DUMMY);
    }
};

/// Void Shift - 155524
class auchindoun_spell_void_shift : public SpellScript
{
    void HandleDummy(SpellEffIndex effIndex)
    {
        if (!GetCaster())
            return;

        Unit* l_Caster = GetCaster();
        int32 l_CalcDamage = 8000;

        std::list<Creature*> l_ListPlayers;
        if (!l_ListPlayers.empty())
        {
            for (std::list<Creature*>::const_iterator l_It = l_ListPlayers.begin(); l_It != l_ListPlayers.end(); ++l_It)
            {
                if (!(*l_It))
                    continue;

                if ((*l_It)->GetTypeId() == TypeID::TYPEID_PLAYER)
                    GetCaster()->CastSpell((*l_It), eAuchindounSpells::SpellVoidShiftDamage, l_CalcDamage);
                else
                    GetCaster()->CastSpell((*l_It), eAuchindounSpells::SpellVoidShiftHeal, l_CalcDamage);
            }
        }
    }

    void Register()
    {
        OnEffectLaunch += SpellEffectFn(auchindoun_spell_void_shift::HandleDummy, SpellEffIndex::EFFECT_0, SpellEffectName::SPELL_EFFECT_DUMMY);
    }
};

/// Void Shell - 160312
class auchindoun_spell_void_shell_filter : public SpellScript
{
    void CorrectTargets(std::list<WorldObject*>& p_Targets)
    {
        p_Targets.clear();

        if (!GetCaster())
            return;

        Unit* l_Caster = GetCaster();

        std::list<Creature*> l_TargetList;
        if (!l_TargetList.empty())
        {
            for (Creature* l_Itr : l_TargetList)
            {
                if (!l_Itr)
                    continue;

                if (l_Itr->GetTypeId() == TypeID::TYPEID_PLAYER)
                    continue;

                if (l_Itr && l_Itr->IsInWorld())
                    p_Targets.push_back(l_Itr);
            }
        }
    }

    void Register()
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(auchindoun_spell_void_shell_filter::CorrectTargets, SpellEffIndex::EFFECT_0, Targets::TARGET_UNIT_SRC_AREA_ALLY);
    }
};

/// Tuulani Unlock Gate - 160415
class auchindoun_spell_tuulani_unlock : public SpellScript
{
    void CorrectTargets(std::list<WorldObject*>& p_Targets)
    {
        /// Clears all targets at start, fetching new ones
        p_Targets.clear();
        std::list<Creature*> l_ListTriggerWall;
        GetCaster()->GetCreatureListWithEntryInGrid(l_ListTriggerWall, eAuchindounCreatures::CreatureLightWallTargets, 15.0f);
        if (!l_ListTriggerWall.empty())
        {
            for (Creature* l_Itr : l_ListTriggerWall)
            {
                if (l_Itr && l_Itr->IsInWorld())
                    p_Targets.push_back(l_Itr->ToUnit());
            }
        }
    }

    void HandleAfterCast()
    {
        if (Unit* l_Caster = GetCaster())
        {
            if (GameObject * l_GameObject = l_Caster->FindNearestGameObject(eAuchindounObjects::GameobjectHolyBarrierEntra, 15.0f))
            {
                l_GameObject->Delete();
            }
        }
    }

    void Register()
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(auchindoun_spell_tuulani_unlock::CorrectTargets, SpellEffIndex::EFFECT_0, Targets::TARGET_UNIT_DEST_AREA_ENTRY);
    }
};

/// Arcane Bolt - 157505
class auchindoun_spell_arcane_bolt : public AuraScript
{
    enum eArcaneBoltSpells
    {
        SpellArcaneBoltPeriod = 157505,
        SpellArcaneBoltProje = 157931
    };

    void HandlePeriodic(AuraEffect const* p_AurEff)
    {
        if (Unit* l_Caster = GetCaster())
        {
            if (l_Caster->IsAIEnabled())
            {
                if (Unit* l_Target = l_Caster->GetAI()->SelectTarget(SelectTargetMethod::Random, 0, 50.0f, true))
                    l_Caster->CastSpell(l_Target, eArcaneBoltSpells::SpellArcaneBoltProje, true);
            }
        }
    }

    void Register()
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(auchindoun_spell_arcane_bolt::HandlePeriodic, SpellEffIndex::EFFECT_0, AuraType::SPELL_AURA_PERIODIC_DUMMY);
    }
};

/// Talador Portal - 236689
struct auchindoun_gob_talador_portal : public GameObjectAI
{
    auchindoun_gob_talador_portal(GameObject* go) : GameObjectAI(go) { }

    bool OnGossipHello(Player* player) override
    {
        player->TeleportTo(1116, 1488.52f, 3077.65f, 108.920f, 4.653427f);
        return true;
    }
};

void AddSC_auchindoun()
{
    RegisterCreatureAI(auchindoun_mob_tuulani);                   ///< 79248
    RegisterCreatureAI(auchindoun_mob_sargerei_soulbinder);       ///< 77812
    RegisterCreatureAI(auchindoun_mob_sargerei_cleric);           ///< 77134
    RegisterCreatureAI(auchindoun_mob_cackling_pyromaniac);       ///< 76260
    RegisterCreatureAI(auchindoun_mob_blazing_trickster);         ///< 79511
    RegisterCreatureAI(auchindoun_mob_warden_hammer);             ///< 76655
    RegisterSpellScript(auchindoun_spell_void_shift);             ///< 155524
    RegisterSpellScript(auchindoun_spell_void_mending);           ///< 154623
    RegisterSpellScript(auchindoun_spell_void_shell_filter);      ///< 160312
    RegisterSpellScript(auchindoun_spell_psychic_terror);         ///< 154356
    RegisterSpellScript(auchindoun_spell_tuulani_unlock);         ///< 160415
    RegisterSpellScript(auchindoun_spell_arcane_bolt);            ///< 157505
    RegisterGameObjectAI(auchindoun_gob_talador_portal);          ///< 236689
    RegisterSpellScript(auchindoun_warden_chain_aura);
}
