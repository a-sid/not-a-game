#include "engine/engine.h"

namespace NotAGame {

Engine::Engine(Mod &M, GlobalMap &Map) noexcept : State_{PrepareGameState{M, Map}} {}

template <typename State, typename Fn>
auto Engine::CheckStateAndCall(Fn &&Func, const char *FnName) noexcept
    -> decltype(Func(static_cast<State *>(nullptr))) {
  auto *SubState = std::get_if<State>(&State_);
  if (!SubState) {
    return Status::Error(ErrorCode::WrongState)
           << "Cannot handle " << FnName << " out of game setup";
  }
  return Func(SubState);
}

ErrorOr<Size> Engine::PlayerConnect() noexcept {
  return CheckStateAndCall<PrepareGameState>([&](auto *State) { return State->PlayerConnect(); },
                                             "PlayerConnect");
}

Status Engine::PlayerDisconnect(PlayerId Id) noexcept {
  return CheckStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->PlayerDisconnect(Id); }, "PlayerDisconnect");
}

Status Engine::PlayerReady(PlayerId Id) noexcept {
  auto ReadyStatus = CheckStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->PlayerReady(Id); }, "PlayerReady");
  if (ReadyStatus.IsError()) {
    return ReadyStatus;
  }
  return StartGame();
}

Status Engine::PlayerNotReady(PlayerId Id) noexcept {
  return CheckStateAndCall<PrepareGameState>([&](auto *State) { return State->PlayerNotReady(Id); },
                                             "PlayerNotReady");
}

Status Engine::SetPlayerName(PlayerId Id, std::string Name) noexcept {
  return CheckStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->SetPlayerName(Id, std::move(Name)); }, "SetPlayerName");
}

Status Engine::SetPlayerLord(PlayerId PlayerId, Id<Lord> LordId) noexcept {
  return CheckStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->SetPlayerLord(PlayerId, LordId); }, "SetPlayerLord");
}

Status Engine::SetPlayerCapital(PlayerId PlayerId, Id<MapObjectPtr> CapitalId) noexcept {
  return CheckStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->SetPlayerCapital(PlayerId, CapitalId); },
      "SetPlayerCapital");
}

Status Engine::PlayerTurnOrderEarlier(PlayerId PlayerId) noexcept {
  return CheckStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->PlayerTurnOrderEarlier(PlayerId); },
      "PlayerTurnOrderEarlier");
}

Status Engine::PlayerTurnOrderLater(PlayerId PlayerId) noexcept {
  return CheckStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->PlayerTurnOrderLater(PlayerId); }, "PlayerTurnOrderLater");
}

Status Engine::StartGame() noexcept {
  // Are the slots filled?
  Player AI{.Name = "Neutral", .PlayerColor = NotAGame::ColorByTurnOrder(0)};
  return Status::Success();
}

} // namespace NotAGame
