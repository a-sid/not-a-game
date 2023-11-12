#include "engine/engine.h"

namespace NotAGame {

Engine::Engine(Mod &M, MapState &MapState) noexcept
    : Mod_{M}, MapState_{MapState}, State_{PrepareGameState{M, MapState.GlobalMap}} {}

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

ErrorOr<LobbyPlayerId> Engine::PlayerConnect(PlayerKind PlayerKind) noexcept {
  return CheckStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->PlayerConnect(PlayerKind); }, "PlayerConnect");
}

Status Engine::PlayerDisconnect(LobbyPlayerId Id) noexcept {
  return CheckStateAndCall<PrepareGameState>(
      [&](auto *State) { return State->PlayerDisconnect(Id); }, "PlayerDisconnect");
}

PlayerReadyResponse Engine::PlayerReady(LobbyPlayerId LobbyPlayerId) noexcept {
  PrepareGameState *CurrentState;
  auto Status = CheckStateAndCall<PrepareGameState>(
      [&](auto *State) {
        CurrentState = State;
        return State->PlayerReady(LobbyPlayerId);
      },
      "PlayerReady");
  PlayerReadyResponse Response;
  if (Status.IsSuccess() && CurrentState->AreAllPlayersReady()) {
    Response.StartGame = StartGame(LobbyPlayerId);
  }
  Response.Result = std::move(Status);
  return Response;
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

NewTurnEvent Engine::NewTurn() noexcept {
  auto &State = std::get<OnlineGameState>(State_);
  auto &PlayerIdx = State.SavedState.CurrentPlayerIdx;
  PlayerIdx = (PlayerIdx + 1) % State.Players.size();
  if (PlayerIdx == 0) {
    ++State.SavedState.Turn;
  }
  const auto PlayerId = State.Players[PlayerIdx].MapId;

  NewTurnEvent Event{.TurnNo = State.SavedState.Turn, .Player = PlayerId};
  auto Income = State.SavedState.Map.Systems.Resources.GetTotalIncome(PlayerId);
  Event.Income = std::move(Income);

  auto CellsGained = State.SavedState.Map.Systems.LandPropagation.Propagate(
      State.SavedState.Map.GlobalMap, PlayerId);
  Event.CellsGained = std::move(CellsGained);
  return Event;
}

const StartGameResponse &Engine::StartGame(LobbyPlayerId LobbyPlayerId) noexcept {
  if (!StartGameResponse_) {
    auto NeutralId = EnsureStateAndCall<PrepareGameState>(
        [&](auto *State) { return State->NeutralPlayerConnect(); }, "NeutralPlayerConnect");
    SetPlayerName(NeutralId, "Neutral");

    auto TurnOrder = EnsureStateAndCall<PrepareGameState>(
        [&](auto *State) { return State->GetTurnOrder(); }, "StartGame");
    auto Players = EnsureStateAndCall<PrepareGameState>(
        [&](auto *State) { return State->CreatePlayers(); }, "CreatePlayers");

    auto &State = State_.emplace<OnlineGameState>(Mod_, MapState_, std::move(Players));
    StartGameResponse_.emplace(
        StartGameResponse{.TurnOrder = {TurnOrder.begin(), TurnOrder.end()}});

    if (State.SavedState.Turn == 0) { // Game start.
      auto Event = NewTurn();
      assert(Event.Income); // FIXME: fine for PoC, but can be nullopt for other players.
      State.SavedState.PlayerStates[Event.Player].ResourcesGained += *Event.Income;

      StartGameResponse_->TurnEvent.emplace(std::move(Event));
    }
  }
  return *StartGameResponse_;
  // TODO: add event to outstanding updates for other players.
}

} // namespace NotAGame
