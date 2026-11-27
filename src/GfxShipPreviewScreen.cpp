#include "rp/GfxShipPreviewScreen.h"

namespace rp {
    GfxShipPreviewScreen::GfxShipPreviewScreen(const std::string& assets, std::list<RaftInfo> rafts)
        : view_(assets) {
        int id = 1;
        for (const auto& raft : rafts)
            player_.rafts.emplace_back(id++, raft.typeId, raft.positionX, raft.positionY,
                                       raft.battlePoints, 0, std::vector<Component>{});
    }
    GfxShipPreviewScreen::~GfxShipPreviewScreen() = default;
    void GfxShipPreviewScreen::aTickMethodThatsNotCalledTick(float dt) { view_.tick(dt); }
    sys::gfx::Image GfxShipPreviewScreen::render(const std::map<int, GridObjectInfo>& catalogue) {
        player_.objects = catalogue;
        return view_.render(player_, -1, false);
    }
}
