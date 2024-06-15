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

ErrorOr<HireLeaderResponse> Engine::HireLeader(PlayerId PlayerId,
                                               Id<GuardComponent> GuardComponentId,
                                               MapObjectId MapObjectId, Id<Unit> UnitPresetId,
                                               Coord GridPosition) noexcept {
  auto &State = std::get<OnlineGameState>(State_);
  auto &PlayerIdx = State.SavedState.CurrentPlayerIdx;
  auto CurrentPlayerId = State.Players[PlayerIdx].MapId;

  if (PlayerId != CurrentPlayerId) {
    return Status::Error(ErrorCode::WrongPlayer, "Not this player's turn!");
  }

  const auto &Unit = Mod_.GetUnitPresets().GetObjectById(UnitPresetId);
  if (Unit.LeaderDataId.IsInvalid()) {
    return Status::Error(ErrorCode::WrongState, "Unit is not a leader!");
  }

  const auto *MapObject = this->MapState_.GlobalMap.TryGetObject(MapObjectId);
  if (!MapObject || !MapObject->GetEntrancePos()) {
    return Status::Error(ErrorCode::WrongState, "Bad map object");
  }

  auto &PlayerState = State.SavedState.PlayerStates[PlayerId];
  if (!(PlayerState.ResourcesGained >= Unit.HireCost)) {
    return Status::Error(ErrorCode::WrongState, "Not enough resources!");
  }

  auto &Systems = State.SavedState.Map.Systems;
  auto &Guard = Systems.Guards.GetComponent(GuardComponentId);
  if (Guard.SquadId.IsValid()) {
    return Status::Error(ErrorCode::WrongState, "Guard slot is busy!");
  }

  PlayerState.ResourcesGained -= Unit.HireCost;

  // Copy preset to the state as a new unit.
  const auto &Leader = Mod_.GetLeaderPresets().GetObjectById(Unit.LeaderDataId);
  auto &AddedLeader = Systems.Leaders.AddComponent(Leader);
  AddedLeader.Name = "Герой";
  auto &AddedUnit = Systems.Units.AddComponent(Unit);
  AddedUnit.LeaderDataId = AddedLeader.ComponentId;

  Squad NewSquad{Mod_.GetGridSettings(), AddedUnit.ComponentId, PlayerId};
  NewSquad.Position = *MapObject->GetEntrancePosAbsolute();
  NewSquad.GuardId = GuardComponentId;

  const auto GridAdd =
      NewSquad.GetGrid().TrySetUnit(AddedUnit.ComponentId, &AddedUnit, GridPosition);
  assert(GridAdd);

  auto &SquadComponent = Systems.Squads.AddComponent(NewSquad);
  AddedUnit.SquadId = SquadComponent.ComponentId;

  Guard.SquadId = SquadComponent.ComponentId;
  // TODO: outstanding updates.

  return HireLeaderResponse{.LeaderId = AddedUnit.ComponentId,
                            .SquadId = SquadComponent.ComponentId};
}

