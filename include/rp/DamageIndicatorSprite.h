#pragma once
namespace rp {
    class DamageIndicatorSprite {
      public:
        static constexpr float FLOAT_TIME=1.5f;
        DamageIndicatorSprite(bool own,int raft,int amount);
        ~DamageIndicatorSprite();
        void tick(float dt);
        float yOffset()const{return -40.f+20.f*remaining;}
        bool own;
        int raft,amount;
        float remaining=FLOAT_TIME;
    };
}
