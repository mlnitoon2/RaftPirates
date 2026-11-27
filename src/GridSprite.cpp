#include "rp/GridSprite.h"
#include "rp/RaftAttackSprite.h"
#include "rp/GridObjectHelpers.h"
#include "sys/gfx/AssetImage.h"
#include <algorithm>
#include <cmath>

namespace rp {
    using namespace sys::gfx;
    using namespace sys::res;

    GridSprite::SpritePiece::~SpritePiece() = default;

    GridSprite::GridSprite(const OceanView& ocean, const GridObject& object,
                           const GridObjectInfo& definition, Vec2 world, float scale)
        : ocean_(ocean), object_(object), definition_(definition), world_(world), scale_(scale) {
        Init();
    }

    GridSprite::~GridSprite() = default;

    bool GridSprite::AttachAccessory(std::unique_ptr<AccessorySprite> accessory) {
        return AddAccessorySprite(std::move(accessory));
    }
    bool GridSprite::AddAccessorySprite(std::unique_ptr<AccessorySprite> accessory) {
        if (!accessory || accessory->parent()!=this || accessories_.count(accessory->GetID())) return false;
        accessory->Init();
        accessory->SetScale(scale_);
        accessory->SetMute(muted_);
        accessory->Update();
        accessory->SetVisible(visible_);
        accessory->SetPriorities();
        accessory->AddToTick();
        const int id = accessory->GetID();
        accessories_.emplace(id, std::move(accessory));
        return true;
    }
    bool GridSprite::DetachAccessory(int id) { return RemoveAccessorySprite(id); }
    AccessorySprite* GridSprite::GetAccessorySprite(int id)const{
        auto it=accessories_.find(id);return it==accessories_.end()?nullptr:it->second.get();
    }
    bool GridSprite::HasAccessorySprite(int id)const{return GetAccessorySprite(id)!=nullptr;}
    bool GridSprite::RemoveAccessorySprite(int id){
        auto it=accessories_.find(id);if(it==accessories_.end())return false;
        it->second->RemoveFromTick();accessories_.erase(it);return true;
    }
    bool GridSprite::ListenForAnimation()const{return true;}
    void GridSprite::gotMsgAnimationStarted(){
        for(auto& item:accessories_)if(item.second->ListenForAnimation())item.second->AnimationStarted();
    }
    void GridSprite::SetMute(bool mute){
        // Native exempts the diving unit.
        if(definition_.name=="DivingUnit")return;
        muted_=mute;
        for(auto& item:accessories_)item.second->SetMute(mute);
    }
    void GridSprite::UpdateParticles(){
        for(auto& item:accessories_)item.second->UpdateParticles();
    }
    void GridSprite::BuildPiece(const GridPiece& def){
        SpritePiece piece;piece.definition=def;piece.order=unsigned(pieces_.size());
        pieces_.push_back(std::move(piece));
    }
    void GridSprite::UpdatePiece(unsigned index){
        auto base=ocean_.WorldToScreen(world_);
        auto& piece=pieces_.at(index);
        piece.appearance.SetPos(base.x+piece.definition.offsetX*scale_,base.y+piece.definition.offsetY*scale_);
        piece.appearance.SetScale(scale_,scale_);
        piece.appearance.SetVisible(visible_);
        piece.appearance.SetColor(colour_.r,colour_.g,colour_.b,colour_.a);
        piece.appearance.SetPriority(piece.priority);
    }

    void GridSprite::Init() {
        pieces_.clear();
        auto definitions = definition_.pieces;
        if (definitions.empty()) {
            GridPiece piece;
            piece.asset = definition_.asset;
            piece.offsetX = definition_.offsetX;
            piece.offsetY = definition_.offsetY;
            piece.cells = definition_.cells;
            definitions.push_back(std::move(piece));
        }
        for(const auto& def:definitions)BuildPiece(def);
        if(pieces_.size()>1&&pieces_.size()<=4096){
            std::vector<GridObject::GridShapePiece> shapes;
            for(unsigned i=0;i<pieces_.size();++i)
                shapes.emplace_back(object_,pieces_[i].definition,i);
            std::vector<GridObject::GridShapePiece*> order;
            performaIsoSort(shapes,order);
            for(unsigned i=0;i<order.size();++i)pieces_[order[i]->index()].order=i;
        }
        Update();
        SetPriorities();
    }

