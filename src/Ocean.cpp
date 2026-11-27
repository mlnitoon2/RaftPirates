#include "rp/Ocean.h"
#include <cmath>

namespace rp {
    Ocean::Ocean() = default;
    Ocean::~Ocean() = default;

    void Ocean::init(int zone) {
        zone_ = zone;
        players_.clear();
    }

    bool Ocean::initWithSFSObject(const sfs::SFSObjectWrapper& packet,const Player& catalogue){
        auto data=packet.getSFSObj("player_object");
        if(!data)return false;
        auto player=std::make_shared<Player>();
        player->objects=catalogue.objects;
        player->initWithSFSObject(*data);
        player->playerName=packet.getString("opponent_name",player->playerName);
        Ocean next;
        next.init(0);
        if(!next.addPlayer(1,std::move(player),0,0))return false;
        *this=std::move(next);return true;
    }

    bool Ocean::addPlayer(unsigned id, std::shared_ptr<Player> player, int x, int y) {
        if (!id || !player)
            return false;
        return players_.emplace(id, Entry{std::move(player), {x, y}}).second;
    }

    bool Ocean::movePlayer(unsigned id, int x, int y) {
        auto it = players_.find(id);
        if (it == players_.end())
            return false;
        it->second.position = {x, y};
        return true;
    }

    bool Ocean::removePlayer(unsigned id) {
        return players_.erase(id) != 0;
    }

    std::shared_ptr<Player> Ocean::GetPlayer(unsigned id) const {
        auto it = players_.find(id);
        return it == players_.end() ? nullptr : it->second.player;
    }

    bool Ocean::gotMsgPlayerBuiltRaft(unsigned id, Raft raft) {
        auto p = GetPlayer(id);
        return p && p->AddRaft(std::move(raft));
    }

    void Ocean::tick(float dt) {
        if (!std::isfinite(dt) || dt <= 0)
            return;
        for (auto& [id, e] : players_)
            for (auto& raft : e.player->rafts)
                for (auto& c : raft.components)
                    c.TempTick(dt);
    }
} // namespace rp
