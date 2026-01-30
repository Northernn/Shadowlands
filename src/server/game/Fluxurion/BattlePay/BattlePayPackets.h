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


#ifndef BattlePayPackets_h__
#define BattlePayPackets_h__

#include "Packet.h"
#include "ItemPackets.h"
#include "WorldSession.h"

namespace WorldPackets
{
    namespace BattlePay
    {
        struct Visual
        {
            std::string Name;
            uint32 CreatureDisplayID;
            uint32 PreviewUIModelSceneID;
            uint32 TransmogSetID;
        };

        struct DisplayInfo
        {
            Optional<uint32> IconFileDataID;
            Optional<uint32> UIModelSceneID;
            std::string Title;
            std::string Title2;
            std::string Description;
            std::string Description2;
            std::string Description3;
            std::string Description4;
            std::string Description5;
            Optional<uint32> IconBorder;
            Optional<uint32> Unk1;
            Optional<uint32> Unk2;
            Optional<uint32> UiTextureAtlasMemberID;
            uint32 CardType;
            uint32 Unk3;
            uint32 Unk4;

            std::vector<Visual> Visuals;
        };

        struct ProductInfo
        {
            uint32 ProductID;
            uint64 NormalPrice;
            uint64 CurrentPrice;
            std::vector<uint32> BundleProductIDs;
            uint32 Unk1;
            uint32 Unk2;
            std::vector<uint32> UnkInts;
            uint32 Flags;
            uint32 ChoiceType;
            Optional<DisplayInfo> Display;
        };

        struct ProductItem
        {
            uint32 ID;
            uint8 UnkByte;
            uint32 ItemID;
            uint32 Quantity;
            uint32 UnkInt1;
            uint32 UnkInt2;
            bool IsPet;
            Optional<uint32> PetResult;
            Optional<DisplayInfo> Display;
        };

        struct Product
        {
            uint32 ProductId;
            uint8 Type;
            uint32 ItemID;
            uint32 ItemCount;
            uint32 MountSpellID;
            uint32 BattlePetSpeciesCreatureID;
            uint32 Unk1;
            uint32 Unk2;
            uint32 Unk3;
            uint32 TransmogSetID;
            uint32 Unk8;
            uint32 Unk9;
            std::string NameSize;
            bool AlreadyOwned;
            Optional<uint32> UnkBits;
            std::vector<ProductItem> Items; // not used in SL and DF do we need ?!
            Optional<DisplayInfo> Display;
        };

        struct Group
        {
            uint32 GroupId;
            uint32 IconFileDataID;
            uint8 DisplayType;
            uint32 Ordering;
            uint32 Unk;
            uint32 MainGroupID;
            std::string Name;
            std::string Description;
        };

        struct Shop
        {
            uint32 EntryID;
            uint32 GroupID;
            uint32 ProductID;
            uint32 Ordering;
            uint32 VasServiceType;
            uint8 StoreDeliveryType;
            Optional<DisplayInfo> Display;
        };

        struct DistributionObject
        {
            uint64 DistributionID = 0;
            uint32 Status = 0;
            uint32 ProductID = 0;
            ObjectGuid TargetPlayer;
            ObjectGuid UnkGuid;
            uint32 TargetVirtualRealm = 0;
            uint32 TargetNativeRealm = 0;
            uint64 PurchaseID = 0;
            uint32 Unk55AC = 0;
            bool Revoked = false;
            Optional<Product> Products;
        };

        struct Purchase
        {
            uint64 PurchaseID = 0;
            uint64 UnkLong = 0;
            uint64 UnkLong2 = 0;
            uint32 Status = 0;
            uint32 ResultCode = 0;
            uint32 ProductID = 0;
            uint32 UnkInt = 0;
            std::string WalletName;
        };

        struct VasPurchase
        {
            std::vector<uint32> ItemIDs;
            ObjectGuid PlayerGuid;
            uint64 UnkLong = 0;
            uint32 UnkInt = 0;
            uint32 UnkInt2 = 0;
        };

        struct AccountCharacterListEntry
        {
            ObjectGuid AccountId;

            uint32 VirtualRealmAddress = 0;
            std::string RealmName;

            ObjectGuid CharacterGuid;
            std::string Name;
            uint8 Race;
            uint8 Class;
            uint8 Sex;
            uint8 Level;
            Timestamp<> LastPlayedTime;
        };

