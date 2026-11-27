#pragma once
#include "rp/RaftComponent.h"
namespace rp {
    // Standalone subclass until the component factory supports polymorphic ownership.
    class Barracks : public RaftComponent {
      public:
        Barracks();
        ~Barracks() override;
        bool Constructing() const;
        std::unique_ptr<GridObject> Duplicate() const override;
        void SetState(int value) override;
        void UseClock(float seconds);
        void initWithSFSObject(const sfs::SFSObjectWrapper& data);
        std::vector<std::string> getAvailableActions(const GridObjectInfo* definition) const override;
        bool TempTick(float dt) override;
    };
}
