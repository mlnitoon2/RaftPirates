#pragma once
#include "sys/gfx/Gfx.h"
namespace rp {
class OceanView;
class GfxOceanCheckerBoard {
public:
    GfxOceanCheckerBoard(const OceanView*, float);
    ~GfxOceanCheckerBoard();
    float FloatMod(float,float) const;
    void Render(sys::gfx::Image&) const;
    void SetPosition(sys::gfx::Vec2);
    void SetUnitSize(float);
private:
    const OceanView* view_;
    sys::gfx::Vec2 position_{};
    float unit_=192;
};
}
