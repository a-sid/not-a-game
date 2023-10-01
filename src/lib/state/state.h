#pragma once

#include "entities/unit.h"
#include "game/map.h"
#include "game/mod.h"
#include "util/map_utils.h"
#include "util/types.h"

#include "engine/player.h"
#include "engine/player_state.h"

#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>

namespace NotAGame {

/*struct PlayerAndState {
  Player Player;
  Size MapNumber;
};*/

using Players = SmallVector<Player, kMaxPlayers>;

using PlayerId = Size;

class PrepareGameState {
public:
  explicit PrepareGameState(Mod &M, GlobalMap &Map) noexcept
      : Mod_{M}, GlobalMap_{Map}, PlayersCount_{GlobalMap_.GetNumCapitals()} {}

  ErrorOr<Size> PlayerConnect() noexcept {
    if (Players_.size() == PlayersCount_) {
      return Status::Error(ErrorCode::LobbyFull, "Lobby is full");
    }

    PlayerId Id = Players_.size();
    Players_.emplace(Id, Player{.PlayerId = Id});
    TurnOrder_.push_back(Id);
    return Id;
  }

  Status PlayerDisconnect(PlayerId Id) noexcept {
    const auto PlayerIt = Players_.find(Id);
    if (PlayerIt == Players_.end()) {
      return MakePlayerNotFoundError(Id);
    }

    PlayerToCapital_.erase(Id);
    const auto &Player = PlayerIt->second;
    if (Player.CapitalId.IsValid()) {
      CapitalToPlayer_.erase(Player.CapitalId);
    }

    if (Player.State == PlayerConnectionState::Ready) {
      --NumPlayersReady_;
    }

    std::erase(TurnOrder_, Id);
    Players_.erase(PlayerIt);
    return Status::Success();
  }

  Status PlayerReady(PlayerId Id) noexcept {
    auto *Player = Utils::MapFindPtr(Players_, Id);
    if (!Player) {
      return MakePlayerNotFoundError(Id);
    }

    if (Player->State == PlayerConnectionState::NotReady) {
      Player->State = PlayerConnectionState::Ready;
      ++NumPlayersReady_;
    }

    return Status::Success();
  }

  Status PlayerNotReady(PlayerId Id) noexcept {
    auto *Player = Utils::MapFindPtr(Players_, Id);
    if (!Player) {
      return MakePlayerNotFoundError(Id);
    }

    if (Player->State == PlayerConnectionState::Ready) {
      Player->State = PlayerConnectionState::NotReady;
      --NumPlayersReady_;
    }

    return Status::Success();
  }

  Status SetPlayerName(PlayerId Id, std::string Name) noexcept {
    auto *Player = Utils::MapFindPtr(Players_, Id);
    if (!Player) {
      return MakePlayerNotFoundError(Id);
    }

    Player->Name = std::move(Name);
    return Status::Success();
  }

  Status SetPlayerLord(PlayerId PlayerId, Id<Lord> LordId) noexcept {
    auto *Player = Utils::MapFindPtr(Players_, PlayerId);
    if (!Player) {
      return MakePlayerNotFoundError(PlayerId);
    }

    if (LordId >= Mod_.GetLords().size()) {
      return Status::Error(ErrorCode::ObjectNotFound) << "Lord with id " << LordId << " not found";
    }

    Player->LordId = LordId;
    return Status::Success();
  }

  Status SetPlayerCapital(PlayerId PlayerId, Id<MapObjectPtr> CapitalId) noexcept {
    auto *Player = Utils::MapFindPtr(Players_, PlayerId);
    if (!Player) {
      return MakePlayerNotFoundError(PlayerId);
    }
    const auto Capitals = GlobalMap_.GetCapitals();
    if (std::find(Capitals.begin(), Capitals.end(), CapitalId) == Capitals.end()) {
      return Status::Error(ErrorCode::ObjectNotFound)
             << "Capital with object id " << CapitalId << " not found";
    }

    const auto *ExistingPlayer = Utils::MapFindPtr(CapitalToPlayer_, CapitalId);
    if (ExistingPlayer && *ExistingPlayer != PlayerId) {
      return Status::Error(ErrorCode::GameSlotBusy)
             << "Capital with object id " << CapitalId << " is busy by another player with id "
             << *ExistingPlayer;
    }

    Player->CapitalId = CapitalId;
    PlayerToCapital_[PlayerId] = CapitalId;
    CapitalToPlayer_[CapitalId] = PlayerId;
    return Status::Success();
  }

  Status PlayerTurnOrderEarlier(PlayerId PlayerId) noexcept {
    auto *Player = Utils::MapFindPtr(Players_, PlayerId);
    if (!Player) {
      return MakePlayerNotFoundError(PlayerId);
    }

    auto Pos = std::find(TurnOrder_.begin(), TurnOrder_.end(), PlayerId);
    if (Pos != TurnOrder_.begin()) {
      auto Prev = std::prev(Pos);
      std::swap(*Pos, *Prev);
    }

    return Status::Success();
  }

  Status PlayerTurnOrderLater(PlayerId PlayerId) noexcept {
    auto *Player = Utils::MapFindPtr(Players_, PlayerId);
    if (!Player) {
      return MakePlayerNotFoundError(PlayerId);
    }

    auto Pos = std::find(TurnOrder_.begin(), TurnOrder_.end(), PlayerId);
    auto Next = std::next(Pos);
    if (Next != TurnOrder_.end()) {
      std::swap(*Pos, *Next);
    }

    return Status::Success();
  }

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
