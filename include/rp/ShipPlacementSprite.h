#pragma once
#include "rp/PlacementSprite.h"
#include "rp/GridSprite.h"

namespace rp {
    // Host placement preview owns the copied raft hierarchy and its graphics.
    // Native Ocean/SparseGrid ownership is not reproduced by this adapter.
    class ShipPlacementSprite final : public PlacementSprite {
      public:
        ShipPlacementSprite(const Game&, const OceanView&, std::vector<Raft>,
                            bool movingExisting = true);
        ~ShipPlacementSprite() override;
        bool ContainsGfx(const sys::gfx::Gfx*) const;
        bool OkayToPlace() const override;
        size_t Pieces() const;
        void SetObjectPosition(Cell);
        void SetPosition(Cell) override;
        void SetPosition(sys::gfx::Vec2& screen);
        void SetPositionOfSprites(sys::gfx::Vec2 world);
        void SetPriority(unsigned index, float priority);
        void SetToObject();
        void Update() override;
        int submit(sys::gfx::GfxManager&, std::map<std::string, std::shared_ptr<sys::gfx::Image>>&,
                   std::map<std::string, sys::res::ResourceAEAnim>&, sys::gfx::AnimationRenderer&,
                   const std::filesystem::path&);

        Cell position() const {
            return position_;
        }

      private:
        struct Entry {
            std::unique_ptr<GridSprite> sprite;
            sys::gfx::Vec2 relative;
            bool raft;
        };

        std::vector<Raft> rafts_;
        std::map<int, GridObjectInfo> catalogue_;
        Cell anchor_{};
        bool movingExisting_;
        std::vector<Entry> entries_;
        void rebuild();
    };
} // namespace rp
