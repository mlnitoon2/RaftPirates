#include "rp/StaticDatabases.h"
#include "sys/File.h"
#include <algorithm>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace rp {
    MaterialInfo::MaterialInfo(const MaterialInfo&) = default;
    namespace {
        using O = sfs::SFSObjectWrapper;

        template <class Row, class Parse, class Id>
        std::map<int, Row> readRows(const O& packet, const char* key, Parse parse, Id id) {
            auto rows = packet.getSFSArray(key);
            if (!rows)
                throw std::runtime_error(std::string("Missing library: ") + key);
            std::map<int, Row> result;
            for (const auto& value : rows->values) {
                auto object = std::get_if<std::shared_ptr<O>>(&value);
                if (!object || !*object)
                    throw std::runtime_error("Invalid library entry");
                Row row = parse(**object);
                int keyId = id(row);
                if (keyId <= 0 || !result.emplace(keyId, std::move(row)).second)
                    throw std::runtime_error("Invalid or duplicate library identifier");
            }
            return result;
        }

        O readCache(const std::filesystem::path& path) {
            return sfs::decodeObject(sys::File::readAll(path));
        }

        void writeCache(const std::filesystem::path& path, const O& packet) {
            auto data = sfs::encodeObject(packet);
            if (!path.parent_path().empty())
                std::filesystem::create_directories(path.parent_path());
            std::ofstream stream(path, std::ios::binary | std::ios::trunc);
            if (!stream || !stream.write(reinterpret_cast<const char*>(data.data()), data.size()))
                throw std::runtime_error("Library cache write failed");
        }

        template <class Row> const Row* byId(const std::map<int, Row>& rows, int id) {
            auto it = rows.find(id);
            return it == rows.end() ? nullptr : &it->second;
        }
    } // namespace

    BlueprintsDB::BlueprintsDB() = default;

    void BlueprintsDB::initWithSFSObject(const O& packet) {
        auto rows = readRows<Blueprint>(
            packet, "blueprint_data",
            [](const O& row) {
                Blueprint b;
                b.initWithSFSObject(row);
                return b;
            },
            [](const Blueprint& b) { return b.id(); });
        // Store a deep copy so subsequent callers cannot mutate the saved library.
        auto snapshot = sfs::decodeObject(sfs::encodeObject(packet));
        packet_ = std::move(snapshot);
        rows_ = std::move(rows);
    }

    void BlueprintsDB::save(const std::filesystem::path& path) const {
        writeCache(path, packet_);
    }

    void BlueprintsDB::load(const std::filesystem::path& path) {
        initWithSFSObject(readCache(path));
    }

    const Blueprint* BlueprintsDB::getBlueprintInfo(unsigned id) const {
        return id > unsigned(std::numeric_limits<int>::max()) ? nullptr : byId(rows_, int(id));
    }

    const Blueprint* BlueprintsDB::getBlueprintInfo(const std::string& name) const {
        for (const auto& row : rows_)
            if (row.second.name() == name)
                return &row.second;
        return nullptr;
    }

    const Blueprint* BlueprintsDB::getBlueprintInfoFromGridObjectID(unsigned id) const {
        for (const auto& row : rows_)
            if (row.second.object > 0 && unsigned(row.second.object) == id)
                return &row.second;
        return nullptr;
    }

    std::vector<int> BlueprintsDB::getAllBlueprintsAvail() const {
        std::vector<int> result;
        for (const auto& row : rows_)
            if (!row.second.deprecated)
                result.push_back(row.first);
        std::stable_sort(result.begin(), result.end(), [&](int a, int b) {
            return rows_.at(a).displayOrder < rows_.at(b).displayOrder;
        });
        return result;
    }

    void BlueprintsDB::getBlueprintDataFromSFSObjectWrapper(const O& row, Blueprint& result) {
        result.initWithSFSObject(row);
    }

    GridObjectsDB::GridObjectsDB() = default;

    void GridObjectsDB::initWithSFSObject(const O& packet) {
        auto rows = readRows<GridObjectInfo>(packet, "gridobject_data", readGridObject,
                                             [](const GridObjectInfo& g) { return g.id; });
        // Store a deep copy so subsequent callers cannot mutate the saved library.
        auto snapshot = sfs::decodeObject(sfs::encodeObject(packet));
        packet_ = std::move(snapshot);
        rows_ = std::move(rows);
    }

    void GridObjectsDB::save(const std::filesystem::path& path) const {
        writeCache(path, packet_);
    }

    void GridObjectsDB::load(const std::filesystem::path& path) {
        initWithSFSObject(readCache(path));
    }

    int GridObjectsDB::getBattlePointsFromGridObject(int id) const {
        auto row = byId(rows_, id);
        return row ? row->bp : 0;
    }

    std::string GridObjectsDB::getDataName(int id) const {
        auto row = byId(rows_, id);
        return row ? row->name : std::string{};
    }

    std::string GridObjectsDB::getDataString(int& id, const std::string& name) const {
        id = 0;
        auto entries = packet_.getSFSArray("gridobject_data");
        if (entries)
            for (const auto& value : entries->values) {
                auto row = std::get_if<std::shared_ptr<O>>(&value);
                if (row && *row && (*row)->getString("name") == name) {
                    id = (*row)->getInt("object_id");
                    return (*row)->getString("data_string");
                }
            }
        return {};
    }

    bool GridObjectsDB::getNewGridObjectData(GridObjectInfo& result,
                                             const std::string& name) const {
        for (const auto& row : rows_)
            if (row.second.name == name) {
                result = row.second;
                return true;
            }
        return false;
    }

    ZonesDB::ZonesDB() = default;

    void ZonesDB::initWithSFSObject(const O& packet) {
        auto rows = readRows<Zone>(
            packet, "zone_data",
            [](const O& row) {
                Zone z;
                z.initWithSFSObject(row);
                return z;
            },
            [](const Zone& z) { return z.id(); });
        // Store a deep copy so subsequent callers cannot mutate the saved library.
        auto snapshot = sfs::decodeObject(sfs::encodeObject(packet));
        packet_ = std::move(snapshot);
        rows_ = std::move(rows);
    }

    void ZonesDB::save(const std::filesystem::path& path) const {
        writeCache(path, packet_);
    }

    void ZonesDB::load(const std::filesystem::path& path) {
        initWithSFSObject(readCache(path));
    }

    const Zone* ZonesDB::getById(int id) const {
        return byId(rows_, id);
    }

    void ZonesDB::buildData() {
        rows_ = readRows<Zone>(
            packet_, "zone_data",
            [](const O& row) {
                Zone z;
                z.initWithSFSObject(row);
                return z;
            },
            [](const Zone& z) { return z.id(); });
    }

    DiscoveryDB::DiscoveryDB() = default;

    void DiscoveryDB::initWithSFSObject(const O& packet) {
        auto rows = readRows<Discovery>(
            packet, "discovery_data",
            [](const O& row) {
                Discovery d;
                d.initWithSFSObject(row);
                return d;
            },
            [](const Discovery& d) { return d.id(); });
        // Store a deep copy so subsequent callers cannot mutate the saved library.
        auto snapshot = sfs::decodeObject(sfs::encodeObject(packet));
        packet_ = std::move(snapshot);
        rows_ = std::move(rows);
    }

    void DiscoveryDB::save(const std::filesystem::path& path) const {
        writeCache(path, packet_);
    }

    void DiscoveryDB::load(const std::filesystem::path& path) {
        initWithSFSObject(readCache(path));
    }

    const Discovery* DiscoveryDB::getById(int id) const {
        return byId(rows_, id);
    }

    void DiscoveryDB::buildData() {
        rows_ = readRows<Discovery>(
            packet_, "discovery_data",
            [](const O& row) {
                Discovery d;
                d.initWithSFSObject(row);
                return d;
            },
            [](const Discovery& d) { return d.id(); });
    }

    MaterialsDB::MaterialsDB() = default;

    void MaterialsDB::initWithSFSObject(const O& packet) {
        auto rows = readRows<MaterialInfo>(
            packet, "material_data",
            [](const O& row) {
                MaterialInfo m;
                MaterialsDB::getMaterialInfoFromSFSData(row, m);
                return m;
            },
            [](const MaterialInfo& m) { return m.id; });
        // Store a deep copy so subsequent callers cannot mutate the saved library.
        auto snapshot = sfs::decodeObject(sfs::encodeObject(packet));
        packet_ = std::move(snapshot);
        rows_ = std::move(rows);
    }

    void MaterialsDB::save(const std::filesystem::path& path) const {
        writeCache(path, packet_);
    }

    void MaterialsDB::load(const std::filesystem::path& path) {
        initWithSFSObject(readCache(path));
    }

    const MaterialInfo* MaterialsDB::getMaterialInfo(unsigned id) const {
        return id > unsigned(std::numeric_limits<int>::max()) ? nullptr : byId(rows_, int(id));
    }

    const MaterialInfo* MaterialsDB::getMaterialInfo(const std::string& name) const {
        for (const auto& row : rows_)
            if (row.second.name == name)
                return &row.second;
        return nullptr;
    }

    void MaterialsDB::getMaterialInfoFromSFSData(const O& data, MaterialInfo& m) {
        m.id = data.getInt("material_id");
        m.name = data.getString("name");
        m.sheet = data.getString("sheet_id");
        m.image = data.getString("image_id");
        if (data.get("gold_cost"))
            m.goldCost = data.getDouble(
                "gold_cost", data.getFloat("gold_cost", float(data.getInt("gold_cost"))));
    }

    QuestTypeDB::QuestTypeDB() = default;

    void QuestTypeDB::initWithSFSObject(const O& packet) {
        auto rows = readRows<sfs::SFSObjectWrapper>(
            packet, "quest_type_data", [](const O& row) { return row; },
            [](const O& row) { return row.getInt("type_id"); });
        // Store a deep copy so subsequent callers cannot mutate the saved library.
        auto snapshot = sfs::decodeObject(sfs::encodeObject(packet));
        packet_ = std::move(snapshot);
        rows_ = std::move(rows);
    }

    void QuestTypeDB::save(const std::filesystem::path& path) const {
        writeCache(path, packet_);
    }

    void QuestTypeDB::load(const std::filesystem::path& path) {
        initWithSFSObject(readCache(path));
    }

    const O* QuestTypeDB::getQuestTypeInfo(unsigned id) const {
        return id > unsigned(std::numeric_limits<int>::max()) ? nullptr : byId(rows_, int(id));
    }

    const O* QuestTypeDB::getQuestTypeInfo(const std::string& name) const {
        for (const auto& row : rows_)
            if (row.second.getString("name") == name)
                return &row.second;
        return nullptr;
    }
} // namespace rp
