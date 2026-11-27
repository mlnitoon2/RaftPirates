#include "rp/BuildModeState.h"
#include <algorithm>
#include <cmath>
#include "rp/RPMenuHandler.h"
#include "rp/TouchControls.h"
#include "rp/RaftGridSprite.h"
#include "rp/BattlePointsViewSprite.h"
#include "rp/Barracks.h"
#include "rp/Wreckage.h"

namespace rp {
    BuildModeState::BuildModeState(Game& game) : game_(game) {}

    BuildModeState::~BuildModeState() = default;

    void BuildModeState::BindView(OceanView* view,TouchControls* controls,RPMenuHandler* menu){
        ocean_=view;controls_=controls;menu_=menu;
    }

    void BuildModeState::ApplyFootprintToRaft(Raft* raft,bool grid){
        if(!ocean_||!raft)return;
        auto* sprite=ocean_->getGridSprite(0,raft->id);
        if(!sprite)return;
        if(grid&&!raft->Destroyed()&&!sprite->HasAccessorySprite(16))
            sprite->AddAccessorySprite(std::make_unique<RaftGridSprite>(ocean_,sprite));
        if(!sprite->HasAccessorySprite(6))
            sprite->AddAccessorySprite(std::make_unique<BattlePointsViewSprite>(ocean_,sprite,"",-1));
        for(const auto& c:raft->components)if(auto* part=ocean_->getGridSprite(raft->id,c.id))
            if(!part->HasAccessorySprite(6))
                part->AddAccessorySprite(std::make_unique<BattlePointsViewSprite>(ocean_,part,"",-1));
    }
    void BuildModeState::CleanupFootprints(Raft* raft,bool grid,bool points){
        if(!ocean_||!raft)return;
        if(auto* sprite=ocean_->getGridSprite(0,raft->id)){
            if(grid)sprite->RemoveAccessorySprite(16);
            if(points)sprite->RemoveAccessorySprite(6);
        }
        if(points)for(const auto& c:raft->components)
            if(auto* sprite=ocean_->getGridSprite(raft->id,c.id))sprite->RemoveAccessorySprite(6);
    }
    void BuildModeState::ChangeView(sys::gfx::Vec2 position,ViewMode mode,bool immediate){
        if(mode!=ViewMode::Normal&&mode!=ViewMode::Footprints)return;
        if(!std::isfinite(position.x)||!std::isfinite(position.y))return;
        viewMode_=mode;
        for(auto& raft:game_.rafts){
            CleanupFootprints(&raft,true,true);
            if(mode==ViewMode::Footprints)ApplyFootprintToRaft(&raft,true);
        }
        if(ocean_){
            ocean_->SetOffset(position,immediate);
            if(controls_){controls_->LockView(false);controls_->LockZoom(false);}
        }
    }
    void BuildModeState::gotMsgBuildModeToggleView(const MsgBuildModeToggleView&){
        ChangeView(ocean_?ocean_->Offset():sys::gfx::Vec2{},
            viewMode_==ViewMode::Footprints?ViewMode::Normal:ViewMode::Footprints,false);
    }
    std::unique_ptr<GridObject> BuildModeState::CreateGridObject(const std::string& name)const{
        auto it=std::find_if(game_.objects.begin(),game_.objects.end(),[&](const auto& p){return p.second.name==name;});
        if(it==game_.objects.end())return {};
        std::unique_ptr<GridObject> value;
        if(name=="Wreckage"||name=="OtherWreckage"||name=="EnemyWreckage"||name=="EnemyParts")
            value=Wreckage::Build(name);
        else if(name=="Barracks")value=std::make_unique<Barracks>();
        else if(it->second.type==0)value=std::make_unique<Raft>();
        else value=std::make_unique<RaftComponent>();
        if(!value)return {};
        value->type=it->first;value->setGridObjectData(name,game_.objects);return value;
    }