ErrorOr<HireUnitResponse> Engine::HireUnit(PlayerId PlayerId, Id<GuardComponent> GuardComponentId,
                                           Id<Unit> UnitPresetId, Coord GridPosition) noexcept {
  auto &State = std::get<OnlineGameState>(State_);
  auto &PlayerIdx = State.SavedState.CurrentPlayerIdx;
  auto CurrentPlayerId = State.Players[PlayerIdx].MapId;

  if (PlayerId != CurrentPlayerId) {
    return Status::Error(ErrorCode::WrongPlayer, "Not this player's turn!");
  }

  const auto &Unit = Mod_.GetUnitPresets().GetObjectById(UnitPresetId);
  if (Unit.LeaderDataId.IsValid()) {
    return Status::Error(ErrorCode::WrongState, "Unit should not be a leader!");
  }

  auto &Systems = State.SavedState.Map.Systems;
  auto *Guard = Systems.Guards.GetComponentOrNull(GuardComponentId);
  if (!Guard) {
    return Status::Error(ErrorCode::WrongState, "Non-existing guard id!");
  }

  auto *Squad = Systems.Squads.GetComponentOrNull(Guard->SquadId);
  if (!Squad) {
    return Status::Error(ErrorCode::WrongState, "Non-existing squad id!");
  }

  auto *LeaderUnit = Systems.Units.GetComponentOrNull(Squad->GetLeader());
  if (!LeaderUnit) {
    return Status::Error(ErrorCode::WrongState, "Non-existing leader unit id!");
  }

  auto *LeaderComponent = Systems.Leaders.GetComponentOrNull(LeaderUnit->LeaderDataId);
  if (!LeaderComponent) {
    return Status::Error(ErrorCode::WrongState, "Non-existing leader data id!");
  }

  const auto RequiredLeadership = Unit.Width * Unit.Height;
  auto &Leadership = LeaderComponent->Leadership;
  const auto AvailableLeadership = Leadership.GetEffectiveValue() - Leadership.GetValue();
  if (AvailableLeadership < RequiredLeadership) {
    return Status::Error(ErrorCode::WrongState, "Insufficient leadership!");
  }

  // TODO: check if unit is from available units list.

  if (!Squad->GetGrid().CanPlaceUnit(&Unit, GridPosition)) {
    return Status::Error(ErrorCode::WrongState, "Cannot place unit!");
  }

  auto &PlayerState = State.SavedState.PlayerStates[PlayerId];
  if (!(PlayerState.ResourcesGained >= Unit.HireCost)) {
    return Status::Error(ErrorCode::WrongState, "Not enough resources!");
  }

  // All checks passed. Hire unit.
  PlayerState.ResourcesGained -= Unit.HireCost;

  // Copy preset to the state as a new unit.
  auto &AddedUnit = Systems.Units.AddComponent(Unit);
  AddedUnit.SquadId = Squad->ComponentId;

  const auto GridAdd = Squad->GetGrid().TrySetUnit(AddedUnit.ComponentId, &AddedUnit, GridPosition);
  assert(GridAdd);
  Squad->Units.push_back(AddedUnit.ComponentId);

  Leadership.SetValue(Leadership.GetValue() + RequiredLeadership);

  // TODO: outstanding updates.

  return HireUnitResponse{.UnitId = AddedUnit.ComponentId};
}

void Engine::NewBattleRound(BattleState &FightState, Squad &Attacker, Squad &Defender) noexcept {
  auto &State = std::get<OnlineGameState>(State_);
  auto &Systems = State.SavedState.Map.Systems;

  ++FightState.RoundNo;
  FightState.Turn = 0;
  FightState.TurnOrder.clear();

  auto AddAllUnits = [&](Squad &S) {
    for (const auto &UnitId : S.Units) {
      const auto &U = Systems.Units.GetComponent(UnitId);
      if (U.IsAlive()) {
        FightState.TurnOrder.push_back(UnitTurn{.UnitId = U.ComponentId,
                                                .Owner = S.Player_,
                                                .Priority = static_cast<int>(U.Speed.GetValue())});
      }
    }
  };
  AddAllUnits(Attacker);
  AddAllUnits(Defender);

  std::ranges::sort(FightState.TurnOrder, [&Systems](const auto &LHS, const auto &RHS) {
    return LHS.Priority > RHS.Priority;
  });
  for (size_t I = 0, E = FightState.TurnOrder.size(); I < E; ++I) {
    FightState.TurnOrder[I].Priority = E - I;
  }
  // TODO: Initiative roll.
  while (DoUnitTurns(Systems, FightState) == UnitTurnResult::TurnOver) {
    ;
  }
}

Id<Squad> Engine::CheckBattleVictory(GameplaySystems &Systems, Squad &Attacker,
                                     Squad &Defender) noexcept {
  auto AllAreDead = [&Systems](const Squad &S) {
    for (auto UnitId : S.Units) {
      if (const auto &Unit = Systems.Units.GetComponent(UnitId); Unit.IsAlive()) {
        return false;
      }
    }
    return true;
  };

  if (AllAreDead(Attacker)) {
    return Defender.ComponentId;
  }

  if (AllAreDead(Defender)) {
    return Attacker.ComponentId;
  }

  return NullId;
}

void Engine::DoAIBattleAction(GameplaySystems &Systems, BattleState &FightState) noexcept {
  auto &UnitTurn = FightState.TurnOrder[FightState.Turn];
  auto &Unit = Systems.Units.GetComponent(UnitTurn.UnitId);
  auto AttackOption = AISelectAction(Systems, FightState, Unit);
  PerformAction(Systems, Unit, AttackOption);
}