        class GetProductList final : public ClientPacket
        {
        public:
            GetProductList(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PAY_GET_PRODUCT_LIST, std::move(packet)) { }

            void Read() override { }
        };

        class GetPurchaseListQuery final : public ClientPacket
        {
        public:
            GetPurchaseListQuery(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PAY_GET_PURCHASE_LIST, std::move(packet)) { }

            void Read() override { }
        };

        class UpdateVasPurchaseStates final : public ClientPacket
        {
        public:
            UpdateVasPurchaseStates(WorldPacket&& packet) : ClientPacket(CMSG_UPDATE_VAS_PURCHASE_STATES, std::move(packet)) { }

            void Read() override { }
        };

        class PurchaseListResponse final : public ServerPacket
        {
        public:
            PurchaseListResponse() : ServerPacket(SMSG_BATTLE_PAY_GET_PURCHASE_LIST_RESPONSE, 8) { }

            WorldPacket const* Write() override;

            uint32 Result = 0;
            std::vector<Purchase> Purchases;
        };

        class SyncWowEntitlements final : public ServerPacket
        {
        public:
            SyncWowEntitlements() : ServerPacket(SMSG_SYNC_WOW_ENTITLEMENTS) { }

            WorldPacket const* Write() override;

            std::vector<uint32> purchaseCount;
            std::vector<uint32> productCount;
            std::vector<Product> Products;
        };

        class DistributionListResponse final : public ServerPacket
        {
        public:
            DistributionListResponse() : ServerPacket(SMSG_BATTLE_PAY_GET_DISTRIBUTION_LIST_RESPONSE, 8) { }

            WorldPacket const* Write() override;

            uint32 Result = 0;
            std::vector<DistributionObject> DistributionObjects;
        };

        class EnumVasPurchaseStatesResponse final : public ServerPacket
        {
        public:
            EnumVasPurchaseStatesResponse() : ServerPacket(SMSG_ENUM_VAS_PURCHASE_STATES_RESPONSE, 4) { }

            WorldPacket const* Write() override;

            uint8 Result = 0;
        };

        class DistributionUpdate final : public ServerPacket
        {
        public:
            DistributionUpdate() : ServerPacket(SMSG_BATTLE_PAY_DISTRIBUTION_UPDATE, 100) { }

            WorldPacket const* Write() override;

            DistributionObject DistributionObjects;
        };

        class ProductListResponse final : public ServerPacket
        {
        public:
            ProductListResponse() : ServerPacket(SMSG_BATTLE_PAY_GET_PRODUCT_LIST_RESPONSE, 4000) { }

            WorldPacket const* Write() override;

            uint32 Result = 0;
            uint32 CurrencyID = 0;
            std::vector<ProductInfo> ProductInfos;
            std::vector<Product> Products;
            std::vector<Group> ProductGroups;
            std::vector<Shop> Shops;
        };

        class StartPurchase final : public ClientPacket
        {
        public:
            StartPurchase(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PAY_START_PURCHASE, std::move(packet)) { }

            void Read() override;

            ObjectGuid TargetCharacter;
            uint32 ClientToken = 0;
            uint32 ProductID = 0;
        };

        class StartPurchaseResponse final : public ServerPacket
        {
        public:
            StartPurchaseResponse() : ServerPacket(SMSG_BATTLE_PAY_START_PURCHASE_RESPONSE, 16) { }

            WorldPacket const* Write() override;

            uint64 PurchaseID = 0;
            uint32 ClientToken = 0;
            uint32 PurchaseResult = 0;
        };

        /* NEED GET CMSG_BATTLE_PAY_PURCHASE_PRODUCT FOR RECENT PATCH (Need sniff an ingame shop buy)
        class PurchaseProduct final : public ClientPacket
        {
        public:
            PurchaseProduct(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PAY_PURCHASE_PRODUCT, std::move(packet)) { }

            void Read() override;

            ObjectGuid TargetCharacter;
            uint32 ClientToken = 0;
            uint32 ProductID = 0;
            std::string WowSytem;
            std::string PublicKey;
        };
        */

        class BattlePayAckFailed final : public ServerPacket
        {
        public:
            BattlePayAckFailed() : ServerPacket(SMSG_BATTLE_PAY_ACK_FAILED, 16) { }

