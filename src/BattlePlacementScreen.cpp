#include "rp/BattlePlacementScreen.h"
#include <stdexcept>

namespace rp {
    BattlePlacementScreen::BattlePlacementScreen() = default;
    BattlePlacementScreen::~BattlePlacementScreen() = default;

    void BattlePlacementScreen::Reset(bool attacker) {
        own_ = {};
        enemy_ = {};
        attacker_ = attacker;
    }

    void BattlePlacementScreen::Update(const Player& own, const Player& enemy) {
        own_.Update(own);
        enemy_.Update(enemy);
        own_.HighlightRaftsNextTo(enemy_);
        enemy_.HighlightRaftsNextTo(own_);
    }

    void BattlePlacementScreen::SetPosition(Cell position) {
        if (attacker_)
            own_.SetObjectPosition(position);
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
        ship.SelectRaft(id);
        return ship.selected() != 0;
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
