#include "rp/OnlineBattle.h"
#include "rp/Game.h"
#include <algorithm>
#include <limits>

namespace rp {
    using O=sfs::SFSObjectWrapper;
    void OnlineBattle::reset(){*this=OnlineBattle{};}
    void OnlineBattle::sendFailed(){if(pending_=="gs_battle_plunder")plunderSent_=false;pending_.clear();message_="Request failed";}
    std::optional<sfs::Message> OnlineBattle::action(const std::string& c,O p,bool withSync){
        if(waiting())return {};
        if(withSync)p.put("sync_counter",int32_t(sync_));
        pending_=c;return Game::request(c,p);
    }
    std::optional<sfs::Message> OnlineBattle::encounterResponse(bool attack){
        if(phase_!=Phase::Encounter)return {};
        decline_=!attack; O p;p.put("attack",attack);return action("gs_encounter_response",p,false);
    }
    std::optional<sfs::Message> OnlineBattle::selectRafts(int own,int enemy,const Player& player){
        if(phase_!=Phase::Positioning||!haveOpponent_)return {};
        const auto* a=player.GetRaftByID(own); const auto* b=opponent_->GetRaftByID(enemy);
        if(!a||!b||a->Destroyed()||b->Destroyed())return {};
        O p;p.put("attack_raft",int32_t(attacker_?own:enemy));p.put("defend_raft",int32_t(attacker_?enemy:own));p.put("ai_selection",int32_t(0));return action("gs_battle_select_raft",p);
    }
    std::optional<sfs::Message> OnlineBattle::clientReady(bool confirmSwap){
        if(phase_!=Phase::Fighting||!needReady_)return {};
        O p;p.put("confirm_swap_stance",confirmSwap);return action("gs_battle_client_ready",p);
    }
    std::optional<sfs::Message> OnlineBattle::attack(int dice){
        if(!readyForDice()||!attacker_||dice<=0||dice>8)return {};
        O p;p.put("attack_dice",int32_t(dice));return action("gs_battle_attack",p);
    }
    std::optional<sfs::Message> OnlineBattle::defend(int dice){
        if(!readyForDice()||attacker_||dice<=0||dice>8)return {};
        O p;p.put("defend_dice",int32_t(dice));return action("gs_battle_defend",p);
    }
    std::optional<sfs::Message> OnlineBattle::retreat(){
        if(phase_!=Phase::Positioning&&phase_!=Phase::Fighting&&phase_!=Phase::Desynced)return {};
        return action("gs_battle_retreat");
    }
    std::optional<sfs::Message> OnlineBattle::plunder(){
        if(phase_!=Phase::Over||plunderSent_||waiting())return {};
        auto result=action("gs_battle_plunder",{},false);if(result)plunderSent_=true;return result;
    }
    bool OnlineBattle::HandleExtensionResponse(const std::string& c,const O& p,const Player& player){
        if(c=="gs_encounter") {if(phase_==Phase::Idle){reset();phase_=Phase::Encounter;}return true;}
        if(c=="gs_start_battle"){
            reset();attacker_=p.getBool("attacker",false);phase_=Phase::Starting;return true;
        }
        if(c=="gs_error"){
            if(phase_==Phase::Idle)return false;
            message_=p.getString("message");if(pending_=="gs_battle_plunder")plunderSent_=false;pending_.clear();return true;
        }
        if(phase_==Phase::Idle)return false;
        if(c=="gs_opponent"){
            if(phase_!=Phase::Starting&&phase_!=Phase::Positioning)return true;
            auto data=p.getSFSObj("player_object");if(!data){message_="Missing opponent data";return true;}
            Player next;next.objects=player.objects;next.initWithSFSObject(*data);next.playerName=p.getString("opponent_name",next.playerName);
            opponent_=std::make_shared<Player>(std::move(next));ocean_.init(0);ocean_.addPlayer(1,opponent_,0,0);haveOpponent_=true;phase_=Phase::Positioning;pending_.clear();return true;
        }
        if(c=="gs_battle_opponent_time"){opponentTime_=std::max(0,p.getInt("opponent_time",0));return true;}
        if(c=="gs_battle_retreat"||c=="gs_encounter_response"){
            if(c=="gs_encounter_response"&&phase_==Phase::Encounter){if(decline_)reset();else {phase_=Phase::Starting;pending_.clear();}return true;}
            if(c=="gs_battle_retreat"){reset();return true;}
        }
        if(c=="gs_battle_over"){
            if(phase_!=Phase::Over){phase_=Phase::Over;lastResult_=p;pending_.clear();}return true;
        }
        if(c=="gs_battle_plunder"){if(phase_==Phase::Over)reset();return true;}
        if(c!="gs_battle_position"&&c!="gs_battle_select_raft"&&c!="gs_battle_attack"&&c!="gs_battle_defend"&&c!="gs_battle_client_ready"&&c!="gs_swap_stance")return false;
        if(phase_==Phase::Over||phase_==Phase::Desynced)return true;
        if(p.get("sync_counter")&&p.getInt("sync_counter",-1)!=sync_){
            int received=p.getInt("sync_counter",-1);
            if(received>=0&&received<sync_)return true;
            phase_=Phase::Desynced;pending_.clear();message_="Battle out of sync";return true;
        }
        if(c=="gs_battle_position"){
            if(phase_!=Phase::Starting&&phase_!=Phase::Positioning)return true;
            if(!p.getBool("attack_position_okay",false)){message_="Position rejected";phase_=Phase::Positioning;}
            else {if(sync_==std::numeric_limits<int>::max()){phase_=Phase::Desynced;return true;}++sync_; phase_=Phase::Positioning; if(haveOpponent_)ocean_.movePlayer(1,p.getInt("attack_x",0),p.getInt("attack_y",0));}
        }else if(c=="gs_battle_select_raft"){if(phase_!=Phase::Positioning)return true;if(sync_==std::numeric_limits<int>::max()){phase_=Phase::Desynced;return true;}++sync_;phase_=Phase::Fighting;needReady_=true;}
        else if(c=="gs_battle_attack"||c=="gs_battle_defend"){
            // Exact result ownership and counter advancement remain under investigation.
            // Retain the complete authoritative packet rather than guessing damage or turn order.
            if(phase_!=Phase::Fighting)return true;
            lastResult_=p;needReady_=true;
        }else if(c=="gs_battle_client_ready"){if(pending_!="gs_battle_client_ready")return true;needReady_=false;
        }else if(c=="gs_swap_stance"){
            // Wait for an explicit authoritative stance instead of locally toggling it twice.
            if(p.get("attacker")){attacker_=p.getBool("attacker");if(sync_==std::numeric_limits<int>::max()){phase_=Phase::Desynced;return true;}++sync_;needReady_=true;}
        }
        if(pending_==c || c=="gs_battle_attack" || c=="gs_battle_defend" || c=="gs_swap_stance")pending_.clear();
        return true;
    }
}