SmallVector<AttackOption, 8> Engine::FillAIAttackOptions(GameplaySystems &Systems,
                                                         BattleState &FightState,
                                                         const Unit &U) noexcept {
  auto &UnitSquad = Systems.Squads.GetComponent(U.SquadId);
  const auto HostileSquadId =
      U.SquadId == FightState.Attacker ? FightState.Defender : FightState.Attacker;
  auto &HostileSquad = Systems.Squads.GetComponent(HostileSquadId);

  SmallVector<AttackOption, 8> Result;
  for (const auto &Action : U.BattleActions) {
    auto Targets = Action.Range->ComputeReachableUnits(Systems, UnitSquad, HostileSquad, U);

    for (auto *Unit : Targets) {
      Result.push_back(AttackOption{.ActionIndex = Action.Index,
                                    .SquadId = Unit->SquadId,
                                    .GridCoord = Unit->GridPosition,
                                    .UnitId = Unit->ComponentId});
    }
  }

  return Result;
}

AttackOption Engine::AISelectAction(GameplaySystems &Systems, BattleState &FightState,
                                    const Unit &U) noexcept {
  auto Options = FillAIAttackOptions(Systems, FightState, U);
  auto &SelectedOption = Options[0];
  return SelectedOption;
}

Engine::UnitTurnResult Engine::DoUnitTurns(GameplaySystems &Systems,
                                           BattleState &FightState) noexcept {
  auto &Attacker = Systems.Squads.GetComponent(FightState.Attacker);
  auto &Defender = Systems.Squads.GetComponent(FightState.Defender);

  while (FightState.Turn < FightState.TurnOrder.size()) {
    // TODO: effects application and removal.
    auto &UnitTurn = FightState.TurnOrder[FightState.Turn];
    const auto &Unit = Systems.Units.GetComponent(UnitTurn.UnitId);
    if (Unit.IsDead()) {
      ++FightState.Turn;
      continue;
    }

    if (GetOnlineState()->Players[UnitTurn.Owner].Source == PlayerKind::Human) {
      return UnitTurnResult::PlayerAwait; // Awaiting player action.
    } else {
      DoAIBattleAction(Systems, FightState);
    }

    const auto WinnerSquadId = CheckBattleVictory(Systems, Attacker, Defender);
    if (WinnerSquadId.IsValid()) {
      return UnitTurnResult::Victory;
    }
  }
  return UnitTurnResult::TurnOver;
}

void Engine::PerformAction(GameplaySystems &Systems, Unit &U,
                           const AttackOption &AttackOpt) noexcept {
  auto &State = std::get<OnlineGameState>(State_);
  auto &FightState = *State.SavedState.FightState;

  auto &Attacker = Systems.Squads.GetComponent(FightState.Attacker);
  auto &Defender = Systems.Squads.GetComponent(FightState.Defender);
  auto &UnitSquad = Systems.Squads.GetComponent(U.SquadId);
  const auto HostileSquadId =
      U.SquadId == FightState.Attacker ? FightState.Defender : FightState.Attacker;
  auto &HostileSquad = Systems.Squads.GetComponent(HostileSquadId);

  const auto &Attack = U.BattleActions[AttackOpt.ActionIndex];
  auto ReachableUnits = Attack.Range->ComputeReachableUnits(Systems, HostileSquad, UnitSquad, U);
  auto &Target = Systems.Units.GetComponent(AttackOpt.UnitId);
  if (std::ranges::find(ReachableUnits, &Target) == ReachableUnits.end()) {
    return; // TODO: return error.
  }
  for (const auto &SubAction : Attack.SubActions) {
    auto Roll = std::rand() % 101;
    if (SubAction.Accuracy.GetValue() < Roll) {
      break;
    }
    SubAction.Effect->Apply(U, Systems.Units.GetComponent(AttackOpt.UnitId));
  }
}

void Engine::CreateBattleState(Squad &Attacker, Squad &Defender) noexcept {
  auto &State = std::get<OnlineGameState>(State_);
  auto &FightState = State.SavedState.FightState.emplace();
  FightState.Attacker = Attacker.ComponentId;
  FightState.Defender = Defender.ComponentId;
  FightState.RoundNo = -1;
  NewBattleRound(FightState, Attacker, Defender);
}

