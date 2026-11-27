#include "rp/FireFXSprite.h"
#include "rp/SplashFXSprite.h"
#include "rp/WreckageSprite.h"
#include "rp/GfxBattleInfoDropDown.h"
#include "sys/gfx/AssetImage.h"
#include "sys/localization/LocalizationManager.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace rp {
using namespace sys::gfx;
FireFXSprite::FireFXSprite(const OceanView* o,GridSprite* p):OverlayAccessory(o,p,12){}
FireFXSprite::~FireFXSprite()=default;
void FireFXSprite::Init(){
    emitters_.clear();if(!ocean_)return;
    const char* configs[]={"particles/raft_item_fire.psi","particles/raft_item_smoke.psi"};
    const char* textures[]={"gfx/fx/particle_fire","gfx/fx/particle_puff"};
    for(unsigned i=0;i<2;++i)try{
        Emitter e;auto info=sys::particles::ParticleInfo::load(ocean_->assetRoot()/configs[i]);
        e.particles=std::make_unique<sys::particles::ParticleSystem>(info,5489+i);
        auto image=std::make_shared<Image>(loadAssetImage(ocean_->assetRoot()/textures[i]));
        e.sprite.image=image;e.sprite.cell={textures[i],0,0,image->width,image->height,0,0,image->width,image->height,false};
        e.sprite.SetColor(tint_.r,tint_.g,tint_.b,tint_.a);
        emitters_.push_back(std::move(e));
    }catch(const std::exception&){}
    Update();SetVisible(visible_);
}
void FireFXSprite::SetColour(uint8_t r,uint8_t g,uint8_t b,uint8_t a){
    tint_={r,g,b,a};for(auto& e:emitters_)e.sprite.SetColor(r,g,b,a);
}
void FireFXSprite::SetVisible(bool v){
    visible_=v;for(auto& e:emitters_){if(v){if(!e.particles->emitting())e.particles->fire();}else e.particles->stop(true);}
}
void FireFXSprite::Update(){
    if(!parent_||!ocean_)return;auto p=ocean_->WorldToScreen(parent_->worldPosition());
    p.y-=20.f*scale_;for(auto& e:emitters_)e.particles->setPosition(p);
}
void FireFXSprite::tick(float dt){if(!visible_||!std::isfinite(dt)||dt<0)return;Update();for(auto& e:emitters_)e.particles->tick(std::min(dt,1.f));}
int FireFXSprite::submit(GfxManager& m,AnimationRenderer&)const{
    if(!visible_)return 0;int n=0;
    for(const auto& e:emitters_){e.particles->submit(m,e.sprite,basePriority()+.04f);n+=int(e.particles->particles().size());}return n;
}
SplashFXSprite::SplashFXSprite(const OceanView* o,GridSprite* p,float seconds):AccessorySprite(o,p,11),seconds_(std::isfinite(seconds)?std::clamp(seconds,0.f,60.f):0){}
SplashFXSprite::~SplashFXSprite()=default;
void SplashFXSprite::Init(){
    splashes_.clear();elapsed_=0;finished_=true;
    if(!ocean_||!parent_||seconds_<=0)return;
    try{
        resource_=std::make_shared<sys::res::ResourceAEAnim>(sys::res::ResourceAEAnim::Create(ocean_->assetRoot()/"xml_bin/anim_water_splash.bin",sys::res::AEFormat::RaftPirates2013));
        if(!resource_->find("splash"))return;
        // Native iterates raft piece cells. Host uses each reconstructed visual piece's world position.
        for(unsigned i=0;i<parent_->Pieces();++i){
            auto a=std::make_unique<AEAnim>(resource_);a->setLoop(false);a->setAnimation("splash");
            splashes_.push_back({std::move(a),parent_->GetWorldPosition(int(i))});
        }finished_=splashes_.empty();
    }catch(const std::exception&){splashes_.clear();resource_.reset();}
}
// Original Update and virtual RemoveFromTick are return-only; tick calls base on completion.
void SplashFXSprite::Update(){}
void SplashFXSprite::RemoveFromTick(){}
void SplashFXSprite::tick(float dt){
    if(finished_||!std::isfinite(dt)||dt<0)return;elapsed_+=dt;
    for(auto& s:splashes_)s.animation->tick(dt);
    splashes_.erase(std::remove_if(splashes_.begin(),splashes_.end(),[](const auto& s){return !s.animation->playing();}),splashes_.end());
    if(splashes_.empty()){finished_=true;AccessorySprite::RemoveFromTick();}
}
int SplashFXSprite::submit(GfxManager& m,AnimationRenderer& r)const{
    if(finished_||!ocean_||!parent_||!parent_->GetVisible())return 0;int n=0;const float s=ocean_->Zoom();
    for(const auto& a:splashes_){auto p=ocean_->WorldToScreen(a.world);
        n+=r.submit(m,*a.animation,Matrix::translation(p.x-a.animation->width()*s*.5f,p.y-a.animation->height()*s*.5f)*Matrix::scale(s,s));
    }return n;
}
WreckageSprite::WreckageSprite(const OceanView& o,const Wreckage& w,const GridObjectInfo& d,Vec2 world,float s)
    :GridSprite(o,w,d,world,s),view_(o),wreckage_(w){Init();}
