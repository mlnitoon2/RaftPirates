#include "rp/AccessorySprite.h"
#include <cstdio>
namespace rp {
    AccessorySprite::AccessorySprite(const OceanView* ocean, GridSprite* parent, int id)
        : ocean_(ocean), parent_(parent), id_(id) {}
    AccessorySprite::~AccessorySprite() { RemoveFromTick(); }
    // GridSprite owns the portable tick registry; the original used a global dispatcher.
    void AccessorySprite::AddToTick() { ticking_ = true; }
    void AccessorySprite::RemoveFromTick() { ticking_ = false; }
    int AccessorySprite::GetID() const { return id_; }
    std::string AccessorySprite::GetTimeString(int seconds) const {
        char buffer[64];
        std::snprintf(buffer,sizeof(buffer),"%02d:%02d:%02d",seconds/3600,(seconds/60)%60,seconds%60);
        return buffer;
    }
    // These original base virtuals return immediately.
    void AccessorySprite::Init() {}
    void AccessorySprite::Update() {}
    void AccessorySprite::tick(float) {}
    void AccessorySprite::SetScale(float) {}
    bool AccessorySprite::ListenForAnimation() { return false; }
}
