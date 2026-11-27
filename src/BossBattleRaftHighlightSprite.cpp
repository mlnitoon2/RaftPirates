#include "rp/BossBattleRaftHighlightSprite.h"
#include "sys/gfx/AssetImage.h"
#include "sys/localization/LocalizationManager.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rp {
    using namespace sys::gfx;

    BossBattleRaftHighlightSprite::BossBattleRaftHighlightSprite(const OceanView* o, GridSprite* p)
        : OverlayAccessory(o, p, 17) {}

    BossBattleRaftHighlightSprite::~BossBattleRaftHighlightSprite() = default;

    void BossBattleRaftHighlightSprite::Init() {
        overlays_.clear();
        loadSheet("xml_resources/construction_footprints_junk.xml");
        appendCell("item_glow_left", {-16, -40}, 1, .01f);
        appendCell("item_glow_right", {16, -40}, 1, .02f);
        layout();
        SetPriorities();
        pulse(phase_, true);
    }

    // Native Update returns directly. Geometry is refreshed on SetScale by the adapter.
    void BossBattleRaftHighlightSprite::Update() {}

    void BossBattleRaftHighlightSprite::tick(float dt) {
        if (std::isfinite(dt) && dt >= 0) {
            phase_ = std::fmod(phase_ + dt * 6.2831853f, 6.2831853f);
            pulse(phase_, true);
        }
    }
} // namespace rp
