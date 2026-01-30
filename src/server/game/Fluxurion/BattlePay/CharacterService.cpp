/* Copyright (c) 2023 - Fluxurion */

#include "Flux.h"
#include "CharacterService.h"
#include "BattlePayPackets.h"
#include "CharacterCache.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "GameTime.h"
#include "Item.h"
#include "Log.h"
#include "LoginQueryHolder.h"
#include "Mail.h"
#include "MiscPackets.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "WorldSession.h"

CharacterService* CharacterService::instance()
{
    static CharacterService instance;
    return &instance;
}

void CharacterService::SetRename(ObjectGuid charGUID)
{
    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
    stmt->setUInt16(0, AT_LOGIN_RENAME);
    stmt->setUInt64(1, charGUID.GetCounter());
    CharacterDatabase.Execute(stmt);
}

void CharacterService::ChangeFaction(ObjectGuid charGUID)
{
    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
    stmt->setUInt16(0, AT_LOGIN_CHANGE_FACTION);
    stmt->setUInt64(1, charGUID.GetCounter());
    CharacterDatabase.Execute(stmt);
}

void CharacterService::ChangeRace(ObjectGuid charGUID)
{
    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
    stmt->setUInt16(0, AT_LOGIN_CHANGE_RACE);
    stmt->setUInt64(1, charGUID.GetCounter());
    CharacterDatabase.Execute(stmt);
}

void CharacterService::Customize(ObjectGuid charGUID)
{
    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
    stmt->setUInt16(0, AT_LOGIN_CUSTOMIZE);
    stmt->setUInt64(1, charGUID.GetCounter());
    CharacterDatabase.Execute(stmt);
}

void CharacterService::RestoreDeletedCharacter(WorldSession* session)
{
    //session->AddAuthFlag(AT_AUTH_FLAG_RESTORE_DELETED_CHARACTER);
    // Need sniff deleted char restore in current expansion!
}

// Offline mail sending prepare function
uint32 CharacterService::PrepareMail(CharacterDatabaseTransaction& trans, ObjectGuid charGUID,  std::string const subject, std::string const body)
{
    uint32 mailId = sObjectMgr->GenerateMailID();

    // Add to DB
    uint8 index = 0;
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_MAIL);
    stmt->setUInt64(index, mailId);
    stmt->setUInt8(++index, MAIL_NORMAL);
    stmt->setInt8(++index, MAIL_STATIONERY_DEFAULT);
    stmt->setUInt16(++index, 0);
    stmt->setUInt64(++index, charGUID.GetCounter());
    stmt->setUInt64(++index, charGUID.GetCounter());
    stmt->setString(++index, subject);
    stmt->setString(++index, body);
    stmt->setBool(++index, true);
    stmt->setInt64(++index, time(NULL) + 180 * DAY);
    stmt->setInt64(++index, time(NULL));
    stmt->setUInt64(++index, 0);
    stmt->setUInt64(++index, 0);
    stmt->setUInt8(++index, 0);
    trans->Append(stmt);

    return mailId;
}

// Offline mail sending function
void CharacterService::SendMail(CharacterDatabaseTransaction& trans, ObjectGuid charGUID, std::string subject, std::string body, std::vector<uint32> items)
{
    if (items.empty())
        return;

    uint32 mailId = PrepareMail(trans, charGUID, subject, body);
    CharacterDatabasePreparedStatement* stmt = NULL;

    for (uint32 itemID : items)
    {
        if (Item* item = Item::CreateItem(itemID, 1, ItemContext::NONE))
        {
            item->SaveToDB(trans);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_MAIL_ITEM);
            stmt->setUInt32(0, mailId);
            stmt->setUInt32(1, item->GetGUID().GetCounter());
            stmt->setUInt32(2, charGUID.GetCounter());
            trans->Append(stmt);
        }
        else
            TC_LOG_ERROR("sql.sql", "Can't create item %u for _SendMail in CharacterService. Skip.", itemID);
    }
}

