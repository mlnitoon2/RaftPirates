#pragma once
#include "sys/State.h"
#include <string>
namespace rp {
class RaftPirates;
class RaftPiratesGameState : public sys::State {
public:
    RaftPiratesGameState(RaftPirates*, sys::State::GameState);
    ~RaftPiratesGameState() override;
    virtual std::string GetStateName() const;
    void Pause() override;
    virtual bool isReadyForBattle() const;
    void tick(float) override;
protected:
    RaftPirates* game_;
    sys::State::GameState state_;
};
}
