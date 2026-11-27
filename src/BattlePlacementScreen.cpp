#include "rp/BattlePlacementScreen.h"
#include <stdexcept>
#include <algorithm>
#include <cmath>

namespace rp {
    BattlePlacementScreen::BattlePlacementScreen() = default;
    BattlePlacementScreen::~BattlePlacementScreen() = default;

    void BattlePlacementScreen::Reset(bool attacker) {
        own_ = {};
        enemy_ = {};
        attacker_ = attacker;
        own_.BindView(ownView_);
        enemy_.BindView(enemyView_);
        ownGfx_.clear();
        enemyGfx_.clear();
    }

    void BattlePlacementScreen::Update(const Player& own, const Player& enemy) {
        ownGfx_.clear();
        enemyGfx_.clear();
        own_.SetToObject(own);
        enemy_.SetToObject(enemy);
        own_.HighlightAttackRafts(enemy_);
        enemy_.HighlightAttackRafts(own_);
    }

    void BattlePlacementScreen::SetPosition(Cell position) {
        if (attacker_) {
            own_.SetPosition(position);
            own_.HighlightAttackRafts(enemy_);
            enemy_.HighlightAttackRafts(own_);
        }
    }

    void BattlePlacementScreen::CheckBossCoreRafts() {
        const auto cores = enemy_.BossCoreRaftsInMe();
        const auto& adjacent = own_.BossCoreRaftsByMe();
        if (!std::all_of(cores.begin(), cores.end(), [&](int id) {
            return std::find(adjacent.begin(), adjacent.end(), id) != adjacent.end();
        })) {
            own_.Revert();
            own_.HighlightAttackRafts(enemy_);
            enemy_.HighlightAttackRafts(own_);
        }
    }

    bool BattlePlacementScreen::ContainsGfx(sys::gfx::Gfx* gfx) {
        if (!gfx) return false;
        if (own_.ContainsGfx(gfx)) return true;
        return std::any_of(ownGfx_.begin(), ownGfx_.end(),
                           [&](const auto& entry) { return entry.second == gfx; });
    }

    void BattlePlacementScreen::SetPriority(unsigned index, float priority) {
        if (!std::isfinite(priority)) return;
        if (ownView_ || enemyView_) {
            const auto count = own_.GraphicsPieces();
            if (index < count) own_.SetPriority(index, priority);
            else enemy_.SetPriority(static_cast<unsigned>(index-count), priority);
            return;
        }
        const auto count = own_.Pieces();
        auto& sprites = index < count ? ownGfx_ : enemyGfx_;
        auto it = sprites.find(index < count ? index : static_cast<unsigned>(index - count));
        if (it != sprites.end()) it->second->SetPriority(priority);
    }

    void BattlePlacementScreen::BindViews(const OceanView* own, const OceanView* enemy) {
        ownView_ = own;
        enemyView_ = enemy;
        own_.BindView(own);
        enemy_.BindView(enemy);
    }

    void BattlePlacementScreen::BindGfx(unsigned side, unsigned index, sys::gfx::Gfx* gfx) {
        if (side > 1 || index >= (*this)[side].Pieces()) return;
        auto& sprites = side ? enemyGfx_ : ownGfx_;
        if (gfx) sprites[index] = gfx;
        else sprites.erase(index);
    }

    void BattlePlacementScreen::SetToObject() {
        own_.Revert();
        enemy_.Revert();
    }

    bool BattlePlacementScreen::OkayToPlace() const {
        return own_.OkayToPlace(enemy_) && !own_.OnlyNextToWreckage(enemy_);
    }

    Cell BattlePlacementScreen::GetClosestPosition(sys::gfx::Vec2 world) const {
        return own_.GetClosestPosition(world);
    }

    Cell BattlePlacementScreen::GetWorldPosition(int side) const {
        return (*this)[unsigned(side)].position();
    }

    bool BattlePlacementScreen::Select(bool own, int id) {
        auto& ship = own ? own_ : enemy_;
        return ship.Select(id);
    }

    size_t BattlePlacementScreen::Pieces() const {
        return 2;
    }

    const BattleShipPlacementSprite& BattlePlacementScreen::operator[](unsigned side) const {
        if (side > 1)
            throw std::out_of_range("Battle fleet");
        return side ? enemy_ : own_;
    }

    std::pair<int, int> BattlePlacementScreen::gotMsgRequestBattlePlacementState() const {
        return {own_.selected(), enemy_.selected()};
    }
} // namespace rp
