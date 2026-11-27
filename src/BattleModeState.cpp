#include "rp/BattleModeState.h"
#include "rp/Game.h"
#include "rp/Messages.h"
#include "rp/RPMenuHandler.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <limits>

namespace rp {
    BattleModeState::BattleModeState()=default;
    BattleModeState::~BattleModeState() = default;
    const char* BattleModeState::name()const{return "rp::BattleMode";}
    void BattleModeState::Pause(){}
    void BattleModeState::HideDropdown(){dropdownVisible_=false;dropdownType_=-1;}
    void BattleModeState::StartWaiting(float seconds){
        if(std::isfinite(seconds))waitRemaining_=std::max(0.f,seconds);
    }
    void BattleModeState::StopWaiting(){waitRemaining_=0;HideDropdown();}
    void BattleModeState::tick(float dt){
        if(!std::isfinite(dt)||dt<=0)return;
        if(waitRemaining_>0){
            waitRemaining_=std::max(0.f,waitRemaining_-dt);
            if(waitRemaining_==0)StopWaiting();
        }
        introRemaining_=std::max(0.f,introRemaining_-dt);
        if(!dicePaused_)animationRemaining_=std::max(0.f,animationRemaining_-dt);
        if(pendingSwap_&&!dicePaused_&&animationRemaining_<=0)SwapPopup();
    }
    void BattleModeState::gotMsgBattleConfirmSwapStance(){
        confirmSwap_=true;swapPopupVisible_=false;
    }
    void BattleModeState::gotMsgToPositioningView(){ToPositioningView();}

    void BattleModeState::ToBattleView(){
        if(!haveOpponent_||phase_!=Phase::Fighting)return;
        battleView_=true;HideDropdown();
    }
    void BattleModeState::ToPositioningView(){
        if(phase_!=Phase::Positioning)return;
        battleView_=false;HideDropdown();
    }
    void BattleModeState::PlayBattleAnimations(){
        rolls_.clear();animationRemaining_=0;
        auto results=lastResult_.getSFSArray("results");if(!results)return;
        for(const auto& item:results->values){
            auto row=std::get_if<std::shared_ptr<sfs::SFSObjectWrapper>>(&item);
            if(!row||!*row)continue;
            const auto& p=**row;
            const int attack=p.getInt("attack_roll",0), defense=p.getInt("defense_roll",0);
            if(attack<1||attack>6||defense<1||defense>6)continue;
            auto target=p.getSFSObj("target_raft");
            rolls_.push_back({attack,defense,target?target->getInt("user_raft_id",0):0});
            if(rolls_.size()==8)break;
        }
        // Native PlayBattleAnimations writes 2.5 s at 0x389618.
        if(!rolls_.empty())animationRemaining_=2.5f;
    }
    void BattleModeState::SwapPopup(){
        if(!pendingSwap_)return;
        if(pendingSwap_->getInt("sync_counter",-1)!=sync_||sync_==std::numeric_limits<int>::max()){
            pendingSwap_.reset();phase_=Phase::Desynced;pending_.clear();
            message_="Battle out of sync";return;
        }
        swapAttacker_=pendingSwap_->getBool("attacker",!attacker_);
        pendingSwap_.reset();++sync_;HideDropdown();SwapStance();
    }
    void BattleModeState::SwapStance(){
        if(phase_!=Phase::Fighting&&phase_!=Phase::Positioning)return;
        // Host always indexes the fleets as own/opponent, unlike the native attacker/defender slots.
        // Changing stance must not exchange the actual Player objects or own/enemy raft IDs.
        attacker_=swapAttacker_.value_or(!attacker_);swapAttacker_.reset();
        swap_=true;needReady_=true;confirmSwap_=false;
        if(phase_==Phase::Positioning){positioned_=!attacker_;ToPositioningView();}
        else ToBattleView();
        MsgBattleInfoDropdown message;message.type=1;gotMsgBattleInfoDropdown(message);
        // Only the real UI asks for confirmation; headless callers may still confirm via SendClientReady.
        swapPopupVisible_=true;
    }
    void BattleModeState::gotMsgBattleInfoDropdown(const MsgBattleInfoDropdown& msg){
        HideDropdown();
        switch(msg.type){case 0:case 1:case 3:case 4:case 5:
            dropdownType_=msg.type;dropdownVisible_=true;break;default:break;}
    }
    std::vector<BattleModeState::BattleInfo> BattleModeState::gotMsgBattleInfoRequest(
        const MsgBattleInfoRequest&,const Player& player)const{
        std::vector<BattleInfo> result;
        auto add=[&](const Player& p,int id,bool own){
            auto* r=p.GetRaftByID(id);
            int points=r?r->GetBattlePoints():p.currentBP();
            int health=r?int(std::clamp(r->GetHealth(p.objects),0.f,1.f)*100.f):0;
            result.push_back({p.playerName,points,health,own});
        };
        add(player,ownRaft_,true);if(haveOpponent_)add(*opponent_,enemyRaft_,false);return result;
    }
    void BattleModeState::gotMsgHighlightHudItem(const MsgHighlightHudItem& msg){
        if(msg.item=="dice_results")dicePaused_=true;
        else if(msg.item=="unpause_dice_results"&&dicePaused_){
            dicePaused_=false;PlayBattleAnimations();
        }
    }
    void BattleModeState::gotMsgToggleBlockHudItem(const MsgToggleBlockHudItem& msg){
        for(const auto& name:msg.block)if(name=="positioningHud"||name=="bossBattleWon")blockedHud_.insert(name);
        for(const auto& name:msg.unblock)blockedHud_.erase(name);
    }
    void BattleModeState::gotMsgQuestRead(const MsgQuestRead&,RPMenuHandler* menu){
        // Native quest completion releases the positioning and boss-win tutorial blocks.
        if(blockedHud_.erase("positioningHud")&&menu)menu->setHudMode(4);
        blockedHud_.erase("bossBattleWon");
    }
    void BattleModeState::gotMsgTransitionDone(const MsgTransitionDone& msg){
        if(!msg.shown&&haveOpponent_&&phase_==Phase::Positioning)introRemaining_=3.f;
    }
    std::optional<sfs::Message> BattleModeState::RaftSelect(int own,int enemy,const Player& p){
        return SendRaftSelection(own,enemy,p);
    }
    std::optional<sfs::Message> BattleModeState::endBattleAndPlunder(){return plunder();}
    std::optional<sfs::Message> BattleModeState::gotMsgBattleOverConfirm(){return endBattleAndPlunder();}
    std::optional<sfs::Message> BattleModeState::gotMsgExitBattleModeRequest(){
        if(phase_==Phase::Over)return endBattleAndPlunder();
        if(phase_==Phase::Encounter)return encounterResponse(false);
        return retreat();
    }

