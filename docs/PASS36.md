# Pass 36 — OceanView and game-session callbacks

ARM32 evidence from the original APK:
- 0x4a07fc, 0x4a0800, 0x4a0804 branch directly to checkKeepAlive (0x4a07b4; threshold constant 30 seconds). Touch events now run the shared keep-alive check.
- 0x4a3abc sends purchase_item with integer material, amount, gold_cost. Blueprint material confirmation computes missing amounts and individual gold costs before dispatching material purchase messages.
- 0x4a3c18 builds repair_grid_object_id, gold_cost, repair_all and calls BuildModeState::sendSelectedObjectRepairDataWithGold (0x3c1288), which adds object_id, object_type and optional parent_id and sends purchase_repair.
- 0x4b0604 sends set_display_name with newName when the keyboard result is not cancelled. The adapter requires an active naming prompt, a ready session and nonempty input; the original menu trigger remains open.
- 0x4a1c6c preloads tabbed_menu_blueprints, tabbed_menu_free, tabbed_menu_inventory and tabbed_menu_misc. PortView now caches parsed definitions of these menus.
- 0x471518 adds a water-bucket accessory for a component below full health if enabled and not already attached. Eligibility is available, not original accessory animation.
- 0x4a3298 / 0x4a32ec switch game-state objects. Portable state tracking follows server-authoritative battle phase, without fabricating combat state.

Ocean sprites are retained using owned snapshots, not raw pointers into mutable player vectors. Health-bar drawing, ordering, animation override lifetime and selection APIs are portable approximations. Original layer/picking ABI, accessory timers, boss background, particle and audio effects remain open. Host tests added but not executed here. Pass 35 ARM64 CI succeeded; Pass 36 not yet verified.
