#include "rp/RPMenuHandler.h"
#include <algorithm>

namespace rp {
    std::vector<int> RPMenuHandler::gotMsgGetAvailStoreItemRequest() const {
        std::vector<int> result;
        for (auto& [id, item] : game_.storeItems) {
            if (item.name.empty())
                continue;
            if (item.group == "blueprints") {
                auto b = std::find_if(game_.blueprints.begin(), game_.blueprints.end(),
                                      [&](const auto& v) { return v.second.name() == item.name; });
                if (b == game_.blueprints.end() || game_.owned.count(b->first) ||
                    b->second.deprecated ||
                    (b->second.purchaseZone > 0 && b->second.purchaseZone > game_.zone))
                    continue;
            }
            result.push_back(id);
        }
        return result;
    }

    std::vector<int> RPMenuHandler::gotMsgGetAllBlueprintsRequest() const {
        std::vector<int> result;
        for (auto& [id, b] : game_.blueprints)
            if (!b.deprecated)
                result.push_back(id);
        std::stable_sort(result.begin(), result.end(), [&](int a, int b) {
            return game_.blueprints.at(a).displayOrder < game_.blueprints.at(b).displayOrder;
        });
        return result;
    }

    std::vector<int> RPMenuHandler::gotMsgGetAvailBlueprintsRequest() const {
        auto result = gotMsgGetAllBlueprintsRequest();
        result.erase(std::remove_if(result.begin(), result.end(),
                                    [&](int id) { return !game_.owned.count(id); }),
                     result.end());
        return result;
    }

    std::vector<int> RPMenuHandler::gotMsgGetAvailMaterialsRequest() const {
        std::vector<int> result;
        for (auto& [id, n] : game_.materials)
            if (n > 0)
                result.push_back(id);
        return result;
    }

    int RPMenuHandler::gotMsgSpecificMaterialAvailAmountRequest(int id) const {
        return game_.numOfMaterialPossessed(unsigned(id));
    }

    bool RPMenuHandler::gotMsgHaveMaterialsRequest(int blueprint) const {
        auto b = game_.blueprints.find(blueprint);
        if (b == game_.blueprints.end())
            return false;
        for (auto& [id, amount] : b->second.recipe)
            if (!game_.hasMaterial(id, amount))
                return false;
        return true;
    }

    std::map<int, int> RPMenuHandler::gotMsgRecipeInformationRequest(int blueprint) const {
        auto b = game_.blueprints.find(blueprint);
        return b == game_.blueprints.end() ? std::map<int, int>{} : b->second.recipe;
    }

    std::vector<int> RPMenuHandler::gotMsgQuestInformationRequest() const {
        std::vector<int> result;
        for (auto& [id, q] : game_.quests)
            if (q.visible)
                result.push_back(id);
        return result;
    }

    bool RPMenuHandler::canPurchase(int id) const {
        auto available = gotMsgGetAvailStoreItemRequest();
        if (std::find(available.begin(), available.end(), id) == available.end())
            return false;
        const auto& item = game_.storeItems.at(id);
        return item.price >= 0 && (item.price == 0 || game_.hasMaterial(item.currency, item.price));
    }
} // namespace rp
