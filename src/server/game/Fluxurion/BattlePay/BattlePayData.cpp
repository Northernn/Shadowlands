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

#include "BattlePayData.h"
#include "BattlePayMgr.h"
#include "Containers.h"
#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include <sstream>

BattlePayDataStoreMgr::BattlePayDataStoreMgr() = default;

BattlePayDataStoreMgr::~BattlePayDataStoreMgr() = default;

BattlePayDataStoreMgr* BattlePayDataStoreMgr::instance()
{
    static BattlePayDataStoreMgr instance;
    return &instance;
}

namespace
{
    std::unordered_map<uint32, BattlePayData::ProductTemplate> _productTemplates;
    std::unordered_map<uint32, BattlePayData::Group> _productGroups;
}

void BattlePayDataStoreMgr::Initialize()
{
    LoadProductTemplates();
    LoadProductGroups();
}

void _LoadProductTemplates(Field* fields)
{
    uint32 entry = fields[0].GetUInt32();

    BattlePayData::ProductTemplate productTemplate;

    productTemplate.Entry = entry;
    productTemplate.IconFileDataID = fields[1].GetUInt32();
    productTemplate.UIModelSceneID = fields[2].GetUInt32();

    std::stringstream previewTitles_stream(fields[3].GetString());
    std::string previewTitles;
    while (std::getline(previewTitles_stream, previewTitles, ','))
        productTemplate.PreviewTitles.push_back(previewTitles.c_str());

    std::stringstream previewCreatureDisplayIDs_stream(fields[4].GetString());
    std::string previewCreatureDisplayIDs;
    while (std::getline(previewCreatureDisplayIDs_stream, previewCreatureDisplayIDs, ','))
        productTemplate.PreviewCreatureDisplayIDs.push_back(atol(previewCreatureDisplayIDs.c_str()));

    std::stringstream previewUIModelSceneIDs_stream(fields[5].GetString());
    std::string previewUIModelSceneIDs;
    while (std::getline(previewUIModelSceneIDs_stream, previewUIModelSceneIDs, ','))
        productTemplate.PreviewUIModelSceneIDs.push_back(atol(previewUIModelSceneIDs.c_str()));

    std::stringstream previewTransmogSets_stream(fields[6].GetString());
    std::string previewTransmogSets;
    while (std::getline(previewTransmogSets_stream, previewTransmogSets, ','))
        productTemplate.PreviewTransmogSets.push_back(atol(previewTransmogSets.c_str()));

    productTemplate.Title = fields[7].GetString();
    productTemplate.Title2 = fields[8].GetString();
    productTemplate.Description = fields[9].GetString();
    productTemplate.Description2 = fields[10].GetString();
    productTemplate.Description3 = fields[11].GetString();
    productTemplate.Description4 = fields[12].GetString();
    productTemplate.Description5 = fields[13].GetString();
    productTemplate.IconBorder = fields[14].GetUInt32();
    productTemplate.UiTextureAtlasMemberID = fields[15].GetUInt32();
    productTemplate.CardType = fields[16].GetUInt32();
    productTemplate.ProductMultiplier = fields[17].GetUInt32();
    productTemplate.Type = fields[18].GetUInt32();
    productTemplate.ItemID = fields[19].GetUInt32();
    productTemplate.ItemCount = fields[20].GetUInt32();
    productTemplate.MountSpellID = fields[21].GetUInt32();
    productTemplate.BattlePetSpeciesCreatureID = fields[22].GetUInt32();
    productTemplate.TransmogSetID = fields[23].GetUInt32();
    productTemplate.AlreadyOwned = fields[24].GetUInt32();
    productTemplate.NormalPrice = fields[25].GetInt32();
    productTemplate.CurrentPrice = fields[26].GetInt32();
    // another cool Fluxurion thing: multiple subproducts can be added in one column
    std::stringstream subproducts_stream(fields[27].GetString());
    std::string subproducts;
    while (std::getline(subproducts_stream, subproducts, ','))
        productTemplate.BundleProductIDs.push_back(atol(subproducts.c_str()));
    productTemplate.Flags = fields[28].GetUInt32();
    productTemplate.ChoiceType = fields[29].GetUInt32();
    productTemplate.GroupID = fields[30].GetUInt32();
    productTemplate.Ordering = fields[31].GetUInt32();
    productTemplate.VasServiceType = fields[32].GetUInt32();
    productTemplate.StoreDeliveryType = fields[33].GetUInt32();
    productTemplate.CustomValue = fields[34].GetUInt32();
    productTemplate.TitleColorIndex = fields[35].GetUInt32();

    _productTemplates.insert(std::make_pair(entry, productTemplate));
}

