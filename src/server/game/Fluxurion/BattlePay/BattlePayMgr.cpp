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

#include "Flux.h"
#include "Common.h"
#include "CollectionMgr.h"
#include "ObjectMgr.h"
#include "BattlePayMgr.h"
#include "CollectionMgr.h"
#include "CharacterService.h"
#include "WorldSession.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Player.h"
#include "BattlePayData.h"
#include "DatabaseEnv.h"
#include "ScriptMgr.h"
#include "AccountMgr.h"
#include "CollectionMgr.h"
#include <sstream>
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Language.h"
#include "SpellPackets.h"
#include "Chat.h"
#include "DB2Stores.h"
#include "BattlePayPackets.h"
#include "Pet.h"
#include "Item.h"
#include "LoginQueryHolder.h"
#include "MiscPackets.h"
#include "Unit.h"

using namespace Battlepay;

BattlepayManager::BattlepayManager(WorldSession* session)
{
    _session = session;
    _purchaseIDCount = 0;
    _distributionIDCount = 0;
    _walletName = "Credits";
}

BattlepayManager::~BattlepayManager() = default;

void BattlepayManager::RegisterStartPurchase(Purchase purchase)
{
    _actualTransaction = purchase;
}

uint64 BattlepayManager::GenerateNewPurchaseID()
{
    return uint64(0x1E77800000000000 | ++_purchaseIDCount);
}

uint64 BattlepayManager::GenerateNewDistributionId()
{
    return uint64(0x1E77800000000000 | ++_distributionIDCount);
}

Purchase* BattlepayManager::GetPurchase()
{
    return &_actualTransaction;
}

std::string const& BattlepayManager::GetDefaultWalletName() const
{
    return _walletName;
}

uint32 BattlepayManager::GetShopCurrency() const
{
    return sWorld->getIntConfig(CONFIG_BATTLE_PAY_CURRENCY);
}

bool BattlepayManager::IsAvailable() const
{
    if (AccountMgr::IsAdminAccount(_session->GetSecurity()))
        return true;

    return sWorld->getBoolConfig(CONFIG_BATTLE_PAY_ENABLED);
}

void BattlepayManager::SavePurchase(Purchase* purchase)
{
    BattlePayData::ProductTemplate const* productTemplate = sBattlePayDataStore->GetProductTemplate(purchase->ProductID);
    if (!productTemplate)
        return;

    ObjectGuid guid = purchase->TargetCharacter.GetCounter() ? purchase->TargetCharacter : _session->GetAccountGUID();

    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_BATTLE_PAY_PURCHASE);
    stmt->setUInt32(0, Battlepay::DistributionStatus::BATTLE_PAY_DIST_STATUS_AVAILABLE);
    stmt->setUInt32(1, _session->GetAccountId());
    stmt->setUInt32(2, GetVirtualRealmAddress());
    stmt->setUInt32(3, guid.GetCounter());
    stmt->setUInt32(4, purchase->ProductID);
    stmt->setString(5, productTemplate->Title);
    stmt->setUInt32(6, purchase->CurrentPrice);
    stmt->setString(7, _session->GetRemoteAddress());
    LoginDatabase.Execute(stmt);
}

std::vector<Battlepay::PurchaseData> BattlepayManager::GetPurchases(uint32 accountID)
{
    std::vector<Battlepay::PurchaseData> purchases = std::vector<Battlepay::PurchaseData>();

    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_BATTLE_PAY_PURCHASES);
    stmt->setUInt32(0, accountID);

    PreparedQueryResult result = LoginDatabase.Query(stmt);

    if (!result)
        return purchases;

    do
    {
        Field* field = result->Fetch();

        Battlepay::PurchaseData data;
        data.PurchaseID = field[0].GetUInt32();
        data.PurchaseStatus = field[1].GetInt8();
        data.characterGuid = ObjectGuid::Create<HighGuid::Player>(field[2].GetUInt32());
        data.productID = field[3].GetUInt32();
        data.productName = field[4].GetString();

        purchases.push_back(data);
    } while (result->NextRow());

    return purchases;
}

static void LearnSkillRecipesHelper(Player* player, uint32 skillId)
{
    uint32 classmask = player->GetClassMask();

    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j);
        if (!skillLine)
            continue;

        // wrong skill
        if (skillLine->SkillLine != int32(skillId))
            continue;

        // not high rank
        if (skillLine->SupercedesSpell)
            continue;

        // skip racial skills
        if (!skillLine->RaceMask.GetMaskForRace(player->GetRace()))
            continue;

        // skip wrong class skills
        if (skillLine->ClassMask && (skillLine->ClassMask & classmask) == 0)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(skillLine->Spell, DIFFICULTY_NONE);
        if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, player, false))
            continue;

        player->LearnSpell(skillLine->Spell, false);
    }
}

