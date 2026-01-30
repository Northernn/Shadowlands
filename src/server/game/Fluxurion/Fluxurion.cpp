/*
 * Copyright 2023 Fluxurion
 */

#include "Flux.h"

// Simple check if the player's quest log has contain the quest by ID
bool Fluxurion::HasQuest(Player* player, uint32 questID)
{
    if (!player)
        return false;

    if (questID == 0)
        return false;

    for (uint8 itr = 0; itr < MAX_QUEST_LOG_SIZE; ++itr)
        if (player->GetQuestSlotQuestId(itr) == questID)
            return true;

    return false;
}

void Fluxurion::ForceCompleteQuest(Player* player, uint32 questID)
{
    bool debug = false; // set this true if you want to debug in console

    if (debug)
        TC_LOG_DEBUG("server.FluxurionDebug", "Fluxurion::ForceCompleteQuest called for player {} in quest: {}.", player->GetName().c_str(), questID);

    Quest const* quest = sObjectMgr->GetQuestTemplate(questID);
    if (!quest)
        return;

    for (uint32 i = 0; i < quest->Objectives.size(); ++i)
    {
        QuestObjective const& obj = quest->Objectives[i];

        switch (obj.Type)
        {
        case QUEST_OBJECTIVE_ITEM:
        {
            uint32 curItemCount = player->GetItemCount(obj.ObjectID, true);
            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, obj.ObjectID, obj.Amount - curItemCount);
            if (msg == EQUIP_ERR_OK)
            {
                Item* item = player->StoreNewItem(dest, obj.ObjectID, true);
                player->SendNewItem(item, obj.Amount - curItemCount, true, false);
            }

            if (debug)
                TC_LOG_INFO("server.FluxurionDebug", "Fluxurion::ForceCompleteQuest QUEST_OBJECTIVE_ITEM id: {}, amount: {}.", obj.ObjectID, obj.Amount);
            break;
        }
        case QUEST_OBJECTIVE_MONSTER:
        {
            if (CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(obj.ObjectID))
                for (uint16 z = 0; z < obj.Amount; ++z)
                    player->KilledMonster(creatureInfo, ObjectGuid::Empty);

            if (debug)
                TC_LOG_INFO("server.FluxurionDebug", "Fluxurion::ForceCompleteQuest QUEST_OBJECTIVE_MONSTER id: {}, amount: {}.", obj.ObjectID, obj.Amount);
            break;
        }
        case QUEST_OBJECTIVE_GAMEOBJECT:
        {
            for (uint16 z = 0; z < obj.Amount; ++z)
                player->KillCreditGO(obj.ObjectID);

            if (debug)
                TC_LOG_INFO("server.FluxurionDebug", "Fluxurion::ForceCompleteQuest QUEST_OBJECTIVE_GAMEOBJECT id: {}, amount: {}.", obj.ObjectID, obj.Amount);
            break;
        }
        case QUEST_OBJECTIVE_MIN_REPUTATION:
        {
            uint32 curRep = player->GetReputationMgr().GetReputation(obj.ObjectID);
            if (curRep < uint32(obj.Amount))
                if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(obj.ObjectID))
                    player->GetReputationMgr().SetReputation(factionEntry, obj.Amount);

            if (debug)
                TC_LOG_INFO("server.FluxurionDebug", "Fluxurion::ForceCompleteQuest QUEST_OBJECTIVE_MIN_REPUTATION id: {}, amount: {}.", obj.ObjectID, obj.Amount);
            break;
        }
        case QUEST_OBJECTIVE_MAX_REPUTATION:
        {
            uint32 curRep = player->GetReputationMgr().GetReputation(obj.ObjectID);
            if (curRep > uint32(obj.Amount))
                if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(obj.ObjectID))
                    player->GetReputationMgr().SetReputation(factionEntry, obj.Amount);

            if (debug)
                TC_LOG_INFO("server.FluxurionDebug", "Fluxurion::ForceCompleteQuest QUEST_OBJECTIVE_MAX_REPUTATION id: {}, amount: {}.", obj.ObjectID, obj.Amount);
            break;
        }
        case QUEST_OBJECTIVE_MONEY:
        {
            player->ModifyMoney(obj.Amount);

            if (debug)
                TC_LOG_INFO("server.FluxurionDebug", "Fluxurion::ForceCompleteQuest QUEST_OBJECTIVE_MONEY amount: {}.", obj.Amount);
            break;
        }
        }
    }

    if (player->GetQuestStatus(questID) != QUEST_STATUS_COMPLETE)
        player->CompleteQuest(questID);

    if (debug)
        TC_LOG_INFO("server.FluxurionDebug", "Fluxurion::ForceCompleteQuest ended.");
}


// Add item to the player with the little notification box like u loot something cool
void Fluxurion::AddItemWithToast(Player* player, uint32 itemID, uint16 quantity, std::vector<int32> const& bonusIDs /*= std::vector<int32>()*/)
{
    if (!player)
        return;

    Item* pItem = Item::CreateItem(itemID, quantity, ItemContext::NONE, player);

    for (uint32 bonusId : bonusIDs)
        pItem->AddBonuses(bonusId);

    player->SendDisplayToast(itemID, DisplayToastType::NewItem, false, quantity, DisplayToastMethod::PersonalLoot, 0U, pItem);
    player->StoreNewItemInBestSlots(itemID, quantity, ItemContext::NONE);
}

