#include "rp/Wreckage.h"
#include "rp/RaftComponent.h"
#include "rp/SparseGrid.h"
#include <algorithm>

namespace rp {
    Wreckage::Wreckage() = default;
    Wreckage::~Wreckage() = default;

    size_t Wreckage::Pieces(const GridObjectInfo* definition) const {
        return dynamic_cast<const RaftComponent*>(object_.get()) ?
            object_->Pieces(definition) : GridObject::Pieces(definition);
    }

    const std::vector<std::string>& Wreckage::RegisterAllClasses() {
        // Four identifiers recovered from PC-relative strings at native 0x5112d0 etc.
        static const std::vector<std::string> names{
            "OtherWreckage", "EnemyParts", "EnemyWreckage", "Wreckage"};
        return names;
    }

    std::unique_ptr<Wreckage> Wreckage::Build(const std::string& name) {
        const auto& classes = RegisterAllClasses();
        if (std::find(classes.begin(), classes.end(), name) == classes.end()) return {};
        return std::make_unique<Wreckage>();
    }

    std::unique_ptr<GridObject> Wreckage::Repair(int value, int maxPoints, SparseGrid& grid) {
        if (!object_) return {};
        auto restored = object_->Duplicate();
        const int oldId = GetUserObjectID();
        const auto* oldFootprint = grid.getObjectByID(oldId);
        if (!oldFootprint || restored->GetUserObjectID() != oldId) return {};
        // Grid entries are identity/footprint records. The caller replaces the owned
        // wreckage with this returned object after the authoritative repair response.
        restored->x = x;
        restored->y = y;
        restored->Repair(value, maxPoints);
        grid.RemoveFromTempTick(this);
        return restored;
    }

    bool Wreckage::Salvage(SparseGrid& grid) {
        if (object_) object_->Salvage(grid);
        // Wrapped and wreckage IDs may be the same in the portable model.
        const bool present = grid.hasObject(GetUserObjectID());
        return GridObject::Salvage(grid) || !present;
    }

    std::vector<std::string> Wreckage::getAvailableActions(const GridObjectInfo* definition) const {
        return object_ ? object_->getAvailableActions(definition) : std::vector<std::string>{};
    }
    std::unique_ptr<GridObject> Wreckage::Duplicate() const {
        auto result = std::make_unique<Wreckage>();
        static_cast<GridObject&>(*result) = static_cast<const GridObject&>(*this);
        result->offsetX = offsetX;
        result->offsetY = offsetY;
        result->SetWreckageObject(object_.get());
        return result;
    }
    int Wreckage::GetTotalBattlePoints(int maxPoints) const {
        return object_ ? object_->GetTotalBattlePoints(maxPoints) : GridObject::GetTotalBattlePoints(maxPoints);
    }
    int Wreckage::GetUserObjectID() const {
        return object_ ? object_->GetUserObjectID() : GridObject::GetUserObjectID();
    }
    const GridObject* Wreckage::GetWreckageObject() const { return object_.get(); }
    void Wreckage::SetWreckageObject(const GridObject* object) {
        object_ = object && object != this ? object->Duplicate() : nullptr;
    }
    void Wreckage::SetState(int value) {
        if (object_) object_->SetState(value);
        else GridObject::SetState(value);
    }
    void Wreckage::initWithSFSObject(const sfs::SFSObjectWrapper& data) {
        offsetX = data.getInt("offsetX");
        offsetY = data.getInt("offsetY");
    }
}
