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

#include "Bag.h"
#include "BattlePayPackets.h"
#include "BattlePayMgr.h"
#include "BattlePayData.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "Random.h"
#include <sstream>

auto GetBagsFreeSlots = [](Player* player) -> uint32
{
    uint32 freeBagSlots = 0;
    for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
        if (auto bag = player->GetBagByPos(i))
            freeBagSlots += bag->GetFreeSlots();

    uint8 inventoryEnd = INVENTORY_SLOT_ITEM_START + player->GetInventorySlotCount();
    for (uint8 i = INVENTORY_SLOT_ITEM_START; i < inventoryEnd; i++)
        if (!player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            ++freeBagSlots;

    return freeBagSlots;
};

void WorldSession::SendStartPurchaseResponse(WorldSession* session, Battlepay::Purchase const& purchase, Battlepay::Error const& result)
{
    WorldPackets::BattlePay::StartPurchaseResponse response;
    response.PurchaseID = purchase.PurchaseID;
    response.ClientToken = purchase.ClientToken;
    response.PurchaseResult = result;
    session->SendPacket(response.Write());
};

void WorldSession::SendPurchaseUpdate(WorldSession* session, Battlepay::Purchase const& purchase, uint32 result)
{
    WorldPackets::BattlePay::PurchaseUpdate packet;
    WorldPackets::BattlePay::Purchase data;
    data.PurchaseID = purchase.PurchaseID;
    data.UnkLong = 0;
    data.UnkLong2 = 0;
    data.Status = purchase.Status;
    data.ResultCode = result;
    data.ProductID = purchase.ProductID;
    data.UnkInt = purchase.ServerToken;
    data.WalletName = session->GetBattlePayMgr()->GetDefaultWalletName();
    packet.Purchases.emplace_back(data);
    session->SendPacket(packet.Write());
};

void WorldSession::HandleGetPurchaseListQuery(WorldPackets::BattlePay::GetPurchaseListQuery& /*packet*/)
{
    WorldPackets::BattlePay::PurchaseListResponse packet; // @TODO
    SendPacket(packet.Write());
}

void WorldSession::HandleUpdateVasPurchaseStates(WorldPackets::BattlePay::UpdateVasPurchaseStates& /*packet*/)
{
    WorldPackets::BattlePay::EnumVasPurchaseStatesResponse response;
    response.Result = 0;
    SendPacket(response.Write());
}

void WorldSession::HandleBattlePayDistributionAssign(WorldPackets::BattlePay::DistributionAssignToTarget& packet)
{
    if (!GetBattlePayMgr()->IsAvailable())
        return;

    GetBattlePayMgr()->AssignDistributionToCharacter(packet.ProductID, packet.DistributionID, packet.CharGUID,  packet.SpecID);
}

void WorldSession::HandleGetProductList(WorldPackets::BattlePay::GetProductList& /*packet*/)
{
    if (!GetBattlePayMgr()->IsAvailable())
        return;

    GetBattlePayMgr()->SendProductList();
    GetBattlePayMgr()->SendAccountCredits();
}

void WorldSession::SendMakePurchase(ObjectGuid targetCharacter, uint32 clientToken, uint32 productID, WorldSession* session)
{
    if (!session || !session->GetBattlePayMgr()->IsAvailable())
        return;

    BattlePayData::ProductTemplate const* productTemplate = sBattlePayDataStore->GetProductTemplate(productID);
    if (!productTemplate)
        return;

    auto mgr = session->GetBattlePayMgr();
    auto accountID = session->GetAccountId();

    Battlepay::Purchase purchase;
    purchase.ProductID = productID;
    purchase.ClientToken = clientToken;
    purchase.TargetCharacter = targetCharacter;
    purchase.Status = Battlepay::UpdateStatus::Loading;
    purchase.DistributionId = productID; // mgr->GenerateNewDistributionId();
    purchase.CurrentPrice = productTemplate->CurrentPrice;

    mgr->RegisterStartPurchase(purchase);

    auto accountCredits = GetBattlePayMgr()->GetBattlePayCredits();
    auto purchaseData = mgr->GetPurchase();

    if (accountCredits < uint32(productTemplate->CurrentPrice))
    {
        SendStartPurchaseResponse(session, *purchaseData, Battlepay::Error::InsufficientBalance);
        return;
    }
    purchaseData->PurchaseID = mgr->GenerateNewPurchaseID();
    purchaseData->ServerToken = urand(0, 0xFFFFFFF);

    SendStartPurchaseResponse(session, *purchaseData, Battlepay::Error::Ok);
    SendPurchaseUpdate(session, *purchaseData, Battlepay::Error::Ok);

    WorldPackets::BattlePay::ConfirmPurchase confirmPurchase;
    confirmPurchase.PurchaseID = purchaseData->PurchaseID;
    confirmPurchase.ServerToken = purchaseData->ServerToken;
    session->SendPacket(confirmPurchase.Write());
};

void WorldSession::HandleBattlePayStartPurchase(WorldPackets::BattlePay::StartPurchase& packet)
{
    SendMakePurchase(packet.TargetCharacter, packet.ClientToken, packet.ProductID, this);
}

void WorldSession::HandleBattlePayConfirmPurchase(WorldPackets::BattlePay::ConfirmPurchaseResponse& packet)
{
    if (!GetBattlePayMgr()->IsAvailable())
        return;

    auto purchase = GetBattlePayMgr()->GetPurchase();

    BattlePayData::ProductTemplate const* productTemplate = sBattlePayDataStore->GetProductTemplate(purchase->ProductID);
    if (!productTemplate)
    {
        SendPurchaseUpdate(this, *purchase, Battlepay::Error::PurchaseDenied);
        return;
    }

    if (purchase->Lock)
    {
        SendPurchaseUpdate(this, *purchase, Battlepay::Error::PurchaseDenied);
        return;
    }

    auto accountBalance = GetBattlePayMgr()->GetBattlePayCredits();
    if (accountBalance < static_cast<uint64>(purchase->CurrentPrice))
    {
        SendPurchaseUpdate(this, *purchase, Battlepay::Error::PurchaseDenied);
        return;
    }
    purchase->Lock = true;
    purchase->Status = Battlepay::UpdateStatus::Finish;

    SendPurchaseUpdate(this, *purchase, Battlepay::Error::Other);
    GetBattlePayMgr()->SavePurchase(purchase);
    GetBattlePayMgr()->ProcessDelivery(purchase);
    GetBattlePayMgr()->UpdateBattlePayCredits(purchase->CurrentPrice);
    GetBattlePayMgr()->SendBattlePayMessage(1, productTemplate->Title);
    GetBattlePayMgr()->SendProductList();
    GetBattlePayMgr()->SendAccountCredits();
}

void WorldSession::HandleBattlePayAckFailedResponse(WorldPackets::BattlePay::BattlePayAckFailedResponse& /*packet*/)
{
}

void WorldSession::HandleBattlePayRequestPriceInfo(WorldPackets::BattlePay::BattlePayRequestPriceInfo& packet)
{
}

void WorldSession::SendDisplayPromo(int32 entry /*= 0*/)
{
    SendPacket(WorldPackets::BattlePay::DisplayPromotion(entry).Write());
}

void WorldSession::SendSyncWowEntitlements()
{
    WorldPackets::BattlePay::SyncWowEntitlements packet;
    SendPacket(packet.Write());
}

void WorldSession::HandleBattlePayStartVasPurchase(WorldPackets::BattlePay::StartVasPurchase& packet)
{
    TC_LOG_INFO("server.debug", "startvaspurchase: packet.VasPurchases.PlayerGuid: {}", packet.VasPurchases.PlayerGuid.GetCounter());
}