    const char* BattleModeState::GetStateName() const {
        return "BattleModeState";
    }

    std::optional<sfs::Message> BattleModeState::gotMsgAttackRequest(int dice) {
        return attacker_ ? attack(dice) : defend(dice);
    }

    using O = sfs::SFSObjectWrapper;

    void BattleModeState::reset() {
        *this = BattleModeState{};
    }

    void BattleModeState::sendFailed() {
        if (pending_ == "gs_battle_plunder")
            plunderSent_ = false;
        pending_.clear();
        message_ = "Request failed";
    }

    std::optional<sfs::Message> BattleModeState::action(const std::string& c, O p, bool withSync) {
        if (waiting())
            return {};
        if (withSync)
            p.put("sync_counter", int32_t(sync_));
        pending_ = c;
        return Game::request(c, p);
    }

    std::optional<sfs::Message> BattleModeState::encounterResponse(bool attack) {
        if (phase_ != Phase::Encounter)
            return {};
        decline_ = !attack;
        O p;
        p.put("attack", attack);
        return action("gs_encounter_response", p, false);
    }

    std::optional<sfs::Message> BattleModeState::gotMsgBattleScreenRaftsSelected(int x, int y) {
        if (phase_ != Phase::Positioning || !haveOpponent_ || !attacker_ || positioned_)
            return {};
        O p;
        p.put("attack_x", int32_t(x));
        p.put("attack_y", int32_t(y));
        return action("gs_battle_position", p);
    }

