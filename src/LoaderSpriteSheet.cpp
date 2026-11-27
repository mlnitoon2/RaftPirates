#include "rp/LoaderSpriteSheet.h"
#include "rp/LoadingManager.h"

namespace rp {
    LoaderSpriteSheet::~LoaderSpriteSheet() = default;
    const char* LoaderSpriteSheet::getType() const { return "SS"; }
    const std::string& LoaderSpriteSheet::getId() const { return id_; }

    bool LoaderSpriteSheet::prepare(LoadingManager* manager) {
        if (!manager) return false;
        const auto sheet = manager->sheet(id_);
        if (!sheet->imagePath.empty()) {
            auto path = std::filesystem::path(sheet->imagePath);
            path.replace_extension();
            manager->addSprite(path.generic_string());
        }
        return true;
    }

    void LoaderSpriteSheet::load(LoadingManager& manager) { manager.sheet(id_); }
}
