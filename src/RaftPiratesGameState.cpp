#include "rp/RaftPiratesGameState.h"
namespace rp {
RaftPiratesGameState::RaftPiratesGameState(RaftPirates* game,sys::State::GameState state):game_(game),state_(state){}
RaftPiratesGameState::~RaftPiratesGameState()=default;
// ARM base defaults at 0x38232c, 0x382330, 0x382334 and 0x382710.
void RaftPiratesGameState::tick(float){}
void RaftPiratesGameState::Pause(){}
bool RaftPiratesGameState::isReadyForBattle()const{return true;}
std::string RaftPiratesGameState::GetStateName()const{return {};}
}