    std::optional<sfs::Message> BattleModeState::SendRaftSelection(int own, int enemy,
                                                                   const Player& player) {
        if (phase_ != Phase::Positioning || !haveOpponent_ || (attacker_ && !positioned_) ||
            waiting())
            return {};
        const auto* a = player.GetRaftByID(own);
        const auto* b = opponent_->GetRaftByID(enemy);
        if (!a || !b || a->Destroyed() || b->Destroyed())
            return {};
        ownRaft_ = own;
        enemyRaft_ = enemy;
        O p;
        p.put("attack_raft", int32_t(attacker_ ? own : enemy));
        p.put("defend_raft", int32_t(attacker_ ? enemy : own));
        p.put("ai_selection", int32_t(0));
        return action("gs_battle_select_raft", p);
    }

    std::optional<sfs::Message> BattleModeState::SendClientReady(bool confirmSwap) {
        if (phase_ != Phase::Fighting || !needReady_ || waiting() || animationRemaining_>0 || dicePaused_)
            return {};
        if(swapPopupVisible_){confirmSwap_=true;swapPopupVisible_=false;}
        O p;
        p.put("confirm_swap_stance", confirmSwap || swap_ || confirmSwap_);
        return action("gs_battle_client_ready", p);
    }

    std::optional<sfs::Message> BattleModeState::attack(int dice) {
        if (!readyForDice() || !attacker_ || dice <= 0 || dice > 8)
            return {};
        O p;
        p.put("attack_dice", int32_t(dice));
        return action("gs_battle_attack", p);
    }

    std::optional<sfs::Message> BattleModeState::defend(int dice) {
        if (!readyForDice() || attacker_ || dice <= 0 || dice > 8)
            return {};
        O p;
        p.put("defend_dice", int32_t(dice));
        return action("gs_battle_defend", p);
    }

    std::optional<sfs::Message> BattleModeState::retreat() {
        if (phase_ != Phase::Positioning && phase_ != Phase::Fighting && phase_ != Phase::Desynced)
            return {};
        return action("gs_battle_retreat");
    }

    std::optional<sfs::Message> BattleModeState::plunder() {
        if (phase_ != Phase::Over || plunderSent_ || waiting())
            return {};
        auto result = action("gs_battle_plunder", {}, false);
        if (result)
            plunderSent_ = true;
        return result;
    }

