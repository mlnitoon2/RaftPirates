#pragma once
#include "sfs/SFSObjectWrapper.h"
#include <string>

namespace rp {
    struct QuestTypeInfo {
        int id = 0;
        std::string name, sheet, image;
        explicit QuestTypeInfo(const sfs::SFSObjectWrapper& data);
        ~QuestTypeInfo();
    };

    class Quest {
      public:
        Quest();
        ~Quest();
        void initWithSFSObject(const sfs::SFSObjectWrapper& state,
                               const sfs::SFSObjectWrapper& definition);
        void markRead();
        void markOpened();
        void setProgressPercentage(int value);
        int id = 0, definitionId = 0, progress = 0, type = 0, group = 0;
        bool unread = false, visible = true, opened = false, popupImmediately = false;
        std::string name, description, sheet, image, highlightFeature;
        std::string sheetLeft, imageLeft, sheetBottom, imageBottom;
        std::vector<std::string> goals, blockInterface, unblockInterface;
    };
} // namespace rp
