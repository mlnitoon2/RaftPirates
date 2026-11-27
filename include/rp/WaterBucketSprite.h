#pragma once
#include <cmath>
namespace rp {
    class WaterBucketSprite {
      public:
        WaterBucketSprite();
        ~WaterBucketSprite();
        void tick(float dt);
        float scale()const{return .875f+.125f*std::sin(6.283185307179586f*phase_);}
      private:
        float phase_=1;
    };
}