// Offline inventory cleanup and send items in mail function
void CharacterService::PrepareInventory(CharacterDatabaseTransaction& trans, ObjectGuid charGUID)
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTERSERVICE_SEL_INVENTORY);
    stmt->setUInt32(0, charGUID.GetCounter());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
        return;

    uint32 mailId = PrepareMail(trans, charGUID, "Previously equipped items.", "Welcome back to the World of Warcraft! We've supplied you with a whole new set of appropriate-level gear, but attached you'll find all the old items and equipment you once carried. - The WoW Dev Team");

    uint32 itemCount = 0;
    do
    {
        if (itemCount > 11)
        {
            itemCount = 0;
            mailId = PrepareMail(trans, charGUID, "Previously equipped items.", "Welcome back to the World of Warcraft! We've supplied you with a whole new set of appropriate-level gear, but attached you'll find all the old items and equipment you once carried. - The WoW Dev Team");
        }

        uint32 itemGuid = (*result)[0].GetUInt32();

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_MAIL_ITEM);
        stmt->setUInt32(0, mailId);
        stmt->setUInt32(1, itemGuid);
        stmt->setUInt32(2, charGUID.GetCounter());
        trans->Append(stmt);

        itemCount++;
    } while (result->NextRow());

    // unequip after sending
    stmt = CharacterDatabase.GetPreparedStatement(CHARACTERSERVICE_DEL_INVENTORY);
    stmt->setUInt32(0, charGUID.GetCounter());
    trans->Append(stmt);
}

uint8 GetSlotEquipmentFromInventory(ItemTemplate const* proto)
{
    switch (proto->GetInventoryType())
    {
    case INVTYPE_HEAD:
        return EQUIPMENT_SLOT_HEAD;
    case INVTYPE_NECK:
        return EQUIPMENT_SLOT_NECK;
    case INVTYPE_SHOULDERS:
        return EQUIPMENT_SLOT_SHOULDERS;
    case INVTYPE_BODY:
        return EQUIPMENT_SLOT_BODY;
    case INVTYPE_CHEST:
        return EQUIPMENT_SLOT_CHEST;
    case INVTYPE_ROBE:
        return EQUIPMENT_SLOT_CHEST;
    case INVTYPE_WAIST:
        return EQUIPMENT_SLOT_WAIST;
    case INVTYPE_LEGS:
        return EQUIPMENT_SLOT_LEGS;
    case INVTYPE_FEET:
        return EQUIPMENT_SLOT_FEET;
    case INVTYPE_WRISTS:
        return EQUIPMENT_SLOT_WRISTS;
    case INVTYPE_HANDS:
        return EQUIPMENT_SLOT_HANDS;
    case INVTYPE_FINGER:
        return EQUIPMENT_SLOT_FINGER1;
    case INVTYPE_TRINKET:
        return EQUIPMENT_SLOT_TRINKET1;
    case INVTYPE_CLOAK:
        return EQUIPMENT_SLOT_BACK;
    case INVTYPE_RANGED:
    case INVTYPE_2HWEAPON:
    case INVTYPE_WEAPON:
    case INVTYPE_WEAPONMAINHAND:
    case INVTYPE_RANGEDRIGHT:
        return EQUIPMENT_SLOT_MAINHAND;
    case INVTYPE_WEAPONOFFHAND:
    case INVTYPE_SHIELD:
        return EQUIPMENT_SLOT_OFFHAND;
    default:
        return EQUIPMENT_SLOT_END;
        break;
    }
}

