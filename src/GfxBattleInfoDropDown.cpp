#include "rp/GfxBattleInfoDropDown.h"
#include "sys/gfx/AssetImage.h"
#include "sys/localization/LocalizationManager.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rp {
    using namespace sys::gfx;

    GfxBattleInfoDropDown::GfxBattleInfoDropDown(OceanView* o, const MsgBattleInfoDropdown& msg)
        : view_(o) {
        // Verified native jump table at 0x3e8680; type 2 and out-of-range have no dropdown.
        switch (msg.type) {
        case 0:
            key_ = "BATTLE_INFO_AUTO_PILOT";
            break;
        case 1:
            key_ = "BATTLE_INFO_RAFT_SWITCHING";
            break;
        case 3:
            key_ = "BATTLE_INFO_UNDER_ATTACK";
            break;
        case 4:
            key_ = "BATTLE_INFO_RETALIATION";
            break;
        case 5:
            key_ = "BATTLE_INFO_WAITING";
            break;
        default:
            return;
        }
        text_ = key_;
        if (o) {
            try {
                sys::localization::LocalizationManager loc;
                loc.loadFile(o->assetRoot() / "text/en.utf8");
                text_ = loc.getText(key_);
            } catch (const std::exception&) {
            }
            try {
                image_ =
                    std::make_shared<Image>(loadAssetImage(o->assetRoot() / "gfx/menu/popup_bg"));
            } catch (const std::exception&) {
            }
            try {
                font_ = std::make_shared<ResourceSpriteFont>(
                    ResourceSpriteFont::Create(o->assetRoot(), "rp_font_title"));
            } catch (const std::exception&) {
            }
        }
        Show();
    }

    GfxBattleInfoDropDown::~GfxBattleInfoDropDown() = default;

    void GfxBattleInfoDropDown::Show() {
        if (key_.empty())
            return;
        from_ = progress_;
        target_ = 1;
        remaining_ = .5f;
    }

    void GfxBattleInfoDropDown::Hide() {
        from_ = progress_;
        target_ = 0;
        remaining_ = .5f;
    }

    void GfxBattleInfoDropDown::aTickMethodThatsNotCalledTick(float dt) {
        if (!std::isfinite(dt) || dt < 0 || remaining_ <= 0)
            return;
        remaining_ = std::max(0.f, remaining_ - dt);
        progress_ = target_ + (from_ - target_) * (remaining_ * 2.f);
    }

    void GfxBattleInfoDropDown::submit(GfxManager& m) const {
        if (progress_ <= 0 || !view_)
            return;
        const float w = std::min(340.f, view_->Width()), h = 50.f, x = (view_->Width() - w) * .5f,
                    y = -h + h * progress_;
        if (image_) {
            GfxSprite s;
            s.image = image_;
            s.cell = {"popup_bg",     0,    0, image_->width, image_->height, 0, 0, image_->width,
                      image_->height, false};
            s.SetScale(w / image_->width, h / image_->height);
            s.SetPos(x, y);
            s.SetPriority(1000);
            m.submit(s);
        }
        if (font_) {
            auto l = font_->layout(text_, .65f, w - 12, false, true, 1);
            font_->submit(m, l, x + (w - l.width) * .5f, y + (h - l.height) * .5f, {}, 1001);
        }
    }
} // namespace rp
