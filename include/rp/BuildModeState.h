#pragma once
#include "rp/Game.h"
#include "rp/Messages.h"
#include "rp/DivingGame.h"
#include <functional>

namespace rp {
    class RPMenuHandler;
    class TouchControls;
    // Reconstructed selection/request portion of the original game state.
    class BuildModeState {
      public:
        enum class ViewMode { Normal=0, Footprints=2 };
        explicit BuildModeState(Game& game);
        void BindView(OceanView* view, TouchControls* controls=nullptr, RPMenuHandler* menu=nullptr);
        void ApplyFootprintToRaft(Raft*, bool grid);
        void CleanupFootprints(Raft*, bool grid, bool points);
        void ChangeView(sys::gfx::Vec2, ViewMode, bool immediate);
        std::unique_ptr<GridObject> CreateGridObject(const std::string&) const;
        bool HandleExtensionResponse(const std::string&, const sfs::SFSObjectWrapper&);
        std::optional<sfs::Message> SendBonusMaterials();
        void cancelNotificationForRaftComponent(int parent, int id);
        void setupNotificationForRaftComponent(int parent, int id, int seconds);
        void gotMsgBuildModeToggleView(const MsgBuildModeToggleView&);
        std::optional<sfs::Message> gotMsgDivingGameDone(const MsgDivingGameDone&);
        void gotMsgDivingGameDoneMoving(const MsgDivingGameDoneMoving&);
        bool gotMsgDivingGameShow(const MsgDivingGameShow&);
        std::optional<sfs::Message> gotMsgGridObjectEvent(const MsgGridObjectEvent&);
        void gotMsgHighlightHudItem(const MsgHighlightHudItem&);
        void gotMsgLoadingAnimFinished(const MsgLoadingAnimFinished&);
        std::optional<sfs::Message> gotMsgOfferWallReward(int amount);
        void gotMsgToggleBlockHudItem(const MsgToggleBlockHudItem&);
        void gotMsgTransitionDone(const MsgTransitionDone&);
        std::optional<sfs::Message> sendSelectedObjectRepairDataWithGold(sfs::SFSObjectWrapper&);
        void tick(float);
        ViewMode viewMode() const { return viewMode_; }
        bool hudBlocked(const std::string& name)const { return blockedHud_.count(name)!=0; }
        struct Notification { int seconds; std::string key, textKey; };
        std::function<void(const Notification&)> scheduleNotification;
        std::function<void(const std::string&)> cancelNotification;
        std::function<void(const std::string&)> questEvent;
        std::function<void(const std::string&)> playMusic;
        std::function<void(const std::string&)> highlightHud;
        ~BuildModeState();
        const char* name()const;
        void Pause();
        void gotMsgSparseGridObjectAdded();
        void SetCheckForHoles(const std::string&);
        bool gotMsgCheckForHoles(int);
        void gotMsgWreckageSelected(int parent,int id);
        std::optional<sfs::Message> doPlaceObject(int blueprint,int parent,Cell position);
        std::optional<sfs::Message> repairObject(bool all)const;
        void selectObject(int parent, int userObjectID);
        void deselectObject();
        std::optional<sfs::Message> repairSelected(bool all) const;
        std::optional<sfs::Message> UseClockSelected() const;
        std::vector<std::string> SendMsgSelectedObjectAvailableActions() const;
        std::optional<sfs::Message> gotMsgRepairSelectedObject() const;
        std::optional<sfs::Message> gotMsgRepairSelectedObjectAll() const;
        std::optional<sfs::Message> gotMsgUseClockOnSelectedObject() const;
        std::optional<sfs::Message> gotMsgSalvageSelectedObject() const;
        std::optional<sfs::Message> salvageSelected() const;
        void gotMsgGridObjectSelected(int parent, int id);
        bool gotMsgStartBuildRequest(int blueprint) const;
        bool CheckForHoles(int removedRaft) const;
        const char* GetStateName() const;

        bool isReadyForBattle() const;
        void checkFocusRaft();
        void StartWaiting(const std::string& reason);
        void FinishWaiting(const std::string& reason);
        void gotMsgBuildCancel();
        void gotMsgDeselectObject();
        std::optional<sfs::Message> gotMsgDestroyObjectRequest() const;
        std::optional<sfs::Message> gotMsgBuildConfirm(int blueprint, int parent, Cell position);
        std::optional<sfs::Message> gotMsgMoveObjectRequest(int newParent, Cell position);
        int gotMsgRequestNumWreckedRafts() const;
        int gotMsgRequestNumWreckedRaftComponents() const;
        void initWreckedRaftCheck();
        void initWreckedRaftComponentCheck();
        void selectWreckedRaft();
        void selectWreckedRaftComponent();
        void gotMsgNextWreckedRaft();
        void gotMsgPreviousWreckedRaft();
        void gotMsgNextWreckedRaftComponent();
        void gotMsgPreviousWreckedRaftComponent();

        std::pair<int, int> selection() const {
            return {parent_, id_};
        }

      private:
        const GridObject* selected() const;
        Game& game_;
        int parent_ = 0, id_ = 0;
        size_t wreckedRaft_ = 0, wreckedComponent_ = 0;
        std::string waiting_;
        bool checkHoles_=false;
        OceanView* ocean_=nullptr;
        TouchControls* controls_=nullptr;
        RPMenuHandler* menu_=nullptr;
        ViewMode viewMode_=ViewMode::Normal;
        std::set<std::string> blockedHud_;
        std::unique_ptr<diving_game::DivingGame> diving_;
        bool closingDive_=false;
        std::map<std::pair<int,int>,Notification> notifications_;
        const RaftComponent* component(int parent,int id)const;
        std::vector<std::pair<int, int>> wrecked(bool components) const;
    };
} // namespace rp