    bool BuildModeState::HandleExtensionResponse(const std::string& cmd,const sfs::SFSObjectWrapper& p){
        // Existing Game reducer owns these packet mutations; never apply them twice.
        static const std::set<std::string> commands{
            "gs_build_add","gs_build_update","gs_build_remove","gs_build_repair","gs_build_complete",
            "gs_materials_changed","gs_blueprints_changed","gs_use_clock","build_limit_update",
            "dive_info","dive_fail","gs_error","swaves_offerwall"};
        if(!commands.count(cmd))return false;
        game_.applyExtension(cmd,p);
        if(cmd=="gs_error"||cmd=="dive_fail")waiting_.clear();
        else FinishWaiting(cmd);
        if(cmd=="gs_materials_changed")FinishWaiting("gs_materials_changed");
        if(cmd=="gs_build_remove"||cmd=="gs_build_update"||cmd=="gs_build_repair"){
            checkFocusRaft();SetCheckForHoles(cmd);
        }
        if(ocean_){ocean_->gotMsgUpdateOceanView(game_);if(viewMode_==ViewMode::Footprints)
            for(auto& raft:game_.rafts)ApplyFootprintToRaft(&raft,true);}
        return true;
    }

    const RaftComponent* BuildModeState::component(int parent,int id)const{
        auto* r=game_.GetRaftByID(parent);if(!r)return nullptr;
        auto it=std::find_if(r->components.begin(),r->components.end(),[&](const auto& c){return c.id==id;});
        return it==r->components.end()?nullptr:&*it;
    }
    void BuildModeState::cancelNotificationForRaftComponent(int parent,int id){
        auto it=notifications_.find({parent,id});if(it==notifications_.end())return;
        if(cancelNotification)cancelNotification(it->second.key);notifications_.erase(it);
    }
    void BuildModeState::setupNotificationForRaftComponent(int parent,int id,int seconds){
        auto* c=component(parent,id);if(!c||seconds<=0)return;
        cancelNotificationForRaftComponent(parent,id);
        bool dive=c->type==12;
        Notification n{seconds,std::string(dive?"DIVE_COMPLETE":"COMPONENT_COMPLETE")+":"+std::to_string(parent)+":"+std::to_string(id),
            dive?"NOTIFICATION_DIVE_COMPLETE":"NOTIFICATION_ITEM_COMPLETE"};
        notifications_[{parent,id}]=n;if(scheduleNotification)scheduleNotification(n);
    }

