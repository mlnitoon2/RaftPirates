#include "rp/DivingUnit.h"
#include "rp/Game.h"
#include "rp/DiveMeterSprite.h"
#include "rp/DiveUnitReadoutSprite.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace rp {
    namespace {
        int num(const sfs::SFSObjectWrapper& o, const char* k, int fallback = 0) {
            auto v = o.get(k);
            if (!v)
                return fallback;
            if (auto n = std::get_if<int32_t>(v))
                return *n;
            return fallback;
        }

        int sum(const std::map<int, int>& values) {
            long long total = 0;
            for (auto& p : values)
                total += std::max(0, p.second);
            return int(std::min<long long>(total, std::numeric_limits<int>::max()));
        }
    } // namespace

    DivingUnit::DivingUnit() = default;

    void DivingUnit::AddedToOcean(OceanView& ocean) const {
        // Game::tick owns countdown advancement; this adapter broadcasts current visual state.
        auto* sprite = ocean.getGridSprite(parent, object);
        if (!sprite)
            return;
        ocean.gotMsgDiveUnitLoadChange(parent, object, GetLoadValue(), GetMaxLoad());
        if (!sprite->HasAccessorySprite(2))
            sprite->AddAccessorySprite(std::make_unique<DiveMeterSprite>(&ocean, sprite));
        if (auto* meter = dynamic_cast<DiveMeterSprite*>(sprite->GetAccessorySprite(2)))
            meter->SetDive(*this);
        if (!sprite->HasAccessorySprite(8))
            sprite->AddAccessorySprite(std::make_unique<DiveUnitReadoutSprite>(&ocean, sprite));
        if (auto* readout = dynamic_cast<DiveUnitReadoutSprite*>(sprite->GetAccessorySprite(8))) {
            MsgDiveDepthChange msg;
            msg.depth = depth;
            msg.maximum = maxDepth;
            readout->gotMsgDiveDepthChange(msg);
        }
    }

    DivingUnit DivingUnit::Duplicate() const {
        return *this;
    }

    bool DivingUnit::RequestDive() const {
        return OkayToDive();
    }

    void DivingUnit::UseClock(float seconds) {
        if (Diving() && !Returning() && std::isfinite(seconds) && seconds >= 0 &&
            seconds < remaining)
            clock.begin(remaining, seconds);
    }

    std::pair<std::vector<std::string>, float> DivingUnit::PlayAnimations() const {
        const std::string idle = load > 0 ? "idle item" : "idle";
        if (!Diving())
            return {{idle}, 0.f};
        if (returnTime > 0)
            return {{"return"}, 3.f - returnTime};
        if (settleTime > 0)
            return {{"up", idle}, .5f - settleTime};
        const float elapsed = std::max(0.f, totalSeconds - remaining);
        if (elapsed < 2.0167f)
            return {{"drop", "diving"}, elapsed};
        return {{"diving"}, elapsed - 2.0167f};
    }

    bool DivingUnit::Diving() const {
        return state == 3;
    }

    float DivingUnit::GetDiveTime() const {
        if (returnTime != 0)
            return returnTime / 3.f;
        return totalSeconds != 0 ? 1.f - remaining / totalSeconds : 0.f;
    }

    int DivingUnit::GetLoadValue() const {
        return load;
    }

    int DivingUnit::GetMaxLoad() const {
        return 200;
    }

    int DivingUnit::numAvailableBonusMaterials() const {
        return sum(bonus);
    }

    int DivingUnit::numCollectedBonusMaterials() const {
        return sum(collected);
    }

    void DivingUnit::SetDiveLevel(int current, int maximum) {
        maxDepth = std::max(0, maximum);
        depth = std::clamp(current, 0, maxDepth);
    }

    bool DivingUnit::OkayToDive() const {
        return parent > 0 && object > 0 && !Diving();
    }

    void DivingUnit::SetState(int value) {
        if (Diving() && value == 0) {
            totalSeconds = 0;
            remaining = 0;
            returnTime = settleTime = introTime = 0;
            clock = {};
        }
        state = value;
    }

    void DivingUnit::SetLoadValue(int value) {
        load = std::clamp(value, 0, GetMaxLoad());
    }

    void DivingUnit::StartDive(float seconds, float left) {
        if (!std::isfinite(seconds) || !std::isfinite(left) || seconds < 0 || seconds > 86400)
            return;
        totalSeconds = seconds;
        // Native StartDive tolerates up to two seconds of difference before
        // using the server's remaining countdown.
        remaining = std::max(0.f, seconds > left + 2.f ? left : seconds);
        returnTime = settleTime = 0;
        clock = {};
        if (state == 0)
            introTime = 1.f;
        SetState(3);
    }

    void DivingUnit::FinishDive() {
        if (totalSeconds > 0) {
            totalSeconds = remaining = 0;
            returnTime = 3.f;
            introTime = settleTime = 0;
            clock = {};
        } else if (!Returning()) {
            SetState(0);
            load = 0;
            bonus.clear();
            collected.clear();
        }
    }

    bool DivingUnit::TempTick(float dt) {
        if (!Diving())
            return true;
        if (!std::isfinite(dt) || dt <= 0)
            return false;
        if (introTime > 0) {
            introTime = std::max(0.f, introTime - dt);
        } else if (returnTime > 0) {
            returnTime = std::max(0.f, returnTime - dt);
            if (returnTime == 0)
                settleTime = .5f;
        } else if (settleTime > 0) {
            settleTime = std::max(0.f, settleTime - dt);
            if (settleTime == 0) {
                SetState(0);
                load = 0;
                bonus.clear();
                collected.clear();
                return true;
            }
        }
        clock.tick(remaining, dt);
        return false;
    }

    void DivingUnit::initWithSFSObject(const sfs::SFSObjectWrapper& p) {
        DivingUnit next;
        next.parent = num(p, "parent_id");
        next.object = num(p, "object_id");
        next.lootLevel = num(p, "loot_level");
        next.SetDiveLevel(num(p, "depth"), num(p, "max_depth", num(p, "depth")));
        next.richZone = num(p, "material_rich_zone");
        next.StartDive(
            float(std::clamp(num(p, "time_total_secs", num(p, "time_remaining_secs")), 0, 86400)),
            float(std::max(0, num(p, "time_remaining_secs"))));
        next.SetState(num(p, "state"));
        if (auto rows = p.getSFSArray("bonus_items")) {
            for (const auto& value : rows->values) {
                const auto item = std::get_if<std::shared_ptr<sfs::SFSObjectWrapper>>(&value);
                if (!item || !*item)
                    continue;
                const int id = num(**item, "material_id"), amount = num(**item, "amt");
                if (id > 0 && amount > 0)
                    next.bonus.emplace(id, amount);
            }
        }
        if (Diving() && next.Diving() && parent == next.parent && object == next.object) {
            if (Returning())
                return;
            next.introTime = introTime;
            if (!p.get("bonus_items"))
                next.bonus = bonus;
            for (const auto& [id, amount] : collected) {
                auto found = next.bonus.find(id);
                if (found != next.bonus.end() && amount > 0 && amount <= found->second)
                    next.collected.emplace(id, amount);
            }
        }
        *this = std::move(next);
    }

    std::vector<std::string> DivingUnit::getAvailableActions() const {
        if (Returning())
            return {};
        if (Diving())
            return remaining <= 0 ? std::vector<std::string>{"collect"}
                                  : std::vector<std::string>{"info"};
        return OkayToDive() ? std::vector<std::string>{"dive"} : std::vector<std::string>{};
    }
} // namespace rp
