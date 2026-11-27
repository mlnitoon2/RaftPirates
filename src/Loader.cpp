#include "rp/Loader.h"
namespace rp {
    Loader::~Loader()=default;
    bool Loader::prepare(LoadingManager*){return false;}
    void Loader::load(LoadingManager&){} // Native base returns an empty resource.
}