// Offline item equip function
void CharacterService::EquipItems(CharacterDatabaseTransaction& trans, ObjectGuid charGUID, std::vector<uint32> loadOut)
{
    std::vector<uint32> finalLoadout = loadOut;
    bool ring1done = false;
    bool trinket1done = false;
    bool onehanded1done = false;
    bool weaponEquipped = false;
    int8 nonequipcounter = 0;
    int8 bagcounter = 0;

    for (uint32 itemID : loadOut)
    {
        if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemID))
        {
            uint32 itemClass = itemTemplate->GetClass();
            uint32 itemSubClass = itemTemplate->GetSubClass();

            // Add 1h weapons twice for dual-wielding spec's
            if (itemClass == ITEM_CLASS_WEAPON &&
                   (itemSubClass == ITEM_SUBCLASS_WEAPON_AXE ||
                       itemSubClass == ITEM_SUBCLASS_WEAPON_DAGGER ||
                       itemSubClass == ITEM_SUBCLASS_WEAPON_SWORD ||
                       itemSubClass == ITEM_SUBCLASS_WEAPON_WARGLAIVES))
            {
                finalLoadout.push_back(itemID);
            }
        }
    }

    for (uint32 itemID : finalLoadout)
    {
        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemID);
        if (!itemTemplate)
            continue;

        if (Item* item = Item::CreateItem(itemID, 1, ItemContext::NONE))
        {
            uint32 itemClass = itemTemplate->GetClass();
            uint32 itemSubClass = itemTemplate->GetSubClass();

            if (itemClass == ITEM_CLASS_ARMOR || itemClass == ITEM_CLASS_WEAPON)
                item->AddBonuses(6771); // scale item to character's level

            item->SaveToDB(trans);

            int8 slot = GetSlotEquipmentFromInventory(itemTemplate);

            if (item->IsBag())
            {
                slot = INVENTORY_SLOT_BAG_START + bagcounter;
                bagcounter++;
            }

            if (slot == EQUIPMENT_SLOT_END)
            {
                slot = INVENTORY_SLOT_ITEM_START + nonequipcounter;
                nonequipcounter++;
            }

            if (slot == EQUIPMENT_SLOT_FINGER1 && !ring1done)
            {
                slot = EQUIPMENT_SLOT_FINGER2;
                ring1done = true;
            }

            if (slot == EQUIPMENT_SLOT_TRINKET1 && !trinket1done)
            {
                slot = EQUIPMENT_SLOT_TRINKET2;
                trinket1done = true;
            }

            // Add 1h weapons twice for dual-wielding spec's
            if (itemClass == ITEM_CLASS_WEAPON &&
                (itemSubClass == ITEM_SUBCLASS_WEAPON_AXE ||
                    itemSubClass == ITEM_SUBCLASS_WEAPON_DAGGER ||
                    itemSubClass == ITEM_SUBCLASS_WEAPON_SWORD ||
                    itemSubClass == ITEM_SUBCLASS_WEAPON_WARGLAIVES))
            {
                if (!onehanded1done)
                {
                    slot = EQUIPMENT_SLOT_MAINHAND;
                    onehanded1done = true;
                }
                else
                {
                    slot = EQUIPMENT_SLOT_OFFHAND;

                }
            }

            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_INVENTORY_ITEM);
            stmt->setUInt32(0, charGUID.GetCounter());
            stmt->setUInt32(1, 0);
            stmt->setUInt8(2, slot);
            stmt->setUInt32(3, item->GetGUID().GetCounter());
            trans->Append(stmt);
        }
    }
}

void CharacterService::ResetHomeBindAndPosition(CharacterDatabaseTransaction& trans, ObjectGuid charGUID, WorldLocation location, uint16 zoneId)
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PLAYER_HOMEBIND);
    stmt->setUInt64(0, charGUID.GetCounter());
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PLAYER_HOMEBIND);
    stmt->setUInt64(0, charGUID.GetCounter());

    stmt->setUInt16(1, location.GetMapId());
    stmt->setUInt16(2, zoneId);
    stmt->setFloat(3, location.GetPositionX());
    stmt->setFloat(4, location.GetPositionY());
    stmt->setFloat(5, location.GetPositionZ());
    trans->Append(stmt);

    Player::SavePositionInDB(location, zoneId, charGUID, trans);
}