// Send multiple items to the player via ingame mail also can add multiple bonusID to the item
void Fluxurion::SendABunchOfItemsInMail(Player* player, std::vector<uint32> BunchOfItems, std::string const& subject, std::vector<int32> const& bonusIDs /*= std::vector<int32>()*/)
{
    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
    std::string _subject(subject);
    MailDraft draft(_subject,
        "This is a system message. Do not answer! Don't forget to take out the items! :)");

    for (const uint32 item : BunchOfItems)
    {
        TC_LOG_INFO("server.worldserver", "[BunchOfItems]: {}.", item);
        if (Item* pItem = Item::CreateItem(item, 1, ItemContext::NONE, player))
        {
            for (int32 bonus : bonusIDs)
                pItem->AddBonuses(bonus);

            pItem->SaveToDB(trans);
            draft.AddItem(pItem);
        }
    }

    draft.SendMailTo(trans, player, MailSender(player, MAIL_STATIONERY_GM), MailCheckMask(MAIL_CHECK_MASK_COPIED | MAIL_CHECK_MASK_RETURNED));
    CharacterDatabase.CommitTransaction(trans);
}

// Fluxurion's function to get loadout items from db2
std::vector<uint32> DB2Manager::GetLoadOut(int8 classID, int8 purpose)
{
    std::vector<uint32> finalLoadout = std::vector<uint32>();
    CharacterLoadoutItemContainer loadoutContainer;

    auto allLoadoutForClass = _characterLoadout.find(classID);
    if (allLoadoutForClass != _characterLoadout.end())
    {
        int8 i = 0;
        for (auto loadoutForClass : allLoadoutForClass->second)
        {
            if (loadoutForClass.Purpose == purpose)
            {
                loadoutContainer.insert(std::make_pair(i, _characterLoadoutItem[loadoutForClass.ID]));
                i++;
            }
        }
    }

    int8 bagcounter = 1;
    // All spec's all item at once to get other spec's weapons too (not blizzlike way, they use a lootable container to get other spec's weapons)
    for (auto allSpecsLoadout : loadoutContainer)
        for (uint32 loadoutItem : allSpecsLoadout.second)
        {
            if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(loadoutItem))
                if (itemTemplate->GetClass() == ITEM_CLASS_CONTAINER)
                    if (bagcounter != 5) // only for purpose 14 where u can get a reagent bag too
                    {
                        finalLoadout.push_back(loadoutItem); // insert bag maximum 4 times
                        bagcounter++;
                        continue;
                    }

            if (std::find(finalLoadout.begin(), finalLoadout.end(), loadoutItem) == finalLoadout.end())
                finalLoadout.push_back(loadoutItem); // insert only if it's not already there
        }

    return finalLoadout;
}

// Duplicate of StoreNewItemInBestSlots but with bonus
bool Fluxurion::StoreNewItemWithBonus(Player* player, uint32 titem_id, uint32 titem_amount, std::vector<int32> const& bonusIDs /*= std::vector<int32>()*/)
{
    TC_LOG_DEBUG("entities.player.items", "Player::StoreNewItemInBestSlots: Player '{}' ({}) creates initial item (ItemID: {}, Count: {})",
        player->GetName().c_str(), player->GetGUID().ToString().c_str(), titem_id, titem_amount);

    // attempt equip by one
    while (titem_amount > 0)
    {
        uint16 eDest;
        InventoryResult msg = player->CanEquipNewItem(NULL_SLOT, eDest, titem_id, true);
        if (msg != EQUIP_ERR_OK)
            break;

        player->EquipNewItem(eDest, titem_id, ItemContext::NONE, true);

        if (Item* item = player->GetItemByPos(eDest))
            for (int32 bonus : bonusIDs)
                item->AddBonuses(bonus);

        player->AutoUnequipOffhandIfNeed();
        --titem_amount;
    }

    if (titem_amount == 0)
        return true;                                        // equipped

    // attempt store
    ItemPosCountVec sDest;
    // store in main bag to simplify second pass (special bags can be not equipped yet at this moment)
    InventoryResult msg = player->CanStoreNewItem(INVENTORY_SLOT_BAG_0, NULL_SLOT, sDest, titem_id, titem_amount);
    if (msg == EQUIP_ERR_OK)
    {
        player->StoreNewItem(sDest, titem_id, true, GenerateItemRandomBonusListId(titem_id), GuidSet(), ItemContext::NONE, &bonusIDs, true);
        return true;                                        // stored
    }

    // item can't be added
    TC_LOG_ERROR("entities.player.items", "Player::StoreNewItemInBestSlots: Player '{}' ({}) can't equip or store initial item (ItemID: {}, Race: {}, Class: {}, InventoryResult: {})",
        player->GetName().c_str(), player->GetGUID().ToString().c_str(), titem_id, player->GetRace(), player->GetClass(), msg);
    return false;
}

