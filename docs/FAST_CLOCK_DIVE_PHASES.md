# Fast-clock and diving phase reconstruction

Reference ARMv7 ELF SHA-256 29544f0b70c947e23b05c01016a88b94b90e28fd204ba956daea16f6fdd26c13.

* RaftComponent::UseClock (0x54b6c4) receives the target remaining time, stores it at offset 52, computes max(1, (remaining-target)/CLOCK_FRAME_COUNT), starts a CLOCK_DELAY_TIME countdown and plays a sound. Data symbols at 0xb698a4 and 0xb698a8 hold 0.05 and 1.0; initializer 0x55316c-0x553194 sets CLOCK_FRAME_COUNT to 2.0 / CLOCK_DELAY_TIME. TempTick (0x54b794) subtracts one step when the delay expires, adds the interval back, reduces the positive target by dt and stops the acceleration on reaching the target. The portable shared countdown executes this sequence instead of immediate subtraction. The native sound dispatch and generic grid event dispatch are not yet wired.
* DivingUnit::UseClock (0x541ac4) tail-calls the base method. Both reconstructed views now use that same timer.
* StartDive (0x54275c) starts the 1-second intro when idle. FinishDive (0x542668) clears total/remaining and starts the 3-second return; TempTick (0x54238c) follows it with a 0.5-second settling phase before SetState(0). GetDiveTime (0x541758) returns returnTime/3 during return and 1-remaining/total otherwise. PlayAnimations (0x541ff4) selects drop/diving, return, up/idle or idle item. Portable animation clock sampling is adapted to the existing sprite renderer, not native message ABI.
* The materials acknowledgment starts the return phase; no local loot is granted. Collection is blocked during return. The server's construction state remains authoritative; a gs_use_clock acknowledgment sets the visual timer's target. Tests cover clock frames and return transitions. The earlier protocol test lacked a state field; the fixture now supplies the same state and duration keys as the server.

Original file names and class layout remain unproven by stripped symbols; existing public class paths are retained.
