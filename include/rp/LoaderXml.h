#pragma once
#include "rp/Loader.h"

namespace rp {
    class LoaderXml final : public Loader {
      public:
        using Loader::Loader;
        ~LoaderXml() override;
        const char* getType() const override;
        const std::string& getId() const override;
        bool prepare(LoadingManager*) override;
        void load(LoadingManager&) override;
    };
}
