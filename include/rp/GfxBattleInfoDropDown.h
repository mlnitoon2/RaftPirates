#pragma once
#include "rp/Game.h"
#include "rp/Messages.h"
#include "sys/gfx/ResourceSpriteFont.h"

namespace rp {
    class GfxBattleInfoDropDown {
      public:
        GfxBattleInfoDropDown(OceanView*, const MsgBattleInfoDropdown&);
        ~GfxBattleInfoDropDown();
        void Hide();
        void Show();
        void aTickMethodThatsNotCalledTick(float);
        void submit(sys::gfx::GfxManager&) const;

        const std::string& localizationKey() const {
            return key_;
        }

        float progress() const {
            return progress_;
        }

      private:
        OceanView* view_;
        std::string key_, text_;
        std::shared_ptr<const sys::gfx::Image> image_;
        std::shared_ptr<sys::gfx::ResourceSpriteFont> font_;
        float progress_ = 0, from_ = 0, target_ = 0, remaining_ = 0;
    };
} // namespace rp
