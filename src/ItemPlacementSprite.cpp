#include "rp/ItemPlacementSprite.h"

namespace rp {
    ItemPlacementSprite::ItemPlacementSprite(const Game& game,const OceanView& view,
        std::shared_ptr<const sys::gfx::Image> image,const sys::res::SpriteCell& valid,
        const sys::res::SpriteCell& invalid)
        :PlacementSprite(game,view),image_(std::move(image)),valid_(valid),invalid_(invalid){}
    ItemPlacementSprite::~ItemPlacementSprite()=default;
    void ItemPlacementSprite::SetPriorities(){if(footprint_)footprint_->SetPriorities();}
    void ItemPlacementSprite::SetPriority(unsigned piece,float value){
        if(footprint_)footprint_->SetPriority(piece,value);
    }
    bool ItemPlacementSprite::OkayToPlace()const{
        auto def=game_.objects.find(object_);
        if(def==game_.objects.end())return false;
        if(def->second.type==1)
            return parent_>0&&game_.canPlaceComponent(object_,parent_,position_.x,position_.y,
                                                       ignoreParent_,ignoreId_);
        return parent_==0&&game_.canBuildRaft(position_);
    }
    void ItemPlacementSprite::Update(){
        const auto* parent=game_.GetRaftByID(parent_);
        if(parent_&&!parent){footprint_.reset();return;}
        const auto def=game_.objects.find(object_);
        auto cells=parent&&def!=game_.objects.end()?def->second.cells:std::vector<Cell>{{0,0}};
        if(cells.empty())cells.push_back({0,0});
        footprint_=std::make_unique<FootprintSprite>(view_,image_,OkayToPlace()?valid_:invalid_);
        footprint_->Update(position_,cells,parent);
        SetPriorities();
    }
    void ItemPlacementSprite::Render(sys::gfx::Image& target)const{
        if(footprint_)footprint_->Render(target);
    }
}
