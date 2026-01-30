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

#include "ScriptMgr.h"
#include "Player.h"
#include "InstanceScript.h"
#include "shrine_of_the_storm.h"
#include "ChallengeMode.h"
#include "CustomInstanceScript.h"

class instance_shrine_of_the_storm : public CustomInstanceScript
{
public:
    instance_shrine_of_the_storm(InstanceMap* map) : CustomInstanceScript(map)
        {
            SetHeaders(DataHeader);
            SetBossNumber(EncounterCount);
        }

        bool HandleGetStartPosition(Position& entrancePosition) const override
        {
            entrancePosition.Relocate(4168.347656f, -1239.824341f, 186.635757f, 4.699543f);
            return true;
        }

        void OnPlayerEnter(Player* player) override
        {
            if (!TeamInInstance)
                TeamInInstance = player->GetTeam();
        }

        void OnCreatureCreate(Creature* creature) override
        {
            InstanceScript::OnCreatureCreate(creature);

            if (!TeamInInstance)
            {
                Map::PlayerList const &players = instance->GetPlayers();
                if (!players.isEmpty())
                    if (Player* player = players.begin()->GetSource())
                        TeamInInstance = player->GetTeam();
            }

            switch (creature->GetEntry())
            {
            case NPC_REXXAR:
                if (TeamInInstance == ALLIANCE)
                    creature->UpdateEntry(NPC_BROTHER_PIKE);
                break;
            default:
                break;
            }
        }

        void SummonChallengeGameObject(bool door) override
        {
            if (door)
            {
                if (auto go = instance->SummonGameObject(MYTHIC_DOOR_4, { 4168.347656f, -1239.824341f, 186.635757f, 4.699543f }, {}, 0))
                {
                    go->SetGoState(GOState::GO_STATE_READY);
                    go->SetFlag(GameObjectFlags::GO_FLAG_NOT_SELECTABLE);
                }
            }
        }

    protected:
        uint32 TeamInInstance;
    };

void AddSC_instance_shrine_of_the_storm()
{
    RegisterInstanceScript(instance_shrine_of_the_storm, 1864);
}