// Custom Gear Giver function which uses characterloadout & characterloadoutitem db2 to give gear to the player.
void Fluxurion::GearUpByLoadout(Player* player, uint32 loadout_purpose, std::vector<int32> const& bonusIDs /*= std::vector<int32>()*/)
{
    uint32 ITEM_HEARTHSTONE = 6948;

    // Get equipped item and store it in bag. If bag is full store it in toBeMailedCurrentEquipment to send it in mail later.
    std::vector<Item*> toBeMailedCurrentEquipment;
    for (int es = EquipmentSlots::EQUIPMENT_SLOT_START; es < EquipmentSlots::EQUIPMENT_SLOT_END; es++)
    {
        if (Item* currentEquiped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, es))
        {
            ItemPosCountVec off_dest;
            if (player->CanStoreItem(NULL_BAG, NULL_SLOT, off_dest, currentEquiped, false) == EQUIP_ERR_OK)
            {
                player->RemoveItem(INVENTORY_SLOT_BAG_0, es, true);
                player->StoreItem(off_dest, currentEquiped, true);
            }
            else
                toBeMailedCurrentEquipment.push_back(currentEquiped);
        }
    }

    // If there are item in the toBeMailedCurrentEquipment vector remove it from inventory and send it in mail.
    if (!toBeMailedCurrentEquipment.empty())
    {
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        MailDraft draft("Inventory Full: Old Equipment.",
            "To equip your new gear, your old gear had to be unequiped. You did not have enough free bag space, the items that could not be added to your bag you can find in this mail.");

        for (Item* currentEquiped : toBeMailedCurrentEquipment)
        {
            player->MoveItemFromInventory(INVENTORY_SLOT_BAG_0, currentEquiped->GetBagSlot(), true);
            currentEquiped->DeleteFromInventoryDB(trans);                   // deletes item from character's inventory
            currentEquiped->SaveToDB(trans);                                // recursive and not have transaction guard into self, item not in inventory and can be save standalone
            draft.AddItem(currentEquiped);
        }

        draft.SendMailTo(trans, player, MailSender(player, MAIL_STATIONERY_GM), MailCheckMask(MAIL_CHECK_MASK_COPIED | MAIL_CHECK_MASK_RETURNED));
        CharacterDatabase.CommitTransaction(trans);
    }

    std::vector<uint32> toBeMailedNewItems;

    // Add the new items from loadout.
    for (const uint32 item : sDB2Manager.GetLoadOut(player->GetClass(), loadout_purpose))
        if (item != ITEM_HEARTHSTONE || !player->HasItemCount(ITEM_HEARTHSTONE, 1, true))
            if (!Fluxurion::StoreNewItemWithBonus(player, item, 1, bonusIDs))
                toBeMailedNewItems.push_back(item);

    // If we added more item than free bag slot send the new item as well in mail.
    if (!toBeMailedNewItems.empty())
    {
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        MailDraft draft("Inventory Full: New Gear.",
            "You did not have enough free bag space to add all your complementary new gear to your bags, those that did not fit you can find in this mail.");

        for (const uint32 item : toBeMailedNewItems)
        {
            if (item != ITEM_HEARTHSTONE || !player->HasItemCount(ITEM_HEARTHSTONE, 1, true))
                if (Item* pItem = Item::CreateItem(item, 1, ItemContext::NONE, player))
                {
                    for (int32 bonus : bonusIDs)
                        pItem->AddBonuses(bonus);

                    if (pItem->GetRequiredLevel() != player->GetLevel())
                        pItem->SetFixedLevel(player->GetLevel());

                    pItem->SaveToDB(trans);
                    draft.AddItem(pItem);
                }
        }

        draft.SendMailTo(trans, player, MailSender(player, MAIL_STATIONERY_GM), MailCheckMask(MAIL_CHECK_MASK_COPIED | MAIL_CHECK_MASK_RETURNED));
        CharacterDatabase.CommitTransaction(trans);
    }

    player->SaveToDB();
}

void Fluxurion::SetChromieTime(Player* player, uint16 chromieTime)
{
    ObjectGuid guid = player->GetGUID();
    UiChromieTimeExpansionInfoEntry const* expansion = sUiChromieTimeExpansionInfoStore.LookupEntry(chromieTime);

    if (!expansion)
    {
        Fluxurion::SetUpdateFieldValue(player, player->m_values.ModifyValue(&Player::m_activePlayerData).ModifyValue(&UF::ActivePlayerData::UiChromieTimeExpansionID), CHROMIE_TIME_CURRENT);
        Fluxurion::SetUpdateFieldValue(player, player->m_values.ModifyValue(&Player::m_playerData).ModifyValue(&UF::PlayerData::CtrOptions).ModifyValue(&UF::CTROptions::Field_4), player->GetTeam() == ALLIANCE ? 3 : 5);
        // Fluxurion::SetUpdateFieldValue(player, player->m_values.ModifyValue(&Player::m_playerData).ModifyValue(&UF::PlayerData::CtrOptions).ModifyValue(&UF::CTROptions::ContentTuningConditionMask), ?); // That's a rough one
        Fluxurion::SetUpdateFieldValue(player, player->m_values.ModifyValue(&Player::m_playerData).ModifyValue(&UF::PlayerData::CtrOptions).ModifyValue(&UF::CTROptions::ExpansionLevelMask), 0);
    }
    else
    {
        Fluxurion::SetUpdateFieldValue(player, player->m_values.ModifyValue(&Player::m_activePlayerData).ModifyValue(&UF::ActivePlayerData::UiChromieTimeExpansionID), expansion->ID);
        Fluxurion::SetUpdateFieldValue(player, player->m_values.ModifyValue(&Player::m_playerData).ModifyValue(&UF::PlayerData::CtrOptions).ModifyValue(&UF::CTROptions::Field_4), player->GetTeam() == ALLIANCE ? 3 : 5);
        // Fluxurion::SetUpdateFieldValue(player, player->m_values.ModifyValue(&Player::m_playerData).ModifyValue(&UF::PlayerData::CtrOptions).ModifyValue(&UF::CTROptions::ContentTuningConditionMask), ?); // That's a rough one
        Fluxurion::SetUpdateFieldValue(player, player->m_values.ModifyValue(&Player::m_playerData).ModifyValue(&UF::PlayerData::CtrOptions).ModifyValue(&UF::CTROptions::ExpansionLevelMask), expansion->ExpansionLevelMask);
    }

    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHROMIE_TIME);
    stmt->setUInt16(0, chromieTime);
    stmt->setUInt64(1, guid.GetCounter());
    CharacterDatabase.Execute(stmt);
}

uint16 Fluxurion::GetChromieTime(Player* player)
{
    ObjectGuid guid = player->GetGUID();
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHROMIE_TIME);
    stmt->setUInt64(0, guid.GetCounter());
    PreparedQueryResult chromieTime = CharacterDatabase.Query(stmt);

    if (!chromieTime)
        return 0;

    Field* fields = chromieTime->Fetch();
    uint16 _chromieTime = fields[0].GetUInt16();

    return _chromieTime;
}

