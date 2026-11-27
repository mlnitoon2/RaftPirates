#include "rp/SelectedWallSprite.h"
#include "sys/gfx/AssetImage.h"
#include "sys/localization/LocalizationManager.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rp {
using namespace sys::gfx;
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
} // namespace rp
