/*
 * Copyright 2023 Fluxurion
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

#ifndef _BATTLE_PAY_DATA_STORE_H
#define _BATTLE_PAY_DATA_STORE_H

#include "BattlePayPackets.h"
#include "BattlePayMgr.h"

namespace BattlePayData
{
    struct ProductTemplate
    {
        uint32 Entry;
        Optional<uint32> IconFileDataID;
        Optional<uint32> UIModelSceneID;
        std::vector<std::string> PreviewTitles;
        std::vector<uint32> PreviewCreatureDisplayIDs;
        std::vector<uint32> PreviewUIModelSceneIDs;
        std::vector<uint32> PreviewTransmogSets;
        std::string Title;
        std::string Title2;
        std::string Description;
        std::string Description2;
        std::string Description3;
        std::string Description4;
        std::string Description5;
        uint32 IconBorder;
        uint32 UiTextureAtlasMemberID;
        uint32 CardType;
        uint32 ProductMultiplier;
        uint8 Type;
        uint32 ItemID;
        uint32 ItemCount;
        uint32 MountSpellID;
        uint32 BattlePetSpeciesCreatureID;
        uint32 TransmogSetID;
        bool AlreadyOwned;
        int32 NormalPrice;
        int32 CurrentPrice;
        std::vector<uint32> BundleProductIDs;
        uint32 Flags;
        uint32 ChoiceType;
        uint32 GroupID;
        uint32 Ordering;
        uint32 VasServiceType;
        uint8 StoreDeliveryType;
        uint32 CustomValue;
        uint8 TitleColorIndex;
    };

    struct Group
    {
        uint32 Entry;
        uint32 GroupId;
        uint32 IconFileDataID;
        uint8 DisplayType;
        uint32 Ordering;
        uint32 Unk;
        uint32 MainGroupID;
        std::string Name;
        std::string Description;
    };
};

class TC_GAME_API BattlePayDataStoreMgr
{
    BattlePayDataStoreMgr();
    ~BattlePayDataStoreMgr();

public:
    static BattlePayDataStoreMgr* instance();

    void Initialize();
    std::unordered_map<uint32, BattlePayData::Group> GetProductGroups();
    std::unordered_map<uint32, BattlePayData::ProductTemplate> GetProductTemplates();
    BattlePayData::ProductTemplate const* GetProductTemplate(uint32 entry);
    BattlePayData::ProductTemplate const* GetRandomProductTemplate();

private:
    void LoadProductGroups();
    void LoadProductTemplates();
};

#define sBattlePayDataStore BattlePayDataStoreMgr::instance()

#endif
