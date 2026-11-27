#pragma once
#include "rp/OverlayAccessory.h"
namespace rp {
class BossBattleRaftHighlightSprite : public OverlayAccessory {
public:
    BossBattleRaftHighlightSprite(const OceanView*,GridSprite*);
    ~BossBattleRaftHighlightSprite() override;
    void Init() override; void Update() override; void tick(float) override;
private:
    float phase_=0;
};
}