bool LearnAllRecipesInProfession(Player* player, SkillType skill)
{
    SkillLineEntry const* SkillInfo = sSkillLineStore.LookupEntry(skill);
    if (!SkillInfo)
        return false;
    ChatHandler handler(player->GetSession());
    //char* skill_name;

    LearnSkillRecipesHelper(player, SkillInfo->ID);

    uint16 maxLevel = player->GetPureMaxSkillValue(SkillInfo->ID);
    player->SetSkill(SkillInfo->ID, player->GetSkillStep(SkillInfo->ID), maxLevel, maxLevel);

    return true;
}

void BattlepayManager::ProcessDelivery(Purchase* purchase)
{
    uint32 accID = _session->GetAccountId();
    Player* player = _session->GetPlayer();

    BattlePayData::ProductTemplate const* productTemplate = sBattlePayDataStore->GetProductTemplate(purchase->ProductID);
    if (!productTemplate)
        return;

    std::vector<uint32> itemstosendinmail;

    ItemTemplate const* item = sObjectMgr->GetItemTemplate(productTemplate->ItemID);

    switch (productTemplate->Type)
    {
    case Battlepay::Item_: // 0
        itemstosendinmail.clear();
        if (item && player)
            if (player->GetFreeInventorySpace() >= productTemplate->ItemCount)
                player->AddItemWithToast(productTemplate->ItemID, productTemplate->ItemCount, {});
            else
                Fluxurion::SendABunchOfItemsInMail(player, { productTemplate->ItemID }, "Ingame Shop item delivery", {});
        else
            _session->SendStartPurchaseResponse(_session, *GetPurchase(), Battlepay::Error::PurchaseDenied);

        if (itemstosendinmail.size() > 0)
            Fluxurion::SendABunchOfItemsInMail(player, itemstosendinmail, "Ingame Shop Item Delivery", {});
        break;

    case Battlepay::LevelBoost: // 1
        break;

    case Battlepay::Pet: // 2
        if (player) // if logged in
            player->GetSession()->GetBattlePayMgr()->AddBattlePetFromBpayShop(productTemplate->BattlePetSpeciesCreatureID);
        else
            _session->SendStartPurchaseResponse(_session, *GetPurchase(), Battlepay::Error::PurchaseDenied);
        break;

    case Battlepay::Mount: // 3
        _session->GetCollectionMgr()->AddMount(productTemplate->MountSpellID, MountStatusFlags::MOUNT_STATUS_NONE);
        break;

    case Battlepay::WoWToken: // 4
        if (item && player)
            if (player->GetFreeInventorySpace() >= productTemplate->ItemCount)
                player->AddItemWithToast(productTemplate->ItemID, productTemplate->ItemCount, {});
            else
                Fluxurion::SendABunchOfItemsInMail(player, { productTemplate->ItemID }, "Ingame Shop - WoW Token Delivery", {});
        else
            _session->SendStartPurchaseResponse(_session, *GetPurchase(), Battlepay::Error::PurchaseDenied);
        break;

    case Battlepay::NameChange: // 5
        if (player)
            player->SetAtLoginFlag(AT_LOGIN_RENAME);
        else
            sCharacterService->SetRename(purchase->TargetCharacter);
        break;
    case Battlepay::FactionChange: // 6
        if (player)
            player->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);
        else
            sCharacterService->ChangeFaction(purchase->TargetCharacter);
        break;
    case Battlepay::RaceChange: // 8
        if (player)
            player->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);
        else
            sCharacterService->ChangeRace(purchase->TargetCharacter);
        break;

    case Battlepay::CharacterTransfer: // 11
        // if u have multiple realms u have to implement this xD otherwise it sends error
        _session->SendStartPurchaseResponse(_session, *GetPurchase(), Battlepay::Error::PurchaseDenied);
        break;

    case Battlepay::Toy: // 14
        _session->GetCollectionMgr()->AddToy(productTemplate->ItemID, false, productTemplate->ItemCount);
        break;

    case Battlepay::Expansion: // 18
        if (player) // if logged in
            //player->SendMovieStart(936); // Play SL Intro - xD what else in a private server we don't sell expansions
            player->SendMovieStart(957); // Play SL Outro - we are preparing for dragonflight xD
        break;

    case Battlepay::GameTime: // 20
        if (item && player)
            if (player->GetFreeInventorySpace() >= productTemplate->ItemCount)
                player->AddItemWithToast(productTemplate->ItemID, productTemplate->ItemCount, {});
            else
                Fluxurion::SendABunchOfItemsInMail(player, { productTemplate->ItemID }, "Ingame Shop - WoW Token Delivery", {});
        else
            _session->SendStartPurchaseResponse(_session, *GetPurchase(), Battlepay::Error::PurchaseDenied);
        break;

    case Battlepay::GuildNameChange: // 21
    case Battlepay::GuildFactionChange: // 22
    case Battlepay::GuildTransfer: // 23
    case Battlepay::GuildFactionTranfer: // 24
        // Not implemented yet - need some more guild functions e.g.: getmembers
        _session->SendStartPurchaseResponse(_session, *GetPurchase(), Battlepay::Error::PurchaseDenied);
        break;

    case Battlepay::TransmogAppearance: // 26
        _session->GetCollectionMgr()->AddTransmogSet(productTemplate->TransmogSetID);
        break;

    case Battlepay::Gold: // 30
        if (player)
            player->ModifyMoney(productTemplate->CustomValue);
        break;

    case Battlepay::Currency: // 31
        if (player)
            player->ModifyCurrency(productTemplate->ItemID, productTemplate->ItemCount); // implement currencyID in DB
        break;

    case Battlepay::ItemSet: // 100
    {
        if (player)
        {
            ItemTemplateContainer const& its = sObjectMgr->GetItemTemplateStore();
            for (auto const& itemTemplatePair : its)
            {
                if (itemTemplatePair.second.GetItemSet() != productTemplate->CustomValue)
                    continue;

                ItemPosCountVec dest;
                InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemTemplatePair.first, 1);
                if (msg == EQUIP_ERR_OK)
                {
                    Item* item = player->StoreNewItem(dest, itemTemplatePair.first, true, {}, GuidSet());

                    player->SendNewItem(item, 1, true, false);
                }
                else
                {
                    itemstosendinmail.push_back(itemTemplatePair.second.GetId());
                }
            }
            if (itemstosendinmail.size() > 0)
                Fluxurion::SendABunchOfItemsInMail(player, itemstosendinmail, "Ingame Shop - You bought an item set!", {});
        }
        else
            _session->SendStartPurchaseResponse(_session, *GetPurchase(), Battlepay::Error::PurchaseDenied);
    }
    break;

    case Battlepay::Buff2Hour: // 101
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(productTemplate->CustomValue, DIFFICULTY_NONE);

        if (productTemplate->CustomValue && player && spellInfo)
        {
            CastSpellExtraArgs args;
            args.SetTriggerFlags(TRIGGERED_FULL_MASK);

            if (spellInfo->HasAura(SPELL_AURA_SCHOOL_ABSORB))
                args.AddSpellMod(SpellValueMod::SPELLVALUE_BASE_POINT0, player->GetHealth() * 100); // 100 * player's hp absorb

            args.AddSpellMod(SpellValueMod::SPELLVALUE_DURATION, 2 * 60 * 60 * 1000); // 2 Hour
            player->CastSpell(player, productTemplate->CustomValue, args);
        }
        else
            _session->SendStartPurchaseResponse(_session, *GetPurchase(), Battlepay::Error::PurchaseDenied);
    }
    break;

    case Battlepay::Loadout: // 103
    {
        if (productTemplate->CustomValue && player)
            Fluxurion::GearUpByLoadout(player, productTemplate->CustomValue, { 6771 /*scaled to level*/ });
        else
            _session->SendStartPurchaseResponse(_session, *GetPurchase(), Battlepay::Error::PurchaseDenied);
    }
    break;

    case Battlepay::GearUpdate: // 104
    {
        if (player)
            sCharacterService->AddGearUpdate(player->GetGUID());
        else if (purchase->TargetCharacter.GetCounter())
            sCharacterService->AddGearUpdate(purchase->TargetCharacter);
        else
            _session->SendStartPurchaseResponse(_session, *GetPurchase(), Battlepay::Error::PurchaseDenied);
    }
    break;

    case Battlepay::Achievement: // 105
    {
        if (player)
        {
            player->CompletedAchievement(productTemplate->CustomValue);
            player->SaveToDB();
        }
        else
            _session->SendStartPurchaseResponse(_session, *GetPurchase(), Battlepay::Error::PurchaseDenied);
    }
    break;

    case Battlepay::MultipleQuest: // 106
    {
        if (player)
        {
            for (uint32 questID : productTemplate->BundleProductIDs)
            {
                const Quest* quest = sObjectMgr->GetQuestTemplate(questID);

                if (!quest || player->GetQuestStatus(questID))
                    continue;

                player->AddQuest(quest, player);
                player->RewardQuest(quest, LootItemType::Item, 0, player);
            }

            player->SaveToDB();
        }
        else
            _session->SendStartPurchaseResponse(_session, *GetPurchase(), Battlepay::Error::PurchaseDenied);
    }
    break;

    }

    // Refresh char screen if player is not loggedin
    if (!player)
    {
        sCharacterService->GetAccountCharacterListResult(_session);
        SendDistributionListResponse();
        _session->SendFeatureSystemStatus();

        /// get all the data necessary for loading all characters (along with their pets) on the account
        std::shared_ptr<EnumCharactersQueryHolder> holder = std::make_shared<EnumCharactersQueryHolder>();
        if (!holder->Initialize(_session->GetAccountId(), sWorld->getBoolConfig(CONFIG_DECLINED_NAMES_USED), false))
        {
            _session->HandleCharEnum(*holder);
            return;
        }

        _session->AddQueryHolderCallback(CharacterDatabase.DelayQueryHolder(holder)).AfterComplete([this](SQLQueryHolderBase const& result)
            {
                _session->HandleCharEnum(static_cast<EnumCharactersQueryHolder const&>(result));
            });
    }

    sScriptMgr->OnBattlePayProductDelivery(_session, *productTemplate);
}

