#include "rp/WreckageSprite.h"
#include "sys/gfx/AssetImage.h"
#include "sys/localization/LocalizationManager.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rp {
    using namespace sys::gfx;

    WreckageSprite::WreckageSprite(const OceanView& o, const Wreckage& w, const GridObjectInfo& d,
                                   Vec2 world, float s)
        : GridSprite(o, w, d, world, s), view_(o), wreckage_(w) {
        Init();
    }

    WreckageSprite::~WreckageSprite() = default;

    void WreckageSprite::Init() {
        debris_.clear();
        GridSprite::Init();
        try {
            auto sheet = sys::res::ResourceSpriteSheet::Create(
                view_.assetRoot() / "xml_resources/construction_footprints_junk.xml");
            auto image =
                std::make_shared<Image>(loadAssetImage(view_.assetRoot() / sheet.imagePath));
            for (unsigned i = 0; i < Pieces(); ++i) {
                for (const auto& name : {std::string("footprint_edgewhite"),
                                         std::string("item_scrap_0") + std::to_string(i % 3 + 1)}) {
                    auto* c = sheet.findSpriteByName(name);
                    if (!c)
                        continue;
                    GfxSprite s;
                    s.image = image;
                    s.cell = *c;
                    debris_.push_back(std::move(s));
                }
            }
        } catch (const std::exception&) {
        }
        layoutDebris();
    }

    void WreckageSprite::layoutDebris() {
        if (!Pieces())
            return;
        const float zoom = view_.Zoom();
        for (unsigned i = 0; i < debris_.size(); ++i) {
            auto& s = debris_[i];
            const auto& p = (*this)[std::min<unsigned>(i / 2, unsigned(Pieces() - 1))].appearance;
            s.SetScale(zoom, zoom);
            s.SetPos(p.pos.x + float(wreckage_.offsetX) * zoom - s.cell.ow * zoom * .5f,
                     p.pos.y + float(wreckage_.offsetY) * zoom - s.cell.oh * zoom * .5f);
            s.SetPriority(p.priority + (i % 2 ? .02f : .01f));
            s.SetAlpha(Alpha());
        }
    }

    float WreckageSprite::GetPriority(unsigned i) const {
        if (i < Pieces())
            return GridSprite::GetPriority(i);
        return debris_.at(i - Pieces()).priority;
    }

    void WreckageSprite::SetPriority(unsigned i, float p) {
        if (i < Pieces())
            GridSprite::SetPriority(i, p);
        else if (i - Pieces() < debris_.size())
            debris_[i - Pieces()].SetPriority(p);
    }

    void WreckageSprite::UpdatePiece(unsigned i) {
        GridSprite::UpdatePiece(i);
        layoutDebris();
    }

    int WreckageSprite::submit(GfxManager& m, std::map<std::string, std::shared_ptr<Image>>& images,
                               std::map<std::string, sys::res::ResourceAEAnim>& bins,
                               AnimationRenderer& r, const std::filesystem::path& root, bool raft) {
        if (!GetVisible())
            return 0;
        int n = GridSprite::submit(m, images, bins, r, root, raft);
        layoutDebris();
        for (auto& s : debris_) {
            m.submit(s);
            ++n;
        }
        return n;
    }
} // namespace rp
