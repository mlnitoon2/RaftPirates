#include "rp/Game.h"
#include "rp/GridSprite.h"
#include "rp/PerimeterWallSprite.h"
#include "rp/RaftAttackSprite.h"
#include "rp/WaterBucketSprite.h"
#include "rp/ThrobingIndicatorSprite.h"
#include "rp/UnitHealthBarSprite.h"
#include "rp/GfxOceanGradient.h"
#include "rp/GfxOceanGrid.h"
#include "sys/gfx/AssetImage.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <limits>

namespace rp {
    using namespace sys::gfx;
    using namespace sys::res;

    struct OceanView::VisualEntry {
        GridObject object;
        GridObjectInfo definition;
        Vec2 world;
        std::unique_ptr<GridSprite> sprite;
        std::vector<std::string> animation;
        float animationStart = 0, animationDelay = 0, slack = 0;
        bool customAnimation = false;
        WaterBucketSprite bucket;
        ThrobingIndicatorSprite indicator;
    };

    OceanView::OceanView(std::filesystem::path root)
        : root_(std::move(root)), animations_(root_),
          gradient_(std::make_unique<GfxOceanGradient>(this, root_)) {}

    OceanView::~OceanView() = default;

    void OceanView::Init() {
        sprites_.clear();
        ticking_.clear();
        mutedRafts_.clear();
        diveLoadKey_.reset();
        diveLoad_ = diveCapacity_ = 0;
        raftOpacity_.clear();
        picked_.reset();
        lastDiveParent_ = lastDiveObject_ = 0;
        lastDiving_ = false;
        returnStart_ = -10;
        animationClock_ = 0;
        lastRenderTime_ = -1;
    }

    namespace {
        bool sameCells(const std::vector<Cell>& a, const std::vector<Cell>& b) {
            return a.size() == b.size() &&
                   std::equal(a.begin(), a.end(), b.begin(), [](const Cell& x, const Cell& y) {
                       return x.x == y.x && x.y == y.y;
                   });
        }

        bool sameArt(const GridObjectInfo& a, const GridObjectInfo& b) {
            if (a.id != b.id || a.asset != b.asset || a.offsetX != b.offsetX ||
                a.offsetY != b.offsetY || a.pieces.size() != b.pieces.size() ||
                !sameCells(a.cells, b.cells))
                return false;
            for (size_t i = 0; i < a.pieces.size(); ++i) {
                const auto& x = a.pieces[i];
                const auto& y = b.pieces[i];
                if (x.asset != y.asset || x.offsetX != y.offsetX || x.offsetY != y.offsetY ||
                    !sameCells(x.cells, y.cells))
                    return false;
            }
            return true;
        }
    } // namespace

    void OceanView::appearance(VisualEntry& e, int raft) {
        const auto it = raftOpacity_.find(raft);
        const float alpha = it == raftOpacity_.end() ? 1 : it->second;
        const bool destroyed = e.object.Destroyed();
        auto shade = uint8_t(destroyed ? 150 : 255);
        e.sprite->SetColour(shade, shade, shade,
                            uint8_t(std::round(alpha * (destroyed ? 180 : 255))));
        e.sprite->SetVisible(alpha > 0);
    }

    void OceanView::AddSprite(int parent, const GridObject& object,
                              const GridObjectInfo& definition, Vec2 world) {
        if (object.id <= 0 || parent < 0 || !std::isfinite(world.x) || !std::isfinite(world.y))
            return;
        const SpriteKey key{parent, object.id};
        auto e = std::make_unique<VisualEntry>();
        e->object = object;
        e->definition = definition;
        e->world = world;
        e->sprite = std::make_unique<GridSprite>(*this, e->object, e->definition, world, zoom_);
        e->sprite->setBattlePoints(object.GetBattlePoints());
        if (const auto* c = dynamic_cast<const RaftComponent*>(&object))
            e->sprite->setConstructionTime(c->GetRemainingTime());
        if (e->sprite->constructionTime() > 0)
            e->sprite->AddAccessorySprite(
                std::make_unique<PerimeterWallSprite>(this, e->sprite.get()));
        appearance(*e, parent ? parent : object.id);
        e->sprite->SetMute(RaftMuted(parent ? parent : object.id));
        sprites_[key] = std::move(e);
        AddToTick(parent, object.id);
    }

