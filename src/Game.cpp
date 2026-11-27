#include "rp/Game.h"
#include "rp/SparseGrid.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace rp {
    using namespace sfs;

    namespace {
        using O = SFSObjectWrapper;
        using A = SFSArrayWrapper;

        int num(const O& o, const std::string& k, int d = 0) {
            auto v = o.get(k);
            if (!v)
                return d;
            if (auto p = std::get_if<int32_t>(v))
                return *p;
            if (auto p = std::get_if<int16_t>(v))
                return *p;
            if (auto p = std::get_if<int8_t>(v))
                return *p;
            if (auto p = std::get_if<bool>(v))
                return *p;
            return d;
        }

        template <class F> void each(const O& o, const std::string& key, F f) {
            auto a = o.getSFSArray(key);
            if (a)
                for (auto& v : a->values)
                    if (auto p = std::get_if<std::shared_ptr<O>>(&v); p && *p)
                        f(**p);
        }

        void delta(Game& g, const O& o) {
            each(o, "materials_changed", [&](auto& v) {
                int id = num(v, "id", num(v, "material"));
                int amt = num(v, "amt", num(v, "amount"));
                if (id > 0)
                    g.giveMaterial(unsigned(id), amt);
            });
        }
    } // namespace

    GridObjectInfo readGridObject(const O& o) {
        auto g = GridObjectInfo::parse(o.getString("data_string"), num(o, "battle_points"));
        g.id = num(o, "object_id");
        g.name = o.getString("name");
        return g;
    }

    Message Game::request(const std::string& cmd, O params) {
        O o;
        o.put("c", cmd);
        o.put("r", int32_t(-1));
        o.put("p", std::make_shared<O>(std::move(params)));
        return {1, 13, std::move(o)};
    }

    Message Game::handshake() const {
        O o;
        o.put("api", "1.1.0");
        o.put("cl", "Java::");
        o.put("bin", true);
        return {0, 0, o};
    }

    Message Game::login(const std::string& id, const std::string& hash) const {
        O o, p;
        p.put("client_version", "1.0.4");
        p.put("last_updated", libraryTimestamp);
        p.put("language", int32_t(0));
        o.put("zn", "RaftPirates");
        o.put("un", id);
        o.put("pw", hash);
        o.put("p", std::make_shared<O>(p));
        return {0, 1, o};
    }

    void Game::accept(const Message& m) {
        if (m.controller == 0) {
            if (m.action == 0) {
                token = m.payload.getString("tk");
                status = "Handshake received";
            } else if (m.action == 1) {
                status = m.payload.get("ec") ? "Login rejected: " + m.payload.getString("em")
                                             : "Logged in; waiting for player";
            }
            return;
        }
        if (m.controller != 1)
            return;
        auto p = m.payload.getSFSObj("p");
        if (p)
            extension(m.payload.getString("c"), *p);
    }

    void Game::extension(const std::string& cmd, const O& p) {
        status = cmd;
        if (p.get("boss_data")) {
            auto boss = p.getSFSObj("boss_data");
            if (boss && boss->getInt("user_id") > 0)
                bossData = *boss;
            else
                bossData.reset();
        }
        if (cmd == "ping_sonar") {
            if (p.getSFSArray("infos")) {
                sonarShips.clear();
                each(p, "infos", [&](const O& info) {
                    if (num(info, "user_id") > 0)
                        sonarShips.push_back(info);
                });
            }
        }
        if (cmd == "ready_for_battle")
            readyForBattle = p.getBool("is_ready", false);
        if (cmd == "ping_sonar" || cmd == "gs_error" || cmd == "gs_encounter")
            sonarPending = false;
        if (cmd == "gs_error") {
            // Legacy generic rejection has no correlation ID. Release pending actions without
            // applying optimistic movement or construction.
            pending.reset();
            pendingMove.reset();
            pendingZone.reset();
            pendingDiscovery.reset();
            pendingMap.reset();
            status = p.getString("message", "Request rejected");
        }
        if (onlineBattle.HandleExtensionResponse(cmd, p, *this))
            return;
        if (cmd == "lib_go_data") {
            gridObjectDB.initWithSFSObject(p);
            objects = gridObjectDB.rows();
        } else if (cmd == "lib_bp_data") {
            blueprintDB.initWithSFSObject(p);
            blueprints = blueprintDB.rows();
        } else if (cmd == "lib_mat_data") {
            materialDB.initWithSFSObject(p);
            materialNames.clear();
            materialGoldCosts.clear();
            for (const auto& row : materialDB.rows()) {
                materialNames[row.first] = row.second.name;
                if (row.second.goldCost)
                    materialGoldCosts[row.first] = *row.second.goldCost;
            }
        } else if (cmd == "lib_quest_type_data") {
            questTypeDB.initWithSFSObject(p);
        } else if (cmd == "lib_store_item_data") {
            storeDB.initWithSFSObject(p, StoreDB::Library::Items);
            storeItems = storeDB.items;
        } else if (cmd == "lib_store_cur_data") {
            storeDB.initWithSFSObject(p, StoreDB::Library::Currencies);
            storeCurrencies = storeDB.currencies;
        } else if (cmd == "lib_store_group_data") {
            storeDB.initWithSFSObject(p, StoreDB::Library::Groups);
            storeGroups = storeDB.groups;
        } else if (cmd == "lib_zone_data") {
            zonesDB.initWithSFSObject(p);
            zoneData = zonesDB.rows();
            zones.clear();
            zoneMaps.clear();
            for (const auto& row : zoneData) {
                zones[row.first] = row.second.name();
                zoneMaps[row.first] = row.second.mapfile();
            }
        } else if (cmd == "lib_discovery_data") {
            discoveryDB.initWithSFSObject(p);
            discoveryData = discoveryDB.rows();
            discoveries.clear();
            for (const auto& row : discoveryData)
                discoveries[row.first] = row.second.name();
        } else if (cmd == "zone_known")
            knownZones = p.getString("zones");
        else if (cmd == "discoveries_known")
            knownDiscoveries = p.getString("discoveries");
        else if (cmd == "gs_player") {
            auto o = p.getSFSObj("player_object");
            if (!o || !o->getSFSArray("rafts") || !o->getSFSArray("materials") ||
                !o->getSFSArray("blueprints"))
                throw std::runtime_error("Incomplete gs_player snapshot");
            initWithSFSObject(*o);
            zone = num(p, "zone_id", 1);
            if (auto map = p.getString("map_name"); !map.empty())
                zoneMaps[zone] = std::move(map);
            mapX = std::clamp(num(p, "map_x"), -95, 95);
            mapY = std::clamp(num(p, "map_y"), -95, 95);
            setSafe(num(p, "safe", 1) != 0);
            daring = num(p, "daring_level");
            daringRemaining = std::max(0, num(p, "daring_timeremaining"));
            daringFraction = 0;
            daringTotal = num(p, "daring_totaltime");
            daringPaused = num(p, "daring_paused") != 0;
            diveState = {};
            diveSeconds = 0;
            diveFraction = 0;
            buildLimit = num(p, "build_limit_left");
            ready = true;
            pending.reset();
            pendingMove.reset();
        } else if (cmd == "zonestatus" || cmd == "zone_status") {
            badZone = num(p, "bad") != 0;
        } else if (cmd == "gs_quest") {
            questCount = 0;
            quests.clear();
            auto result = p.getSFSArray("result");
            if (result)
                for (const auto& entry : result->values) {
                    const O* state = nullptr;
                    const O* definition = nullptr;
                    if (auto item = std::get_if<std::shared_ptr<O>>(&entry); item && *item) {
                        state = item->get();
                        definition = state;
                    } else if (auto pair = std::get_if<std::shared_ptr<A>>(&entry);
                               pair && *pair && (*pair)->values.size() >= 2) {
                        auto a = std::get_if<std::shared_ptr<O>>(&(*pair)->values[0]);
                        auto b = std::get_if<std::shared_ptr<O>>(&(*pair)->values[1]);
                        if (a && *a && b && *b) {
                            state = a->get();
                            definition = b->get();
                        }
                    }
                    if (!state || !definition)
                        continue;
                    ++questCount;
                    Quest q;
                    q.initWithSFSObject(*state, *definition);
                    if (q.id > 0)
                        quests[q.id] = std::move(q);
                }
        } else if (cmd == "quest_progress") {
            auto q = quests.find(num(p, "quest_logid"));
            if (q != quests.end())
                q->second.setProgressPercentage(num(p, "progress", q->second.progress));
        } else if (cmd == "gs_ocean") {
            setSafe(num(p, "safe_water", safe) != 0);
            hideMap = p.getBool("hidemap", false);
            if (pendingMap || p.get("map_x") || p.get("map_y")) {
                mapX = std::clamp(num(p, "map_x", pendingMap ? pendingMap->x : mapX), -95, 95);
                mapY = std::clamp(num(p, "map_y", pendingMap ? pendingMap->y : mapY), -95, 95);
                pendingMap.reset();
            }
            if (p.getString("eventType") == "discovery")
                discovery = num(p, "discovery_id");
        } else if (cmd == "daring_state" || cmd == "daring_advanceresult") {
            if (cmd == "daring_advanceresult" && num(p, "success", 1) == 0) {
                status = "Daring advance rejected";
                return;
            }
            daring = num(p, "level", daring);
            if (p.get("timeremaining")) {
                daringRemaining = std::max(0, num(p, "timeremaining", daringRemaining));
                daringFraction = 0;
            }
            daringTotal = num(p, "totaltime", daringTotal);
            daringPaused = num(p, "paused", daringPaused) != 0;
        } else if (cmd == "gs_materials_changed") {
            delta(*this, p);
            if (p.getBool("diver")) {
                diveState = {};
                diveSeconds = 0;
                diveFraction = 0;
            }
        } else if (cmd == "gs_blueprints_changed")
            each(p, "blueprints_added", [&](auto& x) { giveBlueprint(num(x, "blueprint_id")); });
        else if (cmd == "purchase_response") {
            if (p.getBool("result")) {
                auto b = num(p, "blueprint_id");
                if (b > 0)
                    giveBlueprint(b);
                delta(*this, p);
            } else
                status = "Purchase failed";
        } else if (cmd == "gs_build_add") {
            if (!pending)
                return;
            auto b = *pending;
            pending.reset();
            if (auto o = p.getSFSObj("user_raft_component")) {
                int id = num(*o, "user_raft_component_id");
                if (id <= 0) {
                    status = "Construction rejected";
                    return;
                }
                for (auto& r : rafts)
                    if (r.id == b.parent) {
                        r.components.push_back({id, b.type, b.x, b.y, num(*o, "current_bp"),
                                                num(*o, "state"), num(*o, "time_remaining_secs")});
                        buildLimit = num(*o, "build_limit_left", buildLimit);
                        break;
                    }
            } else if (auto o = p.getSFSObj("user_raft")) {
                int id = num(*o, "user_raft_id");
                if (id > 0 &&
                    AddRaft({id, b.type, b.x, b.y, num(*o, "current_bp"), num(*o, "state"), {}}))
                    buildLimit = num(*o, "build_limit_left", buildLimit);
                else
                    status = "Raft construction rejected";
            } else
                status = "Construction response missing object";
        } else if (cmd == "gs_build_update") {
            if (!pendingMove)
                return;
            auto m = *pendingMove;
            pendingMove.reset();
            if (p.get("success") && !num(p, "success")) {
                status = "Move rejected";
                return;
            }
            auto from = std::find_if(rafts.begin(), rafts.end(),
                                     [&](const auto& r) { return r.id == m.parent; });
            auto to = std::find_if(rafts.begin(), rafts.end(),
                                   [&](const auto& r) { return r.id == m.newParent; });
            if (from == rafts.end() || to == rafts.end())
                return;
            auto c = std::find_if(from->components.begin(), from->components.end(),
                                  [&](const auto& c) { return c.id == m.id; });
            if (c == from->components.end())
                return;
            auto moved = *c;
            moved.x = m.x;
            moved.y = m.y;
            if (from == to)
                *c = moved;
            else {
                from->components.erase(c);
                to->components.push_back(moved);
            }
        } else if (cmd == "gs_build_remove") {
            int id = num(p, "object_id");
            if (num(p, "object_type") == 0)
                rafts.erase(
                    std::remove_if(rafts.begin(), rafts.end(), [&](auto& r) { return r.id == id; }),
                    rafts.end());
            else
                for (auto& r : rafts)
                    if (r.id == num(p, "parent_id"))
                        r.components.erase(std::remove_if(r.components.begin(), r.components.end(),
                                                          [&](auto& c) { return c.id == id; }),
                                           r.components.end());
        } else if (cmd == "dive_info") {
            diveState.initWithSFSObject(p);
            diveSeconds = int(std::ceil(diveState.remaining));
            diveFraction = 0;
        } else if (cmd == "gs_build_complete" || cmd == "gs_use_clock" ||
                   cmd == "gs_build_repair") {
            for (auto& r : rafts) {
                if (num(p, "object_type", 1) == 0 && r.id == num(p, "object_id")) {
                    r.Repair(num(p, "current_bp", num(p, "battle_points", r.bp)),
                             objects.count(r.type) ? objects.at(r.type).bp : r.bp);
                    r.SetState(num(p, "state", r.state));
                }
                if (r.id == num(p, "parent_id"))
                    for (auto& c : r.components)
                        if (c.id == num(p, "object_id")) {
                            c.SetState(num(p, "state", c.state));
                            c.Repair(num(p, "current_bp", num(p, "battle_points", c.bp)),
                                     objects.count(c.type) ? objects.at(c.type).bp : c.bp);
                            if (p.get("time_remaining_secs"))
                                c.SetRemainingTime(num(p, "time_remaining_secs"));
                        }
            }
        } else if (cmd == "build_limit_update")
            buildLimit = num(p, "build_limit_left", buildLimit);
        else if (cmd == "gs_mapactionstate") {
            mapActions = num(p, "count", mapActions);
            mapActionMax = num(p, "maxcount", mapActionMax);
            mapActionTime = num(p, "time", mapActionTime);
        } else if (cmd == "game_vars")
            gameVars = p;
        else if (cmd == "treasuremaps")
            treasureMaps = p.getSFSArray("treasuremaps");
        else if (cmd == "battle_reprieve")
            battleReprieve = std::max(0, num(p, "time_left"));
        else if (cmd == "discovery_goto_result") {
            if (num(p, "success"))
                discovery = num(p, "discoveryid", pendingDiscovery.value_or(0));
            pendingDiscovery.reset();
        } else if (cmd == "dive_fail") {
            diveState = {};
            diveSeconds = 0;
            status = "Dive rejected";
        } else if (cmd == "zone_goto_result") {
            if (num(p, "success")) {
                zone = num(p, "zoneid", pendingZone.value_or(zone));
                sonarShips.clear();
                sonarPending = false;
            } else
                status = "Zone travel rejected";
            pendingZone.reset();
        }
    }

    std::optional<sfs::Message> Game::pingSonar() {
        if (!ready || sonarPending || onlineBattle.phase() != BattleModeState::Phase::Idle)
            return {};
        sonarPending = true;
        return request("ping_sonar");
    }

    bool Game::canPlaceComponent(int object, int parent, int x, int y, int ignoreParent,
                                 int ignoreId) const {
        auto o = objects.find(object);
        auto r = std::find_if(rafts.begin(), rafts.end(),
                              [&](const auto& raft) { return raft.id == parent; });
        return o != objects.end() && r != rafts.end() &&
               r->CanAddObject(o->second, {x, y}, objects, parent == ignoreParent ? ignoreId : 0);
    }

    Message Game::moveComponent(int parent, int id, int newParent, int x, int y) {
        if (!ready || pendingMove || pending)
            throw std::runtime_error("Wait for construction response");
        auto raft =
            std::find_if(rafts.begin(), rafts.end(), [&](const auto& r) { return r.id == parent; });
        if (raft == rafts.end())
            throw std::runtime_error("Unknown parent raft");
        auto part = std::find_if(raft->components.begin(), raft->components.end(),
                                 [&](const auto& c) { return c.id == id; });
        if (part == raft->components.end())
            throw std::runtime_error("Unknown component");
        if (!canPlaceComponent(part->type, newParent, x, y, parent, id))
            throw std::runtime_error("Invalid move footprint");
        O p;
        p.put("object_type", int32_t(1));
        p.put("object_id", int32_t(id));
        p.put("parent_id", int32_t(parent));
        p.put("new_parent_id", int32_t(newParent));
        p.put("pos_x", int32_t(x));
        p.put("pos_y", int32_t(y));
        pendingMove = PendingMove{parent, id, newParent, x, y};
        return request("gs_build_update", p);
    }

    Message Game::build(int object, int parent, int x, int y) {
        if (!ready || pending || pendingMove)
            throw std::runtime_error("Wait for previous construction");
        auto o = objects.find(object);
        if (o == objects.end())
            throw std::runtime_error("Unknown grid object");
        if (!ownsObject(object))
            throw std::runtime_error("Blueprint not owned");
        if (o->second.type == 1) {
            if (!canPlaceComponent(object, parent, x, y))
                throw std::runtime_error("Footprint outside raft or occupied");
        } else {
            parent = 0;
            if (!canBuildRaft({x, y}))
                throw std::runtime_error("Raft must attach to an existing hull edge");
        }
        pending = PendingBuild{object, parent, x, y};
        O p;
        p.put("object_id", int32_t(object));
        p.put("pos_x", int32_t(x));
        p.put("pos_y", int32_t(y));
        if (parent)
            p.put("parent_id", int32_t(parent));
        return request("gs_build_add", p);
    }

    Message Game::remove(int parent, int id) const {
        O p;
        p.put("object_type", int32_t(parent ? 1 : 0));
        p.put("object_id", int32_t(id));
        if (parent)
            p.put("parent_id", int32_t(parent));
        return request("gs_build_remove", p);
    }

    Message Game::dive(const std::string& command) const {
        if (command != "dive_start" && command != "dive_info" && command != "dive_collect")
            throw std::runtime_error("Unknown dive action");
        const auto unit = GetDiveUnit();
        if (unit.second) {
            O p;
            p.put("parent_id", int32_t(unit.first));
            p.put("object_id", int32_t(unit.second));
            return request(command, p);
        }
        throw std::runtime_error("No diving unit");
    }

    Message Game::travel(int id) {
        if (!zones.count(id) || pendingZone)
            throw std::runtime_error("Invalid/pending zone");
        pendingZone = id;
        O p;
        p.put("zoneid", int32_t(id));
        return request("zone_goto", p);
    }

    Message Game::buy(int id) const {
        auto b = blueprints.find(id);
        if (b == blueprints.end())
            throw std::runtime_error("Unknown blueprint");
        O p;
        p.put("itemname", b->second.name());
        return request("purchase_item", p);
    }

    Message Game::buyStoreItem(int id) const {
        auto i = storeItems.find(id);
        if (i == storeItems.end() || i->second.name.empty())
            throw std::runtime_error("Unknown store item");
        O p;
        p.put("itemname", i->second.name);
        return request("purchase_item", p);
    }

    Message Game::openQuest(int id) {
        auto q = quests.find(id);
        if (q == quests.end())
            throw std::runtime_error("Unknown quest");
        q->second.markOpened();
        O p;
        p.put("quest_id", int32_t(id));
        return request("gs_quest_read", p);
    }

    Message Game::discover(int id) {
        if (!discoveries.count(id) || pendingDiscovery)
            throw std::runtime_error("Invalid/pending discovery");
        pendingDiscovery = id;
        O p;
        p.put("discoveryid", int32_t(id));
        return request("discovery_goto", p);
    }

    Message Game::moveMap(int x, int y, bool destinationSafe, int enemy, bool usedSonar) {
        if (pendingMap)
            throw std::runtime_error("Wait for previous movement");
        x = std::clamp(x, -95, 95);
        y = std::clamp(y, -95, 95);
        pendingMap = Cell{x, y};
        O p;
        p.put("map_x", int32_t(x));
        p.put("map_y", int32_t(y));
        p.put("safe", destinationSafe);
        p.put("treasure_id", int32_t(0));
        p.put("enemy_id", int32_t(std::max(0, enemy)));
        p.put("usedSonar", int32_t(usedSonar));
        return request("gs_map_move", p);
    }

    bool Game::daringLocked() const {
        auto info = zoneData.find(zone);
        bool limited = info != zoneData.end() && info->second.daringLimit() > 0 &&
                       daring >= info->second.daringLimit();
        return safe || badZone || daringPaused || limited;
    }

    bool Game::canAdvanceDaring() const {
        return !daringLocked() && daringTotal > 0 && daringRemaining == 0;
    }

    double Game::daringProgress() const {
        if (daringLocked() || daringTotal <= 0)
            return 0;
        return std::clamp(1.0 - double(daringRemaining) / daringTotal, 0.0, 1.0);
    }

    void Game::tick(float dt) {
        if (!std::isfinite(dt) || dt <= 0)
            return;
        // The server remains authoritative; only the local countdown advances here.
        if (!daringPaused && daringRemaining > 0) {
            double elapsed = double(daringFraction) + dt;
            if (elapsed >= daringRemaining) {
                daringRemaining = 0;
                daringFraction = 0;
            } else {
                int seconds = int(elapsed);
                daringRemaining -= seconds;
                daringFraction = float(elapsed - seconds);
            }
        }

        if (diveState.Diving()) {
            diveState.TempTick(dt);
            diveSeconds = int(std::ceil(diveState.remaining));
        } else if (diveSeconds > 0) { // Compatibility for old test fixtures and callers.
            diveFraction += std::min(dt, 1.f);
            if (diveFraction >= 1) {
                int seconds = int(diveFraction);
                diveFraction -= seconds;
                diveSeconds = std::max(0, diveSeconds - seconds);
            }
        }
        for (auto& raft : rafts)
            for (auto& component : raft.components)
                if (component.Building())
                    component.TempTick(dt);
    }

    std::string Game::summary() const {
        std::ostringstream o;
        o << status << " | " << playerName << " | Zone " << zone << " | Daring " << daring << " | "
          << (safe ? "Safe" : "Pirate") << " waters\n"
          << rafts.size() << " rafts | Build limit " << buildLimit << " | Dive " << diveSeconds
          << " s\n";
        for (auto [id, n] : materials) {
            auto it = materialNames.find(id);
            o << (it == materialNames.end() ? std::to_string(id) : it->second) << ": " << n << "  ";
        }
        return o.str();
    }

    std::string Game::catalog() const {
        std::ostringstream o;
        for (auto& kv : blueprints) {
            auto& b = kv.second;
            o << b.id() << '\t' << b.object << '\t' << (owned.count(b.id()) ? 1 : 0) << '\t'
              << b.name() << '\n';
        }
        return o.str();
    }
} // namespace rp
