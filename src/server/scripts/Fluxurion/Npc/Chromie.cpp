/*
 * Copyright 2023 Fluxurion
 */

#include "Flux.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "WorldPacket.h"

enum Chromie
{
    GOSSIP_MENU_SELECT_CHROMIE_TIME = 25426,
    GOSSIP_MENU_QUESTIONS_ABOUT_TIMEWALKING = 31336,
    GOSSIP_MENU_CHOOSABLE_TIMELINES = 31365,
    GOSSIP_MENU_CAN_MY_FRIENDS_JOIN_ME = 31368,
    GOSSIP_MENU_DONT_WANT_TO_STAY = 31369,
    GOSSIP_MENU_WHAT_ARE_TIMEWALKING_CAMPAIGNS = 31370,
    GOSSIP_MENU_TIMELINE_CATACLYSM = 31376,
    GOSSIP_MENU_TIMELINE_PANDARIA = 31381,
    GOSSIP_MENU_TIMELINE_NORTHREND = 31382,
    GOSSIP_MENU_TIMELINE_OUTLAND = 31383,
    GOSSIP_MENU_TIMELINE_SHADOWLANDS = 31384,
    GOSSIP_MENU_TIMELINE_LEGION = 31385,
    SPELL_SELECT_CHROMIETIME_PRESENT = 335482,
    SPELL_SELECT_CHROMIETIME_NORTHREND = 325042,
    SPELL_SELECT_CHROMIETIME_OUTLAND = 325400,
    SPELL_SELECT_CHROMIETIME_PANDARIA = 325530,
    SPELL_SELECT_CHROMIETIME_DRAENOR = 325534,
    SPELL_SELECT_CHROMIETIME_CATACLYSM = 325537,
    SPELL_SELECT_CHROMIETIME_LEGION = 325539,
    SPELL_SELECT_CHROMIETIME_BFA = 420123,
    SPELL_SELECT_CHROMIETIME_SHADOWLANDS = 397733,
    SPELL_PLAY_MOVIE_CATA = 408936,
    SPELL_PLAY_MOVIE_WOTLK = 408987,
    SPELL_PLAY_MOVIE_BC = 408988,
    SPELL_PLAY_MOVIE_MOP = 408989,
    SPELL_PLAY_MOVIE_WOD = 408991,
    SPELL_PLAY_MOVIE_LEGION = 408993,
    SPELL_PLAY_MOVIE_SL = 408994,
};

struct npc_chromie_167032 : public ScriptedAI
{
    npc_chromie_167032(Creature* creature) : ScriptedAI(creature) { }

