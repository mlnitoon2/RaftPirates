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

    std::vector<int> RPMenuHandler::storeItemsForTab(const std::string& menu)const{
        if(menu.rfind("tabbed_menu_",0)!=0)return {};
        const auto category=menu.substr(12);
        auto ids=gotMsgGetAvailStoreItemRequest();
        ids.erase(std::remove_if(ids.begin(),ids.end(),[&](int id){
            const auto& group=game_.storeItems.at(id).group;
            if(category=="diamonds")return group!="diamonds"&&group!="diamond";
            if(category=="gold")return group!="gold"&&group!="offerwall";
            return group!=category;
        }),ids.end());
        return ids;
    }
    std::optional<int> RPMenuHandler::storeCategoryId()const{
        for(const auto& [id,name]:game_.storeGroups)if(name==category_)return id;
        return {};
    }
    void RPMenuHandler::gotMsgStoreDisplayStatus(const MsgStoreDisplayStatus& msg){storeVisible_=msg.visible;}
    void RPMenuHandler::gotMsgPopPopUpGlobal(){popPopUp();}
    std::optional<sfs::Message> RPMenuHandler::gotMsgQuestRead(Game& game,const MsgQuestRead& msg){
        if(&game!=&game_||!game.quests.count(msg.id))return {};
        return game.openQuest(msg.id);
    }
    void RPMenuHandler::gotMsgStartDialog(const std::string& name){displayPopUpImmediately(name);}

    std::string RPMenuHandler::gotMsgRequestLocalizedChristeningText(
            const std::string& localized, const Player* player) const {
        // Native reads TUT_CHRISTEN_D, replaces the first percent-delimited
        // segment with the requested player's name and sends localized insert text.
        if (!player) return {};
        const auto first = localized.find('%');
        if (first == std::string::npos) return localized;
        const auto last = localized.find('%', first + 1);
        if (last == std::string::npos) return localized;
        return localized.substr(0, first) + player->playerName + localized.substr(last + 1);
    }
    std::vector<int> RPMenuHandler::gotSimpleRequest(const std::string& kind)const{
        if(kind=="AVAIL_STOREITEMS")return category_.empty()?gotMsgGetAvailStoreItemRequest():storeItemsForTab("tabbed_menu_"+category_);
        if(kind=="AVAIL_BLUEPRINTS")return gotMsgGetAvailBlueprintsRequest();
        if(kind=="AVAIL_MATERIALS")return gotMsgGetAvailMaterialsRequest();
        return {};
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

    const Blueprint* RPMenuHandler::gotMsgBlueprintDisplayRequest(int id) const {
        auto it = game_.blueprints.find(id);
        return it == game_.blueprints.end() ? nullptr : &it->second;
    }

    const StoreItem* RPMenuHandler::gotMsgStoreItemDisplayRequest(int id) const {
        auto it = game_.storeItems.find(id);
        return it == game_.storeItems.end() ? nullptr : &it->second;
    }

    std::optional<MsgStoreItemDisplayResponse> RPMenuHandler::storeDisplayMessage(int id)const{
        auto p=gotMsgStoreItemDisplayRequest(id);
        if(!p)return {};
        return MsgStoreItemDisplayResponse(*p);
    }
    std::optional<MsgQuestData> RPMenuHandler::questDataMessage(int id)const{
        auto p=game_.quests.find(id);
        if(p==game_.quests.end())return {};
        return MsgQuestData(p->second);
    }
    const MaterialInfo* RPMenuHandler::gotMsgMaterialInformationRequest(int id) const {
        return id > 0 ? game_.materialDB.getMaterialInfo(unsigned(id)) : nullptr;
    }

    const sfs::SFSObjectWrapper* RPMenuHandler::gotMsgQuestTypeInformationRequest(int id) const {
        return id > 0 ? game_.questTypeDB.getQuestTypeInfo(unsigned(id)) : nullptr;
    }

    bool RPMenuHandler::gotMsgRequestOceanSafetyUpdate() const {
        return game_.isSafe();
    }

    int RPMenuHandler::gotMsgShowBattlePointsRequest() const {
        return game_.currentBP();
    }

    bool RPMenuHandler::canPurchase(int id) const {
        auto available = gotMsgGetAvailStoreItemRequest();
        if (std::find(available.begin(), available.end(), id) == available.end())
            return false;
        const auto& item = game_.storeItems.at(id);
        return item.price >= 0 && (item.price == 0 || game_.hasMaterial(item.currency, item.price));
    }

    void RPMenuHandler::gotMsgSetBlueprintSelected(int id) {
        blueprintSelected_ = gotMsgBlueprintDisplayRequest(id) ? id : 0;
    }

    void RPMenuHandler::gotMsgSetBlueprintPage(int page) {
        blueprintPage_ = std::max(0, page);
    }

    void RPMenuHandler::gotMsgSetStoreItemPage(int page) {
        storePage_ = std::max(0, page);
    }

    void RPMenuHandler::gotMsgSetStoreCategory(std::string category) {
        category_ = std::move(category);
        storePage_ = 0;
        storeSelected_ = 0;
    }

    void RPMenuHandler::gotMsgSetStoreItemSelected(int id) {
        storeSelected_ = gotMsgStoreItemDisplayRequest(id) ? id : 0;
    }

    int RPMenuHandler::gotMsgStoreItemPageRequest() const {
        return storePage_;
    }

    int RPMenuHandler::gotMsgBlueprintPageRequest() const {
        return blueprintPage_;
    }

    const std::string& RPMenuHandler::gotMsgStoreCategoryRequest() const {
        return category_;
    }

    const StoreItem* RPMenuHandler::gotMsgStoreItemSelectedRequest() const {
        return gotMsgStoreItemDisplayRequest(storeSelected_);
    }

    std::map<int, int> RPMenuHandler::gotMsgRecipeSelectedRequest() const {
        return gotMsgRecipeInformationRequest(blueprintSelected_);
    }

    void RPMenuHandler::setHudMode(int mode) {
        hudMode_ = mode;
    }

    int RPMenuHandler::gotMsgHudModeRequest() const {
        return hudMode_;
    }

    void RPMenuHandler::gotMsgSetHudModeRequest(int mode) {
        setHudMode(mode);
    }

    void RPMenuHandler::pushMenu(std::string menu) {
        if (!menu.empty())
            menus_.push_back(std::move(menu));
    }

    void RPMenuHandler::popMenu() {
        if (!menus_.empty())
            menus_.pop_back();
    }

    void RPMenuHandler::pushPopUp(std::string popup) {
        if (!popup.empty())
            popups_.push_back(std::move(popup));
    }

    void RPMenuHandler::popPopUp() {
        if (!popups_.empty())
            popups_.pop_back();
    }

    std::string RPMenuHandler::topPopUp() const {
        return popups_.empty() ? std::string{} : popups_.back();
    }

    void RPMenuHandler::displayPopUpImmediately(std::string popup) {
        pushPopUp(std::move(popup));
    }

    void RPMenuHandler::gotMsgQueueMenuPopup(std::string popup) {
        if (!popup.empty())
            queued_.push_back(std::move(popup));
    }

    bool RPMenuHandler::hasAnyQueuedPopups() const {
        return !queued_.empty();
    }

    void RPMenuHandler::enableNewQuestPopupHandling(bool enabled) {
        newQuestPopups_ = enabled;
    }

    void RPMenuHandler::showQuestDialogImmediately(int id) {
        if (newQuestPopups_ && game_.quests.count(id))
            displayPopUpImmediately("quest:" + std::to_string(id));
    }

    void RPMenuHandler::tick(float dt) {
        if (dt >= 0 && popups_.empty() && !queued_.empty()) {
            pushPopUp(std::move(queued_.front()));
            queued_.pop_front();
        }
    }

    void RPMenuHandler::destroyCurMenuData() {
        menus_.clear();
        popups_.clear();
        queued_.clear();
        blueprintSelected_ = storeSelected_ = blueprintPage_ = storePage_ = hudMode_ = 0;
        category_.clear();
        storeVisible_=false;
    }

    namespace {
        std::vector<int> pageOf(std::vector<int> ids, int page, size_t size) {
            if (!size || size_t(page) > ids.size() / size)
                return {};
            size_t start = size_t(page) * size, end = start + std::min(size, ids.size() - start);
            return {ids.begin() + start, ids.begin() + end};
        }
    } // namespace

    std::vector<int> RPMenuHandler::currentStorePage(size_t size) const {
        auto ids = gotMsgGetAvailStoreItemRequest();
        if (!category_.empty())ids=storeItemsForTab("tabbed_menu_"+category_);
        return pageOf(std::move(ids), storePage_, size);
    }

    std::vector<int> RPMenuHandler::currentBlueprintPage(size_t size) const {
        return pageOf(gotMsgGetAvailBlueprintsRequest(), blueprintPage_, size);
    }
} // namespace rp
