#include "rp/FireFXSprite.h"
#include "sys/gfx/AssetImage.h"
#include "sys/localization/LocalizationManager.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rp {
using namespace sys::gfx;
FireFXSprite::FireFXSprite(const OceanView* o,GridSprite* p):OverlayAccessory(o,p,12){}
FireFXSprite::~FireFXSprite()=default;
void FireFXSprite::Init(){
    emitters_.clear();if(!ocean_)return;
    const char* configs[]={"particles/raft_item_fire.psi","particles/raft_item_smoke.psi"};
    const char* textures[]={"gfx/fx/particle_fire","gfx/fx/particle_puff"};
    for(unsigned i=0;i<2;++i)try{
        Emitter e;auto info=sys::particles::ParticleInfo::load(ocean_->assetRoot()/configs[i]);
        e.particles=std::make_unique<sys::particles::ParticleSystem>(info,5489+i);
        auto image=std::make_shared<Image>(loadAssetImage(ocean_->assetRoot()/textures[i]));
        e.sprite.image=image;e.sprite.cell={textures[i],0,0,image->width,image->height,0,0,image->width,image->height,false};
        e.sprite.SetColor(tint_.r,tint_.g,tint_.b,tint_.a);
        emitters_.push_back(std::move(e));
    }catch(const std::exception&){}
    Update();SetVisible(visible_);
}
void FireFXSprite::SetColour(uint8_t r,uint8_t g,uint8_t b,uint8_t a){
    tint_={r,g,b,a};for(auto& e:emitters_)e.sprite.SetColor(r,g,b,a);
}
void FireFXSprite::SetVisible(bool v){
    visible_=v;for(auto& e:emitters_){if(v){if(!e.particles->emitting())e.particles->fire();}else e.particles->stop(true);}
}
void FireFXSprite::Update(){
    if(!parent_||!ocean_)return;auto p=ocean_->WorldToScreen(parent_->worldPosition());
    p.y-=20.f*scale_;for(auto& e:emitters_)e.particles->setPosition(p);
}
void FireFXSprite::tick(float dt){if(!visible_||!std::isfinite(dt)||dt<0)return;Update();for(auto& e:emitters_)e.particles->tick(std::min(dt,1.f));}
int FireFXSprite::submit(GfxManager& m,AnimationRenderer&)const{
    if(!visible_)return 0;int n=0;
    for(const auto& e:emitters_){e.particles->submit(m,e.sprite,basePriority()+.04f);n+=int(e.particles->particles().size());}return n;
}
} // namespace rp