    bool OnGossipHello(Player* player) override
    {
        InitGossipMenuFor(player, GOSSIP_MENU_SELECT_CHROMIE_TIME);

        if (player->GetLevel() >= 10 && player->GetLevel() < 60)
        {
            if (Fluxurion::GetChromieTime(player) == CHROMIE_TIME_CURRENT)
                AddGossipItemFor(player, GOSSIP_MENU_SELECT_CHROMIE_TIME, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 0); // Select a timeline.

            if (Fluxurion::GetChromieTime(player) > CHROMIE_TIME_CURRENT)
            {
                AddGossipItemFor(player, GOSSIP_MENU_SELECT_CHROMIE_TIME, 1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1); // Select a different timeline.
                AddGossipItemFor(player, GOSSIP_MENU_SELECT_CHROMIE_TIME, 2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2); // I'd like to return to the present timeline, Chromie.
            }

            AddGossipItemFor(player, GOSSIP_MENU_SELECT_CHROMIE_TIME, 5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10); // I have a question about Timewalking Campaigns.
        }

        player->TalkedToCreature(me->GetEntry(), me->GetGUID());
        SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player* player, uint32 menuId, uint32 gossipListId) override
    {
        uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
        ClearGossipMenuFor(player);

        WorldPackets::NPC::NPCInteractionOpenResult npcInteraction;
        npcInteraction.Npc = me->GetGUID();
        npcInteraction.InteractionType = PlayerInteractionType::ChromieTime;
        npcInteraction.Success = true;

        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF + 10: // I have a question about Timewalking Campaigns. // I have another question.
            InitGossipMenuFor(player, GOSSIP_MENU_QUESTIONS_ABOUT_TIMEWALKING);
            AddGossipItemFor(player, GOSSIP_MENU_QUESTIONS_ABOUT_TIMEWALKING, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11); // What timeline should I choose ?
            AddGossipItemFor(player, GOSSIP_MENU_QUESTIONS_ABOUT_TIMEWALKING, 1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1); // What are Timewalking Campaigns?
            AddGossipItemFor(player, GOSSIP_MENU_QUESTIONS_ABOUT_TIMEWALKING, 2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2); // Can my friends join me?
            AddGossipItemFor(player, GOSSIP_MENU_QUESTIONS_ABOUT_TIMEWALKING, 3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3); // What if I don't want to stay in the timeline I chose?
            AddGossipItemFor(player, GOSSIP_MENU_QUESTIONS_ABOUT_TIMEWALKING, 4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4); // I'm ready to select a timeline.
            SendGossipMenuFor(player, 400000, me->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 11: // What timeline should I choose ?
            InitGossipMenuFor(player, GOSSIP_MENU_CHOOSABLE_TIMELINES);
            AddGossipItemFor(player, GOSSIP_MENU_CHOOSABLE_TIMELINES, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 0); // I want to explore Azeroth.
            AddGossipItemFor(player, GOSSIP_MENU_CHOOSABLE_TIMELINES, 1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1); // I want to explore another planet.
            AddGossipItemFor(player, GOSSIP_MENU_CHOOSABLE_TIMELINES, 2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2); // I want to fight an army of evil undead.
            AddGossipItemFor(player, GOSSIP_MENU_CHOOSABLE_TIMELINES, 3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3); // I want a contemplative adventure.
            AddGossipItemFor(player, GOSSIP_MENU_CHOOSABLE_TIMELINES, 4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4); // I want to command my own garrison.
            AddGossipItemFor(player, GOSSIP_MENU_CHOOSABLE_TIMELINES, 5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5); // I want to be the coolest $c on Azeroth!
            AddGossipItemFor(player, GOSSIP_MENU_CHOOSABLE_TIMELINES, 6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6); // I want to explore the afterlives.
            AddGossipItemFor(player, GOSSIP_MENU_CHOOSABLE_TIMELINES, 7, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10); // I have another question.
            SendGossipMenuFor(player, 400001, me->GetGUID());
            break;
        }

        switch (menuId)
        {
            case GOSSIP_MENU_SELECT_CHROMIE_TIME:
            {
                switch (action)
                {
                case GOSSIP_ACTION_INFO_DEF + 0: // Select a timeline.
                case GOSSIP_ACTION_INFO_DEF + 1: // Select a different timeline.
                    player->SendDirectMessage(npcInteraction.Write());
                    break;
                case GOSSIP_ACTION_INFO_DEF + 2: // I'd like to return to the present timeline, Chromie.
                    player->CastSpell(player, SPELL_SELECT_CHROMIETIME_PRESENT, true);
                    CloseGossipMenuFor(player);
                    break;
                }
            }
            break;

            case GOSSIP_MENU_QUESTIONS_ABOUT_TIMEWALKING:
            {
                switch (action)
                {
                case GOSSIP_ACTION_INFO_DEF + 1: // What are Timewalking Campaigns?
                    InitGossipMenuFor(player, GOSSIP_MENU_WHAT_ARE_TIMEWALKING_CAMPAIGNS);
                    AddGossipItemFor(player, GOSSIP_MENU_WHAT_ARE_TIMEWALKING_CAMPAIGNS, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11); // What timeline should I choose?
                    AddGossipItemFor(player, GOSSIP_MENU_WHAT_ARE_TIMEWALKING_CAMPAIGNS, 1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10); // I have another question.
                    SendGossipMenuFor(player, 400002, me->GetGUID());
                    break;
                case GOSSIP_ACTION_INFO_DEF + 2: // Can my friends join me?
                    InitGossipMenuFor(player, GOSSIP_MENU_CAN_MY_FRIENDS_JOIN_ME);
                    AddGossipItemFor(player, GOSSIP_MENU_CAN_MY_FRIENDS_JOIN_ME, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10); // I have another question.
                    SendGossipMenuFor(player, 400003, me->GetGUID());
                    break;
                case GOSSIP_ACTION_INFO_DEF + 3: // What if I don't want to stay in the timeline I chose?
                    InitGossipMenuFor(player, GOSSIP_MENU_DONT_WANT_TO_STAY);
                    AddGossipItemFor(player, GOSSIP_MENU_DONT_WANT_TO_STAY, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10); // I have another question.
                    SendGossipMenuFor(player, 400004, me->GetGUID());
                    break;
                case GOSSIP_ACTION_INFO_DEF + 4: // I'm ready to select a timeline.
                    player->SendDirectMessage(npcInteraction.Write());
                    CloseGossipMenuFor(player);
                    break;
                }
                break;
            }
            break;

            case GOSSIP_MENU_CHOOSABLE_TIMELINES:
            {
                switch (action)
                {
                case GOSSIP_ACTION_INFO_DEF + 0: // I want to explore Azeroth.
                    InitGossipMenuFor(player, GOSSIP_MENU_TIMELINE_CATACLYSM);
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_CATACLYSM, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 0); // Put me in the Cataclysm timeline.
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_CATACLYSM, 1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1); // Tell me more. <Watch cutscene.>
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_CATACLYSM, 2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10); // I have another question.
                    SendGossipMenuFor(player, 400005, me->GetGUID());
                    break;
                case GOSSIP_ACTION_INFO_DEF + 1: // I want to explore another planet.
                    InitGossipMenuFor(player, GOSSIP_MENU_TIMELINE_OUTLAND);
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_OUTLAND, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 0); // Put me in Outland's timeline.
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_OUTLAND, 1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1); // Tell me more. <Watch cutscene.>
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_OUTLAND, 2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10); // I have another question.
                    SendGossipMenuFor(player, 400006, me->GetGUID());
                    break;
                case GOSSIP_ACTION_INFO_DEF + 2: // I want to fight an army of evil undead.
                    InitGossipMenuFor(player, GOSSIP_MENU_TIMELINE_NORTHREND);
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_NORTHREND, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 0); // Put me in Northrend's timeline.
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_NORTHREND, 1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1); // Tell me more. <Watch cutscene.>
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_NORTHREND, 2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10); // I have another question.
                    SendGossipMenuFor(player, 400007, me->GetGUID());
                    break;
                case GOSSIP_ACTION_INFO_DEF + 3: // I want a contemplative adventure.
                    InitGossipMenuFor(player, GOSSIP_MENU_TIMELINE_PANDARIA);
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_PANDARIA, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 0); // Put me in Pandaria's timeline.
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_PANDARIA, 1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1); // Tell me more. <Watch cutscene.>
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_PANDARIA, 2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10); // I have another question.
                    SendGossipMenuFor(player, 400008, me->GetGUID());
                    break;
                /* I DON'T HAVE THIS GOSSIPMENU OPTION IN MY SNIFF :(
                case GOSSIP_ACTION_INFO_DEF + 4: // I want to command my own garrison.
                    InitGossipMenuFor(player, GOSSIP_MENU_TIMELINE_DRAENOR);
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_DRAENOR, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 0); // WOD
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_DRAENOR, 1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1); // Tell me more. <Watch cutscene.>
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_DRAENOR, 2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10); // I have another question.
                    SendGossipMenuFor(player, idk that broadcasttext id either, me->GetGUID());
                    */
                    break;
                case GOSSIP_ACTION_INFO_DEF + 5: // I want to be the coolest $c on Azeroth!
                    InitGossipMenuFor(player, GOSSIP_MENU_TIMELINE_LEGION);
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_LEGION, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 0); // Put me in the Legion Invasion timeline.
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_LEGION, 1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1); // Tell me more. <Watch cutscene.>
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_LEGION, 2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10); // I have another question.
                    SendGossipMenuFor(player, 400009, me->GetGUID());
                    break;
                case GOSSIP_ACTION_INFO_DEF + 6: // I want to explore the afterlives.
                    InitGossipMenuFor(player, GOSSIP_MENU_TIMELINE_SHADOWLANDS);
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_SHADOWLANDS, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 0); // Put me in the Shadowlands' timeline.
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_SHADOWLANDS, 1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1); // Tell me more. <Watch cutscene.>
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_SHADOWLANDS, 2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10); // I have another question.
                    SendGossipMenuFor(player, 400010, me->GetGUID());
                    break;
                case GOSSIP_ACTION_INFO_DEF + 7: // I want to explore the afterlives.
                    InitGossipMenuFor(player, GOSSIP_MENU_TIMELINE_SHADOWLANDS);
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_SHADOWLANDS, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 0); // Put me in the Shadowlands' timeline.
                    AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_SHADOWLANDS, 1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1); // Tell me more. <Watch cutscene.>
                    //AddGossipItemFor(player, GOSSIP_MENU_TIMELINE_SHADOWLANDS, 2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2); // I have another question.
                    AddGossipItemFor(player, GOSSIP_MENU_SELECT_CHROMIE_TIME, 5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
                    SendGossipMenuFor(player, 400010, me->GetGUID());
                    break;
                }
                break;
            }
            break;

            case GOSSIP_MENU_TIMELINE_CATACLYSM:
            {
                switch (action)
                {
                case GOSSIP_ACTION_INFO_DEF + 0: // Put me in the Cataclysm timeline.
                    player->CastSpell(player, SPELL_SELECT_CHROMIETIME_CATACLYSM, true);
                    CloseGossipMenuFor(player);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 1: // Tell me more. <Watch cutscene.>
                    player->CastSpell(player, SPELL_PLAY_MOVIE_CATA, true);
                    CloseGossipMenuFor(player);
                    break;
                }
                break;
            }
            break;

            case GOSSIP_MENU_TIMELINE_OUTLAND:
            {
                switch (action)
                {
                case GOSSIP_ACTION_INFO_DEF + 0: // Put me in Outland's timeline.
                    player->CastSpell(player, SPELL_SELECT_CHROMIETIME_OUTLAND, true);
                    CloseGossipMenuFor(player);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 1: // Tell me more. <Watch cutscene.>
                    player->CastSpell(player, SPELL_PLAY_MOVIE_BC, true);
                    CloseGossipMenuFor(player);
                    break;
                }
                break;
            }
            break;

            case GOSSIP_MENU_TIMELINE_NORTHREND:
            {
                switch (action)
                {
                case GOSSIP_ACTION_INFO_DEF + 0: // Put me in Northrend's timeline
                    player->CastSpell(player, SPELL_SELECT_CHROMIETIME_NORTHREND, true);
                    CloseGossipMenuFor(player);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 1: // Tell me more. <Watch cutscene.>
                    player->CastSpell(player, SPELL_PLAY_MOVIE_WOTLK, true);
                    CloseGossipMenuFor(player);
                    break;
                }
                break;
            }
            break;

            case GOSSIP_MENU_TIMELINE_PANDARIA:
            {
                switch (action)
                {
                case GOSSIP_ACTION_INFO_DEF + 0: // Put me in Pandaria's timeline.
                    player->CastSpell(player, SPELL_SELECT_CHROMIETIME_PANDARIA, true);
                    CloseGossipMenuFor(player);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 1: // Tell me more. <Watch cutscene.>
                    player->CastSpell(player, SPELL_PLAY_MOVIE_MOP, true);
                    CloseGossipMenuFor(player);
                    break;
                }
                break;
            }
            break;

            case GOSSIP_MENU_TIMELINE_LEGION:
            {
                switch (action)
                {
                case GOSSIP_ACTION_INFO_DEF + 0: // Put me in the Legion Invasion timeline.
                    player->CastSpell(player, SPELL_SELECT_CHROMIETIME_LEGION, true);
                    CloseGossipMenuFor(player);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 1: // Tell me more. <Watch cutscene.>
                    player->CastSpell(player, SPELL_PLAY_MOVIE_LEGION, true);
                    CloseGossipMenuFor(player);
                    break;
                }
                break;
            }
            break;

            case GOSSIP_MENU_TIMELINE_SHADOWLANDS:
            {
                switch (action)
                {
                case GOSSIP_ACTION_INFO_DEF + 0: // Put me in the Shadowlands' timeline.
                    player->CastSpell(player, SPELL_SELECT_CHROMIETIME_SHADOWLANDS, true);
                    CloseGossipMenuFor(player);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 1: // Tell me more. <Watch cutscene.>
                    player->CastSpell(player, SPELL_PLAY_MOVIE_SL, true);
                    CloseGossipMenuFor(player);
                    break;
                }
                break;
            }
            break;

        }

        return true;
    }
};

class set_chromietime_onlogin : public PlayerScript
{
public:
    set_chromietime_onlogin() : PlayerScript("set_chromietime_onlogin") { }

    void OnLogin(Player* player, bool /*loginFirst*/)
    {
        if (Fluxurion::GetChromieTime(player))
		    Fluxurion::SetChromieTime(player, Fluxurion::GetChromieTime(player));
	}
};

void AddSC_Chromie()
{
    RegisterCreatureAI(npc_chromie_167032);
    new set_chromietime_onlogin();
}
