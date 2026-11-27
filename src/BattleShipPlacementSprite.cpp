#include "rp/BattleShipPlacementSprite.h"
#include "rp/GridSprite.h"
#include <algorithm>
#include <cmath>

namespace rp {
    namespace {
        long long distance(const Raft& a, Cell ap, const Raft& b, Cell bp) {
            return std::abs(static_cast<long long>(a.x) + ap.x - b.x - bp.x) +
                   std::abs(static_cast<long long>(a.y) + ap.y - b.y - bp.y);
        }
    } // namespace

    BattleShipPlacementSprite::BattleShipPlacementSprite() = default;
    BattleShipPlacementSprite::~BattleShipPlacementSprite() = default;

    void BattleShipPlacementSprite::Update(const Player& player) {
        pieces_ = player.rafts;
        bossCores_.clear();adjacentBossCores_.clear();
        for(const auto& raft:pieces_)if(!raft.Destroyed()&&raft.getName(player.objects)=="BossCoreRaft")bossCores_.push_back(raft.id);
        SelectRaft(selected_);
        if (!selected_) {
            auto it = std::find_if(pieces_.begin(), pieces_.end(),
                                   [](const auto& r) { return !r.Destroyed() && r.bp > 0; });
            if (it != pieces_.end())
                selected_ = it->id;
        }
    }

    void BattleShipPlacementSprite::SetToObject(const Player& player) { Update(player); }
    void BattleShipPlacementSprite::SetPosition(Cell position) { SetObjectPosition(position); }
    bool BattleShipPlacementSprite::Select(int id) { SelectRaft(id); return selected_ != 0; }
    void BattleShipPlacementSprite::HighlightAttackRafts(const BattleShipPlacementSprite& other) {
        HighlightRaftsNextTo(other);
    }
    // Original ARM implementation is a return-only weak hook.
    void BattleShipPlacementSprite::UpdateParticles() {}

    std::vector<GridSprite*> BattleShipPlacementSprite::graphics() const {
        std::vector<GridSprite*> result;
        if (!view_) return result;
        for (const auto& raft : pieces_) {
            if (auto* sprite = view_->getGridSprite(0, raft.id)) result.push_back(sprite);
            for (const auto& component : raft.components)
                if (auto* sprite = view_->getGridSprite(raft.id, component.id)) result.push_back(sprite);
        }
        return result;
    }

    bool BattleShipPlacementSprite::ContainsGfx(const sys::gfx::Gfx* gfx) const {
        if (!gfx) return false;
        for (auto* sprite : graphics()) if (sprite->ContainsGfx(gfx)) return true;
        return false;
    }

    bool BattleShipPlacementSprite::HasAccessorySprite(int id) const {
        for (auto* sprite : graphics()) if (sprite->HasAccessorySprite(id)) return true;
        return false;
    }

    size_t BattleShipPlacementSprite::GraphicsPieces() const {
        size_t count = 0;
        for (auto* sprite : graphics()) count += sprite->Pieces();
        return count;
    }

    void BattleShipPlacementSprite::SetPriority(unsigned index, float priority) {
        if (!std::isfinite(priority)) return;
        size_t i = index;
        for (auto* sprite : graphics()) {
            if (i < sprite->Pieces()) { sprite->SetPriority(static_cast<unsigned>(i), priority); return; }
            i -= sprite->Pieces();
        }
    }

    void BattleShipPlacementSprite::SetScale(float scale) {
        if (!std::isfinite(scale) || scale <= 0) return;
        for (auto* sprite : graphics()) sprite->SetScale(scale);
    }

    void BattleShipPlacementSprite::SetObjectPosition(Cell position) {
        offset_ = position;
    }

    void BattleShipPlacementSprite::SetPositionOfSprites(Cell position) {
        SetObjectPosition(position);
    }

    std::vector<int> BattleShipPlacementSprite::BossCoreRaftsInMe()const{return bossCores_;}
    const std::vector<int>& BattleShipPlacementSprite::BossCoreRaftsByMe()const{return adjacentBossCores_;}
    void BattleShipPlacementSprite::Revert() {
        offset_ = {};
        highlighted_.clear();adjacentBossCores_.clear();
    }

    bool BattleShipPlacementSprite::OkayToPlace(const BattleShipPlacementSprite& other) const {
        if (pieces_.empty() || other.pieces_.empty())
            return false;
        for (const auto& a : pieces_)
            for (const auto& b : other.pieces_)
                if (distance(a, offset_, b, other.offset_) == 0)
                    return false;
        return true;
    }

    bool
    BattleShipPlacementSprite::OnlyNextToWreckage(const BattleShipPlacementSprite& other) const {
        bool wreckage = false;
        for (const auto& a : pieces_) {
            if (a.Destroyed())
                continue;
            for (const auto& b : other.pieces_)
                if (distance(a, offset_, b, other.offset_) == 1) {
                    if (!b.Destroyed())
                        return false;
                    wreckage = true;
                }
        }
        return wreckage;
    }

    void BattleShipPlacementSprite::SelectRaft(int id) {
        auto it = std::find_if(pieces_.begin(), pieces_.end(), [&](const auto& r) {
            return r.id == id && !r.Destroyed() && r.bp > 0;
        });
        selected_ = it == pieces_.end() ? 0 : id;
    }

    size_t BattleShipPlacementSprite::Pieces() const {
        return pieces_.size();
    }

    const Raft& BattleShipPlacementSprite::operator[](unsigned index) const {
        return pieces_.at(index);
    }

    Cell BattleShipPlacementSprite::GetClosestPosition(sys::gfx::Vec2 world) const {
        if (!std::isfinite(world.x) || !std::isfinite(world.y))
            return offset_;
        return {int(std::lround(std::clamp(world.x, -10000.f, 10000.f))),
                int(std::lround(std::clamp(world.y, -10000.f, 10000.f)))};
    }

    void BattleShipPlacementSprite::HighlightRaftsNextTo(const BattleShipPlacementSprite& other) {
        highlighted_.clear();adjacentBossCores_.clear();
        for(const auto& b:other.pieces_)if(std::find(other.bossCores_.begin(),other.bossCores_.end(),b.id)!=other.bossCores_.end())
            for(const auto& a:pieces_)if(!a.Destroyed()&&distance(a,offset_,b,other.offset_)==1){adjacentBossCores_.push_back(b.id);break;}
        for (const auto& a : pieces_)
            if (!a.Destroyed())
                for (const auto& b : other.pieces_)
                    if (!b.Destroyed() && distance(a, offset_, b, other.offset_) == 1) {
                        highlighted_.push_back(a.id);
                        break;
                    }
    }
} // namespace rp