bool BattlepayManager::AlreadyOwnProduct(uint32 itemId) const
{
    auto const& player = _session->GetPlayer();
    if (player)
    {
        auto itemTemplate = sObjectMgr->GetItemTemplate(itemId);
        if (!itemTemplate)
            return true;

        for (auto itr : itemTemplate->Effects)
            if (itr->TriggerType == ITEM_SPELLTRIGGER_ON_LEARN && player->HasSpell(itr->SpellID))
                return true;

        uint8 inventoryEnd = INVENTORY_SLOT_ITEM_START + player->GetInventorySlotCount();
        for (uint8 i = INVENTORY_SLOT_ITEM_START; i < inventoryEnd; i++)
            if (player->GetItemCount(itemId))
                return true;
    }

    return false;
}

void BattlepayManager::SendProductList()
{
    WorldPackets::BattlePay::ProductListResponse response;
    Player* player = _session->GetPlayer(); // it's a false value if player is in character screen

    if (!IsAvailable() || sBattlePayDataStore->GetProductTemplates().size() == 0)
    {
        response.Result = ProductListResult::LockUnk1;
        _session->SendPacket(response.Write());
        return;
    }

    response.Result = ProductListResult::Available;
    response.CurrencyID = GetShopCurrency() > 0 ? GetShopCurrency() : 1;

    // BATTLEPAY GROUP
    for (auto _group : sBattlePayDataStore->GetProductGroups())
    {
        WorldPackets::BattlePay::Group group;
        group.GroupId = _group.second.GroupId;
        group.IconFileDataID = _group.second.IconFileDataID;
        group.DisplayType = _group.second.DisplayType;
        group.Ordering = _group.second.Ordering;
        group.Unk = _group.second.Unk;
        group.MainGroupID = _group.second.MainGroupID;
        group.Name = _group.second.Name;
        group.Description = _group.second.Description;

        if (!player) // when logged out don't show everything
        {
            if (group.GroupId == 22 || group.MainGroupID == 22) // restrict to services until i implement offline sending completely
                response.ProductGroups.push_back(group);
            else
                continue;
        }
        else
            response.ProductGroups.push_back(group);
    }

    // SHOP
    for (auto productTemplate : sBattlePayDataStore->GetProductTemplates())
    {
        WorldPackets::BattlePay::Shop shop;
        shop.EntryID = productTemplate.second.Entry;
        shop.GroupID = productTemplate.second.GroupID;
        shop.ProductID = productTemplate.second.Entry;
        shop.Ordering = productTemplate.second.Ordering;
        shop.VasServiceType = productTemplate.second.VasServiceType;
        shop.StoreDeliveryType = productTemplate.second.StoreDeliveryType;

        auto shopDisplay = WriteDisplayInfo(productTemplate.second.Entry, productTemplate.second.TitleColorIndex);
        if (std::get<0>(shopDisplay))
        {
            shop.Display.emplace();
            shop.Display = std::get<1>(shopDisplay);
        }

        // Restrict transmog sets to player class
        if (player && productTemplate.second.Type == 26 && productTemplate.second.CustomValue && productTemplate.second.CustomValue != player->GetClass())
            continue;

        response.Shops.push_back(shop);
    }

    // PRODUCT INFO
    for (auto productTemplate : sBattlePayDataStore->GetProductTemplates())
    {
        WorldPackets::BattlePay::ProductInfo productinfo;
        productinfo.ProductID = productTemplate.second.Entry;
        productinfo.NormalPrice = productTemplate.second.NormalPrice;
        productinfo.CurrentPrice = productTemplate.second.CurrentPrice;
        productinfo.BundleProductIDs = productTemplate.second.BundleProductIDs;
        productinfo.Unk1 = 15;
        productinfo.Unk2 = 0;
        productinfo.UnkInts = {};
        productinfo.Flags = productTemplate.second.Flags;
        productinfo.ChoiceType = productTemplate.second.ChoiceType;

        auto productinfoDisplay = WriteDisplayInfo(productTemplate.second.Entry, productTemplate.second.TitleColorIndex);
        if (std::get<0>(productinfoDisplay))
        {
            productinfo.Display.emplace();
            productinfo.Display = std::get<1>(productinfoDisplay);
        }
        response.ProductInfos.push_back(productinfo);
    }

    // PRODUCT
    for (auto productTemplate : sBattlePayDataStore->GetProductTemplates())
    {
        WorldPackets::BattlePay::Product product;
        product.ProductId = productTemplate.second.Entry;
        product.Type = productTemplate.second.Type;
        product.ItemID = productTemplate.second.Flags;
        product.ItemCount = productTemplate.second.ItemCount;
        product.MountSpellID = productTemplate.second.MountSpellID;
        product.BattlePetSpeciesCreatureID = productTemplate.second.BattlePetSpeciesCreatureID;
        product.Unk1 = 0;
        product.Unk2 = 0;
        if (productTemplate.second.Type == 1)
        {
            // if boost set boosttype to Level 60 boost (6) .. i didn't made a boosttype row in db yet
            product.Unk1 = 6;
            product.Unk1 = 4;
        }
        product.Unk3 = 0;
        product.TransmogSetID = productTemplate.second.TransmogSetID;
        product.Unk8 = 0;
        product.Unk9 = 0;
        product.NameSize = productTemplate.second.Title.size();
        product.UnkBits = 0;
        product.AlreadyOwned = productTemplate.second.AlreadyOwned;
        product.Items = {};  // not used in SL and DF do we need ?!


            auto productDisplay = WriteDisplayInfo(productTemplate.second.Entry, productTemplate.second.TitleColorIndex);
            if (std::get<0>(productDisplay))
            {
                product.Display.emplace();
                product.Display = std::get<1>(productDisplay);
            }
        response.Products.push_back(product);
    }

    _session->SendPacket(response.Write());
}

