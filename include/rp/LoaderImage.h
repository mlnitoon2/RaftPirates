#pragma once
#include "rp/Loader.h"

namespace rp {
    class LoaderImage final : public Loader {
      public:
        static const char* ID;
        using Loader::Loader;
        ~LoaderImage() override;
        const char* getType() const override;
        const std::string& getId() const override;
        void load(LoadingManager&) override;
    };
} // namespace rp
