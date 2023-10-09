#pragma once

#include "engine/player.h"
#include "entities/global_map.h"
#include "state/state.h"
#include "status/status.h"

namespace NotAGame {

struct PlayerAddResult {
  Size PlayerId;
};

struct StartGameResponse {
  SmallVector<Size, kMaxPlayers> TurnOrder;
};

class EventListener {};

class Engine {
public:
  Engine(Mod &M, GlobalMap &Map) noexcept;

  ErrorOr<LobbyPlayerId> PlayerConnect(PlayerSource PlayerSource) noexcept;
  Status PlayerDisconnect(LobbyPlayerId Id) noexcept;
  ErrorOr<StartGameResponse> PlayerReady(LobbyPlayerId Id) noexcept;
  Status PlayerNotReady(LobbyPlayerId Id) noexcept;
  Status SetPlayerName(LobbyPlayerId Id, std::string Name) noexcept;
  Status SetPlayerLord(LobbyPlayerId PlayerId, Id<Lord> LordId) noexcept;
  Status SetPlayerId(LobbyPlayerId LobbyPlayerId, PlayerId PlayerId) noexcept;
  Status PlayerTurnOrderEarlier(LobbyPlayerId LobbyPlayerId) noexcept;
  Status PlayerTurnOrderLater(LobbyPlayerId LobbyPlayerId) noexcept;

  StartGameResponse StartGame() noexcept;
  Status EndTurn(const Player &Player) noexcept;

  void SetEventListener(EventListener *Listener) noexcept { EventListener_ = Listener; }

private:
  template <typename State, typename Fn>
  auto CheckStateAndCall(Fn &&Func, const char *FnName) noexcept
      -> decltype(Func(static_cast<State *>(nullptr)));

  template <typename State, typename Fn>
  auto EnsureStateAndCall(Fn &&Func, const char *FnName) noexcept
      -> decltype(Func(static_cast<State *>(nullptr)));

  GameState State_;
  EventListener *EventListener_;
};

} // namespace NotAGame
