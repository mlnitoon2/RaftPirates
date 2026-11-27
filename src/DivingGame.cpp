#include "rp/DivingGame.h"
#include <stdexcept>
#include "sys/gfx/AssetImage.h"
#include <algorithm>
#include <cmath>

namespace rp::diving_game {
    DivingGame::ItemFloater::ItemFloater(CollectableType type,float x,float y):DivingCollectable(type,x,y){}
    DivingGame::ItemFloater::~ItemFloater()=default;
    void DivingGame::ItemFloater::tick(float dt,float speed){
        if(std::isfinite(dt)&&dt>0&&std::isfinite(speed))y-=speed*dt;
    }
    using namespace sys::gfx;

    DivingCollectable::DivingCollectable(CollectableType kind, float px, float py)
        : type(kind), x(px), y(py) {}

    DivingGame::DivingGame(std::filesystem::path assets, unsigned seed)
        : root_(std::move(assets)), renderer_(root_), rng_(seed) {
        atlas_ = std::make_shared<sys::res::ResourceSpriteSheet>(
            sys::res::ResourceSpriteSheet::Create(root_ / "xml_resources/rp_diving.xml"));
        image_ = std::make_shared<Image>(loadAssetImage(root_ / atlas_->imagePath));
        robot_ = sys::res::ResourceAEAnim::Create(root_ / "xml_bin/anim_diving_robot.bin");
        // Draw the continuous rope separately, without copying compositions every frame.
        for(auto& comp:robot_.comps)
            comp.layers.erase(std::remove_if(comp.layers.begin(),comp.layers.end(),[](const auto& layer){return layer.name=="rope";}),comp.layers.end());
        label_.init(std::make_shared<sys::gfx::ResourceSpriteFont>(
                        sys::gfx::ResourceSpriteFont::Create(root_, "rp_font_title")),
                    "", {});
        label_.SetScale(.65f, .65f);
        Reset();
    }

    DivingGame::~DivingGame() = default;

    void DivingGame::Reset() {
        items_.clear();
        scrolling_.clear();
        scrolling_.emplace_back(std::vector<std::string>{"diving_layer003_01","diving_layer003_02"},128,.25f,288);
        scrolling_.emplace_back(std::vector<std::string>{"diving_layer002_01","diving_layer002_02"},128,.5f,288);
        scrolling_.emplace_back(std::vector<std::string>{"diving_layer001_01","diving_layer001_02"},128,1.f,288);
        x_=targetX_=320; y_=144;
        setPos(210,22);
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
        if (!std::isfinite(seconds)) return;
        duration_ = std::clamp(seconds, 1.f, 120.f);
        Reset();
    }

    void DivingGame::InitLevel() {
        speed_ = 95 + 25 * level_;
        for(auto& layer:scrolling_)layer.SetSpeed(speed_);
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
        items_.emplace_back(type, std::clamp(x, 238.f, 402.f), 277);
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
        panelPosition_=panelFrom_=panelTarget_={x,y};
        panelElapsed_=panelDuration_=0;
    }

    void DivingGame::moveTo(float x, float y, float duration) {
        if (!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(duration)) return;
        if(duration<=0){setPos(x,y);return;}
        panelFrom_=panelPosition_;panelTarget_={x,y};panelElapsed_=0;panelDuration_=duration;
        pressed_=false;direction_=0;
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
            targetX_ = std::clamp(p.x,238.f,402.f);
    }

    void DivingGame::updateButtonStates() {
        if (!pressed_)
            direction_ = 0;
    }

    void DivingGame::touchDown(const Vec2& p) {
        if (finished_ || moving() || !std::isfinite(p.x) || !std::isfinite(p.y))
            return;
        pressed_ = true;
        CheckMenuButtonStates(p);
    }

    void DivingGame::touchDrag(const Vec2& p) {
        if (pressed_ && !finished_ && !moving() && std::isfinite(p.x) && std::isfinite(p.y))
            CheckMenuButtonStates(p);
    }

