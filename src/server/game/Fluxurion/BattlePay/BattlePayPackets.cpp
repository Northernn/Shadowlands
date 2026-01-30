/*
* Copyright 2023 Fluxurion
*
*This program is free software; you can redistribute itand /or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
*This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
*FITNESS FOR A PARTICULAR PURPOSE.See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program.If not, see < http://www.gnu.org/licenses/>.
*/

#include "BattlePayPackets.h"

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::DisplayInfo const& info)
{
    data.WriteBit(info.IconFileDataID.has_value());
    data.WriteBit(info.UIModelSceneID.has_value());
    data.WriteBits(info.Title.size(), 10);
    data.WriteBits(info.Title2.size(), 10);
    data.WriteBits(info.Description.size(), 13);
    data.WriteBits(info.Description2.size(), 13);
    data.WriteBits(info.Description3.size(), 13);
    data.WriteBit(info.IconBorder.has_value());
    data.WriteBit(info.Unk1.has_value());
    data.WriteBit(info.Unk2.has_value());
    data.WriteBit(info.UiTextureAtlasMemberID.has_value());
    data.WriteBits(info.Description4.size(), 13);
    data.WriteBits(info.Description5.size(), 13);
    data.FlushBits();

    data << (uint32)info.Visuals.size();
    data << info.CardType;
    data << info.Unk3;
    data << info.Unk4;
    if (info.IconFileDataID.has_value())
        data << *info.IconFileDataID;
    if (info.UIModelSceneID.has_value())
        data << *info.UIModelSceneID;
    data.WriteString(info.Title);
    data.WriteString(info.Title2);
    data.WriteString(info.Description);
    data.WriteString(info.Description2);
    data.WriteString(info.Description3);
    if (info.IconBorder.has_value())
        data << *info.IconBorder;
    if (info.Unk1.has_value())
        data << *info.Unk1;
    if (info.Unk2.has_value())
        data << *info.Unk2;
    if (info.UiTextureAtlasMemberID.has_value())
        data << *info.UiTextureAtlasMemberID;
    data.WriteString(info.Description4);
    data.WriteString(info.Description5);

    for (auto const& visual : info.Visuals)
    {
        data.WriteBits(visual.Name.size(), 10);
        data.FlushBits();
        data << visual.CreatureDisplayID;
        data << visual.PreviewUIModelSceneID;
        data << visual.TransmogSetID;
        data.WriteString(visual.Name);
    }

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::ProductInfo const& p)
{
    data << p.ProductID;
    data << p.NormalPrice;
    data << p.CurrentPrice;
    data << (uint32)p.BundleProductIDs.size();
    data << p.Unk1;
    data << p.Unk2;
    data << (uint32)p.UnkInts.size();
    data << p.Flags;
    for (uint32 id : p.BundleProductIDs)
        data << id;
    for (uint32 id : p.UnkInts)
        data << id;
    data.WriteBits(p.ChoiceType, 7);
    bool wrote = data.WriteBit(p.Display.has_value());
    data.FlushBits();
    if (wrote)
        data << *p.Display;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::ProductItem const& p)
{
    data << p.ID;
    data << p.UnkByte;
    data << p.ItemID;
    data << p.Quantity;
    data << p.UnkInt1;
    data << p.UnkInt2;
    data.WriteBit(p.IsPet);
    data.WriteBit(p.PetResult.has_value());
    data.WriteBit(p.Display.has_value());
    if (p.PetResult.has_value())
        data.WriteBits(*p.PetResult, 4);
    data.FlushBits();
    if (p.Display.has_value())
        data << *p.Display;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::Product const& p)
{
    data << p.ProductId;
    data << p.Type;
    data << p.ItemID;
    data << p.ItemCount;
    data << p.MountSpellID;
    data << p.BattlePetSpeciesCreatureID;
    data << p.Unk1;
    data << p.Unk2;
    data << p.Unk3;
    data << p.TransmogSetID;
    data << p.Unk8;
    data << p.Unk9;
    data.WriteBits(p.NameSize.size(), 8);
    data.WriteBit(p.AlreadyOwned);
    data.WriteBit(p.UnkBits.has_value());
    data.WriteBits(p.Items.size(), 7);
    data.WriteBit(p.Display.has_value());
    if (p.UnkBits.has_value())
        data.WriteBits(*p.UnkBits, 4);

    data.FlushBits();

    for (auto const& productItem : p.Items)
    {
        data << productItem.ID;
        data << productItem.UnkByte;
        data << productItem.ItemID;
        data << productItem.Quantity;
        data << productItem.UnkInt1;
        data << productItem.UnkInt2;

        data.WriteBit(productItem.IsPet);
        data.WriteBit(productItem.PetResult.has_value());
        data.WriteBit(productItem.Display.has_value());

        if (productItem.PetResult)
            data.WriteBits(*productItem.PetResult, 4);

        data.FlushBits();

        if (productItem.Display)
            data << *productItem.Display;
    }

    data.WriteString(p.NameSize);

    if (p.Display.has_value())
        data << *p.Display;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::Group const& group)
{
    data << group.GroupId;
    data << group.IconFileDataID;
    data << group.DisplayType;
    data << group.Ordering;
    data << group.Unk;
    data << group.MainGroupID;
    data.WriteBits(group.Name.size(), 8);
    data.WriteBits(group.Description.size() + 1, 24);
    data.FlushBits();
    data.WriteString(group.Name);
    if (!group.Description.empty())
        data << group.Description;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::Shop const& entry)
{
    data << entry.EntryID;
    data << entry.GroupID;
    data << entry.ProductID;
    data << entry.Ordering;
    data << entry.VasServiceType;
    data << entry.StoreDeliveryType;
    data.WriteBit(entry.Display.has_value());
    data.FlushBits();
    if (entry.Display.has_value())
    {
        data.FlushBits();
        data << *entry.Display;
    }
    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::DistributionObject const& object)
{
    data << object.DistributionID;
    data << object.Status;
    data << object.ProductID;
    data << object.TargetPlayer;
    data << object.UnkGuid;
    data << object.TargetVirtualRealm;
    data << object.TargetNativeRealm;
    data << object.PurchaseID;
    data << object.Unk55AC;
    data.WriteBit(object.Products.has_value());
    data.WriteBit(object.Revoked);
    data.FlushBits();

    if (object.Products)
        data << *object.Products;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::Purchase const& purchase)
{
    data << purchase.PurchaseID;
    data << purchase.Status;
    data << purchase.ResultCode;
    data << purchase.ProductID;
    data << purchase.UnkLong;
    data << purchase.UnkLong2;
    data << purchase.UnkInt;

    data.WriteBits(purchase.WalletName.length(), 8);
    data.WriteString(purchase.WalletName);

    return data;
}

