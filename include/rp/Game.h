#pragma once
#include "sfs/SFSWire.h"
#include "rp/Player.h"
#include "rp/Zone.h"
#include "rp/Discovery.h"
#include "rp/Quest.h"
#include "rp/StaticDatabases.h"
#include "rp/StoreDB.h"
#include "rp/DivingUnit.h"
#include "rp/BattleModeState.h"
#include "sys/gfx/AnimationRenderer.h"
#include <optional>
#include <set>

namespace rp {
    struct PendingBuild {
        int type = 0, parent = 0, x = 0, y = 0;
    };

    struct PendingMove {
        int parent = 0, id = 0, newParent = 0, x = 0, y = 0;
    };

    class Game : public Player {
      public:
        BlueprintsDB blueprintDB;
        GridObjectsDB gridObjectDB;
        MaterialsDB materialDB;
        ZonesDB zonesDB;
        DiscoveryDB discoveryDB;
        QuestTypeDB questTypeDB;
        StoreDB storeDB;
        std::map<int, std::string> zones, zoneMaps;
        std::map<int, std::string> discoveries;
        std::map<int, Zone> zoneData;
        std::map<int, Discovery> discoveryData;
        bool badZone = false, autoSafe = false;
        int questCount = 0;
        std::map<int, Quest> quests;
        sfs::Message openQuest(int id);
        std::string knownDiscoveries;
        std::map<int, StoreItem> storeItems;
        std::map<int, StoreCurrency> storeCurrencies;
        std::map<int, std::string> storeGroups;
        DiveState diveState;
        BattleModeState onlineBattle;
        std::optional<sfs::SFSObjectWrapper> bossData;
        std::vector<sfs::SFSObjectWrapper> sonarShips;
        bool sonarPending = false, readyForBattle = false;
        std::optional<sfs::Message> pingSonar();
        int daringRemaining = 0, daringTotal = 0;
        bool daringPaused = false;
        int zone = 1, mapX = 0, mapY = 0, daring = 0, buildLimit = 0, diveSeconds = 0;
        bool ready = false;
        int64_t libraryTimestamp = 0;
        std::string token, status = "Not connected";
        std::optional<PendingBuild> pending;
        std::optional<PendingMove> pendingMove;
        std::optional<int> pendingZone;
        std::optional<int> pendingDiscovery;
        std::optional<Cell> pendingMap;
        int discovery = 0, mapActions = 0;
        int mapActionMax = 0, mapActionTime = 0, battleReprieve = 0;
        bool hideMap = false;
        sfs::SFSObjectWrapper gameVars;
        std::shared_ptr<sfs::SFSArrayWrapper> treasureMaps;
        float diveFraction = 0;
        float daringFraction = 0;
        void accept(const sfs::Message&);
        void extension(const std::string&, const sfs::SFSObjectWrapper&);
        sfs::Message handshake() const;
        sfs::Message login(const std::string& id, const std::string& hash) const;
        static sfs::Message request(const std::string&, sfs::SFSObjectWrapper = {});
        sfs::Message build(int object, int parent, int x, int y);
        sfs::Message remove(int parent, int id) const;
        sfs::Message moveComponent(int parent, int id, int newParent, int x, int y);
        bool canPlaceComponent(int object, int parent, int x, int y, int ignoreParent = 0,
                               int ignoreId = 0) const;
        sfs::Message dive(const std::string& command) const;
        sfs::Message travel(int id);
        sfs::Message buy(int id) const;
        sfs::Message buyStoreItem(int id) const;
        sfs::Message discover(int id);
        sfs::Message moveMap(int x, int y, bool destinationSafe, int enemy = 0,
                             bool usedSonar = false);
        bool daringLocked() const;
        bool canAdvanceDaring() const;
        bool daringAdvancePending = false;
        std::optional<sfs::Message> advanceDaring();
        double daringProgress() const;
        void tick(float dt);
        std::string summary() const;
        std::string catalog() const;
    };

