#include "rp/BattleArrowSprite.h"
#include <cmath>
namespace rp {
    BattleArrowSprite::BattleArrowSprite()=default;
    BattleArrowSprite::~BattleArrowSprite()=default;
    void BattleArrowSprite::tick(float dt){if(std::isfinite(dt)&&dt>0){phase_=std::fmod(phase_-dt,1.f);if(phase_<0)phase_+=1;}}
    float BattleArrowSprite::yOffset()const{return -30.f-30.f*std::sin(3.14159265358979323846f*phase_);}
}
