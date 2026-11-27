#include "rp/LoaderImage.h"
#include "rp/LoadingManager.h"

namespace rp {
    const char* LoaderImage::ID = "SPR";

    LoaderImage::~LoaderImage() = default;

    const char* LoaderImage::getType() const {
        return ID;
    }

    const std::string& LoaderImage::getId() const {
        return id_;
    }

    void LoaderImage::load(LoadingManager& m) {
        m.loadImageJob(id_);
    }
} // namespace rp