    std::optional<sfs::Message> BuildModeState::SendBonusMaterials(){
        const auto& dive=game_.diveState;
        if(!game_.ready||!waiting_.empty()||game_.diveCollectPending||!dive.Diving()||dive.parent<=0||dive.object<=0||dive.collected.empty())return {};
        sfs::SFSObjectWrapper p;
        p.put("object_id",int32_t(dive.object));p.put("object_type",int32_t(1));p.put("parent_id",int32_t(dive.parent));
        auto bonus=std::make_shared<sfs::SFSArrayWrapper>();
        for(const auto& [id,amount]:dive.collected){
            auto available=dive.bonus.find(id);
            if(id<=0||amount<=0||available==dive.bonus.end()||amount>available->second)return {};
            auto item=std::make_shared<sfs::SFSObjectWrapper>();
            item->put("id",int32_t(id));item->put("amt",int32_t(amount));bonus->values.emplace_back(item);
        }
        p.put("bonus_items",bonus);StartWaiting("gs_materials_changed");game_.diveCollectPending=true;
        return Game::request("dive_collect",p);
    }
    bool BuildModeState::gotMsgDivingGameShow(const MsgDivingGameShow&){
        if(!ocean_||diving_||!game_.diveState.Diving())return false;
        try{
            diving_=std::make_unique<diving_game::DivingGame>(ocean_->assetRoot(),unsigned(std::max(1,game_.diveState.depth+1)));
            diving_->setDuration(float(std::max(8,game_.diveState.totalSeconds)));
            diving_->setPos(ocean_->Width()+210,22);diving_->moveTo(210,22,1.f);
        }catch(const std::exception&){diving_.reset();return false;}
        if(menu_)menu_->setHudMode(2);
        if(playMusic)playMusic("audio/music/underwater");
        return true;
    }
    std::optional<sfs::Message> BuildModeState::gotMsgDivingGameDone(const MsgDivingGameDone&){
        if(!diving_||closingDive_)return {};
        closingDive_=true;diving_->moveTo((ocean_?ocean_->Width():640)+210,22,1.f);
        if(menu_)menu_->setHudMode(1);
        if(playMusic)playMusic("audio/music/ambient");
        return {};
    }
    std::optional<sfs::Message> BuildModeState::gotMsgDivingGameDoneMoving(const MsgDivingGameDoneMoving&){
        if(!closingDive_||!diving_||diving_->moving())return {};
        diving_.reset();closingDive_=false;
        return SendBonusMaterials();
    }
    std::optional<sfs::Message> BuildModeState::gotMsgGridObjectEvent(const MsgGridObjectEvent& msg){
        if(!game_.ready||msg.id<=0)return {};
        if(msg.parent?!component(msg.parent,msg.id):!game_.GetRaftByID(msg.id))return {};
        sfs::SFSObjectWrapper p;p.put("object_id",int32_t(msg.id));p.put("object_type",int32_t(msg.parent?1:0));
        if(msg.parent)p.put("parent_id",int32_t(msg.parent));
        const char* cmd=nullptr;
        switch(msg.event){
        case 1:cmd="gs_build_complete";break;
        case 4:cmd="dive_start";break;
        case 5:if(questEvent)questEvent("dive");break;
        case 6:cmd="dive_info";if(questEvent)questEvent("dive_bottom");break;
        case 7:if(questEvent)questEvent("dive_finished");break;
        default:break;
        }
        if(!cmd)return {};
        if(msg.event==4||msg.event==6){
            if(game_.GetDiveUnit()!=std::make_pair(msg.parent,msg.id))return {};
        }
        if(msg.event==4)StartWaiting("dive_info");
        return Game::request(cmd,p);
    }
    void BuildModeState::gotMsgHighlightHudItem(const MsgHighlightHudItem& msg){
        if(highlightHud)highlightHud(msg.item);
        if(ocean_&&(msg.item=="diving_game"||msg.item=="dive_unit")){
            auto id=game_.GetDiveUnit();ocean_->gotMsgGfxLayerPickResult(id.first,id.second);
        }
    }
    void BuildModeState::gotMsgLoadingAnimFinished(const MsgLoadingAnimFinished&){
        // ARM 0x3b0714: safe players enter the title menu. The unsafe branch
        // sends a native message whose full dispatch has not yet been recovered.
        if(menu_&&game_.safe)menu_->pushMenu("title");
    }
    std::optional<sfs::Message> BuildModeState::gotMsgOfferWallReward(int amount){
        if(!game_.ready||amount<=0||!waiting_.empty())return {};
        sfs::SFSObjectWrapper p;p.put("amount",int32_t(amount));
        if(menu_)menu_->popPopUp();
        StartWaiting("swaves_offerwall");return Game::request("swaves_offerwall",p);
    }
    void BuildModeState::gotMsgToggleBlockHudItem(const MsgToggleBlockHudItem& msg){
        for(const auto& item:msg.block)if(item=="diving"||item=="diveFinish")blockedHud_.insert(item);
        for(const auto& item:msg.unblock)blockedHud_.erase(item);
    }
    void BuildModeState::gotMsgTransitionDone(const MsgTransitionDone& msg){
        if(msg.shown)return;
        if(menu_)menu_->setHudMode(1);
        if(playMusic)playMusic("audio/music/ambient");
        SetCheckForHoles("gotMsgTransitionDone");
    }
    std::optional<sfs::Message> BuildModeState::sendSelectedObjectRepairDataWithGold(sfs::SFSObjectWrapper& p){
        if(!game_.ready||!waiting_.empty()||!selected())return {};
        if(!repairObject(p.getInt("repair_all",0)!=0))return {};
        p.put("object_id",int32_t(id_));p.put("object_type",int32_t(parent_?1:0));
        if(parent_)p.put("parent_id",int32_t(parent_));
        StartWaiting("gs_build_repair");return Game::request("purchase_repair",p);
    }
    void BuildModeState::tick(float dt){
        if(!std::isfinite(dt)||dt<0)return;
        checkFocusRaft();
        if(checkHoles_){checkHoles_=false;if(!game_.mainRaft())deselectObject();}
        if(diving_)diving_->tick(dt);
        if(ocean_&&viewMode_==ViewMode::Footprints)
            for(auto& raft:game_.rafts)ApplyFootprintToRaft(&raft,true);
        for(auto it=notifications_.begin();it!=notifications_.end();){
            if(!component(it->first.first,it->first.second)){
                if(cancelNotification)cancelNotification(it->second.key);it=notifications_.erase(it);
            }else ++it;
        }
    }
    const char* BuildModeState::name()const{return "rp::BuildMode";}
    void BuildModeState::Pause(){} // Original ARM single return.
    void BuildModeState::gotMsgSparseGridObjectAdded(){} // Original ARM single return.
    void BuildModeState::SetCheckForHoles(const std::string&){checkHoles_=true;}
    bool BuildModeState::gotMsgCheckForHoles(int id){
        SetCheckForHoles("check");checkHoles_=false;return CheckForHoles(id);
    }
    void BuildModeState::gotMsgWreckageSelected(int parent,int id){
        gotMsgGridObjectSelected(parent,id);
        if(!selected()||!selected()->Destroyed())deselectObject();
    }

