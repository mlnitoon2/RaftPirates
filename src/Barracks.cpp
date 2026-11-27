#include "rp/Barracks.h"
#include "rp/SparseGrid.h"
#include <cmath>
#include <limits>

namespace rp {
    Barracks::Barracks() = default;
    Barracks::~Barracks() = default;

    bool Barracks::Constructing() const {
        return state == 4;
    }

    void Barracks::AddedToOcean() {
        attached_ = true;
    }

    void Barracks::AddedToOcean(SparseGrid& grid) {
        RaftComponent::AddedToOcean(grid);
        attached_ = true;
        if (Constructing())
            grid.AddToTempTick(this);
    }

    void Barracks::SetConstructionInfo(float seconds, int objectType) {
        if (!std::isfinite(seconds) || seconds < 0 ||
            double(seconds) > std::numeric_limits<int>::max())
            return;
        const auto whole = int(std::ceil(double(seconds)));
        SetRemainingTime(whole);
        remainingFraction = whole - double(seconds);
        // Native builds a component through its factory at zero. Store the requested
        // type only; no speculative insertion or reward before a server snapshot.
        constructionType_ = objectType;
    }

    std::unique_ptr<GridObject> Barracks::Duplicate() const {
        return std::make_unique<Barracks>(*this);
    }

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
} // namespace rp
