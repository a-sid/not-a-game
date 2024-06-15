#pragma once

#include "engine/event.h"
#include "engine/path.h"
#include "engine/player.h"
#include "entities/global_map.h"
#include "state/state.h"
#include "status/status.h"

namespace NotAGame {

struct PlayerAddResult {
  Size PlayerId;
};

struct NewTurnEvent : public Event {
  Size TurnNo;
  PlayerId Player;
  std::vector<Coord3D> CellsGained;
  std::optional<Resources> Income; // Only for the player becoming active.
};

struct StartGameResponse {
  SmallVector<Size, kMaxPlayers> TurnOrder;
  Size TurnNo;
  std::optional<NewTurnEvent> TurnEvent;
};

struct PlayerReadyResponse {
  Status Result;
  std::optional<StartGameResponse> StartGame;
};

struct HireLeaderResponse {
  Id<Unit> LeaderId;
  Id<Squad> SquadId;
};

struct HireUnitResponse {
  Id<Unit> UnitId;
};

struct UnitAttackRequest {
  Id<Unit> Attacker;
  Id<Squad> TargetSquadId;
  Coord TargetGridCoord;
};

struct UnitAttackResponse {
  Id<Unit> Target;
  SmallVector<Effect, 4> Effects;
};

struct UnitDefenceRequest {
  Id<Unit> Attacker;
};

struct UnitWaitRequest {
  Id<Unit> Attacker;
};

struct MoveSquadResponse {
  Size NumSteps;
  Size MovePointsRemaining;
  Id<Squad> SquadAttacked;
};

struct AttackOption {
  Size ActionIndex;
  Id<Squad> SquadId;
  Coord GridCoord;
  Id<Unit> UnitId;
};

class EventListener {
public:
  virtual void OnPlayerNewTurn(const NewTurnEvent &Event) noexcept = 0;
};

class Engine {
public:
  Engine(Mod &M, MapState &MapState) noexcept;

  ErrorOr<LobbyPlayerId> PlayerConnect(PlayerKind PlayerKind) noexcept;
  Status PlayerDisconnect(LobbyPlayerId Id) noexcept;
  PlayerReadyResponse PlayerReady(LobbyPlayerId Id) noexcept;
  Status PlayerNotReady(LobbyPlayerId Id) noexcept;
  Status SetPlayerName(LobbyPlayerId Id, std::string Name) noexcept;
  Status SetPlayerLord(LobbyPlayerId PlayerId, Id<Lord> LordId) noexcept;
  Status SetPlayerId(LobbyPlayerId LobbyPlayerId, PlayerId PlayerId) noexcept;
  Status PlayerTurnOrderEarlier(LobbyPlayerId LobbyPlayerId) noexcept;
  Status PlayerTurnOrderLater(LobbyPlayerId LobbyPlayerId) noexcept;

  const StartGameResponse &StartGame(LobbyPlayerId LobbyPlayerId) noexcept;
  ErrorOr<HireLeaderResponse> HireLeader(PlayerId PlayerId, Id<GuardComponent> GuardComponentId,
                                         MapObjectId MapObjectId, Id<Unit> UnitPresetId,
                                         Coord GridPosition) noexcept;
  ErrorOr<HireUnitResponse> HireUnit(PlayerId PlayerId, Id<GuardComponent> GuardComponentId,
                                     Id<Unit> UnitPresetId, Coord GridPosition) noexcept;

  ErrorOr<MoveSquadResponse> MoveSquad(PlayerId PlayerId, Id<Squad> SquadId,
                                       const Path &Path) noexcept;
  void PerformAction(GameplaySystems &Systems, Unit &U,
                     const AttackOption &AttackOpt) noexcept;

  Status EndTurn(const Player &Player) noexcept;

  void SetEventListener(EventListener *Listener) noexcept { EventListener_ = Listener; }

  OnlineGameState *GetOnlineState() { return std::get_if<OnlineGameState>(&State_); }

private:
  enum class UnitTurnResult {
    PlayerAwait,
    Victory,
    TurnOver,
  };

  template <typename State, typename Fn>
  auto CheckStateAndCall(Fn &&Func, const char *FnName) noexcept
      -> decltype(Func(static_cast<State *>(nullptr)));

  template <typename State, typename Fn>
  auto EnsureStateAndCall(Fn &&Func, const char *FnName) noexcept
      -> decltype(Func(static_cast<State *>(nullptr)));

  NewTurnEvent NewTurn() noexcept;

  void CreateBattleState(Squad &Attacker, Squad &Defender) noexcept;
  void NewBattleRound(BattleState &BattleState, Squad &Attacker, Squad &Defender) noexcept;
  Id<Squad> CheckBattleVictory(GameplaySystems &Systems, Squad &Attacker, Squad &Defender) noexcept;
  UnitTurnResult DoUnitTurns(GameplaySystems &Systems, BattleState &FightState) noexcept;
  void DoAIBattleAction(GameplaySystems &Systems, BattleState &FightState) noexcept;
  SmallVector<AttackOption, 8> FillAIAttackOptions(GameplaySystems &Systems,
                                                   BattleState &FightState, const Unit &U) noexcept;
  AttackOption AISelectAction(GameplaySystems &Systems, BattleState &FightState,
                              const Unit &U) noexcept;

  Mod &Mod_;
  MapState &MapState_;
  GameState State_;
  EventListener *EventListener_;
  std::optional<StartGameResponse> StartGameResponse_;

  using OutstandingUpdates = SmallVector<std::unique_ptr<Event>, 16>;
  SmallVector<OutstandingUpdates, kMaxPlayers> OutstandingUpdates_;
};

} // namespace NotAGame
