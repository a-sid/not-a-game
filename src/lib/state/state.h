#pragma once

#include "entities/unit.h"
#include "game/map.h"
#include "game/mod.h"
#include "util/types.h"

#include "engine/player.h"
#include "engine/player_state.h"

#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>

namespace NotAGame {

using Players = SmallVector<Player, kMaxPlayers>;

using PlayerId = Size;

class PrepareGameState {
public:
  explicit PrepareGameState(Mod &M, GlobalMap &Map) noexcept
      : Mod_{M}, GlobalMap_{Map}, PlayersCount_{GlobalMap_.GetNumCapitals()} {}

  ErrorOr<Size> PlayerConnect() noexcept;
  Status PlayerDisconnect(PlayerId Id) noexcept;
  Status PlayerReady(PlayerId Id) noexcept;
  Status PlayerNotReady(PlayerId Id) noexcept;
  Status SetPlayerName(PlayerId Id, std::string Name) noexcept;
  Status SetPlayerLord(PlayerId PlayerId, Id<Lord> LordId) noexcept;
  Status SetPlayerCapital(PlayerId PlayerId, Id<MapObjectPtr> CapitalId) noexcept;
  Status PlayerTurnOrderEarlier(PlayerId PlayerId) noexcept;
  Status PlayerTurnOrderLater(PlayerId PlayerId) noexcept;

  bool AreAllPlayersReady() const noexcept { return NumPlayersReady_ == PlayersCount_; }

private:
  static Status MakePlayerNotFoundError(PlayerId Id) noexcept {
    return Status::Error(ErrorCode::LobbyFull) << "Player " << Id << " not found";
  }

  Mod &Mod_;
  GlobalMap &GlobalMap_;
  std::unordered_map<Size, Player> Players_;
  std::unordered_map<Id<MapObjectPtr>, Size> CapitalToPlayer_;
  std::unordered_map<Size, Id<MapObjectPtr>> PlayerToCapital_;

  std::vector<PlayerId> TurnOrder_;
  Size PlayersCount_;
  Size NumPlayersReady_ = 0;
  Size DifficultyLevel_ = 0;
};

class OnlineGameState {
public:
  OnlineGameState(GlobalMap &Map) noexcept : GlobalMap_{Map} {}

private:
  std::unordered_map<Size, Id<Player>> MapIndexToPlayer_;
  std::unordered_map<Id<Player>, Size> PlayerToMapIndex_;

  GlobalMap &GlobalMap_;
  // std::deque<Unit> Units_;
  // std::deque<Item> Items_;

  Players Players_;
  Players::iterator CurrentPlayer_;
  Size Turn_ = 0;
};

using GameState = std::variant<PrepareGameState, OnlineGameState>;

} // namespace NotAGame
