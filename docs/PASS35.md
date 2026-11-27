# Pass 35

Original ARM32 `GotMsgStartTransition` at 0x4a180c tests a boolean message field. False calls Transition::hide; true disables network message processing then calls Transition::show. `GotTransitionMsgDone` at 0x4a1840 re-enables message processing before dispatching a completion message. Portable handlers now preserve this queue behavior, with transition visibility tracked separately. A regression asserts that messages remain queued until completion, including when hiding the transition. The visual transition and original message bus hookup are not yet implemented.

The preceding Pass 34 Android CI run 45924852753 succeeded. Pass 35 host regressions and Android compatibility are not yet verified. See REMAINING.md for the last measured inventory and subsequent source changes.
