#pragma once
#include "rp/Loader.h"
namespace rp {
    class LoaderImage final:public Loader {
      public:
        using Loader::Loader;
        ~LoaderImage() override;
        const char* getType()const override;
        const std::string& getId()const override;
        void load(LoadingManager&) override;
    };
}