std::tuple<bool, WorldPackets::BattlePay::DisplayInfo> BattlepayManager::WriteDisplayInfo(uint32 productTemplateEntry, int8 titleColorIndex /* = 0 */)
{
    auto info = WorldPackets::BattlePay::DisplayInfo();

    BattlePayData::ProductTemplate const* productTemplate = sBattlePayDataStore->GetProductTemplate(productTemplateEntry);
    if (!productTemplate)
        return std::make_tuple(false, info);

    auto qualityColor = [](int8 _titleColorIndex) -> std::string
        {
            switch (_titleColorIndex)
            {
            case 0:
                return "|cffffffff";
            case 1:
                return "|cff1eff00";
            case 2:
                return "|cff0070dd";
            case 3:
                return "|cffa335ee";
            case 4:
                return "|cffff8000";
            case 5:
                return "|cffe5cc80";
            case 6:
                return "|cffe5cc80";
            default:
                return "|cffffffff";
            }
        };

    if (productTemplate->IconFileDataID == 0 && productTemplate->ItemID)
    {
        if (ItemModifiedAppearanceEntry const* itemModifiedAppearance = sDB2Manager.GetItemModifiedAppearance(productTemplate->ItemID, 0))
            if (ItemAppearanceEntry const* itemAppearance = sDB2Manager.GetItemAppearanceEntry(itemModifiedAppearance->ItemAppearanceID))
                info.IconFileDataID = itemAppearance->DefaultIconFileDataID;
    }
    else
        info.IconFileDataID = productTemplate->IconFileDataID;

    info.UIModelSceneID = productTemplate->UIModelSceneID ? productTemplate->UIModelSceneID : 10;

    if (!productTemplate->Title.size() && productTemplate->ItemID)
    {
        if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(productTemplate->ItemID))
            info.Title = qualityColor(titleColorIndex) + itemTemplate->GetName(LOCALE_enUS);
    }
    else
        info.Title = qualityColor(titleColorIndex) + productTemplate->Title;

    info.Title2 = productTemplate->Title2;
    info.Description = productTemplate->Description;
    info.Description2 = productTemplate->Description2;
    info.Description3 = productTemplate->Description3;
    info.Description4 = productTemplate->Description4;
    info.Description5 = productTemplate->Description5;
    info.IconBorder = productTemplate->IconBorder;
    info.Unk1 = false;
    info.Unk2 = false;
    info.UiTextureAtlasMemberID = productTemplate->UiTextureAtlasMemberID;
    info.CardType = productTemplate->CardType;

    // Sending visuals for the product (support multiple)
    if (productTemplate->PreviewUIModelSceneIDs.size())
    {
        for (int8 i = 0; i < productTemplate->PreviewUIModelSceneIDs.size(); i++)
        {
            WorldPackets::BattlePay::Visual visual;

            visual.Name = productTemplate->PreviewTitles.size() ? productTemplate->PreviewTitles[i] : "";
            visual.CreatureDisplayID = productTemplate->PreviewCreatureDisplayIDs.size() ? productTemplate->PreviewCreatureDisplayIDs[i] : 0;
            visual.PreviewUIModelSceneID = productTemplate->PreviewUIModelSceneIDs.size() ? productTemplate->PreviewUIModelSceneIDs[i] : 0;
            visual.TransmogSetID = productTemplate->PreviewTransmogSets.size() ? productTemplate->PreviewTransmogSets[i] : productTemplate->TransmogSetID;

            if (visual.Name.size() || visual.CreatureDisplayID || visual.PreviewUIModelSceneID || visual.TransmogSetID)
                info.Visuals.push_back(visual);
        }
    }

    // Automatic db2 data collecting of items if no parsed data
    if (!info.Visuals.size() && productTemplate->ItemID)
    {
        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(productTemplate->ItemID);
        if (!itemTemplate)
            return std::make_tuple(false, info);

        WorldPackets::BattlePay::Visual visual;
        visual.CreatureDisplayID = 0;
        visual.Name = "";
        visual.PreviewUIModelSceneID = 0;
        visual.TransmogSetID = 0;
        if (!productTemplate->Title.size())
            visual.Name = itemTemplate->GetName(LOCALE_enUS);
        else
            visual.Name = productTemplate->Title;

        for (ItemEffectEntry const* itemEffect : itemTemplate->Effects)
        {
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itemEffect->SpellID, DIFFICULTY_NONE))
            {
                for (auto& spellEffect : spellInfo->GetEffects())
                {
                    if (spellEffect.ApplyAuraName == SPELL_AURA_MOUNTED)
                    {
                        if (MountEntry const* mountEntry = sDB2Manager.GetMount(itemEffect->SpellID))
                        {
                            DB2Manager::MountXDisplayContainer const* mountDisplays = sDB2Manager.GetMountDisplays(mountEntry->ID);
                            DB2Manager::MountXDisplayContainer usableDisplays;
                            std::copy(mountDisplays->begin(), mountDisplays->end(), std::back_inserter(usableDisplays));
                            uint32 displayId = Trinity::Containers::SelectRandomContainerElement(usableDisplays)->CreatureDisplayInfoID;

                            visual.CreatureDisplayID = displayId ? displayId : 0;
                            visual.PreviewUIModelSceneID = 300; // mostly 300 for mounts
                            info.UIModelSceneID = 267; // mostly 267 for mounts
                        }
                    }
                    if (spellEffect.Effect == SPELL_EFFECT_SUMMON)
                    {
                        if (BattlePetSpeciesEntry const* bpetEntry = sDB2Manager.GetSpeciesBySpell(itemEffect->SpellID))
                        {
                            if (CreatureTemplate const* creatureTemplate = sObjectMgr->GetCreatureTemplate(bpetEntry->CreatureID))
                            {
                                visual.CreatureDisplayID = creatureTemplate->Models[0].CreatureDisplayID;
                                visual.PreviewUIModelSceneID = 10; // mostly 10 for pets
                                info.UIModelSceneID = 10; // mostly 10 for pets
                            }
                        }
                    }
                    if (spellEffect.Effect == SPELL_EFFECT_LEARN_TRANSMOG_SET)
                    {
                        visual.TransmogSetID = spellInfo->GetEffect(EFFECT_0).MiscValue;
                        visual.PreviewUIModelSceneID = 293; // mostly 293 for tmogs
                        info.UIModelSceneID = 420; // mostly 420 for tmogs
                    }
                }
            }
        }

        if (visual.CreatureDisplayID || visual.TransmogSetID)
            info.Visuals.push_back(visual);
    }

    return std::make_tuple(true, info);
}

