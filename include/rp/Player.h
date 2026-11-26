#pragma once
#include "rp/Raft.h"
#include "rp/Blueprint.h"
#include "sfs/SFSObjectWrapper.h"
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>

namespace rp {
    struct GridObjectPiece {
        GridObjectPiece() = default;
        explicit GridObjectPiece(const std::string& data);
        ~GridObjectPiece();
        std::string asset;
        int offsetX = 0, offsetY = 0;
        std::vector<Cell> cells;
    };

    using GridPiece = GridObjectPiece;

    struct GridObjectInfo {
        GridObjectInfo() = default;
        GridObjectInfo(const std::string& data, int battlePoints);
        GridObjectInfo(int id, int type, int points, std::string name, std::string asset,
                       int offsetX, int offsetY, std::vector<Cell> cells,
                       std::vector<GridPiece> pieces = {});
        int id = 0, type = 0, bp = 0;
        std::string name, asset;
        int offsetX = 0, offsetY = 0;
        std::vector<Cell> cells;
        std::vector<GridPiece> pieces;
        static GridObjectInfo parse(const std::string& data, int battlePoints);
    };

    GridObjectInfo readGridObject(const sfs::SFSObjectWrapper&);

    // Owns player inventory and raft instances. Static definitions are injected from libraries.
    class Player {
      public:
        std::map<int, GridObjectInfo> objects;
        std::map<int, Blueprint> blueprints;
        std::map<int, std::string> materialNames;
        std::map<int, double> materialGoldCosts;
        std::map<int, int> materials;
        std::set<int> owned;
        std::vector<Raft> rafts;
        bool safe = true;
        std::string playerName, knownZones;
        void initWithSFSObject(const sfs::SFSObjectWrapper& data);
        int currentBP() const;
        std::optional<int> goldCostToBuild(int blueprintId) const;
        Cell EastMostPoint() const;
        Cell WestMostPoint() const;
        bool isSafe() const;
        void setSafe(bool value);
        Raft* GetRaftByID(int id);
        const Raft* GetRaftByID(int id) const;
        const Raft* GetRaftAtPosition(Cell position) const;
        const Raft* mainRaft() const;
        bool hasRaft(int id) const;
        std::vector<Cell> GetAvailableEdgeGridSpace() const;
        std::vector<int> RaftsConnectedToRaft(int id) const;
        bool canBuildRaft(Cell pos) const;
        std::set<std::string> getResourceFiles() const;
        bool AddRaft(Raft raft);
        bool RemoveRaft(int id);
        int numOfMaterialPossessed(unsigned id) const;
        int numOfMaterialPossessed(const std::string& name) const;
        bool hasMaterial(int id, int count) const;
        bool hasMaterial(const std::string& name, int count) const;
        void giveMaterial(unsigned id, int count);
        void giveBlueprint(int id);
        bool hasBlueprint(const std::string& name) const;
        bool ownsObject(int id) const;
        bool RaftRepairRequired() const;
        bool RaftCleanupRequired() const;
        bool isZoneKnown(unsigned id) const;
        // Parent and component IDs; zero pair if no usable diver exists.
        std::pair<int, int> GetDiveUnit() const;
    };
} // namespace rp
