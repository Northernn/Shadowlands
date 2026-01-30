/* Copyright (c) 2023 - Fluxurion */

#include "BattlePayPackets.h"

#ifndef _sCharService
#define _sCharService

class CharacterService
{
	CharacterService() = default;
	~CharacterService() = default;
	
public:
    void SetRename(ObjectGuid charGUID);
    void ChangeFaction(ObjectGuid charGUID);
    void ChangeRace(ObjectGuid charGUID);
    void Customize(ObjectGuid charGUID);
    void RestoreDeletedCharacter(WorldSession* session);

    uint32 PrepareMail(CharacterDatabaseTransaction& trans, ObjectGuid charGUID, std::string const subject, std::string const body);
    void SendMail(CharacterDatabaseTransaction& trans, ObjectGuid charGUID, std::string subject, std::string body, std::vector<uint32> items);
    void PrepareInventory(CharacterDatabaseTransaction& trans, ObjectGuid charGUID);
    void EquipItems(CharacterDatabaseTransaction& trans, ObjectGuid charGUID, std::vector<uint32> loadOut);
    void ResetHomeBindAndPosition(CharacterDatabaseTransaction& trans, ObjectGuid charGUID, WorldLocation location, uint16 zoneId);
    void UpgradeCharacter(WorldSession* session, ObjectGuid charGUID, int8 specID, int8 setLevel);
    bool GearUpdateAvailable(ObjectGuid charGUID);
    void AddGearUpdate(ObjectGuid charGUID);
    void RemoveGearUpdate(ObjectGuid charGUID);
    void GetAccountCharacterListResult(WorldSession* session);
    std::vector<WorldPackets::BattlePay::AccountCharacterListEntry> GetAccountCharacterList(WorldSession* session);

	static CharacterService* instance();
};

#define sCharacterService CharacterService::instance()

#endif
