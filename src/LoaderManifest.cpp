#include "rp/LoaderManifest.h"
#include "rp/LoadingManager.h"

namespace rp {
    LoaderManifest::~LoaderManifest() = default;

    const char* LoaderManifest::getType() const {
        return "MANI";
    }

    const std::string& LoaderManifest::getId() const {
        return id_;
    }

    bool LoaderManifest::prepare(LoadingManager* m) {
        if (!m)
            return false;
        m->prepareManifest(id_);
        return true;
    }
} // namespace rp
