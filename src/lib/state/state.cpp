#include "state/state.h"

#include "util/map_utils.h"

#include <range/v3/all.hpp>

namespace NotAGame {

ErrorOr<LobbyPlayerId> PrepareGameState::PlayerConnect(PlayerKind PlayerKind) noexcept {
  if (Players_.size() == PlayersCount_) {
    return Status::Error(ErrorCode::LobbyFull, "Lobby is full");
  }

  LobbyPlayerId Id = LobbyPlayerIdCounter_++;
  Players_.emplace(Id, Player{.LobbyId = Id, .Source = PlayerKind});
  TurnOrder_.push_back(Id);
  return Id;
}

LobbyPlayerId PrepareGameState::NeutralPlayerConnect() noexcept {
  LobbyPlayerId Id = LobbyPlayerIdCounter_++;
  Players_.emplace(Id, Player{.LobbyId = Id, .Source = PlayerKind::AI});
  TurnOrder_.push_back(Id);
  return Id;
}

Status PrepareGameState::PlayerDisconnect(LobbyPlayerId LobbyPlayerId) noexcept {
  const auto PlayerIt = Players_.find(LobbyPlayerId);
  if (PlayerIt == Players_.end()) {
    return MakePlayerNotFoundError(LobbyPlayerId);
  }

  LobbyIdToMapId_.erase(LobbyPlayerId);
  const auto &Player = PlayerIt->second;
  if (Player.CapitalId.IsValid()) {
    PlayerIdToLobbyId_.erase(Player.MapId);
  }

  if (Player.State == PlayerConnectionState::Ready) {
    --NumPlayersReady_;
  }

  auto TurnIt = ranges::find(TurnOrder_, LobbyPlayerId);
  assert(TurnIt != TurnOrder_.end());
  TurnOrder_.erase(TurnIt);
  Players_.erase(PlayerIt);
  return Status::Success();
}

Status PrepareGameState::PlayerReady(LobbyPlayerId LobbyPlayerId) noexcept {
  auto *Player = Utils::MapFindPtr(Players_, LobbyPlayerId);
  if (!Player) {
    return MakePlayerNotFoundError(LobbyPlayerId);
  }

  if (Player->State == PlayerConnectionState::NotReady) {
    Player->State = PlayerConnectionState::Ready;
    ++NumPlayersReady_;
  }

  return Status::Success();
}

Status PrepareGameState::PlayerNotReady(LobbyPlayerId LobbyPlayerId) noexcept {
  auto *Player = Utils::MapFindPtr(Players_, LobbyPlayerId);
  if (!Player) {
    return MakePlayerNotFoundError(LobbyPlayerId);
  }

  if (Player->State == PlayerConnectionState::Ready) {
    Player->State = PlayerConnectionState::NotReady;
    --NumPlayersReady_;
  }

  return Status::Success();
}

Status PrepareGameState::SetPlayerName(LobbyPlayerId LobbyPlayerId, std::string Name) noexcept {
  auto *Player = Utils::MapFindPtr(Players_, LobbyPlayerId);
  if (!Player) {
    return MakePlayerNotFoundError(LobbyPlayerId);
  }

  Player->Name = std::move(Name);
  return Status::Success();
}

Status PrepareGameState::SetPlayerLord(LobbyPlayerId LobbyPlayerId, Id<Lord> LordId) noexcept {
  auto *Player = Utils::MapFindPtr(Players_, LobbyPlayerId);
  if (!Player) {
    return MakePlayerNotFoundError(LobbyPlayerId);
  }

  if (LordId >= Mod_.GetLords().size()) {
    return Status::Error(ErrorCode::ObjectNotFound) << "Lord with id " << LordId << " not found";
  }

  Player->LordId = LordId;
  return Status::Success();
}

Status PrepareGameState::SetPlayerId(LobbyPlayerId LobbyPlayerId, PlayerId PlayerId) noexcept {
  auto *Player = Utils::MapFindPtr(Players_, LobbyPlayerId);
  if (!Player) {
    return MakePlayerNotFoundError(LobbyPlayerId);
  }

  const auto [It, IsNew] = PlayerIdToLobbyId_.emplace(PlayerId, LobbyPlayerId);
  if (!IsNew && It->second != LobbyPlayerId) {
    return Status::Error(ErrorCode::GameSlotBusy)
           << "Player with id " << PlayerId << " is taken by another player with id " << It->second;
  }

  Player->MapId = PlayerId;
  Player->CapitalId = PlayerId.GetValue();
  Player->FractionId = GlobalMap_.GetCapitals()[PlayerId].FractionId;

  LobbyIdToMapId_[LobbyPlayerId] = PlayerId;
  return Status::Success();
}

Status PrepareGameState::PlayerTurnOrderEarlier(LobbyPlayerId LobbyPlayerId) noexcept {
  auto *Player = Utils::MapFindPtr(Players_, LobbyPlayerId);
  if (!Player) {
    return MakePlayerNotFoundError(LobbyPlayerId);
  }

  auto Pos = std::find(TurnOrder_.begin(), TurnOrder_.end(), LobbyPlayerId);
  if (Pos != TurnOrder_.begin()) {
    auto Prev = std::prev(Pos);
    std::swap(*Pos, *Prev);
  }

  return Status::Success();
}

Status PrepareGameState::PlayerTurnOrderLater(LobbyPlayerId LobbyPlayerId) noexcept {
  auto *Player = Utils::MapFindPtr(Players_, LobbyPlayerId);
  if (!Player) {
    return MakePlayerNotFoundError(LobbyPlayerId);
  }

  auto Pos = std::find(TurnOrder_.begin(), TurnOrder_.end(), LobbyPlayerId);
  auto Next = std::next(Pos);
  if (Next != TurnOrder_.end()) {
    std::swap(*Pos, *Next);
  }

  return Status::Success();
}

PlayersList PrepareGameState::CreatePlayers() const noexcept {
  PlayersList Result;
  for (auto Id : TurnOrder_) {
    Result.push_back(Players_.at(Id));
  }
  return Result;
}
/*
PlayerGameState PlayerGameState::MakeInitialState(GameplaySystems &Systems,
                                                  GlobalMap &Map) noexcept {
    for (const auto& Object : Map.GetTowns()) {
        if ()

    }
}*/

PlayerGameState::PlayerGameState(PlayerId PlayerId, const Mod &M, MapState &Map) noexcept
    : Player{PlayerId}, M{M}, Map{Map}, ResourcesGained{M.GetResources()} {
  /*Systems.Visibility.Reset(PlayerId);
    for (Dim Layer = 0, LE = Map.GetNumLayers(); Layer < LE; ++Layer) {
        for (Dim Layer = 0, LE = Map.GetNumLayers(); Layer < LE; ++Layer) {

    }
  for (Dim x = X; x < MaxX; ++x) {
    for (Dim y = Y; y < MaxY; ++y) {
      Systems.LandPropagation.GetTile(Layer, x, y).Object_ = Id;
    }
  }*/
}

OnlineGameState::OnlineGameState(Mod &M, MapState &Map, PlayersList Players) noexcept
    : SavedState{M, Map, static_cast<Size>(Players.size())}, Players{std::move(Players)} {
  /*for (auto &Obj : Map.GetObjects()) {
    Obj.Register(M, GameplaySystems_);
  }*/
  /* for (Dim Layer = 0, Layers = Map.GetNumLayers(); Layer < Layers; ++Layer) {
     for (Dim X = 0, W = Map.GetWidth(); X < W; ++X) {
       for (Dim Y = 0, H = Map.GetHeight(); Y < H; ++Y) {
         GameplaySystems_.LandPropagation.SetOwner(X, Y, Layer, PlayerId);
       }
     }
   }*/
}

} // namespace NotAGame
