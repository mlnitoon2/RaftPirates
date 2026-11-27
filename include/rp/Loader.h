#pragma once
#include <string>
#include <utility>
namespace rp {
    class LoadingManager;
    class Loader {
      public:
        explicit Loader(std::string id):id_(std::move(id)){}
        virtual ~Loader();
        virtual const char* getType()const=0;
        virtual const std::string& getId()const=0;
        virtual bool prepare(LoadingManager*);
        virtual void load(LoadingManager&);
      protected:
        std::string id_;
    };
}
