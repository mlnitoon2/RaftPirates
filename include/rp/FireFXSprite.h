#pragma once
#include "rp/OverlayAccessory.h"
#include "sys/particles/ParticleSystem.h"
namespace rp {
class FireFXSprite : public OverlayAccessory {
public:
    FireFXSprite(const OceanView*,GridSprite*);
    ~FireFXSprite() override;
    void Init() override;
    void SetColour(uint8_t,uint8_t,uint8_t,uint8_t) override;
    void SetVisible(bool) override;
    void Update() override;
    void tick(float) override;
    int submit(sys::gfx::GfxManager&,sys::gfx::AnimationRenderer&)const override;
private:
    struct Emitter { std::unique_ptr<sys::particles::ParticleSystem> particles; sys::gfx::GfxSprite sprite; };
    std::vector<Emitter> emitters_;
    sys::gfx::Color tint_{};
};
}
