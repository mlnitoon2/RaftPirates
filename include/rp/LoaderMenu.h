#pragma once
#include "rp/Loader.h"

namespace rp {
    class LoaderMenu final : public Loader {
      public:
        static const char* ID;
        using Loader::Loader;
        ~LoaderMenu() override;
        const char* getType() const override;
        const std::string& getId() const override;
        bool prepare(LoadingManager*) override;
    };
} // namespace rp
