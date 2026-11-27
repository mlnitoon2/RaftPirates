#include "rp/GfxOceanCheckerBoard.h"
#include "rp/Game.h"
#include <cmath>
namespace rp {
using namespace sys::gfx;
GfxOceanCheckerBoard::GfxOceanCheckerBoard(const OceanView* view,float unit):view_(view){SetUnitSize(unit);}
GfxOceanCheckerBoard::~GfxOceanCheckerBoard()=default;
float GfxOceanCheckerBoard::FloatMod(float a,float b)const{return std::isfinite(a)&&std::isfinite(b)&&b!=0?std::fmod(a,b):0;}
void GfxOceanCheckerBoard::SetPosition(Vec2 p){position_=p;}
void GfxOceanCheckerBoard::SetUnitSize(float u){if(std::isfinite(u)&&u>0)unit_=u;}
void GfxOceanCheckerBoard::Render(Image& image)const{
    if(!view_||view_->Unit()<=0)return;
    const auto origin=view_->ScreenToWorld(position_);
    const double scale=double(view_->Unit())/unit_;
    // CPU approximation: native computes tile parity then submits GL geometry.
    const auto a=view_->ScreenToWorld({0,0}), b=view_->ScreenToWorld({1,0}), c=view_->ScreenToWorld({0,1});
    for(int y=0;y<image.height;++y)for(int x=0;x<image.width;++x){
        double u=(a.x-origin.x+(b.x-a.x)*x+(c.x-a.x)*y)*scale;
        double v=(a.y-origin.y+(b.y-a.y)*x+(c.y-a.y)*y)*scale;
        if(!std::isfinite(u)||!std::isfinite(v))continue;
        if(std::abs(std::fmod(std::floor(u)+std::floor(v),2.0))==1){
            auto& p=image.at(x,y);p.r=uint8_t(p.r*.92f);p.g=uint8_t(p.g*.92f);p.b=uint8_t(p.b*.92f);
        }
    }
}
}
