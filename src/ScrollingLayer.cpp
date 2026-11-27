#include "rp/ScrollingLayer.h"
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <utility>
namespace rp::diving_game {
    ScrollingLayer::ScrollingLayer(std::vector<std::string> images,float h,float factor,float vh)
        :images_(std::move(images)),height_(h),multiplier_(factor),viewportHeight_(vh){
        if(images_.empty()||!std::isfinite(h)||h<=0||!std::isfinite(vh)||vh<=0||
            !std::isfinite(factor)||factor<0||vh/h>1024)throw std::invalid_argument("Invalid scrolling layer");
        for(float y=0;y<vh+h;y+=h)addSprite(y);
    }
    void ScrollingLayer::SetSpeed(float speed){if(std::isfinite(speed)&&speed>=0&&std::isfinite(speed*multiplier_))speed_=speed*multiplier_;}
    void ScrollingLayer::addSprite(float y){if(!std::isfinite(y))return;tiles_.push_back({images_[next_++%images_.size()],y});}
    void ScrollingLayer::tick(float dt){
        if(!std::isfinite(dt)||dt<=0||tiles_.empty())return;
        // Native subtracts dt*speed. Normalize long gaps before recycling.
        const double span=double(height_)*tiles_.size();
        float step=float(std::fmod(double(dt)*speed_,span));
        for(auto& tile:tiles_)tile.y-=step;
        while(tiles_.front().y+height_<=0){
            float y=tiles_.back().y+height_;
            tiles_.erase(tiles_.begin());addSprite(y);
        }
    }
}
