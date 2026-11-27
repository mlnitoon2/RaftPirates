#include "rp/DiveUnitReadoutSprite.h"
#include "sys/gfx/AssetImage.h"
#include "sys/localization/LocalizationManager.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rp {
using namespace sys::gfx;
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
} // namespace rp

namespace rp {
    DiveUnitReadoutSprite::ReadoutLight::ReadoutLight() = default;
    DiveUnitReadoutSprite::ReadoutLight::~ReadoutLight() = default;
} // namespace rp
