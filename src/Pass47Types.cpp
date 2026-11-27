#include "rp/Blueprint.h"
#include "rp/BattleModeState.h"
#include "rp/Messages.h"
#include "rp/Quest.h"
#include "rp/DivingGame.h"
#include "rp/DiveUnitReadoutSprite.h"
#include "rp/PerimeterWallSprite.h"
#include "sys/gfx/AEAnim.h"
#include <stdexcept>

namespace rp {
    BlueprintInfo::~BlueprintInfo() = default;
    BattleModeState::BattleDice::BattleDice(const BattleDice&) = default;
    BattleModeState::BattleDice::~BattleDice() = default;
    DiveUnitReadoutSprite::ReadoutLight::ReadoutLight() = default;
    DiveUnitReadoutSprite::ReadoutLight::~ReadoutLight() = default;
    PerimeterWallSprite::SpriteBalloon::~SpriteBalloon() = default;

    MsgStartDialog::MsgStartDialog(int identifier, const std::string& a, const std::string& b,
                                  const std::string& c, const std::vector<std::string>& d,
                                  const std::vector<std::string>& e, const std::string& f,
                                  const std::string& g, const std::string& h, const std::string& i,
                                  const std::string& j, const std::string& k)
        : id(identifier), leading{a,b,c}, first(d), second(e), trailing{f,g,h,i,j,k} {}
    MsgStartDialog::~MsgStartDialog() = default;

    QuestTypeInfo::QuestTypeInfo(const sfs::SFSObjectWrapper& row)
        : id(row.getInt("type_id")), name(row.getString("name")),
          sheet(row.getString("sheet_id")), image(row.getString("image_id")) {}
    QuestTypeInfo::~QuestTypeInfo() = default;

    namespace diving_game {
        DivingMaterial::DivingMaterial(int px, int py, sys::gfx::Gfx* parent)
            : x(static_cast<float>(px)), y(static_cast<float>(py)) {
            sprite.setParent(parent);
            sprite.setHAnchor(sys::gfx::Gfx::HorizontalAnchor::Center);
            sprite.setVAnchor(sys::gfx::Gfx::VerticalAnchor::Center);
            sprite.SetPriority(50);
            sprite.SetPos(x, y);
        }

        void DivingMaterial::configure(std::shared_ptr<const sys::res::ResourceSpriteSheet> atlas,
                                      std::shared_ptr<const sys::gfx::Image> image,
                                      const std::string& cell) {
            if (!atlas || !image || !atlas->findSpriteByName(cell))
                throw std::invalid_argument("Missing diving material atlas cell");
            sprite.sheet = std::move(atlas);
            sprite.SetImage(std::move(image));
            sprite.setSpriteName(cell);
        }
    }
}