    void OceanView::UpdateSprite(int parent, const GridObject& object,
                                 const GridObjectInfo& definition, Vec2 world) {
        auto it = sprites_.find({parent, object.id});
        if (it == sprites_.end() || it->second->object.type != object.type ||
            !sameArt(it->second->definition, definition)) {
            AddSprite(parent, object, definition, world);
            return;
        }
        auto& e = *it->second;
        e.object = object;
        e.sprite->setBattlePoints(object.GetBattlePoints());
        e.definition = definition;
        e.world = world;
        if (const auto* c = dynamic_cast<const RaftComponent*>(&object))
            e.sprite->setConstructionTime(c->GetRemainingTime());
        else
            e.sprite->setConstructionTime(0);
        if (e.sprite->constructionTime() > 0 && !e.sprite->HasAccessorySprite(1))
            e.sprite->AddAccessorySprite(
                std::make_unique<PerimeterWallSprite>(this, e.sprite.get()));
        if (e.sprite->constructionTime() > 0)
            if (auto* a = e.sprite->GetAccessorySprite(1))
                a->AddToTick();
        e.sprite->SetGeometry(world, zoom_);
        appearance(e, parent ? parent : object.id);
    }

    void OceanView::RemoveSprite(int parent, int id) {
        const SpriteKey key{parent, id};
        if (picked_ == key)
            picked_.reset();
        RemoveFromTick(parent, id);
        if (diveLoadKey_ == key)
            diveLoadKey_.reset();
        sprites_.erase(key);
    }

    void OceanView::AddGrid(const Raft& r, const std::map<int, GridObjectInfo>& defs) {
        if (auto d = defs.find(r.type); d != defs.end())
            UpdateSprite(0, r, d->second, {float(r.x), float(r.y)});
        for (const auto& c : r.components)
            if (auto d = defs.find(c.type); d != defs.end())
                UpdateSprite(r.id, c, d->second,
                             {float(r.x) + float(c.x) / 6 + 1.f / 12,
                              float(r.y) + float(c.y) / 6 + 1.f / 12});
    }

    void OceanView::RemoveGrid(int raft) {
        RemoveSprite(0, raft);
        for (auto it = sprites_.begin(); it != sprites_.end();) {
            if (it->first.first == raft) {
                if (picked_ == it->first)
                    picked_.reset();
                ticking_.erase(it->first);
                if (diveLoadKey_ == it->first)
                    diveLoadKey_.reset();
                it = sprites_.erase(it);
            } else
                ++it;
        }
        raftOpacity_.erase(raft);
        mutedRafts_.erase(raft);
    }

    void OceanView::sync(const Player& game) {
        std::set<SpriteKey> present;
        std::set<int> rafts;
        for (const auto& r : game.rafts) {
            rafts.insert(r.id);
            AddGrid(r, game.objects);
            if (game.objects.count(r.type))
                present.emplace(0, r.id);
            for (const auto& c : r.components)
                if (game.objects.count(c.type))
                    present.emplace(r.id, c.id);
        }
        for (auto it = sprites_.begin(); it != sprites_.end();) {
            if (!present.count(it->first)) {
                if (picked_ == it->first)
                    picked_.reset();
                ticking_.erase(it->first);
                if (diveLoadKey_ == it->first)
                    diveLoadKey_.reset();
                it = sprites_.erase(it);
            } else
                ++it;
        }
        for (auto it = mutedRafts_.begin(); it != mutedRafts_.end();)
            if (!rafts.count(*it))
                it = mutedRafts_.erase(it);
            else
                ++it;
        for (auto it = raftOpacity_.begin(); it != raftOpacity_.end();)
            if (!rafts.count(it->first))
                it = raftOpacity_.erase(it);
            else
                ++it;
    }