WorldPacket const* WorldPackets::BattlePay::PurchaseListResponse::Write()
{
    _worldPacket << Result;
    _worldPacket << static_cast<uint32>(Purchases.size());
    for (auto const& purchaseData : Purchases)
        _worldPacket << purchaseData;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::DistributionListResponse::Write()
{
    _worldPacket << Result;
    _worldPacket.WriteBits(DistributionObjects.size(), 11);
    for (BattlePay::DistributionObject const& objectData : DistributionObjects)
        _worldPacket << objectData;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::DistributionUpdate::Write()
{
    _worldPacket << DistributionObjects;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::ProductListResponse::Write()
{
    _worldPacket << Result;
    _worldPacket << CurrencyID;
    _worldPacket << (uint32)ProductInfos.size();
    _worldPacket << (uint32)Products.size();
    _worldPacket << (uint32)ProductGroups.size();
    _worldPacket << (uint32)Shops.size();

    for (auto const& p : ProductInfos)
        _worldPacket << p;
    for (auto const& p : Products)
        _worldPacket << p;
    for (auto const& p : ProductGroups)
        _worldPacket << p;
    for (auto const& p : Shops)
        _worldPacket << p;

    return &_worldPacket;
}

void WorldPackets::BattlePay::StartPurchase::Read()
{
    _worldPacket >> ClientToken;
    _worldPacket >> ProductID;
    _worldPacket >> TargetCharacter;
}

/*void WorldPackets::BattlePay::PurchaseProduct::Read()
{
    _worldPacket >> ClientToken;
    _worldPacket >> ProductID;
    _worldPacket >> TargetCharacter;

    uint32 strlen1 = _worldPacket.ReadBits(6);
    uint32 strlen2 = _worldPacket.ReadBits(12);
    WowSytem = _worldPacket.ReadString(strlen1);
    PublicKey = _worldPacket.ReadString(strlen2);
}*/

WorldPacket const* WorldPackets::BattlePay::SyncWowEntitlements::Write()
{
    TC_LOG_INFO("server.BattlePay", "SyncWowEntitlements");
    _worldPacket << uint32(purchaseCount.size());
    _worldPacket << uint32(Products.size());

    for (auto purchases : purchaseCount)
    {
        _worldPacket << uint32(0); // productID ?
        _worldPacket << uint32(0); // flags?
        _worldPacket << uint32(0); // idem to flags?
        _worldPacket << uint32(0); // always 0
        _worldPacket.WriteBits(0, 7); // always 0
        _worldPacket.WriteBit(false); // always false
    }

    for (auto const& product : Products)
    {
        _worldPacket << product.ProductId;
        _worldPacket << product.Type;
        _worldPacket << product.ItemID;
        _worldPacket << product.ItemCount;
        _worldPacket << product.MountSpellID;
        _worldPacket << product.BattlePetSpeciesCreatureID;
        _worldPacket << uint32(0);
        _worldPacket << uint32(2);
        _worldPacket << uint32(0);
        _worldPacket << uint32(0);
        _worldPacket << uint32(0);
        _worldPacket << uint32(0);

        _worldPacket.WriteBits(product.NameSize.size(), 8);
        _worldPacket.WriteBit(product.UnkBits.has_value());
        _worldPacket.WriteBit(product.AlreadyOwned);
        _worldPacket.WriteBits(product.Items.size(), 7);
        _worldPacket.WriteBit(product.Display.has_value());
        _worldPacket.WriteBit(0); // unk

        if (product.UnkBits)
            _worldPacket.WriteBits(*product.UnkBits, 4);

        _worldPacket.FlushBits();

        for (auto productItem : product.Items)
        {
            _worldPacket << uint32(productItem.ID);
            _worldPacket << uint8(productItem.UnkByte);
            _worldPacket << uint32(productItem.ItemID);
            _worldPacket << uint32(productItem.Quantity);
            _worldPacket << uint32(productItem.UnkInt1);
            _worldPacket << uint32(productItem.UnkInt2);

            _worldPacket.WriteBit(productItem.IsPet);
            _worldPacket.WriteBit(productItem.PetResult.has_value());
            _worldPacket.WriteBit(productItem.Display.has_value());

            if (productItem.PetResult)
                _worldPacket.WriteBits(*productItem.PetResult, 4);

            _worldPacket.FlushBits();

            if (productItem.Display)
                _worldPacket << *productItem.Display;
        }

        _worldPacket.WriteString(product.NameSize);

        if (product.Display)
            _worldPacket << *product.Display;
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::StartPurchaseResponse::Write()
{
    _worldPacket << PurchaseID;
    _worldPacket << PurchaseResult;
    _worldPacket << ClientToken;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::BattlePayAckFailed::Write()
{
    _worldPacket << PurchaseID;
    _worldPacket << PurchaseResult;
    _worldPacket << ClientToken;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::PurchaseUpdate::Write()
{
    _worldPacket << static_cast<uint32>(Purchases.size());
    for (auto const& purchaseData : Purchases)
        _worldPacket << purchaseData;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::ConfirmPurchase::Write()
{
    _worldPacket << PurchaseID;
    _worldPacket << ServerToken;

    return &_worldPacket;
}

void WorldPackets::BattlePay::ConfirmPurchaseResponse::Read()
{
    ConfirmPurchase = _worldPacket.ReadBit();
    _worldPacket >> ServerToken;
    _worldPacket >> ClientCurrentPriceFixedPoint;
}

void WorldPackets::BattlePay::BattlePayAckFailedResponse::Read()
{
    _worldPacket >> ServerToken;
}

WorldPacket const* WorldPackets::BattlePay::DeliveryEnded::Write()
{
    _worldPacket << DistributionID;

    _worldPacket << static_cast<int32>(item.size());
    for (Item::ItemInstance const& itemData : item)
        _worldPacket << itemData;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::BattlePayDeliveryStarted::Write()
{
    _worldPacket << DistributionID;

    return &_worldPacket;
}

void WorldPackets::BattlePay::DistributionAssignToTarget::Read()
{
    _worldPacket >> ProductID;
    _worldPacket >> DistributionID;
    _worldPacket >> CharGUID;
    _worldPacket >> SpecID;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::VasPurchase const& purchase)
{
    data << purchase.PlayerGuid;
    data << purchase.UnkInt;
    data << purchase.UnkInt2;
    data << purchase.UnkLong;
    data.WriteBits(purchase.ItemIDs.size(), 2);
    data.FlushBits();

    for (auto const& itemID : purchase.ItemIDs)
        data << itemID;

    return data;
}

WorldPacket const* WorldPackets::BattlePay::CharacterUpgradeStarted::Write()
{
    _worldPacket << CharGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::CharacterUpgradeComplete::Write()
{
    _worldPacket << CharGUID;
    _worldPacket << LoadOutItemCount;
    for (uint32 item : LoadOut)
        _worldPacket << item;

    return &_worldPacket;
}


void WorldPackets::BattlePay::StartVasPurchase::Read()
{
    _worldPacket << UnkInt;
    _worldPacket << VasPurchases;
}

/*WorldPacket const* WorldPackets::BattlePay::CharacterClassTrialCreate::Write()
{
    _worldPacket << Result;
    return &_worldPacket;
}*/

/*void WorldPackets::BattlePay::BattlePayTrialBoostCharacter::Read()
{
    _worldPacket >> Character;
    _worldPacket >> SpecializationID;
}*/

/*WorldPacket const* WorldPackets::BattlePay::BattlePayVasPurchaseList::Write()
{
    _worldPacket.WriteBits(VasPurchase.size(), 6);
    _worldPacket.FlushBits();
    for (auto const& itr : VasPurchase)
        _worldPacket << itr;

    return &_worldPacket;
}*/

WorldPacket const* WorldPackets::BattlePay::EnumVasPurchaseStatesResponse::Write()
{
    _worldPacket.WriteBits(Result, 2);

    return &_worldPacket;
}

/*WorldPacket const* WorldPackets::BattlePay::PurchaseDetails::Write()
{
    _worldPacket << UnkInt;
    _worldPacket << VasPurchaseProgress;
    _worldPacket << UnkLong;

    _worldPacket.WriteBits(Key.length(), 6);
    _worldPacket.WriteBits(Key2.length(), 6);
    _worldPacket.WriteString(Key);
    _worldPacket.WriteString(Key2);

    return &_worldPacket;
}*/

/*WorldPacket const* WorldPackets::BattlePay::PurchaseUnk::Write()
{
    _worldPacket << UnkByte;
    _worldPacket << UnkInt;

    _worldPacket.WriteBits(Key.length(), 7);
    _worldPacket.WriteString(Key);

    return &_worldPacket;
}*/

WorldPacket const* WorldPackets::BattlePay::BattlePayBattlePetDelivered::Write()
{
    _worldPacket << DisplayID;
    _worldPacket << BattlePetGuid;

    return &_worldPacket;
}

/*void WorldPackets::BattlePay::PurchaseDetailsResponse::Read()
{
    _worldPacket >> UnkByte;
}*/

/*/void WorldPackets::BattlePay::PurchaseUnkResponse::Read()
{
    auto keyLen = _worldPacket.ReadBits(6);
    auto key2Len = _worldPacket.ReadBits(7);
    Key = _worldPacket.ReadString(keyLen);
    Key2 = _worldPacket.ReadString(key2Len);
}*/

WorldPacket const* WorldPackets::BattlePay::DisplayPromotion::Write()
{
    _worldPacket << PromotionID;

    return &_worldPacket;
}

/*WorldPacket const* WorldPackets::BattlePay::BattlepayUnk::Write()
{
    _worldPacket << UnkInt;

    return &_worldPacket;
}*/

WorldPacket const* WorldPackets::BattlePay::BattlePayStartDistributionAssignToTargetResponse::Write()
{
    _worldPacket << DistributionID;
    _worldPacket << unkint1;
    _worldPacket << unkint2;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::GetAccountCharacterListResult::Write()
{
    _worldPacket << Token;
    _worldPacket.WriteBits(accountCharacterListEntry.size(), 2);
    _worldPacket.FlushBits();

    _worldPacket << UnkBit;

    for (AccountCharacterListEntry character : accountCharacterListEntry)
    {
        _worldPacket << character.AccountId;
        _worldPacket << character.VirtualRealmAddress;
        _worldPacket << character.RealmName;
        _worldPacket << character.CharacterGuid;
        _worldPacket << character.Name;
        _worldPacket << character.Race;
        _worldPacket << character.Class;
        _worldPacket << character.Sex;
        _worldPacket << character.Level;
        _worldPacket << character.LastPlayedTime;
    }

    return &_worldPacket;
}
