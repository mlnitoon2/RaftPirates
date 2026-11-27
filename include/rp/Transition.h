#pragma once
#include "sys/gfx/AnimationRenderer.h"
#include "sys/msg/MsgReceiver.h"
namespace rp {
    class Transition {
      public:
        enum class Type { Curtains, Alternate };
        struct MsgDone : MsgBase {
            bool shown=false;
            ~MsgDone() override;
        };
        explicit Transition(std::filesystem::path assets,MsgReceiver* messages=nullptr);
        ~Transition();
        void changeType(Type);
        void disableAllInput(bool disable);
        void gotMsgAnimationFinished(const sys::msg::MsgAnimationFinished&);
        void show();
        void hide();
        void tick(float dt);
        void Render(sys::gfx::Image&);
        bool active()const{return state_!=State::Hidden;}
        bool blocksInput()const{return inputDisabled_;}
        std::function<void(bool)> done;
      private:
        enum class State { Hidden, Intro, Idle, Outro };
        Type type_=Type::Curtains;
        State state_=State::Hidden;
        bool inputDisabled_=false;
        float time_=0;
        MsgReceiver* messages_;
        sys::gfx::AnimationRenderer renderer_;
        std::optional<sys::res::ResourceAEAnim> resource_;
        const sys::res::AEComp* composition()const;
        std::string animationName()const;
        float duration()const;
        void notify(bool shown);
    };
}
