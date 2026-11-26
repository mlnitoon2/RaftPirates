#pragma once
#include "rp/Game.h"

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

      private:
        const Game& game_;
    };
} // namespace rp
