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

using Players = SmallVector<Player, kMaxPlayers>;

using LobbyPlayerId = Size;

class PrepareGameState {
public:
  explicit PrepareGameState(Mod &M, GlobalMap &Map) noexcept
      : Mod_{M}, GlobalMap_{Map}, PlayersCount_{GlobalMap_.GetNumCapitals()} {}

  ErrorOr<LobbyPlayerId> PlayerConnect(PlayerSource PlayerSource) noexcept;
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

private:
  static Status MakePlayerNotFoundError(PlayerId Id) noexcept {
    return Status::Error(ErrorCode::LobbyFull) << "Player " << Id << " not found";
  }

  Mod &Mod_;
  GlobalMap &GlobalMap_;
  std::unordered_map<Size, Player> Players_;
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

class PlayerGameState {
public:
  PlayerGameState(GlobalMap &Map, std::vector<bool> TileVisibility) noexcept;

  static PlayerGameState MakeInitialState(GlobalMap &Map) noexcept;

private:
  GlobalMap &GlobalMap_;
  PlayerId Player_;
  std::vector<bool> TileVisibility_;
  std::unordered_set<Id<MapObjectPtr>> Towns_;
  std::unordered_set<Id<Building>> Buildings_;
  std::unordered_set<Id<Squad>> Squads_;
  SpellBook Spells_;
};

class OnlineGameState {
public:
  OnlineGameState(GlobalMap &Map) noexcept;

private:
  std::unordered_map<Size, Id<Player>> MapIndexToPlayer_;
  std::unordered_map<Id<Player>, Size> PlayerToMapIndex_;

  SmallVector<PlayerState, kMaxPlayers> PlayerStates_;

  GlobalMap &GlobalMap_;
  // std::deque<Unit> Units_;
  // std::deque<Item> Items_;

  Players Players_;
  Players::iterator CurrentPlayer_;

  Size Turn_ = 0;
};

using GameState = std::variant<PrepareGameState, OnlineGameState>;

} // namespace NotAGame