            WorldPacket const* Write() override;

            uint64 PurchaseID = 0;
            uint32 ClientToken = 0;
            uint32 PurchaseResult = 0;
        };

        class PurchaseUpdate final : public ServerPacket
        {
        public:
            PurchaseUpdate() : ServerPacket(SMSG_BATTLE_PAY_PURCHASE_UPDATE, 4) { }

            WorldPacket const* Write() override;

            std::vector<Purchase> Purchases;
        };

        class ConfirmPurchase final : public ServerPacket
        {
        public:
            ConfirmPurchase() : ServerPacket(SMSG_BATTLE_PAY_CONFIRM_PURCHASE, 20) { }

            WorldPacket const* Write() override;

            uint64 PurchaseID = 0;
            uint32 ServerToken = 0;
        };

        class ConfirmPurchaseResponse final : public ClientPacket
        {
        public:
            ConfirmPurchaseResponse(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PAY_CONFIRM_PURCHASE_RESPONSE, std::move(packet)) { }

            void Read() override;

            uint64 ClientCurrentPriceFixedPoint = 0;
            uint32 ServerToken = 0;
            bool ConfirmPurchase = false;
        };

        class BattlePayAckFailedResponse final : public ClientPacket
        {
        public:
            BattlePayAckFailedResponse(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PAY_ACK_FAILED_RESPONSE, std::move(packet)) { }

            void Read() override;

            uint32 ServerToken = 0;
        };

        class DeliveryEnded final : public ServerPacket
        {
        public:
            DeliveryEnded() : ServerPacket(SMSG_BATTLE_PAY_DELIVERY_ENDED, 12) { }

            WorldPacket const* Write() override;

            std::vector<Item::ItemInstance> item;
            uint64 DistributionID = 0;
        };

        class BattlePayDeliveryStarted final : public ServerPacket
        {
        public:
            BattlePayDeliveryStarted() : ServerPacket(SMSG_BATTLE_PAY_DELIVERY_STARTED, 8) { }

            WorldPacket const* Write() override;

            uint64 DistributionID = 0;
        };

        class DistributionAssignToTarget final : public ClientPacket
        {
        public:
            DistributionAssignToTarget(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PAY_DISTRIBUTION_ASSIGN_TO_TARGET, std::move(packet)) { }

            void Read() override;

            uint32 ProductID = 0;
            uint64 DistributionID = 0;
            ObjectGuid CharGUID = ObjectGuid::Empty;
            int32 SpecID = 0;
        };

        class CharacterUpgradeStarted final : public ServerPacket
        {
        public:
            CharacterUpgradeStarted() : ServerPacket(SMSG_CHARACTER_UPGRADE_STARTED, 9) { }

            WorldPacket const* Write() override;

            ObjectGuid CharGUID;
        };

        class CharacterUpgradeComplete final : public ServerPacket
        {
        public:
            CharacterUpgradeComplete() : ServerPacket(SMSG_CHARACTER_UPGRADE_COMPLETE, 77) { }

            WorldPacket const* Write() override;

            ObjectGuid CharGUID;
            int32 LoadOutItemCount;
            std::vector<uint32> LoadOut;
        };

        class StartVasPurchase final : public ClientPacket
        {
        public:
            StartVasPurchase(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PAY_START_VAS_PURCHASE, std::move(packet)) { }

            void Read() override;

            VasPurchase VasPurchases;
            uint32 UnkInt = 0;
        };

        /* class CharacterClassTrialCreate final : public ServerPacket
        {
        public:
            CharacterClassTrialCreate() : ServerPacket(SMSG_CHARACTER_CLASS_TRIAL_CREATE, 4) { }

            WorldPacket const* Write() override;

            uint32 Result = 0;
        }; */

        /* class BattlePayQueryClassTrialResult final : public ClientPacket
        {
        public:
            BattlePayQueryClassTrialResult(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PAY_QUERY_CLASS_TRIAL_BOOST_RESULT, std::move(packet)) { }

            void Read() override { }
        }; */

        /* class BattlePayTrialBoostCharacter final : public ClientPacket
        {
        public:
            BattlePayTrialBoostCharacter(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PAY_TRIAL_BOOST_CHARACTER, std::move(packet)) { }

            void Read() override;

            ObjectGuid Character;
            uint32 SpecializationID = 0;
        }; */

