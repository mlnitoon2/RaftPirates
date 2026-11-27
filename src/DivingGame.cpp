#include "rp/DivingGame.h"
#include "sys/gfx/AssetImage.h"
#include <algorithm>
#include <cmath>

namespace rp::diving_game {
    using namespace sys::gfx;

    DivingCollectable::DivingCollectable(CollectableType kind, float px, float py)
        : type(kind), x(px), y(py) {}

    DivingGame::DivingGame(std::filesystem::path assets, unsigned seed)
        : root_(std::move(assets)), renderer_(root_), rng_(seed) {
        atlas_ = std::make_shared<sys::res::ResourceSpriteSheet>(
            sys::res::ResourceSpriteSheet::Create(root_ / "xml_resources/rp_diving.xml"));
        image_ = std::make_shared<Image>(loadAssetImage(root_ / atlas_->imagePath));
        robot_ = sys::res::ResourceAEAnim::Create(root_ / "xml_bin/anim_diving_robot.bin");
        label_.init(std::make_shared<sys::gfx::ResourceSpriteFont>(
                        sys::gfx::ResourceSpriteFont::Create(root_, "rp_font_title")),
                    "", {});
        label_.SetScale(.65f, .65f);
        Reset();
    }

    DivingGame::~DivingGame() = default;

    void DivingGame::Reset() {
        items_.clear();
        setPos(320,270);
        gears_ = 0;
        level_ = 1;
        shape_ = 0;
        elapsed_ = 0;
        remaining_ = duration_;
        finished_ = confirmed_ = pressed_ = false;
        direction_ = 0;
        InitLevel();
    }

    void DivingGame::setDuration(float seconds) {
        duration_ = std::clamp(seconds, 1.f, 120.f);
        Reset();
    }

    void DivingGame::InitLevel() {
        speed_ = 95 + 25 * level_;
        spawn_ = 0.3f;
    }

    void DivingGame::NextLevel() {
        ++level_;
        InitLevel();
    }

    void DivingGame::NextShape() {
        ++shape_;
        spawnItem(DivingCollectable::CollectableType::Gear, GetNextGearX());
        if (shape_ % 3 == 0)
            spawnItem(DivingCollectable::CollectableType::Mine, GetNextMine());
    }

    float DivingGame::GetNextGearX() {
        return std::uniform_real_distribution<float>(238, 402)(rng_);
    }

    float DivingGame::GetNextMine() {
        return std::uniform_real_distribution<float>(238, 402)(rng_);
    }

    void DivingGame::spawnItem(DivingCollectable::CollectableType type, float x) {
        items_.emplace_back(type, std::clamp(x, 238.f, 402.f), 40);
    }

    bool DivingGame::robotCollidesWithItem(const DivingCollectable& item) const {
        return std::abs(item.x - x_) < 18 && std::abs(item.y - y_) < 21;
    }

    void DivingGame::UpdateGearCount() {
        ++gears_;
    }

    void DivingGame::SaveYourself() {
        finished_ = true;
        pressed_ = false;
        direction_ = 0;
    }

    void DivingGame::ConfirmDiveFinish() {
        if (finished_)
            confirmed_ = true;
    }

    void DivingGame::moveDiver(DiverMoveDirection direction, float seconds) {
        targetX_ = std::clamp(targetX_ + (direction == DiverMoveDirection::Left ? -1.f : 1.f) *
                                             180 * seconds,
                              238.f, 402.f);
    }

    void DivingGame::setPos(float x,float y) {
        if (!std::isfinite(x)||!std::isfinite(y))return;
        x_=targetX_=std::clamp(x,238.f,402.f);
        y_=std::clamp(y,180.f,285.f);
    }

    void DivingGame::moveTo(float x, float y, float) {
        targetX_ = std::clamp(x, 238.f, 402.f);
        y_ = std::clamp(y, 180.f, 285.f);
    }

    bool DivingGame::wasLeftButtonTouched(const Vec2& p) const {
        return p.y >= 340 && p.x < 300;
    }

    bool DivingGame::wasRightButtonTouched(const Vec2& p) const {
        return p.y >= 340 && p.x > 340;
    }

    bool DivingGame::wasExitButtonTouched(const Vec2& p) const {
        return p.y >= 340 && p.x >= 300 && p.x <= 340;
    }

    void DivingGame::CheckMenuButtonStates(const Vec2& p) {
        direction_ = wasLeftButtonTouched(p) ? -1 : wasRightButtonTouched(p) ? 1 : 0;
        if (!direction_ && p.y < 340)
            moveTo(p.x, y_, .15f);
    }

    void DivingGame::updateButtonStates() {
        if (!pressed_)
            direction_ = 0;
    }

    void DivingGame::touchDown(const Vec2& p) {
        if (finished_)
            return;
        pressed_ = true;
        CheckMenuButtonStates(p);
    }

