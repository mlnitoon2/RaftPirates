#pragma once
#include "sfs/SFSObjectWrapper.h"
#include <string>
#include <map>

namespace rp {
    // Static catalogue entry; ID lookup is also used by owned blueprints.
    struct BlueprintInfo {
        ~BlueprintInfo();
        std::string sheet, image;
      protected:
        std::string name_;
    };

    class Blueprint : public BlueprintInfo {
      public:
        Blueprint() = default;
        Blueprint(int id, int object, int unlock, std::string name);
        void init(int id);
        void initWithSFSObject(const sfs::SFSObjectWrapper& data);
        int id() const;
        const std::string& name() const;
        int object = 0, unlock = 0;
        bool deprecated = false;
        int purchaseZone = 0, displayOrder = 0, buildSeconds = 0;
        std::map<int, int> recipe;

      private:
        int id_ = 0;
    };
} // namespace rp