    void BuildModeState::selectObject(int parent, int userObjectID) {
        parent_ = parent;
        id_ = userObjectID;
    }

    void BuildModeState::deselectObject() {
        parent_ = id_ = 0;
    }

    const GridObject* BuildModeState::selected() const {
        if (!id_)
            return nullptr;
        if (!parent_)
            return game_.GetRaftByID(id_);
        auto raft = game_.GetRaftByID(parent_);
        if (!raft)
            return nullptr;
        auto component = std::find_if(raft->components.begin(), raft->components.end(),
                                      [&](const auto& c) { return c.id == id_; });
        return component == raft->components.end() ? nullptr : &*component;
    }

    std::optional<sfs::Message> BuildModeState::repairSelected(bool all) const {return repairObject(all);}
    std::optional<sfs::Message> BuildModeState::repairObject(bool all) const {
        auto object = selected();
        if (!game_.ready || !object)
            return {};
        auto info = object->getResourceData(game_.objects);
        if (!info || (!all && object->bp >= info->bp))
            return {};
        if (auto c = dynamic_cast<const RaftComponent*>(object); c && c->Building())
            return {};
        sfs::SFSObjectWrapper p;
        p.put("object_id", int32_t(id_));
        p.put("object_type", int32_t(parent_ ? 1 : 0));
        if (parent_)
            p.put("parent_id", int32_t(parent_));
        p.put("repair_all", int32_t(all));
        return Game::request("gs_build_repair", p);
    }

    std::optional<sfs::Message> BuildModeState::UseClockSelected() const {
        auto object = dynamic_cast<const RaftComponent*>(selected());
        if (!game_.ready || !object || !object->Building() || object->GetRemainingTime() <= 0 ||
            !game_.hasMaterial(10, 1))
            return {};
        sfs::SFSObjectWrapper p;
        p.put("object_id", int32_t(id_));
        p.put("object_type", int32_t(1));
        p.put("parent_id", int32_t(parent_));
        return Game::request("gs_use_clock", p);
    }

