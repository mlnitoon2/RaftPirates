#pragma once
#include "rp/Player.h"
#include "rp/Ocean.h"
#include "sfs/SFSWire.h"
#include <optional>
#include <array>
#include <set>

namespace sys::gfx {
    class AEAnim;
    class Gfx;
} // namespace sys::gfx

namespace rp {
    class Game;
    class RPMenuHandler;
    struct MsgBattleInfoDropdown;
    struct MsgBattleInfoRequest;
    struct MsgHighlightHudItem;
    struct MsgQuestRead;
    struct MsgToggleBlockHudItem;
    struct MsgTransitionDone;

    // Original battle state name; online actions remain server-authoritative.
    class BattleModeState {
      public:
        // Native copy constructor owns a string and two graphics references.
        // Unidentified scalar slots are preserved without assigning gameplay semantics.
        struct BattleDice {
            BattleDice() = default;
            BattleDice(const BattleDice&);
            BattleDice& operator=(const BattleDice&) = default;
            ~BattleDice();
            std::array<uint32_t, 3> leading{};
            std::string label;
            std::array<uint32_t, 8> trailing{};
            std::shared_ptr<sys::gfx::AEAnim> animation;
            std::shared_ptr<sys::gfx::Gfx> graphics;
        };

        BattleModeState();
        const char* name() const;
        void Pause();
        void HideDropdown();
        void StartWaiting(float seconds);
        void StopWaiting();
        void tick(float dt);

        float waitRemaining() const {
            return waitRemaining_;
        }

        void gotMsgBattleConfirmSwapStance();
        void gotMsgToPositioningView();

        struct BattleRoll {
            int attack = 0, defense = 0, targetRaft = 0;
        };

        struct BattleInfo {
            std::string name;
            int points = 0, healthPercent = 0;
            bool own = false;
        };

        void PlayBattleAnimations();
        void SwapPopup();
        void SwapStance();
        void ToBattleView();
        void ToPositioningView();
        void gotMsgBattleInfoDropdown(const MsgBattleInfoDropdown&);
        std::vector<BattleInfo> gotMsgBattleInfoRequest(const MsgBattleInfoRequest&,
                                                        const Player&) const;
        void gotMsgHighlightHudItem(const MsgHighlightHudItem&);
        void gotMsgQuestRead(const MsgQuestRead&, RPMenuHandler* menu = nullptr);
        void gotMsgToggleBlockHudItem(const MsgToggleBlockHudItem&);
        void gotMsgTransitionDone(const MsgTransitionDone&);

        const std::vector<BattleRoll>& battleRolls() const {
            return rolls_;
        }

        float animationRemaining() const {
            return animationRemaining_;
        }

        float introRemaining() const {
            return introRemaining_;
        }

        bool dicePaused() const {
            return dicePaused_;
        }

        bool battleView() const {
            return battleView_;
        }

        bool swapPopupVisible() const {
            return swapPopupVisible_;
        }

        int dropdownType() const {
            return dropdownType_;
        }

        bool hudBlocked(const std::string& item) const {
            return blockedHud_.count(item) != 0;
        }

        bool shouldSendClientReady() const {
            return phase_ == Phase::Fighting && needReady_ && !waiting() && !dicePaused_ &&
                   animationRemaining_ <= 0 && !swapPopupVisible_ && !pendingSwap_;
        }

        unsigned resultSequence() const {
            return resultSequence_;
        }

        bool resultAttacker() const {
            return resultAttacker_;
        }

        int resultAttackRaft() const {
            return resultAttackRaft_;
        }

        void cancelSwapPopup() {
            swapPopupVisible_ = false;
        }

        std::optional<sfs::Message> RaftSelect(int own, int enemy, const Player&);
        std::optional<sfs::Message> endBattleAndPlunder();
        std::optional<sfs::Message> gotMsgBattleOverConfirm();
        std::optional<sfs::Message> gotMsgExitBattleModeRequest();
        ~BattleModeState();
        const char* GetStateName() const;
        std::optional<sfs::Message> gotMsgAttackRequest(int dice);
        enum class Phase { Idle, Encounter, Starting, Positioning, Fighting, Over, Desynced };

        Phase phase() const {
            return phase_;
        }

        const Player& opponent() const {
            return *opponent_;
        }

        bool hasOpponent() const {
            return haveOpponent_;
        }

        const Ocean& ocean() const {
            return ocean_;
        }

        bool isBoss() const {
            return boss_;
        }

        int ownRaftId() const {
            return ownRaft_;
        }

        int enemyRaftId() const {
            return enemyRaft_;
        }

        bool positioned() const {
            return positioned_;
        }

        bool readyForDice() const {
            return phase_ == Phase::Fighting && !waiting() && !needReady_ && !dicePaused_ &&
                   animationRemaining_ <= 0 && !swapPopupVisible_ && !pendingSwap_;
        }

        bool attacker() const {
            return attacker_;
        }

        int syncCounter() const {
            return sync_;
        }

        int opponentTime() const {
            return opponentTime_;
        }

        bool waiting() const {
            return !pending_.empty();
        }

        const std::string& message() const {
            return message_;
        }

        const sfs::SFSObjectWrapper& lastResult() const {
            return lastResult_;
        }

        std::optional<sfs::Message> encounterResponse(bool attack);
        std::optional<sfs::Message> gotMsgBattleScreenRaftsSelected(int x, int y);
        std::optional<sfs::Message> SendRaftSelection(int own, int enemy, const Player& player);
        std::optional<sfs::Message> SendClientReady(bool confirmSwap = false);
        std::optional<sfs::Message> attack(int dice);
        std::optional<sfs::Message> defend(int dice);
        std::optional<sfs::Message> retreat();
        std::optional<sfs::Message> plunder();
        bool HandleExtensionResponse(const std::string& command, const sfs::SFSObjectWrapper& data,
                                     Player& player);
        void reset();
        void sendFailed();

      private:
        Phase phase_ = Phase::Idle;
        std::shared_ptr<Player> opponent_ = std::make_shared<Player>();
        Ocean ocean_;
        bool attacker_ = false, haveOpponent_ = false, plunderSent_ = false, needReady_ = false,
             decline_ = false, boss_ = false, positioned_ = false, swap_ = false;
        int ownRaft_ = 0, enemyRaft_ = 0;
        int sync_ = 0, opponentTime_ = 0;
        std::string pending_, message_;
        float waitRemaining_ = 0;
        bool dropdownVisible_ = false, confirmSwap_ = false;
        sfs::SFSObjectWrapper lastResult_;
        std::optional<sfs::SFSObjectWrapper> pendingSwap_;
        std::optional<bool> swapAttacker_;
        std::vector<BattleRoll> rolls_;
        std::set<std::string> blockedHud_;
        float animationRemaining_ = 0, introRemaining_ = 0;
        bool dicePaused_ = false, battleView_ = false, swapPopupVisible_ = false;
        int dropdownType_ = -1;
        unsigned resultSequence_ = 0;
        bool resultAttacker_ = false;
        int resultAttackRaft_ = 0;
        std::optional<sfs::Message> action(const std::string& command,
                                           sfs::SFSObjectWrapper data = {}, bool withSync = true);
    };
} // namespace rp