void BattlepayManager::SendAccountCredits()
{
    if (!_session)
        return;

    auto player = _session->GetPlayer();
    if (!player)
        return;

    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_BATTLE_PAY_ACCOUNT_CREDITS);
    stmt->setUInt32(0, _session->GetAccountId());
    PreparedQueryResult result = LoginDatabase.Query(stmt);


    uint64 balance = 0;
    if (result)
        balance = result->Fetch()[0].GetUInt32();

    SendBattlePayMessage(2, "");
}

void BattlepayManager::SendDistributionUpdate(uint32 purchaseID, uint8 distributionStatus)
{
    WorldPackets::BattlePay::DistributionUpdate distributionUpdate;

    // First we update in DB
    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_BATTLE_PAY_PURCHASE);
    stmt->setInt8(0, distributionStatus);
    stmt->setUInt32(1, purchaseID);
    LoginDatabase.Execute(stmt);

    // Then we get data from DB
    for (auto purchase : GetPurchases(_session->GetAccountId()))
    {
        BattlePayData::ProductTemplate const* productTemplate = sBattlePayDataStore->GetProductTemplate(purchase.productID);
        if (productTemplate && purchase.PurchaseID == purchaseID)
        {
            WorldPackets::BattlePay::DistributionObject distObj;
            distObj.DistributionID = purchase.PurchaseID; // GenerateNewDistributionId();
            distObj.PurchaseID = purchase.PurchaseID;
            distObj.TargetPlayer = purchase.characterGuid;
            distObj.Status = purchase.PurchaseStatus;
            distObj.ProductID = purchase.productID;
            distObj.TargetVirtualRealm = 0;
            distObj.TargetNativeRealm = 0;
            distObj.Revoked = false;

            // BATTLEPAY PRODUCTS
            WorldPackets::BattlePay::Product product;
            product.ProductId = productTemplate->Entry;
            product.Type = productTemplate->Type;
            product.ItemID = productTemplate->ItemID;
            product.ItemCount = productTemplate->ItemCount;
            product.MountSpellID = productTemplate->MountSpellID;
            product.BattlePetSpeciesCreatureID = productTemplate->BattlePetSpeciesCreatureID;
            product.Unk1 = 6; // BoostType: 6 == Level 60 Boost
            product.Unk2 = 4;
            if (productTemplate->Title == "Level 50 Boost")
            {
                product.Unk1 = 4; // BoostType: 5 == Level 50 Boost
                product.Unk2 = 4;
            }
            product.Unk3 = 0;
            product.TransmogSetID = productTemplate->TransmogSetID;
            product.Unk8 = 0;
            product.Unk9 = 0;
            product.NameSize = productTemplate->Title.size();
            product.AlreadyOwned = productTemplate->AlreadyOwned;
            product.UnkBits = 0;

            auto display = WriteDisplayInfo(productTemplate->Entry);
            if (std::get<0>(display))
            {
                product.Display.emplace();
                product.Display = std::get<1>(display);
            }

            distObj.Products.emplace();
            distObj.Products = product;

            distributionUpdate.DistributionObjects = distObj;
        }
    }

    _session->SendPacket(distributionUpdate.Write());
}

