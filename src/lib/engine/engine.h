#pragma once

#include "engine/player.h"
#include "entities/global_map.h"
#include "state/state.h"
#include "status/status.h"

namespace NotAGame {

struct PlayerAddResult {
  Size PlayerId;
};

class Engine {
public:
  Engine(Mod &M, GlobalMap &Map) noexcept;
  ErrorOr<PlayerAddResult> AddPlayer(const Player &Player) noexcept;

  ErrorOr<Size> PlayerConnect() noexcept;
  Status PlayerDisconnect(PlayerId Id) noexcept;
  Status PlayerReady(PlayerId Id) noexcept;
  Status PlayerNotReady(PlayerId Id) noexcept;
  Status SetPlayerName(PlayerId Id, std::string Name) noexcept;
  Status SetPlayerLord(PlayerId PlayerId, Id<Lord> LordId) noexcept;
  Status SetPlayerCapital(PlayerId PlayerId, Id<MapObjectPtr> CapitalId) noexcept;
  Status PlayerTurnOrderEarlier(PlayerId PlayerId) noexcept;
  Status PlayerTurnOrderLater(PlayerId PlayerId) noexcept;

  Status StartGame() noexcept;
  Status EndTurn(const Player &Player) noexcept;

private:
  template <typename State, typename Fn>
  auto CheckStateAndCall(Fn &&Func, const char *FnName) noexcept
      -> decltype(Func(static_cast<State *>(nullptr)));

  GameState State_;
};

} // namespace NotAGame