    void OceanView::SetOcean(const Player& game) {
        Init();
        sync(game);
    }

    GridSprite* OceanView::getGridSprite(int parent, int id) const {
        auto it = sprites_.find({parent, id});
        return it == sprites_.end() ? nullptr : it->second->sprite.get();
    }

    size_t OceanView::spriteCount() const {
        return sprites_.size();
    }

    void OceanView::SetRaftTransparent(int raft, float alpha) {
        if (!std::isfinite(alpha) || raft <= 0)
            return;
        raftOpacity_[raft] = std::clamp(alpha, 0.f, 1.f);
        for (auto& [key, e] : sprites_)
            if (key.first == raft || (key.first == 0 && key.second == raft))
                appearance(*e, raft);
    }

    void OceanView::AddAnimSlack(int parent, int id, float seconds) {
        auto it = sprites_.find({parent, id});
        if (it != sprites_.end() && std::isfinite(seconds) && seconds > 0)
            it->second->slack = std::min(3600.f, it->second->slack + seconds);
    }

    void OceanView::UpdateAnimations(int parent, int id, float seconds) {
        if (auto sprite = getGridSprite(parent, id))
            sprite->UpdateAnimations(seconds);
    }

    void OceanView::gotMsgAnimateGridSprite(int parent, int id, std::vector<std::string> names) {
        auto it = sprites_.find({parent, id});
        if (it == sprites_.end())
            return;
        auto& e = *it->second;
        e.animation = std::move(names);
        e.animationStart = animationClock_;
        e.animationDelay = 0;
        e.customAnimation = !e.animation.empty();
        e.sprite->SetAnimation(e.animation);
    }

    void OceanView::PlayRaftAttackAnims(int raft, float delay) {
        if (!std::isfinite(delay) || delay < 0)
            return;
        if (auto* sprite = getGridSprite(0, raft)) {
            try {
                auto resource = std::make_shared<sys::res::ResourceAEAnim>(
                    sys::res::ResourceAEAnim::Create(root_ / "xml_bin/raft_cannons_anim.bin",
                                                     sys::res::AEFormat::RaftPirates2013));
                sprite->RemoveAccessorySprite(15);
                sprite->AddAccessorySprite(
                    std::make_unique<RaftAttackSprite>(this, sprite, resource, delay));
            } catch (const std::exception&) {
            }
        }
        for (auto& [key, e] : sprites_) {
            if (key.first != raft && !(key.first == 0 && key.second == raft))
                continue;
            if (!e->sprite->HasAttack())
                continue;
            gotMsgAnimateGridSprite(key.first, key.second, {"attack", "Attack"});
            e->animationDelay = delay;
        }
    }

    void OceanView::gotMsgSparseGridObjectAdded(int parent, const GridObject& object,
                                                const GridObjectInfo& def, Vec2 world) {
        UpdateSprite(parent, object, def, world);
    }

    void OceanView::gotMsgSparseGridObjectRemoved(int parent, int id) {
        if (parent == 0)
            RemoveGrid(id);
        else
            RemoveSprite(parent, id);
    }

    void OceanView::gotMsgGridObjectEvent(int parent, const GridObject& object,
                                          const GridObjectInfo& def, Vec2 world) {
        UpdateSprite(parent, object, def, world);
    }

    void OceanView::gotMsgUpdateOceanView(const Player& game) {
        sync(game);
    }

    void OceanView::SetOrdering() {
        for (auto& [key, e] : sprites_)
            e->sprite->SetPriorities();
    }

    void OceanView::SetTempPriorities(GridSprite& sprite) {
        sprite.SetPriorities();
    }

    bool OceanView::GetPriority(const GridSprite* sprite, unsigned piece, float& result) const {
        if (!sprite || piece >= sprite->Pieces())
            return false;
        result = sprite->GetPriority(piece);
        return true;
    }