void BattlepayManager::AssignDistributionToCharacter(uint32 ProductID, uint64 DistributionID, ObjectGuid CharGUID, int32 SpecID)
{
    SendDistributionUpdate(DistributionID, Battlepay::DistributionStatus::BATTLE_PAY_DIST_STATUS_ADD_TO_PROCESS);

    uint32 level = 0;
    for (auto purchase : GetPurchases(_session->GetAccountId()))
    {
        BattlePayData::ProductTemplate const* productTemplate = sBattlePayDataStore->GetProductTemplate(purchase.productID);
        if (productTemplate && purchase.PurchaseID == DistributionID)
        {
            if (productTemplate->Title == "Level 60 Boost")
                level = 60;

            if (productTemplate->Title == "Level 50 Boost")
                level = 50;
        }
    }

    sCharacterService->UpgradeCharacter(_session, CharGUID, SpecID, level);

    SendDistributionUpdate(DistributionID, Battlepay::DistributionStatus::BATTLE_PAY_DIST_STATUS_PROCESS_COMPLETE);

    WorldPackets::BattlePay::BattlePayStartDistributionAssignToTargetResponse assignResponse;
    assignResponse.DistributionID = DistributionID;
    assignResponse.unkint1 = ProductID;
    assignResponse.unkint2 = SpecID;
    _session->SendPacket(assignResponse.Write());

    SendDistributionUpdate(DistributionID, Battlepay::DistributionStatus::BATTLE_PAY_DIST_STATUS_FINISHED);
}