ErrorOr<MoveSquadResponse> Engine::MoveSquad(PlayerId PlayerId, Id<Squad> SquadId,
                                             const Path &Path) noexcept {
  auto &State = std::get<OnlineGameState>(State_);
  auto &PlayerIdx = State.SavedState.CurrentPlayerIdx;
  auto CurrentPlayerId = State.Players[PlayerIdx].MapId;

  if (PlayerId != CurrentPlayerId) {
    return Status::Error(ErrorCode::WrongPlayer, "Not this player's turn!");
  }

  auto &Systems = State.SavedState.Map.Systems;
  auto *Squad = Systems.Squads.GetComponentOrNull(SquadId);
  if (!Squad) {
    return Status::Error(ErrorCode::WrongState, "Non-existing squad id!");
  }

  auto *LeaderUnit = Systems.Units.GetComponentOrNull(Squad->GetLeader());
  if (!LeaderUnit) {
    return Status::Error(ErrorCode::WrongState, "Non-existing leader unit id!");
  }

  auto *LeaderComponent = Systems.Leaders.GetComponentOrNull(LeaderUnit->LeaderDataId);
  if (!LeaderComponent) {
    return Status::Error(ErrorCode::WrongState, "Non-existing leader data id!");
  }

  if (Path.Waypoints.size() < 2) {
    return Status::Error(ErrorCode::WrongState, "Empty path!");
  }
  if (Path.Waypoints[0].Coord != Squad->Position) {
    return Status::Error(ErrorCode::WrongState, "Wrong start point!");
  }

  auto IsReachable = [](Coord3D From, Coord3D To) {
    auto DX = std::max(From.X, To.X) - std::min(From.X, To.X);
    auto DY = std::max(From.Y, To.Y) - std::min(From.Y, To.Y);
    return DX <= 1 && DY <= 1;
  };
  auto MovePointsRemaining = LeaderComponent->Steps.GetValue();
  auto &Map = MapState_.GlobalMap;
  // Dry run.
  size_t Steps = 0;
  for (size_t I = 1; I < Path.Waypoints.size(); ++I, ++Steps) {
    if (MovePointsRemaining == 0) {
      break;
    }
    if (!IsReachable(Path.Waypoints[I - 1].Coord, Path.Waypoints[I].Coord)) {
      return Status::Error(ErrorCode::WrongState, "Unreachable jump");
    }
    auto Cost = ComputeMoveCost(Path.Waypoints[I - 1].Coord, Path.Waypoints[I].Coord, Map, Mod_);
    MovePointsRemaining -= std::min(MovePointsRemaining, Cost);
  }

  if (Squad->GuardId.IsValid()) { // Leaving.
    auto &Guard = Systems.Guards.GetComponent(Squad->GuardId);
    Guard.SquadId = NullId;
  } else {
    Map.GetTile(Squad->Position).Squad_ = NullId;
  }

  Squad->Position = Path.Waypoints[Steps].Coord;
  Map.GetTile(Squad->Position).Squad_ = SquadId;
  LeaderComponent->Steps.SetValue(MovePointsRemaining);

  NotAGame::Squad *OpponentSquad = nullptr;
  for (const auto Neighboor : GetPlaneNeighboors(Squad->Position)) {
    if (!Map.IsValid(Neighboor)) {
      continue;
    }
    if (const auto NeighboorSquadId = Map.GetTile(Neighboor).Squad_; NeighboorSquadId.IsValid()) {
      auto &NeighboorSquad = Systems.Squads.GetComponent(NeighboorSquadId);
      if (NeighboorSquad.Player_ != PlayerId) {
        OpponentSquad = &NeighboorSquad;
        break;
      }
    }
  }

  if (OpponentSquad) {
    CreateBattleState(*Squad, *OpponentSquad);
  }

  // TODO: outstanding updates.
  return MoveSquadResponse{.NumSteps = static_cast<Size>(Steps),
                           .MovePointsRemaining = MovePointsRemaining,
                           .SquadAttacked = OpponentSquad ? OpponentSquad->ComponentId : NullId};
}

} // namespace NotAGame
