#include "CriteriaHandler.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "Scenario.h"
#include "scenario_the_throughway.h"
#include "Vehicle.h"
#include "TemporarySummon.h"
#include "Player.h"
#include "CombatAI.h"
#include "Creature.h"
#include "CreatureGroups.h"
#include "GameObject.h"
#include "InstanceScript.h"
#include "Scenario.h"

Position const wrathionpath[8] =
{
  { 131.60591f, -915.16144f, 839.13513f ,0 },
  { 131.17361f, -910.86804f, 841.0954f  ,0 },
  { 130.4566f,  -905.40625f, 842.89886f ,0 },
  { 129.34723f, -898.92706f, 844.72906f ,0 },
  { 128.0868f,  -891.8559f,  846.507f   ,0 },
  { 126.43924f, -884.5417f,  848.43097f ,0 },
  { 124.68229f, -876.9132f,  850.5709f  ,0 },
  { 121.68924f, -864.9809f,  853.3789f  ,0 },
};

struct instance_scenario_the_throughway : public InstanceScript
{
    instance_scenario_the_throughway(InstanceMap* map) : InstanceScript(map) { }

    bool isFirstIntro = false;

    void OnPlayerEnter(Player* player) override
    {
         if (!isFirstIntro)
         {
             isFirstIntro = true;
             player->GetSceneMgr().PlayScene(3175);
             return;
         }
    }
};

//3175
class scene_intro_entrance_to_scenario : public SceneScript
{
public:
    scene_intro_entrance_to_scenario() : SceneScript("scene_intro_entrance_to_scenario") { }

    void StartIntroConvosabellian(Player* player)
    {
        player->CastSpell(player, SPELL_CONVERSATION_SABELLIAN_INTRO, true);
    }

    void OnSceneComplete(Player* player, uint32 /*sceneInstanceID*/, SceneTemplate const* /*sceneTemplate*/) override
    {
        StartIntroConvosabellian(player);
    }

    void OnSceneCancel(Player* player, uint32 /*sceneInstanceID*/, SceneTemplate const* /*sceneTemplate*/) override
    {
        StartIntroConvosabellian(player);
    }

};

//21001
struct conversation_Sabellian_201411 : public ConversationScript
{
    conversation_Sabellian_201411() : ConversationScript("conversation_Sabellian_201411") { }

    void OnConversationCreate(Conversation* conversation, Unit* creator) override
    {
        if (Creature* Sabellian = creator->FindNearestCreature(201411, 50.0f, true))
            conversation->AddActor(201411, 1, Sabellian->GetGUID());
    }
};

//21000
struct conversation_Sabellian_and_Wrathion : public ConversationScript
{
    conversation_Sabellian_and_Wrathion() : ConversationScript("conversation_Sabellian_and_Wrathion") { }

    void OnConversationCreate(Conversation* conversation, Unit* creator) override
    {
        if (Creature* wrathion = creator->FindNearestCreature(201412, 50.0f, true))
            conversation->AddActor(201412, 1, wrathion->GetGUID());

        if (Creature* Sabellian = creator->FindNearestCreature(201411, 50.0f, true))
            conversation->AddActor(201411, 0, Sabellian->GetGUID());
    }
};


//201411
struct npc_sabellian : public ScriptedAI
{
public:
    npc_sabellian(Creature* creature) : ScriptedAI(creature) { Reset(); }

private:
    bool SayHeroes;

    void Reset() override
    {
    }

   //void SpellHit(WorldObject* /*caster*/, SpellInfo const* spellInfo) override
   //{
   //    if (spellInfo->Id == 406669)
   //    {
   //        DoCastSelf(407131, true);
   //
   //        if (Creature* wrathion = me->FindNearestCreature(NPC_WARTHION_INTRO, 50.0f))
   //        {
   //            me->CastSpell(wrathion, 407131);
   //        }
   //    }
   //}

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

