#include "rp/ThrobingIndicatorSprite.h"
namespace rp {
    ThrobingIndicatorSprite::ThrobingIndicatorSprite()=default;
    ThrobingIndicatorSprite::~ThrobingIndicatorSprite()=default;
    void ThrobingIndicatorSprite::tick(float dt){if(std::isfinite(dt)&&dt>0){phase_=std::fmod(phase_-dt,1.f);if(phase_<0)phase_+=1;}}
}