        /* class BattlePayVasPurchaseList final : public ServerPacket
        {
        public:
            BattlePayVasPurchaseList() : ServerPacket(SMSG_BATTLE_PAY_VAS_PURCHASE_LIST, 4) { }

            WorldPacket const* Write() override;

            std::vector<VasPurchase> VasPurchase;
        }; */

        /* class PurchaseDetails final : public ServerPacket
        {
        public:
            PurchaseDetails() : ServerPacket(SMSG_BATTLE_PAY_PURCHASE_DETAILS, 20) { }

            WorldPacket const* Write() override;

            uint64 UnkLong = 0;
            uint32 UnkInt = 0;
            uint32 VasPurchaseProgress = 0;
            std::string Key;
            std::string Key2;
        }; */

        /* class PurchaseUnk final : public ServerPacket
        {
        public:
            PurchaseUnk() : ServerPacket(SMSG_BATTLE_PAY_PURCHASE_UNK, 20) { }

            WorldPacket const* Write() override;

            uint32 UnkInt = 0;
            std::string Key;
            uint8 UnkByte = 0;
        }; */

        class BattlePayBattlePetDelivered final : public ServerPacket
        {
        public:
            BattlePayBattlePetDelivered() : ServerPacket(SMSG_BATTLE_PAY_BATTLE_PET_DELIVERED, 20) { }

            WorldPacket const* Write() override;

            ObjectGuid BattlePetGuid;
            uint32 DisplayID = 0;
        };

        /* class PurchaseDetailsResponse final : public ClientPacket
        {
        public:
            PurchaseDetailsResponse(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PAY_PURCHASE_DETAILS_RESPONSE, std::move(packet)) { }

            void Read() override;

            uint8 UnkByte = 0;
        }; */

        /* class PurchaseUnkResponse final : public ClientPacket
        {
        public:
            PurchaseUnkResponse(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PAY_PURCHASE_UNK_RESPONSE, std::move(packet)) { }

            void Read() override;

            std::string Key;
            std::string Key2;
        }; */

        class DisplayPromotion final : public ServerPacket
        {
        public:
            DisplayPromotion(uint32 ID) : ServerPacket(SMSG_DISPLAY_PROMOTION, 4), PromotionID(ID) { }

            WorldPacket const* Write() override;

            uint32 PromotionID = 0;
        };

        /* class BattlepayUnk final : public ServerPacket
        {
        public:
            BattlepayUnk(uint32 unkInt) : ServerPacket(SMSG_BATTLE_PAY_UNK, 4), UnkInt(unkInt) { }

            WorldPacket const* Write() override;

            uint32 UnkInt = 0;
        }; */

        class BattlePayStartDistributionAssignToTargetResponse final : public ServerPacket
        {
        public:
            BattlePayStartDistributionAssignToTargetResponse() : ServerPacket(SMSG_BATTLE_PAY_START_DISTRIBUTION_ASSIGN_TO_TARGET_RESPONSE, 16) { }

            WorldPacket const* Write() override;

            uint64 DistributionID = 0;
            uint32 unkint1 = 0;
            uint32 unkint2 = 0;
        };

        class BattlePayRequestPriceInfo final : public ClientPacket
        {
        public:
            BattlePayRequestPriceInfo(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PAY_REQUEST_PRICE_INFO, std::move(packet)) { }

            void Read() override { }

            uint8 UnkByte = 0;
        };

        // This one is for updating character list screen after boost distribution assign or gear update packet where i use db queries to modify characters
        class GetAccountCharacterListResult final : public ServerPacket
        {
        public:
            GetAccountCharacterListResult() : ServerPacket(SMSG_GET_ACCOUNT_CHARACTER_LIST_RESULT, 1614) { }

            WorldPacket const* Write() override;

            uint32 Token = 0;
            bool UnkBit = false; // maybe check for boost?
            std::vector<WorldPackets::BattlePay::AccountCharacterListEntry> accountCharacterListEntry = std::vector<WorldPackets::BattlePay::AccountCharacterListEntry>();
        };
    }
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::DisplayInfo const& displayInfo);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::Product const& product);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::DistributionObject const& object);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::Purchase const& purchase);

#endif // BattlePayPackets_h__
