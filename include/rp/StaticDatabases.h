#pragma once
#include "rp/Player.h"
#include "rp/Zone.h"
#include "rp/Discovery.h"
#include "sfs/SFSWire.h"
#include <filesystem>

namespace rp {
    // Portable static-library caches. Cache files contain SFS objects, not the original XML format.
    struct MaterialInfo {
        MaterialInfo() = default;
        MaterialInfo(const MaterialInfo&);
        MaterialInfo& operator=(const MaterialInfo&) = default;
        int id = 0;
        std::string name, sheet, image;
        std::optional<double> goldCost;
    };

    class BlueprintsDB {
      public:
        BlueprintsDB();
        void initWithSFSObject(const sfs::SFSObjectWrapper& packet);
        void save(const std::filesystem::path& path) const;
        void load(const std::filesystem::path& path);

        const std::map<int, Blueprint>& rows() const {
            return rows_;
        }

        const Blueprint* getBlueprintInfo(unsigned id) const;
        const Blueprint* getBlueprintInfo(const std::string& name) const;
        const Blueprint* getBlueprintInfoFromGridObjectID(unsigned id) const;
        std::vector<int> getAllBlueprintsAvail() const;
        static void getBlueprintDataFromSFSObjectWrapper(const sfs::SFSObjectWrapper&, Blueprint&);

      private:
        sfs::SFSObjectWrapper packet_;
        std::map<int, Blueprint> rows_;
    };

    class GridObjectsDB {
      public:
        GridObjectsDB();
        void initWithSFSObject(const sfs::SFSObjectWrapper& packet);
        void save(const std::filesystem::path& path) const;
        void load(const std::filesystem::path& path);

        const std::map<int, GridObjectInfo>& rows() const {
            return rows_;
        }

        int getBattlePointsFromGridObject(int id) const;
        std::string getDataName(int id) const;
        std::string getDataString(int& id, const std::string& name) const;
        bool getNewGridObjectData(GridObjectInfo& result, const std::string& name) const;

      private:
        sfs::SFSObjectWrapper packet_;
        std::map<int, GridObjectInfo> rows_;
    };

    class ZonesDB {
      public:
        ZonesDB();
        void initWithSFSObject(const sfs::SFSObjectWrapper& packet);
        void save(const std::filesystem::path& path) const;
        void load(const std::filesystem::path& path);

        const std::map<int, Zone>& rows() const {
            return rows_;
        }

        const Zone* getById(int id) const;
        void buildData();

      private:
        sfs::SFSObjectWrapper packet_;
        std::map<int, Zone> rows_;
    };

    class DiscoveryDB {
      public:
        DiscoveryDB();
        void initWithSFSObject(const sfs::SFSObjectWrapper& packet);
        void save(const std::filesystem::path& path) const;
        void load(const std::filesystem::path& path);

        const std::map<int, Discovery>& rows() const {
            return rows_;
        }

        const Discovery* getById(int id) const;
        void buildData();

      private:
        sfs::SFSObjectWrapper packet_;
        std::map<int, Discovery> rows_;
    };

    class MaterialsDB {
      public:
        MaterialsDB();
        void initWithSFSObject(const sfs::SFSObjectWrapper& packet);
        void save(const std::filesystem::path& path) const;
        void load(const std::filesystem::path& path);

        const std::map<int, MaterialInfo>& rows() const {
            return rows_;
        }

        const MaterialInfo* getMaterialInfo(unsigned id) const;
        const MaterialInfo* getMaterialInfo(const std::string& name) const;
        static void getMaterialInfoFromSFSData(const sfs::SFSObjectWrapper&, MaterialInfo&);

      private:
        sfs::SFSObjectWrapper packet_;
        std::map<int, MaterialInfo> rows_;
    };

    class QuestTypeDB {
      public:
        QuestTypeDB();
        void initWithSFSObject(const sfs::SFSObjectWrapper& packet);
        void save(const std::filesystem::path& path) const;
        void load(const std::filesystem::path& path);

        const std::map<int, sfs::SFSObjectWrapper>& rows() const {
            return rows_;
        }

        const sfs::SFSObjectWrapper* getQuestTypeInfo(unsigned id) const;
        const sfs::SFSObjectWrapper* getQuestTypeInfo(const std::string& name) const;

      private:
        sfs::SFSObjectWrapper packet_;
        std::map<int, sfs::SFSObjectWrapper> rows_;
    };
} // namespace rp
