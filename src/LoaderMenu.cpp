#include "rp/LoaderMenu.h"
#include "rp/LoadingManager.h"

namespace rp {
    const char* LoaderMenu::ID = "MENU";

    LoaderMenu::~LoaderMenu() = default;

    const char* LoaderMenu::getType() const {
        return ID;
    }

    const std::string& LoaderMenu::getId() const {
        return id_;
    }

    bool LoaderMenu::prepare(LoadingManager* m) {
        if (!m)
            return false;
        m->loadMenuJob(id_);
        return true;
    }
} // namespace rp