    void GridSprite::Update() {
        for(unsigned i=0;i<pieces_.size();++i)UpdatePiece(i);
        for (auto& entry : accessories_) entry.second->Update();
    }

    void GridSprite::SetGeometry(Vec2 world, float scale) {
        world_ = world;
        scale_ = scale;
        for (auto& entry : accessories_) entry.second->SetScale(scale);
        Update();
        SetPriorities();
    }

    void GridSprite::SetTime(float time) {
        time_ = std::isfinite(time) ? std::max(0.f, time) : 0;
    }

    void GridSprite::SetVisible(bool v) {
        visible_ = v;
        for (auto& p : pieces_)
            p.appearance.SetVisible(v);
        for (auto& entry : accessories_) entry.second->SetVisible(v);
    }

    bool GridSprite::GetVisible() const {
        return visible_;
    }

    void GridSprite::SetColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        colour_ = {r, g, b, a};
        for (auto& entry : accessories_) entry.second->SetColour(r,g,b,a);
        Update();
    }

    uint8_t GridSprite::Alpha() const {
        return colour_.a;
    }

    void GridSprite::SetPriority(unsigned index, float p) {
        auto& piece = pieces_.at(index);
        piece.priority = p;
        piece.appearance.SetPriority(p);
    }

    float GridSprite::GetPriority(unsigned index) const {
        return pieces_.at(index).priority;
    }

    void GridSprite::SetPriorities() {
        auto base = ocean_.WorldToScreen(world_);
        for (unsigned i = 0; i < pieces_.size(); ++i)
            SetPriority(i, ocean_.Priority() + base.y + pieces_[i].order * .01f + pieces_[i].suborder * .0001f);
        for (auto& entry : accessories_) entry.second->SetPriorities();
    }

    void GridSprite::SetOrder(unsigned i, unsigned order, unsigned sub) {
        auto& p = pieces_.at(i);
        p.order = order;
        p.suborder = sub;
        SetPriorities();
    }

    unsigned GridSprite::GetOrder(unsigned i) const {
        return pieces_.at(i).order;
    }

    unsigned GridSprite::GetMaxOrder() const {
        unsigned result = 0;
        for (const auto& p : pieces_)
            result = std::max(result, p.order);
        return result;
    }

    size_t GridSprite::Pieces() const {
        return pieces_.size();
    }

    const GridSprite::SpritePiece& GridSprite::operator[](unsigned i) const {
        return pieces_.at(i);
    }

    bool GridSprite::ContainsGfx(const Gfx* gfx) const {
        if (!gfx) return false;
        for (const auto& entry : accessories_) if (entry.second->ContainsGfx(gfx)) return true;
        return std::any_of(pieces_.begin(), pieces_.end(),
                                  [&](const auto& p) { return &p.appearance == gfx; });
    }

    bool GridSprite::Select(const Gfx* gfx) {
        selected_ = ContainsGfx(gfx);
        return selected_;
    }

    Vec2 GridSprite::GetWorldPosition(int i) const {
        auto& piece = pieces_.at(unsigned(i));
        auto base = ocean_.WorldToScreen(world_);
        return ocean_.ScreenToWorld({base.x + piece.definition.offsetX * scale_,
                                     base.y + piece.definition.offsetY * scale_});
    }

    void GridSprite::AddAnimSlack(float dt) {
        if (std::isfinite(dt) && dt >= 0)
            slack_ += dt;
    }

    void GridSprite::UpdateAnimations(float dt) {
        if (std::isfinite(dt) && dt >= 0) {
            // ARM UpdateAnimations consumes all slack as additional elapsed time.
            time_ += dt + slack_;
            slack_ = 0;
            for (auto& entry : accessories_) if (entry.second->ticking()) entry.second->tick(dt);
            // Defer removal until after callbacks; deleting an accessory during its tick
            // invalidates the iterator and destroys the executing object.
            for (auto it = accessories_.begin(); it != accessories_.end();) {
                if (it->second->finished()) it = accessories_.erase(it);
                else ++it;
            }
            UpdateParticles();
        }
    }

    void GridSprite::SetAnimation(std::vector<std::string> names) {
        if(animations_==names)return;
        animations_ = std::move(names);
        time_ = 0;
        gotMsgAnimationStarted();
    }

    bool GridSprite::HasAttack() const {
        return hasAttack_;
    }

    void GridSprite::Attack() {
        SetAnimation({"attack", "Attack"});
    }

    int GridSprite::submit(GfxManager& manager,
                           std::map<std::string, std::shared_ptr<Image>>& images,
                           std::map<std::string, ResourceAEAnim>& bins, AnimationRenderer& renderer,
                           const std::filesystem::path& root, bool raft) {
        if (!visible_)
            return 0;
        int drawn = 0;
        hasAttack_ = false;
        for (auto& p : pieces_) {
            auto path = p.definition.asset;
            if (path.empty())
                continue;
            try {
                if (std::filesystem::path(path).extension() == ".bin") {
                    auto a = bins.find(path);
                    if (a == bins.end())
                        a = bins.emplace(path, ResourceAEAnim::Create(root / path,
                                                                      AEFormat::RaftPirates2013))
                                .first;
                    const auto& bin = a->second;
                    const AEComp* comp = nullptr;
                    for (const auto& candidate : bin.comps)
                        if (candidate.name.find("attack") != std::string::npos ||
                            candidate.name.find("Attack") != std::string::npos)
                            hasAttack_ = true;
                    for (const auto& name : animations_)
                        if ((comp = bin.find(name)))
                            break;
                    if (!comp)
                        for (const auto& candidate : bin.comps)
                            if (candidate.name == "idle" || candidate.name == "Idle" ||
                                candidate.name.find("idle") != std::string::npos) {
                                comp = &candidate;
                                break;
                            }
                    if (!comp && !bin.comps.empty())
                        comp = &bin.comps.front();
                    if (!comp || comp->width <= 0 || comp->height <= 0)
                        continue;
                    float time = comp->duration() > 0 ? std::fmod(time_, comp->duration()) : 0;
                    float scale = raft ? scale_ : scale_ * .5f;
                    auto point = p.appearance.pos;
                    if (!raft) {
                        point.x -= definition_.offsetX * scale_;
                        point.y -= definition_.offsetY * scale_;
                    }
                    auto matrix = Matrix::translation(point.x - comp->width * scale / 2,
                                                      point.y - comp->height * scale * .5f -
                                                          (raft ? 0 : 12.f * scale_)) *
                                  Matrix::scale(scale, scale);
                    drawn += renderer.submit(manager, bin, *comp, time, matrix, p.priority,
                                             &p.appearance);
                } else {
                    auto a = images.find(path);
                    if (a == images.end())
                        a = images
                                .emplace(path, std::make_shared<Image>(loadAssetImage(root / path)))
                                .first;
                    auto im = a->second;
                    GfxSprite sprite;
                    sprite.image = im;
                    sprite.cell = {path, 0, 0,         im->width,  im->height,
                                   0,    0, im->width, im->height, false};
                    sprite.SetPos(p.appearance.pos.x, p.appearance.pos.y);
                    sprite.SetScale(scale_, scale_);
                    sprite.SetColor(colour_.r, colour_.g, colour_.b, colour_.a);
                    sprite.SetPriority(p.priority);
                    manager.submit(std::move(sprite));
                    ++drawn;
                }
            } catch (const std::exception&) {
            }
        }
        for (const auto& entry : accessories_)
            drawn += entry.second->submit(manager, renderer);
        return drawn;
    }
} // namespace rp