std::string Fluxurion::GetChromieTimeName(Player* player)
{
    uint16 chromieTime = GetChromieTime(player);
    std::string chromieTimeName = "Unknown Chromie Time ?!";
    switch (chromieTime)
    {
    case ChromieTime::CHROMIE_TIME_BFA:
    case ChromieTime::CHROMIE_TIME_CURRENT:
        chromieTimeName = "Battle for Azeroth";
        break;
    case ChromieTime::CHROMIE_TIME_CATACLYSM:
        chromieTimeName = "Cataclysm";
        break;
    case ChromieTime::CHROMIE_TIME_DRAENOR:
        chromieTimeName = "Draenor";
        break;
    case ChromieTime::CHROMIE_TIME_LEGION:
        chromieTimeName = "Legion";
        break;
    case ChromieTime::CHROMIE_TIME_NORTHREND:
        chromieTimeName = "Northrend";
        break;
    case ChromieTime::CHROMIE_TIME_OUTLAND:
        chromieTimeName = "Outland";
        break;
    case ChromieTime::CHROMIE_TIME_PANDARIA:
        chromieTimeName = "Pandaria";
        break;
    case ChromieTime::CHROMIE_TIME_SHADOWLANDS:
        chromieTimeName = "Shadowlands";
        break;
    }
    return chromieTimeName;
}

uint8 Fluxurion::GetChromieTimeExpansionLevel(uint8 chromieTime)
{
    switch (chromieTime)
    {
    case ChromieTime::CHROMIE_TIME_OUTLAND:
        return EXPANSION_THE_BURNING_CRUSADE;
        break;
    case ChromieTime::CHROMIE_TIME_NORTHREND:
        return EXPANSION_WRATH_OF_THE_LICH_KING;
        break;
    case ChromieTime::CHROMIE_TIME_CATACLYSM:
        return EXPANSION_CATACLYSM;
        break;
    case ChromieTime::CHROMIE_TIME_PANDARIA:
        return EXPANSION_MISTS_OF_PANDARIA;
        break;
    case ChromieTime::CHROMIE_TIME_DRAENOR:
        return EXPANSION_WARLORDS_OF_DRAENOR;
        break;
    case ChromieTime::CHROMIE_TIME_LEGION:
        return EXPANSION_LEGION;
        break;
    case ChromieTime::CHROMIE_TIME_SHADOWLANDS:
        return EXPANSION_SHADOWLANDS;
        break;
    case ChromieTime::CHROMIE_TIME_CURRENT: // BFA
    case ChromieTime::CHROMIE_TIME_BFA: // BFA
    default:
        return EXPANSION_BATTLE_FOR_AZEROTH;
        break;
    }
}

bool Fluxurion::CanTakeQuestFromSpell(Player* player, uint32 questGiverSpellId)
{
    // Extra check for legion questline starter spell which has 6 quest
    if (questGiverSpellId == 281351 && (HasQuest(player, 43926) || player->GetQuestStatus(43926) == QUEST_STATUS_REWARDED || HasQuest(player, 40519) || player->GetQuestStatus(40519) == QUEST_STATUS_REWARDED))
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(questGiverSpellId, DIFFICULTY_NONE);
    if (!spellInfo)
    {
        TC_LOG_DEBUG("server.CanTakeQuestFromQuestSpell", "Can't get spellinfo for spell: {}", questGiverSpellId);
        return false;
    }

    std::vector<uint32> questIds;
    std::vector<uint32> acceptableQuestIds;

    for (SpellEffectInfo const& effect : spellInfo->GetEffects())
    {
        if (effect.Effect == SPELL_EFFECT_QUEST_START)
            questIds.push_back(effect.MiscValue);
    }

    for (uint32 questId : questIds)
    {
        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
        {
            TC_LOG_DEBUG("server.CanTakeQuestFromQuestSpell", "Can't get quest template for quest: {}", questId);
            continue;
        }
        else if (!HasQuest(player, questId) && player->GetQuestStatus(questId) != QUEST_STATUS_REWARDED)
        {
            acceptableQuestIds.push_back(questId);

            TC_LOG_DEBUG("server.CanTakeQuestFromQuestSpell", "Player can take quest: {}", quest->GetQuestId());
        }
    }

    if (acceptableQuestIds.size() > 0)
        return true;

    return false;
}

