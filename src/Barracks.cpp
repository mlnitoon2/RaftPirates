#include "rp/Barracks.h"
namespace rp {
    Barracks::Barracks() = default;
    Barracks::~Barracks() = default;
    bool Barracks::Constructing() const { return state == 4; }
    std::unique_ptr<GridObject> Barracks::Duplicate() const { return std::make_unique<Barracks>(*this); }
    // Native overrides are return-only; the authoritative state comes from snapshots.
    void Barracks::SetState(int) {}
    void Barracks::UseClock(float) {}
    void Barracks::initWithSFSObject(const sfs::SFSObjectWrapper& data) {
        RaftComponent::initWithSFSObject(data);
    }
    std::vector<std::string> Barracks::getAvailableActions(const GridObjectInfo* definition) const {
        return RaftComponent::getAvailableActions(definition);
    }
    bool Barracks::TempTick(float dt) {
        RaftComponent::TempTick(dt);
        // Original completion event and constructed-object creation still need routing.
        return !Constructing() || GetRemainingTime() <= 0;
    }
}
