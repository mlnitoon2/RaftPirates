#pragma once
#include "rp/BattleShipPlacementSprite.h"

namespace rp {
    class BattlePlacementScreen {
      public:
        BattlePlacementScreen();
        ~BattlePlacementScreen();
        void Reset(bool attacker);
        void Update(const Player& own, const Player& enemy);
        void SetPosition(Cell position);
        void SetToObject();
        bool OkayToPlace() const;
        Cell GetClosestPosition(sys::gfx::Vec2 world) const;
        Cell GetWorldPosition(int side) const;
        bool Select(bool own, int id);
        size_t Pieces() const;
        const BattleShipPlacementSprite& operator[](unsigned side) const;
        std::pair<int, int> gotMsgRequestBattlePlacementState() const;

      private:
        BattleShipPlacementSprite own_, enemy_;
        bool attacker_ = false;
    };
} // namespace rp
