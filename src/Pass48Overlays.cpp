#include "rp/OverlayAccessory.h"
#include "rp/SelectedWallSprite.h"
#include "rp/BossBattleRaftHighlightSprite.h"
#include "rp/RaftGridSprite.h"
#include "rp/PerimeterWallSprite.h"
#include "sys/gfx/AssetImage.h"
#include <cmath>
#include <algorithm>
namespace rp {
using namespace sys::gfx;
bool OverlayAccessory::loadSheet(const char* path){
    if(!ocean_)return false;
    try {
        sheet_=std::make_shared<sys::res::ResourceSpriteSheet>(sys::res::ResourceSpriteSheet::Create(ocean_->assetRoot()/path));
        atlas_=std::make_shared<Image>(loadAssetImage(ocean_->assetRoot()/sheet_->imagePath));
        return true;
    }catch(const std::exception&){sheet_.reset();atlas_.reset();return false;}
}
bool OverlayAccessory::appendCell(const char* name,Vec2 offset,float size,float priority){
    if(!sheet_||!atlas_)return false;
    auto* cell=sheet_->findSpriteByName(name);if(!cell)return false;
    Piece p; p.sprite.image=atlas_;p.sprite.cell=*cell;p.offset=offset;p.size=size;p.priorityOffset=priority;
    overlays_.push_back(std::move(p));return true;
}
bool OverlayAccessory::appendImage(const char* path,Vec2 offset,float size,float priority){
    if(!ocean_)return false;
    try {
        Piece p;auto image=std::make_shared<Image>(loadAssetImage(ocean_->assetRoot()/path));p.sprite.image=image;
        p.sprite.cell={path,0,0,image->width,image->height,0,0,image->width,image->height,false};
        p.offset=offset;p.size=size;p.priorityOffset=priority;overlays_.push_back(std::move(p));return true;
    }catch(const std::exception&){return false;}
}
bool OverlayAccessory::changeCell(unsigned i,const char* name){
    if(i>=overlays_.size()||!sheet_)return false;
    auto* cell=sheet_->findSpriteByName(name);if(!cell)return false;overlays_[i].sprite.cell=*cell;return true;
}
float OverlayAccessory::basePriority()const{return parent_&&parent_->Pieces()?parent_->GetPriority(0):0;}
void OverlayAccessory::layout(){
    if(!ocean_||!parent_)return;
    auto p=ocean_->WorldToScreen(parent_->worldPosition());
    for(auto& a:overlays_){
        const float s=scale_*a.size;
        a.sprite.SetScale(s,s);
        a.sprite.SetPos(p.x+a.offset.x*scale_-a.sprite.cell.ow*s*.5f,p.y+a.offset.y*scale_-a.sprite.cell.oh*s*.5f);
        a.sprite.SetVisible(visible_);
    }
}
void OverlayAccessory::SetScale(float s){if(std::isfinite(s)&&s>0){scale_=s;layout();}}
void OverlayAccessory::SetVisible(bool v){visible_=v;for(auto& p:overlays_)p.sprite.SetVisible(v);}
void OverlayAccessory::SetColour(uint8_t r,uint8_t g,uint8_t b,uint8_t a){for(auto& p:overlays_)p.sprite.SetColor(r,g,b,a);}
void OverlayAccessory::SetPriorities(){for(unsigned i=0;i<overlays_.size();++i)SetPriority(i,basePriority()+overlays_[i].priorityOffset);}
void OverlayAccessory::SetPriority(unsigned i,float p){if(i<overlays_.size()&&std::isfinite(p))overlays_[i].sprite.SetPriority(p);}
bool OverlayAccessory::ContainsGfx(const Gfx* g)const{if(!g)return false;for(const auto& p:overlays_)if(g==&p.sprite)return true;return false;}
int OverlayAccessory::submit(GfxManager& mgr,AnimationRenderer&)const{
    if(!visible_)return 0;int n=0;for(const auto& p:overlays_)if(p.sprite.image&&p.sprite.visible){mgr.submit(p.sprite);++n;}return n;
}
void OverlayAccessory::pulse(float phase,bool red){
    const auto alpha=uint8_t(255-uint8_t(63.75f+std::sin(phase)*63.75f));
    SetColour(255,red?128:255,red?128:255,alpha);
}
SelectedWallSprite::SelectedWallSprite(const OceanView* o,GridSprite* p):OverlayAccessory(o,p,3){}
SelectedWallSprite::~SelectedWallSprite()=default;
void SelectedWallSprite::Init(){
    overlays_.clear();loadSheet("xml_resources/construction_footprints_junk.xml");
    appendCell("item_glow_left",{-16,-40},1,.01f);appendCell("item_glow_right",{16,-40},1,.02f);
    Update();
}
void SelectedWallSprite::SetPriorities(){OverlayAccessory::SetPriorities();}
void SelectedWallSprite::SetPriority(unsigned i,float p){OverlayAccessory::SetPriority(i,p);}
void SelectedWallSprite::Update(){layout();SetPriorities();}
void SelectedWallSprite::tick(float dt){if(std::isfinite(dt)&&dt>=0){phase_=std::fmod(phase_+dt*6.2831853f,6.2831853f);pulse(phase_,false);}}
BossBattleRaftHighlightSprite::BossBattleRaftHighlightSprite(const OceanView* o,GridSprite* p):OverlayAccessory(o,p,17){}
BossBattleRaftHighlightSprite::~BossBattleRaftHighlightSprite()=default;
void BossBattleRaftHighlightSprite::Init(){
    overlays_.clear();loadSheet("xml_resources/construction_footprints_junk.xml");
    appendCell("item_glow_left",{-16,-40},1,.01f);appendCell("item_glow_right",{16,-40},1,.02f);
    layout();SetPriorities();pulse(phase_,true);
}
// Native Update returns directly. Geometry is refreshed on SetScale by the adapter.
void BossBattleRaftHighlightSprite::Update(){}
void BossBattleRaftHighlightSprite::tick(float dt){if(std::isfinite(dt)&&dt>=0){phase_=std::fmod(phase_+dt*6.2831853f,6.2831853f);pulse(phase_,true);}}
RaftGridSprite::RaftGridSprite(const OceanView* o,GridSprite* p):OverlayAccessory(o,p,16){}
RaftGridSprite::~RaftGridSprite()=default;
void RaftGridSprite::Init(){
    overlays_.clear();appendImage("gfx/rafts/raft_grid",{},1,.01f);appendImage("gfx/rafts/raft_topgrid",{},1,.02f);
    layout();SetPriorities();
}
void RaftGridSprite::SetPriorities(){OverlayAccessory::SetPriorities();}
void RaftGridSprite::SetPriority(unsigned i,float p){OverlayAccessory::SetPriority(i,p);}
// Native Update is a direct return; geometry is set by SetScale.
void RaftGridSprite::Update(){}
PerimeterWallSprite::PerimeterWallSprite(const OceanView* o,GridSprite* p):OverlayAccessory(o,p,1){}
PerimeterWallSprite::~PerimeterWallSprite()=default;
bool PerimeterWallSprite::building()const{return parent_&&parent_->constructionTime()>0;}
void PerimeterWallSprite::Init(){
    overlays_.clear();balloons_.clear();elapsed_=0;celebrating_=false;wasBuilding_=building();
    loadSheet("xml_resources/construction_footprints_junk.xml");
    appendCell("build_tarp_dark",{-16,-20},1,.01f);
    appendCell("build_tarp_light",{16,-20},1,.02f);
    appendCell("construction_timer",{0,-64},1,.03f);
    Update();
}
void PerimeterWallSprite::AddBalloons(){
    if(!ocean_||celebrating_)return;
    celebrating_=true;elapsed_=0;balloons_.clear();
    try{
        balloonResource_=std::make_shared<sys::res::ResourceAEAnim>(sys::res::ResourceAEAnim::Create(ocean_->assetRoot()/"xml_bin/anim_build_complete.bin",sys::res::AEFormat::RaftPirates2013));
        for(const std::string color:{"red","blue","green","yellow"}){
            if(!balloonResource_->find(color+"_intro"))continue;
            auto a=std::make_unique<AEAnim>(balloonResource_);a->setLoop(false);a->setAnimation(color+"_intro");
            if(balloonResource_->find(color+"_idle"))a->queueAnimation(color+"_idle");
            balloons_.push_back(std::move(a));
        }
    }catch(const std::exception&){balloonResource_.reset();}
}
void PerimeterWallSprite::RemoveFromTick(){if(!building())AccessorySprite::RemoveFromTick();}
void PerimeterWallSprite::SetPriorities(){OverlayAccessory::SetPriorities();}
void PerimeterWallSprite::SetPriority(unsigned i,float p){OverlayAccessory::SetPriority(i,p);}
void PerimeterWallSprite::SetVisible(bool v){OverlayAccessory::SetVisible(v);for(auto& b:balloons_)b->SetVisible(v);}
void PerimeterWallSprite::Update(){layout();SetPriorities();}
void PerimeterWallSprite::tick(float dt){
    if(!std::isfinite(dt)||dt<0)return;
    const bool now=building();if(wasBuilding_&&!now)AddBalloons();wasBuilding_=now;
    if(celebrating_){elapsed_+=dt;for(auto& a:balloons_)a->tick(dt);
        if(elapsed_>=2.f){balloons_.clear();celebrating_=false;AccessorySprite::RemoveFromTick();}}
}
int PerimeterWallSprite::submit(GfxManager& m,AnimationRenderer& r)const{
    int n=building()?OverlayAccessory::submit(m,r):0;
    if(!visible_||!celebrating_||!parent_||!ocean_)return n;
    auto p=ocean_->WorldToScreen(parent_->worldPosition());
    for(unsigned i=0;i<balloons_.size();++i){
        const auto& a=balloons_[i];
        const float rise=std::max(0.f,elapsed_-.5f)*100.f*scale_;
        auto matrix=Matrix::translation(p.x+(float(i)-1.5f)*25*scale_-a->width()*scale_*.5f,p.y-a->height()*scale_-rise)*Matrix::scale(scale_,scale_);
        n+=r.submit(m,*a,matrix);
    }return n;
}
}
