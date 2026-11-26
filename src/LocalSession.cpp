#include "rp/LocalSession.h"
#include <algorithm>
#include <stdexcept>

namespace rp {
    namespace {
        using O = sfs::SFSObjectWrapper;
        using A = sfs::SFSArrayWrapper;

        int number(const O& obj, const std::string& key) {
            auto v = obj.get(key);
            if (!v)
                return 0;
            if (auto n = std::get_if<int32_t>(v))
                return *n;
            return 0;
        }
    } // namespace

    void LocalSession::start(Game& g) {
        nextId_ = 10;
        g.demo();
        g.safe = false;
        g.daringTotal = 60;
        g.daringRemaining = 60;
        g.materialNames[2] = "Wood";
        g.materialNames[3] = "Metal";
        g.materials[2] = 1000;
        g.materials[3] = 1000;
        g.storeItems[1] = {1, 4, 100, 1, "random_pack_sm", "misc", "Salvage pack"};
        g.blueprints[2] = {2, 2, 1, "Boom-Boom Buddha"};
        g.storeItems[2] = {2, 4, 100, 1, "Boom-Boom Buddha", "blueprints", "Boom-Boom Buddha"};
        g.zones[2] = "The Big Blue";
        g.zoneMaps[2] = "xml_resources/maps/map_big_blue.tmx";
        g.status = "LOCAL SANDBOX";
    }

    void LocalSession::dispatch(Game& g, const sfs::Message& message) {
        const auto cmd = message.payload.getString("c");
        auto p = message.payload.getSFSObj("p");
        if (!p)
            throw std::runtime_error("Missing local request data");
        O response;
        if (cmd == "gs_build_add") {
            if (!g.pending)
                return;
            const auto build = *g.pending;
            auto info = g.objects.find(build.type);
            if (info == g.objects.end())
                throw std::runtime_error("Unknown object");
            auto obj = std::make_shared<O>();
            obj->put(build.parent ? "user_raft_component_id" : "user_raft_id", int32_t(nextId_++));
            obj->put("current_bp", int32_t(info->second.bp));
            obj->put("state", int32_t(0));
            obj->put("time_remaining_secs", int32_t(0));
            obj->put("build_limit_left", int32_t(std::max(0, g.buildLimit - 1)));
            response.put(build.parent ? "user_raft_component" : "user_raft", obj);
            g.extension(cmd, response);
        } else if (cmd == "gs_build_update") {
            response.put("success", int32_t(1));
            g.extension(cmd, response);
        } else if (cmd == "gs_build_remove") {
            g.extension(cmd, *p);
        } else if (cmd == "purchase_item") {
            const auto name = p->getString("itemname");
            bool bought = false;
            int bp = 0;
            auto delta = std::make_shared<A>();
            auto change = [&](int id, int amt) {
                auto d = std::make_shared<O>();
                d->put("id", int32_t(id));
                d->put("amt", int32_t(amt));
                delta->add(d);
            };
            for (const auto& [id, item] : g.storeItems)
                if (item.name == name && g.hasMaterial(item.currency, item.price)) {
                    if (item.group == "blueprints") {
                        for (const auto& [bid, b] : g.blueprints)
                            if (b.name() == name && !g.owned.count(bid))
                                bp = bid;
                        if (!bp)
                            break;
                    } else if (name == "random_pack_sm") {
                        change(2, 100);
                        change(3, 100);
                    } else
                        break;
                    bought = true;
                    change(item.currency, -item.price);
                    break;
                }
            response.put("result", bought);
            response.put("blueprint_id", int32_t(bp));
            response.put("materials_changed", delta);
            g.extension("purchase_response", response);
        } else if (cmd == "dive_start" || cmd == "dive_info") {
            response = *p;
            response.put("state", int32_t(cmd == "dive_start" ? 3 : g.diveState.state));
            response.put("depth", int32_t(1));
            response.put("max_depth", int32_t(10));
            response.put("loot_level", int32_t(1));
            response.put("time_remaining_secs", int32_t(20));
            response.put("time_total_secs", int32_t(20));
            g.extension("dive_info", response);
        } else if (cmd == "dive_collect") {
            if (g.diveState.state != 3) {
                g.status = "Start a dive first";
                return;
            }
            auto delta = std::make_shared<A>();
            auto d = std::make_shared<O>();
            d->put("id", int32_t(2));
            d->put("amt", int32_t(50));
            delta->add(d);
            response.put("materials_changed", delta);
            g.extension("gs_materials_changed", response);
            g.diveState = {};
            g.diveSeconds = 0;
        } else if (cmd == "gs_map_move") {
            response.put("safe_water", p->getBool("safe"));
            g.extension("gs_ocean", response);
        } else if (cmd == "zone_goto") {
            response.put("success", int32_t(1));
            response.put("zoneid", int32_t(number(*p, "zoneid")));
            g.extension("zone_goto_result", response);
        } else if (cmd == "daring_advance") {
            response.put("success", int32_t(g.canAdvanceDaring()));
            response.put("level", int32_t(g.daring + 1));
            response.put("timeremaining", int32_t(60));
            response.put("totaltime", int32_t(60));
            g.extension("daring_advanceresult", response);
        } else if (cmd == "gs_quest") {
            response.put("result", std::make_shared<A>());
            g.extension(cmd, response);
        } else
            throw std::runtime_error("Local action not implemented: " + cmd);
    }
} // namespace rp
