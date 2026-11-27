#pragma once
#include "rp/Player.h"

namespace rp {
    // Independent player fleets and positions. Original server snapshot format is not assumed.
    class Ocean {
      public:
        struct Entry {
            std::shared_ptr<Player> player;
            Cell position;
        };

        Ocean();
        ~Ocean();
        void init(int zone);
        // Snapshot adapter for the existing gs_opponent player_object payload.
        bool initWithSFSObject(const sfs::SFSObjectWrapper&,const Player& catalogue);
        bool addPlayer(unsigned id, std::shared_ptr<Player> player, int x, int y);
        bool movePlayer(unsigned id, int x, int y);
        bool removePlayer(unsigned id);
        std::shared_ptr<Player> GetPlayer(unsigned id) const;
        bool gotMsgPlayerBuiltRaft(unsigned id, Raft raft);
        void tick(float dt);

        const std::map<unsigned, Entry>& players() const {
            return players_;
        }

        int zone() const {
            return zone_;
        }

      private:
        int zone_ = 0;
        std::map<unsigned, Entry> players_;
    };
} // namespace rp
