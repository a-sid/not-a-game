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

template <typename State, typename Fn>
auto Engine::EnsureStateAndCall(Fn &&Func, const char *FnName) noexcept
    -> decltype(Func(static_cast<State *>(nullptr))) {
  auto *SubState = std::get_if<State>(&State_);
  assert(SubState);
  return Func(SubState);
}

ErrorOr<LobbyPlayerId> Engine::PlayerConnect(PlayerSource PlayerSource) noexcept {
  return CheckStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->PlayerConnect(PlayerSource); }, "PlayerConnect");
}

Status Engine::PlayerDisconnect(LobbyPlayerId Id) noexcept {
  return CheckStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->PlayerDisconnect(Id); }, "PlayerDisconnect");
}

ErrorOr<StartGameResponse> Engine::PlayerReady(LobbyPlayerId LobbyPlayerId) noexcept {
  return CheckStateAndCall<PrepareGameState>(
      [&](auto *State) -> ErrorOr<StartGameResponse> {
        auto ReadyStatus = State->PlayerReady(LobbyPlayerId);
        if (ReadyStatus.IsError()) {
          return ReadyStatus;
        }
        if (State->AreAllPlayersReady()) {
          return StartGame();
        }
        return Status::Success();
      },
      "PlayerReady");
}

Status Engine::PlayerNotReady(LobbyPlayerId LobbyPlayerId) noexcept {
  return CheckStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->PlayerNotReady(LobbyPlayerId); }, "PlayerNotReady");
}

Status Engine::SetPlayerName(LobbyPlayerId LobbyPlayerId, std::string Name) noexcept {
  return CheckStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->SetPlayerName(LobbyPlayerId, std::move(Name)); },
      "SetPlayerName");
}

Status Engine::SetPlayerLord(LobbyPlayerId LobbyPlayerId, Id<Lord> LordId) noexcept {
  return CheckStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->SetPlayerLord(LobbyPlayerId, LordId); }, "SetPlayerLord");
}

Status Engine::SetPlayerId(LobbyPlayerId LobbyPlayerId, PlayerId PlayerId) noexcept {
  return CheckStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->SetPlayerId(LobbyPlayerId, PlayerId); }, "SetPlayerCapital");
}

Status Engine::PlayerTurnOrderEarlier(LobbyPlayerId LobbyPlayerId) noexcept {
  return CheckStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->PlayerTurnOrderEarlier(LobbyPlayerId); },
      "PlayerTurnOrderEarlier");
}

Status Engine::PlayerTurnOrderLater(LobbyPlayerId LobbyPlayerId) noexcept {
  return CheckStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->PlayerTurnOrderLater(LobbyPlayerId); },
      "PlayerTurnOrderLater");
}

StartGameResponse Engine::StartGame() noexcept {
  auto NeutralId = EnsureStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->NeutralPlayerConnect(); }, "NeutralPlayerConnect");
  SetPlayerName(NeutralId, "Neutral");

  auto TurnOrder = EnsureStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->GetTurnOrder(); }, "StartGame");
  StartGameResponse Response{.TurnOrder = {TurnOrder.begin(), TurnOrder.end()}};
  return Response;
}

} // namespace NotAGame
