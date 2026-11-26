#pragma once
#include "rp/Game.h"

namespace rp {
    // Local, transient sandbox. Never used to send simulated packets to an online server.
    class LocalSession {
      public:
        void start(Game& game);
        void dispatch(Game& game, const sfs::Message& message);

      private:
        int nextId_ = 10;
    };
} // namespace rp
