#pragma once
#include "rp/FootprintSprite.h"

namespace rp {
    // Portable base for placement previews. Native base Select and OkayToPlace return false.
    class PlacementSprite {
      public:
        PlacementSprite(const Game&, const OceanView&);
        virtual ~PlacementSprite();
        virtual bool OkayToPlace() const;
        virtual bool Select(const sys::gfx::Gfx*);
        static std::optional<Cell> GetClosestPosition(sys::gfx::Vec2 world,
                                                      const std::vector<Cell>& candidates);
        virtual void SetPosition(Cell);
        virtual void SetToObject(int object, int parent, int ignoreParent = 0, int ignoreId = 0);
        virtual void SetPriorities();
        virtual void Update();

      protected:
        const Game& game_;
        const OceanView& view_;
        int object_ = 0, parent_ = 0, ignoreParent_ = 0, ignoreId_ = 0;
        Cell position_{};
    };
} // namespace rp