    bool BuildModeState::isReadyForBattle() const {
        return game_.ready && !game_.safe && !game_.badZone && !game_.RaftCleanupRequired() &&
               game_.mainRaft() && !game_.mainRaft()->Destroyed() && waiting_.empty() &&
               !game_.pending && !game_.pendingMove;
    }

    void BuildModeState::checkFocusRaft() {
        if (!selected())
            deselectObject();
    }

    void BuildModeState::StartWaiting(const std::string& reason) {
        waiting_ = reason;
    }

    void BuildModeState::FinishWaiting(const std::string& reason) {
        if (waiting_ == reason)
            waiting_.clear();
    }

    void BuildModeState::gotMsgBuildCancel() {
        deselectObject();
        waiting_.clear();
    }

    void BuildModeState::gotMsgDeselectObject() {
        deselectObject();
    }

    std::optional<sfs::Message> BuildModeState::gotMsgDestroyObjectRequest() const {
        if (!game_.ready || !selected() || !waiting_.empty())
            return {};
        if (!parent_ && CheckForHoles(id_))
            return {};
        return game_.remove(parent_, id_);
    }

    std::optional<sfs::Message> BuildModeState::gotMsgBuildConfirm(int blueprint, int parent,
                                                                   Cell pos) {
        return doPlaceObject(blueprint,parent,pos);
    }
    std::optional<sfs::Message> BuildModeState::doPlaceObject(int blueprint,int parent,Cell pos){
        if (!waiting_.empty() || !gotMsgStartBuildRequest(blueprint))
            return {};
        auto b = game_.blueprints.find(blueprint);
        if (b == game_.blueprints.end() || !game_.owned.count(blueprint))
            return {};
        auto object = game_.objects.find(b->second.object);
        if (object == game_.objects.end())
            return {};
        if (parent ? !game_.canPlaceComponent(object->first, parent, pos.x, pos.y)
                   : !game_.canBuildRaft(pos))
            return {};
        return game_.build(object->first, parent, pos.x, pos.y);
    }

    std::optional<sfs::Message> BuildModeState::gotMsgMoveObjectRequest(int newParent, Cell pos) {
        if (!game_.ready || !parent_ || !selected() || game_.pendingMove || !waiting_.empty())
            return {};
        if (!game_.canPlaceComponent(selected()->type, newParent, pos.x, pos.y, parent_, id_))
            return {};
        return game_.moveComponent(parent_, id_, newParent, pos.x, pos.y);
    }

    std::vector<std::pair<int, int>> BuildModeState::wrecked(bool components) const {
        std::vector<std::pair<int, int>> out;
        for (const auto& r : game_.rafts) {
            if (components) {
                for (const auto& c : r.components)
                    if (c.Destroyed())
                        out.emplace_back(r.id, c.id);
            } else if (r.Destroyed())
                out.emplace_back(0, r.id);
        }
        return out;
    }

    int BuildModeState::gotMsgRequestNumWreckedRafts() const {
        return int(wrecked(false).size());
    }

    int BuildModeState::gotMsgRequestNumWreckedRaftComponents() const {
        return int(wrecked(true).size());
    }

    void BuildModeState::initWreckedRaftCheck() {
        wreckedRaft_ = 0;
        selectWreckedRaft();
    }

    void BuildModeState::initWreckedRaftComponentCheck() {
        wreckedComponent_ = 0;
        selectWreckedRaftComponent();
    }

    void BuildModeState::selectWreckedRaft() {
        auto entries = wrecked(false);
        if (entries.empty()) {
            deselectObject();
            return;
        }
        wreckedRaft_ %= entries.size();
        selectObject(0, entries[wreckedRaft_].second);
    }

    void BuildModeState::selectWreckedRaftComponent() {
        auto entries = wrecked(true);
        if (entries.empty()) {
            deselectObject();
            return;
        }
        wreckedComponent_ %= entries.size();
        auto p = entries[wreckedComponent_];
        selectObject(p.first, p.second);
    }

