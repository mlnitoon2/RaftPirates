#include "rp/Game.h"
#include <stdexcept>

namespace {
    using O = sfs::SFSObjectWrapper;
    using A = sfs::SFSArrayWrapper;

    void check(bool result, const char* description) {
        if (!result)
            throw std::runtime_error(description);
    }

    O payload(const std::optional<sfs::Message>& m, const std::string& command) {
        check(m && m->controller == 1 && m->action == 13 && m->payload.getString("c") == command,
              "wrong extension request");
        return *m->payload.getSFSObj("p");
    }
} // namespace

int main() {
    rp::Game game;
    game.ready = true;
    O map, boss;
    boss.put("user_id", int32_t(3));
    boss.put("display_name", "Boss fixture");
    boss.put("zone", int32_t(1));
    boss.put("battle_points", int32_t(1200));
    map.put("boss_data", std::make_shared<O>(boss));
    game.extension("gs_ocean", map);
    check(game.bossData && game.bossData->getInt("battle_points") == 1200, "boss map record");
    game.objects[1] = rp::GridObjectInfo(1, 0, 100, "Core", "", 0, 0, {});
    game.rafts.emplace_back(1, 1, 0, 0, 100, 0, std::vector<rp::Component>{});
    auto& battle = game.onlineBattle;
    using Phase = rp::BattleModeState::Phase;
    payload(game.pingSonar(), "ping_sonar");
    check(!game.pingSonar(), "duplicate sonar");
    game.extension("gs_encounter", {});
    check(battle.phase() == Phase::Encounter && !game.sonarPending, "encounter");
    check(payload(battle.encounterResponse(true), "gs_encounter_response").getBool("attack"),
          "accept");
    O start;
    start.put("attacker", true);
    game.extension("gs_start_battle", start);
    O other, snapshot, raft;
    raft.put("user_raft_id", int32_t(2));
    raft.put("raft", int32_t(1));
    raft.put("current_bp", int32_t(100));
    auto rafts = std::make_shared<A>();
    rafts->values.emplace_back(std::make_shared<O>(raft));
    snapshot.put("rafts", rafts);
    other.put("player_object", std::make_shared<O>(snapshot));
    other.put("opponent_name", "Fixture");
    other.put("is_boss", true);
    game.extension("gs_opponent", other);
    check(battle.hasOpponent() && battle.opponent().GetRaftByID(2), "opponent snapshot");
    check(!battle.SendRaftSelection(1, 9, game), "invalid raft");
    check(battle.isBoss(), "boss flag");
    auto placement = payload(battle.gotMsgBattleScreenRaftsSelected(2, -1), "gs_battle_position");
    check(placement.getInt("attack_x") == 2 && placement.getInt("attack_y") == -1,
          "position fields");
    O position;
    position.put("sync_counter", int32_t(0));
    position.put("attack_position_okay", true);
    game.extension("gs_battle_position", position);
    check(battle.syncCounter() == 1, "position sync");
    auto select = payload(battle.SendRaftSelection(1, 2, game), "gs_battle_select_raft");
    check(select.getInt("attack_raft") == 1 && select.getInt("defend_raft") == 2 &&
              select.getInt("sync_counter") == 1,
          "selection fields");
    O selected;
    selected.put("sync_counter", int32_t(1));
    game.extension("gs_battle_select_raft", selected);
    check(battle.phase() == Phase::Fighting && !battle.attack(4), "ready gate");
    auto ready = payload(battle.SendClientReady(), "gs_battle_client_ready");
    check(ready.getInt("sync_counter") == 2 && !ready.getBool("confirm_swap_stance"),
          "ready fields");
    game.extension("gs_battle_client_ready", {});
    check(!battle.attack(9) && !battle.defend(2), "dice/stance guard");
    auto attack = payload(battle.attack(4), "gs_battle_attack");
    check(attack.getInt("attack_dice") == 4 && attack.getInt("sync_counter") == 2, "attack fields");
    check(!battle.attack(4), "duplicate attack");
    O result;
    result.put("sync_counter", int32_t(2));
    auto outcomes = std::make_shared<A>();
    O outcome, target;
    target.put("user_raft_id", int32_t(2));
    target.put("current_bp", int32_t(40));
    outcome.put("target_raft", std::make_shared<O>(target));
    outcomes->values.emplace_back(std::make_shared<O>(outcome));
    result.put("results", outcomes);
    game.extension("gs_battle_attack", result);
    check(battle.lastResult().getSFSArray("results") && game.rafts.front().bp == 100,
          "authoritative result retained without invented damage");
    check(battle.opponent().GetRaftByID(2)->bp == 40 && battle.syncCounter() == 3,
          "authoritative boss damage");
    game.extension("gs_battle_attack", result);
    check(battle.syncCounter() == 3, "duplicate result ignored");
    O swap;
    swap.put("sync_counter", int32_t(3));
    game.extension("gs_swap_stance", swap);
    check(!battle.attacker(), "stance swap");
    check(
        payload(battle.SendClientReady(), "gs_battle_client_ready").getBool("confirm_swap_stance"),
        "swap confirmation");
    game.extension("gs_battle_client_ready", {});
    payload(battle.defend(3), "gs_battle_defend");
    O bad;
    bad.put("sync_counter", int32_t(99));
    game.extension("gs_battle_attack", bad);
    check(battle.phase() == Phase::Desynced, "reject mismatched sync");
    payload(battle.retreat(), "gs_battle_retreat");
    game.extension("gs_battle_retreat", {});
    check(battle.phase() == Phase::Idle, "retreat resets");
    game.extension("gs_encounter", {});
    check(!payload(battle.encounterResponse(false), "gs_encounter_response").getBool("attack"),
          "decline");
    game.extension("gs_encounter_response", {});
    check(battle.phase() == Phase::Idle, "decline resets");
    game.extension("gs_start_battle", start);
    game.extension("gs_battle_over", {});
    payload(battle.plunder(), "gs_battle_plunder");
    check(!battle.plunder(), "duplicate plunder");
    game.extension("gs_battle_plunder", {});
    check(battle.phase() == Phase::Idle && game.materials.empty(), "no local rewards");
}
