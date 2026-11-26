#pragma once
#include "rp/Game.h"
#include <deque>

namespace rp {
    // Typed equivalents of native menu messages; not the original message ABI.
    class RPMenuHandler {
      public:
        explicit RPMenuHandler(const Game& game) : game_(game) {}

        std::vector<int> gotMsgGetAvailStoreItemRequest() const;
        std::vector<int> gotMsgGetAllBlueprintsRequest() const;
        std::vector<int> gotMsgGetAvailBlueprintsRequest() const;
        std::vector<int> gotMsgGetAvailMaterialsRequest() const;
        int gotMsgSpecificMaterialAvailAmountRequest(int id) const;
        bool gotMsgHaveMaterialsRequest(int blueprint) const;
        std::map<int, int> gotMsgRecipeInformationRequest(int blueprint) const;
        std::vector<int> gotMsgQuestInformationRequest() const;
        bool canPurchase(int item) const;
        void gotMsgSetBlueprintSelected(int id);
        void gotMsgSetBlueprintPage(int page);
        void gotMsgSetStoreItemPage(int page);
        void gotMsgSetStoreCategory(std::string category);
        void gotMsgSetStoreItemSelected(int id);
        int gotMsgStoreItemPageRequest() const;
        int gotMsgBlueprintPageRequest() const;
        const std::string& gotMsgStoreCategoryRequest() const;
        const StoreItem* gotMsgStoreItemSelectedRequest() const;
        std::map<int, int> gotMsgRecipeSelectedRequest() const;
        void setHudMode(int mode);
        int gotMsgHudModeRequest() const;
        void gotMsgSetHudModeRequest(int mode);
        void pushMenu(std::string menu);
        void popMenu();
        void pushPopUp(std::string popup);
        void popPopUp();
        std::string topPopUp() const;
        void displayPopUpImmediately(std::string popup);
        void gotMsgQueueMenuPopup(std::string popup);
        bool hasAnyQueuedPopups() const;
        void enableNewQuestPopupHandling(bool enabled);
        void showQuestDialogImmediately(int id);
        void tick(float dt);
        void destroyCurMenuData();
        std::vector<int> currentStorePage(size_t pageSize) const;
        std::vector<int> currentBlueprintPage(size_t pageSize) const;
        const Blueprint* gotMsgBlueprintDisplayRequest(int id) const;
        const StoreItem* gotMsgStoreItemDisplayRequest(int id) const;
        const MaterialInfo* gotMsgMaterialInformationRequest(int id) const;
        const sfs::SFSObjectWrapper* gotMsgQuestTypeInformationRequest(int id) const;
        bool gotMsgRequestOceanSafetyUpdate() const;
        int gotMsgShowBattlePointsRequest() const;

      private:
        const Game& game_;
        int blueprintSelected_ = 0, storeSelected_ = 0, blueprintPage_ = 0, storePage_ = 0,
            hudMode_ = 0;
        std::string category_;
        std::vector<std::string> menus_, popups_;
        std::deque<std::string> queued_;
        bool newQuestPopups_ = true;
    };
} // namespace rp
