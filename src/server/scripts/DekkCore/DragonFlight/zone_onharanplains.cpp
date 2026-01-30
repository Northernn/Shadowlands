/*
 * Copyright 2023 DekkCore
 *
 */

#include "GameObject.h"
#include "GameObjectAI.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "ScriptedCreature.h"

enum quest
{
  QUEST_SHAPING_A_SHPERD = 68085,
};

class npc_Shepherd_Tevatei_192633 : public ScriptedAI
{
public:
    npc_Shepherd_Tevatei_192633(Creature* creature) : ScriptedAI(creature) { }

    bool OnGossipHello(Player* player) override
    {
        if (me->IsQuestGiver())
            player->PrepareQuestMenu(me->GetGUID());

        if (player->HasQuest(QUEST_SHAPING_A_SHPERD))
        {
            player->ForceCompleteQuest(QUEST_SHAPING_A_SHPERD);
        }

        SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
        return true;
    }
};

void AddSC_zone_onharanplains()
{
    RegisterCreatureAI(npc_Shepherd_Tevatei_192633);
}
