#pragma once
#include "sfs/SFSObjectWrapper.h"
#include <string>

namespace rp {
    class Quest {
      public:
        Quest();
        ~Quest();
        void initWithSFSObject(const sfs::SFSObjectWrapper& state,
                               const sfs::SFSObjectWrapper& definition);
        void markRead();
        void markOpened();
        void setProgressPercentage(int value);
        int id = 0, progress = 0, type = 0;
        bool unread = false, visible = true, opened = false;
        std::string name, description, sheet, image;
    };
} // namespace rp
