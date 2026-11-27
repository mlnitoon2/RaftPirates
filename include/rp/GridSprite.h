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
        virtual ~GridSprite();
        virtual void Init();
        bool AttachAccessory(std::unique_ptr<AccessorySprite> accessory);
        bool AddAccessorySprite(std::unique_ptr<AccessorySprite>);
        AccessorySprite* GetAccessorySprite(int id) const;
        bool HasAccessorySprite(int id) const;
        bool RemoveAccessorySprite(int id);
        bool ListenForAnimation() const;
        void gotMsgAnimationStarted();
        void SetMute(bool mute);

        bool muted() const {
            return muted_;
        }

        void UpdateParticles();
        void BuildPiece(const GridPiece&);
        virtual void UpdatePiece(unsigned index);
        bool DetachAccessory(int id);
        void Update();
        void SetGeometry(sys::gfx::Vec2 world, float scale);

        void SetScale(float scale) {
            SetGeometry(world_, scale);
        }

        sys::gfx::Vec2 worldPosition() const {
            return world_;
        }

        void SetTime(float time);
        void SetVisible(bool);
        bool GetVisible() const;
        void SetColour(uint8_t, uint8_t, uint8_t, uint8_t);
        uint8_t Alpha() const;
        virtual void SetPriority(unsigned, float);
        virtual float GetPriority(unsigned) const;
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

        const std::string& definitionName() const {
            return definition_.name;
        }

        const GridObjectInfo& definition() const {
            return definition_;
        }

        void setConstructionTime(float time) {
            constructionTime_ = time;
        }

        float constructionTime() const {
            return constructionTime_;
        }

        int objectState() const {
            return object_.state;
        }

        int battlePoints() const {
            return visualBattlePoints_.value_or(object_.GetBattlePoints());
        }

        void setBattlePoints(int points) {
            visualBattlePoints_ = points;
        }

        void Attack();
        virtual int submit(sys::gfx::GfxManager&,
                           std::map<std::string, std::shared_ptr<sys::gfx::Image>>&,
                           std::map<std::string, sys::res::ResourceAEAnim>&,
                           sys::gfx::AnimationRenderer&, const std::filesystem::path&, bool raft);

      private:
        const OceanView& ocean_;
        const GridObject& object_;
        std::optional<int> visualBattlePoints_;
        const GridObjectInfo& definition_;
        sys::gfx::Vec2 world_;
        float scale_ = 1, time_ = 0, slack_ = 0, constructionTime_ = 0;
        bool visible_ = true, selected_ = false, hasAttack_ = false, muted_ = false;
        sys::gfx::Color colour_{};
        std::vector<SpritePiece> pieces_;
        std::map<int, std::unique_ptr<AccessorySprite>> accessories_;
        std::vector<std::string> animations_;
    };
} // namespace rp