WreckageSprite::~WreckageSprite()=default;
void WreckageSprite::Init(){
    debris_.clear();GridSprite::Init();
    try{
        auto sheet=sys::res::ResourceSpriteSheet::Create(view_.assetRoot()/"xml_resources/construction_footprints_junk.xml");
        auto image=std::make_shared<Image>(loadAssetImage(view_.assetRoot()/sheet.imagePath));
        for(unsigned i=0;i<Pieces();++i){
            for(const auto& name:{std::string("footprint_edgewhite"),std::string("item_scrap_0")+std::to_string(i%3+1)}){
                auto* c=sheet.findSpriteByName(name);if(!c)continue;
                GfxSprite s;s.image=image;s.cell=*c;debris_.push_back(std::move(s));
            }
        }
    }catch(const std::exception&){}
    layoutDebris();
}
void WreckageSprite::layoutDebris(){
    if(!Pieces())return;
    const float zoom=view_.Zoom();
    for(unsigned i=0;i<debris_.size();++i){
        auto& s=debris_[i];const auto& p=(*this)[std::min<unsigned>(i/2,unsigned(Pieces()-1))].appearance;
        s.SetScale(zoom,zoom);s.SetPos(p.pos.x+float(wreckage_.offsetX)*zoom-s.cell.ow*zoom*.5f,p.pos.y+float(wreckage_.offsetY)*zoom-s.cell.oh*zoom*.5f);
        s.SetPriority(p.priority+(i%2?.02f:.01f));s.SetAlpha(Alpha());
    }
}
float WreckageSprite::GetPriority(unsigned i)const{
    if(i<Pieces())return GridSprite::GetPriority(i);
    return debris_.at(i-Pieces()).priority;
}
void WreckageSprite::SetPriority(unsigned i,float p){
    if(i<Pieces())GridSprite::SetPriority(i,p);
    else if(i-Pieces()<debris_.size())debris_[i-Pieces()].SetPriority(p);
}
void WreckageSprite::UpdatePiece(unsigned i){GridSprite::UpdatePiece(i);layoutDebris();}
int WreckageSprite::submit(GfxManager& m,std::map<std::string,std::shared_ptr<Image>>& images,
    std::map<std::string,sys::res::ResourceAEAnim>& bins,AnimationRenderer& r,const std::filesystem::path& root,bool raft){
    if(!GetVisible())return 0;
    int n=GridSprite::submit(m,images,bins,r,root,raft);layoutDebris();
    for(auto& s:debris_){m.submit(s);++n;}return n;
}
GfxBattleInfoDropDown::GfxBattleInfoDropDown(OceanView* o,const MsgBattleInfoDropdown& msg):view_(o){
    // Verified native jump table at 0x3e8680; type 2 and out-of-range have no dropdown.
    switch(msg.type){case 0:key_="BATTLE_INFO_AUTO_PILOT";break;case 1:key_="BATTLE_INFO_RAFT_SWITCHING";break;case 3:key_="BATTLE_INFO_UNDER_ATTACK";break;case 4:key_="BATTLE_INFO_RETALIATION";break;case 5:key_="BATTLE_INFO_WAITING";break;default:return;}
    text_=key_;
    if(o){
        try{sys::localization::LocalizationManager loc;loc.loadFile(o->assetRoot()/"text/en.utf8");text_=loc.getText(key_);}catch(const std::exception&){}
        try{image_=std::make_shared<Image>(loadAssetImage(o->assetRoot()/"gfx/menu/popup_bg"));}catch(const std::exception&){}
        try{font_=std::make_shared<ResourceSpriteFont>(ResourceSpriteFont::Create(o->assetRoot(),"rp_font_title"));}catch(const std::exception&){}
    }Show();
}
GfxBattleInfoDropDown::~GfxBattleInfoDropDown()=default;
void GfxBattleInfoDropDown::Show(){if(key_.empty())return;from_=progress_;target_=1;remaining_=.5f;}
void GfxBattleInfoDropDown::Hide(){from_=progress_;target_=0;remaining_=.5f;}
void GfxBattleInfoDropDown::aTickMethodThatsNotCalledTick(float dt){
    if(!std::isfinite(dt)||dt<0||remaining_<=0)return;
    remaining_=std::max(0.f,remaining_-dt);progress_=target_+(from_-target_)*(remaining_*2.f);
}
void GfxBattleInfoDropDown::submit(GfxManager& m)const{
    if(progress_<=0||!view_)return;
    const float w=std::min(340.f,view_->Width()),h=50.f,x=(view_->Width()-w)*.5f,y=-h+h*progress_;
    if(image_){GfxSprite s;s.image=image_;s.cell={"popup_bg",0,0,image_->width,image_->height,0,0,image_->width,image_->height,false};s.SetScale(w/image_->width,h/image_->height);s.SetPos(x,y);s.SetPriority(1000);m.submit(s);}
    if(font_){auto l=font_->layout(text_,.65f,w-12,false,true,1);font_->submit(m,l,x+(w-l.width)*.5f,y+(h-l.height)*.5f,{},1001);}
}
}