    float OceanView::GetSpritePriority(const GridSprite* sprite, unsigned piece) const {
        float result = priority_;
        GetPriority(sprite, piece, result);
        return result;
    }

    std::vector<OceanView::SpriteKey> OceanView::FillPrioritySorter() const {
        std::vector<SpriteKey> out;
        for (const auto& [key, e] : sprites_)
            out.push_back(key);
        std::stable_sort(out.begin(), out.end(), [&](const SpriteKey& a, const SpriteKey& b) {
            return GetSpritePriority(sprites_.at(a)->sprite.get(), 0) <
                   GetSpritePriority(sprites_.at(b)->sprite.get(), 0);
        });
        return out;
    }

    Vec2 OceanView::FocalPoint() const {
        return ScreenToWorld({width_ / 2, height_ / 2});
    }

    void OceanView::SetAxiseseses(Vec2 x, Vec2 y) {
        const double determinant = double(x.x) * y.y - double(y.x) * x.y;
        if (!std::isfinite(determinant) || std::abs(determinant) < 1e-6)
            return;
        axisX_ = x;
        axisY_ = y;
        customAxes_ = true;
    }

    void OceanView::showHealthBars(bool show) {
        healthBars_ = show;
    }

    void OceanView::removeAllHealthBars() {
        healthBars_ = false;
    }

    void OceanView::ShowWaterBuckets(bool show) {
        waterBuckets_ = show;
    }

    bool OceanView::CheckForBucket(int parent, int id) const {
        auto it = sprites_.find({parent, id});
        return waterBuckets_ && parent > 0 && it != sprites_.end() &&
               it->second->definition.bp > 0 &&
               it->second->object.GetHealth(it->second->definition.bp) < 1;
    }

    void OceanView::AddToTick(int parent, int id) {
        if (sprites_.count({parent, id}))
            ticking_.emplace(parent, id);
    }

    void OceanView::RemoveFromTick(int parent, int id) {
        ticking_.erase({parent, id});
    }

    unsigned OceanView::Layer() const {
        return layer_;
    }

    void OceanView::SetLayer(unsigned layer) {
        layer_ = layer;
    }

    void OceanView::SetBossOcean(bool enabled) {
        bossOcean_ = enabled;
    }

    void OceanView::SetRaftMute(int raft, bool mute) {
        if (raft <= 0)
            return;
        if (mute)
            mutedRafts_.insert(raft);
        else
            mutedRafts_.erase(raft);
        for (auto& item : sprites_)
            if (item.first.first == raft || (!item.first.first && item.first.second == raft))
                item.second->sprite->SetMute(mute);
    }

    bool OceanView::RaftMuted(int raft) const {
        return mutedRafts_.count(raft) != 0;
    }

    void OceanView::gotMsgDiveUnitLoadChange(int parent, int id, int load, int maximum) {
        if (maximum <= 0 || !getGridSprite(parent, id)) {
            diveLoadKey_.reset();
            diveLoad_ = diveCapacity_ = 0;
            return;
        }
        diveLoadKey_ = SpriteKey{parent, id};
        diveCapacity_ = maximum;
        diveLoad_ = std::clamp(load, 0, maximum);
    }

    void OceanView::tick(float dt) {
        if (!std::isfinite(dt) || dt < 0)
            return;
        animationClock_ += dt;
        gradient_->tick(dt);
        for (const auto& key : ticking_) {
            auto& e = *sprites_.at(key);
            e.slack = std::max(0.f, e.slack - dt);
            e.sprite->UpdateAnimations(dt);
            e.bucket.tick(dt);
            e.indicator.tick(dt);
        }
    }

    bool OceanView::gotMsgGfxLayerPickResult(int parent, int id) {
        const SpriteKey key{parent, id};
        auto sprite = getGridSprite(parent, id);
        if (!sprite || !sprite->GetVisible()) {
            picked_.reset();
            return false;
        }
        picked_ = key;
        return true;
    }

