# Coverage and verification

| Measurement | Result |
|---|---:|
| Historical source-name inventory (includes adapters) | 1191/1191 |
| Historical compiled host name overlap | 503/1191 (42.23%) |
| Current compiled name/signature overlap | Not remeasured |
| Instruction-level equivalence | Not measured |
| Complete gameplay parity | Not established |

The [historical symbol report](MATCHING_STATUS.md) identifies the original ELF and measured baseline. The source-name inventory is not a completion score. A matching name does not establish signature, behavior, layout, or ABI equivalence.

Android ARM64 CI built source revision [6a24086](https://github.com/mlnitoon2/RaftPirates/commit/6a24086ec8ffd1f8f1efbee172dab724b5cd013b) and published [660e7f4](https://github.com/mlnitoon2/RaftPirates/commit/660e7f4e945a62de641a2fed8ea0ff081f12d862). Host regressions and device gameplay need independent verification. See [remaining work](REMAINING.md).

The [host regression workflow](../.github/workflows/host.yml) builds the C++ tests against HydraLegacy and runs CTest independently of Android library publication. Its results are available in [Actions](https://github.com/mlnitoon2/RaftPirates/actions).

The previously published Android build is [65f1afa](https://github.com/mlnitoon2/RaftPirates/commit/65f1afa8a62c10c5adc6424fe79e1a3cdc2a53cc). Device testing of that build identified remaining visual and interaction defects. Original-assets HUD tests are opt-in with `-DRAFT_ASSET_ROOT=/path/to/extracted/assets`; the default green host CI does not establish that those tests or actual Android menu interactions pass.
