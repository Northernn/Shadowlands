/*
* Copyright 2021 Thordekk
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

#ifndef __TRINITY_BATTLEPAYMGR_H
#define __TRINITY_BATTLEPAYMGR_H

#include "BattlePayPackets.h"
#include "WorldSession.h"
#include "BattlePayData.h"

class LoginQueryHolder;

namespace Battlepay
{
    /// Client error enum See Blizzard_StoreUISecure.lua Last update : 9.0.2 36474
    enum Error : uint32
    {
        Ok = 0,
        PurchaseDenied = 1,
        PaymentFailed = 2,
        Other = 3,
        WrongCurrency = 12,
        BattlepayDisabled = 13,
        InvalidPaymentMethod = 25,
        InsufficientBalance = 28,
        ParentalControlsNoPurchase = 34,
        ConsumableTokenOwned = 46,
        TooManyTokens = 47,
    };

    namespace DistributionStatus
    {
        enum
        {
            BATTLE_PAY_DIST_STATUS_NONE = 0,
            BATTLE_PAY_DIST_STATUS_AVAILABLE = 1,
            BATTLE_PAY_DIST_STATUS_ADD_TO_PROCESS = 2,
            BATTLE_PAY_DIST_STATUS_PROCESS_COMPLETE = 3,
            BATTLE_PAY_DIST_STATUS_FINISHED = 4
        };
    };

    namespace VasPurchaseProgress
    {
        enum : uint8
        {
            Invalid = 0,
            PrePurchase = 1,
            PaymentPending = 2,
            ApplyingLicense = 3,
            WaitingOnQueue = 4,
            Ready = 5,
            ProcessingFactionChange = 6,
            Complete = 7,
        };
    }

    namespace ProductListResult
    {
        enum
        {
            Available = 0,
            LockUnk1 = 1,
            LockUnk2 = 2,
            RegionLocked = 3
        };
    }

    namespace UpdateStatus
    {
        enum
        {
            Loading = 9,
            Ready = 6,
            Finish = 3
        };
    }

    enum String
    {
        AtGoldLimit = 14090,
        NeedToBeInGame = 14091,
        TooHighLevel = 14092,
        YouAlreadyOwnThat = 14093,
        Level50Required = 14094,
        ReachPrimaryProfessionLimit = 14095,
        NotEnoughFreeBagSlots = 14096
    };

    enum ProductType
    {
        // retail values:
        Item_ = 0,
        LevelBoost = 1,
        Pet = 2,
        Mount = 3,
        WoWToken = 4,
        NameChange = 5,
        FactionChange = 6,
        RaceChange = 8,
        CharacterTransfer = 11,
        Toy = 14,
        Expansion = 18,
        GameTime = 20,
        GuildNameChange = 21,
        GuildFactionChange = 22,
        GuildTransfer = 23,
        GuildFactionTranfer = 24,
        TransmogAppearance = 26,
        Gold = 30,
        Currency = 31,
        // custom values:
        ItemSet = 100,
        Buff2Hour = 101,
        UnlockAllHeirloom = 102,
        Loadout = 103,
        GearUpdate = 104,
        Achievement = 105,
        MultipleQuest = 106,
    };

    struct Purchase
    {
        Purchase()
        {
            memset(this, 0, sizeof(Purchase));
        }

        ObjectGuid TargetCharacter;
        uint64 DistributionId;
        uint64 PurchaseID;
        uint64 CurrentPrice;
        uint32 ClientToken;
        uint32 ServerToken;
        uint32 ProductID;
        uint8 Status;
        bool Lock;
    };

    struct PurchaseData
    {
        uint32 PurchaseID;
        int8 PurchaseStatus;
        ObjectGuid characterGuid;
        uint32 productID;
        std::string productName;
    };
}

class TC_GAME_API BattlepayManager
{
    Battlepay::Purchase _actualTransaction;
    std::map<uint32, WorldPackets::BattlePay::Product> _existProducts;

private:
    WorldSession* _session;
    uint64 _purchaseIDCount;
    uint64 _distributionIDCount;
    std::string _walletName;
public:
    explicit BattlepayManager(WorldSession* session);
    ~BattlepayManager();

    uint32 GetBattlePayCredits() const;
    bool HasBattlePayCredits(uint32 count) const;
    bool UpdateBattlePayCredits(uint64 price) const;
    bool ModifyBattlePayCredits(uint64 credits) const;
    void SendBattlePayMessage(uint32 bpaymessageID, std::string name, uint32 value = 0) const;
    void SendBattlePayBattlePetDelivered(ObjectGuid petguid, uint32 creatureID) const;
    uint32 GetShopCurrency() const;
    bool IsAvailable() const;
    bool AlreadyOwnProduct(uint32 itemId) const;
    void SavePurchase(Battlepay::Purchase* purchase);
    std::vector<Battlepay::PurchaseData> GetPurchases(uint32 accountID);
    void ProcessDelivery(Battlepay::Purchase* purchase);
    void RegisterStartPurchase(Battlepay::Purchase purchase);
    uint64 GenerateNewPurchaseID();
    uint64 GenerateNewDistributionId();
    Battlepay::Purchase* GetPurchase();
    std::string const& GetDefaultWalletName() const;
    std::tuple<bool, WorldPackets::BattlePay::DisplayInfo> WriteDisplayInfo(uint32 productTemplateEntry, int8 titleColorIndex = 0);
    void SendProductList();
    void SendAccountCredits();
    void SendDistributionListResponse();
    void SendDistributionUpdate(uint32 purchaseID, uint8 distributionStatus);
    void AssignDistributionToCharacter(uint32 ProductID, uint64 DistributionID, ObjectGuid CharGUID, int32 SpecID);
    void Update(uint32 diff);
    void AddBattlePetFromBpayShop(uint32 battlePetCreatureID) const;
};

#endif
