/*
 * Copyright 2024 DekkCore Team Devs
 *
 */

enum quests
{
    QUEST_QUESTIONABLE_OFFERING = 48678,
};

 //Lord Autumnvale 127080
class npc_lord_autumnVale127080 : public CreatureScript
{
public:
    npc_lord_autumnVale127080() : CreatureScript("npc_lord_autumnVale127080") { }

    struct npc_lord_autumnVale127080AI : public ScriptedAI
    {
        npc_lord_autumnVale127080AI(Creature* creature) : ScriptedAI(creature) { }

        bool OnGossipHello(Player* player) override
        {
            if (me->IsQuestGiver())
                player->PrepareQuestMenu(me->GetGUID());

            if (player->HasQuest(QUEST_QUESTIONABLE_OFFERING))
                player->ForceCompleteQuest(QUEST_QUESTIONABLE_OFFERING);

            SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lord_autumnVale127080AI(creature);
    }
};

void AddSC_zone_drustvar()
{
    new npc_lord_autumnVale127080();
}
