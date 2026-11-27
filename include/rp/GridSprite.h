#pragma once
#include "rp/Game.h"
#include "rp/AccessorySprite.h"

namespace rp {
    // Reconstructed per-object visual state. No original graphics-pointer ABI is assumed.
    class GridSprite {
      public:
        struct SpritePiece {
            ~SpritePiece();
            GridPiece definition;
            sys::gfx::Gfx appearance;
            unsigned order = 0, suborder = 0;
            float priority = 0;
        };

        GridSprite(const OceanView& ocean, const GridObject& object,
                   const GridObjectInfo& definition, sys::gfx::Vec2 world, float scale);
        ~GridSprite();
        void Init();
        bool AttachAccessory(std::unique_ptr<AccessorySprite> accessory);
        bool DetachAccessory(int id);
        void Update();
        void SetGeometry(sys::gfx::Vec2 world, float scale);
        void SetTime(float time);
        void SetVisible(bool);
        bool GetVisible() const;
        void SetColour(uint8_t, uint8_t, uint8_t, uint8_t);
        uint8_t Alpha() const;
        void SetPriority(unsigned, float);
        float GetPriority(unsigned) const;
        void SetPriorities();
        void SetOrder(unsigned, unsigned, unsigned);
        unsigned GetOrder(unsigned) const;
        unsigned GetMaxOrder() const;
        size_t Pieces() const;
        const SpritePiece& operator[](unsigned) const;
        bool ContainsGfx(const sys::gfx::Gfx*) const;
        bool Select(const sys::gfx::Gfx*);
        sys::gfx::Vec2 GetWorldPosition(int) const;
        void AddAnimSlack(float);
        void UpdateAnimations(float);
        void SetAnimation(std::vector<std::string> names);
        bool HasAttack() const;
        void Attack();
        int submit(sys::gfx::GfxManager&, std::map<std::string, std::shared_ptr<sys::gfx::Image>>&,
                   std::map<std::string, sys::res::ResourceAEAnim>&, sys::gfx::AnimationRenderer&,
                   const std::filesystem::path&, bool raft);

      private:
        const OceanView& ocean_;
        const GridObject& object_;
        const GridObjectInfo& definition_;
        sys::gfx::Vec2 world_;
        float scale_ = 1, time_ = 0, slack_ = 0;
        bool visible_ = true, selected_ = false, hasAttack_ = false;
        sys::gfx::Color colour_{};
        std::vector<SpritePiece> pieces_;
        std::map<int, std::unique_ptr<AccessorySprite>> accessories_;
        std::vector<std::string> animations_;
    };
} // namespace rp