void Fluxurion::SendPlayerChoice(Player* player, ObjectGuid sender, int32 choiceId)
{
    PlayerChoice const* playerChoice = sObjectMgr->GetPlayerChoice(choiceId);
    if (!playerChoice)
        return;

    LocaleConstant locale = player->GetSession()->GetSessionDbLocaleIndex();
    PlayerChoiceLocale const* playerChoiceLocale = locale != DEFAULT_LOCALE ? sObjectMgr->GetPlayerChoiceLocale(choiceId) : nullptr;

    player->PlayerTalkClass->GetInteractionData().Reset();
    player->PlayerTalkClass->GetInteractionData().SourceGuid = sender;
    player->PlayerTalkClass->GetInteractionData().PlayerChoiceId = uint32(choiceId);

    WorldPackets::Quest::DisplayPlayerChoice displayPlayerChoice;

    displayPlayerChoice.Responses.resize(playerChoice->Responses.size());

    displayPlayerChoice.SenderGUID = sender;
    displayPlayerChoice.ChoiceID = choiceId;
    displayPlayerChoice.UiTextureKitID = playerChoice->UiTextureKitId;
    displayPlayerChoice.SoundKitID = playerChoice->SoundKitId;
    displayPlayerChoice.Question = playerChoice->Question;
    if (playerChoiceLocale)
        ObjectMgr::GetLocaleString(playerChoiceLocale->Question, locale, displayPlayerChoice.Question);
    displayPlayerChoice.CloseChoiceFrame = false;
    displayPlayerChoice.HideWarboardHeader = playerChoice->HideWarboardHeader;
    displayPlayerChoice.KeepOpenAfterChoice = playerChoice->KeepOpenAfterChoice;

    std::vector<PlayerChoiceResponse> playerChoiceResponses;

    for (std::size_t i = 0; i < playerChoice->Responses.size(); ++i)
    {
        if (!playerChoice || i >= playerChoice->Responses.size())
        {
            break;
        }

        PlayerChoiceResponse const& playerChoiceResponseTemplate = playerChoice->Responses[i];

        if ((choiceId == 342 || choiceId == 352) && playerChoiceResponseTemplate.Reward && !CanTakeQuestFromSpell(player, playerChoiceResponseTemplate.Reward->SpellID))
            continue;

        playerChoiceResponses.push_back(playerChoiceResponseTemplate);
    }

    // don't send empty choice (warboard)
    if ((choiceId == 342 || choiceId == 352) && playerChoiceResponses.empty())
    {
        ChatHandler(player->GetSession()).PSendSysMessage("Please come back later.");
        return;
    }

    int32 count = 0;
    for (std::size_t i = 0; i < playerChoiceResponses.size(); ++i)
    {
        PlayerChoiceResponse const& playerChoiceResponseTemplate = playerChoiceResponses[i];

        count++;
        if (count > 4 && choiceId != 644)
            continue;
        WorldPackets::Quest::PlayerChoiceResponse& playerChoiceResponse = displayPlayerChoice.Responses[static_cast<std::vector<WorldPackets::Quest::PlayerChoiceResponse, std::allocator<WorldPackets::Quest::PlayerChoiceResponse>>::size_type>(count) - 1];
        playerChoiceResponse.ResponseID = playerChoiceResponseTemplate.ResponseId;
        playerChoiceResponse.ResponseIdentifier = playerChoiceResponseTemplate.ResponseIdentifier;
        playerChoiceResponse.ChoiceArtFileID = playerChoiceResponseTemplate.ChoiceArtFileId;
        playerChoiceResponse.Flags = playerChoiceResponseTemplate.Flags;
        playerChoiceResponse.WidgetSetID = playerChoiceResponseTemplate.WidgetSetID;
        playerChoiceResponse.UiTextureAtlasElementID = playerChoiceResponseTemplate.UiTextureAtlasElementID;
        playerChoiceResponse.SoundKitID = playerChoiceResponseTemplate.SoundKitID;
        playerChoiceResponse.GroupID = playerChoiceResponseTemplate.GroupID;
        playerChoiceResponse.UiTextureKitID = playerChoiceResponseTemplate.UiTextureKitID;
        playerChoiceResponse.Answer = playerChoiceResponseTemplate.Answer;
        playerChoiceResponse.Header = playerChoiceResponseTemplate.Header;
        playerChoiceResponse.SubHeader = playerChoiceResponseTemplate.SubHeader;
        playerChoiceResponse.ButtonTooltip = playerChoiceResponseTemplate.ButtonTooltip;
        playerChoiceResponse.Description = playerChoiceResponseTemplate.Description;
        playerChoiceResponse.Confirmation = playerChoiceResponseTemplate.Confirmation;
        if (playerChoiceLocale)
        {
            if (PlayerChoiceResponseLocale const* playerChoiceResponseLocale = Trinity::Containers::MapGetValuePtr(playerChoiceLocale->Responses, playerChoiceResponseTemplate.ResponseId))
            {
                ObjectMgr::GetLocaleString(playerChoiceResponseLocale->Answer, locale, playerChoiceResponse.Answer);
                ObjectMgr::GetLocaleString(playerChoiceResponseLocale->Header, locale, playerChoiceResponse.Header);
                ObjectMgr::GetLocaleString(playerChoiceResponseLocale->SubHeader, locale, playerChoiceResponse.SubHeader);
                ObjectMgr::GetLocaleString(playerChoiceResponseLocale->ButtonTooltip, locale, playerChoiceResponse.ButtonTooltip);
                ObjectMgr::GetLocaleString(playerChoiceResponseLocale->Description, locale, playerChoiceResponse.Description);
                ObjectMgr::GetLocaleString(playerChoiceResponseLocale->Confirmation, locale, playerChoiceResponse.Confirmation);
            }
        }

        if (playerChoiceResponseTemplate.Reward)
        {
            playerChoiceResponse.Reward.emplace();
            playerChoiceResponse.Reward->TitleID = playerChoiceResponseTemplate.Reward->TitleId;
            playerChoiceResponse.Reward->PackageID = playerChoiceResponseTemplate.Reward->PackageId;
            playerChoiceResponse.Reward->SkillLineID = playerChoiceResponseTemplate.Reward->SkillLineId;
            playerChoiceResponse.Reward->SkillPointCount = playerChoiceResponseTemplate.Reward->SkillPointCount;
            playerChoiceResponse.Reward->ArenaPointCount = playerChoiceResponseTemplate.Reward->ArenaPointCount;
            playerChoiceResponse.Reward->HonorPointCount = playerChoiceResponseTemplate.Reward->HonorPointCount;
            playerChoiceResponse.Reward->Money = playerChoiceResponseTemplate.Reward->Money;
            playerChoiceResponse.Reward->Xp = playerChoiceResponseTemplate.Reward->Xp;

            for (PlayerChoiceResponseRewardItem const& item : playerChoiceResponseTemplate.Reward->Items)
            {
                playerChoiceResponse.Reward->Items.emplace_back();
                WorldPackets::Quest::PlayerChoiceResponseRewardEntry& rewardEntry = playerChoiceResponse.Reward->Items.back();
                rewardEntry.Item.ItemID = item.Id;
                rewardEntry.Quantity = item.Quantity;
                if (!item.BonusListIDs.empty())
                {
                    rewardEntry.Item.ItemBonus.emplace();
                    rewardEntry.Item.ItemBonus->BonusListIDs = item.BonusListIDs;
                }
            }
            for (PlayerChoiceResponseRewardEntry const& currency : playerChoiceResponseTemplate.Reward->Currency)
            {
                playerChoiceResponse.Reward->Items.emplace_back();
                WorldPackets::Quest::PlayerChoiceResponseRewardEntry& rewardEntry = playerChoiceResponse.Reward->Items.back();
                rewardEntry.Item.ItemID = currency.Id;
                rewardEntry.Quantity = currency.Quantity;
            }
            for (PlayerChoiceResponseRewardEntry const& faction : playerChoiceResponseTemplate.Reward->Faction)
            {
                playerChoiceResponse.Reward->Items.emplace_back();
                WorldPackets::Quest::PlayerChoiceResponseRewardEntry& rewardEntry = playerChoiceResponse.Reward->Items.back();
                rewardEntry.Item.ItemID = faction.Id;
                rewardEntry.Quantity = faction.Quantity;
            }
            for (PlayerChoiceResponseRewardItem const& item : playerChoiceResponseTemplate.Reward->ItemChoices)
            {
                playerChoiceResponse.Reward->ItemChoices.emplace_back();
                WorldPackets::Quest::PlayerChoiceResponseRewardEntry& rewardEntry = playerChoiceResponse.Reward->ItemChoices.back();
                rewardEntry.Item.ItemID = item.Id;
                rewardEntry.Quantity = item.Quantity;
                if (!item.BonusListIDs.empty())
                {
                    rewardEntry.Item.ItemBonus.emplace();
                    rewardEntry.Item.ItemBonus->BonusListIDs = item.BonusListIDs;
                }
            }
        }

        playerChoiceResponse.RewardQuestID = playerChoiceResponseTemplate.RewardQuestID;

        if (playerChoiceResponseTemplate.MawPower)
        {
            WorldPackets::Quest::PlayerChoiceResponseMawPower& mawPower = playerChoiceResponse.MawPower.emplace();
            mawPower.TypeArtFileID = playerChoiceResponseTemplate.MawPower->TypeArtFileID;
            mawPower.Rarity = playerChoiceResponseTemplate.MawPower->Rarity;
            mawPower.RarityColor = playerChoiceResponseTemplate.MawPower->RarityColor;
            mawPower.SpellID = playerChoiceResponseTemplate.MawPower->SpellID;
            mawPower.MaxStacks = playerChoiceResponseTemplate.MawPower->MaxStacks;
        }
    }

    // Resize to 4 if not Covenant Choice (covenant choice has 8 response!)
    if (playerChoice->Responses.size() > 4 && choiceId != 644)
        displayPlayerChoice.Responses.resize(4);

    // Resize to 3 if it's Warboard Choice
    if (playerChoiceResponses.size() > 3 && (choiceId == 342 || choiceId == 352))
        displayPlayerChoice.Responses.resize(3);

    player->SendDirectMessage(displayPlayerChoice.Write());
}

