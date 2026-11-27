#include "rp/ShipPlacementSprite.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

namespace rp {
    ShipPlacementSprite::ShipPlacementSprite(const Game& game, const OceanView& view,
                                             std::vector<Raft> rafts, bool movingExisting)
        : PlacementSprite(game, view), rafts_(std::move(rafts)), catalogue_(game.objects),
          movingExisting_(movingExisting) {
        if (!rafts_.empty())
            anchor_ = rafts_.front().GetGridPosition();
        position_ = anchor_;
        rebuild();
    }

    ShipPlacementSprite::~ShipPlacementSprite() = default;

    void ShipPlacementSprite::rebuild() {
        entries_.clear();
        for (const auto& raft : rafts_) {
            const sys::gfx::Vec2 base{float(double(raft.x) - anchor_.x),
                                      float(double(raft.y) - anchor_.y)};
            auto add = [&](const GridObject& object, sys::gfx::Vec2 relative, bool isRaft) {
                auto def = catalogue_.find(object.type);
                if (def == catalogue_.end())
                    return;
                auto sprite = std::make_unique<GridSprite>(
                    view_, object, def->second,
                    sys::gfx::Vec2{position_.x + relative.x, position_.y + relative.y},
                    view_.Zoom());
                entries_.push_back({std::move(sprite), relative, isRaft});
            };
            add(raft, base, true);
            for (const auto& part : raft.components)
                add(part, {base.x + part.x / 6.f + 1.f / 12, base.y + part.y / 6.f + 1.f / 12},
                    false);
        }
        Update();
    }

    bool ShipPlacementSprite::ContainsGfx(const sys::gfx::Gfx* gfx) const {
        if (!gfx)
            return false;
        for (const auto& entry : entries_)
            if (entry.sprite->ContainsGfx(gfx))
                return true;
        return false;
    }

    bool ShipPlacementSprite::OkayToPlace() const {
        if (rafts_.empty())
            return false;
        std::set<int> moving;
        if (movingExisting_)
            for (const auto& raft : rafts_)
                moving.insert(raft.id);
        std::set<std::pair<int, int>> occupied;
        for (const auto& raft : game_.rafts) {
            if (moving.count(raft.id))
                continue;
            try {
                for (auto c : raft.occupiedCells(raft.getResourceData(game_.objects)))
                    occupied.emplace(c.x, c.y);
            } catch (const std::overflow_error&) {
                return false;
            }
        }
        std::set<std::pair<int, int>> target;
        for (const auto& raft : rafts_) {
            try {
                for (auto cell : raft.occupiedCells(raft.getResourceData(catalogue_))) {
                    auto x = int64_t(cell.x) - anchor_.x + position_.x;
                    auto y = int64_t(cell.y) - anchor_.y + position_.y;
                    if (x < INT32_MIN || x > INT32_MAX || y < INT32_MIN || y > INT32_MAX)
                        return false;
                    auto key = std::make_pair(int(x), int(y));
                    if (occupied.count(key) || !target.insert(key).second)
                        return false;
                }
            } catch (const std::overflow_error&) {
                return false;
            }
        }
        return true;
    }

    size_t ShipPlacementSprite::Pieces() const {
        size_t result = 0;
        for (const auto& entry : entries_)
            result += entry.sprite->Pieces();
        return result;
    }

    void ShipPlacementSprite::SetObjectPosition(Cell position) {
        position_ = position;
        Update();
    }

    void ShipPlacementSprite::SetPosition(Cell position) {
        SetObjectPosition(position);
    }

    void ShipPlacementSprite::SetPosition(sys::gfx::Vec2& screen) {
        auto point = view_.ScreenToWorld(screen);
        if (!std::isfinite(point.x) || !std::isfinite(point.y))
            return;
        auto clamp = [](float n) { return int(std::lround(std::clamp(n, -10000.f, 10000.f))); };
        SetObjectPosition({clamp(point.x), clamp(point.y)});
    }

    void ShipPlacementSprite::SetPositionOfSprites(sys::gfx::Vec2 world) {
        if (!std::isfinite(world.x) || !std::isfinite(world.y))
            return;
        for (auto& entry : entries_)
            entry.sprite->SetGeometry({world.x + entry.relative.x, world.y + entry.relative.y},
                                      view_.Zoom());
    }

    void ShipPlacementSprite::SetPriority(unsigned index, float priority) {
        if (!std::isfinite(priority))
            return;
        size_t i = index;
        for (auto& entry : entries_) {
            if (i < entry.sprite->Pieces()) {
                entry.sprite->SetPriority(static_cast<unsigned>(i), priority);
                return;
            }
            i -= entry.sprite->Pieces();
        }
    }

    void ShipPlacementSprite::SetToObject() {
        SetObjectPosition(anchor_);
    }

    void ShipPlacementSprite::Update() {
        SetPositionOfSprites({float(position_.x), float(position_.y)});
        bool valid = OkayToPlace();
        for (auto& entry : entries_)
            entry.sprite->SetColour(255, valid ? 255 : 100, valid ? 255 : 100, 200);
    }

    int ShipPlacementSprite::submit(sys::gfx::GfxManager& gfx,
                                    std::map<std::string, std::shared_ptr<sys::gfx::Image>>& images,
                                    std::map<std::string, sys::res::ResourceAEAnim>& bins,
                                    sys::gfx::AnimationRenderer& renderer,
                                    const std::filesystem::path& root) {
        int count = 0;
        for (auto& entry : entries_)
            count += entry.sprite->submit(gfx, images, bins, renderer, root, entry.raft);
        return count;
    }
} // namespace rp
