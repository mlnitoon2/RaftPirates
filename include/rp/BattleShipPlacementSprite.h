#pragma once
#include "rp/Game.h"

namespace rp {
    class BattleShipPlacementSprite {
      public:
        BattleShipPlacementSprite();
        ~BattleShipPlacementSprite();
        void Update(const Player& player);
        void SetToObject(const Player& player);
        void SetPosition(Cell position);
        bool Select(int id);
        void HighlightAttackRafts(const BattleShipPlacementSprite& other);
        void UpdateParticles();
        void BindView(const OceanView* view) { view_ = view; }
        bool ContainsGfx(const sys::gfx::Gfx*) const;
        bool HasAccessorySprite(int id) const;
        void SetPriority(unsigned index, float priority);
        void SetScale(float scale);
        size_t GraphicsPieces() const;
        std::vector<int> BossCoreRaftsInMe()const;
        const std::vector<int>& BossCoreRaftsByMe()const;
        void SetObjectPosition(Cell position);
        void SetPositionOfSprites(Cell position);
        void Revert();
        bool OkayToPlace(const BattleShipPlacementSprite& other) const;
        bool OnlyNextToWreckage(const BattleShipPlacementSprite& other) const;
        void SelectRaft(int id);

        int selected() const {
            return selected_;
        }

        size_t Pieces() const;
        const Raft& operator[](unsigned index) const;
        Cell GetClosestPosition(sys::gfx::Vec2 world) const;
        void HighlightRaftsNextTo(const BattleShipPlacementSprite& other);

        const std::vector<int>& highlighted() const {
            return highlighted_;
        }

        Cell position() const {
            return offset_;
        }

      private:
        std::vector<GridSprite*> graphics() const;
        const OceanView* view_ = nullptr;
        std::vector<Raft> pieces_;
        std::vector<int> highlighted_,bossCores_,adjacentBossCores_;
        Cell offset_{};
        int selected_ = 0;
    };
} // namespace rp