void Fluxurion::AddCreditForQuestObjective(Player* player, uint32 questID, uint32 objectiveID)
{
    if (Quest const* quest = sObjectMgr->GetQuestTemplate(questID))
        for (QuestObjective const& obj : quest->GetObjectives())
            if (uint32(obj.ObjectID) == objectiveID)
            {
                uint16 slot = player->FindQuestSlot(questID);
                if (!player->IsQuestObjectiveComplete(slot, quest, obj)) // just make it complete once
                {
                    player->SetQuestObjectiveData(obj, 1);
                    player->SendQuestUpdateAddCredit(quest, ObjectGuid::Empty, obj, 1);
                }
            }
}

// This function was slightly fucked up with commit 330f3f925f2d49a2ebb67e2ee5506f72c98407b9
lfg::LfgLockMap Fluxurion::GetLockedDungeons(ObjectGuid guid)
{
    TC_LOG_TRACE("lfg.data.player.dungeons.locked.get", "Player: {}, LockedDungeons.", guid.ToString());
    lfg::LfgLockMap lock;
    Player* player = ObjectAccessor::FindConnectedPlayer(guid);
    if (!player)
    {
        TC_LOG_WARN("lfg.data.player.dungeons.locked.get", "Player: {} not ingame while retrieving his LockedDungeons.", guid.ToString());
        return lock;
    }

    uint8 playerLevel = player->GetLevel();
    uint8 playerSelectedExpansion = Fluxurion::GetChromieTimeExpansionLevel(Fluxurion::GetChromieTime(player));
    uint32 contentTuningReplacementConditionMask = player->m_playerData->CtrOptions->ContentTuningConditionMask;
    bool denyJoin = !player->GetSession()->HasPermission(rbac::RBAC_PERM_JOIN_DUNGEON_FINDER);

    for (LFGDungeonsEntry const* dungeon : sLFGDungeonsStore)
    {
        uint32 lockStatus = [&]() -> uint32
        {
            if (denyJoin)
                return lfg::LFG_LOCKSTATUS_RAID_LOCKED;
            if (DisableMgr::IsDisabledFor(DISABLE_TYPE_MAP, dungeon->MapID, player))
                return lfg::LFG_LOCKSTATUS_NOT_IN_SEASON;
            if (DisableMgr::IsDisabledFor(DISABLE_TYPE_LFG_MAP, dungeon->MapID, player))
                return lfg::LFG_LOCKSTATUS_RAID_LOCKED;
            //if (sInstanceLockMgr.FindActiveInstanceLock(guid, { dungeon->MapID, Difficulty(dungeon->DifficultyID) }))
                //return LFG_LOCKSTATUS_RAID_LOCKED;
            if (playerLevel < 60)
            {
                if (playerSelectedExpansion == EXPANSION_CATACLYSM)
                {
                    if (dungeon->ExpansionLevel != EXPANSION_CATACLYSM && dungeon->ExpansionLevel != EXPANSION_CLASSIC)
                        return lfg::LFG_LOCKSTATUS_INSUFFICIENT_EXPANSION;
                }
                else
                {
                    if (dungeon->ExpansionLevel != playerSelectedExpansion)
                        return lfg::LFG_LOCKSTATUS_INSUFFICIENT_EXPANSION;
                }
                if (Optional<ContentTuningLevels> levels = sDB2Manager.GetContentTuningData(dungeon->ContentTuningID, contentTuningReplacementConditionMask))
                {
                    if (levels->MinLevel > playerLevel && dungeon->ExpansionLevel != EXPANSION_SHADOWLANDS) // we exclude min level check for SL
                        return lfg::LFG_LOCKSTATUS_TOO_LOW_LEVEL;
                    /*
                    if (levels->MaxLevel < playerLevel)
                        return lfg::LFG_LOCKSTATUS_TOO_HIGH_LEVEL;
                        */
                }
            }
            if (playerLevel == 60)
            {
                if (dungeon->ExpansionLevel == EXPANSION_DRAGONFLIGHT && dungeon->DifficultyID == DIFFICULTY_HEROIC)
                    return lfg::LFG_LOCKSTATUS_INSUFFICIENT_EXPANSION;
                if (dungeon->ExpansionLevel != EXPANSION_SHADOWLANDS && dungeon->ExpansionLevel != EXPANSION_DRAGONFLIGHT)
                    return lfg::LFG_LOCKSTATUS_INSUFFICIENT_EXPANSION;
            }
            if (playerLevel > 60 && playerLevel < 70)
            {
                if (dungeon->MinGear < player->GetAverageItemLevel())
                {
                    if (dungeon->ExpansionLevel != EXPANSION_DRAGONFLIGHT)
                        return lfg::LFG_LOCKSTATUS_INSUFFICIENT_EXPANSION;
                }
                else
                {
                    if (dungeon->ExpansionLevel != EXPANSION_DRAGONFLIGHT && dungeon->ExpansionLevel != EXPANSION_SHADOWLANDS)
                        return lfg::LFG_LOCKSTATUS_INSUFFICIENT_EXPANSION;
                }
                if (dungeon->ExpansionLevel == EXPANSION_DRAGONFLIGHT && dungeon->DifficultyID == DIFFICULTY_HEROIC)
                    return lfg::LFG_LOCKSTATUS_INSUFFICIENT_EXPANSION;
            }
            if (playerLevel == 70 && dungeon->ExpansionLevel != EXPANSION_DRAGONFLIGHT)
                return lfg::LFG_LOCKSTATUS_INSUFFICIENT_EXPANSION;
            if ((dungeon->Flags[0] & lfg::LFG_FLAG_SEASONAL) && !sLFGMgr->IsSeasonActive(dungeon->ID))
                return lfg::LFG_LOCKSTATUS_NOT_IN_SEASON;
            if (dungeon->MinGear > player->GetAverageItemLevel())
                return lfg::LFG_LOCKSTATUS_TOO_LOW_GEAR_SCORE;
            if (AccessRequirement const* ar = sObjectMgr->GetAccessRequirement(dungeon->MapID, Difficulty(dungeon->DifficultyID)))
            {
                if (ar->achievement && !player->HasAchieved(ar->achievement))
                    return lfg::LFG_LOCKSTATUS_MISSING_ACHIEVEMENT;
                if (player->GetTeam() == ALLIANCE && ar->quest_A && !player->GetQuestRewardStatus(ar->quest_A))
                    return lfg::LFG_LOCKSTATUS_QUEST_NOT_COMPLETED;
                if (player->GetTeam() == HORDE && ar->quest_H && !player->GetQuestRewardStatus(ar->quest_H))
                    return lfg::LFG_LOCKSTATUS_QUEST_NOT_COMPLETED;

                if (ar->item)
                {
                    if (!player->HasItemCount(ar->item) && (!ar->item2 || !player->HasItemCount(ar->item2)))
                        return lfg::LFG_LOCKSTATUS_MISSING_ITEM;
                }
                else if (ar->item2 && !player->HasItemCount(ar->item2))
                    return lfg::LFG_LOCKSTATUS_MISSING_ITEM;
            }
            if (dungeon->DifficultyID != DIFFICULTY_NORMAL && dungeon->DifficultyID != DIFFICULTY_HEROIC)
                return lfg::LFG_LOCKSTATUS_INSUFFICIENT_EXPANSION;
            /* @todo VoA closed if WG is not under team control (LFG_LOCKSTATUS_RAID_LOCKED)
            lockData = LFG_LOCKSTATUS_TOO_HIGH_GEAR_SCORE;
            lockData = LFG_LOCKSTATUS_ATTUNEMENT_TOO_LOW_LEVEL;
            lockData = LFG_LOCKSTATUS_ATTUNEMENT_TOO_HIGH_LEVEL;
            */

            return 0;
        }();

        if (lockStatus)
        {
            switch (lockStatus)
            {
            case lfg::LFG_LOCKSTATUS_INSUFFICIENT_EXPANSION:
                lock[dungeon->Entry()] = lfg::LfgLockInfoData(lockStatus, dungeon->MinGear, player->GetAverageItemLevel(), true);
                break;
            default:
                lock[dungeon->Entry()] = lfg::LfgLockInfoData(lockStatus, dungeon->MinGear, player->GetAverageItemLevel(), false);
                break;
            }
        }
    }

    return lock;
}

