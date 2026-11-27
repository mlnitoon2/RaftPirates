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
        void CheckBossCoreRafts();
        bool ContainsGfx(sys::gfx::Gfx* gfx);
        void SetPriority(unsigned index, float priority);
        void BindGfx(unsigned side, unsigned index, sys::gfx::Gfx* gfx);
        void BindViews(const OceanView* own, const OceanView* enemy);
        bool OkayToPlace() const;
        Cell GetClosestPosition(sys::gfx::Vec2 world) const;
        Cell GetWorldPosition(int side) const;
        bool Select(bool own, int id);
        size_t Pieces() const;
        const BattleShipPlacementSprite& operator[](unsigned side) const;
        std::pair<int, int> gotMsgRequestBattlePlacementState() const;

      private:
        BattleShipPlacementSprite own_, enemy_;
        const OceanView* ownView_ = nullptr;
        const OceanView* enemyView_ = nullptr;
        bool attacker_ = false;
        std::map<unsigned, sys::gfx::Gfx*> ownGfx_, enemyGfx_;
    };
} // namespace rp