    std::optional<OceanView::SpriteKey> OceanView::pickedSprite() const {
        return picked_;
    }

    void OceanView::SetPosition(Vec2 p) {
        position_ = p;
    }

    void OceanView::SetWidth(float w) {
        if (w > 0 && std::isfinite(w))
            width_ = w;
    }

    void OceanView::SetHeight(float h) {
        if (h > 0 && std::isfinite(h))
            height_ = h;
    }

    void OceanView::SetPriority(float p) {
        priority_ = p;
    }

    void OceanView::SetUnitSize(float size) {
        if (size > 0 && std::isfinite(size)) {
            unit_ = size;
            customAxes_ = false;
            CalculateHelpers();
        }
    }

    Vec2 OceanView::Position() const {
        return position_;
    }

    float OceanView::Width() const {
        return width_;
    }

    float OceanView::Height() const {
        return height_;
    }

    float OceanView::Priority() const {
        return priority_;
    }

    float OceanView::Unit() const {
        return unit_;
    }

    Vec2 OceanView::AxisX() const {
        return axisX_;
    }

    Vec2 OceanView::AxisY() const {
        return axisY_;
    }

    Vec2 OceanView::ConvertToViewSpecial(Vec2 w) const {
        return {192 * (w.x - w.y), 96 * (w.x + w.y)};
    }

    Vec2 OceanView::ConvertToView(Vec2 w) const {
        return {w.x * axisX_.x + w.y * axisY_.x, w.x * axisX_.y + w.y * axisY_.y};
    }

    Vec2 OceanView::ConvertToWorld(Vec2 v) const {
        const double det = double(axisX_.x) * axisY_.y - double(axisY_.x) * axisX_.y;
        return {float((v.x * double(axisY_.y) - v.y * double(axisY_.x)) / det),
                float((axisX_.x * double(v.y) - axisX_.y * double(v.x)) / det)};
    }

    Vec2 OceanView::WorldToScreen(Vec2 w) const {
        auto v = ConvertToView({w.x - offset_.x, w.y - offset_.y});
        return {position_.x + v.x * zoom_, position_.y + v.y * zoom_};
    }

    Vec2 OceanView::ScreenToWorld(Vec2 s) const {
        auto w = ConvertToWorld({(s.x - position_.x) / zoom_, (s.y - position_.y) / zoom_});
        return {w.x + offset_.x, w.y + offset_.y};
    }

    void OceanView::SetZoom(float z) {
        if (z > 0 && std::isfinite(z))
            zoom_ = z;
    }

    float OceanView::Zoom() const {
        return zoom_;
    }

    void OceanView::SetOffset(Vec2 o, bool) {
        offset_ = o;
    }

    Vec2 OceanView::Offset() const {
        return offset_;
    }

    void OceanView::CalculateHelpers() {
        if (customAxes_)
            return;
        axisX_ = {unit_, unit_ / 2};
        axisY_ = {-unit_, unit_ / 2};
    }

    void OceanView::Update() {
        CalculateHelpers();
        for (auto& [key, e] : sprites_)
            e->sprite->SetGeometry(e->world, zoom_);
    }

    bool OceanView::Pick(const Player& game, int parent, int object, Vec2 screen, float time) {
        if (screen.x < 0 || screen.y < 0 || screen.x >= 640 || screen.y >= 400)
            return false;
        sync(game);
        auto raft = game.GetRaftByID(parent);
        if (!raft)
            return false;
        for (const auto& c : raft->components) {
            if (c.id != object)
                continue;
            auto def = game.objects.find(c.type);
            if (def == game.objects.end())
                return false;
            auto sprite = getGridSprite(parent, object);
            if (!sprite || !sprite->GetVisible())
                return false;
            Image mask(640, 400, {0, 0, 0, 0});
            GfxManager mgr;
            sprite->submit(mgr, images_, bins_, animations_, root_, false);
            mgr.RenderScreen(mask);
            if (mask.at(int(screen.x), int(screen.y)).a > 32)
                return gotMsgGfxLayerPickResult(parent, object);
            auto world = ScreenToWorld(screen);
            int cx = int(std::floor((world.x - raft->x) * 6)),
                cy = int(std::floor((world.y - raft->y) * 6));
            for (auto cell : def->second.cells)
                if (cx == c.x + cell.x && cy == c.y + cell.y)
                    return gotMsgGfxLayerPickResult(parent, object);
        }
        return false;
    }

