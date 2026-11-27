# Dive and construction timing audit

ARMv7 SHA-256: 29544f0b70c947e23b05c01016a88b94b90e28fd204ba956daea16f6fdd26c13.

- `rp::RaftComponent::SetRemainingTime(float)` at 0x54b98c stores the floating argument and adds to the owning grid's temporary tick list when attached. Restore its float signature and fractional values without changing portable integer/fraction storage. The grid registration remains a separate portable lifecycle operation.
- `rp::DivingUnit::StartDive(float,float)` at 0x54275c stores the first argument as total duration, uses the first argument unless it exceeds the second by more than 2.0 seconds, clamps the countdown at zero, and sets state 3. Restore this comparison and floating duration storage. The native introductory animation and event timers are still not reproduced.
- `rp::DivingUnit::SetState(int)` at 0x541b48 clears remaining time and total/return timers on transition from state 3 to 0. Restore the available countdown fields without introducing fake return events or client-authoritative gameplay.
- `RaftComponent::UseClock` at 0x54b6c4 schedules reductions across animation frames, not a simple immediate subtraction. `DivingUnit::UseClock` at 0x541ac4 tail-calls it. These still differ in the portable reconstruction; do not infer a complete implementation from method coverage. Native FinishDive starts a three-second return timer (0x542668); the current portable server-driven completion is unchanged pending full event integration.

The source filenames correspond to exported class names but exact original paths and ABI layout are unknown. No server wire keys or external networking behavior changed.
