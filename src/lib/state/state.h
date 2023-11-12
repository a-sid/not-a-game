#pragma once

#include "entities/unit.h"
#include "game/map.h"
#include "game/mod.h"
#include "util/types.h"

#include "engine/player.h"
#include "engine/player_state.h"

#include <cstdint>
#include <deque>
#include <span>
#include <string>
#include <unordered_map>

namespace NotAGame {

using PlayersList = SmallVector<Player, kMaxPlayers>;

using LobbyPlayerId = Size;

class PrepareGameState {
public:
  explicit PrepareGameState(Mod &M, GlobalMap &Map) noexcept
      : Mod_{M}, GlobalMap_{Map}, PlayersCount_{GlobalMap_.GetNumCapitals()} {}

  ErrorOr<LobbyPlayerId> PlayerConnect(PlayerKind PlayerKind) noexcept;
  LobbyPlayerId NeutralPlayerConnect() noexcept;
  Status PlayerDisconnect(LobbyPlayerId LobbyPlayerId) noexcept;
  Status PlayerReady(LobbyPlayerId LobbyPlayerId) noexcept;
  Status PlayerNotReady(LobbyPlayerId LobbyPlayerId) noexcept;
  Status SetPlayerName(LobbyPlayerId LobbyPlayerId, std::string Name) noexcept;
  Status SetPlayerLord(LobbyPlayerId LobbyPlayerId, Id<Lord> LordId) noexcept;
  Status SetPlayerId(LobbyPlayerId LobbyPlayerId, PlayerId PlayerId) noexcept;
  Status PlayerTurnOrderEarlier(LobbyPlayerId LobbyPlayerId) noexcept;
  Status PlayerTurnOrderLater(LobbyPlayerId LobbyPlayerId) noexcept;

  bool AreAllPlayersReady() const noexcept { return NumPlayersReady_ == PlayersCount_; }
  std::span<const LobbyPlayerId> GetTurnOrder() const noexcept {
    return {&*TurnOrder_.begin(), TurnOrder_.size()};
  }

  PlayersList CreatePlayers() const noexcept;

private:
  static Status MakePlayerNotFoundError(PlayerId Id) noexcept {
    return Status::Error(ErrorCode::LobbyFull) << "Player " << Id << " not found";
  }

  Mod &Mod_;
  GlobalMap &GlobalMap_;
  std::unordered_map<LobbyPlayerId, Player> Players_;
  std::unordered_map<PlayerId, LobbyPlayerId> PlayerIdToLobbyId_;
  std::unordered_map<LobbyPlayerId, PlayerId> LobbyIdToMapId_;

  SmallVector<LobbyPlayerId, kMaxPlayers> TurnOrder_;
  Size PlayersCount_;
  LobbyPlayerId LobbyPlayerIdCounter_ = 0;
  Size NumPlayersReady_ = 0;
  Size DifficultyLevel_ = 0;
};

struct SpellBook {
  SmallVector<std::vector<Id<Spell>>, 8> SpellsByLevels;
};

struct MapState {
  NotAGame::GlobalMap GlobalMap;
  NotAGame::GameplaySystems Systems;
};

struct PlayerGameState {
  PlayerGameState(PlayerId PlayerId, const Mod &M, MapState &Map) noexcept;

  // static PlayerGameState MakeInitialState(GameplaySystems &Systems, GlobalMap &Map) noexcept;

  PlayerId Player;
  const Mod &M;
  MapState &Map;
  Resources ResourcesGained;
  std::unordered_set<Id<Building>> Buildings;
  SpellBook Spells;
  // std::unordered_set<Id<MapObjectPtr>> Towns_;
  //  std::unordered_set<Id<Squad>> Squads_;
};

struct SavedGameState {
  SavedGameState(Mod &M, MapState &Map, Size NumPlayers) noexcept : M{M}, Map{Map} {
    PlayerStates.reserve(NumPlayers);
    for (size_t I = 0; I < NumPlayers; ++I) {
      PlayerStates.emplace_back(I, M, Map);
    }
  }

  Mod &M;
  MapState &Map;

  SmallVector<PlayerGameState, kMaxPlayers> PlayerStates;

  Size CurrentPlayerIdx = -1;
  Size Turn = 0;
};

struct OnlineGameState {
public:
  OnlineGameState(Mod &M, MapState &Map, PlayersList Players) noexcept;

  std::unordered_map<Size, Id<Player>> MapIndexToPlayer;
  std::unordered_map<Id<Player>, Size> PlayerToMapIndex;

  SavedGameState SavedState;

  PlayersList Players;
};

using GameState = std::variant<PrepareGameState, OnlineGameState>;

} // namespace NotAGame
