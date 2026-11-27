#pragma once
#include "rp/GridObject.h"
#include "sfs/SFSObjectWrapper.h"

namespace rp {
    // Owned snapshot of a wrecked object. Does not transfer live SparseGrid ownership.
    class Wreckage : public GridObject {
      public:
        Wreckage();
        ~Wreckage() override;
        std::unique_ptr<GridObject> Duplicate() const override;
        int GetTotalBattlePoints(int maxPoints) const override;
        int GetUserObjectID() const override;
        const GridObject* GetWreckageObject() const;
        void SetWreckageObject(const GridObject* object);
        void SetState(int value) override;
        size_t Pieces(const GridObjectInfo* definition) const override;
        static const std::vector<std::string>& RegisterAllClasses();
        static std::unique_ptr<Wreckage> Build(const std::string& name);
        std::unique_ptr<GridObject> Repair(int value, int maxPoints, SparseGrid& grid);
        bool Salvage(SparseGrid& grid);
        std::vector<std::string>
        getAvailableActions(const GridObjectInfo* definition) const override;
        void initWithSFSObject(const sfs::SFSObjectWrapper& data);
        int offsetX = 0, offsetY = 0;

      private:
        std::unique_ptr<GridObject> object_;
    };
} // namespace rp