void BattlepayManager::SendDistributionListResponse()
{
    // @TODO add to m_events

    auto& trans = _actualTransaction;

    WorldPackets::BattlePay::DistributionListResponse packet;
    packet.Result = Battlepay::Error::Ok;

    for (auto purchase : GetPurchases(_session->GetAccountId()))
    {
        BattlePayData::ProductTemplate const* productTemplate = sBattlePayDataStore->GetProductTemplate(purchase.productID);
        if (productTemplate && productTemplate->Type == 1 /* CHAR BOOST */
            && purchase.PurchaseStatus == Battlepay::DistributionStatus::BATTLE_PAY_DIST_STATUS_AVAILABLE)
        {
            WorldPackets::BattlePay::DistributionObject distObj;
            distObj.DistributionID = purchase.PurchaseID; // GenerateNewDistributionId();
            distObj.PurchaseID = purchase.PurchaseID;
            distObj.TargetPlayer = purchase.characterGuid;
            distObj.Status = purchase.PurchaseStatus;
            distObj.ProductID = purchase.productID;
            distObj.TargetVirtualRealm = 0;
            distObj.TargetNativeRealm = 0;
            distObj.Revoked = false;

            // BATTLEPAY PRODUCTS
            WorldPackets::BattlePay::Product product;
            product.ProductId = productTemplate->Entry;
            product.Type = productTemplate->Type;
            product.ItemID = productTemplate->ItemID;
            product.ItemCount = productTemplate->ItemCount;
            product.MountSpellID = productTemplate->MountSpellID;
            product.BattlePetSpeciesCreatureID = productTemplate->BattlePetSpeciesCreatureID;
            product.Unk1 = 6; // BoostType: 6 == Level 60 Boost
            product.Unk2 = 4;
            if (productTemplate->Title == "Level 50 Boost")
            {
                product.Unk1 = 4; // BoostType: 4 == Level 50 Boost
                product.Unk2 = 596;
            }
            product.Unk3 = 0;
            product.TransmogSetID = productTemplate->TransmogSetID;
            product.Unk8 = 0;
            product.Unk9 = 0;
            product.NameSize = productTemplate->Title.size();
            product.AlreadyOwned = productTemplate->AlreadyOwned;
            product.UnkBits = 0;

            auto display = WriteDisplayInfo(productTemplate->Entry);
            if (std::get<0>(display))
            {
                product.Display.emplace();
                product.Display = std::get<1>(display);
            }

            distObj.Products.emplace();
            distObj.Products = product;

            packet.DistributionObjects.emplace_back(distObj);
        }
    }

    _session->SendPacket(packet.Write());
}

