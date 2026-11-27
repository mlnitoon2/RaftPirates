#include "rp/LoaderAEAnim.h"
#include "rp/LoadingManager.h"

namespace rp {
    LoaderAEAnim::~LoaderAEAnim() = default;
    const char* LoaderAEAnim::getType() const { return "AE"; }
    const std::string& LoaderAEAnim::getId() const { return id_; }

    bool LoaderAEAnim::prepare(LoadingManager* manager) {
        if (!manager) return false;
        const auto anim = manager->cache_.acquire<sys::res::ResourceAEAnim>(
            id_, [&] { return sys::res::ResourceAEAnim::Create(manager->checked(id_)); });
        for (const auto& image : anim->images) {
            std::filesystem::path path(image.path);
            const auto extension = path.extension().string();
            if (extension == ".xml") {
                if (!path.has_parent_path()) path = std::filesystem::path("xml_resources") / path;
                manager->addSheet(path.generic_string());
            } else if (extension == ".png" || extension == ".pvr") {
                path.replace_extension();
                manager->addSprite(path.generic_string());
            }
        }
        return true;
    }

    void LoaderAEAnim::load(LoadingManager& manager) {
        manager.cache_.acquire<sys::res::ResourceAEAnim>(
            id_, [&] { return sys::res::ResourceAEAnim::Create(manager.checked(id_)); });
    }
}
