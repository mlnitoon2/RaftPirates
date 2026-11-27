#pragma once
#include "sfs/SFSWire.h"
#include <deque>
#include <optional>

namespace rp {
    // Owns protocol packet copies, rather than original intrusive MsgBase clones.
    class MsgQueuer {
      public:
        MsgQueuer();
        ~MsgQueuer();
        void add(sfs::Message message);
        void clear();
        size_t numQueued() const;
        std::optional<sfs::Message> processOne();

      private:
        std::deque<sfs::Message> packets_;
    };
} // namespace rp
