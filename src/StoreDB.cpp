#include "rp/StoreDB.h"
#include "sfs/SFSWire.h"
#include "sys/File.h"
#include <fstream>
#include <limits>
#include <stdexcept>

namespace rp {
    namespace {
        using O = sfs::SFSObjectWrapper;

        int number(const O& o, const std::string& key) {
            auto p = o.get(key);
            if (!p)
                return 0;
            if (auto v = std::get_if<int32_t>(p))
                return *v;
            if (auto v = std::get_if<int16_t>(p))
                return *v;
            if (auto v = std::get_if<int8_t>(p))
                return *v;
            throw std::runtime_error("Invalid store integer: " + key);
        }

        const char* key(StoreDB::Library k) {
            switch (k) {
            case StoreDB::Library::Items:
                return "store_item_data";
            case StoreDB::Library::Currencies:
                return "store_currency_data";
            case StoreDB::Library::Groups:
                return "store_group_data";
            }
            throw std::invalid_argument("Invalid store library");
        }
    } // namespace

    StoreDB::StoreDB() = default;

    void StoreDB::initializeStore() {
        items.clear();
        currencies.clear();
        groups.clear();
        packets_.clear();
    }

    void StoreDB::initWithSFSObject(const O& packet, Library library) {
        auto array = packet.getSFSArray(key(library));
        if (!array)
            throw std::runtime_error("Missing store library");
        std::map<int, StoreItem> newItems;
        std::map<int, StoreCurrency> newCurrencies;
        std::map<int, std::string> newGroups;
        for (const auto& v : array->values) {
            auto entry = std::get_if<std::shared_ptr<O>>(&v);
            if (!entry || !*entry)
                throw std::runtime_error("Invalid store row");
            const auto& x = **entry;
            if (library == Library::Items) {
                StoreItem i{
                    number(x, "storeitem_id"), number(x, "currency_id"), number(x, "price"),
                    number(x, "amount"),       x.getString("item_name"), x.getString("group_name"),
                    x.getString("item_title"), x.getString("sheet_id"),  x.getString("image_id")};
                if (i.id <= 0 || i.price < 0 || i.amount < 0 || i.name.empty() ||
                    !newItems.emplace(i.id, i).second)
                    throw std::runtime_error("Invalid store item");
            } else if (library == Library::Currencies) {
                StoreCurrency c{number(x, "storecur_id"), number(x, "starting_amount"),
                                x.getString("currency_name")};
                if (c.id <= 0 || c.name.empty() || !newCurrencies.emplace(c.id, c).second)
                    throw std::runtime_error("Invalid store currency");
            } else {
                int id = number(x, "storegroup_id");
                auto name = x.getString("group_name");
                if (id <= 0 || name.empty() || !newGroups.emplace(id, name).second)
                    throw std::runtime_error("Invalid store group");
            }
        }
        auto copy = sfs::decodeObject(sfs::encodeObject(packet));
        packets_[library] = std::move(copy);
        if (library == Library::Items)
            items = std::move(newItems);
        else if (library == Library::Currencies)
            currencies = std::move(newCurrencies);
        else
            groups = std::move(newGroups);
    }

    const StoreItem* StoreDB::getStoreInfo(unsigned id) const {
        if (id > unsigned(std::numeric_limits<int>::max()))
            return nullptr;
        auto it = items.find(int(id));
        return it == items.end() ? nullptr : &it->second;
    }

    const StoreItem* StoreDB::getStoreInfo(const std::string& name) const {
        for (const auto& p : items)
            if (p.second.name == name)
                return &p.second;
        return nullptr;
    }

    void StoreDB::save(const std::filesystem::path& path) const {
        O all;
        for (const auto& p : packets_)
            all.put(key(p.first), std::make_shared<O>(p.second));
        auto bytes = sfs::encodeObject(all);
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out || !out.write(reinterpret_cast<const char*>(bytes.data()), bytes.size()))
            throw std::runtime_error("Store cache write failed");
    }

    void StoreDB::load(const std::filesystem::path& path) {
        auto all = sfs::decodeObject(sys::File::readAll(path));
        StoreDB next;
        for (auto k : {Library::Items, Library::Currencies, Library::Groups})
            if (auto packet = all.getSFSObj(key(k)))
                next.initWithSFSObject(*packet, k);
        *this = std::move(next);
    }
} // namespace rp