void CharacterService::UpgradeCharacter(WorldSession* session, ObjectGuid charGUID, int8 specID, int8 setLevel)
{
    // Start Upgrade
    WorldPackets::BattlePay::CharacterUpgradeStarted startUpgrade;
    startUpgrade.CharGUID = charGUID;
    session->SendPacket(startUpgrade.Write());

    // Remove items from character and send them in mail
    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
    PrepareInventory(trans, charGUID);

    // Set new level
    if (setLevel)
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_LEVEL);
        stmt->setUInt32(0, setLevel);
        stmt->setUInt32(1, charGUID.GetCounter());
        trans->Append(stmt);
    }

    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTERSERVICE_SEL_CLASS);
    stmt->setUInt32(0, charGUID.GetCounter());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
        return;

    int8 classID = (*result)[0].GetInt8();

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTERSERVICE_SEL_RACE);
    stmt->setUInt32(0, charGUID.GetCounter());
    result = CharacterDatabase.Query(stmt);

    if (!result)
        return;

    int8 raceID = (*result)[0].GetInt8();

    // Get LoadOut items
    std::vector<uint32> loadOut = sDB2Manager.GetLoadOut(classID, 14);

    int8 sizeOfLoadOut = loadOut.size();

    // Equip new items
    if (sizeOfLoadOut)
        EquipItems(trans, charGUID, loadOut);

    // Reset homebind and position
    WorldLocation loc;
    uint16 zoneId = 0;

    if (Player::TeamForRace(raceID) == ALLIANCE)
    {
        // Stormwind 
        loc.WorldRelocate(0, -8438.93, 327.30f, 145.0f, 2.258f);
        zoneId = 1519;

        if (setLevel < 60) // Teleport to Chromie before level 60
        {
            // Chromie Location
            loc.WorldRelocate(0, -8195.f, 749.73f, 76.42f, 4.25f);
            zoneId = 1519;
        }
    }
    else
    {
        // Orgrimmar 
        loc.WorldRelocate(1, 1608.22f, -4375.17f, 23.3f, 3.74f);
        zoneId = 1637;

        if (setLevel < 60) // Teleport to Chromie before level 60
        {
            loc.WorldRelocate(1, 1561.47f, -4215.73f, 54.09f, 3.37f);
            zoneId = 1637;
        }
    }

    ResetHomeBindAndPosition(trans, charGUID, loc, zoneId);

    CharacterDatabase.CommitTransaction(trans);

    // Finish Upgrade
    WorldPackets::BattlePay::CharacterUpgradeComplete completeUpgrade;
    completeUpgrade.CharGUID = charGUID;
    completeUpgrade.LoadOutItemCount = sizeOfLoadOut;
    completeUpgrade.LoadOut = loadOut;
    session->SendPacket(completeUpgrade.Write());

    WorldPackets::Misc::InvalidatePlayer invalidatePlayer;
    invalidatePlayer.Guid = charGUID;
    sWorld->SendGlobalMessage(invalidatePlayer.Write());

    std::string charName;
    sCharacterCache->GetCharacterNameByGuid(charGUID, charName);
    sCharacterCache->DeleteCharacterCacheEntry(charGUID, charName);

    /// get all the data necessary for loading all characters (along with their pets) on the account
    std::shared_ptr<EnumCharactersQueryHolder> holder = std::make_shared<EnumCharactersQueryHolder>();
    if (!holder->Initialize(session->GetAccountId(), sWorld->getBoolConfig(CONFIG_DECLINED_NAMES_USED), false))
    {
        session->HandleCharEnum(*holder);
        return;
    }

    session->AddQueryHolderCallback(CharacterDatabase.DelayQueryHolder(holder)).AfterComplete([session](SQLQueryHolderBase const& result)
        {
            session->HandleCharEnum(static_cast<EnumCharactersQueryHolder const&>(result));
        });

    GetAccountCharacterList(session);
}

