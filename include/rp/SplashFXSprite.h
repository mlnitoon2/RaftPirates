#pragma once
#include "rp/OverlayAccessory.h"
namespace rp {
class SplashFXSprite : public AccessorySprite {
public:
    SplashFXSprite(const OceanView*,GridSprite*,float);
    ~SplashFXSprite() override;
    void Init() override;
    void RemoveFromTick() override;
    void Update() override;
    void tick(float) override;
    bool finished()const override{return finished_;}
    int submit(sys::gfx::GfxManager&,sys::gfx::AnimationRenderer&)const override;
private:
    struct Splash { std::unique_ptr<sys::gfx::AEAnim> animation; sys::gfx::Vec2 world; };
    std::vector<Splash> splashes_;
    std::shared_ptr<const sys::res::ResourceAEAnim> resource_;
    float seconds_=0,elapsed_=0;
    bool finished_=false;
};
}
