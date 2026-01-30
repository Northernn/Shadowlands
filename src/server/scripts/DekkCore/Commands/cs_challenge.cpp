#include "ScriptMgr.h"
#include "ChallengeModeMgr.h"
#include "Chat.h"
#include "ChatCommand.h"
#include "DB2Stores.h"
#include "Language.h"
#include "Player.h"
#include "PlayerChallenge.h"
#include "RBAC.h"
#include "WorldStateMgr.h"

using namespace Trinity::ChatCommands;

class challenge_commandscript : public CommandScript
{
public:
    challenge_commandscript() : CommandScript("challenge_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable challengeKeyCommandTable =
        {
            { "add",            HandleAddChallengeKeyCommand,               rbac::RBAC_PERM_COMMAND_DEBUG, Console::No },
            { "addCurrentMap",  HandleAddCurrentMapChallengeKeyCommand,     rbac::RBAC_PERM_COMMAND_DEBUG, Console::No },
        };
        static ChatCommandTable challengeCommandTable =
        {
            { "key",        challengeKeyCommandTable },
            { "getSteps",   HandleGetScenarioStepsCommand,  rbac::RBAC_PERM_COMMAND_DEBUG, Console::No },
        };
        static ChatCommandTable commandTable =
        {
            { "challenge", challengeCommandTable },
        };
        return commandTable;
    }

    static bool HandleAddChallengeKeyCommand(ChatHandler* handler, uint32 challengeId, uint8 challengeLevel)
    {
        Player* target = handler->getSelectedPlayer();
        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }
        target->GetPlayerChallenge()->CreateMythicKeystone(challengeLevel, challengeId);
        return true;
    }

    static bool HandleAddCurrentMapChallengeKeyCommand(ChatHandler* handler, uint8 challengeLevel)
    {
        Player* target = handler->getSelectedPlayer();
        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 challengeId = sChallengeModeMgr->GetChallengeIdForMap(target->GetMapId());
        if (!challengeId)
            return false;

        target->GetPlayerChallenge()->CreateMythicKeystone(challengeLevel, challengeId);
        return true;
    }

    static bool HandleGetScenarioStepsCommand(ChatHandler* handler)
    {
        Player* target = handler->getSelectedPlayer();
        if (!target)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Scenario* scenario = target->GetScenario();
        if (!scenario)
            return false;

        ScenarioStepEntry const* step = scenario->GetStep();
        CriteriaTree const* criteriaTree = sCriteriaMgr->GetCriteriaTree(step->Criteriatreeid);

        bool isMainCompleted = scenario->IsCompletedCriteriaTree(criteriaTree);
        handler->PSendSysMessage("MainTree %u - IsCompleted : %s", criteriaTree->ID, (isMainCompleted ? "true" : "false"));

        for (CriteriaTree const* childTree : criteriaTree->Children)
        {
            bool isChildCompleted = scenario->IsCompletedCriteriaTree(childTree);
            handler->PSendSysMessage("Child %u - IsCompleted : %s", childTree->ID, (isChildCompleted ? "true": "false"));
        }

        return true;
    }
};

void AddSC_dekkcore_challenge_commandscript()
{
    new challenge_commandscript();
}