    void DivingGame::touchDrag(const Vec2& p) {
        if (pressed_ && !finished_)
            CheckMenuButtonStates(p);
    }

    void DivingGame::touchUp(const Vec2& p) {
        if (finished_) {
            ConfirmDiveFinish();
            return;
        }
        if (wasExitButtonTouched(p))
            SaveYourself();
        else if (!pressed_) {
            if (wasLeftButtonTouched(p))
                moveDiver(DiverMoveDirection::Left, .25f);
            else if (wasRightButtonTouched(p))
                moveDiver(DiverMoveDirection::Right, .25f);
            else
                moveTo(p.x, y_, .15f);
        }
        pressed_ = false;
        updateButtonStates();
    }

    void DivingGame::touchCancel(const Vec2&) {
        pressed_ = false;
        updateButtonStates();
    }

    void DivingGame::AutoScaleText(GfxText* text) {
        if (!text)
            return;
        GfxText::Format format;
        format.maxWidth = 120;
        format.maxHeight = 40;
        format.autoScale = true;
        text->setFormat(format);
    }

    void DivingGame::onMsgTouchState(const sys::msg::MsgTouchState& msg) {
        if (msg.points.empty()) {
            if (pressed_)
                touchCancel({});
            return;
        }
        const auto& point = msg.points.front();
        if (pressed_)
            touchDrag({point.first, point.second});
        else
            touchDown({point.first, point.second});
    }

    void DivingGame::gotMsgUpdate(const sys::msg::MsgUpdate& msg) {
        tick(msg.dt);
    }

    void DivingGame::tick(float dt) {
        if (finished_ || !std::isfinite(dt) || dt <= 0)
            return;
        dt = std::min(dt, .1f);
        elapsed_ += dt;
        remaining_ = std::max(0.f, remaining_ - dt);
        if (direction_)
            moveDiver(direction_ < 0 ? DiverMoveDirection::Left : DiverMoveDirection::Right, dt);
        x_ += std::clamp(targetX_ - x_, -240 * dt, 240 * dt);
        if (int(elapsed_ / 8) + 1 > level_)
            NextLevel();
        spawn_ -= dt;
        if (spawn_ <= 0) {
            NextShape();
            spawn_ += .65f;
        }
        for (auto i = items_.begin(); i != items_.end();) {
            i->y += speed_ * dt;
            if (robotCollidesWithItem(*i)) {
                if (i->type == DivingCollectable::CollectableType::Mine) {
                    SaveYourself();
                    return;
                }
                UpdateGearCount();
                i = items_.erase(i);
            } else if (i->y > 335)
                i = items_.erase(i);
            else
                ++i;
        }
        if (remaining_ <= 0)
            SaveYourself();
    }

    void DivingGame::sprite(Image& target, const char* name, float x, float y, float scale,
                            bool flip) {
        auto cell = atlas_->findSpriteByName(name);
        if (!cell)
            return;
        GfxSprite s;
        s.cell = *cell;
        s.image = image_;
        s.SetPos(x, y);
        s.SetScale(scale, scale);
        s.setHFlip(flip);
        drawSprite(target, s);
    }

    void DivingGame::render(Image& target) {
        target = Image(640, 400, {9, 36, 60, 255});
        sprite(target, "diving_bg", 210, 22, 1);
        for (auto& i : items_)
            sprite(target,
                   i.type == DivingCollectable::CollectableType::Mine ? "diving_mine"
                                                                      : "diving_gear",
                   i.x - 10, i.y - 10);
        if (!robot_.comps.empty()) {
            const auto* composition =
                robot_.find(finished_                                     ? "end"
                            : (std::abs(targetX_ - x_) > 1 || direction_) ? "moving"
                                                                          : "idle");
            const auto& c = composition ? *composition : robot_.comps.front();
            GfxManager m;
            renderer_.submit(m, robot_, c, c.duration() > 0 ? std::fmod(elapsed_, c.duration()) : 0,
                             Matrix::translation(x_, y_) * Matrix::scale(.65f, .65f) *
                                 Matrix::translation(-c.width / 2.f, -c.height / 2.f));
            m.RenderScreen(target);
        }
        for (auto name :
             {"diving_frame_top", "diving_frame_bottom", "diving_frame_left", "diving_frame_right"})
            sprite(target, name, 210, 22);
        sprite(target, "diving_arrow", 242, 350, 1.4f, true);
        sprite(target, "diving_arrow", 365, 350, 1.4f);
        sprite(target, "diving_gear", 307, 350, 1.2f);
        label_.changeText(std::to_string(gears_));
        AutoScaleText(&label_);
        label_.SetPos(20, 50);
        label_.Render(target);
    }
} // namespace rp::diving_game
