#include "rp/LoaderXml.h"
#include "rp/LoadingManager.h"

namespace rp {
    const char* LoaderXml::ID = "XMLDATA";

    LoaderXml::~LoaderXml() = default;

    const char* LoaderXml::getType() const {
        return ID;
    }

    const std::string& LoaderXml::getId() const {
        return id_;
    }

    bool LoaderXml::prepare(LoadingManager*) {
        return true;
    }

    void LoaderXml::load(LoadingManager& manager) {
        manager.cache_.acquire<sys::res::XmlNode>(
            id_, [&] { return sys::res::loadXML(manager.checked(id_)); });
    }
} // namespace rp
