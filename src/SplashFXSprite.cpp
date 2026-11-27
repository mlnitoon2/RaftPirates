#include "rp/SplashFXSprite.h"
#include "sys/gfx/AssetImage.h"
#include "sys/localization/LocalizationManager.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rp {
using namespace sys::gfx;
SplashFXSprite::SplashFXSprite(const OceanView* o,GridSprite* p,float seconds):AccessorySprite(o,p,11),seconds_(std::isfinite(seconds)?std::clamp(seconds,0.f,60.f):0){}
SplashFXSprite::~SplashFXSprite()=default;
void SplashFXSprite::Init(){
    splashes_.clear();elapsed_=0;finished_=true;
    if(!ocean_||!parent_||seconds_<=0)return;
    try{
        resource_=std::make_shared<sys::res::ResourceAEAnim>(sys::res::ResourceAEAnim::Create(ocean_->assetRoot()/"xml_bin/anim_water_splash.bin",sys::res::AEFormat::Rev1_5));
        if(!resource_->find("splash"))return;
        // Native iterates raft piece cells. Host uses each reconstructed visual piece's world position.
        for(unsigned i=0;i<parent_->Pieces();++i){
            auto a=std::make_unique<AEAnim>(resource_);a->setLoop(false);a->setAnimation("splash");
            splashes_.push_back({std::move(a),parent_->GetWorldPosition(int(i))});
        }finished_=splashes_.empty();
    }catch(const std::exception&){splashes_.clear();resource_.reset();}
}
// Original Update and virtual RemoveFromTick are return-only; tick calls base on completion.
void SplashFXSprite::Update(){}
void SplashFXSprite::RemoveFromTick(){}
void SplashFXSprite::tick(float dt){
    if(finished_||!std::isfinite(dt)||dt<0)return;elapsed_+=dt;
    for(auto& s:splashes_)s.animation->tick(dt);
    splashes_.erase(std::remove_if(splashes_.begin(),splashes_.end(),[](const auto& s){return !s.animation->playing();}),splashes_.end());
    if(splashes_.empty()){finished_=true;AccessorySprite::RemoveFromTick();}
}
int SplashFXSprite::submit(GfxManager& m,AnimationRenderer& r)const{
    if(finished_||!ocean_||!parent_||!parent_->GetVisible())return 0;int n=0;const float s=ocean_->Zoom();
    for(const auto& a:splashes_){auto p=ocean_->WorldToScreen(a.world);
        n+=r.submit(m,*a.animation,Matrix::translation(p.x-a.animation->width()*s*.5f,p.y-a.animation->height()*s*.5f)*Matrix::scale(s,s));
    }return n;
}
} // namespace rp