void BattlepayManager::SendBattlePayMessage(uint32 bpaymessageID, std::string name, uint32 value) const
{
    std::ostringstream msg;
    if (bpaymessageID == 1)
        msg << "The purchase '" << name << "' was successful!";
    if (bpaymessageID == 2)
        msg << "Remaining credits: " << GetBattlePayCredits() / 10000 << " .";

    if (bpaymessageID == 10)
        msg << "You cannot purchase '" << name << "' . Contact a game master to find out more.";
    if (bpaymessageID == 11)
        msg << "Your bags are too full to add : " << name << " .";
    if (bpaymessageID == 12)
        msg << "You have already purchased : " << name << " .";

    if (bpaymessageID == 20)
        msg << "The battle pay credits have been updated for the character '" << name << "' ! Available credits:" << value << " .";
    if (bpaymessageID == 21)
        msg << "You must enter an amount !";
    if (bpaymessageID == 3)
        msg << "You have now '" << value << "' credits.";

    ChatHandler(_session).SendSysMessage(msg.str().c_str());
}

uint32 BattlepayManager::GetBattlePayCredits() const
{
    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_BATTLE_PAY_ACCOUNT_CREDITS);

    stmt->setUInt32(0, _session->GetBattlenetAccountId());

    PreparedQueryResult result_don = LoginDatabase.Query(stmt);

    if (!result_don)
        return 0;

    Field* fields = result_don->Fetch();
    uint32 credits = fields[0].GetUInt32();

    return credits * 10000; // currency precision .. in retail it like gold and copper .. 10 usd is 100000 battlepay credit
}

bool BattlepayManager::HasBattlePayCredits(uint32 count) const
{
    if (GetBattlePayCredits() >= count)
        return true;
    return false;
}

bool BattlepayManager::UpdateBattlePayCredits(uint64 price) const
{
    //TC_LOG_INFO("server.BattlePay", "UpdateBattlePayCredits: GetBattlePayCredits(): %u - price: %lu", GetBattlePayCredits(), price);
    uint64 calcCredit = (GetBattlePayCredits() - price) / 10000;
    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_BATTLE_PAY_ACCOUNT_CREDITS);
    stmt->setUInt64(0, calcCredit);
    stmt->setUInt32(1, _session->GetBattlenetAccountId());
    LoginDatabase.Execute(stmt);

    return true;
}

bool BattlepayManager::ModifyBattlePayCredits(uint64 credits) const
{
    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_BATTLE_PAY_ACCOUNT_CREDITS);
    stmt->setUInt64(0, credits);
    stmt->setUInt32(1, _session->GetBattlenetAccountId());
    LoginDatabase.Execute(stmt);
    SendBattlePayMessage(3, "", credits);

    return true;
}

void BattlepayManager::SendBattlePayBattlePetDelivered(ObjectGuid petguid, uint32 creatureID) const
{
    WorldPackets::BattlePay::BattlePayBattlePetDelivered response;
    response.DisplayID = creatureID;
    response.BattlePetGuid = petguid;
    _session->SendPacket(response.Write());
}

void BattlepayManager::AddBattlePetFromBpayShop(uint32 battlePetCreatureID) const
{
    Player* player = _session->GetPlayer();
    if (!player)
        return;

    player->AddBattlePetByCreatureId(battlePetCreatureID, true, true);

    //it gives back false information need to get the pet guid from the add pet method somehow
    SendBattlePayBattlePetDelivered(ObjectGuid::Create<HighGuid::BattlePet>(sObjectMgr->GetGenerator<HighGuid::BattlePet>().Generate()), battlePetCreatureID);
}
