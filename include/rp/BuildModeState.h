#pragma once
#include "rp/Game.h"

namespace rp {
    // Reconstructed selection/request portion of the original game state.
    class BuildModeState {
      public:
        explicit BuildModeState(Game& game);
        ~BuildModeState();
        void selectObject(int parent, int userObjectID);
        void deselectObject();
        std::optional<sfs::Message> repairSelected(bool all) const;
        std::optional<sfs::Message> UseClockSelected() const;
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
        std::vector<std::pair<int, int>> wrecked(bool components) const;
    };
} // namespace rp
