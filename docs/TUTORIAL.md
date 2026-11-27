# Tutorial reconstruction

Native ARM32 RaftPirates::checkTutorialStart (0x49ef14) recognizes definition ID 189. The jump table in checkTutorialCompletion (0x49e5dc; entries 0x49e60c onward) maps IDs to 33 milestone strings. Stage 33 passes true to the historical analytics tracker; other entries pass false. The portable client records these milestones locally, without contacting retired analytics endpoints. The native extension dispatcher reads questid before calling checkTutorialCompletion (0x4b162c), and id before checkTutorialStart (0x4b1ee8).

The original hud.xml highlightItem event recognizes challenge_charter, main_menu_map, main_menu_rafts, main_menu_weapons and toggle_button. The client displays the original battle_slider_arrow near the relevant HUD control. Server-defined popup_immediately opens the existing quest dialog and sends gs_quest_read once per quest per PortView. The existing quest art slots are bound to server-provided sheet/image names, including rp_tutorial_popups.

Still missing: the original quest database with objectives and rewards, exact tutorial arrow bounce and placement, UI blocking/unblocking, battle-specific scripted tutorials, and persistent tutorial state on the server. No synthetic quest IDs/rewards are sent. A server with no tutorial quest definitions will not start this tutorial.
