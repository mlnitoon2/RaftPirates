#pragma once
#include "rp/OverlayAccessory.h"
namespace rp {
class SelectedWallSprite : public OverlayAccessory {
public:
    SelectedWallSprite(const OceanView*,GridSprite*);
    ~SelectedWallSprite() override;
    void Init() override; void SetPriorities() override; void SetPriority(unsigned,float) override; void Update() override; void tick(float) override;
private:
    float phase_=0;
};
}
