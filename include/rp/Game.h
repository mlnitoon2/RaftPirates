#pragma once
#include "sfs/SFSWire.h"
#include "rp/Raft.h"
#include "rp/Zone.h"
#include "rp/Discovery.h"
#include "sys/gfx/AnimationRenderer.h"
#include <optional>
#include <set>

namespace rp {
    struct Cell {
        int x = 0, y = 0;
    };

    struct GridPiece {
        std::string asset;
        int offsetX = 0, offsetY = 0;
        std::vector<Cell> cells;
    };

    struct GridObject {
        int id = 0, type = 0, bp = 0;
        std::string name, asset;
        int offsetX = 0, offsetY = 0;
        std::vector<Cell> cells;
        std::vector<GridPiece> pieces;
    };

    GridObject readGridObject(const sfs::SFSObjectWrapper&);

    struct Blueprint {
        int id = 0, object = 0, unlock = 0;
        std::string name;
    };

    struct StoreItem {
        int id = 0, currency = 0, price = 0, amount = 0;
        std::string name, group, title;
    };

    struct StoreCurrency {
        int id = 0, startingAmount = 0;
        std::string name;
    };

    struct DiveState {
        int parent = 0, object = 0, state = 0, lootLevel = 0, depth = 0, maxDepth = 0, richZone = 0,
            totalSeconds = 0;
    };

    struct PendingBuild {
        int type = 0, parent = 0, x = 0, y = 0;
    };

    struct PendingMove {
        int parent = 0, id = 0, newParent = 0, x = 0, y = 0;
    };

    class Game {
      public:
        std::map<int, GridObject> objects;
        std::map<int, Blueprint> blueprints;
        std::map<int, std::string> materialNames;
        std::map<int, int> materials;
        std::set<int> owned;
        std::vector<Raft> rafts;
        std::map<int, std::string> zones, zoneMaps;
        std::map<int, std::string> discoveries;
        std::map<int, Zone> zoneData;
        std::map<int, Discovery> discoveryData;
        bool badZone = false;
        int questCount = 0;
        std::string knownZones, knownDiscoveries;
        std::map<int, StoreItem> storeItems;
        std::map<int, StoreCurrency> storeCurrencies;
        std::map<int, std::string> storeGroups;
        DiveState diveState;
        int daringRemaining = 0, daringTotal = 0;
        bool daringPaused = false;
        int zone = 1, mapX = 0, mapY = 0, daring = 0, buildLimit = 0, diveSeconds = 0;
        bool safe = true, ready = false;
        std::string playerName, token, status = "Not connected";
        std::optional<PendingBuild> pending;
        std::optional<PendingMove> pendingMove;
        std::optional<int> pendingZone;
        std::optional<int> pendingDiscovery;
        std::optional<Cell> pendingMap;
        int discovery = 0, mapActions = 0;
        float diveFraction = 0;
        float daringFraction = 0;
        void accept(const sfs::Message&);
        void extension(const std::string&, const sfs::SFSObjectWrapper&);
        sfs::Message handshake() const;
        sfs::Message login(const std::string& id, const std::string& hash) const;
        static sfs::Message request(const std::string&, sfs::SFSObjectWrapper = {});
        sfs::Message build(int object, int parent, int x, int y);
        sfs::Message remove(int parent, int id) const;
        sfs::Message moveComponent(int parent, int id, int newParent, int x, int y);
        bool canPlaceComponent(int object, int parent, int x, int y, int ignoreParent = 0,
                               int ignoreId = 0) const;
        sfs::Message dive(const std::string& command) const;
        sfs::Message travel(int id);
        sfs::Message buy(int id) const;
        sfs::Message buyStoreItem(int id) const;
        sfs::Message discover(int id);
        sfs::Message moveMap(int x, int y, bool destinationSafe);
        bool daringLocked() const;
        bool canAdvanceDaring() const;
        double daringProgress() const;
        void tick(float dt);
        void demo();
        std::string summary() const;
        std::string catalog() const;
    };

    class OceanView {
      public:
        explicit OceanView(std::filesystem::path root)
            : root_(std::move(root)), animations_(root_) {}

        sys::gfx::Image render(const Game&, float time, bool grid);

      private:
        std::filesystem::path root_;
        sys::gfx::AnimationRenderer animations_;
        std::map<std::string, std::shared_ptr<sys::gfx::Image>> images_;
        std::map<std::string, sys::res::ResourceAEAnim> bins_;
    };
} // namespace rp
