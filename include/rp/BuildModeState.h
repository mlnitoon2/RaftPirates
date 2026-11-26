#pragma once
#include "rp/Game.h"

namespace rp {
    // Reconstructed selection/request portion of the original game state.
    class BuildModeState {
      public:
        explicit BuildModeState(Game& game);
        ~BuildModeState();
        void selectObject(int parent, int userObjectID);
        void deselectObject();
        std::optional<sfs::Message> repairSelected(bool all) const;
        std::optional<sfs::Message> UseClockSelected() const;

      private:
        const GridObject* selected() const;
        Game& game_;
        int parent_ = 0, id_ = 0;
    };
} // namespace rp
