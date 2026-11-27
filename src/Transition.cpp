#include "rp/Transition.h"
#include <algorithm>
#include <cmath>

namespace rp {
    using namespace sys::gfx;
    Transition::MsgDone::~MsgDone() = default;

    Transition::Transition(std::filesystem::path assets, MsgReceiver* messages)
        : messages_(messages), renderer_(assets) {
        try {
            resource_ = sys::res::ResourceAEAnim::Create(
                assets / "xml_bin/anim_loading_curtains.bin", sys::res::AEFormat::RaftPirates2013);
        } catch (const std::exception&) {
        }
    }

    Transition::~Transition() = default;

    void Transition::changeType(Type type) {
        type_ = type;
    }

    void Transition::disableAllInput(bool disable) {
        inputDisabled_ = disable;
    }

    std::string Transition::animationName() const {
        std::string name = state_ == State::Intro   ? "intro"
                           : state_ == State::Outro ? "outro"
                                                    : "idle";
        if (type_ == Type::Alternate && resource_ && resource_->find(name + " 2"))
            name += " 2";
        return name;
    }

    const sys::res::AEComp* Transition::composition() const {
        return resource_ ? resource_->find(animationName()) : nullptr;
    }

    float Transition::duration() const {
        auto comp = composition();
        return comp && comp->duration() > 0 ? comp->duration() : .25f;
    }

    void Transition::show() {
        if (state_ == State::Intro || state_ == State::Idle)
            return;
        state_ = State::Intro;
        time_ = 0;
        disableAllInput(true);
    }

    void Transition::hide() {
        if (state_ == State::Hidden || state_ == State::Outro)
            return;
        state_ = State::Outro;
        time_ = 0;
    }

    void Transition::notify(bool shown) {
        MsgDone msg;
        msg.shown = shown;
        if (messages_)
            messages_->SendQueued(msg);
        if (done)
            done(shown);
    }

    void Transition::gotMsgAnimationFinished(const sys::msg::MsgAnimationFinished& msg) {
        if (msg.name != animationName())
            return;
        if (state_ == State::Intro) {
            state_ = State::Idle;
            time_ = 0;
            notify(true);
        } else if (state_ == State::Outro) {
            state_ = State::Hidden;
            time_ = 0;
            disableAllInput(false);
            notify(false);
        }
    }

    void Transition::tick(float dt) {
        if (!active() || !std::isfinite(dt) || dt <= 0)
            return;
        time_ += std::min(dt, 1.f);
        const float d = duration();
        if (time_ < d)
            return;
        if (state_ == State::Idle)
            time_ = std::fmod(time_, d);
        else
            gotMsgAnimationFinished(sys::msg::MsgAnimationFinished(animationName()));
    }

    void Transition::Render(Image& dst) {
        if (!active())
            return;
        auto comp = composition();
        if (comp && comp->width > 0 && comp->height > 0) {
            GfxManager gfx;
            renderer_.submit(
                gfx, *resource_, *comp, std::min(time_, comp->duration()),
                Matrix::scale(float(dst.width) / comp->width, float(dst.height) / comp->height));
            gfx.RenderScreen(dst);
        } else {
            const float alpha =
                state_ == State::Idle ? 1.f : std::clamp(time_ / duration(), 0.f, 1.f);
            const float visibility = state_ == State::Outro ? 1 - alpha : alpha;
            for (auto& pixel : dst.pixels) {
                pixel.r = uint8_t(pixel.r * (1 - visibility));
                pixel.g = uint8_t(pixel.g * (1 - visibility));
                pixel.b = uint8_t(pixel.b * (1 - visibility));
            }
        }
    }
} // namespace rp
