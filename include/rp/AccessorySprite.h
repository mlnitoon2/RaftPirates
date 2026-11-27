#pragma once
namespace rp {
    class OceanView;
    class GridSprite;
    class AccessorySprite {
      public:
        AccessorySprite(const OceanView* ocean, GridSprite* parent, int id);
        virtual ~AccessorySprite();
        void AddToTick();
        void RemoveFromTick();
        int GetID() const;
        virtual void Init();
        virtual void Update();
        virtual void tick(float dt);
        virtual void SetScale(float scale);
        virtual bool ListenForAnimation();
        bool ticking() const { return ticking_; }
      protected:
        const OceanView* ocean_;
        GridSprite* parent_;
      private:
        int id_;
        bool ticking_ = false;
    };
}
