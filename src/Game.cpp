#include "rp/Game.h"
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

        Component component(const O& o) {
            return {num(o, "user_raft_component_id"),
                    num(o, "component"),
                    num(o, "pos_x"),
                    num(o, "pos_y"),
                    num(o, "current_bp"),
                    num(o, "state"),
                    num(o, "time_remaining_secs")};
        }

        Raft raft(const O& o) {
            Raft r{num(o, "user_raft_id"),
                   num(o, "raft"),
                   num(o, "pos_x"),
                   num(o, "pos_y"),
                   num(o, "current_bp"),
                   num(o, "state"),
                   {}};
            each(o, "components", [&](auto& x) { r.components.push_back(component(x)); });
            return r;
        }

        void delta(Game& g, const O& o) {
            each(o, "materials_changed", [&](auto& v) {
                int id = num(v, "id", num(v, "material"));
                int amt = num(v, "amt", num(v, "amount"));
                if (id > 0)
                    g.materials[id] += amt;
            });
        }
    } // namespace

    GridObject readGridObject(const O& o) {
        GridObject g;
        g.id = num(o, "object_id");
        g.bp = num(o, "battle_points");
        g.name = o.getString("name");
        std::istringstream data(o.getString("data_string"));
        std::string piece;
        if (std::getline(data, piece, ';') && !piece.empty())
            g.type = std::stoi(piece);
        while (std::getline(data, piece, ';')) {
            if (piece.empty())
                continue;
            GridPiece part;
            std::istringstream s(piece);
            if (!(s >> part.asset >> part.offsetX >> part.offsetY))
                throw std::runtime_error("Invalid grid object piece");
            std::string xy;
            while (s >> xy) {
                auto comma = xy.find(',');
                if (comma == std::string::npos)
                    throw std::runtime_error("Invalid grid cell");
                Cell c{std::stoi(xy.substr(0, comma)), std::stoi(xy.substr(comma + 1))};
                part.cells.push_back(c);
                if (std::none_of(g.cells.begin(), g.cells.end(),
                                 [&](Cell a) { return a.x == c.x && a.y == c.y; }))
                    g.cells.push_back(c);
            }
            if (g.pieces.empty()) {
                g.asset = part.asset;
                g.offsetX = part.offsetX;
                g.offsetY = part.offsetY;
            }
            g.pieces.push_back(std::move(part));
        }
        if (g.cells.empty())
            g.cells.push_back({0, 0});
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
        p.put("client_version", "1.0.0");
        p.put("last_updated", int64_t(0));
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
        if (cmd == "lib_go_data") {
            objects.clear();
            each(p, "gridobject_data", [&](auto& x) {
                auto g = readGridObject(x);
                objects[g.id] = g;
            });
        } else if (cmd == "lib_bp_data") {
            blueprints.clear();
            each(p, "blueprint_data", [&](auto& x) {
                Blueprint b{num(x, "blueprint_id"), num(x, "object_id"), num(x, "unlock_level"),
                            x.getString("name")};
                blueprints[b.id] = b;
            });
        } else if (cmd == "lib_mat_data") {
            materialNames.clear();
            each(p, "material_data",
                 [&](auto& x) { materialNames[num(x, "material_id")] = x.getString("name"); });
        } else if (cmd == "lib_store_item_data") {
            storeItems.clear();
            each(p, "store_item_data", [&](auto& x) {
                StoreItem i{num(x, "storeitem_id"),   num(x, "currency_id"),
                            num(x, "price"),          num(x, "amount"),
                            x.getString("item_name"), x.getString("group_name"),
                            x.getString("item_title")};
                storeItems[i.id] = std::move(i);
            });
        } else if (cmd == "lib_store_cur_data") {
            storeCurrencies.clear();
            each(p, "store_currency_data", [&](auto& x) {
                StoreCurrency c{num(x, "storecur_id"), num(x, "starting_amount"),
                                x.getString("currency_name")};
                storeCurrencies[c.id] = std::move(c);
            });
        } else if (cmd == "lib_store_group_data") {
            storeGroups.clear();
            each(p, "store_group_data", [&](auto& x) {
                storeGroups[num(x, "storegroup_id")] = x.getString("group_name");
            });
        } else if (cmd == "lib_zone_data") {
            zones.clear();
            zoneMaps.clear();
            each(p, "zone_data", [&](auto& x) {
                int id = num(x, "zone_id");
                zones[id] = x.getString("name");
                zoneMaps[id] = x.getString("mapfile");
            });
        } else if (cmd == "lib_discovery_data") {
            discoveries.clear();
            each(p, "discovery_data",
                 [&](auto& x) { discoveries[num(x, "id")] = x.getString("name"); });
        } else if (cmd == "zone_known")
            knownZones = p.getString("zones");
        else if (cmd == "discoveries_known")
            knownDiscoveries = p.getString("discoveries");
        else if (cmd == "gs_player") {
            auto o = p.getSFSObj("player_object");
            if (!o)
                throw std::runtime_error("gs_player lacks player_object");
            rafts.clear();
            materials.clear();
            owned.clear();
            playerName = o->getString("display_name");
            each(*o, "rafts", [&](auto& x) { rafts.push_back(raft(x)); });
            each(*o, "materials",
                 [&](auto& x) { materials[num(x, "material")] = num(x, "amount"); });
            each(*o, "blueprints", [&](auto& x) { owned.insert(num(x, "blueprint")); });
            zone = num(p, "zone_id", 1);
            mapX = std::clamp(num(p, "map_x"), -95, 95);
            mapY = std::clamp(num(p, "map_y"), -95, 95);
            safe = num(p, "safe", 1) != 0;
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
        } else if (cmd == "gs_ocean") {
            safe = p.getBool("safe_water", safe);
            if (pendingMap) {
                mapX = pendingMap->x;
                mapY = pendingMap->y;
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
        } else if (cmd == "gs_materials_changed")
            delta(*this, p);
        else if (cmd == "gs_blueprints_changed")
            each(p, "blueprints_added", [&](auto& x) { owned.insert(num(x, "blueprint_id")); });
        else if (cmd == "purchase_response") {
            if (p.getBool("result")) {
                auto b = num(p, "blueprint_id");
                if (b > 0)
                    owned.insert(b);
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
                if (id > 0)
                    rafts.push_back(
                        {id, b.type, b.x, b.y, num(*o, "current_bp"), num(*o, "state"), {}});
            } else
                status = "Construction response missing object";
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
            diveState = {
                num(p, "parent_id"),          num(p, "object_id"),      num(p, "state"),
                num(p, "loot_level"),         num(p, "depth"),          num(p, "max_depth"),
                num(p, "material_rich_zone"), num(p, "time_total_secs")};
            diveSeconds = std::max(0, num(p, "time_remaining_secs"));
            diveFraction = 0;
        } else if (cmd == "gs_build_complete" || cmd == "gs_use_clock" ||
                   cmd == "gs_build_repair") {
            for (auto& r : rafts) {
                if (num(p, "object_type", 1) == 0 && r.id == num(p, "object_id")) {
                    r.bp = num(p, "battle_points", r.bp);
                    r.state = num(p, "state", r.state);
                }
                if (r.id == num(p, "parent_id"))
                    for (auto& c : r.components)
                        if (c.id == num(p, "object_id")) {
                            c.state = num(p, "state", c.state);
                            c.bp = num(p, "battle_points", c.bp);
                            if (p.get("time_remaining_secs"))
                                c.SetRemainingTime(num(p, "time_remaining_secs"));
                        }
            }
        } else if (cmd == "build_limit_update")
            buildLimit = num(p, "build_limit_left", buildLimit);
        else if (cmd == "gs_mapactionstate")
            mapActions = num(p, "count", mapActions);
        else if (cmd == "discovery_goto_result") {
            if (num(p, "success"))
                discovery = num(p, "discoveryid", pendingDiscovery.value_or(0));
            pendingDiscovery.reset();
        } else if (cmd == "dive_fail")
            status = "Dive rejected";
        else if (cmd == "zone_goto_result") {
            if (num(p, "success"))
                zone = num(p, "zoneid", pendingZone.value_or(zone));
            else
                status = "Zone travel rejected";
            pendingZone.reset();
        }
    }

    Message Game::build(int object, int parent, int x, int y) {
        if (!ready || pending)
            throw std::runtime_error("Wait for previous construction");
        auto o = objects.find(object);
        if (o == objects.end())
            throw std::runtime_error("Unknown grid object");
        bool has = false;
        for (auto id : owned) {
            auto bp = blueprints.find(id);
            if (bp != blueprints.end() && bp->second.object == object)
                has = true;
        }
        if (!has)
            throw std::runtime_error("Blueprint not owned");
        if (o->second.type == 1) {
            auto r =
                std::find_if(rafts.begin(), rafts.end(), [&](auto& a) { return a.id == parent; });
            if (r == rafts.end())
                throw std::runtime_error("Unknown parent raft");
            for (auto cell : o->second.cells) {
                int xx = x + cell.x, yy = y + cell.y;
                if (xx < 0 || yy < 0 || xx >= 6 || yy >= 6)
                    throw std::runtime_error("Footprint outside raft");
                for (auto& c : r->components) {
                    auto other = objects.find(c.type);
                    if (other == objects.end()) {
                        if (c.x == xx && c.y == yy)
                            throw std::runtime_error("Cell occupied");
                    } else
                        for (auto v : other->second.cells)
                            if (c.x + v.x == xx && c.y + v.y == yy)
                                throw std::runtime_error("Cell occupied");
                }
            }
        } else {
            parent = 0;
            for (auto& r : rafts)
                if (r.x == x && r.y == y)
                    throw std::runtime_error("Raft position occupied");
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
        for (auto& r : rafts)
            for (auto& c : r.components) {
                auto o = objects.find(c.type);
                if (o != objects.end() && o->second.name == "DivingUnit") {
                    O p;
                    p.put("parent_id", int32_t(r.id));
                    p.put("object_id", int32_t(c.id));
                    return request(command, p);
                }
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
        p.put("itemname", b->second.name);
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

    Message Game::discover(int id) {
        if (!discoveries.count(id) || pendingDiscovery)
            throw std::runtime_error("Invalid/pending discovery");
        pendingDiscovery = id;
        O p;
        p.put("discoveryid", int32_t(id));
        return request("discovery_goto", p);
    }

    Message Game::moveMap(int x, int y, bool destinationSafe) {
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
        p.put("enemy_id", int32_t(0));
        p.put("usedSonar", int32_t(0));
        return request("gs_map_move", p);
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

        diveFraction += std::min(dt, 1.f);
        if (diveFraction >= 1) {
            int seconds = int(diveFraction);
            diveFraction -= seconds;
            diveSeconds = std::max(0, diveSeconds - seconds);
        }
        for (auto& raft : rafts)
            for (auto& component : raft.components)
                if (component.Building())
                    component.TempTick(dt);
    }

    void Game::demo() {
        *this = Game{};
        objects[1] = {1, 0, 1000, "CoreRaft", "gfx/rafts/raft_core.png", -192, 0, {{0, 0}}};
        objects[2] = {2, 1, 1000,    "boom_buddha", "xml_bin/anim_boom_buddha_robot.bin",
                      0, 0, {{0, 0}}};
        objects[3] = {3, 1, 1000, "DivingUnit", "xml_bin/anim_core_dive_unit.bin", 0, 0, {{0, 0}}};
        objects[4] = {4,
                      1,
                      1000,
                      "pain_sprinkler",
                      "xml_bin/anim_pain_sprinkler_front.bin",
                      0,
                      0,
                      {{0, 1}, {1, 0}, {1, 1}, {1, 2}, {2, 1}}};
        blueprints[1] = {1, 4, 1, "Pain Sprinkler"};
        owned.insert(1);
        rafts.push_back(
            {1, 1, 0, 0, 1000, 0, {{1, 2, 4, 1, 1000, 0, 0}, {2, 3, 1, 3, 1000, 0, 0}}});
        playerName = "Offline demo";
        materials[4] = 99999;
        materialNames[4] = "Gold";
        zones[1] = "Sailor's Sea";
        zoneMaps[1] = "xml_resources/maps/map_sailers_sea.tmx";
        mapX = 9;
        mapY = 21;
        daring = 1;
        buildLimit = 100;
        ready = true;
        status = "Offline preview (not connected)";
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
            o << b.id << '\t' << b.object << '\t' << (owned.count(b.id) ? 1 : 0) << '\t' << b.name
              << '\n';
        }
        return o.str();
    }
} // namespace rp
