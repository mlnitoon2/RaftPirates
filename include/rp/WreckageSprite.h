#pragma once
#include "rp/GridSprite.h"
#include "rp/Wreckage.h"
namespace rp {
// Caller retains the Wreckage and its definition while this sprite is live.
class WreckageSprite : public GridSprite {
public:
    WreckageSprite(const OceanView&,const Wreckage&,const GridObjectInfo&,sys::gfx::Vec2,float);
    ~WreckageSprite() override;
    void Init() override;
    float GetPriority(unsigned) const override;
    void SetPriority(unsigned,float) override;
    void UpdatePiece(unsigned) override;
    int submit(sys::gfx::GfxManager&, std::map<std::string,std::shared_ptr<sys::gfx::Image>>&,
               std::map<std::string,sys::res::ResourceAEAnim>&,sys::gfx::AnimationRenderer&,
               const std::filesystem::path&,bool) override;
private:
    const OceanView& view_;
    const Wreckage& wreckage_;
    std::vector<sys::gfx::GfxSprite> debris_;
    void layoutDebris();
};
}
