#include "rp/RaftGridSprite.h"
#include "sys/gfx/AssetImage.h"
#include "sys/localization/LocalizationManager.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rp {
using namespace sys::gfx;
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
} // namespace rp
