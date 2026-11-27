#pragma once
#include "rp/Loader.h"

namespace rp {
    class LoaderAEAnim final : public Loader {
      public:
        using Loader::Loader;
        ~LoaderAEAnim() override;
        const char* getType() const override;
        const std::string& getId() const override;
        bool prepare(LoadingManager*) override;
        void load(LoadingManager&) override;
    };
}
