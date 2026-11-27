#include "rp/OverlayAccessory.h"
#include "sys/gfx/AssetImage.h"
#include "sys/localization/LocalizationManager.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rp {
    using namespace sys::gfx;

    bool OverlayAccessory::loadSheet(const char* path) {
        if (!ocean_)
            return false;
        try {
            sheet_ = std::make_shared<sys::res::ResourceSpriteSheet>(
                sys::res::ResourceSpriteSheet::Create(ocean_->assetRoot() / path));
            atlas_ =
                std::make_shared<Image>(loadAssetImage(ocean_->assetRoot() / sheet_->imagePath));
            return true;
        } catch (const std::exception&) {
            sheet_.reset();
            atlas_.reset();
            return false;
        }
    }

    bool OverlayAccessory::appendCell(const char* name, Vec2 offset, float size, float priority) {
        if (!sheet_ || !atlas_)
            return false;
        auto* cell = sheet_->findSpriteByName(name);
        if (!cell)
            return false;
        Piece p;
        p.sprite.image = atlas_;
        p.sprite.cell = *cell;
        p.offset = offset;
        p.size = size;
        p.priorityOffset = priority;
        overlays_.push_back(std::move(p));
        return true;
    }

    bool OverlayAccessory::appendImage(const char* path, Vec2 offset, float size, float priority) {
        if (!ocean_)
            return false;
        try {
            Piece p;
            auto image = std::make_shared<Image>(loadAssetImage(ocean_->assetRoot() / path));
            p.sprite.image = image;
            p.sprite.cell = {
                path, 0, 0, image->width, image->height, 0, 0, image->width, image->height, false};
            p.offset = offset;
            p.size = size;
            p.priorityOffset = priority;
            overlays_.push_back(std::move(p));
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }

    bool OverlayAccessory::changeCell(unsigned i, const char* name) {
        if (i >= overlays_.size() || !sheet_)
            return false;
        auto* cell = sheet_->findSpriteByName(name);
        if (!cell)
            return false;
        overlays_[i].sprite.cell = *cell;
        return true;
    }

    float OverlayAccessory::basePriority() const {
        return parent_ && parent_->Pieces() ? parent_->GetPriority(0) : 0;
    }

    void OverlayAccessory::layout() {
        if (!ocean_ || !parent_)
            return;
        auto p = ocean_->WorldToScreen(parent_->worldPosition());
        for (auto& a : overlays_) {
            const float s = scale_ * a.size;
            a.sprite.SetScale(s, s);
            a.sprite.SetPos(p.x + a.offset.x * scale_ - a.sprite.cell.ow * s * .5f,
                            p.y + a.offset.y * scale_ - a.sprite.cell.oh * s * .5f);
            a.sprite.SetVisible(visible_);
        }
    }

    void OverlayAccessory::SetScale(float s) {
        if (std::isfinite(s) && s > 0) {
            scale_ = s;
            layout();
        }
    }

    void OverlayAccessory::SetVisible(bool v) {
        visible_ = v;
        for (auto& p : overlays_)
            p.sprite.SetVisible(v);
    }

    void OverlayAccessory::SetColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        for (auto& p : overlays_)
            p.sprite.SetColor(r, g, b, a);
    }

    void OverlayAccessory::SetPriorities() {
        for (unsigned i = 0; i < overlays_.size(); ++i)
            SetPriority(i, basePriority() + overlays_[i].priorityOffset);
    }

    void OverlayAccessory::SetPriority(unsigned i, float p) {
        if (i < overlays_.size() && std::isfinite(p))
            overlays_[i].sprite.SetPriority(p);
    }

    bool OverlayAccessory::ContainsGfx(const Gfx* g) const {
        if (!g)
            return false;
        for (const auto& p : overlays_)
            if (g == &p.sprite)
                return true;
        return false;
    }

    int OverlayAccessory::submit(GfxManager& mgr, AnimationRenderer&) const {
        if (!visible_)
            return 0;
        int n = 0;
        for (const auto& p : overlays_)
            if (p.sprite.image && p.sprite.visible) {
                mgr.submit(p.sprite);
                ++n;
            }
        return n;
    }

    void OverlayAccessory::pulse(float phase, bool red) {
        const auto alpha = uint8_t(255 - uint8_t(63.75f + std::sin(phase) * 63.75f));
        SetColour(255, red ? 128 : 255, red ? 128 : 255, alpha);
    }
} // namespace rp