    bool BattleModeState::HandleExtensionResponse(const std::string& c, const O& p,
                                                  Player& player) {
        if (c == "gs_encounter") {
            if (phase_ == Phase::Idle) {
                reset();
                phase_ = Phase::Encounter;
            }
            return true;
        }
        if (c == "gs_start_battle") {
            reset();
            attacker_ = p.getBool("attacker", false);
            phase_ = Phase::Starting;
            return true;
        }
        if (c == "gs_error") {
            if (phase_ == Phase::Idle)
                return false;
            message_ = p.getString("message");
            if (pending_ == "gs_battle_plunder")
                plunderSent_ = false;
            pending_.clear();
            return true;
        }
        if (phase_ == Phase::Idle)
            return false;
        if (c == "gs_opponent") {
            if (phase_ != Phase::Starting && phase_ != Phase::Positioning)
                return true;
            if(!ocean_.initWithSFSObject(p,player)){
                message_="Missing opponent data";
                return true;
            }
            opponent_=ocean_.GetPlayer(1);
            haveOpponent_ = true;
            boss_ = p.getBool("is_boss", false);
            phase_ = Phase::Positioning;
            ToPositioningView();
            pending_.clear();
            return true;
        }
        if (c == "combat_warning") {
            message_ = "Combat time remaining: " +
                       std::to_string(std::max(0, p.getInt("time_remaining", 0)));
            return true;
        }
        if (c == "gs_battle_opponent_time") {
            opponentTime_ = std::max(0, p.getInt("opponent_time", 0));
            return true;
        }
        if (c == "gs_battle_retreat" || c == "gs_encounter_response") {
            if (c == "gs_encounter_response" && phase_ == Phase::Encounter) {
                if (decline_)
                    reset();
                else {
                    phase_ = Phase::Starting;
                    pending_.clear();
                }
                return true;
            }
            if (c == "gs_battle_retreat") {
                reset();
                return true;
            }
        }
        if (c == "gs_battle_over") {
            if (phase_ != Phase::Over) {
                phase_ = Phase::Over;
                lastResult_ = p;
                pending_.clear();
            }
            return true;
        }
        if (c == "gs_battle_plunder") {
            if (phase_ == Phase::Over)
                reset();
            return true;
        }
        if (c != "gs_battle_position" && c != "gs_battle_select_raft" && c != "gs_battle_attack" &&
            c != "gs_battle_defend" && c != "gs_battle_client_ready" && c != "gs_swap_stance")
            return false;
        if (phase_ == Phase::Over || phase_ == Phase::Desynced)
            return true;
        if (p.get("sync_counter") && p.getInt("sync_counter", -1) != sync_) {
            int received = p.getInt("sync_counter", -1);
            if (received >= 0 && received < sync_)
                return true;
            phase_ = Phase::Desynced;
            pending_.clear();
            message_ = "Battle out of sync";
            return true;
        }
        if (c == "gs_battle_position") {
            if (phase_ != Phase::Starting && phase_ != Phase::Positioning)
                return true;
            if (!p.getBool("attack_position_okay", false)) {
                message_ = "Position rejected";
                phase_ = Phase::Positioning;
            } else {
                if (sync_ == std::numeric_limits<int>::max()) {
                    phase_ = Phase::Desynced;
                    return true;
                }
                ++sync_;
                positioned_ = true;
                phase_ = Phase::Positioning;
                if (haveOpponent_)
                    ocean_.movePlayer(1, p.getInt("attack_x", 0), p.getInt("attack_y", 0));
            }
        } else if (c == "gs_battle_select_raft") {
            if (phase_ != Phase::Positioning)
                return true;
            if (sync_ == std::numeric_limits<int>::max()) {
                phase_ = Phase::Desynced;
                return true;
            }
            if (p.get("attack_raft") && p.get("defend_raft")) {
                ownRaft_ = p.getInt(attacker_ ? "attack_raft" : "defend_raft");
                enemyRaft_ = p.getInt(attacker_ ? "defend_raft" : "attack_raft");
            }
            if (!player.GetRaftByID(ownRaft_) || !opponent_->GetRaftByID(enemyRaft_)) {
                phase_ = Phase::Desynced;
                pending_.clear();
                message_ = "Invalid selected rafts";
                return true;
            }
            ++sync_;
            phase_ = Phase::Fighting;
            needReady_ = true;
            ToBattleView();
        } else if (c == "gs_battle_attack" || c == "gs_battle_defend") {
            if (phase_ != Phase::Fighting || !p.get("sync_counter"))
                return true;
            auto results = p.getSFSArray("results");
            if (!results)
                return true; // Acknowledgement without dice results.
            auto* own = player.GetRaftByID(ownRaft_);
            auto* enemy = opponent_->GetRaftByID(enemyRaft_);
            if (!own || !enemy || own->id == enemy->id ||
                sync_ == std::numeric_limits<int>::max()) {
                phase_ = Phase::Desynced;
                pending_.clear();
                message_ = "Ambiguous battle result ownership";
                return true;
            }
            own->HandleBattleOutcome(results.get(), player.objects);
            enemy->HandleBattleOutcome(results.get(), opponent_->objects);
            lastResult_ = p;
            PlayBattleAnimations();
            ++sync_;
            needReady_ = true;
            if (own->Destroyed() || enemy->Destroyed()) {
                phase_ = Phase::Positioning;
                needReady_ = false;
                ToPositioningView();
            }
        } else if (c == "gs_battle_client_ready") {
            if (pending_ != "gs_battle_client_ready")
                return true;
            needReady_ = false;
            swap_ = false;
            confirmSwap_=false;swapPopupVisible_=false;HideDropdown();
        } else if (c == "gs_swap_stance") {
            if (p.get("sync_counter")&&!pendingSwap_) {
                pendingSwap_=p;
                if(animationRemaining_<=0&&!dicePaused_)SwapPopup();
            }
        }
        if (pending_ == c || c == "gs_battle_attack" || c == "gs_battle_defend" ||
            c == "gs_swap_stance")
            pending_.clear();
        return true;
    }
} // namespace rp

namespace rp {
    BattleModeState::BattleDice::BattleDice(const BattleDice&) = default;
    BattleModeState::BattleDice::~BattleDice() = default;
} // namespace rp
