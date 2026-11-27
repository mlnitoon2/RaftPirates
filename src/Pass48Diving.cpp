#include "rp/DiveUnitReadoutSprite.h"
#include "rp/DiveMeterSprite.h"
#include <algorithm>
#include <cmath>
namespace rp {
using namespace sys::gfx;
DiveMeterSprite::DiveMeterSprite(const OceanView* o,GridSprite* p):OverlayAccessory(o,p,2){
    loadSheet("xml_resources/rp_menu.xml");
    if(appendCell("dive_game_meter_frame",{0,-64},1,.03f))frame_=int(overlays_.size())-1;
    if(appendImage("gfx/HUD/dive_game_meter",{0,-64},1,.04f))fill_=int(overlays_.size())-1;
    if(appendCell("dive_game_button",{0,-10},.5f,.05f))button_=int(overlays_.size())-1;
    if(o)try{font_=std::make_shared<ResourceSpriteFont>(ResourceSpriteFont::Create(o->assetRoot(),"rp_font_title"));}catch(const std::exception&){}
    layout();OverlayAccessory::SetPriorities();
}
DiveMeterSprite::~DiveMeterSprite()=default;
// Native setters return immediately; tick reads authoritative dive progress.
void DiveMeterSprite::SetPriorities(){}
void DiveMeterSprite::SetTime(int){}
void DiveMeterSprite::SetDive(const DivingUnit& d){dive_=d;tick(0);}
bool DiveMeterSprite::Select(Gfx* gfx){
    if(!gfx||!visible_||!dive_.Diving()||dive_.remaining<=0||button_<0||gfx!=&overlays_[button_].sprite)return false;
    if(selected_)selected_();return true;
}
void DiveMeterSprite::tick(float dt){
    if(!std::isfinite(dt)||dt<0)return;
    phase_=std::fmod(phase_+dt,1.5f);layout();OverlayAccessory::SetPriorities();
    if(fill_>=0){
        auto& s=overlays_[fill_].sprite;const float h=float(s.cell.oh);
        const float f=dive_.GetDiveTime();
        s.SetMask(0,h*(1-f),float(s.cell.ow),h*f,true);
    }
    if(button_>=0){
        auto& b=overlays_[button_];const float s=b.size*scale_*(1.f+.1f*std::sin(phase_*(6.2831853f/1.5f)));
        b.sprite.SetScale(s,s);b.sprite.SetVisible(visible_&&dive_.Diving()&&dive_.remaining>0);
    }
}
int DiveMeterSprite::submit(GfxManager& m,AnimationRenderer& r)const{
    if(!visible_||!dive_.Diving())return 0;
    int n=OverlayAccessory::submit(m,r);
    if(font_&&parent_&&ocean_){
        auto p=ocean_->WorldToScreen(parent_->worldPosition());
        auto l=font_->layout(GetTimeString(int(std::ceil(std::max(0.f,dive_.remaining)))),.65f*scale_);
        font_->submit(m,l,p.x-l.width*.5f,p.y-62.5f*scale_-l.height*.5f,{},basePriority()+.06f);
    }return n;
}
DiveUnitReadoutSprite::DiveUnitReadoutSprite(const OceanView* o,GridSprite* p):OverlayAccessory(o,p,8){
    if(loadSheet("xml_resources/texture_water_splash.xml")){
        appendCell("c_dive_sign",{0,-28},1,.03f);
        for(unsigned i=0;i<7;++i)appendCell("sign_light_off",{-30.f+10.f*i,-33},1,-.01f);
        for(unsigned i=0;i<2;++i)appendCell("sign_long_light_off",{-18.f+36.f*i,-20},1,-.01f);
    }layout();OverlayAccessory::SetPriorities();
}
DiveUnitReadoutSprite::~DiveUnitReadoutSprite()=default;
void DiveUnitReadoutSprite::SetPriorities(){}
void DiveUnitReadoutSprite::SetVisible(bool v){OverlayAccessory::SetVisible(v);refresh();}
void DiveUnitReadoutSprite::gotMsgDiveDepthChange(const MsgDiveDepthChange& msg){
    const int max=std::max(0,msg.maximum), depth=std::clamp(msg.depth,0,max);
    if(max==maximum_&&depth==depth_)return;
    maximum_=max;depth_=depth;elapsed_=0;refresh();
}
void DiveUnitReadoutSprite::refresh(){
    if(overlays_.empty())return;
    layout();OverlayAccessory::SetPriorities();
    const int count=std::clamp(maximum_,0,9), filled=std::clamp(depth_,0,count);
    for(unsigned i=1;i<overlays_.size();++i){
        const bool longLight=i>7;const char* name=longLight?"sign_long_light_off":"sign_light_off";
        if(int(i)<=filled) name=longLight?"sign_long_light_green":"sign_light_green";
        else if(int(i)==filled+1&&elapsed_<2.5f&&std::fmod(elapsed_,2.f)<1.f)
            name=longLight?"sign_long_light_red":"sign_light_red";
        changeCell(i,name);overlays_[i].sprite.SetVisible(visible_&&int(i)<=count);
    }
}
void DiveUnitReadoutSprite::tick(float dt){
    if(!std::isfinite(dt)||dt<0)return;elapsed_=std::min(2.5f,elapsed_+dt);refresh();
}
}