    void DivingGame::touchUp(const Vec2& p) {
        if(moving() || !std::isfinite(p.x) || !std::isfinite(p.y)) return;
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
                targetX_ = std::clamp(p.x,238.f,402.f);
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
        if (!std::isfinite(dt) || dt <= 0) return;
        if(moving()){
            const float step=std::min(dt,panelDuration_-panelElapsed_);
            panelElapsed_+=step;
            const float t=std::min(1.f,panelElapsed_/panelDuration_);
            panelPosition_={panelFrom_.x+(panelTarget_.x-panelFrom_.x)*t,
                            panelFrom_.y+(panelTarget_.y-panelFrom_.y)*t};
            if(!moving())panelPosition_=panelTarget_;
            dt-=step;
        }
        if(finished_ || dt<=0) return;
        dt = std::min(dt, .1f);
        elapsed_ += dt;
        for(auto& layer:scrolling_)layer.tick(dt);
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
            i->tick(dt,speed_);
            if (robotCollidesWithItem(*i)) {
                if (i->type == DivingCollectable::CollectableType::Mine) {
                    SaveYourself();
                    return;
                }
                UpdateGearCount();
                i = items_.erase(i);
            } else if (i->y < 20)
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
        if(target.width!=640||target.height!=400)target=Image(640,400,{9,36,60,255});
        const float x=panelPosition_.x-210,y=panelPosition_.y-22;
        if(std::abs(x)>=640||std::abs(y)>=400)return;
        Image contents;renderContents(contents);
        const int dx=int(std::round(x)),dy=int(std::round(y));
        for(int row=std::max(0,-dy);row<std::min(400,400-dy);++row)
            for(int col=std::max(0,-dx);col<std::min(640,640-dx);++col)
                target.at(col+dx,row+dy)=contents.at(col,row);
    }

    void DivingGame::renderContents(Image& target) {
        target = Image(640, 400, {9, 36, 60, 255});
        sprite(target, "diving_bg", 210, 22, 1);
        {
            GfxManager background;
            background.setScreenClip(std::array<float,4>{219,29,421,277});
            for(const auto& layer:scrolling_)for(const auto& tile:layer.tiles())
                if(auto c=atlas_->findSpriteByName(tile.name)){
                    GfxSprite s;s.cell=*c;s.image=image_;s.SetPos(210,22+tile.y);background.submit(s);
                }
            background.RenderScreen(target);
        }
        {
            GfxManager items;
            items.setScreenClip(std::array<float,4>{219,29,421,277});
            for (auto& i : items_) {
                auto cell = atlas_->findSpriteByName(
                    i.type == DivingCollectable::CollectableType::Mine ? "diving_mine" : "diving_gear");
                if (!cell) continue;
                GfxSprite s; s.cell = *cell; s.image = image_;
                s.SetPos(i.x - cell->ow * .5f, i.y - cell->oh * .5f);
                items.submit(s);
            }
            items.RenderScreen(target);
        }
        if (!robot_.comps.empty()) {
            const auto* composition =
                robot_.find(finished_                                     ? "end"
                            : (std::abs(targetX_ - x_) > 1 || direction_) ? "moving"
                                                                          : "idle");
            const auto& c = composition ? *composition : robot_.comps.front();
            GfxManager m;
            // Robot animation includes the rope; let it reach the inner top/bottom frame edges.
            m.setScreenClip(std::array<float,4>{219,22,421,310});
            // The atlas rope has 32 transparent pixels in its logical height. Tile the
            // visible region end-to-end; the exported pair otherwise leaves a gap.
            if(auto rope=atlas_->findSpriteByName("diving_rope")){
                auto cell=*rope;cell.ox=cell.oy=0;cell.ow=cell.w;cell.oh=cell.h;
                GfxManager ropeGfx;
                ropeGfx.setScreenClip(std::array<float,4>{219,29,421,y_});
                for(float y=29;y<y_;y+=cell.h){
                    GfxSprite s;s.cell=cell;s.image=image_;s.SetPos(x_+3-cell.w*.5f,y);
                    ropeGfx.submit(s);
                }
                ropeGfx.RenderScreen(target);
            }
            renderer_.submit(m, robot_, c, c.duration() > 0 ? std::fmod(elapsed_, c.duration()) : 0,
                             Matrix::translation(x_, y_) *
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
        label_.SetPos(298, 31);
        label_.Render(target);
    }
} // namespace rp::diving_game

namespace rp::diving_game {
DivingMaterial::DivingMaterial(int px, int py, sys::gfx::Gfx* parent)
            : x(static_cast<float>(px)), y(static_cast<float>(py)) {
            sprite.setParent(parent);
            sprite.setHAnchor(sys::gfx::Gfx::HorizontalAnchor::Center);
            sprite.setVAnchor(sys::gfx::Gfx::VerticalAnchor::Center);
            sprite.SetPriority(50);
            sprite.SetPos(x, y);
        }

        void DivingMaterial::configure(std::shared_ptr<const sys::res::ResourceSpriteSheet> atlas,
                                      std::shared_ptr<const sys::gfx::Image> image,
                                      const std::string& cell) {
            if (!atlas || !image || !atlas->findSpriteByName(cell))
                throw std::invalid_argument("Missing diving material atlas cell");
            sprite.sheet = std::move(atlas);
            sprite.SetImage(std::move(image));
            sprite.setSpriteName(cell);
        }
} // namespace rp::diving_game