bool CharacterService::GearUpdateAvailable(ObjectGuid charGUID)
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTERSERVICE_SEL_GEARUPDATE);
    stmt->setUInt32(0, charGUID.GetCounter());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
        return false;

    Field* fields = result->Fetch();
    int8 gearUpdateAvailable = fields[0].GetInt8();
    time_t logoutTime = fields[1].GetInt64();
    time_t totaltime = fields[2].GetInt64();

    int8 rpeReqDays = sConfigMgr->GetIntDefault("GearUpdateRequiredDays", 60);

    if (totaltime == 0) // firstlogin
        return false;

    if (GameTime::GetGameTime() - logoutTime >= rpeReqDays * DAY || gearUpdateAvailable > 0)
        return true;

    return false;
}

void CharacterService::AddGearUpdate(ObjectGuid charGUID)
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTERSERVICE_UPD_GEARUPDATE);
    stmt->setInt8(0, 1);
    stmt->setUInt32(1, charGUID.GetCounter());
    CharacterDatabase.Execute(stmt);
}

void CharacterService::RemoveGearUpdate(ObjectGuid charGUID)
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTERSERVICE_SEL_GEARUPDATE);
    stmt->setUInt32(0, charGUID.GetCounter());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
        return;

    Field* fields = result->Fetch();
    int8 gearUpdate  = fields[0].GetInt8();

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTERSERVICE_UPD_GEARUPDATE);
    stmt->setInt8(0, gearUpdate - 1);
    stmt->setUInt32(1, charGUID.GetCounter());
    CharacterDatabase.Execute(stmt);
}

std::vector<WorldPackets::BattlePay::AccountCharacterListEntry> CharacterService::GetAccountCharacterList(WorldSession* session)
{
    std::vector<WorldPackets::BattlePay::AccountCharacterListEntry> characters = std::vector<WorldPackets::BattlePay::AccountCharacterListEntry>();

    auto result = CharacterDatabase.PQuery("SELECT guid, name, race, class, gender, level, logout_time FROM characters WHERE account = {}", session->GetAccountId());
    if (!result)
        return characters;

    do
    {
        Field* field = result->Fetch();

        ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(field[0].GetUInt32());
        std::string name = field[1].GetString();
        uint8 raceId = field[2].GetUInt8();
        uint8 classId = field[3].GetUInt8();
        uint8 gender = field[4].GetUInt8();
        int16 level = field[5].GetInt16();
        uint32 logoutTime = field[6].GetUInt32();

        WorldPackets::BattlePay::AccountCharacterListEntry charListEntry;
        charListEntry.AccountId = session->GetBattlenetAccountGUID();
        charListEntry.CharacterGuid = guid;
        charListEntry.Class = classId;
        charListEntry.LastPlayedTime = logoutTime;
        charListEntry.Level = level;
        charListEntry.Name = name;
        charListEntry.Race = raceId;
        charListEntry.RealmName = ""; // idk+idc how to get that
        charListEntry.Sex = gender;
        charListEntry.VirtualRealmAddress = 0; // idk+idc how to get that

        characters.push_back(charListEntry);
    } while (result->NextRow());

    return characters;
}

void CharacterService::GetAccountCharacterListResult(WorldSession* session)
{
    WorldPackets::BattlePay::GetAccountCharacterListResult getAccountCharacterListResult;
    getAccountCharacterListResult.Token = 0;
    getAccountCharacterListResult.UnkBit = false;

    auto charListEntry = GetAccountCharacterList(session);
    getAccountCharacterListResult.accountCharacterListEntry = charListEntry;

    for (auto character : charListEntry)
        sCharacterCache->UpdateCharacterData(character.CharacterGuid, character.Name, character.Sex, character.Race);

    session->SendPacket(getAccountCharacterListResult.Write());
}
