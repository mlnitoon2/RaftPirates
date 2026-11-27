#include "rp/Player.h"
#include "rp/Material.h"
#include <algorithm>
#include <limits>
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace rp {
    namespace {
        using O = sfs::SFSObjectWrapper;

        int num(const O& o, const std::string& k, int fallback = 0) {
            auto v = o.get(k);
            if (!v)
                return fallback;
            if (auto n = std::get_if<int32_t>(v))
                return *n;
            if (auto n = std::get_if<int16_t>(v))
                return *n;
            if (auto n = std::get_if<int8_t>(v))
                return *n;
            if (auto n = std::get_if<bool>(v))
                return *n;
            return fallback;
        }

        template <class F> void each(const O& o, const std::string& key, F f) {
            auto a = o.getSFSArray(key);
            if (a)
                for (auto& v : a->values)
                    if (auto p = std::get_if<std::shared_ptr<O>>(&v); p && *p)
                        f(**p);
        }

    } // namespace

    void RaftComponent::initWithSFSObject(const O& data) {
        id = num(data, "user_raft_component_id");
        type = num(data, "component");
        x = num(data, "pos_x");
        y = num(data, "pos_y");
        bp = num(data, "current_bp");
        state = num(data, "state");
        SetRemainingTime(num(data, "time_remaining_secs"));
    }

    void Raft::initWithSFSObject(const O& data) {
        id = num(data, "user_raft_id");
        type = num(data, "raft");
        x = num(data, "pos_x");
        y = num(data, "pos_y");
        bp = num(data, "current_bp");
        state = num(data, "state");
        bonusBattlePoints = 0;
        components.clear();
        each(data, "components", [&](const O& item) {
            Component component;
            component.initWithSFSObject(item);
            components.push_back(std::move(component));
        });
    }

    Player::Player() {
        init();
    }

    void Player::init() {
        materials.clear();
        owned.clear();
        rafts.clear();
        playerName.clear();
        knownZones.clear();
        safe = true;
    }

    void Player::initWithSFSObject(const O& data) {
        std::vector<Raft> nextRafts;
        std::map<int, int> nextMaterials;
        std::set<int> nextOwned;
        each(data, "rafts", [&](const O& x) {
            Raft raft;
            raft.initWithSFSObject(x);
            nextRafts.push_back(std::move(raft));
        });
        each(data, "materials", [&](const O& x) {
            Material m;
            m.initWithSFSObject(x);
            nextMaterials[m.id()] = m.amount();
        });
        each(data, "blueprints", [&](const O& x) { nextOwned.insert(num(x, "blueprint")); });
        rafts = std::move(nextRafts);
        materials = std::move(nextMaterials);
        owned = std::move(nextOwned);
        playerName = data.getString("display_name");
    }

    Cell Player::EastMostPoint() const {
        const Raft* result = mainRaft();
        for (const auto& raft : rafts)
            if (!result || raft.x > result->x)
                result = &raft;
        return result ? result->GetGridPosition() : Cell{};
    }

    Cell Player::WestMostPoint() const {
        const Raft* result = mainRaft();
        for (const auto& raft : rafts)
            if (!result || raft.x < result->x)
                result = &raft;
        return result ? result->GetGridPosition() : Cell{};
    }

    std::optional<int> Player::goldCostToBuild(int blueprintId) const {
        auto it = blueprints.find(blueprintId);
        if (it == blueprints.end())
            return {};
        double total = 0;
        for (const auto& req : it->second.recipe) {
            int missing = std::max(0, req.second - numOfMaterialPossessed(unsigned(req.first)));
            if (!missing)
                continue;
            auto price = materialGoldCosts.find(req.first);
            if (price == materialGoldCosts.end() || !std::isfinite(price->second) ||
                price->second < 0)
                return {};
            total += std::ceil(missing * price->second);
            if (total > std::numeric_limits<int>::max())
                return {};
        }
        return int(total);
    }

    int Player::currentBP() const {
        long long total = 0;
        for (const auto& r : rafts)
            total += r.GetBattlePoints();
        return int(std::min(total, static_cast<long long>(std::numeric_limits<int>::max())));
    }

    bool Player::isSafe() const {
        return safe;
    }

    void Player::setSafe(bool value) {
        safe = value;
    }

    Raft* Player::GetRaftByID(int id) {
        for (auto& r : rafts)
            if (r.id == id)
                return &r;
        return nullptr;
    }

    const Raft* Player::GetRaftByID(int id) const {
        for (const auto& r : rafts)
            if (r.id == id)
                return &r;
        return nullptr;
    }

    const Raft* Player::GetRaftAtPosition(Cell p) const {
        for (const auto& r : rafts)
            if (r.x == p.x && r.y == p.y)
                return &r;
        return nullptr;
    }

    const Raft* Player::mainRaft() const {
        for (const auto& r : rafts) {
            auto i = objects.find(r.type);
            if (i != objects.end() && i->second.name == "CoreRaft")
                return &r;
        }
        return nullptr;
    }

    bool Player::hasRaft(int id) const {
        return GetRaftByID(id) != nullptr;
    }

    std::vector<Cell> Player::GetAvailableEdgeGridSpace() const {
        std::set<std::pair<int, int>> cells;
        for (auto& r : rafts)
            if (!r.Destroyed())
                for (auto d : {Cell{1, 0}, Cell{-1, 0}, Cell{0, 1}, Cell{0, -1}}) {
                    long long x = static_cast<long long>(r.x) + d.x,
                              y = static_cast<long long>(r.y) + d.y;
                    if (x < std::numeric_limits<int>::min() ||
                        x > std::numeric_limits<int>::max() ||
                        y < std::numeric_limits<int>::min() || y > std::numeric_limits<int>::max())
                        continue;
                    if (!GetRaftAtPosition({int(x), int(y)}))
                        cells.emplace(int(x), int(y));
                }
        std::vector<Cell> result;
        for (auto p : cells)
            result.push_back({p.first, p.second});
        return result;
    }

    bool Player::canBuildRaft(Cell pos) const {
        auto spaces = GetAvailableEdgeGridSpace();
        return std::any_of(spaces.begin(), spaces.end(),
                           [&](Cell p) { return p.x == pos.x && p.y == pos.y; });
    }

    std::vector<int> Player::RaftsConnectedToRaft(int id) const {
        std::vector<int> result;
        if (!GetRaftByID(id))
            return result;
        result.push_back(id);
        for (size_t i = 0; i < result.size(); ++i) {
            auto a = GetRaftByID(result[i]);
            for (auto& b : rafts)
                if (std::abs(static_cast<long long>(a->x) - b.x) +
                            std::abs(static_cast<long long>(a->y) - b.y) ==
                        1 &&
                    std::find(result.begin(), result.end(), b.id) == result.end())
                    result.push_back(b.id);
        }
        return result;
    }

    std::set<std::string> Player::getResourceFiles() const {
        std::set<std::string> files;
        auto add = [&](int id) {
            auto i = objects.find(id);
            if (i == objects.end())
                return;
            if (!i->second.asset.empty())
                files.insert(i->second.asset);
            for (auto& p : i->second.pieces)
                if (!p.asset.empty())
                    files.insert(p.asset);
        };
        for (auto& r : rafts) {
            add(r.type);
            for (auto& c : r.components)
                add(c.type);
        }
        return files;
    }

    bool Player::AddRaft(Raft r) {
        if (r.id <= 0 || hasRaft(r.id) || GetRaftAtPosition({r.x, r.y}))
            return false;
        rafts.push_back(std::move(r));
        return true;
    }

    bool Player::RemoveRaft(int id) {
        auto n = rafts.size();
        rafts.erase(
            std::remove_if(rafts.begin(), rafts.end(), [&](const auto& r) { return r.id == id; }),
            rafts.end());
        return n != rafts.size();
    }

    int Player::numOfMaterialPossessed(unsigned id) const {
        if (id > unsigned(std::numeric_limits<int>::max()))
            return 0;
        auto i = materials.find(int(id));
        return i == materials.end() ? 0 : i->second;
    }

    int Player::numOfMaterialPossessed(const std::string& name) const {
        for (const auto& m : materialNames)
            if (m.second == name)
                return numOfMaterialPossessed(unsigned(m.first));
        return 0;
    }

    bool Player::hasMaterial(int id, int count) const {
        return id > 0 && count >= 0 && numOfMaterialPossessed(unsigned(id)) >= count;
    }

    bool Player::hasMaterial(const std::string& name, int count) const {
        return count >= 0 && numOfMaterialPossessed(name) >= count;
    }

    void Player::giveMaterial(unsigned id, int count) {
        if (!id || id > unsigned(std::numeric_limits<int>::max()))
            return;
        auto& amount = materials[int(id)];
        amount = int(std::clamp(static_cast<long long>(amount) + count, 0ll,
                                static_cast<long long>(std::numeric_limits<int>::max())));
    }

    void Player::giveBlueprint(int id) {
        if (id > 0)
            owned.insert(id);
    }

    bool Player::hasBlueprint(const std::string& name) const {
        for (const auto& b : blueprints)
            if (b.second.name() == name && owned.count(b.first))
                return true;
        return false;
    }

    bool Player::ownsObject(int id) const {
        for (int b : owned) {
            auto i = blueprints.find(b);
            if (i != blueprints.end() && i->second.object == id)
                return true;
        }
        return false;
    }

    bool Player::RaftRepairRequired() const {
        for (const auto& r : rafts)
            if (!r.Destroyed() && r.GetHealth(objects) < 1)
                return true;
        return false;
    }

    bool Player::RaftCleanupRequired() const {
        for (const auto& r : rafts)
            if (r.Destroyed())
                return true;
        return false;
    }

    bool Player::isZoneKnown(unsigned id) const {
        std::string text = knownZones;
        for (char& c : text)
            if (c == ',' || c == ';' || c == '|')
                c = ' ';
        std::istringstream stream(text);
        unsigned n;
        while (stream >> n)
            if (n == id)
                return true;
        return false;
    }

    std::pair<int, int> Player::GetDiveUnit() const {
        for (const auto& r : rafts)
            for (const auto& c : r.components) {
                auto o = objects.find(c.type);
                if (o == objects.end() || c.Destroyed())
                    continue;
                const auto& def = o->second;
                auto isDiveAsset = [](const std::string& asset) {
                    return asset.find("anim_core_dive_unit") != std::string::npos;
                };
                if (def.name == "DivingUnit" || isDiveAsset(def.asset) ||
                    std::any_of(def.pieces.begin(), def.pieces.end(),
                                [&](const auto& piece) { return isDiveAsset(piece.asset); }))
                    return {r.id, c.id};
            }
        return {};
    }

    std::pair<Cell, Cell> Player::getBoundingBox() const {
        if (rafts.empty())
            return {{0, 0}, {0, 0}};
        Cell low{rafts.front().x, rafts.front().y}, high = low;
        for (const auto& raft : rafts) {
            low.x = std::min(low.x, raft.x);
            low.y = std::min(low.y, raft.y);
            high.x = std::max(high.x, raft.x);
            high.y = std::max(high.y, raft.y);
        }
        return {low, high};
    }

} // namespace rp