LFGDungeonsEntry Fluxurion::GetDungeonEntry(uint32 dungeonID)
{
    LFGDungeonsEntry dungeon;

    for (LFGDungeonsEntry const* _dungeon : sLFGDungeonsStore)
    {
        if (_dungeon->ID == dungeonID)
            dungeon = *_dungeon;
    }

    return dungeon;
}

bool Fluxurion::AccountHasAchievement(uint32 accountID, uint32 achievementID)
{
    QueryResult result = CharacterDatabase.PQuery("SELECT * FROM character_achievement WHERE guid IN (SELECT guid FROM characters WHERE account = {}) AND achievement = {}", accountID, achievementID);
    if (!result)
        return false;

    return true;
}

void Player::SetPetCustomization(uint32 CustomizedChrModelID, uint32 ChrCustomizationChoiceID)
{
    uint32 displayID = sDB2Manager.GetCreatureDisplayIDForCustomization(ChrCustomizationChoiceID);
    if (!displayID)
        return;

    TC_LOG_INFO("server.debug", "SetPetCustomization CustomizedChrModelID {} ChrCustomizationChoiceID {} displayID {}", CustomizedChrModelID, ChrCustomizationChoiceID, displayID);

    // Some HardCode incoming
    uint32 petCreatureEntry = 0;
    switch (CustomizedChrModelID)
    {
    case 148: // Imp
        petCreatureEntry = 416;
        break;
    case 176: // Felhunter
        petCreatureEntry = 417;
        break;
    case 180: // Voidwalker
        petCreatureEntry = 1860;
        break;
    case 181: // Felguard
        petCreatureEntry = 17252;
        break;
    case 182: // Doomguard
        petCreatureEntry = 78158;
        break;
    case 183: // Succubus
        petCreatureEntry = 1863;
        break;
    case 184: // Infernal
        petCreatureEntry = 78217;
        break;
    }

    if (petCreatureEntry)
        SetPetModel(displayID, petCreatureEntry);

    UnsummonPetTemporaryIfAny();
}