void BattlePayDataStoreMgr::LoadProductTemplates()
{
    TC_LOG_INFO("server.loading", "Loading BattlePay Product Templates ...");
    _productTemplates.clear();

    auto result = WorldDatabase.PQuery("SELECT `Entry`, `IconFileDataID`, `UIModelSceneID`, `PreviewTitles`, `PreviewCreatureDisplayIDs`, `PreviewUIModelSceneIDs`, `PreviewTransmogSets`, `Title`, `Title2`, `Description`, `Description2`, `Description3`, `Description4`, `Description5`, `IconBorder`, `UiTextureAtlasMemberID`, `CardType`, `ProductMultiplier`, `Type`, `ItemID`, `ItemCount`, `MountSpellID`, `BattlePetSpeciesCreatureID`, `TransmogSetID`, `AlreadyOwned`, `NormalPrice`, `CurrentPrice`, `BundleProductIDs`, `Flags`, `ChoiceType`, `GroupID`, `Ordering`, `VasServiceType`, `StoreDeliveryType`, `CustomValue`, `TitleColorIndex` FROM `battlepay_product_template`");
    if (!result)
        return;

    auto oldMsTime = getMSTime();

    _productTemplates.rehash(result->GetRowCount());
    do
    {
        Field* fields = result->Fetch();
        _LoadProductTemplates(fields);
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded {} BattlePay Product Templates in {} ms", uint64(_productTemplates.size()), GetMSTimeDiffToNow(oldMsTime));
}

void BattlePayDataStoreMgr::LoadProductGroups()
{
    TC_LOG_INFO("server.loading", "Loading BattlePay Product Groups ...");
    _productGroups.clear();

    auto result = WorldDatabase.PQuery("SELECT Entry, GroupId, IconFileDataID, DisplayType, Ordering, Unk, MainGroupID, Name, Description FROM battlepay_group");
    if (!result)
        return;

    auto oldMsTime = getMSTime();

    do
    {
        auto fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        BattlePayData::Group productGroup;

        productGroup.Entry = entry;
        productGroup.GroupId = fields[1].GetUInt32();
        productGroup.IconFileDataID = fields[2].GetUInt32();
        productGroup.DisplayType = fields[3].GetUInt32();
        productGroup.Ordering = fields[4].GetUInt32();
        productGroup.Unk = fields[5].GetUInt32();
        productGroup.MainGroupID = fields[6].GetUInt32();
        productGroup.Name = fields[7].GetString();
        productGroup.Description = fields[8].GetString();


        _productGroups.insert(std::make_pair(entry, productGroup));

    } while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded {} BattlePay Product Groups in {} ms", uint64(_productGroups.size()), GetMSTimeDiffToNow(oldMsTime));
}

std::unordered_map<uint32, BattlePayData::ProductTemplate> BattlePayDataStoreMgr::GetProductTemplates()
{
    return _productTemplates;
}

std::unordered_map<uint32, BattlePayData::Group> BattlePayDataStoreMgr::GetProductGroups()
{
    return _productGroups;
}

BattlePayData::ProductTemplate const* BattlePayDataStoreMgr::GetProductTemplate(uint32 entry)
{
    std::unordered_map<uint32, BattlePayData::ProductTemplate>::const_iterator itr = _productTemplates.find(entry);
    if (itr != _productTemplates.end())
        return &(itr->second);

    return nullptr;
}

BattlePayData::ProductTemplate const* BattlePayDataStoreMgr::GetRandomProductTemplate()
{
    int random_index = rand() % _productTemplates.size();

    return GetProductTemplate(random_index);
}