    Image OceanView::render(const Player& game, float time, bool grid, const DiveState* dive) {
        sync(game);
        if (std::isfinite(time)) {
            if (lastRenderTime_ >= 0 && time >= lastRenderTime_)
                tick(time - lastRenderTime_);
            lastRenderTime_ = time;
        }
        auto animate = [&](VisualEntry& e, float fallback, const std::vector<std::string>& names) {
            if (e.customAnimation && animationClock_ >= e.animationStart + e.animationDelay &&
                animationClock_ - e.animationStart - e.animationDelay <= 3.f) {
                e.sprite->SetAnimation(e.animation);
                e.sprite->SetTime(
                    std::max(0.f, animationClock_ - e.animationStart - e.animationDelay - e.slack));
            } else {
                if (e.customAnimation &&
                    animationClock_ - e.animationStart - e.animationDelay > 3.f)
                    e.customAnimation = false;
                e.sprite->SetAnimation(names);
                e.sprite->SetTime(std::max(0.f, fallback - e.slack));
            }
        };
        const bool diving = dive && dive->Diving();
        const bool changed =
            diving && (lastDiveParent_ != dive->parent || lastDiveObject_ != dive->object);
        if (lastDiving_ && (!diving || changed)) {
            if (auto* previous = getGridSprite(lastDiveParent_, lastDiveObject_)) {
                previous->RemoveAccessorySprite(2);
                previous->RemoveAccessorySprite(8);
            }
            diveLoadKey_.reset();
        }
        if (lastDiving_ && !diving)
            returnStart_ = time;
        if (diving) {
            lastDiveParent_ = dive->parent;
            lastDiveObject_ = dive->object;
        }
        lastDiving_ = diving;
        if (diving)
            dive->AddedToOcean(*this);
        else {
            diveLoadKey_.reset();
            if (auto* sprite = getGridSprite(lastDiveParent_, lastDiveObject_)) {
                sprite->RemoveAccessorySprite(2);
                sprite->RemoveAccessorySprite(8);
            }
        }
        Image out(640, 400);
        gradient_->SetZone(bossOcean_  ? GfxOceanGradient::GradientZone::Boss
                           : game.safe ? GfxOceanGradient::GradientZone::Safe
                                       : GfxOceanGradient::GradientZone::Open);
        gradient_->Render(out);
        GfxManager mgr;
        mgr.setLayerOffset(Layer());
        auto base = [this](const Raft& r) { return WorldToScreen({float(r.x), float(r.y)}); };
        for (const auto& r : game.rafts) {
            auto it = game.objects.find(r.type);
            if (it == game.objects.end())
                continue;
            auto& e = *sprites_.at({0, r.id});
            animate(e, time, {});
            e.sprite->submit(mgr, images_, bins_, animations_, root_, true);
        }
        mgr.RenderScreen(out);
        for (auto& r : game.rafts) {
            auto b = base(r);
            if (grid) {
                GfxOceanGrid overlay(this, Unit());
                overlay.SetPosition(b);
                overlay.Render(out);
            }
            for (auto& c : r.components) {
                auto it = game.objects.find(c.type);
                if (it == game.objects.end())
                    continue;
                auto& e = *sprites_.at({r.id, c.id});
                std::vector<std::string> names;
                float animTime = std::max(0.f, time);
                if (diving && dive->parent == r.id && dive->object == c.id) {
                    auto pose = dive->PlayAnimations();
                    names = std::move(pose.first);
                    animTime = pose.second;
                } else if (r.id == lastDiveParent_ && c.id == lastDiveObject_ &&
                           time - returnStart_ < .6833f && time >= returnStart_) {
                    names = {"return"};
                    animTime = time - returnStart_;
                }
                animate(e, animTime, names);
                e.sprite->submit(mgr, images_, bins_, animations_, root_, false);
            }
        }
        mgr.RenderScreen(out);
        if (healthBars_) {
            try {
                auto load = [&](const std::string& path) {
                    auto it = images_.find(path);
                    if (it == images_.end())
                        it = images_
                                 .emplace(path,
                                          std::make_shared<Image>(loadAssetImage(root_ / path)))
                                 .first;
                    return it->second;
                };
                UnitHealthBarSprite bar(*this, load("gfx/HUD/health_meter.png"),
                                        load("gfx/HUD/health_meter_green.pvr"));
                for (const auto& [key, e] : sprites_) {
                    if (!e->sprite->GetVisible() || e->definition.bp <= 0)
                        continue;
                    bar.Update(e->world, e->object.GetHealth(e->definition.bp));
                    bar.Render(out);
                }
            } catch (const std::exception&) { /* Original health-meter assets unavailable. */
            }
        }
        if (waterBuckets_ || picked_) {
            try {
                const std::string key = "xml_resources/rp_menu.xml";
                if (!overlaySheet_)
                    overlaySheet_ = std::make_shared<ResourceSpriteSheet>(
                        ResourceSpriteSheet::Create(root_ / key));
                auto image = images_.find(overlaySheet_->imagePath);
                if (image == images_.end())
                    image = images_
                                .emplace(overlaySheet_->imagePath,
                                         std::make_shared<Image>(
                                             loadAssetImage(root_ / overlaySheet_->imagePath)))
                                .first;
                GfxManager overlays;
                auto draw = [&](const VisualEntry& entry, const char* name, float scale,
                                float offset) {
                    if (!entry.sprite->GetVisible())
                        return;
                    auto cell = overlaySheet_->findSpriteByName(name);
                    if (!cell)
                        return;
                    const auto point = WorldToScreen(entry.world);
                    GfxSprite s;
                    s.image = image->second;
                    s.cell = *cell;
                    s.SetPos(point.x - cell->ow * scale * .5f,
                             point.y + offset - cell->oh * scale * .5f);
                    s.SetScale(scale, scale);
                    overlays.submit(s);
                };
                for (const auto& entry : sprites_) {
                    const auto& k = entry.first;
                    const auto& visual = *entry.second;
                    if (CheckForBucket(k.first, k.second))
                        draw(visual, "btn_water", visual.bucket.scale(), -34);
                    if (picked_ && *picked_ == k)
                        draw(visual, "battle_slider_arrow", 1, visual.indicator.offset());
                }
                overlays.RenderScreen(out);
            } catch (const std::exception&) {
            }
        }
        if (diveLoadKey_ && diveCapacity_ > 0) {
            const auto& entry = *sprites_.at(*diveLoadKey_);
            const auto p = WorldToScreen(entry.world);
            if (entry.sprite->GetVisible() && std::isfinite(p.x) && std::isfinite(p.y) &&
                p.x >= -30 && p.x < out.width + 30 && p.y >= -30 && p.y < out.height + 30) {
                const int fill = int(30.0 * diveLoad_ / diveCapacity_);
                for (int y = 0; y < 4; ++y)
                    for (int x = 0; x < 30; ++x) {
                        const int px = int(p.x) - 15 + x, py = int(p.y) - 34 + y;
                        if (px >= 0 && px < out.width && py >= 0 && py < out.height)
                            out.at(px, py) =
                                x < fill ? Color{240, 203, 78, 255} : Color{40, 40, 40, 255};
                    }
            }
        }
        return out;
    }
} // namespace rp