void Player::SetPetModel(uint32 displayID, uint32 petCreatureEntry, bool withSave/*= true*/)
{
    // Save model to DB
    if (withSave)
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_PET_MODELID);
        stmt->setUInt32(0, displayID);
        stmt->setUInt64(1, GetGUID().GetCounter());
        stmt->setUInt32(2, petCreatureEntry);
        CharacterDatabase.Execute(stmt);
    }

    switch (petCreatureEntry)
    {
    case 416: // Imp
        CastSpell(this, 688, true);
        break;
    case 1860: // Voidwalker
        CastSpell(this, 697, true);
        break;
    case 417: // Felhunter
        CastSpell(this, 691, true);
        break;
    case 1863: // Succubus (Sayaad)
        CastSpell(this, 366222, true);
        break;
    case 17252: // Felguard
        CastSpell(this, 30146, true);
        break;
    case 78217: // Infernal (Maintain)
        CastSpell(this, 157898, true);
        break;
    case 78158: // Doomguard (Maintain)
        CastSpell(this, 157757, true);
        break;
    }

    if (Pet* pet = GetPet())
        if (pet->GetEntry() == petCreatureEntry)
        {
            pet->SetDisplayId(displayID);
            Fluxurion::PetSizeCorrection(pet);
        }

}

void Fluxurion::PetSizeCorrection(Guardian* guardian)
{
    float sizeCorrection = 1.f;
    switch (guardian->GetDisplayId())
    {
    case 51650: // Abyssal Green
    case 16874: // Abyssal Orange
    case 48209: // Abyssal Blue
        sizeCorrection = 3.f;
        break;
    default:
        sizeCorrection = 1.f;
        break;
    }
    if (guardian->GetEntry() == PET_FELGUARD)
        sizeCorrection = 1.5f;

    if (sizeCorrection > 1.f)
        guardian->SetObjectScale(sizeCorrection);
}

void Fluxurion::PlayWarlockDemonSummoningVisual(Spell* spell)
{
    if (spell->GetCaster()->IsPlayer())
    {
        Player* playerCaster = spell->GetCaster()->ToPlayer();
        if (playerCaster->GetClass() == CLASS_WARLOCK && spell->GetSpellInfo()->GetEffect(EFFECT_0).Effect == SPELL_EFFECT_SUMMON_PET)
        {
            uint32 spellVisualID = 0;
            switch (spell->GetSpellInfo()->Id)
            {
            case 688: // Summon Imp
            case 30146: // Summon Felguard
                spellVisualID = 81019; // fel summon visual
                break;
            case 691: // Summon Felhunter
            case 1860: // Summon Voidwalker
            case 1863: // Summon Succubus (Sayaad)
                spellVisualID = 81021; // shadow summon visual
                break;
            default:
                spellVisualID = 81022; // void summon visual
                break;
            }
            Position spellVisualTarget = Fluxurion::GetPetSummonPosition(playerCaster);
            spellVisualTarget.RelocateOffset(Position(-0.5f, 0.f, -1.f, 0.f)); // set the portal visual back of the pet (minor correction)
            playerCaster->SendPlaySpellVisual(spellVisualTarget, spellVisualID, 0, 0, 100.0f * playerCaster->m_unitData->ModCastingSpeed);
        }
    }
}

ItemEntry const* DB2Manager::GetItemEntry(uint32 itemID)
{
    return sItemStore.LookupEntry(itemID);
}

CreatureDisplayInfoEntry const* DB2Manager::GetCreatureDisplayInfoEntry(uint32 creatureDisplayInfoID)
{
    return sCreatureDisplayInfoStore.LookupEntry(creatureDisplayInfoID);
}

ItemAppearanceEntry const* DB2Manager::GetItemAppearanceEntry(uint32 itemID)
{
    return sItemAppearanceStore.LookupEntry(itemID);
}

ChrSpecializationEntry const* DB2Manager::GetChrSpecializationByID(int16 specID)
{
    return sChrSpecializationStore.LookupEntry(specID);
}