    void BuildModeState::gotMsgNextWreckedRaft() {
        ++wreckedRaft_;
        selectWreckedRaft();
    }

    void BuildModeState::gotMsgPreviousWreckedRaft() {
        auto n = wrecked(false).size();
        if (n)
            wreckedRaft_ = (wreckedRaft_ % n + n - 1) % n;
        selectWreckedRaft();
    }

    void BuildModeState::gotMsgNextWreckedRaftComponent() {
        ++wreckedComponent_;
        selectWreckedRaftComponent();
    }

    void BuildModeState::gotMsgPreviousWreckedRaftComponent() {
        auto n = wrecked(true).size();
        if (n)
            wreckedComponent_ = (wreckedComponent_ % n + n - 1) % n;
        selectWreckedRaftComponent();
    }

    std::vector<std::string> BuildModeState::SendMsgSelectedObjectAvailableActions() const {
        const auto* object = selected();
        if (!game_.ready || !object || !waiting_.empty() || game_.pending || game_.pendingMove)
            return {};
        auto actions = object->getAvailableActions(object->getResourceData(game_.objects));
        if (!parent_ && (game_.mainRaft() == object || CheckForHoles(id_))) {
            actions.erase(std::remove(actions.begin(), actions.end(), "destroy"), actions.end());
            actions.erase(std::remove(actions.begin(), actions.end(), "salvage"), actions.end());
        }
        return actions;
    }

    std::optional<sfs::Message> BuildModeState::gotMsgRepairSelectedObject() const {
        return repairSelected(false);
    }

    std::optional<sfs::Message> BuildModeState::gotMsgRepairSelectedObjectAll() const {
        return repairSelected(true);
    }

    std::optional<sfs::Message> BuildModeState::gotMsgUseClockOnSelectedObject() const {
        return UseClockSelected();
    }

    std::optional<sfs::Message> BuildModeState::gotMsgSalvageSelectedObject() const {
        return salvageSelected();
    }

    std::optional<sfs::Message> BuildModeState::salvageSelected() const {
        if (!selected() || !selected()->Destroyed())
            return {};
        if(auto raft=dynamic_cast<const Raft*>(selected());raft&&!raft->Salvage())return {};
        return gotMsgDestroyObjectRequest();
    }

    void BuildModeState::gotMsgGridObjectSelected(int parent, int id) {
        selectObject(parent, id);
        checkFocusRaft();
    }

    bool BuildModeState::gotMsgStartBuildRequest(int blueprint) const {
        auto b = game_.blueprints.find(blueprint);
        if (!game_.ready || game_.pending || game_.pendingMove || b == game_.blueprints.end() ||
            !game_.owned.count(blueprint))
            return false;
        if (!game_.objects.count(b->second.object) || b->second.deprecated)
            return false;
        for (auto [id, count] : b->second.recipe)
            if (!game_.hasMaterial(id, count))
                return false;
        return true;
    }

    bool BuildModeState::CheckForHoles(int removedRaft) const {
        const auto* core = game_.mainRaft();
        if (!core || core->id == removedRaft)
            return true;
        std::set<int> seen{core->id};
        std::vector<int> queue{core->id};
        for (size_t i = 0; i < queue.size(); ++i) {
            const auto* current = game_.GetRaftByID(queue[i]);
            for (const auto& raft : game_.rafts)
                if (raft.id != removedRaft &&
                    std::abs(int64_t(current->x) - raft.x) +
                            std::abs(int64_t(current->y) - raft.y) ==
                        1 &&
                    seen.insert(raft.id).second)
                    queue.push_back(raft.id);
        }
        for (const auto& raft : game_.rafts)
            if (raft.id != removedRaft && !seen.count(raft.id))
                return true;
        return false;
    }

    const char* BuildModeState::GetStateName() const {
        return "BuildModeState";
    }

} // namespace rp