    bool OnGossipSelect(Player* player, uint32 /*MenuID*/, uint32 gossipListId) override
    {
        if (gossipListId == 1) //Start Scenario Event
        {
            ClearGossipMenuFor(player);
            _scheduler.Schedule(1s, [player, this](TaskContext /*context*/)
            {
                player->CastSpell(player, SPELL_CONVERSATION_DISCUSS_THE_PLAN);
                me->CastSpell(player, SPELL_CAST_EARTH_WARDERS_PROTECTION);
            });
            _scheduler.Schedule(2s, [this](TaskContext /*context*/)
            {
                if (Creature* wrathion = me->FindNearestCreature(201412, 50.0f))
                    wrathion->AI()->DoAction(ACTION_START_WARTHION_MOVE);
            });
            _scheduler.Schedule(9s, [this, player](TaskContext /*context*/)
            {
                player->CastSpell(player, AURA_EARTH_WARDERS_PROTECTION);
            });
            _scheduler.Schedule(10s, [this](TaskContext /*context*/)
            {
                me->GetMotionMaster()->MovePoint(0, -13.923788f, 760.272888f, 93.151299f, true);
            });
            _scheduler.Schedule(21s, [this, player](TaskContext /*context*/)
            {
                if (Scenario* scenario = player->GetScenario())
                {
                    if (scenario->GetStep() == sScenarioStepStore.LookupEntry(5418))
                    {
                        scenario->SetStepState(sScenarioStepStore.LookupEntry(5418), SCENARIO_STEP_IN_PROGRESS);
                        scenario->CompleteStep(sScenarioStepStore.LookupEntry(5418));
                        scenario->SetStep(sScenarioStepStore.LookupEntry(5464));
                    }
                }

                if (Creature* wrathion = me->FindNearestCreature(NPC_WARTHION_INTRO, 50.0f))
                    wrathion->AI()->DoAction(ACTION_IN_COMBAT);

                if (Creature* monstrous = me->FindNearestCreature(NPC_MONSTROUS_MUD_INTRO, 150.0f))
                    monstrous->AI()->DoAction(ACTION_PREDATOR_PRESENCE);

            });
            _scheduler.Schedule(22s, [player](TaskContext /*context*/)
            {
                player->CastSpell(player, 406249); //Battle RP Starts, Boss Conversation
            });
            _scheduler.Schedule(23s, [this](TaskContext /*context*/)
            {
                 me->CastSpell(me, 410723);
            });

        }
        return true;
    }

    private:
        TaskScheduler _scheduler;
};

//201412
struct npc_wrathion_201412 : public  ScriptedAI
{
public:
    npc_wrathion_201412(Creature* creature) : ScriptedAI(creature)
    {

    }

    void Reset() override
    {

    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_START_WARTHION_MOVE)
        {
            me->SetWalk(true);
            me->GetMotionMaster()->MovePoint(0, -7.203760f, 767.083252f, 95.777267f, true);
        }
        if (action == ACTION_IN_COMBAT)
        {
            me->SetEmoteState(EMOTE_STATE_READY1H);
        }
        if (action == ACTION_WARTHION_FLY)
        {
            me->SetWalk(false);
            me->GetMotionMaster()->MoveSmoothPath(0, wrathionpath, 2, false, true);

        }
    }
};

//202593
struct npc_monstrous_mud : public  ScriptedAI
{
public:
    npc_monstrous_mud(Creature* creature) : ScriptedAI(creature)
    {
        me->AddAura(412583, me); //(Predator Presence)
        me->SetUnitFlag(UnitFlags(33088));
    }

    void Reset() override
    {}

    void DoAction(int32 action) override
    {
        if (action == ACTION_PREDATOR_PRESENCE)
        {
           me->CastSpell(me, 412416);

          _scheduler.Schedule(1s, [this](TaskContext /*context*/)
          {
              if (Creature* Sabellian = me->FindNearestCreature(201411, 200.0f)) //Summitshaper Lorac
              {
                  me->CastSpell(Sabellian, 406669);
              }
          });
          _scheduler.Schedule(4s, [this](TaskContext /*context*/)
          {
              if (Creature* wrathion = me->FindNearestCreature(NPC_WARTHION_INTRO, 250.0f))
                  wrathion->AI()->DoAction(ACTION_WARTHION_FLY);
          });
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
};

//202591
struct npc_Summitshaper_Lorac : public  ScriptedAI
{
public:
    npc_Summitshaper_Lorac(Creature* creature) : ScriptedAI(creature)
    {
    }

    void Reset() override
    {}

    void DoAction(int32 action) override
    {
        if (action == ACTION_START_EVENT_LORAC)
        {

        }

    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
};

void AddSC_instance_scenario_the_throughway()
{
    RegisterInstanceScript(instance_scenario_the_throughway, 2597);
    //Conversation
    new conversation_Sabellian_201411();
    new conversation_Sabellian_and_Wrathion();

    //scene
    new scene_intro_entrance_to_scenario();

    // Creatures
    RegisterCreatureAI(npc_sabellian);
    RegisterCreatureAI(npc_wrathion_201412);
    RegisterCreatureAI(npc_monstrous_mud);
    RegisterCreatureAI(npc_Summitshaper_Lorac);
}