    class GridSprite;
    class GfxOceanGradient;
    class OceanView {
      public:
        using SpriteKey = std::pair<int, int>; // parent raft, object ID (parent=0 for raft)
        explicit OceanView(std::filesystem::path root);
        ~OceanView();
        OceanView(const OceanView&) = delete;
        OceanView& operator=(const OceanView&) = delete;
        void Init();
        void AddToTick(int parent, int id);
        void RemoveFromTick(int parent, int id);
        unsigned Layer() const;
        void SetLayer(unsigned layer);
        void SetBossOcean(bool enabled);
        void SetRaftMute(int raft, bool mute);
        bool RaftMuted(int raft) const;
        void gotMsgDiveUnitLoadChange(int parent, int id, int load, int maximum);
        void SetOcean(const Player&);
        void AddGrid(const Raft&, const std::map<int, GridObjectInfo>&);
        void RemoveGrid(int raft);
        void AddSprite(int parent, const GridObject&, const GridObjectInfo&, sys::gfx::Vec2 world);
        void RemoveSprite(int parent, int id);
        void UpdateSprite(int parent, const GridObject&, const GridObjectInfo&, sys::gfx::Vec2 world);
        GridSprite* getGridSprite(int parent, int id) const;
        void SetRaftTransparent(int raft, float alpha);
        void AddAnimSlack(int parent, int id, float seconds);
        void UpdateAnimations(int parent, int id, float seconds);
        void gotMsgAnimateGridSprite(int parent, int id, std::vector<std::string> names);
        void gotMsgSparseGridObjectAdded(int parent, const GridObject&, const GridObjectInfo&,
                                         sys::gfx::Vec2 world);
        void gotMsgSparseGridObjectRemoved(int parent, int id);
        void gotMsgGridObjectEvent(int parent, const GridObject&, const GridObjectInfo&,
                                  sys::gfx::Vec2 world);
        void gotMsgUpdateOceanView(const Player&);
        void PlayRaftAttackAnims(int raft, float delay);
        void SetOrdering();
        void SetTempPriorities(GridSprite&);
        bool GetPriority(const GridSprite*, unsigned piece, float& result) const;
        float GetSpritePriority(const GridSprite*, unsigned piece) const;
        std::vector<SpriteKey> FillPrioritySorter() const;
        sys::gfx::Vec2 FocalPoint() const;
        void SetAxiseseses(sys::gfx::Vec2 x, sys::gfx::Vec2 y);
        void showHealthBars(bool show);
        void removeAllHealthBars();
        void ShowWaterBuckets(bool show);
        bool CheckForBucket(int parent, int id) const;
        void tick(float dt);
        size_t spriteCount() const;
        bool gotMsgGfxLayerPickResult(int parent, int id);
        std::optional<SpriteKey> pickedSprite() const;

        sys::gfx::Image render(const Player&, float time, bool grid,
                               const DiveState* dive = nullptr);
        bool Pick(const Player&, int parent, int object, sys::gfx::Vec2 screen, float time);
        void SetPosition(sys::gfx::Vec2 p);
        void SetWidth(float w);
        void SetHeight(float h);
        void SetPriority(float p);
        void SetUnitSize(float size);
        sys::gfx::Vec2 Position() const;
        float Width() const;
        float Height() const;
        float Priority() const;
        float Unit() const;
        sys::gfx::Vec2 AxisX() const;
        sys::gfx::Vec2 AxisY() const;
        sys::gfx::Vec2 ConvertToViewSpecial(sys::gfx::Vec2 world) const;
        sys::gfx::Vec2 ConvertToView(sys::gfx::Vec2 world) const;
        sys::gfx::Vec2 ConvertToWorld(sys::gfx::Vec2 view) const;
        sys::gfx::Vec2 WorldToScreen(sys::gfx::Vec2 world) const;
        sys::gfx::Vec2 ScreenToWorld(sys::gfx::Vec2 screen) const;
        void SetZoom(float zoom);
        float Zoom() const;
        const std::filesystem::path& assetRoot() const { return root_; }
        void SetOffset(sys::gfx::Vec2 offset, bool immediate);
        sys::gfx::Vec2 Offset() const;
        void CalculateHelpers();
        void Update();

      private:
        struct VisualEntry;
        std::map<SpriteKey, std::unique_ptr<VisualEntry>> sprites_;
        std::map<int, float> raftOpacity_;
        std::set<SpriteKey> ticking_;
        std::set<int> mutedRafts_;
        std::optional<SpriteKey> diveLoadKey_;
        int diveLoad_ = 0, diveCapacity_ = 0;
        unsigned layer_ = 0;
        bool bossOcean_ = false;
        std::optional<SpriteKey> picked_;
        bool healthBars_ = false, waterBuckets_ = false, customAxes_ = false;
        float animationClock_ = 0, lastRenderTime_ = -1;
        void sync(const Player&);
        void appearance(VisualEntry&, int raft);
        sys::gfx::Vec2 position_{320, 86}, offset_{}, axisX_{192, 96}, axisY_{-192, 96};
        float width_ = 640, height_ = 400, priority_ = 0, unit_ = 192, zoom_ = 1;
        std::filesystem::path root_;
        int lastDiveParent_ = 0, lastDiveObject_ = 0;
        bool lastDiving_ = false;
        float returnStart_ = -10;
        sys::gfx::AnimationRenderer animations_;
        std::unique_ptr<GfxOceanGradient> gradient_;
        std::map<std::string, std::shared_ptr<sys::gfx::Image>> images_;
        std::map<std::string, sys::res::ResourceAEAnim> bins_;
        std::shared_ptr<sys::res::ResourceSpriteSheet> overlaySheet_;
    };
} // namespace rp
