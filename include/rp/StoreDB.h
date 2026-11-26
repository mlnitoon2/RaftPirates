#pragma once
#include "sfs/SFSObjectWrapper.h"
#include <filesystem>
#include <map>

namespace rp {
    struct StoreItem {
        int id = 0, currency = 0, price = 0, amount = 0;
        std::string name, group, title, sheet, image;
    };

    struct StoreCurrency {
        int id = 0, startingAmount = 0;
        std::string name;
    };

    class StoreDB {
      public:
        StoreDB();
        enum class Library { Items, Currencies, Groups };
        void initWithSFSObject(const sfs::SFSObjectWrapper& packet, Library library);
        void initializeStore();
        const StoreItem* getStoreInfo(unsigned id) const;
        const StoreItem* getStoreInfo(const std::string& name) const;
        void save(const std::filesystem::path& path) const;
        void load(const std::filesystem::path& path);
        std::map<int, StoreItem> items;
        std::map<int, StoreCurrency> currencies;
        std::map<int, std::string> groups;

      private:
        std::map<Library, sfs::SFSObjectWrapper> packets_;
    };
} // namespace rp
