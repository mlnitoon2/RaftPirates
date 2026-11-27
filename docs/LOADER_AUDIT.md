# Native loader audit

Reference: original armeabi-v7a ELF, SHA-256 29544f0b70c947e23b05c01016a88b94b90e28fd204ba956daea16f6fdd26c13.

The six rp::Loader static ID pointers at 0xb5dc4c–0xb5dc60 resolve to SPR, AE, SS, MENU, MANI and XMLDATA respectively. Native getType methods at 0x243158–0x2431f0 load these pointers. The original rp::LoadingManager::resourceExists at 0x2436c4 iterates its loader vector, comparing virtual getId and getType strings; it is not a disk file-existence test. The reconstruction now queries queued job ID and type, and uses the separately named portable assetExists for physical startup files. Exact original virtual ABI and resource pointer ownership remain unrecovered.

The original loader classes and their RTTI reside in rp, while PNG/JPEG/PVR decoders reside in sys. Do not move rp manifest scheduling into the engine merely to consolidate file names. HydraLegacy owns the generic resource and image primitives.

## Progress and cancellation

`LoadingManager::setPercentage` at `0x2438c4` multiplies its float argument by 100 and converts it to a signed integer before dispatching a progress message. `update` at `0x2459f8` divides the current loader index by the loader count before calling it. The portable method now accepts a fraction and exposes the truncated displayed percent. `cancel` at `0x24327c` only clears the running state; `start` at `0x249104` refuses to restart an active session and clears its cursors when starting an idle one. The portable methods now preserve queued jobs on cancel, restart counters on start, and expose a separate reset for clearing the queue. The native prepare/load phases and 34ms time budget remain separate future reconstruction work; the portable one-job-per-tick scheduler is intentionally retained.
