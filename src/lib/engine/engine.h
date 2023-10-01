#pragma once

#include "engine/player.h"
#include "entities/global_map.h"
#include "state/state.h"
#include "status/status.h"

namespace NotAGame {

struct PlayerAddResult {
  Size PlayerId;
};

class Engine {
public:
  Engine(Mod &M, GlobalMap &Map) noexcept;
  ErrorOr<PlayerAddResult> AddPlayer(const Player &Player) noexcept;

  ErrorOr<Size> PlayerConnect() noexcept {
    return CheckStateAndCall<PrepareGameState>([&](auto *State) { return State->PlayerConnect(); },
                                               "PlayerConnect");
  }

  Status PlayerDisconnect(PlayerId Id) noexcept {
    return CheckStateAndCall<PrepareGameState>(
        [&](auto *State) { return State->PlayerDisconnect(Id); }, "PlayerDisconnect");
  }

  Status PlayerReady(PlayerId Id) noexcept {
    auto ReadyStatus = CheckStateAndCall<PrepareGameState>(
        [&](auto *State) { return State->PlayerReady(Id); }, "PlayerReady");
    if (ReadyStatus.IsError()) {
      return ReadyStatus;
    }
    return StartGame();
  }

  Status PlayerNotReady(PlayerId Id) noexcept {
    return CheckStateAndCall<PrepareGameState>(
        [&](auto *State) { return State->PlayerNotReady(Id); }, "PlayerNotReady");
  }

  Status SetPlayerName(PlayerId Id, std::string Name) noexcept {
    return CheckStateAndCall<PrepareGameState>(
        [&](auto *State) { return State->SetPlayerName(Id, std::move(Name)); }, "SetPlayerName");
  }

  Status SetPlayerLord(PlayerId PlayerId, Id<Lord> LordId) noexcept {
    return CheckStateAndCall<PrepareGameState>(
        [&](auto *State) { return State->SetPlayerLord(PlayerId, LordId); }, "SetPlayerLord");
  }

  Status SetPlayerCapital(PlayerId PlayerId, Id<MapObjectPtr> CapitalId) noexcept {
    return CheckStateAndCall<PrepareGameState>(
        [&](auto *State) { return State->SetPlayerCapital(PlayerId, CapitalId); },
        "SetPlayerCapital");
  }

  Status PlayerTurnOrderEarlier(PlayerId PlayerId) noexcept {
    return CheckStateAndCall<PrepareGameState>(
        [&](auto *State) { return State->PlayerTurnOrderEarlier(PlayerId); },
        "PlayerTurnOrderEarlier");
  }

  Status PlayerTurnOrderLater(PlayerId PlayerId) noexcept {
    return CheckStateAndCall<PrepareGameState>(
        [&](auto *State) { return State->PlayerTurnOrderLater(PlayerId); }, "PlayerTurnOrderLater");
  }

  Status StartGame() noexcept;
  Status EndTurn(const Player &Player) noexcept;

private:
  template <typename State, typename Fn>
  auto CheckStateAndCall(Fn &&Func, const char *FnName) noexcept
      -> decltype(Func(static_cast<State *>(nullptr))) {
    auto *SubState = std::get_if<State>(&State_);
    if (!SubState) {
      return Status::Error(ErrorCode::WrongState)
             << "Cannot handle " << FnName << " out of game setup";
    }
    return Func(SubState);
  }

  GameState State_;
};

} // namespace NotAGame
