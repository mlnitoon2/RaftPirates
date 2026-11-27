#include "rp/LoaderXml.h"
#include "rp/LoadingManager.h"

namespace rp {
    LoaderXml::~LoaderXml() = default;
    const char* LoaderXml::getType() const { return "XMLDATA"; }
    const std::string& LoaderXml::getId() const { return id_; }
    bool LoaderXml::prepare(LoadingManager*) { return true; }
    void LoaderXml::load(LoadingManager& manager) {
        manager.cache_.acquire<sys::res::XmlNode>(
            id_, [&] { return sys::res::loadXML(manager.checked(id_)); });
    }
}
