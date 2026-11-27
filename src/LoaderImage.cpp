#include "rp/LoaderImage.h"
#include "rp/LoadingManager.h"

namespace rp {
    LoaderImage::~LoaderImage() = default;

    const char* LoaderImage::getType() const {
        return "SPR";
    }

    const std::string& LoaderImage::getId() const {
        return id_;
    }

    void LoaderImage::load(LoadingManager& m) {
        m.loadImageJob(id_);
    }
} // namespace rp
