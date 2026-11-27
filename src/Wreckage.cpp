#include "rp/Wreckage.h"

namespace rp {
    Wreckage::Wreckage() = default;
    Wreckage::~Wreckage() = default;
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
